// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES.
 *
 * The iopt_pages is the center of the storage and motion of PFNs. Each
 * iopt_pages represents a logical linear array of full PFNs. The array is 0
 * based and has npages in it. Accessors use 'index' to refer to the entry in
 * this logical array, regardless of its storage location.
 *
 * PFNs are stored in a tiered scheme:
 *  1) iopt_pages::pinned_pfns xarray
 *  2) An iommu_domain
 *  3) The origin of the PFNs, i.e. the userspace pointer
 *
 * PFN have to be copied between all combinations of tiers, depending on the
 * configuration.
 *
 * When a PFN is taken out of the userspace pointer it is pinned exactly once.
 * The storage locations of the PFN's index are tracked in the two interval
 * trees. If no interval includes the index then it is not pinned.
 *
 * If users_itree includes the PFN's index then an in-kernel user has requested
 * the page. The PFN is stored in the xarray so other requestors can continue to
 * find it.
 *
 * If the domains_itree includes the PFN's index then an iommu_domain is storing
 * the PFN and it can be read back using iommu_iova_to_phys(). To avoid
 * duplicating storage the xarray is not used if only iommu_domains are using
 * the PFN's index.
 *
 * As a general principle this is designed so that destroy never fails. This
 * means removing an iommu_domain or releasing a in-kernel user will not fail
 * due to insufficient memory. In practice this means some cases have to hold
 * PFNs in the xarray even though they are also being stored in an iommu_domain.
 *
 * While the iopt_pages can use an iommu_domain as storage, it does not have an
 * IOVA itself. Instead the iopt_area represents a range of IOVA and uses the
 * iopt_pages as the PFN provider. Multiple iopt_areas can share the iopt_pages
 * and reference their own slice of the PFN array, with sub page granularity.
 *
 * In this file the term 'last' indicates an inclusive and closed interval, eg
 * [0,0] refers to a single PFN. 'end' means an open range, eg [0,0) refers to
 * no PFNs.
 */
#include <linux/overflow.h>
#include <linux/slab.h>
#include <linux/iommu.h>
#include <linux/sched/mm.h>

#include "io_pagetable.h"

#define TEMP_MEMORY_LIMIT 65536
#define BATCH_BACKUP_SIZE 32

/*
 * More memory makes pin_user_pages() and the batching more efficient, but as
 * this is only a performance optimization don't try too hard to get it. A 64k
 * allocation can hold about 26M of 4k pages and 13G of 2M pages in an
 * pfn_batch. Various destroy paths cannot fail and provide a small amount of
 * stack memory as a backup contingency. If backup_len is given this cannot
 * fail.
 */
static void *temp_kmalloc(size_t *size, void *backup, size_t backup_len)
{
	void *res;

	if (*size < backup_len)
		return backup;
	*size = min_t(size_t, *size, TEMP_MEMORY_LIMIT);
	res = kmalloc(*size, GFP_KERNEL | __GFP_NOWARN | __GFP_NORETRY);
	if (res)
		return res;
	*size = PAGE_SIZE;
	if (backup_len) {
		res = kmalloc(*size, GFP_KERNEL | __GFP_NOWARN | __GFP_NORETRY);
		if (res)
			return res;
		*size = backup_len;
		return backup;
	}
	return kmalloc(*size, GFP_KERNEL);
}

static void iopt_pages_add_npinned(struct iopt_pages *pages, size_t npages)
{
	int rc;

	rc = check_add_overflow(pages->npinned, npages, &pages->npinned);
	if (IS_ENABLED(CONFIG_IOMMUFD_TEST))
		WARN_ON(rc || pages->npinned > pages->npages);
}

static void iopt_pages_sub_npinned(struct iopt_pages *pages, size_t npages)
{
	int rc;

	rc = check_sub_overflow(pages->npinned, npages, &pages->npinned);
	if (IS_ENABLED(CONFIG_IOMMUFD_TEST))
		WARN_ON(rc || pages->npinned > pages->npages);
}

/*
 * index is the number of PAGE_SIZE units from the start of the area's
 * iopt_pages. If the iova is sub page-size then the area has an iova that
 * covers a portion of the first and last pages in the range.
 */
static unsigned long iopt_area_index_to_iova(struct iopt_area *area,
					     unsigned long index)
{
	if (IS_ENABLED(CONFIG_IOMMUFD_TEST))
		WARN_ON(index < iopt_area_index(area) ||
			index > iopt_area_last_index(area));
	index -= iopt_area_index(area);
	if (index == 0)
		return iopt_area_iova(area);
	return iopt_area_iova(area) - area->page_offset + index * PAGE_SIZE;
}

static unsigned long iopt_area_index_to_iova_last(struct iopt_area *area,
						  unsigned long index)
{
	if (IS_ENABLED(CONFIG_IOMMUFD_TEST))
		WARN_ON(index < iopt_area_index(area) ||
			index > iopt_area_last_index(area));
	if (index == iopt_area_last_index(area))
		return iopt_area_last_iova(area);
	return iopt_area_iova(area) - area->page_offset +
	       (index - iopt_area_index(area) + 1) * PAGE_SIZE - 1;
}

static void iommu_unmap_nofail(struct iommu_domain *domain, unsigned long iova,
			       size_t size)
{
	size_t ret;

	ret = iommu_unmap(domain, iova, size);
	/*
	 * It is a logic error in this code or a driver bug if the IOMMU unmaps
	 * something other than exactly as requested.
	 */
	WARN_ON(ret != size);
}

static struct iopt_area *iopt_pages_find_domain_area(struct iopt_pages *pages,
						     unsigned long index)
{
	struct interval_tree_node *node;

	node = interval_tree_iter_first(&pages->domains_itree, index, index);
	if (!node)
		return NULL;
	return container_of(node, struct iopt_area, pages_node);
}

/*
 * A simple datastructure to hold a vector of PFNs, optimized for contiguous
 * PFNs. This is used as a temporary holding memory for shuttling pfns from one
 * place to another. Generally everything is made more efficient if operations
 * work on the largest possible grouping of pfns. eg fewer lock/unlock cycles,
 * better cache locality, etc
 */
struct pfn_batch {
	unsigned long *pfns;
	u16 *npfns;
	unsigned int array_size;
	unsigned int end;
	unsigned int total_pfns;
};

static void batch_clear(struct pfn_batch *batch)
{
	batch->total_pfns = 0;
	batch->end = 0;
	batch->pfns[0] = 0;
	batch->npfns[0] = 0;
}

static int __batch_init(struct pfn_batch *batch, size_t max_pages, void *backup,
			size_t backup_len)
{
	const size_t elmsz = sizeof(*batch->pfns) + sizeof(*batch->npfns);
	size_t size = max_pages * elmsz;

	batch->pfns = temp_kmalloc(&size, backup, backup_len);
	if (!batch->pfns)
		return -ENOMEM;
	batch->array_size = size / elmsz;
	batch->npfns = (u16 *)(batch->pfns + batch->array_size);
	batch_clear(batch);
	return 0;
}

static int batch_init(struct pfn_batch *batch, size_t max_pages)
{
	return __batch_init(batch, max_pages, NULL, 0);
}

static void batch_init_backup(struct pfn_batch *batch, size_t max_pages,
			      void *backup, size_t backup_len)
{
	__batch_init(batch, max_pages, backup, backup_len);
}

static void batch_destroy(struct pfn_batch *batch, void *backup)
{
	if (batch->pfns != backup)
		kfree(batch->pfns);
}

/* true if the pfn could be added, false otherwise */
static bool batch_add_pfn(struct pfn_batch *batch, unsigned long pfn)
{
	/* FIXME: U16 is too small */
	if (batch->end &&
	    pfn == batch->pfns[batch->end - 1] + batch->npfns[batch->end - 1] &&
	    batch->npfns[batch->end - 1] != U16_MAX) {
		batch->npfns[batch->end - 1]++;
		batch->total_pfns++;
		return true;
	}
	if (batch->end == batch->array_size)
		return false;
	batch->total_pfns++;
	batch->pfns[batch->end] = pfn;
	batch->npfns[batch->end] = 1;
	batch->end++;
	return true;
}

/*
 * Fill the batch with pfns from the domain. When the batch is full, or it
 * reaches last_index, the function will return. The caller should use
 * batch->total_pfns to determine the starting point for the next iteration.
 */
static void batch_from_domain(struct pfn_batch *batch,
			      struct iommu_domain *domain,
			      struct iopt_area *area, unsigned long index,
			      unsigned long last_index)
{
	unsigned int page_offset = 0;
	unsigned long iova;
	phys_addr_t phys;

	batch_clear(batch);
	iova = iopt_area_index_to_iova(area, index);
	if (index == iopt_area_index(area))
		page_offset = area->page_offset;
	while (index <= last_index) {
		/*
		 * This is pretty slow, it would be nice to get the page size
		 * back from the driver, or have the driver directly fill the
		 * batch.
		 */
		phys = iommu_iova_to_phys(domain, iova) - page_offset;
		if (!batch_add_pfn(batch, PHYS_PFN(phys)))
			return;
		iova += PAGE_SIZE - page_offset;
		page_offset = 0;
		index++;
	}
}

static int batch_to_domain(struct pfn_batch *batch, struct iommu_domain *domain,
			   struct iopt_area *area, unsigned long start_index)
{
	unsigned long last_iova = iopt_area_last_iova(area);
	unsigned int page_offset = 0;
	unsigned long start_iova;
	unsigned long next_iova;
	unsigned int cur = 0;
	unsigned long iova;
	int rc;

	/* The first index might be a partial page */
	if (start_index == iopt_area_index(area))
		page_offset = area->page_offset;
	next_iova = iova = start_iova =
		iopt_area_index_to_iova(area, start_index);
	while (cur < batch->end) {
		next_iova = min(last_iova + 1,
				next_iova + batch->npfns[cur] * PAGE_SIZE -
					page_offset);
		rc = iommu_map(domain, iova,
			       PFN_PHYS(batch->pfns[cur]) + page_offset,
			       next_iova - iova, area->iommu_prot);
		if (rc)
			goto out_unmap;
		iova = next_iova;
		page_offset = 0;
		cur++;
	}
	return 0;
out_unmap:
	if (start_iova != iova)
		iommu_unmap_nofail(domain, start_iova, iova - start_iova);
	return rc;
}

static void batch_from_xarray(struct pfn_batch *batch, struct xarray *xa,
			      unsigned long start_index,
			      unsigned long last_index)
{
	XA_STATE(xas, xa, start_index);
	void *entry;

	rcu_read_lock();
	while (true) {
		entry = xas_next(&xas);
		if (xas_retry(&xas, entry))
			continue;
		WARN_ON(!xa_is_value(entry));
		if (!batch_add_pfn(batch, xa_to_value(entry)) ||
		    start_index == last_index)
			break;
		start_index++;
	}
	rcu_read_unlock();
}

static void clear_xarray(struct xarray *xa, unsigned long index,
			 unsigned long last)
{
	XA_STATE(xas, xa, index);
	void *entry;

	xas_lock(&xas);
	xas_for_each (&xas, entry, last)
		xas_store(&xas, NULL);
	xas_unlock(&xas);
}

static int batch_to_xarray(struct pfn_batch *batch, struct xarray *xa,
			   unsigned long start_index)
{
	XA_STATE(xas, xa, start_index);
	unsigned int npage = 0;
	unsigned int cur = 0;

	do {
		xas_lock(&xas);
		while (cur < batch->end) {
			void *old;

			old = xas_store(&xas,
					xa_mk_value(batch->pfns[cur] + npage));
			if (xas_error(&xas))
				break;
			WARN_ON(old);
			npage++;
			if (npage == batch->npfns[cur]) {
				npage = 0;
				cur++;
			}
			xas_next(&xas);
		}
		xas_unlock(&xas);
	} while (xas_nomem(&xas, GFP_KERNEL));

	if (xas_error(&xas)) {
		if (xas.xa_index != start_index)
			clear_xarray(xa, start_index, xas.xa_index - 1);
		return xas_error(&xas);
	}
	return 0;
}

static void batch_to_pages(struct pfn_batch *batch, struct page **pages)
{
	unsigned int npage = 0;
	unsigned int cur = 0;

	while (cur < batch->end) {
		*pages++ = pfn_to_page(batch->pfns[cur] + npage);
		npage++;
		if (npage == batch->npfns[cur]) {
			npage = 0;
			cur++;
		}
	}
}

static void batch_from_pages(struct pfn_batch *batch, struct page **pages,
			     size_t npages)
{
	struct page **end = pages + npages;

	for (; pages != end; pages++)
		if (!batch_add_pfn(batch, page_to_pfn(*pages)))
			break;
}

static void batch_unpin(struct pfn_batch *batch, struct iopt_pages *pages,
			unsigned int offset, size_t npages)
{
	unsigned int cur = 0;

	while (offset) {
		if (batch->npfns[cur] > offset)
			break;
		offset -= batch->npfns[cur];
		cur++;
	}

	while (npages) {
		size_t to_unpin =
			min_t(size_t, npages, batch->npfns[cur] - offset);

		unpin_user_page_range_dirty_lock(
			pfn_to_page(batch->pfns[cur] + offset), to_unpin,
			pages->writable);
		iopt_pages_sub_npinned(pages, to_unpin);
		cur++;
		offset = 0;
		npages -= to_unpin;
	}
}

/*
 * PFNs are stored in three places, in order of preference:
 * - The iopt_pages xarray. This is only populated if there is a
 *   iopt_pages_user
 * - The iommu_domain under an area
 * - The original PFN source, ie pages->source_mm
 *
 * This iterator reads the pfns optimizing to load according to the
 * above order.
 */
struct pfn_reader {
	struct iopt_pages *pages;
	struct interval_tree_span_iter span;
	struct pfn_batch batch;
	unsigned long batch_start_index;
	unsigned long batch_end_index;
	unsigned long last_index;

	struct page **upages;
	size_t upages_len;
	unsigned long upages_start;
	unsigned long upages_end;

	unsigned int gup_flags;
};

static void update_unpinned(struct iopt_pages *pages)
{
	unsigned long npages = pages->last_npinned - pages->npinned;

	lockdep_assert_held(&pages->mutex);

	if (pages->has_cap_ipc_lock) {
		pages->last_npinned = pages->npinned;
		return;
	}

	if (WARN_ON(pages->npinned > pages->last_npinned) ||
	    WARN_ON(atomic_long_read(&pages->source_user->locked_vm) < npages))
		return;
	atomic_long_sub(npages, &pages->source_user->locked_vm);
	atomic64_sub(npages, &pages->source_mm->pinned_vm);
	pages->last_npinned = pages->npinned;
}

/*
 * Changes in the number of pages pinned is done after the pages have been read
 * and processed. If the user lacked the limit then the error unwind will unpin
 * everything that was just pinned.
 */
static int update_pinned(struct iopt_pages *pages)
{
	unsigned long lock_limit;
	unsigned long cur_pages;
	unsigned long new_pages;
	unsigned long npages;

	lockdep_assert_held(&pages->mutex);

	if (pages->has_cap_ipc_lock) {
		pages->last_npinned = pages->npinned;
		return 0;
	}

	if (pages->npinned == pages->last_npinned)
		return 0;

	if (pages->npinned < pages->last_npinned) {
		update_unpinned(pages);
		return 0;
	}

	lock_limit =
		task_rlimit(pages->source_task, RLIMIT_MEMLOCK) >> PAGE_SHIFT;
	npages = pages->npinned - pages->last_npinned;
	do {
		cur_pages = atomic_long_read(&pages->source_user->locked_vm);
		new_pages = cur_pages + npages;
		if (new_pages > lock_limit)
			return -ENOMEM;
	} while (atomic_long_cmpxchg(&pages->source_user->locked_vm, cur_pages,
				     new_pages) != cur_pages);
	atomic64_add(npages, &pages->source_mm->pinned_vm);
	pages->last_npinned = pages->npinned;
	return 0;
}

static int pfn_reader_pin_pages(struct pfn_reader *pfns)
{
	struct iopt_pages *pages = pfns->pages;
	unsigned long npages;
	long rc;

	if (!pfns->upages) {
		/* All undone in iopt_pfn_reader_destroy */
		pfns->upages_len =
			(pfns->last_index - pfns->batch_end_index + 1) *
			sizeof(*pfns->upages);
		pfns->upages = temp_kmalloc(&pfns->upages_len, NULL, 0);
		if (!pfns->upages)
			return -ENOMEM;

		if (!mmget_not_zero(pages->source_mm)) {
			kfree(pfns->upages);
			pfns->upages = NULL;
			return -EINVAL;
		}
		mmap_read_lock(pages->source_mm);
	}

	npages = min_t(unsigned long,
		       pfns->span.last_hole - pfns->batch_end_index + 1,
		       pfns->upages_len / sizeof(*pfns->upages));

	/* FIXME use pin_user_pages_fast() if current == source_mm */
	rc = pin_user_pages_remote(
		pages->source_mm,
		(uintptr_t)(pages->uptr + pfns->batch_end_index * PAGE_SIZE),
		npages, pfns->gup_flags, pfns->upages, NULL, NULL);
	if (rc < 0)
		return rc;
	if (WARN_ON(!rc))
		return -EFAULT;
	iopt_pages_add_npinned(pages, rc);
	pfns->upages_start = pfns->batch_end_index;
	pfns->upages_end = pfns->batch_end_index + rc;
	return 0;
}

/*
 * The batch can contain a mixture of pages that are still in use and pages that
 * need to be unpinned. Unpin only pages that are not held anywhere else.
 */
static void iopt_pages_unpin(struct iopt_pages *pages, struct pfn_batch *batch,
			     unsigned long index, unsigned long last)
{
	struct interval_tree_span_iter user_span;
	struct interval_tree_span_iter area_span;

	lockdep_assert_held(&pages->mutex);

	for (interval_tree_span_iter_first(&user_span, &pages->users_itree, 0,
					   last);
	     !interval_tree_span_iter_done(&user_span);
	     interval_tree_span_iter_next(&user_span)) {
		if (!user_span.is_hole)
			continue;

		for (interval_tree_span_iter_first(
			     &area_span, &pages->domains_itree,
			     user_span.start_hole, user_span.last_hole);
		     !interval_tree_span_iter_done(&area_span);
		     interval_tree_span_iter_next(&area_span)) {
			if (!area_span.is_hole)
				continue;

			batch_unpin(batch, pages, area_span.start_hole - index,
				    area_span.last_hole - area_span.start_hole +
					    1);
		}
	}
}

/* Process a single span in the users_itree */
static int pfn_reader_fill_span(struct pfn_reader *pfns)
{
	struct interval_tree_span_iter *span = &pfns->span;
	struct iopt_area *area;
	int rc;

	if (!span->is_hole) {
		batch_from_xarray(&pfns->batch, &pfns->pages->pinned_pfns,
				  pfns->batch_end_index, span->last_used);
		return 0;
	}

	/* FIXME: This should consider the entire hole remaining */
	area = iopt_pages_find_domain_area(pfns->pages, pfns->batch_end_index);
	if (area) {
		unsigned int last_index;

		last_index = min(iopt_area_last_index(area), span->last_hole);
		/* The storage_domain cannot change without the pages mutex */
		batch_from_domain(&pfns->batch, area->storage_domain, area,
				  pfns->batch_end_index, last_index);
		return 0;
	}

	if (pfns->batch_end_index >= pfns->upages_end) {
		rc = pfn_reader_pin_pages(pfns);
		if (rc)
			return rc;
	}

	batch_from_pages(&pfns->batch,
			 pfns->upages +
				 (pfns->batch_end_index - pfns->upages_start),
			 pfns->upages_end - pfns->batch_end_index);
	return 0;
}

static bool pfn_reader_done(struct pfn_reader *pfns)
{
	return pfns->batch_start_index == pfns->last_index + 1;
}

static int pfn_reader_next(struct pfn_reader *pfns)
{
	int rc;

	batch_clear(&pfns->batch);
	pfns->batch_start_index = pfns->batch_end_index;
	while (pfns->batch_end_index != pfns->last_index + 1) {
		rc = pfn_reader_fill_span(pfns);
		if (rc)
			return rc;
		pfns->batch_end_index =
			pfns->batch_start_index + pfns->batch.total_pfns;
		if (pfns->batch_end_index != pfns->span.last_used + 1)
			return 0;
		interval_tree_span_iter_next(&pfns->span);
	}
	return 0;
}

/*
 * Adjust the pfn_reader to start at an externally determined hole span in the
 * users_itree.
 */
static int pfn_reader_seek_hole(struct pfn_reader *pfns,
				struct interval_tree_span_iter *span)
{
	pfns->batch_start_index = span->start_hole;
	pfns->batch_end_index = span->start_hole;
	pfns->last_index = span->last_hole;
	pfns->span = *span;
	return pfn_reader_next(pfns);
}

static int pfn_reader_init(struct pfn_reader *pfns, struct iopt_pages *pages,
			   unsigned long index, unsigned long last)
{
	int rc;

	lockdep_assert_held(&pages->mutex);

	rc = batch_init(&pfns->batch, last - index + 1);
	if (rc)
		return rc;
	pfns->pages = pages;
	pfns->batch_start_index = index;
	pfns->batch_end_index = index;
	pfns->last_index = last;
	pfns->upages = NULL;
	pfns->upages_start = 0;
	pfns->upages_end = 0;
	interval_tree_span_iter_first(&pfns->span, &pages->users_itree, index,
				      last);

	if (pages->writable) {
		pfns->gup_flags = FOLL_LONGTERM | FOLL_WRITE;
	} else {
		/* Still need to break COWs on read */
		pfns->gup_flags = FOLL_LONGTERM | FOLL_FORCE | FOLL_WRITE;
	}
	return 0;
}

static void pfn_reader_destroy(struct pfn_reader *pfns)
{
	if (pfns->upages) {
		size_t npages = pfns->upages_end - pfns->batch_end_index;

		mmap_read_unlock(pfns->pages->source_mm);
		mmput(pfns->pages->source_mm);

		/* Any pages not transferred to the batch are just unpinned */
		unpin_user_pages(pfns->upages + (pfns->batch_end_index -
						 pfns->upages_start),
				 npages);
		kfree(pfns->upages);
		pfns->upages = NULL;
	}

	if (pfns->batch_start_index != pfns->batch_end_index)
		iopt_pages_unpin(pfns->pages, &pfns->batch,
				 pfns->batch_start_index,
				 pfns->batch_end_index - 1);
	batch_destroy(&pfns->batch, NULL);
	WARN_ON(pfns->pages->last_npinned != pfns->pages->npinned);
}

static int pfn_reader_first(struct pfn_reader *pfns, struct iopt_pages *pages,
			    unsigned long index, unsigned long last)
{
	int rc;

	rc = pfn_reader_init(pfns, pages, index, last);
	if (rc)
		return rc;
	rc = pfn_reader_next(pfns);
	if (rc) {
		pfn_reader_destroy(pfns);
		return rc;
	}
	return 0;
}

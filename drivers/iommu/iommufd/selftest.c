// SPDX-License-Identifier: GPL-2.0
/* Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES.
 *
 * Kernel side components to support tools/testing/selftests/iommu
 */
#include <linux/slab.h>
#include <linux/iommu.h>
#include <linux/xarray.h>

#include "iommufd_private.h"
#include "iommufd_test.h"

size_t iommufd_test_memory_limit = 65536;

enum {
	MOCK_IO_PAGE_SIZE = PAGE_SIZE / 2,

	/*
	 * Like a real page table alignment requires the low bits of the address
	 * to be zero. xarray also requires the high bit to be zero, so we store
	 * the pfns shifted. The upper bits are used for metadata.
	 */
	MOCK_PFN_MASK = ULONG_MAX / MOCK_IO_PAGE_SIZE,

	_MOCK_PFN_START = MOCK_PFN_MASK + 1,
	MOCK_PFN_START_IOVA = _MOCK_PFN_START,
	MOCK_PFN_LAST_IOVA = _MOCK_PFN_START,
};

struct mock_iommu_domain {
	struct iommu_domain domain;
	struct xarray pfns;
};

enum selftest_obj_type {
	TYPE_ACCESS,
	TYPE_IDEV,
};

struct selftest_obj {
	struct iommufd_object obj;
	enum selftest_obj_type type;

	union {
		struct {
			struct iommufd_ioas *ioas;
			unsigned long iova;
			size_t length;
		} access;
		struct {
			struct iommufd_hw_pagetable *hwpt;
			struct iommufd_ctx *ictx;
		} idev;
	};
};

static struct iommu_domain *mock_domain_alloc(unsigned int iommu_domain_type)
{
	struct mock_iommu_domain *mock;

	if (WARN_ON(iommu_domain_type != IOMMU_DOMAIN_UNMANAGED))
		return NULL;

	mock = kzalloc(sizeof(*mock), GFP_KERNEL);
	if (!mock)
		return NULL;
	mock->domain.geometry.aperture_start = MOCK_APERTURE_START;
	mock->domain.geometry.aperture_end = MOCK_APERTURE_LAST;
	mock->domain.pgsize_bitmap = MOCK_IO_PAGE_SIZE;
	xa_init(&mock->pfns);
	return &mock->domain;
}

static void mock_domain_free(struct iommu_domain *domain)
{
	struct mock_iommu_domain *mock =
		container_of(domain, struct mock_iommu_domain, domain);

	WARN_ON(!xa_empty(&mock->pfns));
	kfree(mock);
}

static int mock_domain_map_pages(struct iommu_domain *domain,
				 unsigned long iova, phys_addr_t paddr,
				 size_t pgsize, size_t pgcount, int prot,
				 gfp_t gfp, size_t *mapped)
{
	struct mock_iommu_domain *mock =
		container_of(domain, struct mock_iommu_domain, domain);
	unsigned long flags = MOCK_PFN_START_IOVA;

	WARN_ON(iova % MOCK_IO_PAGE_SIZE);
	WARN_ON(pgsize % MOCK_IO_PAGE_SIZE);
	for (; pgcount; pgcount--) {
		size_t cur;

		for (cur = 0; cur != pgsize; cur += MOCK_IO_PAGE_SIZE) {
			void *old;

			if (pgcount == 1 && cur + MOCK_IO_PAGE_SIZE == pgsize)
				flags = MOCK_PFN_LAST_IOVA;
			old = xa_store(&mock->pfns, iova / MOCK_IO_PAGE_SIZE,
				       xa_mk_value((paddr / MOCK_IO_PAGE_SIZE) | flags),
				       GFP_KERNEL);
			if (xa_is_err(old))
				return xa_err(old);
			WARN_ON(old);
			iova += MOCK_IO_PAGE_SIZE;
			paddr += MOCK_IO_PAGE_SIZE;
			*mapped += MOCK_IO_PAGE_SIZE;
			flags = 0;
		}
	}
	return 0;
}

static size_t mock_domain_unmap_pages(struct iommu_domain *domain,
				      unsigned long iova, size_t pgsize,
				      size_t pgcount,
				      struct iommu_iotlb_gather *iotlb_gather)
{
	struct mock_iommu_domain *mock =
		container_of(domain, struct mock_iommu_domain, domain);
	bool first = true;
	size_t ret = 0;
	void *ent;

	WARN_ON(iova % MOCK_IO_PAGE_SIZE);
	WARN_ON(pgsize % MOCK_IO_PAGE_SIZE);

	for (; pgcount; pgcount--) {
		size_t cur;

		for (cur = 0; cur != pgsize; cur += MOCK_IO_PAGE_SIZE) {
			ent = xa_erase(&mock->pfns, iova / MOCK_IO_PAGE_SIZE);
			WARN_ON(!ent);
			/*
			 * iommufd generates unmaps that must be a strict
			 * superset of the map's performend So every starting
			 * IOVA should have been an iova passed to map, and the
			 *
			 * First IOVA must be present and have been a first IOVA
			 * passed to map_pages
			 */
			if (first) {
				WARN_ON(!(xa_to_value(ent) &
					  MOCK_PFN_START_IOVA));
				first = false;
			}
			if (pgcount == 1 && cur + MOCK_IO_PAGE_SIZE == pgsize)
				WARN_ON(!(xa_to_value(ent) &
					  MOCK_PFN_LAST_IOVA));

			iova += MOCK_IO_PAGE_SIZE;
			ret += MOCK_IO_PAGE_SIZE;
		}
	}
	return ret;
}

static phys_addr_t mock_domain_iova_to_phys(struct iommu_domain *domain,
					    dma_addr_t iova)
{
	struct mock_iommu_domain *mock =
		container_of(domain, struct mock_iommu_domain, domain);
	void *ent;

	WARN_ON(iova % MOCK_IO_PAGE_SIZE);
	ent = xa_load(&mock->pfns, iova / MOCK_IO_PAGE_SIZE);
	WARN_ON(!ent);
	return (xa_to_value(ent) & MOCK_PFN_MASK) * MOCK_IO_PAGE_SIZE;
}

static const struct iommu_ops mock_ops = {
	.owner = THIS_MODULE,
	.pgsize_bitmap = MOCK_IO_PAGE_SIZE,
	.domain_alloc = mock_domain_alloc,
	.default_domain_ops =
		&(struct iommu_domain_ops){
			.free = mock_domain_free,
			.map_pages = mock_domain_map_pages,
			.unmap_pages = mock_domain_unmap_pages,
			.iova_to_phys = mock_domain_iova_to_phys,
		},
};

static inline struct iommufd_hw_pagetable *
get_md_pagetable(struct iommufd_ucmd *ucmd, u32 mockpt_id,
		 struct mock_iommu_domain **mock)
{
	struct iommufd_hw_pagetable *hwpt;
	struct iommufd_object *obj;

	obj = iommufd_get_object(ucmd->ictx, mockpt_id,
				 IOMMUFD_OBJ_HW_PAGETABLE);
	if (IS_ERR(obj))
		return ERR_CAST(obj);
	hwpt = container_of(obj, struct iommufd_hw_pagetable, obj);
	if (hwpt->domain->ops != mock_ops.default_domain_ops) {
		return ERR_PTR(-EINVAL);
		iommufd_put_object(&hwpt->obj);
	}
	*mock = container_of(hwpt->domain, struct mock_iommu_domain, domain);
	return hwpt;
}

/* Create an hw_pagetable with the mock domain so we can test the domain ops */
static int iommufd_test_mock_domain(struct iommufd_ucmd *ucmd,
				    struct iommu_test_cmd *cmd)
{
	struct bus_type mock_bus = { .iommu_ops = &mock_ops };
	struct device mock_dev = { .bus = &mock_bus };
	struct iommufd_hw_pagetable *hwpt;
	struct selftest_obj *sobj;
	int rc;

	sobj = iommufd_object_alloc(ucmd->ictx, sobj, IOMMUFD_OBJ_SELFTEST);
	if (IS_ERR(sobj))
		return PTR_ERR(sobj);
	sobj->idev.ictx = ucmd->ictx;
	sobj->type = TYPE_IDEV;

	hwpt = iommufd_hw_pagetable_from_id(ucmd->ictx, cmd->id, &mock_dev);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_sobj;
	}
	if (WARN_ON(refcount_read(&hwpt->obj.users) != 2)) {
		rc = -EINVAL;
		goto out_hwpt;
	}
	sobj->idev.hwpt = hwpt;

	/* Creating a real iommufd_device is too hard, fake one */
	rc = iopt_table_add_domain(&hwpt->ioas->iopt, hwpt->domain);
	if (rc)
		goto out_hwpt;

	/* Convert auto domain to user created */
	list_del_init(&hwpt->auto_domains_item);
	cmd->id = hwpt->obj.id;
	cmd->mock_domain.device_id = sobj->obj.id;
	iommufd_object_finalize(ucmd->ictx, &sobj->obj);
	return iommufd_ucmd_respond(ucmd, sizeof(*cmd));

out_hwpt:
	iommufd_hw_pagetable_put(ucmd->ictx, hwpt);
out_sobj:
	iommufd_object_abort(ucmd->ictx, &sobj->obj);
	return rc;
}

/* Add an additional reserved IOVA to the IOAS */
static int iommufd_test_add_reserved(struct iommufd_ucmd *ucmd,
				     unsigned int mockpt_id,
				     unsigned long start, size_t length)
{
	struct iommufd_ioas *ioas;
	int rc;

	ioas = iommufd_get_ioas(ucmd, mockpt_id);
	if (IS_ERR(ioas))
		return PTR_ERR(ioas);
	down_write(&ioas->iopt.iova_rwsem);
	rc = iopt_reserve_iova(&ioas->iopt, start, start + length - 1, NULL);
	up_write(&ioas->iopt.iova_rwsem);
	iommufd_put_object(&ioas->obj);
	return rc;
}

/* Check that every pfn under each iova matches the pfn under a user VA */
static int iommufd_test_md_check_pa(struct iommufd_ucmd *ucmd,
				    unsigned int mockpt_id, unsigned long iova,
				    size_t length, void __user *uptr)
{
	struct iommufd_hw_pagetable *hwpt;
	struct mock_iommu_domain *mock;
	int rc;

	if (iova % MOCK_IO_PAGE_SIZE || length % MOCK_IO_PAGE_SIZE ||
	    (uintptr_t)uptr % MOCK_IO_PAGE_SIZE)
		return -EINVAL;

	hwpt = get_md_pagetable(ucmd, mockpt_id, &mock);
	if (IS_ERR(hwpt))
		return PTR_ERR(hwpt);

	for (; length; length -= MOCK_IO_PAGE_SIZE) {
		struct page *pages[1];
		unsigned long pfn;
		long npages;
		void *ent;

		npages = get_user_pages_fast((uintptr_t)uptr & PAGE_MASK, 1, 0,
					     pages);
		if (npages < 0) {
			rc = npages;
			goto out_put;
		}
		if (WARN_ON(npages != 1)) {
			rc = -EFAULT;
			goto out_put;
		}
		pfn = page_to_pfn(pages[0]);
		put_page(pages[0]);

		ent = xa_load(&mock->pfns, iova / MOCK_IO_PAGE_SIZE);
		if (!ent ||
		    (xa_to_value(ent) & MOCK_PFN_MASK) * MOCK_IO_PAGE_SIZE !=
			    pfn * PAGE_SIZE + ((uintptr_t)uptr % PAGE_SIZE)) {
			rc = -EINVAL;
			goto out_put;
		}
		iova += MOCK_IO_PAGE_SIZE;
		uptr += MOCK_IO_PAGE_SIZE;
	}
	rc = 0;

out_put:
	iommufd_put_object(&hwpt->obj);
	return rc;
}

/* Check that the page ref count matches, to look for missing pin/unpins */
static int iommufd_test_md_check_refs(struct iommufd_ucmd *ucmd,
				      void __user *uptr, size_t length,
				      unsigned int refs)
{
	if (length % PAGE_SIZE || (uintptr_t)uptr % PAGE_SIZE)
		return -EINVAL;

	for (; length; length -= PAGE_SIZE) {
		struct page *pages[1];
		long npages;

		npages = get_user_pages_fast((uintptr_t)uptr, 1, 0, pages);
		if (npages < 0)
			return npages;
		if (WARN_ON(npages != 1))
			return -EFAULT;
		if (!PageCompound(pages[0])) {
			unsigned int count;

			count = page_ref_count(pages[0]);
			if (count / GUP_PIN_COUNTING_BIAS != refs) {
				put_page(pages[0]);
				return -EIO;
			}
		}
		put_page(pages[0]);
		uptr += PAGE_SIZE;
	}
	return 0;
}

/* Check that the pages in a page array match the pages in the user VA */
static int iommufd_test_check_pages(void __user *uptr, struct page **pages,
				    size_t npages)
{
	for (; npages; npages--) {
		struct page *tmp_pages[1];
		long rc;

		rc = get_user_pages_fast((uintptr_t)uptr, 1, 0, tmp_pages);
		if (rc < 0)
			return rc;
		if (WARN_ON(rc != 1))
			return -EFAULT;
		put_page(tmp_pages[0]);
		if (tmp_pages[0] != *pages)
			return -EBADE;
		pages++;
		uptr += PAGE_SIZE;
	}
	return 0;
}

/* Test iopt_access_pages() by checking it returns the correct pages */
static int iommufd_test_access_pages(struct iommufd_ucmd *ucmd,
				     unsigned int ioas_id, unsigned long iova,
				     size_t length, void __user *uptr,
				     u32 flags)
{
	struct iommu_test_cmd *cmd = ucmd->cmd;
	struct selftest_obj *sobj;
	struct page **pages;
	size_t npages;
	int rc;

	if (flags & ~MOCK_FLAGS_ACCESS_WRITE)
		return -EOPNOTSUPP;

	sobj = iommufd_object_alloc(ucmd->ictx, sobj, IOMMUFD_OBJ_SELFTEST);
	if (IS_ERR(sobj))
		return PTR_ERR(sobj);
	sobj->type = TYPE_ACCESS;

	npages = (ALIGN(iova + length, PAGE_SIZE) -
		  ALIGN_DOWN(iova, PAGE_SIZE)) /
		 PAGE_SIZE;
	pages = kvcalloc(npages, sizeof(*pages), GFP_KERNEL);
	if (!pages) {
		rc = -ENOMEM;
		goto out_abort;
	}

	sobj->access.ioas = iommufd_get_ioas(ucmd, ioas_id);
	if (IS_ERR(sobj->access.ioas)) {
		rc = PTR_ERR(sobj->access.ioas);
		goto out_free;
	}

	sobj->access.iova = iova;
	sobj->access.length = length;
	rc = iopt_access_pages(&sobj->access.ioas->iopt, iova, length, pages,
			       flags & MOCK_FLAGS_ACCESS_WRITE);
	if (rc)
		goto out_put;

	rc = iommufd_test_check_pages(
		uptr - (iova - ALIGN_DOWN(iova, PAGE_SIZE)), pages, npages);
	if (rc)
		goto out_unaccess;

	cmd->access_pages.out_access_id = sobj->obj.id;
	rc = iommufd_ucmd_respond(ucmd, sizeof(*cmd));
	if (rc)
		goto out_unaccess;

	iommufd_object_finalize(ucmd->ictx, &sobj->obj);
	iommufd_put_object_keep_user(&sobj->access.ioas->obj);
	kvfree(pages);
	return 0;
out_unaccess:
	iopt_unaccess_pages(&sobj->access.ioas->iopt, iova, length);
out_put:
	iommufd_put_object(&sobj->access.ioas->obj);
out_free:
	kvfree(pages);
out_abort:
	iommufd_object_abort(ucmd->ictx, &sobj->obj);
	return rc;
}

void iommufd_selftest_destroy(struct iommufd_object *obj)
{
	struct selftest_obj *sobj = container_of(obj, struct selftest_obj, obj);

	switch (sobj->type) {
	case TYPE_IDEV:
		iopt_table_remove_domain(&sobj->idev.hwpt->ioas->iopt,
					 sobj->idev.hwpt->domain);
		iommufd_hw_pagetable_put(sobj->idev.ictx, sobj->idev.hwpt);
		break;
	case TYPE_ACCESS:
		iopt_unaccess_pages(&sobj->access.ioas->iopt,
				    sobj->access.iova, sobj->access.length);
		refcount_dec(&sobj->access.ioas->obj.users);
		break;
	}
}

int iommufd_test(struct iommufd_ucmd *ucmd)
{
	struct iommu_test_cmd *cmd = ucmd->cmd;

	switch (cmd->op) {
	case IOMMU_TEST_OP_ADD_RESERVED:
		return iommufd_test_add_reserved(ucmd, cmd->id,
						 cmd->add_reserved.start,
						 cmd->add_reserved.length);
	case IOMMU_TEST_OP_MOCK_DOMAIN:
		return iommufd_test_mock_domain(ucmd, cmd);
	case IOMMU_TEST_OP_MD_CHECK_MAP:
		return iommufd_test_md_check_pa(
			ucmd, cmd->id, cmd->check_map.iova,
			cmd->check_map.length,
			u64_to_user_ptr(cmd->check_map.uptr));
	case IOMMU_TEST_OP_MD_CHECK_REFS:
		return iommufd_test_md_check_refs(
			ucmd, u64_to_user_ptr(cmd->check_refs.uptr),
			cmd->check_refs.length, cmd->check_refs.refs);
	case IOMMU_TEST_OP_ACCESS_PAGES:
		return iommufd_test_access_pages(
			ucmd, cmd->id, cmd->access_pages.iova,
			cmd->access_pages.length,
			u64_to_user_ptr(cmd->access_pages.uptr),
			cmd->access_pages.flags);
	case IOMMU_TEST_OP_SET_TEMP_MEMORY_LIMIT:
		iommufd_test_memory_limit = cmd->memory_limit.limit;
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

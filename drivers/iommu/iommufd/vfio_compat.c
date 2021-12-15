// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#include <linux/file.h>
#include <linux/interval_tree.h>
#include <linux/iommu.h>
#include <linux/iommufd.h>
#include <linux/slab.h>
#include <linux/vfio.h>
#include <uapi/linux/vfio.h>
#include <uapi/linux/iommufd.h>

#include "iommufd_private.h"

static struct iommufd_ioas *get_compat_ioas(struct iommufd_ctx *ictx)
{
	struct iommufd_ioas *ioas = ERR_PTR(-ENODEV);

	xa_lock(&ictx->objects);
	if (!ictx->vfio_ioas || !iommufd_lock_obj(&ictx->vfio_ioas->obj))
		goto out_unlock;
	ioas = ictx->vfio_ioas;
out_unlock:
	xa_unlock(&ictx->objects);
	return ioas;
}

/*
 * Only attaching a group should cause a default creation of the internal ioas,
 * this returns the existing ioas if it has already been assigned somehow
 * FIXME: maybe_unused
 */
static __maybe_unused struct iommufd_ioas *
create_compat_ioas(struct iommufd_ctx *ictx)
{
	struct iommufd_ioas *ioas = NULL;
	struct iommufd_ioas *out_ioas;

	ioas = iommufd_ioas_alloc(ictx);
	if (IS_ERR(ioas))
		return ioas;

	xa_lock(&ictx->objects);
	if (ictx->vfio_ioas && iommufd_lock_obj(&ictx->vfio_ioas->obj))
		out_ioas = ictx->vfio_ioas;
	else
		out_ioas = ioas;
	xa_unlock(&ictx->objects);

	if (out_ioas != ioas) {
		iommufd_object_abort(ictx, &ioas->obj);
		return out_ioas;
	}
	if (!iommufd_lock_obj(&ioas->obj))
		WARN_ON(true);
	iommufd_object_finalize(ictx, &ioas->obj);
	return ioas;
}

int iommufd_vfio_ioas(struct iommufd_ucmd *ucmd)
{
	struct iommu_vfio_ioas *cmd = ucmd->cmd;
	struct iommufd_ioas *ioas;

	if (cmd->__reserved)
		return -EOPNOTSUPP;
	switch (cmd->op) {
	case IOMMU_VFIO_IOAS_GET:
		ioas = get_compat_ioas(ucmd->ictx);
		if (IS_ERR(ioas))
			return PTR_ERR(ioas);
		cmd->ioas_id = ioas->obj.id;
		iommufd_put_object(&ioas->obj);
		return iommufd_ucmd_respond(ucmd, sizeof(*cmd));

	case IOMMU_VFIO_IOAS_SET:
		ioas = iommufd_get_ioas(ucmd, cmd->ioas_id);
		if (IS_ERR(ioas))
			return PTR_ERR(ioas);
		xa_lock(&ucmd->ictx->objects);
		ucmd->ictx->vfio_ioas = ioas;
		xa_unlock(&ucmd->ictx->objects);
		iommufd_put_object(&ioas->obj);
		return 0;

	case IOMMU_VFIO_IOAS_CLEAR:
		xa_lock(&ucmd->ictx->objects);
		ucmd->ictx->vfio_ioas = NULL;
		xa_unlock(&ucmd->ictx->objects);
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

static int iommufd_vfio_map_dma(struct iommufd_ctx *ictx, unsigned int cmd,
				void __user *arg)
{
	u32 supported_flags = VFIO_DMA_MAP_FLAG_READ | VFIO_DMA_MAP_FLAG_WRITE;
	size_t minsz = offsetofend(struct vfio_iommu_type1_dma_map, size);
	struct vfio_iommu_type1_dma_map map;
	int iommu_prot = IOMMU_CACHE;
	struct iommufd_ioas *ioas;
	unsigned long iova;
	int rc;

	if (copy_from_user(&map, arg, minsz))
		return -EFAULT;

	if (map.argsz < minsz || map.flags & ~supported_flags)
		return -EINVAL;

	if (map.flags & VFIO_DMA_MAP_FLAG_READ)
		iommu_prot |= IOMMU_READ;
	if (map.flags & VFIO_DMA_MAP_FLAG_WRITE)
		iommu_prot |= IOMMU_WRITE;

	ioas = get_compat_ioas(ictx);
	if (IS_ERR(ioas))
		return PTR_ERR(ioas);

	iova = map.iova;
	rc = iopt_map_user_pages(&ioas->iopt, &iova,
				 u64_to_user_ptr(map.vaddr), map.size,
				 iommu_prot, 0);
	iommufd_put_object(&ioas->obj);
	return rc;
}

static int iommufd_vfio_unmap_dma(struct iommufd_ctx *ictx, unsigned int cmd,
				  void __user *arg)
{
	size_t minsz = offsetofend(struct vfio_iommu_type1_dma_unmap, size);
	u32 supported_flags = VFIO_DMA_UNMAP_FLAG_ALL;
	struct vfio_iommu_type1_dma_unmap unmap;
	struct iommufd_ioas *ioas;
	int rc;

	if (copy_from_user(&unmap, arg, minsz))
		return -EFAULT;

	if (unmap.argsz < minsz || unmap.flags & ~supported_flags)
		return -EINVAL;

	ioas = get_compat_ioas(ictx);
	if (IS_ERR(ioas))
		return PTR_ERR(ioas);

	if (unmap.flags & VFIO_DMA_UNMAP_FLAG_ALL)
		rc = iopt_unmap_all(&ioas->iopt);
	else
		rc = iopt_unmap_iova(&ioas->iopt, unmap.iova, unmap.size);
	iommufd_put_object(&ioas->obj);
	return rc;
}

static int iommufd_vfio_check_extension(unsigned long type)
{
	switch (type) {
	case VFIO_TYPE1v2_IOMMU:
	case VFIO_UNMAP_ALL:
		return 1;
	/*
	 * FIXME: The type1 iommu allows splitting of maps, which can fail. This is doable but
	 * is a bunch of extra code that is only for supporting this case.
	 */
	case VFIO_TYPE1_IOMMU:
	/*
	 * FIXME: No idea what VFIO_TYPE1_NESTING_IOMMU does as far as the uAPI
	 * is concerned. Seems like it was never completed, it only does
	 * something on ARM, but I can't figure out what or how to use it. Can't
	 * find any user implementation either.
	 */
	case VFIO_TYPE1_NESTING_IOMMU:
	/*
	 * FIXME: Easy to support, but needs rework in the Intel iommu driver
	 * to expose the no snoop squashing to iommufd
	 */
	case VFIO_DMA_CC_IOMMU:
	/*
	 * FIXME: VFIO_DMA_MAP_FLAG_VADDR
	 * https://lore.kernel.org/kvm/1611939252-7240-1-git-send-email-steven.sistare@oracle.com/
	 * Wow, what a wild feature. This should have been implemented by
	 * allowing a iopt_pages to be associated with a memfd. It can then
	 * source mapping requests directly from a memfd without going through a
	 * mm_struct and thus doesn't care that the original qemu exec'd itself.
	 * The idea that userspace can flip a flag and cause kernel users to
	 * block indefinately is unacceptable.
	 *
	 * For VFIO compat we should implement this in a slightly different way,
	 * Creating a access_user that spans the whole area will immediately
	 * stop new faults as they will be handled from the xarray. We can then
	 * reparent the iopt_pages to the new mm_struct and undo the
	 * access_user. No blockage of kernel users required, does require
	 * filling the xarray with pages though.
	 */
	case VFIO_UPDATE_VADDR:
	default:
		return 0;
	}

 /* FIXME: VFIO_DMA_UNMAP_FLAG_GET_DIRTY_BITMAP I think everything with dirty
  * tracking should be in its own ioctl, not muddled in unmap. If we want to
  * atomically unmap and get the dirty bitmap it should be a flag in the dirty
  * tracking ioctl, not here in unmap. Overall dirty tracking needs a careful
  * review along side HW drivers implementing it.
  */
}

static int iommufd_vfio_set_iommu(struct iommufd_ctx *ictx, unsigned long type)
{
	struct iommufd_ioas *ioas = NULL;

	if (type != VFIO_TYPE1v2_IOMMU)
		return -EINVAL;

	/* VFIO fails the set_iommu if there is no group */
	ioas = get_compat_ioas(ictx);
	if (IS_ERR(ioas))
		return PTR_ERR(ioas);
	iommufd_put_object(&ioas->obj);
	return 0;
}

static u64 iommufd_get_pagesizes(struct iommufd_ioas *ioas)
{
	/* FIXME: See vfio_update_pgsize_bitmap(), for compat this should return
	 * the high bits too, and we need to decide if we should report that
	 * iommufd supports less than PAGE_SIZE alignment or stick to strict
	 * compatibility. qemu only cares about the first set bit.
	 */
	return ioas->iopt.iova_alignment;
}

static int iommufd_fill_cap_iova(struct iommufd_ioas *ioas,
				 struct vfio_info_cap_header __user *cur,
				 size_t avail)
{
	struct vfio_iommu_type1_info_cap_iova_range __user *ucap_iovas =
		container_of(cur, struct vfio_iommu_type1_info_cap_iova_range,
			     header);
	struct vfio_iommu_type1_info_cap_iova_range cap_iovas = {
		.header = {
			.id = VFIO_IOMMU_TYPE1_INFO_CAP_IOVA_RANGE,
			.version = 1,
		},
	};
	struct interval_tree_span_iter span;

	for (interval_tree_span_iter_first(
		     &span, &ioas->iopt.reserved_iova_itree, 0, ULONG_MAX);
	     !interval_tree_span_iter_done(&span);
	     interval_tree_span_iter_next(&span)) {
		struct vfio_iova_range range;

		if (!span.is_hole)
			continue;
		range.start = span.start_hole;
		range.end = span.last_hole;
		if (avail >= struct_size(&cap_iovas, iova_ranges,
					 cap_iovas.nr_iovas + 1) &&
		    copy_to_user(&ucap_iovas->iova_ranges[cap_iovas.nr_iovas],
				 &range, sizeof(range)))
			return -EFAULT;
		cap_iovas.nr_iovas++;
	}
	if (avail >= struct_size(&cap_iovas, iova_ranges, cap_iovas.nr_iovas) &&
	    copy_to_user(ucap_iovas, &cap_iovas, sizeof(cap_iovas)))
		return -EFAULT;
	return struct_size(&cap_iovas, iova_ranges, cap_iovas.nr_iovas);
}

static int iommufd_fill_cap_dma_avail(struct iommufd_ioas *ioas,
				      struct vfio_info_cap_header __user *cur,
				      size_t avail)
{
	struct vfio_iommu_type1_info_dma_avail cap_dma = {
		.header = {
			.id = VFIO_IOMMU_TYPE1_INFO_DMA_AVAIL,
			.version = 1,
		},
		/* iommufd has no limit, return the same value as VFIO. */
		.avail = U16_MAX,
	};

	if (avail >= sizeof(cap_dma) &&
	    copy_to_user(cur, &cap_dma, sizeof(cap_dma)))
		return -EFAULT;
	return sizeof(cap_dma);
}

static int iommufd_vfio_iommu_get_info(struct iommufd_ctx *ictx,
				       void __user *arg)
{
	typedef int (*fill_cap_fn)(struct iommufd_ioas *ioas,
				   struct vfio_info_cap_header __user *cur,
				   size_t avail);
	static const fill_cap_fn fill_fns[] = {
		iommufd_fill_cap_iova,
		iommufd_fill_cap_dma_avail,
	};
	size_t minsz = offsetofend(struct vfio_iommu_type1_info, iova_pgsizes);
	struct vfio_info_cap_header __user *last_cap = NULL;
	struct vfio_iommu_type1_info info;
	struct iommufd_ioas *ioas;
	size_t total_cap_size;
	int rc;
	int i;

	if (copy_from_user(&info, arg, minsz))
		return -EFAULT;

	if (info.argsz < minsz)
		return -EINVAL;
	minsz = min_t(size_t, info.argsz, sizeof(info));

	ioas = get_compat_ioas(ictx);
	if (IS_ERR(ioas))
		return PTR_ERR(ioas);

	down_read(&ioas->iopt.iova_rwsem);
	info.flags = VFIO_IOMMU_INFO_PGSIZES;
	info.iova_pgsizes = iommufd_get_pagesizes(ioas);
	info.cap_offset = 0;

	total_cap_size = sizeof(info);
	for (i = 0; i != ARRAY_SIZE(fill_fns); i++) {
		int cap_size;

		if (info.argsz > total_cap_size)
			cap_size = fill_fns[i](ioas, arg + total_cap_size,
					       info.argsz - total_cap_size);
		else
			cap_size = fill_fns[i](ioas, NULL, 0);
		if (cap_size < 0) {
			rc = cap_size;
			goto out_put;
		}
		if (last_cap && info.argsz >= total_cap_size &&
		    put_user(total_cap_size, &last_cap->next)) {
			rc = -EFAULT;
			goto out_put;
		}
		last_cap = arg + total_cap_size;
		total_cap_size += cap_size;
	}

	/*
	 * If the user did not provide enough space then only some caps are
	 * returned and the argsz will be updated to the correct amount to get
	 * all caps.
	 */
	if (info.argsz >= total_cap_size)
		info.cap_offset = sizeof(info);
	info.argsz = total_cap_size;
	info.flags |= VFIO_IOMMU_INFO_CAPS;
	if (copy_to_user(arg, &info, minsz))
		rc = -EFAULT;
	rc = 0;

out_put:
	up_read(&ioas->iopt.iova_rwsem);
	iommufd_put_object(&ioas->obj);
	return rc;
}

/* FIXME TODO:
PowerPC SPAPR only:
#define VFIO_IOMMU_ENABLE	_IO(VFIO_TYPE, VFIO_BASE + 15)
#define VFIO_IOMMU_DISABLE	_IO(VFIO_TYPE, VFIO_BASE + 16)
#define VFIO_IOMMU_SPAPR_TCE_GET_INFO	_IO(VFIO_TYPE, VFIO_BASE + 12)
#define VFIO_IOMMU_SPAPR_REGISTER_MEMORY	_IO(VFIO_TYPE, VFIO_BASE + 17)
#define VFIO_IOMMU_SPAPR_UNREGISTER_MEMORY	_IO(VFIO_TYPE, VFIO_BASE + 18)
#define VFIO_IOMMU_SPAPR_TCE_CREATE	_IO(VFIO_TYPE, VFIO_BASE + 19)
#define VFIO_IOMMU_SPAPR_TCE_REMOVE	_IO(VFIO_TYPE, VFIO_BASE + 20)
*/

int iommufd_vfio_ioctl(struct iommufd_ctx *ictx, unsigned int cmd,
		       unsigned long arg)
{
	void __user *uarg = (void __user *)arg;

	switch (cmd) {
	case VFIO_GET_API_VERSION:
		return VFIO_API_VERSION;
	case VFIO_SET_IOMMU:
		return iommufd_vfio_set_iommu(ictx, arg);
	case VFIO_CHECK_EXTENSION:
		return iommufd_vfio_check_extension(arg);
	case VFIO_IOMMU_GET_INFO:
		return iommufd_vfio_iommu_get_info(ictx, uarg);
	case VFIO_IOMMU_MAP_DMA:
		return iommufd_vfio_map_dma(ictx, cmd, uarg);
	case VFIO_IOMMU_UNMAP_DMA:
		return iommufd_vfio_unmap_dma(ictx, cmd, uarg);
	case VFIO_IOMMU_DIRTY_PAGES:
	default:
		return -ENOIOCTLCMD;
	}
	return -ENOIOCTLCMD;
}

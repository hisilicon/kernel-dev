// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#include <linux/iommu.h>
#include <uapi/linux/iommufd.h>

#include "../iommu-priv.h"
#include "iommufd_private.h"

void iommufd_hw_pagetable_destroy(struct iommufd_object *obj)
{
	struct iommufd_hw_pagetable *hwpt =
		container_of(obj, struct iommufd_hw_pagetable, obj);

	if (!list_empty(&hwpt->hwpt_item)) {
		mutex_lock(&hwpt->ioas->mutex);
		list_del(&hwpt->hwpt_item);
		mutex_unlock(&hwpt->ioas->mutex);

		iopt_table_remove_domain(&hwpt->ioas->iopt, hwpt->domain);
	}

	if (hwpt->domain)
		iommu_domain_free(hwpt->domain);

	if (hwpt->parent)
		refcount_dec(&hwpt->parent->obj.users);
	refcount_dec(&hwpt->ioas->obj.users);
}

int iommufd_hw_pagetable_enforce_cc(struct iommufd_hw_pagetable *hwpt)
{
	if (hwpt->enforce_cache_coherency)
		return 0;

	if (hwpt->domain->ops->enforce_cache_coherency)
		hwpt->enforce_cache_coherency =
			hwpt->domain->ops->enforce_cache_coherency(
				hwpt->domain);
	if (!hwpt->enforce_cache_coherency)
		return -EINVAL;
	return 0;
}

static int iommufd_hw_pagetable_link_ioas(struct iommufd_hw_pagetable *hwpt)
{
	int rc;

	/*
	 * Only a parent hwpt needs to be linked to the IOAS. And a hwpt->parent
	 * must be linked to the IOAS already, when it's being allocated.
	 */
	if (hwpt->parent)
		return 0;

	rc = iopt_table_add_domain(&hwpt->ioas->iopt, hwpt->domain);
	if (rc)
		return rc;
	list_add_tail(&hwpt->hwpt_item, &hwpt->ioas->hwpt_list);
	return 0;
}

/**
 * iommufd_hw_pagetable_alloc() - Get an iommu_domain for a device
 * @ictx: iommufd context
 * @ioas: IOAS to associate the domain with
 * @idev: Device to get an iommu_domain for
 * @parent: Optional parent HWPT to associate with the domain with
 * @user_data: Optional user_data pointer
 * @immediate_attach: True if idev should be attached to the hwpt
 *
 * Allocate a new iommu_domain and return it as a hw_pagetable. The HWPT
 * will be linked to the given ioas and upon return the underlying iommu_domain
 * is fully popoulated.
 */
struct iommufd_hw_pagetable *
iommufd_hw_pagetable_alloc(struct iommufd_ctx *ictx, struct iommufd_ioas *ioas,
			   struct iommufd_device *idev,
			   struct iommufd_hw_pagetable *parent,
			   void *user_data, bool immediate_attach)
{
	const struct iommu_ops *ops = dev_iommu_ops(idev->dev);
	struct iommu_domain *parent_domain = NULL;
	struct iommufd_hw_pagetable *hwpt;
	int rc;

	lockdep_assert_held(&ioas->mutex);

	if (parent && !ops->domain_alloc_user)
		return ERR_PTR(-EOPNOTSUPP);

	hwpt = iommufd_object_alloc(ictx, hwpt, IOMMUFD_OBJ_HW_PAGETABLE);
	if (IS_ERR(hwpt))
		return hwpt;

	INIT_LIST_HEAD(&hwpt->hwpt_item);
	/* Pairs with iommufd_hw_pagetable_destroy() */
	refcount_inc(&ioas->obj.users);
	hwpt->ioas = ioas;
	if (parent) {
		hwpt->parent = parent;
		parent_domain = parent->domain;
		refcount_inc(&parent->obj.users);
	}

	if (ops->domain_alloc_user)
		hwpt->domain = ops->domain_alloc_user(idev->dev,
						      parent_domain, user_data);
	else
		hwpt->domain = iommu_domain_alloc(idev->dev->bus);
	if (!hwpt->domain) {
		rc = -ENOMEM;
		goto out_abort;
	}

	/* It must be either NESTED or UNMANAGED, depending on parent_domain */
	if ((parent_domain && hwpt->domain->type != IOMMU_DOMAIN_NESTED) ||
	    (!parent_domain && hwpt->domain->type != IOMMU_DOMAIN_UNMANAGED))
		goto out_abort;

	/*
	 * Set the coherency mode before we do iopt_table_add_domain() as some
	 * iommus have a per-PTE bit that controls it and need to decide before
	 * doing any maps. It is an iommu driver bug to report
	 * IOMMU_CAP_ENFORCE_CACHE_COHERENCY but fail enforce_cache_coherency on
	 * a new domain.
	 */
	if (idev->enforce_cache_coherency) {
		rc = iommufd_hw_pagetable_enforce_cc(hwpt);
		if (WARN_ON(rc))
			goto out_abort;
	}

	/*
	 * immediate_attach exists only to accommodate iommu drivers that cannot
	 * directly allocate a domain. These drivers do not finish creating the
	 * domain until attach is completed. Thus we must have this call
	 * sequence. Once those drivers are fixed this should be removed.
	 *
	 * Note we hold the igroup->lock here which prevents any other thread
	 * from observing igroup->hwpt until we finish setting it up.
	 */
	if (immediate_attach) {
		rc = iommufd_hw_pagetable_attach(hwpt, idev);
		if (rc)
			goto out_abort;
	}

	rc = iommufd_hw_pagetable_link_ioas(hwpt);
	if (rc)
		goto out_detach;

	return hwpt;

out_detach:
	if (immediate_attach)
		iommufd_hw_pagetable_detach(idev);
out_abort:
	iommufd_object_abort_and_destroy(ictx, &hwpt->obj);
	return ERR_PTR(rc);
}

/*
 * size of page table type specific data, indexed by
 * enum iommu_hwpt_type.
 */
static const size_t iommufd_hwpt_alloc_data_size[] = {
	[IOMMU_HWPT_TYPE_DEFAULT] = 0,
};

/*
 * bitmaps of supported hwpt types of by underlying iommu, indexed
 * by ops->driver_type which is one of enum iommu_hw_info_type.
 */
const u64 iommufd_hwpt_type_bitmaps[] =  {
	[IOMMU_HW_INFO_TYPE_DEFAULT] = BIT_ULL(IOMMU_HWPT_TYPE_DEFAULT),
};

int iommufd_hwpt_alloc(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_alloc *cmd = ucmd->cmd;
	struct iommufd_hw_pagetable *hwpt, *parent = NULL;
	struct iommufd_object *pt_obj;
	struct iommufd_device *idev;
	struct iommufd_ioas *ioas;
	const struct iommu_ops *ops;
	void *data = NULL;
	u32 driver_type, klen;
	int rc;

	if (cmd->flags || cmd->__reserved)
		return -EOPNOTSUPP;

	idev = iommufd_get_device(ucmd, cmd->dev_id);
	if (IS_ERR(idev))
		return PTR_ERR(idev);

	ops = dev_iommu_ops(idev->dev);
	if (!ops) {
		rc = -EOPNOTSUPP;
		goto out_put_idev;
	}

	driver_type = ops->driver_type;

	/* data_type should be a supported type by the driver */
	if (WARN_ON(driver_type >= ARRAY_SIZE(iommufd_hwpt_type_bitmaps)) ||
	    !((1 << cmd->data_type) &
			iommufd_hwpt_type_bitmaps[driver_type])) {
		rc = -EINVAL;
		goto out_put_idev;
	}

	pt_obj = iommufd_get_object(ucmd->ictx, cmd->pt_id, IOMMUFD_OBJ_ANY);
	if (IS_ERR(pt_obj)) {
		rc = -EINVAL;
		goto out_put_idev;
	}

	switch (pt_obj->type) {
	case IOMMUFD_OBJ_IOAS:
		ioas = container_of(pt_obj, struct iommufd_ioas, obj);
		break;
	case IOMMUFD_OBJ_HW_PAGETABLE:
		/* pt_id points HWPT only when data_type is !IOMMU_HWPT_TYPE_DEFAULT */
		if (cmd->data_type == IOMMU_HWPT_TYPE_DEFAULT) {
			rc = -EINVAL;
			goto out_put_pt;
		}

		parent = container_of(pt_obj, struct iommufd_hw_pagetable, obj);
		/*
		 * Cannot allocate user-managed hwpt linking to auto_created
		 * hwpt. If the parent hwpt is already a user-managed hwpt,
		 * don't allocate another user-managed hwpt linking to it.
		 */
		if (parent->auto_domain || parent->parent) {
			rc = -EINVAL;
			goto out_put_pt;
		}
		ioas = parent->ioas;
		break;
	default:
		rc = -EINVAL;
		goto out_put_pt;
	}

	klen = iommufd_hwpt_alloc_data_size[cmd->data_type];
	if (klen) {
		if (!cmd->data_len) {
			rc = -EINVAL;
			goto out_put_pt;
		}

		data = kzalloc(klen, GFP_KERNEL);
		if (!data) {
			rc = -ENOMEM;
			goto out_put_pt;
		}

		rc = copy_struct_from_user(data, klen,
					   u64_to_user_ptr(cmd->data_uptr),
					   cmd->data_len);
		if (rc)
			goto out_free_data;
	}

	mutex_lock(&ioas->mutex);
	hwpt = iommufd_hw_pagetable_alloc(ucmd->ictx, ioas, idev,
					  parent, data, false);
	mutex_unlock(&ioas->mutex);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_free_data;
	}

	cmd->out_hwpt_id = hwpt->obj.id;
	rc = iommufd_ucmd_respond(ucmd, sizeof(*cmd));
	if (rc)
		goto out_hwpt;
	iommufd_object_finalize(ucmd->ictx, &hwpt->obj);
	goto out_free_data;

out_hwpt:
	iommufd_object_abort_and_destroy(ucmd->ictx, &hwpt->obj);
out_free_data:
	kfree(data);
out_put_pt:
	iommufd_put_object(pt_obj);
out_put_idev:
	iommufd_put_object(&idev->obj);
	return rc;
}

/*
 * size of page table type specific invalidate_info, indexed by
 * enum iommu_hwpt_type.
 */
static const size_t iommufd_hwpt_invalidate_info_size[] = {};

int iommufd_hwpt_invalidate(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_invalidate *cmd = ucmd->cmd;
	struct iommufd_hw_pagetable *hwpt;
	u64 user_ptr;
	u32 user_data_len, klen;
	int rc = 0;

	/*
	 * For a user-managed HWPT, type should not be IOMMU_HWPT_TYPE_DEFAULT.
	 * data_len should not exceed the size of iommufd_invalidate_buffer.
	 */
	if (cmd->data_type == IOMMU_HWPT_TYPE_DEFAULT || !cmd->data_len ||
	    cmd->data_type >= ARRAY_SIZE(iommufd_hwpt_invalidate_info_size))
		return -EOPNOTSUPP;

	hwpt = iommufd_get_hwpt(ucmd, cmd->hwpt_id);
	if (IS_ERR(hwpt))
		return PTR_ERR(hwpt);

	/* Do not allow any kernel-managed hw_pagetable */
	if (!hwpt->parent) {
		rc = -EINVAL;
		goto out_put_hwpt;
	}

	klen = iommufd_hwpt_invalidate_info_size[cmd->data_type];
	if (!klen) {
		rc = -EINVAL;
		goto out_put_hwpt;
	}

	/*
	 * Copy the needed fields before reusing the ucmd buffer, this
	 * avoids memory allocation in this path.
	 */
	user_ptr = cmd->data_uptr;
	user_data_len = cmd->data_len;

	rc = copy_struct_from_user(cmd, klen,
				   u64_to_user_ptr(user_ptr), user_data_len);
	if (rc)
		goto out_put_hwpt;

	hwpt->domain->ops->cache_invalidate_user(hwpt->domain, cmd);
out_put_hwpt:
	iommufd_put_object(&hwpt->obj);
	return rc;
}

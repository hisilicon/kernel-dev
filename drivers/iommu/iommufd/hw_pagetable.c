// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#include <linux/iommu.h>
#include <uapi/linux/iommufd.h>

#include "iommufd_private.h"

void iommufd_hw_pagetable_destroy(struct iommufd_object *obj)
{
	struct iommufd_hw_pagetable *hwpt =
		container_of(obj, struct iommufd_hw_pagetable, obj);

	WARN_ON(!list_empty(&hwpt->devices));

	kfree(hwpt->user_data);
	iommu_domain_free(hwpt->domain);
	refcount_dec(&hwpt->ioas->obj.users);
	if (hwpt->parent) {
		refcount_dec(&hwpt->parent->obj.users);
	} else {
		WARN_ON(!refcount_dec_if_one(hwpt->devices_users));
		mutex_destroy(hwpt->devices_lock);
		kfree(hwpt->devices_lock);
	}
}

static struct iommufd_hw_pagetable *
__iommufd_hw_pagetable_alloc(struct iommufd_ctx *ictx, struct device *dev,
			     struct iommufd_ioas *ioas,
			     struct iommufd_hw_pagetable *parent,
			     void *user_data, size_t data_len)
{
	struct iommu_domain *parent_domain = NULL;
	struct iommufd_hw_pagetable *hwpt;
	int rc;

	if (WARN_ON(!parent && !ioas))
		return ERR_PTR(-EINVAL);

	hwpt = iommufd_object_alloc(ictx, hwpt, IOMMUFD_OBJ_HW_PAGETABLE);
	if (IS_ERR(hwpt))
		return hwpt;

	if (parent)
		parent_domain = parent->domain;

	hwpt->domain = iommu_domain_alloc_user(dev, parent_domain,
					       user_data, data_len);
	if (!hwpt->domain) {
		rc = -ENOMEM;
		goto out_abort;
	}

	INIT_LIST_HEAD(&hwpt->devices);
	INIT_LIST_HEAD(&hwpt->hwpt_item);
	/* Pairs with iommufd_hw_pagetable_destroy() */
	refcount_inc(&ioas->obj.users);
	hwpt->ioas = ioas;
	hwpt->parent = parent;
	if (parent) {
		/* Always reuse parent's devices_lock and devices_users... */
		hwpt->devices_users = parent->devices_users;
		hwpt->devices_lock = parent->devices_lock;
		refcount_inc(&parent->obj.users);
	} else {
		/* ...otherwise, allocate a new pair */
		hwpt->devices_lock = kzalloc(sizeof(*hwpt->devices_lock) +
					     sizeof(*hwpt->devices_users),
					     GFP_KERNEL);
		if (!hwpt->devices_lock) {
			rc = -ENOMEM;
			goto out_free_domain;
		}
		hwpt->devices_users = (refcount_t *)&hwpt->devices_lock[1];
		refcount_set(hwpt->devices_users, 1);
		mutex_init(hwpt->devices_lock);
	}
	return hwpt;

out_free_domain:
	iommu_domain_free(hwpt->domain);
out_abort:
	iommufd_object_abort(ictx, &hwpt->obj);
	return ERR_PTR(rc);
}

/**
 * iommufd_hw_pagetable_alloc() - Get an iommu_domain for a device
 * @ictx: iommufd context
 * @ioas: IOAS to associate the domain with
 * @dev: Device to get an iommu_domain for
 *
 * Allocate a new iommu_domain and return it as a hw_pagetable.
 */
struct iommufd_hw_pagetable *
iommufd_hw_pagetable_alloc(struct iommufd_ctx *ictx, struct iommufd_ioas *ioas,
			   struct device *dev)
{
	return __iommufd_hw_pagetable_alloc(ictx, dev, ioas, NULL, NULL, 0);
}

int iommufd_hwpt_alloc(struct iommufd_ucmd *ucmd)
{
	struct iommufd_hw_pagetable *hwpt, *parent = NULL;
	struct iommu_hwpt_alloc *cmd = ucmd->cmd;
	struct iommufd_ctx *ictx = ucmd->ictx;
	struct iommufd_object *pt_obj = NULL;
	struct iommufd_ioas *ioas = NULL;
	struct iommufd_device *idev;
	void *data = NULL;
	int rc;

	if (cmd->__reserved || cmd->flags)
		return -EOPNOTSUPP;

	idev = iommufd_device_get_by_id(ictx, cmd->dev_id);
	if (IS_ERR(idev))
		return PTR_ERR(idev);

	pt_obj = iommufd_get_object(ictx, cmd->pt_id, IOMMUFD_OBJ_ANY);
	if (IS_ERR(pt_obj)) {
		rc = -EINVAL;
		goto out_put_dev;
	}

	switch (pt_obj->type) {
	case IOMMUFD_OBJ_HW_PAGETABLE:
		parent = container_of(pt_obj, struct iommufd_hw_pagetable, obj);
		if (parent->auto_domain) {
			rc = -EINVAL;
			goto out_put_pt;
		}
		ioas = parent->ioas;
		break;
	case IOMMUFD_OBJ_IOAS:
		ioas = container_of(pt_obj, struct iommufd_ioas, obj);
		break;
	default:
		rc = -EINVAL;
		goto out_put_pt;
	}

	if (cmd->data_len && cmd->data_type != IOMMU_DEVICE_DATA_NONE) {
		data = kzalloc(cmd->data_len, GFP_KERNEL);
		if (!data) {
			rc = -ENOMEM;
			goto out_put_pt;
		}

		rc = copy_struct_from_user(data, cmd->data_len,
					   (void __user *)cmd->data_uptr,
					   cmd->data_len);
		if (rc)
			goto out_free_data;
	}

	mutex_lock(&ioas->mutex);
	hwpt = __iommufd_hw_pagetable_alloc(ictx, idev->dev, ioas, parent,
					    data, cmd->data_len);
	mutex_unlock(&ioas->mutex);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_free_data;
	}

	hwpt->user_data = data;
	cmd->out_hwpt_id = hwpt->obj.id;

	rc = iommufd_ucmd_respond(ucmd, sizeof(*cmd));
	if (rc)
		goto out_destroy_hwpt;

	iommufd_object_finalize(ucmd->ictx, &hwpt->obj);
	iommufd_put_object(pt_obj);
	iommufd_put_object(&idev->obj);
	return 0;
out_destroy_hwpt:
	iommufd_object_abort_and_destroy(ucmd->ictx, &hwpt->obj);
out_free_data:
	kfree(data);
out_put_pt:
	iommufd_put_object(pt_obj);
out_put_dev:
	iommufd_put_object(&idev->obj);
	return rc;
}

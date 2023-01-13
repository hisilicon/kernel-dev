// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#include <linux/iommu.h>
#include <uapi/linux/iommufd.h>

#include "../iommu-priv.h"
#include "iommufd_private.h"

static void iommufd_user_managed_hwpt_destroy(struct iommufd_object *obj)
{
	struct iommufd_hw_pagetable *hwpt =
		container_of(obj, struct iommufd_hw_pagetable, obj);

	if (hwpt->domain)
		iommu_domain_free(hwpt->domain);

	refcount_dec(&hwpt->parent->obj.users);
}

static void iommufd_kernel_managed_hwpt_destroy(struct iommufd_object *obj)
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

	refcount_dec(&hwpt->ioas->obj.users);
}

void iommufd_hw_pagetable_destroy(struct iommufd_object *obj)
{
	container_of(obj, struct iommufd_hw_pagetable, obj)->destroy(obj);
}

static void iommufd_user_managed_hwpt_abort(struct iommufd_object *obj)
{
	struct iommufd_hw_pagetable *hwpt =
		container_of(obj, struct iommufd_hw_pagetable, obj);

	/* The parent->mutex must be held until finalize is called. */
	lockdep_assert_held(&hwpt->parent->mutex);

	iommufd_hw_pagetable_destroy(obj);
}

static void iommufd_kernel_managed_hwpt_abort(struct iommufd_object *obj)
{
	struct iommufd_hw_pagetable *hwpt =
		container_of(obj, struct iommufd_hw_pagetable, obj);

	/* The ioas->mutex must be held until finalize is called. */
	lockdep_assert_held(&hwpt->ioas->mutex);

	if (!list_empty(&hwpt->hwpt_item)) {
		list_del_init(&hwpt->hwpt_item);
		iopt_table_remove_domain(&hwpt->ioas->iopt, hwpt->domain);
	}
	iommufd_hw_pagetable_destroy(obj);
}

void iommufd_hw_pagetable_abort(struct iommufd_object *obj)
{
	container_of(obj, struct iommufd_hw_pagetable, obj)->abort(obj);
}

/**
 * iommufd_user_managed_hwpt_alloc() - Get a user-managed hw_pagetable
 * @ictx: iommufd context
 * @pt_obj: Parent object to an HWPT to associate the domain with
 * @idev: Device to get an iommu_domain for
 * @hwpt_type: Requested type of hw_pagetable
 * @user_data: user_data pointer
 *
 * Allocate a new iommu_domain (must be IOMMU_DOMAIN_NESTED) and return it as
 * a user-managed hw_pagetable.
 */
static struct iommufd_hw_pagetable *
iommufd_user_managed_hwpt_alloc(struct iommufd_ctx *ictx,
				struct iommufd_object *pt_obj,
				struct iommufd_device *idev,
				enum iommu_hwpt_type hwpt_type,
				struct iommu_user_data *user_data,
				bool dummy)
{
	struct iommufd_hw_pagetable *parent =
		container_of(pt_obj, struct iommufd_hw_pagetable, obj);
	const struct iommu_ops *ops = dev_iommu_ops(idev->dev);
	struct iommufd_hw_pagetable *hwpt;
	int rc;

	if (!user_data)
		return ERR_PTR(-EINVAL);
	if (parent->auto_domain)
		return ERR_PTR(-EINVAL);
	if (hwpt_type == IOMMU_HWPT_TYPE_DEFAULT)
		return ERR_PTR(-EINVAL);

	if (!ops->domain_alloc_user)
		return ERR_PTR(-EOPNOTSUPP);

	lockdep_assert_held(&parent->mutex);

	hwpt = iommufd_object_alloc(ictx, hwpt, IOMMUFD_OBJ_HW_PAGETABLE);
	if (IS_ERR(hwpt))
		return hwpt;

	refcount_inc(&parent->obj.users);
	hwpt->parent = parent;
	hwpt->user_managed = true;
	hwpt->abort = iommufd_user_managed_hwpt_abort;
	hwpt->destroy = iommufd_user_managed_hwpt_destroy;

	hwpt->domain = ops->domain_alloc_user(idev->dev, hwpt_type,
					      parent->domain, user_data);
	if (IS_ERR(hwpt->domain)) {
		rc = PTR_ERR(hwpt->domain);
		hwpt->domain = NULL;
		goto out_abort;
	}

	if (WARN_ON_ONCE(hwpt->domain->type != IOMMU_DOMAIN_NESTED)) {
		rc = -EINVAL;
		goto out_abort;
	}
	/* Driver is buggy by missing cache_invalidate_user in domain_ops */
	if (WARN_ON_ONCE(!hwpt->domain->ops->cache_invalidate_user)) {
		rc = -EINVAL;
		goto out_abort;
	}
	return hwpt;

out_abort:
	iommufd_object_abort_and_destroy(ictx, &hwpt->obj);
	return ERR_PTR(rc);
}

int iommufd_hw_pagetable_enforce_cc(struct iommufd_hw_pagetable *hwpt)
{
	/* Always enforce cache coherency on a kernel-managed hw_pagetable */
	if (hwpt->user_managed)
		hwpt = hwpt->parent;

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

/**
 * iommufd_hw_pagetable_alloc() - Get a kernel-managed iommu_domain for a device
 * @ictx: iommufd context
 * @pt_obj: An object to an IOAS to associate the domain with
 * @idev: Device to get an iommu_domain for
 * @hwpt_type: Requested type of hw_pagetable
 * @user_data: Optional user_data pointer
 * @immediate_attach: True if idev should be attached to the hwpt
 *
 * Allocate a new iommu_domain (must be IOMMU_DOMAIN_UNMANAGED) and return it as
 * a kernel-managed hw_pagetable. The HWPT will be linked to the given ioas and
 * upon return the underlying iommu_domain is fully popoulated.
 *
 * The caller must hold the ioas->mutex until after
 * iommufd_object_abort_and_destroy() or iommufd_object_finalize() is called on
 * the returned hwpt.
 */
struct iommufd_hw_pagetable *
iommufd_hw_pagetable_alloc(struct iommufd_ctx *ictx,
			   struct iommufd_object *pt_obj,
			   struct iommufd_device *idev,
			   enum iommu_hwpt_type hwpt_type,
			   struct iommu_user_data *user_data,
			   bool immediate_attach)
{
	struct iommufd_ioas *ioas =
		container_of(pt_obj, struct iommufd_ioas, obj);
	const struct iommu_ops *ops = dev_iommu_ops(idev->dev);
	struct iommufd_hw_pagetable *hwpt;
	int rc;

	lockdep_assert_held(&ioas->mutex);

	if (user_data && hwpt_type == IOMMU_HWPT_TYPE_DEFAULT)
		return ERR_PTR(-EINVAL);
	if (user_data && !ops->domain_alloc_user)
		return ERR_PTR(-EOPNOTSUPP);

	hwpt = iommufd_object_alloc(ictx, hwpt, IOMMUFD_OBJ_HW_PAGETABLE);
	if (IS_ERR(hwpt))
		return hwpt;

	mutex_init(&hwpt->mutex);
	INIT_LIST_HEAD(&hwpt->hwpt_item);
	/* Pairs with iommufd_hw_pagetable_destroy() */
	refcount_inc(&ioas->obj.users);
	hwpt->ioas = ioas;
	hwpt->abort = iommufd_kernel_managed_hwpt_abort;
	hwpt->destroy = iommufd_kernel_managed_hwpt_destroy;

	if (ops->domain_alloc_user) {
		hwpt->domain = ops->domain_alloc_user(idev->dev, hwpt_type,
						      NULL, user_data);
		if (IS_ERR(hwpt->domain)) {
			rc = PTR_ERR(hwpt->domain);
			hwpt->domain = NULL;
			goto out_abort;
		}
	} else {
		hwpt->domain = iommu_domain_alloc(idev->dev->bus);
		if (!hwpt->domain) {
			rc = -ENOMEM;
			goto out_abort;
		}
	}

	if (WARN_ON_ONCE(hwpt->domain->type != IOMMU_DOMAIN_UNMANAGED)) {
		rc = -EINVAL;
		goto out_abort;
	}
	/* Driver is buggy by mixing user-managed op in kernel-managed ops */
	if (WARN_ON_ONCE(hwpt->domain->ops->cache_invalidate_user)) {
		rc = -EINVAL;
		goto out_abort;
	}

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
	 */
	if (immediate_attach) {
		rc = iommufd_hw_pagetable_attach(hwpt, idev);
		if (rc)
			goto out_abort;
	}

	rc = iopt_table_add_domain(&hwpt->ioas->iopt, hwpt->domain);
	if (rc)
		goto out_detach;
	list_add_tail(&hwpt->hwpt_item, &hwpt->ioas->hwpt_list);
	return hwpt;

out_detach:
	if (immediate_attach)
		iommufd_hw_pagetable_detach(idev);
out_abort:
	iommufd_object_abort_and_destroy(ictx, &hwpt->obj);
	return ERR_PTR(rc);
}

int iommufd_hwpt_alloc(struct iommufd_ucmd *ucmd)
{
	struct iommufd_hw_pagetable *(*alloc_fn)(
			struct iommufd_ctx *ictx, struct iommufd_object *pt_obj,
			struct iommufd_device *idev, enum iommu_hwpt_type type,
			struct iommu_user_data *user_data, bool flag);
	struct iommufd_hw_pagetable *hwpt, *parent;
	struct iommu_hwpt_alloc *cmd = ucmd->cmd;
	struct iommu_user_data *data = NULL;
	struct iommufd_object *pt_obj;
	struct iommufd_device *idev;
	struct iommufd_ioas *ioas;
	struct mutex *mutex;
	int rc;

	if (cmd->flags || cmd->__reserved)
		return -EOPNOTSUPP;
	if ((!cmd->data_len && cmd->hwpt_type != IOMMU_HWPT_TYPE_DEFAULT) ||
	    (!!cmd->data_uptr != !!cmd->data_len))
		return -EINVAL;

	idev = iommufd_get_device(ucmd, cmd->dev_id);
	if (IS_ERR(idev))
		return PTR_ERR(idev);

	pt_obj = iommufd_get_object(ucmd->ictx, cmd->pt_id, IOMMUFD_OBJ_ANY);
	if (IS_ERR(pt_obj)) {
		rc = -EINVAL;
		goto out_put_idev;
	}

	switch (pt_obj->type) {
	case IOMMUFD_OBJ_IOAS:
		ioas = container_of(pt_obj, struct iommufd_ioas, obj);
		mutex = &ioas->mutex;
		alloc_fn = iommufd_hw_pagetable_alloc;
		break;
	case IOMMUFD_OBJ_HW_PAGETABLE:
		parent = container_of(pt_obj, struct iommufd_hw_pagetable, obj);
		/* No user-managed HWPT on top of an user-managed one */
		if (parent->user_managed) {
			rc = -EINVAL;
			goto out_put_pt;
		}
		mutex = &parent->mutex;
		alloc_fn = iommufd_user_managed_hwpt_alloc;
		break;
	default:
		rc = -EINVAL;
		goto out_put_pt;
	}

	if (cmd->data_len) {
		data = kzalloc(sizeof(*data), GFP_KERNEL);
		if (!data) {
			rc = -ENOMEM;
			goto out_put_pt;
		}
		data->uptr = u64_to_user_ptr(cmd->data_uptr);
		data->len = cmd->data_len;
	}

	mutex_lock(mutex);
	hwpt = alloc_fn(ucmd->ictx, pt_obj, idev, cmd->hwpt_type, data, false);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_unlock;
	}

	cmd->out_hwpt_id = hwpt->obj.id;
	rc = iommufd_ucmd_respond(ucmd, sizeof(*cmd));
	if (rc)
		goto out_hwpt;
	iommufd_object_finalize(ucmd->ictx, &hwpt->obj);
	goto out_unlock;

out_hwpt:
	iommufd_object_abort_and_destroy(ucmd->ictx, &hwpt->obj);
out_unlock:
	mutex_unlock(mutex);
	kfree(data);
out_put_pt:
	iommufd_put_object(pt_obj);
out_put_idev:
	iommufd_put_object(&idev->obj);
	return rc;
}

int iommufd_hwpt_invalidate(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_invalidate *cmd = ucmd->cmd;
	struct iommu_user_data_array data_array = {
		.uptr = u64_to_user_ptr(cmd->reqs_uptr),
		.entry_len = cmd->req_len,
		.entry_num = cmd->req_num,
	};
	struct iommufd_hw_pagetable *hwpt;
	int rc = 0;

	if (!cmd->req_len || !cmd->req_num)
		return -EOPNOTSUPP;

	hwpt = iommufd_get_hwpt(ucmd, cmd->hwpt_id);
	if (IS_ERR(hwpt))
		return PTR_ERR(hwpt);

	if (!hwpt->user_managed) {
		rc = -EINVAL;
		goto out_put_hwpt;
	}

	rc = hwpt->domain->ops->cache_invalidate_user(hwpt->domain, &data_array,
						      &cmd->out_driver_error_code);
	cmd->req_num = data_array.entry_num;
	if (iommufd_ucmd_respond(ucmd, sizeof(*cmd)))
		return -EFAULT;
out_put_hwpt:
	iommufd_put_object(&hwpt->obj);
	return rc;
}

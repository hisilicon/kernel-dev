// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#include <linux/iommu.h>

#include "iommufd_private.h"

void iommufd_hw_pagetable_destroy(struct iommufd_object *obj)
{
	struct iommufd_hw_pagetable *hwpt =
		container_of(obj, struct iommufd_hw_pagetable, obj);
	struct iommufd_ioas *ioas = hwpt->ioas;

	WARN_ON(!list_empty(&hwpt->devices));
	mutex_lock(&ioas->mutex);
	list_del(&hwpt->auto_domains_item);
	mutex_unlock(&ioas->mutex);

	iommu_domain_free(hwpt->domain);
	refcount_dec(&hwpt->ioas->obj.users);
	mutex_destroy(&hwpt->devices_lock);
}

/*
 * When automatically managing the domains we search for a compatible domain in
 * the iopt and if one is found use it, otherwise create a new domain.
 * Automatic domain selection will never pick a manually created domain.
 */
static struct iommufd_hw_pagetable *
iommufd_hw_pagetable_auto_get(struct iommufd_ctx *ictx,
			      struct iommufd_ioas *ioas, struct device *dev)
{
	struct iommufd_hw_pagetable *hwpt;
	int rc;

	/*
	 * There is no differentiation when domains are allocated, so any domain
	 * from the right ops is interchangeable with any other.
	 */
	mutex_lock(&ioas->mutex);
	list_for_each_entry (hwpt, &ioas->auto_domains, auto_domains_item) {
		/*
		 * FIXME: We really need an op from the driver to test if a
		 * device is compatible with a domain. This thing from VFIO
		 * works sometimes.
		 */
		if (hwpt->domain->ops == dev_iommu_ops(dev)->default_domain_ops) {
			if (refcount_inc_not_zero(&hwpt->obj.users)) {
				mutex_unlock(&ioas->mutex);
				return hwpt;
			}
		}
	}

	hwpt = iommufd_object_alloc(ictx, hwpt, IOMMUFD_OBJ_HW_PAGETABLE);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_unlock;
	}

	hwpt->domain = iommu_domain_alloc(dev->bus);
	if (!hwpt->domain) {
		rc = -ENOMEM;
		goto out_abort;
	}

	INIT_LIST_HEAD(&hwpt->devices);
	mutex_init(&hwpt->devices_lock);
	hwpt->ioas = ioas;
	/* The calling driver is a user until iommufd_hw_pagetable_put() */
	refcount_inc(&ioas->obj.users);

	list_add_tail(&hwpt->auto_domains_item, &ioas->auto_domains);
	/*
	 * iommufd_object_finalize() consumes the refcount, get one for the
	 * caller. This pairs with the first put in
	 * iommufd_object_destroy_user()
	 */
	refcount_inc(&hwpt->obj.users);
	iommufd_object_finalize(ictx, &hwpt->obj);

	mutex_unlock(&ioas->mutex);
	return hwpt;

out_abort:
	iommufd_object_abort(ictx, &hwpt->obj);
out_unlock:
	mutex_unlock(&ioas->mutex);
	return ERR_PTR(rc);
}

/**
 * iommufd_hw_pagetable_from_id() - Get an iommu_domain for a device
 * @ictx: iommufd context
 * @pt_id: ID of the IOAS or hw_pagetable object
 * @dev: Device to get an iommu_domain for
 *
 * Turn a general page table ID into an iommu_domain contained in a
 * iommufd_hw_pagetable object. If a hw_pagetable ID is specified then that
 * iommu_domain is used, otherwise a suitable iommu_domain in the IOAS is found
 * for the device, creating one automatically if necessary.
 */
struct iommufd_hw_pagetable *
iommufd_hw_pagetable_from_id(struct iommufd_ctx *ictx, u32 pt_id,
			     struct device *dev)
{
	struct iommufd_object *obj;

	obj = iommufd_get_object(ictx, pt_id, IOMMUFD_OBJ_ANY);
	if (IS_ERR(obj))
		return ERR_CAST(obj);

	switch (obj->type) {
	case IOMMUFD_OBJ_HW_PAGETABLE:
		iommufd_put_object_keep_user(obj);
		return container_of(obj, struct iommufd_hw_pagetable, obj);
	case IOMMUFD_OBJ_IOAS: {
		struct iommufd_ioas *ioas =
			container_of(obj, struct iommufd_ioas, obj);
		struct iommufd_hw_pagetable *hwpt;

		hwpt = iommufd_hw_pagetable_auto_get(ictx, ioas, dev);
		iommufd_put_object(obj);
		return hwpt;
	}
	default:
		iommufd_put_object(obj);
		return ERR_PTR(-EINVAL);
	}
}

void iommufd_hw_pagetable_put(struct iommufd_ctx *ictx,
			      struct iommufd_hw_pagetable *hwpt)
{
	if (list_empty(&hwpt->auto_domains_item)) {
		/* Manually created hw_pagetables just keep going */
		refcount_dec(&hwpt->obj.users);
		return;
	}
	iommufd_object_destroy_user(ictx, &hwpt->obj);
}

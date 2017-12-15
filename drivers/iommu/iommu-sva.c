/*
 * Track processes address spaces bound to devices
 *
 * Copyright (C) 2017 ARM Ltd.
 * Author: Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <linux/iommu.h>

/* TODO: stub for the fault queue. Remove later. */
#define iommu_fault_queue_flush(...)

/**
 * iommu_sva_bind_device - Bind a process address space to a device
 *
 * @dev:	the device
 * @mm:		the mm to bind, caller must hold a reference to it
 * @pasid:	valid address where the PASID will be stored
 * @flags:	bond properties
 *
 * Create a bond between device and task, allowing the device to access the mm
 * using the returned PASID.
 *
 * On success, 0 is returned and @pasid contains a valid ID. Otherwise, an error
 * is returned.
 */
int iommu_sva_bind_device(struct device *dev, struct mm_struct *mm, int *pasid,
			  int flags)
{
	struct iommu_domain *domain;
	domain = iommu_get_domain_for_dev(dev);
	if (WARN_ON(!domain))
		return -EINVAL;

	return -ENOSYS; /* TODO */
}
EXPORT_SYMBOL_GPL(iommu_sva_bind_device);

/**
 * iommu_sva_unbind_device - Remove a bond created with iommu_sva_bind_device.
 *
 * @dev:	the device
 * @pasid:	the pasid returned by bind
 *
 * Returns 0 on success, or an error value
 */
int iommu_sva_unbind_device(struct device *dev, int pasid)
{
	struct iommu_domain *domain;

	domain = iommu_get_domain_for_dev(dev);
	if (WARN_ON(!domain))
		return -EINVAL;

	/*
	 * Caller stopped the device from issuing PASIDs, now make sure they are
	 * out of the fault queue.
	 */
	iommu_fault_queue_flush(dev);

	return -ENOSYS; /* TODO */
}
EXPORT_SYMBOL_GPL(iommu_sva_unbind_device);

/*
 * __iommu_sva_unbind_dev_all - Detach all address spaces attached to this
 * device.
 *
 * When detaching @device from @domain, IOMMU drivers should use this helper.
 */
void __iommu_sva_unbind_dev_all(struct iommu_domain *domain, struct device *dev)
{
	iommu_fault_queue_flush(dev);
}
EXPORT_SYMBOL_GPL(__iommu_sva_unbind_dev_all);

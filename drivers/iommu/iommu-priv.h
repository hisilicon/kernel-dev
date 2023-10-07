/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2023, NVIDIA CORPORATION & AFFILIATES.
 */
#ifndef __LINUX_IOMMU_PRIV_H
#define __LINUX_IOMMU_PRIV_H

#include <linux/iommu.h>

static inline const struct iommu_ops *dev_iommu_ops(struct device *dev)
{
	/*
	 * Assume that valid ops must be installed if iommu_probe_device()
	 * has succeeded. The device ops are essentially for internal use
	 * within the IOMMU subsystem itself, so we should be able to trust
	 * ourselves not to misuse the helper.
	 */
	return dev->iommu->iommu_dev->ops;
}

int iommu_group_replace_domain(struct iommu_group *group,
			       struct iommu_domain *new_domain);

int iommu_device_register_bus(struct iommu_device *iommu,
			      const struct iommu_ops *ops, struct bus_type *bus,
			      struct notifier_block *nb);
void iommu_device_unregister_bus(struct iommu_device *iommu,
				 struct bus_type *bus,
				 struct notifier_block *nb);

#ifdef CONFIG_IOMMU_IOPF
void *iopf_pasid_cookie_set(struct device *dev, ioasid_t pasid, void *cookie);
void *iopf_pasid_cookie_get(struct device *dev, ioasid_t pasid);
#else
static inline void *iopf_pasid_cookie_set(struct device *dev, ioasid_t pasid, void *cookie)
{
	return ERR_PTR(-ENODEV);
}

static inline void *iopf_pasid_cookie_get(struct device *dev, ioasid_t pasid)
{
	return ERR_PTR(-ENODEV);
}
#endif /* CONFIG_IOMMU_IOPF */

#endif /* __LINUX_IOMMU_PRIV_H */

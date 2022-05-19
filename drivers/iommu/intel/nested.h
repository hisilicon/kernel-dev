/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 Intel Corporation
 *
 * Author: Lu Baolu <baolu.lu@linux.intel.com>
 */

#ifndef __INTEL_NESTED_H
#define __INTEL_NESTED_H

#include <uapi/linux/iommufd.h>

struct iommu_domain *intel_nested_domain_alloc(struct iommu_domain *s2_domain,
					       unsigned long s1_ptr,
					       union iommu_stage1_config *cfg);

#endif /* __INTEL_NESTED_H */

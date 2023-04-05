// SPDX-License-Identifier: GPL-2.0
/*
 * nested.c - nested mode translation support
 *
 * Copyright (C) 2023 Intel Corporation
 *
 * Author: Lu Baolu <baolu.lu@linux.intel.com>
 *         Jacob Pan <jacob.jun.pan@linux.intel.com>
 */

#define pr_fmt(fmt)	"DMAR: " fmt

#include <linux/iommu.h>
#include <linux/pci.h>
#include <linux/pci-ats.h>

#include "iommu.h"
#include "pasid.h"

static int intel_nested_attach_dev(struct iommu_domain *domain,
				   struct device *dev)
{
	struct device_domain_info *info = dev_iommu_priv_get(dev);
	struct dmar_domain *dmar_domain = to_dmar_domain(domain);
	struct intel_iommu *iommu = info->iommu;
	unsigned long flags;
	int ret = 0;

	if (info->domain)
		device_block_translation(dev);

	/* Is s2_domain compatible with this IOMMU? */
	ret = prepare_domain_attach_device(&dmar_domain->s2_domain->domain, dev);
	if (ret) {
		dev_err_ratelimited(dev, "s2 domain is not compatible\n");
		return ret;
	}

	ret = domain_attach_iommu(dmar_domain, iommu);
	if (ret) {
		dev_err_ratelimited(dev, "Failed to attach domain to iommu\n");
		return ret;
	}

	ret = intel_pasid_setup_nested(iommu, dev,
				       PASID_RID2PASID, dmar_domain);
	if (ret) {
		domain_detach_iommu(dmar_domain, iommu);
		dev_err_ratelimited(dev, "Failed to setup pasid entry\n");
		return ret;
	}

	info->domain = dmar_domain;
	spin_lock_irqsave(&dmar_domain->lock, flags);
	list_add(&info->link, &dmar_domain->devices);
	spin_unlock_irqrestore(&dmar_domain->lock, flags);
	domain_update_iommu_cap(dmar_domain);

	return 0;
}

static void intel_nested_domain_free(struct iommu_domain *domain)
{
	struct dmar_domain *dmar_domain = to_dmar_domain(domain);
	struct dmar_domain *s2_domain = dmar_domain->s2_domain;
	unsigned long flags;

	spin_lock_irqsave(&s2_domain->lock, flags);
	s2_domain->nested_users--;
	spin_unlock_irqrestore(&s2_domain->lock, flags);
	kfree(dmar_domain);
}

static void intel_nested_invalidate(struct device *dev,
				    struct dmar_domain *domain,
				    struct iommu_hwpt_invalidate_request_intel_vtd *req)
{
	struct device_domain_info *info = dev_iommu_priv_get(dev);
	struct intel_iommu *iommu = info->iommu;

	if (req->addr == 0 && req->nb_pages == -1)
		intel_flush_iotlb_all(&domain->domain);
	else
		iommu_flush_iotlb_psi(iommu, domain,
				      req->addr >> VTD_PAGE_SHIFT,
				      req->nb_pages, 1, 0);
}

static int intel_nested_cache_invalidate_user(struct iommu_domain *domain,
					      void *user_data)
{
	struct iommu_hwpt_invalidate_request_intel_vtd *req = user_data;
	struct iommu_hwpt_invalidate_intel_vtd *inv_info = user_data;
	struct dmar_domain *dmar_domain = to_dmar_domain(domain);
	unsigned int entry_size = inv_info->entry_size;
	u64 uptr = inv_info->inv_data_uptr;
	u64 nr_uptr = inv_info->entry_nr_uptr;
	struct device_domain_info *info;
	uint32_t entry_nr, index;
	unsigned long flags;
	int ret = 0;

	if (WARN_ON(!user_data))
		return 0;

	if (get_user(entry_nr, (uint32_t __user *)u64_to_user_ptr(nr_uptr)))
		return -EFAULT;

	if (!entry_nr)
		return -EINVAL;

	for (index = 0; index < entry_nr; index++) {
		ret = copy_struct_from_user(req, sizeof(*req),
					    u64_to_user_ptr(uptr + index * entry_size),
					    entry_size);
		if (ret) {
			pr_err_ratelimited("Failed to fetch invalidation request\n");
			break;
		}

		if (req->__reserved || (req->flags & (~IOMMU_VTD_QI_FLAGS_LEAF)) ||
		    !IS_ALIGNED(req->addr, VTD_PAGE_SIZE)) {
			ret = -EINVAL;
			break;
		}

		spin_lock_irqsave(&dmar_domain->lock, flags);
		list_for_each_entry(info, &dmar_domain->devices, link)
			intel_nested_invalidate(info->dev, dmar_domain, req);
		spin_unlock_irqrestore(&dmar_domain->lock, flags);
	}

	if (ret && put_user(index, (uint32_t __user *)u64_to_user_ptr(nr_uptr)))
		return -EFAULT;

	return ret;
}

static const struct iommu_domain_ops intel_nested_domain_ops = {
	.attach_dev		= intel_nested_attach_dev,
	.cache_invalidate_user	= intel_nested_cache_invalidate_user,
	.cache_invalidate_user_data_len =
		sizeof(struct iommu_hwpt_invalidate_intel_vtd),
	.free			= intel_nested_domain_free,
	.enforce_cache_coherency = intel_iommu_enforce_cache_coherency,
};

struct iommu_domain *intel_nested_domain_alloc(struct iommu_domain *s2_domain,
					       const void *user_data)
{
	struct dmar_domain *s2_dmar_domain = to_dmar_domain(s2_domain);
	const struct iommu_hwpt_intel_vtd *vtd = user_data;
	struct dmar_domain *domain;
	unsigned long flags;

	domain = kzalloc(sizeof(*domain), GFP_KERNEL_ACCOUNT);
	if (!domain)
		return NULL;

	spin_lock_irqsave(&s2_dmar_domain->lock, flags);
	if (s2_dmar_domain->read_only_mapped) {
		spin_unlock_irqrestore(&s2_dmar_domain->lock, flags);
		kfree(domain);
		return NULL;
	}
	s2_dmar_domain->nested_users++;
	spin_unlock_irqrestore(&s2_dmar_domain->lock, flags);

	domain->use_first_level = true;
	domain->s2_domain = s2_dmar_domain;
	domain->s1_pgtbl = vtd->pgtbl_addr;
	domain->s1_cfg = *vtd;
	domain->domain.ops = &intel_nested_domain_ops;
	domain->domain.type = IOMMU_DOMAIN_NESTED;
	INIT_LIST_HEAD(&domain->devices);
	spin_lock_init(&domain->lock);
	xa_init(&domain->iommu_array);

	return &domain->domain;
}

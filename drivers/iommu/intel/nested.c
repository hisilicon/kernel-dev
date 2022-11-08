// SPDX-License-Identifier: GPL-2.0
/*
 * nested.c - nested mode translation support
 *
 * Copyright (C) 2022 Intel Corporation
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
	struct dmar_domain *s2_domain = dmar_domain->s2_domain;
	struct intel_iommu *iommu = info->iommu;
	unsigned long flags;
	int ret = 0;

	if (info->domain)
		device_block_translation(dev);

	/* Is s2_domain compatible with this IOMMU? */
	ret = prepare_domain_attach_device(&s2_domain->domain, dev);
	if (ret) {
		dev_err_ratelimited(dev, "s2 domain is not compatible\n");
		return ret;
	}

	ret = domain_attach_iommu(s2_domain, iommu);
	if (ret) {
		dev_err_ratelimited(dev, "Failed to attach domain to iommu\n");
		return ret;
	}

	ret = intel_pasid_setup_nested(iommu, dev,
				       PASID_RID2PASID, dmar_domain);
	if (ret) {
		domain_detach_iommu(s2_domain, iommu);
		return ret;
	}

	info->domain = dmar_domain;
	spin_lock_irqsave(&dmar_domain->lock, flags);
	list_add(&info->link, &dmar_domain->devices);
	spin_unlock_irqrestore(&dmar_domain->lock, flags);

	return 0;
}

static void intel_nested_domain_free(struct iommu_domain *domain)
{
	kfree(to_dmar_domain(domain));
}

/*
 * 2D array for converting and sanitizing IOMMU generic TLB granularity to
 * VT-d granularity. Invalidation is typically included in the unmap operation
 * as a result of DMA or VFIO unmap. However, for assigned devices guest
 * owns the first level page tables. Invalidations of translation caches in the
 * guest are trapped and passed down to the host.
 *
 * vIOMMU in the guest will only expose first level page tables, therefore
 * we do not support IOTLB granularity for request without PASID (second level).
 *
 * For example, to find the VT-d granularity encoding for IOTLB
 * type and page selective granularity within PASID:
 * X: indexed by iommu cache type
 * Y: indexed by enum iommu_inv_granularity
 * [IOMMU_VTD_QI_TYPE_NR][IOMMU_VTD_QI_GRAN_NR]
 */
static const int
inv_type_granu_table[IOMMU_VTD_QI_TYPE_NR][IOMMU_VTD_QI_GRAN_NR] = {
	/*
	 * PASID based IOTLB invalidation: PASID selective (per PASID),
	 * page selective (address granularity)
	 */
	{-EINVAL, QI_GRAN_NONG_PASID, QI_GRAN_PSI_PASID},
	/* PASID based dev TLBs */
	{-EINVAL, -EINVAL, QI_DEV_IOTLB_GRAN_PASID_SEL},
	/* PASID cache */
	{-EINVAL, -EINVAL, -EINVAL}
};

static inline int to_vtd_granularity(int type, int granu)
{
	return inv_type_granu_table[type][granu];
}

static inline u64 to_vtd_size(u64 granu_size, u64 nr_granules)
{
	u64 nr_pages = (granu_size * nr_granules) >> VTD_PAGE_SHIFT;

	/* VT-d size is encoded as 2^size of 4K pages, 0 for 4k, 9 for 2MB, etc.
	 * IOMMU cache invalidate API passes granu_size in bytes, and number of
	 * granu size in contiguous memory.
	 */
	return order_base_2(nr_pages);
}

static void intel_nested_invalidate(struct device *dev, u16 did,
				    void *user_data, size_t data_len)
{
	struct iommu_hwpt_invalidate_intel_vtd *inv_info = user_data;
	struct device_domain_info *info = dev_iommu_priv_get(dev);
	struct intel_iommu *iommu = info->iommu;
	u8 devfn = info->devfn;
	u8 bus = info->bus;
	int cache_type;
	u64 size = 0;
	u16 sid;

	if (!user_data || data_len != sizeof(*inv_info))
		return;

	sid = PCI_DEVID(bus, devfn);

	/* Size is only valid in address selective invalidation */
	if (inv_info->granularity == IOMMU_VTD_QI_GRAN_ADDR)
		size = to_vtd_size(inv_info->granule_size,
				   inv_info->nb_granules);

	for_each_set_bit(cache_type,
			 (unsigned long *)&inv_info->cache,
			 IOMMU_VTD_QI_TYPE_NR) {
		int granu = 0;
		u64 pasid = 0;
		u64 addr = 0;

		granu = to_vtd_granularity(cache_type, inv_info->granularity);
		if (granu == -EINVAL) {
			pr_err_ratelimited("Invalid cache type and granu combination %d/%d\n",
					   cache_type, inv_info->granularity);
			break;
		}

		if ((inv_info->granularity == IOMMU_VTD_QI_GRAN_PASID ||
		     inv_info->granularity == IOMMU_VTD_QI_GRAN_ADDR) &&
		    (inv_info->flags & IOMMU_VTD_QI_FLAGS_PASID))
			pasid = inv_info->pasid;

		switch (BIT(cache_type)) {
		case IOMMU_VTD_QI_TYPE_IOTLB:
			/* HW will ignore LSB bits based on address mask */
			if (inv_info->granularity == IOMMU_VTD_QI_GRAN_ADDR &&
			    size &&
			    (inv_info->addr & ((BIT(VTD_PAGE_SHIFT + size)) - 1))) {
				pr_err_ratelimited("User address not aligned, 0x%llx, size order %llu\n",
						   inv_info->addr, size);
			}

			/*
			 * If granu is PASID-selective, address is ignored.
			 * We use npages = -1 to indicate that.
			 */
			qi_flush_piotlb(iommu, did, pasid,
					inv_info->addr,
					(granu == QI_GRAN_NONG_PASID) ? -1 : 1 << size,
					inv_info->flags & IOMMU_VTD_QI_FLAGS_LEAF);

			if (!info->ats_enabled)
				break;
			/*
			 * Always flush device IOTLB if ATS is enabled. vIOMMU
			 * in the guest may assume IOTLB flush is inclusive,
			 * which is more efficient.
			 */
			fallthrough;
		case IOMMU_VTD_QI_TYPE_DEV_IOTLB:
			/*
			 * PASID based device TLB invalidation does not support
			 * IOMMU_VTD_QI_GRAN_PASID granularity but only supports
			 * IOMMU_VTD_QI_GRAN_ADDR.
			 * The equivalent of that is we set the size to be the
			 * entire range of 64 bit. User only provides PASID info
			 * without address info. So we set addr to 0.
			 */
			if (inv_info->granularity == IOMMU_VTD_QI_GRAN_PASID) {
				size = 64 - VTD_PAGE_SHIFT;
				addr = 0;
			} else if (inv_info->granularity == IOMMU_VTD_QI_GRAN_ADDR) {
				addr = inv_info->addr;
			}

			if (info->ats_enabled)
				qi_flush_dev_iotlb_pasid(iommu, sid,
						info->pfsid, pasid,
						info->ats_qdep, addr,
						size);
			else
				pr_warn_ratelimited("Passdown device IOTLB flush w/o ATS!\n");
			break;
		default:
			dev_err_ratelimited(dev, "Unsupported IOMMU invalidation type %d\n",
					    cache_type);
			return;
		}
	}
}

static void intel_nested_iotlb_sync_user(struct iommu_domain *domain,
					 void *user_data, size_t data_len)
{
	struct dmar_domain *dmar_domain = to_dmar_domain(domain);
	struct device_domain_info *info;
	unsigned long flags;
	u16 did;

	spin_lock_irqsave(&dmar_domain->lock, flags);
	list_for_each_entry(info, &dmar_domain->devices, link) {
		did = domain_id_iommu(dmar_domain->s2_domain, info->iommu);
		intel_nested_invalidate(info->dev, did, user_data, data_len);
	}
	spin_unlock_irqrestore(&dmar_domain->lock, flags);
}

static const struct iommu_domain_ops intel_nested_domain_ops = {
	.attach_dev		= intel_nested_attach_dev,
	.iotlb_sync_user	= intel_nested_iotlb_sync_user,
	.free			= intel_nested_domain_free,
	.enforce_cache_coherency = intel_iommu_enforce_cache_coherency,
};

struct iommu_domain *intel_nested_domain_alloc(struct iommu_domain *s2_domain,
					       void *user_data, size_t data_len)
{
	struct iommu_hwpt_intel_vtd *vtd = user_data;
	struct dmar_domain *domain;

	if (!user_data || data_len != sizeof(*vtd))
		return NULL;

	domain = kzalloc(sizeof(*domain), GFP_KERNEL);
	if (!domain)
		return NULL;

	domain->nested = true;
	domain->s2_domain = to_dmar_domain(s2_domain);
	domain->s1_pgtbl = vtd->s1_pgtbl;
	domain->s1_cfg = *vtd;
	domain->domain.ops = &intel_nested_domain_ops;
	INIT_LIST_HEAD(&domain->devices);
	spin_lock_init(&domain->lock);

	return &domain->domain;
}

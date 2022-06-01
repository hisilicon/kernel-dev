// SPDX-License-Identifier: GPL-2.0
/*
 * nested.c - nested mode translation support
 *
 * Copyright (C) 2022 Intel Corporation
 *
 * Author: Lu Baolu <baolu.lu@linux.intel.com>
 */

#define pr_fmt(fmt)	"DMAR: " fmt

#include <linux/intel-iommu.h>
#include <linux/iommu.h>
#include <linux/pci.h>
#include <linux/pci-ats.h>

#include "pasid.h"

struct nested_domain {
	struct dmar_domain *s2_domain;
	unsigned long s1_pgtbl;
	struct iommu_stage1_config_vtd s1_cfg;

	/* Protect the device list. */
	struct mutex mutex;
	struct list_head devices;

	struct iommu_domain domain;
};

#define to_nested_domain(dom) container_of(dom, struct nested_domain, domain)

static int intel_nested_attach_dev_pasid(struct iommu_domain *domain,
					 struct device *dev, ioasid_t pasid)
{
	struct device_domain_info *info = dev_iommu_priv_get(dev);
	struct nested_domain *ndomain = to_nested_domain(domain);
	struct dmar_domain *dmar_domain = ndomain->s2_domain;
	struct intel_iommu *iommu = info->iommu;
	int ret = 0;

	/*
	 * Set up device context entry for PASID if not enabled already, one
	 * strange thing, the intel_iommu_enable_pasid() is supposed to
	 * be done when calling iommu_dev_enable_feature(, SVA). However,
	 * without below intel_iommu_enable_pasid(), error is observed.
	 * "SM: PASID Enable field in Context Entry is clear". @Baolu,
	 * may you have a look.
	 */
	ret = intel_iommu_enable_pasid(iommu, dev);
	if (ret) {
		dev_err_ratelimited(dev, "Failed to enable PASID capability\n");
		return ret;
	}

	spin_lock(&iommu->lock);
	ret = intel_pasid_setup_nested(iommu, dev, pasid,
				       (pgd_t *)(uintptr_t)ndomain->s1_pgtbl,
				       dmar_domain, &ndomain->s1_cfg);
	spin_unlock(&iommu->lock);
	if (ret)
		return ret;

	mutex_lock(&ndomain->mutex);
	list_add(&info->nested, &ndomain->devices);
	mutex_unlock(&ndomain->mutex);

	return ret;
}

static void intel_nested_detach_dev_pasid(struct iommu_domain *domain,
					  struct device *dev, ioasid_t pasid)
{
	struct device_domain_info *info = dev_iommu_priv_get(dev);
	struct nested_domain *ndomain = to_nested_domain(domain);
	struct intel_iommu *iommu = info->iommu;

	spin_lock(&iommu->lock);
	intel_pasid_tear_down_entry(iommu, dev, pasid, false);
	/* Revist: Need to drain the prq when PR is support. */
	spin_unlock(&iommu->lock);

	mutex_lock(&ndomain->mutex);
	list_del(&info->nested);
	mutex_unlock(&ndomain->mutex);
}

static int intel_nested_attach_dev(struct iommu_domain *domain,
				   struct device *dev)
{
	return intel_nested_attach_dev_pasid(domain, dev, PASID_RID2PASID);
}

static void intel_nested_detach_dev(struct iommu_domain *domain,
				    struct device *dev)
{
	intel_nested_detach_dev_pasid(domain, dev, PASID_RID2PASID);
}

static void intel_nested_domain_free(struct iommu_domain *domain)
{
	kfree(to_nested_domain(domain));
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
 * [IOMMU_CACHE_INV_TYPE_IOTLB][IOMMU_INV_GRANU_ADDR]
 */
static const int
inv_type_granu_table[IOMMU_CACHE_INV_TYPE_NR][IOMMU_INV_GRANU_NR] = {
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

static int
intel_nested_invalidate(struct dmar_domain *domain, struct device *dev,
			struct iommu_cache_invalidate_info *inv_info)
{
	struct device_domain_info *info = dev_iommu_priv_get(dev);
	struct intel_iommu *iommu = info->iommu;
	u8 devfn = info->devfn;
	u8 bus = info->bus;
	int cache_type;
	u16 did, sid;
	int ret = 0;
	u64 size = 0;

	did = domain->iommu_did[iommu->seq_id];
	sid = PCI_DEVID(bus, devfn);

	/* Size is only valid in address selective invalidation */
	if (inv_info->granularity == IOMMU_INV_GRANU_ADDR)
		size = to_vtd_size(inv_info->granu.addr_info.granule_size,
				   inv_info->granu.addr_info.nb_granules);

	for_each_set_bit(cache_type,
			 (unsigned long *)&inv_info->cache,
			 IOMMU_CACHE_INV_TYPE_NR) {
		int granu = 0;
		u64 pasid = 0;
		u64 addr = 0;

		granu = to_vtd_granularity(cache_type, inv_info->granularity);
		if (granu == -EINVAL) {
			pr_err_ratelimited("Invalid cache type and granu combination %d/%d\n",
					   cache_type, inv_info->granularity);
			break;
		}

		/*
		 * PASID is stored in different locations based on the
		 * granularity.
		 */
		if (inv_info->granularity == IOMMU_INV_GRANU_PASID &&
		    (inv_info->granu.pasid_info.flags & IOMMU_INV_PASID_FLAGS_PASID))
			pasid = inv_info->granu.pasid_info.pasid;
		else if (inv_info->granularity == IOMMU_INV_GRANU_ADDR &&
			 (inv_info->granu.addr_info.flags & IOMMU_INV_ADDR_FLAGS_PASID))
			pasid = inv_info->granu.addr_info.pasid;

		switch (BIT(cache_type)) {
		case IOMMU_CACHE_INV_TYPE_IOTLB:
			/* HW will ignore LSB bits based on address mask */
			if (inv_info->granularity == IOMMU_INV_GRANU_ADDR &&
			    size &&
			    (inv_info->granu.addr_info.addr & ((BIT(VTD_PAGE_SHIFT + size)) - 1))) {
				pr_err_ratelimited("User address not aligned, 0x%llx, size order %llu\n",
						   inv_info->granu.addr_info.addr, size);
			}

			/*
			 * If granu is PASID-selective, address is ignored.
			 * We use npages = -1 to indicate that.
			 */
			qi_flush_piotlb(iommu, did, pasid,
					inv_info->granu.addr_info.addr,
					(granu == QI_GRAN_NONG_PASID) ? -1 : 1 << size,
					inv_info->granu.addr_info.flags & IOMMU_INV_ADDR_FLAGS_LEAF);

			if (!info->ats_enabled)
				break;
			/*
			 * Always flush device IOTLB if ATS is enabled. vIOMMU
			 * in the guest may assume IOTLB flush is inclusive,
			 * which is more efficient.
			 */
			fallthrough;
		case IOMMU_CACHE_INV_TYPE_DEV_IOTLB:
			/*
			 * PASID based device TLB invalidation does not support
			 * IOMMU_INV_GRANU_PASID granularity but only supports
			 * IOMMU_INV_GRANU_ADDR.
			 * The equivalent of that is we set the size to be the
			 * entire range of 64 bit. User only provides PASID info
			 * without address info. So we set addr to 0.
			 */
			if (inv_info->granularity == IOMMU_INV_GRANU_PASID) {
				size = 64 - VTD_PAGE_SHIFT;
				addr = 0;
			} else if (inv_info->granularity == IOMMU_INV_GRANU_ADDR) {
				addr = inv_info->granu.addr_info.addr;
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
			ret = -EINVAL;
		}
	}

	return ret;
}

static int
intel_nested_cache_invalidate(struct iommu_domain *domain,
			      struct iommu_cache_invalidate_info *inv_info)
{
	struct nested_domain *ndomain = to_nested_domain(domain);
	struct device_domain_info *info;
	int ret = 0;

	mutex_lock(&ndomain->mutex);
	list_for_each_entry(info, &ndomain->devices, nested) {
		ret = intel_nested_invalidate(ndomain->s2_domain,
					      info->dev, inv_info);
		if (ret)
			break;
	}
	mutex_unlock(&ndomain->mutex);

	return ret;
}

static const struct iommu_domain_ops intel_nested_domain_ops = {
	.attach_dev		= intel_nested_attach_dev,
	.detach_dev		= intel_nested_detach_dev,
	.attach_dev_pasid	= intel_nested_attach_dev_pasid,
	.detach_dev_pasid	= intel_nested_detach_dev_pasid,
	.cache_invalidate	= intel_nested_cache_invalidate,
	.free			= intel_nested_domain_free,
};

struct iommu_domain *intel_nested_domain_alloc(struct iommu_domain *s2_domain,
					       unsigned long s1_pgtbl,
					       union iommu_stage1_config *cfg)
{
	struct nested_domain *ndomain;

	ndomain = kzalloc(sizeof(*ndomain), GFP_KERNEL);
	if (!ndomain)
		return NULL;

	ndomain->s2_domain = to_dmar_domain(s2_domain);
	ndomain->s1_pgtbl = s1_pgtbl;
	ndomain->s1_cfg = cfg->vtd;
	ndomain->domain.ops = &intel_nested_domain_ops;
	mutex_init(&ndomain->mutex);
	INIT_LIST_HEAD(&ndomain->devices);

	return &ndomain->domain;
}

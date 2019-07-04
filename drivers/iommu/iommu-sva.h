/* SPDX-License-Identifier: GPL-2.0 */
/*
 * SVA library for IOMMU drivers
 */
#ifndef _IOMMU_SVA_H
#define _IOMMU_SVA_H

#include <linux/iommu.h>
#include <linux/kref.h>
#include <linux/mmu_notifier.h>

struct io_mm_ops {
	/* Allocate a PASID context for an mm */
	void *(*alloc)(struct mm_struct *mm);

	/*
	 * Attach a PASID context to a device. Write the entry into the PASID
	 * table.
	 *
	 * @attach_domain is true when no other device in the IOMMU domain is
	 *   already attached to this context. IOMMU drivers that share the
	 *   PASID tables within a domain don't need to write the PASID entry
	 *   when @attach_domain is false.
	 */
	int (*attach)(struct device *dev, int pasid, void *ctx,
		      bool attach_domain);

	/*
	 * Detach a PASID context from a device. Clear the entry from the PASID
	 * table and invalidate if necessary.
	 *
	 * @detach_domain is true when no other device in the IOMMU domain is
	 *   still attached to this context. IOMMU drivers that share the PASID
	 *   table within a domain don't need to clear the PASID entry when
	 *   @detach_domain is false, only invalidate the caches.
	 */
	void (*detach)(struct device *dev, int pasid, void *ctx,
		       bool detach_domain);

	/* Invalidate a range of addresses. Cannot sleep. */
	void (*invalidate)(struct device *dev, int pasid, void *ctx,
			   unsigned long vaddr, size_t size);

	/* Free a context. Cannot sleep. */
	void (*release)(void *ctx);
};

struct iommu_sva_param {
	u32			min_pasid;
	u32			max_pasid;
	int			nr_bonds;
};

struct iommu_sva *
iommu_sva_bind_generic(struct device *dev, struct mm_struct *mm,
		       const struct io_mm_ops *ops, void *drvdata);
void iommu_sva_unbind_generic(struct iommu_sva *handle);
int iommu_sva_get_pasid_generic(struct iommu_sva *handle);

int iommu_sva_enable(struct device *dev, struct iommu_sva_param *sva_param);
int iommu_sva_disable(struct device *dev);
bool iommu_sva_enabled(struct device *dev);

#endif /* _IOMMU_SVA_H */

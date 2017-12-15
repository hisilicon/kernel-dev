/*
 * Track processes address spaces bound to devices
 *
 * Copyright (C) 2017 ARM Ltd.
 * Author: Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <linux/idr.h>
#include <linux/iommu.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

/* TODO: stub for the fault queue. Remove later. */
#define iommu_fault_queue_flush(...)

/* Link between a domain and a mm */
struct iommu_context {
	struct io_mm		*io_mm;
	struct iommu_domain	*domain;

	struct list_head	mm_head;
	struct list_head	domain_head;

	/* Number of devices that use this context */
	refcount_t		ref;
};

/*
 * Because we're using an IDR, PASIDs are limited to 31 bits (the sign bit is
 * used for returning errors). In practice implementations will use at most 20
 * bits, which is the PCI limit.
 */
static DEFINE_IDR(iommu_pasid_idr);

/*
 * For the moment this is an all-purpose lock. It serializes
 * access/modifications to contexts (mm-domain links), access/modifications
 * to the PASID IDR, and changes to io_mm refcount as well.
 */
static DEFINE_SPINLOCK(iommu_sva_lock);

static struct io_mm *
io_mm_alloc(struct iommu_domain *domain, struct mm_struct *mm)
{
	int err;
	int pasid;
	struct io_mm *io_mm;

	if (WARN_ON(!domain->ops->mm_alloc || !domain->ops->mm_free))
		return ERR_PTR(-ENODEV);

	io_mm = domain->ops->mm_alloc(domain, mm);
	if (IS_ERR(io_mm))
		return io_mm;
	if (!io_mm)
		return ERR_PTR(-ENOMEM);

	io_mm->mm		= mm;
	io_mm->release		= domain->ops->mm_free;
	INIT_LIST_HEAD(&io_mm->domains);

	idr_preload(GFP_KERNEL);
	spin_lock(&iommu_sva_lock);
	pasid = idr_alloc_cyclic(&iommu_pasid_idr, io_mm, domain->min_pasid,
				 domain->max_pasid + 1, GFP_ATOMIC);
	io_mm->pasid = pasid;
	spin_unlock(&iommu_sva_lock);
	idr_preload_end();

	if (pasid < 0) {
		err = pasid;
		goto err_free_mm;
	}

	/* TODO: keep track of mm. For the moment, abort. */
	err = -ENOSYS;
	spin_lock(&iommu_sva_lock);
	idr_remove(&iommu_pasid_idr, io_mm->pasid);
	spin_unlock(&iommu_sva_lock);

err_free_mm:
	domain->ops->mm_free(io_mm);

	return ERR_PTR(err);
}

static void io_mm_free(struct io_mm *io_mm)
{
	void (*release)(struct io_mm *);

	release = io_mm->release;

	release(io_mm);
}

static void io_mm_release(struct kref *kref)
{
	struct io_mm *io_mm;

	io_mm = container_of(kref, struct io_mm, kref);
	WARN_ON(!list_empty(&io_mm->domains));

	idr_remove(&iommu_pasid_idr, io_mm->pasid);

	io_mm_free(io_mm);
}

/*
 * Returns non-zero if a reference to the io_mm was successfully taken.
 * Returns zero if the io_mm is being freed and should not be used.
 */
static int io_mm_get_locked(struct io_mm *io_mm)
{
	if (io_mm)
		return kref_get_unless_zero(&io_mm->kref);

	return 0;
}

static void io_mm_put_locked(struct io_mm *io_mm)
{
	kref_put(&io_mm->kref, io_mm_release);
}

static void io_mm_put(struct io_mm *io_mm)
{
	spin_lock(&iommu_sva_lock);
	io_mm_put_locked(io_mm);
	spin_unlock(&iommu_sva_lock);
}

static int io_mm_attach(struct iommu_domain *domain, struct device *dev,
			struct io_mm *io_mm)
{
	int err;
	int pasid = io_mm->pasid;
	struct iommu_context *context;

	if (WARN_ON(!domain->ops->mm_attach || !domain->ops->mm_detach))
		return -ENODEV;

	if (pasid > domain->max_pasid || pasid < domain->min_pasid)
		return -ENOSPC;

	context = kzalloc(sizeof(*context), GFP_KERNEL);
	if (!context)
		return -ENOMEM;

	context->io_mm		= io_mm;
	context->domain		= domain;
	refcount_set(&context->ref, 1);

	spin_lock(&iommu_sva_lock);
	err = domain->ops->mm_attach(domain, dev, io_mm, true);
	if (err) {
		kfree(context);
		spin_unlock(&iommu_sva_lock);
		return err;
	}

	list_add(&context->mm_head, &io_mm->domains);
	list_add(&context->domain_head, &domain->mm_list);
	spin_unlock(&iommu_sva_lock);

	return 0;
}

static void iommu_context_free(struct iommu_context *context)
{
	if (WARN_ON(!context->io_mm || !context->domain))
		return;

	list_del(&context->mm_head);
	list_del(&context->domain_head);
	io_mm_put_locked(context->io_mm);

	kfree(context);
}

/* Attach an existing context to the device */
static int io_mm_attach_locked(struct iommu_context *context,
			       struct device *dev)
{
	int err = context->domain->ops->mm_attach(context->domain, dev,
						  context->io_mm, false);

	if (!err)
		refcount_inc(&context->ref);
	return err;
}

/* Detach device from context and release it if necessary */
static void io_mm_detach_locked(struct iommu_context *context,
				struct device *dev)
{
	bool last = false;
	struct iommu_domain *domain = context->domain;

	if (refcount_dec_and_test(&context->ref))
		last = true;

	domain->ops->mm_detach(domain, dev, context->io_mm, last);

	if (last)
		iommu_context_free(context);
}

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
	int i, err;
	struct io_mm *io_mm = NULL;
	struct iommu_domain *domain;
	struct iommu_context *cur_context;
	struct iommu_context *context = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (WARN_ON(!domain))
		return -EINVAL;

	/* If an io_mm already exists, use it */
	spin_lock(&iommu_sva_lock);
	idr_for_each_entry(&iommu_pasid_idr, io_mm, i) {
		if (io_mm->mm != mm)
			continue;

		if (!io_mm_get_locked(io_mm)) {
			/* Process is defunct, create a new one */
			io_mm = NULL;
			break;
		}

		*pasid = io_mm->pasid;

		/* Great, is it also bound to this domain? */
		list_for_each_entry(cur_context, &io_mm->domains, mm_head) {
			if (cur_context->domain != domain)
				continue;

			io_mm_put_locked(io_mm);
			context = cur_context;

			/* Splendid, tell the driver and increase the ref */
			err = io_mm_attach_locked(context, dev);
			break;
		}
		break;
	}
	spin_unlock(&iommu_sva_lock);

	if (context)
		return err;

	if (!io_mm) {
		io_mm = io_mm_alloc(domain, mm);
		if (IS_ERR(io_mm))
			return PTR_ERR(io_mm);
		*pasid = io_mm->pasid;
	}

	err = io_mm_attach(domain, dev, io_mm);
	if (err)
		io_mm_put(io_mm);

	return err;
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
	struct io_mm *io_mm;
	struct iommu_domain *domain;
	struct iommu_context *cur_context;
	struct iommu_context *context = NULL;

	domain = iommu_get_domain_for_dev(dev);
	if (WARN_ON(!domain))
		return -EINVAL;

	/*
	 * Caller stopped the device from issuing PASIDs, now make sure they are
	 * out of the fault queue.
	 */
	iommu_fault_queue_flush(dev);

	spin_lock(&iommu_sva_lock);
	io_mm = idr_find(&iommu_pasid_idr, pasid);
	if (!io_mm) {
		spin_unlock(&iommu_sva_lock);
		return -ESRCH;
	}

	list_for_each_entry(cur_context, &io_mm->domains, mm_head) {
		if (cur_context->domain == domain) {
			context = cur_context;
			break;
		}
	}

	if (context)
		io_mm_detach_locked(context, dev);
	spin_unlock(&iommu_sva_lock);

	return context ? 0 : -ESRCH;
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
	struct iommu_context *context, *next;

	iommu_fault_queue_flush(dev);

	spin_lock(&iommu_sva_lock);
	list_for_each_entry_safe(context, next, &domain->mm_list, domain_head)
		io_mm_detach_locked(context, dev);
	spin_unlock(&iommu_sva_lock);

}
EXPORT_SYMBOL_GPL(__iommu_sva_unbind_dev_all);

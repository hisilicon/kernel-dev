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
#include <linux/mmu_notifier.h>
#include <linux/sched/mm.h>
#include <linux/slab.h>
#include <linux/sched/mm.h>
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

static struct mmu_notifier_ops iommu_mmu_notifier;

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
	io_mm->notifier.ops	= &iommu_mmu_notifier;
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

	err = mmu_notifier_register(&io_mm->notifier, mm);
	if (err)
		goto err_free_pasid;

	/*
	 * Now that the MMU notifier is valid, we can allow users to grab this
	 * io_mm by setting a valid refcount. Before that it was accessible in
	 * the IDR but invalid.
	 *
	 * The following barrier ensures that users, who obtain the io_mm with
	 * kref_get_unless_zero, don't read uninitialized fields in the
	 * structure.
	 */
	smp_wmb();
	kref_init(&io_mm->kref);

	return io_mm;

err_free_pasid:
	/*
	 * Even if the io_mm is accessible from the IDR at this point, kref is
	 * 0 so no user could get a reference to it. Free it manually.
	 */
	spin_lock(&iommu_sva_lock);
	idr_remove(&iommu_pasid_idr, io_mm->pasid);
	spin_unlock(&iommu_sva_lock);

err_free_mm:
	domain->ops->mm_free(io_mm);

	return ERR_PTR(err);
}

static void io_mm_free(struct rcu_head *rcu)
{
	struct io_mm *io_mm;

	void (*release)(struct io_mm *);

	io_mm = container_of(rcu, struct io_mm, rcu);
	release = io_mm->release;

	release(io_mm);
}

static void io_mm_release(struct kref *kref)
{
	struct io_mm *io_mm;

	io_mm = container_of(kref, struct io_mm, kref);
	WARN_ON(!list_empty(&io_mm->domains));

	idr_remove(&iommu_pasid_idr, io_mm->pasid);

	/*
	 * If we're being released from mm exit, the notifier callback ->release
	 * has already been called. Otherwise we don't need ->release, the io_mm
	 * isn't attached to anything anymore. Hence no_release.
	 */
	mmu_notifier_unregister_no_release(&io_mm->notifier, io_mm->mm);

	/*
	 * We can't free the structure here, because if mm exits right after
	 * unbind(), then ->release might be attempting to grab the io_mm
	 * concurrently. And in the other case, if ->release is calling
	 * io_mm_release, then __mmu_notifier_release expects to still have a
	 * valid mn when returning. So free the structure when it's safe, after
	 * the RCU grace period elapsed.
	 */
	mmu_notifier_call_srcu(&io_mm->rcu, io_mm_free);
}

/*
 * Returns non-zero if a reference to the io_mm was successfully taken.
 * Returns zero if the io_mm is being freed and should not be used.
 */
static int io_mm_get_locked(struct io_mm *io_mm)
{
	if (io_mm && kref_get_unless_zero(&io_mm->kref)) {
		/*
		 * kref_get_unless_zero doesn't provide ordering for reads. This
		 * barrier pairs with the one in io_mm_alloc.
		 */
		smp_rmb();
		return 1;
	}

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

	if (WARN_ON(!domain->ops->mm_attach || !domain->ops->mm_detach ||
		    !domain->ops->mm_exit || !domain->ops->mm_invalidate))
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

/*
 * Called when the mm exits. Might race with unbind() or any other function
 * dropping the last reference to the mm. As the mmu notifier doesn't hold any
 * reference to the io_mm when calling ->release(), try to take a reference.
 */
static void iommu_notifier_release(struct mmu_notifier *mn, struct mm_struct *mm)
{
	struct iommu_context *context, *next;
	struct io_mm *io_mm = container_of(mn, struct io_mm, notifier);

	/*
	 * If the mm is exiting then domains are still attached to the io_mm.
	 * A few things need to be done before it is safe to release
	 *
	 * 1) Tell the IOMMU driver to stop using this PASID (and forward the
	 *    message to attached device drivers. It can then clear the PASID
	 *    table and invalidate relevant TLBs.
	 *
	 * 2) Drop all references to this io_mm, by freeing the contexts.
	 */
	spin_lock(&iommu_sva_lock);
	if (!io_mm_get_locked(io_mm)) {
		/* Someone's already taking care of it. */
		spin_unlock(&iommu_sva_lock);
		return;
	}

	list_for_each_entry_safe(context, next, &io_mm->domains, mm_head) {
		context->domain->ops->mm_exit(context->domain, io_mm);
		iommu_context_free(context);
	}
	spin_unlock(&iommu_sva_lock);

	iommu_fault_queue_flush(NULL);

	/*
	 * We're now reasonably certain that no more fault is being handled for
	 * this io_mm, since we just flushed them all out of the fault queue.
	 * Release the last reference to free the io_mm.
	 */
	io_mm_put(io_mm);
}

static void iommu_notifier_invalidate_range(struct mmu_notifier *mn, struct mm_struct *mm,
					    unsigned long start, unsigned long end)
{
	struct iommu_context *context;
	struct io_mm *io_mm = container_of(mn, struct io_mm, notifier);

	spin_lock(&iommu_sva_lock);
	list_for_each_entry(context, &io_mm->domains, mm_head) {
		context->domain->ops->mm_invalidate(context->domain, io_mm,
						    start, end - start);
	}
	spin_unlock(&iommu_sva_lock);
}

static int iommu_notifier_clear_flush_young(struct mmu_notifier *mn,
					    struct mm_struct *mm,
					    unsigned long start,
					    unsigned long end)
{
	iommu_notifier_invalidate_range(mn, mm, start, end);
	return 0;
}

static void iommu_notifier_change_pte(struct mmu_notifier *mn, struct mm_struct *mm,
				      unsigned long address, pte_t pte)
{
	iommu_notifier_invalidate_range(mn, mm, address, address + PAGE_SIZE);
}

static struct mmu_notifier_ops iommu_mmu_notifier = {
	.release		= iommu_notifier_release,
	.clear_flush_young	= iommu_notifier_clear_flush_young,
	.change_pte		= iommu_notifier_change_pte,
	.invalidate_range	= iommu_notifier_invalidate_range,
};

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

/**
 * iommu_sva_find - Find mm associated to the given PASID
 *
 * Returns the mm corresponding to this PASID, or NULL if not found. A reference
 * to the mm is taken, and must be released with mmput.
 */
struct mm_struct *iommu_sva_find(int pasid)
{
	struct io_mm *io_mm;
	struct mm_struct *mm = NULL;

	spin_lock(&iommu_sva_lock);
	io_mm = idr_find(&iommu_pasid_idr, pasid);
	if (io_mm && io_mm_get_locked(io_mm)) {
		if (mmget_not_zero(io_mm->mm))
			mm = io_mm->mm;

		io_mm_put_locked(io_mm);
	}
	spin_unlock(&iommu_sva_lock);

	return mm;
}
EXPORT_SYMBOL_GPL(iommu_sva_find);

/**
 * iommu_set_mm_exit_handler() - set a callback for stopping the use of PASID in
 * a device.
 *
 * @dev:	the device
 * @handler:	exit handler
 * @token:	user data, will be passed back to the exit handler
 *
 * Users of the bind/unbind API should call this function to set a
 * device-specific callback telling them when a mm is exiting.
 *
 * After the callback returns, the device must not issue any more transaction
 * with the PASIDs given as argument to the handler. It can be a single PASID
 * value or the special IOMMU_MM_EXIT_ALL.
 *
 * The handler itself should return 0 on success, and an appropriate error code
 * otherwise.
 */
void iommu_set_mm_exit_handler(struct device *dev,
			       iommu_mm_exit_handler_t handler, void *token)
{
	struct iommu_domain *domain = iommu_get_domain_for_dev(dev);

	if (WARN_ON(!domain))
		return;

	domain->mm_exit = handler;
	domain->mm_exit_token = token;
}
EXPORT_SYMBOL_GPL(iommu_set_mm_exit_handler);

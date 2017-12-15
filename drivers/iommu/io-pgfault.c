/*
 * Handle device page faults
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * Copyright (C) 2017 ARM Ltd.
 *
 * Author: Jean-Philippe Brucker <jean-philippe.brucker@arm.com>
 */

#include <linux/iommu.h>
#include <linux/list.h>
#include <linux/sched/mm.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

static struct workqueue_struct *iommu_fault_queue;
static DECLARE_RWSEM(iommu_fault_queue_sem);
static refcount_t iommu_fault_queue_refs = REFCOUNT_INIT(0);
static BLOCKING_NOTIFIER_HEAD(iommu_fault_queue_flush_notifiers);

/* Used to store incomplete fault groups */
static LIST_HEAD(iommu_partial_faults);
static DEFINE_SPINLOCK(iommu_partial_faults_lock);

struct iommu_fault_context {
	struct iommu_domain	*domain;
	struct device		*dev;
	struct iommu_fault	params;
	struct list_head	head;
};

struct iommu_fault_group {
	struct list_head	faults;
	struct work_struct	work;
};

/*
 * iommu_fault_finish - Finish handling a fault
 *
 * Send a response if necessary and pass on the sanitized status code
 */
static int iommu_fault_finish(struct iommu_domain *domain, struct device *dev,
			      struct iommu_fault *fault, int status)
{
	/*
	 * There is no "handling" an unrecoverable fault, so the only valid
	 * return values are 0 or an error.
	 */
	if (!(fault->flags & IOMMU_FAULT_RECOVERABLE))
		return status > 0 ? 0 : status;

	/* Device driver took ownership of the fault and will complete it later */
	if (status == IOMMU_FAULT_STATUS_IGNORE)
		return 0;

	/*
	 * There was an internal error with handling the recoverable fault (e.g.
	 * OOM or no handler). Try to complete the fault if possible.
	 */
	if (status <= 0)
		status = IOMMU_FAULT_STATUS_INVALID;

	if (WARN_ON(!domain->ops->fault_response))
		/*
		 * The IOMMU driver shouldn't have submitted recoverable faults
		 * if it cannot receive a response.
		 */
		return -EINVAL;

	return domain->ops->fault_response(domain, dev, fault, status);
}

static int iommu_fault_handle_single(struct iommu_fault_context *fault)
{
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	int ret = IOMMU_FAULT_STATUS_INVALID;
	unsigned int access_flags = 0;
	unsigned int fault_flags = FAULT_FLAG_REMOTE;
	struct iommu_fault *params = &fault->params;

	if (!(params->flags & IOMMU_FAULT_PASID))
		return ret;

	if ((params->flags & (IOMMU_FAULT_LAST | IOMMU_FAULT_READ |
			      IOMMU_FAULT_WRITE)) == IOMMU_FAULT_LAST)
		/* Special case: PASID Stop Marker doesn't require a response */
		return IOMMU_FAULT_STATUS_IGNORE;

	mm = iommu_sva_find(params->pasid);
	if (!mm)
		return ret;

	down_read(&mm->mmap_sem);

	vma = find_extend_vma(mm, params->address);
	if (!vma)
		/* Unmapped area */
		goto out_put_mm;

	if (params->flags & IOMMU_FAULT_READ)
		access_flags |= VM_READ;

	if (params->flags & IOMMU_FAULT_WRITE) {
		access_flags |= VM_WRITE;
		fault_flags |= FAULT_FLAG_WRITE;
	}

	if (params->flags & IOMMU_FAULT_EXEC) {
		access_flags |= VM_EXEC;
		fault_flags |= FAULT_FLAG_INSTRUCTION;
	}

	if (!(params->flags & IOMMU_FAULT_PRIV))
		fault_flags |= FAULT_FLAG_USER;

	if (access_flags & ~vma->vm_flags)
		/* Access fault */
		goto out_put_mm;

	ret = handle_mm_fault(vma, params->address, fault_flags);
	ret = ret & VM_FAULT_ERROR ? IOMMU_FAULT_STATUS_INVALID :
		IOMMU_FAULT_STATUS_HANDLED;

out_put_mm:
	up_read(&mm->mmap_sem);

	/*
	 * If the process exits while we're handling the fault on its mm, we
	 * can't do mmput. exit_mm would release the MMU notifier, calling
	 * iommu_notifier_release, which has to flush the fault queue that we're
	 * executing on... So mmput_async moves the release of the mm to another
	 * thread, if we're the last user.
	 */
	mmput_async(mm);

	return ret;
}

static void iommu_fault_handle_group(struct work_struct *work)
{
	struct iommu_fault_group *group;
	struct iommu_fault_context *fault, *next;
	int status = IOMMU_FAULT_STATUS_HANDLED;

	group = container_of(work, struct iommu_fault_group, work);

	list_for_each_entry_safe(fault, next, &group->faults, head) {
		struct iommu_fault *params = &fault->params;
		/*
		 * Errors are sticky: don't handle subsequent faults in the
		 * group if there is an error.
		 */
		if (status == IOMMU_FAULT_STATUS_HANDLED)
			status = iommu_fault_handle_single(fault);

		if (params->flags & IOMMU_FAULT_LAST ||
		    !(params->flags & IOMMU_FAULT_GROUP)) {
			iommu_fault_finish(fault->domain, fault->dev,
					   &fault->params, status);
		}

		kfree(fault);
	}

	kfree(group);
}

static int iommu_queue_fault(struct iommu_domain *domain, struct device *dev,
			     struct iommu_fault *params)
{
	struct iommu_fault_group *group;
	struct iommu_fault_context *fault = kzalloc(sizeof(*fault), GFP_KERNEL);

	/*
	 * FIXME There is a race here, with queue_register. The last IOMMU
	 * driver has to ensure no fault is reported anymore before
	 * unregistering, so that doesn't matter. But you could have an IOMMU
	 * device that didn't register to the fault queue and is still reporting
	 * faults while the last queue user disappears. It really shouldn't get
	 * here, but it currently does if there is a blocking handler.
	 */
	if (!iommu_fault_queue)
		return -ENOSYS;

	if (!fault)
		return -ENOMEM;

	fault->dev = dev;
	fault->domain = domain;
	fault->params = *params;

	if ((params->flags & IOMMU_FAULT_LAST) || !(params->flags & IOMMU_FAULT_GROUP)) {
		group = kzalloc(sizeof(*group), GFP_KERNEL);
		if (!group) {
			kfree(fault);
			return -ENOMEM;
		}

		INIT_LIST_HEAD(&group->faults);
		list_add(&fault->head, &group->faults);
		INIT_WORK(&group->work, iommu_fault_handle_group);
	} else {
		/* Non-last request of a group. Postpone until the last one */
		spin_lock(&iommu_partial_faults_lock);
		list_add(&fault->head, &iommu_partial_faults);
		spin_unlock(&iommu_partial_faults_lock);

		return IOMMU_FAULT_STATUS_IGNORE;
	}

	if (params->flags & IOMMU_FAULT_GROUP) {
		struct iommu_fault_context *cur, *next;

		/* See if we have pending faults for this group */
		spin_lock(&iommu_partial_faults_lock);
		list_for_each_entry_safe(cur, next, &iommu_partial_faults, head) {
			if (cur->params.id == params->id && cur->dev == dev) {
				list_del(&cur->head);
				/* Insert *before* the last fault */
				list_add(&cur->head, &group->faults);
			}
		}
		spin_unlock(&iommu_partial_faults_lock);
	}

	queue_work(iommu_fault_queue, &group->work);

	/* Postpone the fault completion */
	return IOMMU_FAULT_STATUS_IGNORE;
}

/**
 * handle_iommu_fault - Handle fault in device driver or mm
 *
 * If the device driver expressed interest in handling fault, report it throught
 * the domain handler. If the fault is recoverable, try to page in the address.
 */
int handle_iommu_fault(struct iommu_domain *domain, struct device *dev,
		       struct iommu_fault *fault)
{
	int ret = -ENOSYS;

	/*
	 * if upper layers showed interest and installed a fault handler,
	 * invoke it.
	 */
	if (domain->ext_handler) {
		ret = domain->ext_handler(domain, dev, fault,
					  domain->handler_token);

		if (ret != IOMMU_FAULT_STATUS_NONE)
			return iommu_fault_finish(domain, dev, fault, ret);
	} else if (domain->handler && !(fault->flags &
		   (IOMMU_FAULT_RECOVERABLE | IOMMU_FAULT_PASID))) {
		/* Fall back to the old method if possible */
		ret = domain->handler(domain, dev, fault->address,
				      fault->flags, domain->handler_token);
		if (ret)
			return ret;
	}

	/* If the handler is blocking, handle fault in the workqueue */
	if (fault->flags & IOMMU_FAULT_RECOVERABLE)
		ret = iommu_queue_fault(domain, dev, fault);

	return iommu_fault_finish(domain, dev, fault, ret);
}
EXPORT_SYMBOL_GPL(handle_iommu_fault);

/**
 * iommu_fault_response - Complete a recoverable fault
 * @domain: iommu domain passed to the handler
 * @dev: device passed to the handler
 * @fault: fault passed to the handler
 * @status: action to perform
 *
 * An atomic handler that took ownership of the fault (by returning
 * IOMMU_FAULT_STATUS_IGNORE) must complete the fault by calling this function.
 */
int iommu_fault_response(struct iommu_domain *domain, struct device *dev,
			 struct iommu_fault *fault, enum iommu_fault_status status)
{
	/* No response is need for unrecoverable faults... */
	if (!(fault->flags & IOMMU_FAULT_RECOVERABLE))
		return -EINVAL;

	/* Ignore is certainly the wrong thing to do at this point */
	if (WARN_ON(status == IOMMU_FAULT_STATUS_IGNORE ||
		    status == IOMMU_FAULT_STATUS_NONE))
		status = IOMMU_FAULT_STATUS_INVALID;

	return iommu_fault_finish(domain, dev, fault, status);
}
EXPORT_SYMBOL_GPL(iommu_fault_response);

/**
 * iommu_fault_queue_register - register an IOMMU driver to the global fault
 * queue
 *
 * @flush_notifier: a notifier block that is called before the fault queue is
 * flushed. The IOMMU driver should commit all faults that are pending in its
 * low-level queues at the time of the call, into the fault queue. The notifier
 * takes a device pointer as argument, hinting what endpoint is causing the
 * flush. When the device is NULL, all faults should be committed.
 */
int iommu_fault_queue_register(struct notifier_block *flush_notifier)
{
	/*
	 * The WQ is unordered because the low-level handler enqueues faults by
	 * group. PRI requests within a group have to be ordered, but once
	 * that's dealt with, the high-level function can handle groups out of
	 * order.
	 */
	down_write(&iommu_fault_queue_sem);
	if (!iommu_fault_queue) {
		iommu_fault_queue = alloc_workqueue("iommu_fault_queue",
						    WQ_UNBOUND, 0);
		if (iommu_fault_queue)
			refcount_set(&iommu_fault_queue_refs, 1);
	} else {
		refcount_inc(&iommu_fault_queue_refs);
	}
	up_write(&iommu_fault_queue_sem);

	if (!iommu_fault_queue)
		return -ENOMEM;

	if (flush_notifier)
		blocking_notifier_chain_register(&iommu_fault_queue_flush_notifiers,
						 flush_notifier);

	return 0;
}
EXPORT_SYMBOL_GPL(iommu_fault_queue_register);

/**
 * iommu_fault_queue_flush - Ensure that all queued faults have been processed.
 * @dev: the endpoint whose faults need to be flushed. If NULL, flush all
 *       pending faults.
 *
 * Users must call this function when releasing a PASID, to ensure that all
 * pending faults affecting this PASID have been handled, and won't affect the
 * address space of a subsequent process that reuses this PASID.
 */
void iommu_fault_queue_flush(struct device *dev)
{
	blocking_notifier_call_chain(&iommu_fault_queue_flush_notifiers, 0, dev);

	down_read(&iommu_fault_queue_sem);
	/*
	 * Don't flush the partial faults list. All PRGs with the PASID are
	 * complete and have been submitted to the queue.
	 */
	if (iommu_fault_queue)
		flush_workqueue(iommu_fault_queue);
	up_read(&iommu_fault_queue_sem);
}
EXPORT_SYMBOL_GPL(iommu_fault_queue_flush);

/**
 * iommu_fault_queue_unregister - Unregister an IOMMU driver from the global
 * fault queue.
 *
 * @flush_notifier: same parameter as iommu_fault_queue_register
 */
void iommu_fault_queue_unregister(struct notifier_block *flush_notifier)
{
	down_write(&iommu_fault_queue_sem);
	if (refcount_dec_and_test(&iommu_fault_queue_refs)) {
		destroy_workqueue(iommu_fault_queue);
		iommu_fault_queue = NULL;
	}
	up_write(&iommu_fault_queue_sem);

	if (flush_notifier)
		blocking_notifier_chain_unregister(&iommu_fault_queue_flush_notifiers,
						   flush_notifier);
}
EXPORT_SYMBOL_GPL(iommu_fault_queue_unregister);

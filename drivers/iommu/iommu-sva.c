// SPDX-License-Identifier: GPL-2.0
/*
 * Helpers for IOMMU drivers implementing SVA
 */
#include <linux/mutex.h>
#include <linux/iommu.h>
#include <linux/slab.h>
#include <linux/sched/mm.h>

#include "iommu-sva.h"

static DEFINE_MUTEX(iommu_sva_lock);
static DECLARE_IOASID_SET(iommu_sva_pasid);
static DEFINE_XARRAY_ALLOC(iommu_sva_ioas_array);

struct iommu_sva_ioas {
	struct mm_struct *mm;
	ioasid_t pasid;

	/* Counter of domains attached to this ioas. */
	refcount_t users;

	/* All bindings are linked here. */
	struct list_head bonds;
};

struct mm_struct *iommu_sva_domain_mm(struct iommu_domain *domain)
{
	return domain->sva_ioas->mm;
}

/**
 * iommu_sva_alloc_pasid - Allocate a PASID for the mm
 * @mm: the mm
 * @min: minimum PASID value (inclusive)
 * @max: maximum PASID value (inclusive)
 *
 * Try to allocate a PASID for this mm, or take a reference to the existing one
 * provided it fits within the [@min, @max] range. On success the PASID is
 * available in mm->pasid and will be available for the lifetime of the mm.
 *
 * Returns 0 on success and < 0 on error.
 */
static int iommu_sva_alloc_pasid(struct mm_struct *mm,
				 ioasid_t min, ioasid_t max)
{
	int ret = 0;
	ioasid_t pasid;

	if (min == INVALID_IOASID || max == INVALID_IOASID ||
	    min == 0 || max < min)
		return -EINVAL;

	mutex_lock(&iommu_sva_lock);
	/* Is a PASID already associated with this mm? */
	if (pasid_valid(mm->pasid)) {
		if (mm->pasid < min || mm->pasid >= max)
			ret = -EOVERFLOW;
		goto out;
	}

	pasid = ioasid_alloc(&iommu_sva_pasid, min, max, mm);
	if (!pasid_valid(pasid))
		ret = -ENOMEM;
	else
		mm_pasid_set(mm, pasid);
out:
	mutex_unlock(&iommu_sva_lock);
	return ret;
}

/*
 * Get or put an ioas for a shared memory.
 */
static struct iommu_sva_ioas *iommu_sva_ioas_get(struct mm_struct *mm,
						 ioasid_t pasid)
{
	struct iommu_sva_ioas *ioas;
	int ret;

	ioas = xa_load(&iommu_sva_ioas_array, pasid);
	if (ioas) {
		if (WARN_ON(ioas->mm != mm))
			return ERR_PTR(-EINVAL);
		refcount_inc(&ioas->users);
		return ioas;
	}

	ioas = kzalloc(sizeof(*ioas), GFP_KERNEL);
	if (!ioas)
		return ERR_PTR(-ENOMEM);

	ioas->mm = mm;
	ioas->pasid = pasid;
	refcount_set(&ioas->users, 1);
	INIT_LIST_HEAD(&ioas->bonds);
	ret = xa_err(xa_store(&iommu_sva_ioas_array, pasid, ioas, GFP_KERNEL));
	if (ret) {
		kfree(ioas);
		return ERR_PTR(ret);
	}

	mmgrab(mm);

	return ioas;
}

static void iommu_sva_ioas_put(struct iommu_sva_ioas *ioas)
{
	if (refcount_dec_and_test(&ioas->users)) {
		WARN_ON(!list_empty(&ioas->bonds));
		xa_erase(&iommu_sva_ioas_array, ioas->pasid);
		mmdrop(ioas->mm);
		kfree(ioas);
	}
}

/*
 * I/O page fault handler for SVA
 *
 * Copied from io-pgfault.c with mmget_not_zero() added before
 * mmap_read_lock().
 */
static enum iommu_page_response_code
iommu_sva_handle_iopf(struct iommu_fault *fault, struct device *dev, void *data)
{
	vm_fault_t ret;
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	unsigned int access_flags = 0;
	struct iommu_domain *domain = data;
	unsigned int fault_flags = FAULT_FLAG_REMOTE;
	struct iommu_fault_page_request *prm = &fault->prm;
	enum iommu_page_response_code status = IOMMU_PAGE_RESP_INVALID;

	if (!(prm->flags & IOMMU_FAULT_PAGE_REQUEST_PASID_VALID))
		return status;

	mm = iommu_sva_domain_mm(domain);
	if (IS_ERR_OR_NULL(mm) || !mmget_not_zero(mm))
		return status;

	mmap_read_lock(mm);

	vma = find_extend_vma(mm, prm->addr);
	if (!vma)
		/* Unmapped area */
		goto out_put_mm;

	if (prm->perm & IOMMU_FAULT_PERM_READ)
		access_flags |= VM_READ;

	if (prm->perm & IOMMU_FAULT_PERM_WRITE) {
		access_flags |= VM_WRITE;
		fault_flags |= FAULT_FLAG_WRITE;
	}

	if (prm->perm & IOMMU_FAULT_PERM_EXEC) {
		access_flags |= VM_EXEC;
		fault_flags |= FAULT_FLAG_INSTRUCTION;
	}

	if (!(prm->perm & IOMMU_FAULT_PERM_PRIV))
		fault_flags |= FAULT_FLAG_USER;

	if (access_flags & ~vma->vm_flags)
		/* Access fault */
		goto out_put_mm;

	ret = handle_mm_fault(vma, prm->addr, fault_flags, NULL);
	status = ret & VM_FAULT_ERROR ? IOMMU_PAGE_RESP_INVALID :
		IOMMU_PAGE_RESP_SUCCESS;

out_put_mm:
	mmap_read_unlock(mm);
	mmput(mm);

	return status;
}

/*
 * IOMMU SVA driver-oriented interfaces
 */
static struct iommu_domain *
iommu_sva_alloc_domain(struct device *dev, struct iommu_sva_ioas *ioas)
{
	struct bus_type *bus = dev->bus;
	struct iommu_domain *domain;

	if (!bus || !bus->iommu_ops)
		return NULL;

	domain = bus->iommu_ops->domain_alloc(IOMMU_DOMAIN_SVA);
	if (!domain)
		return NULL;

	/* The caller must hold a reference to ioas. */
	domain->sva_ioas = ioas;
	domain->type = IOMMU_DOMAIN_SVA;
	domain->iopf_handler = iommu_sva_handle_iopf;
	domain->fault_data = domain;

	return domain;
}

/**
 * iommu_sva_bind_device() - Bind a process address space to a device
 * @dev: the device
 * @mm: the mm to bind, caller must hold a reference to it
 * @drvdata: opaque data pointer to pass to bind callback
 *
 * Create a bond between device and address space, allowing the device to access
 * the mm using the returned PASID. If a bond already exists between @device and
 * @mm, it is returned and an additional reference is taken. Caller must call
 * iommu_sva_unbind_device() to release each reference.
 *
 * iommu_dev_enable_feature(dev, IOMMU_DEV_FEAT_SVA) must be called first, to
 * initialize the required SVA features.
 *
 * On error, returns an ERR_PTR value.
 */
struct iommu_sva *
iommu_sva_bind_device(struct device *dev, struct mm_struct *mm, void *drvdata)
{
	int ret = -EINVAL;
	struct iommu_sva *handle;
	struct iommu_domain *domain;
	struct iommu_sva_ioas *ioas;

	/*
	 * TODO: Remove the drvdata parameter after kernel PASID support is
	 * enabled for the idxd driver.
	 */
	if (drvdata)
		return ERR_PTR(-EOPNOTSUPP);

	/* Allocate mm->pasid if necessary. */
	ret = iommu_sva_alloc_pasid(mm, 1, (1U << dev->iommu->pasid_bits) - 1);
	if (ret)
		return ERR_PTR(ret);

	mutex_lock(&iommu_sva_lock);
	ioas = iommu_sva_ioas_get(mm, mm->pasid);
	if (IS_ERR(ioas)) {
		ret = PTR_ERR(ioas);
		goto out_unlock;
	}

	/* Search for an existing bond. */
	list_for_each_entry(handle, &ioas->bonds, node) {
		if (handle->dev == dev) {
			refcount_inc(&handle->users);
			/* No new bond, drop the counter. */
			iommu_sva_ioas_put(ioas);
			goto out_success;
		}
	}

	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if (!handle) {
		ret = -ENOMEM;
		goto out_put_ioas;
	}

	/* The reference to ioas will be kept until domain free. */
	domain = iommu_sva_alloc_domain(dev, ioas);
	if (!domain) {
		ret = -ENODEV;
		goto out_free_handle;
	}

	ret = iommu_attach_device_pasid(domain, dev, mm->pasid);
	if (ret)
		goto out_free_domain;

	handle->dev = dev;
	handle->domain = domain;
	handle->sva_ioas = ioas;
	refcount_set(&handle->users, 1);
	list_add_tail(&handle->node, &ioas->bonds);

out_success:
	mutex_unlock(&iommu_sva_lock);
	return handle;

out_free_domain:
	iommu_domain_free(domain);
out_free_handle:
	kfree(handle);
out_put_ioas:
	iommu_sva_ioas_put(ioas);
out_unlock:
	mutex_unlock(&iommu_sva_lock);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(iommu_sva_bind_device);

/**
 * iommu_sva_unbind_device() - Remove a bond created with iommu_sva_bind_device
 * @handle: the handle returned by iommu_sva_bind_device()
 *
 * Put reference to a bond between device and address space. The device should
 * not be issuing any more transaction for this PASID. All outstanding page
 * requests for this PASID must have been flushed to the IOMMU.
 */
void iommu_sva_unbind_device(struct iommu_sva *handle)
{
	struct device *dev = handle->dev;
	struct iommu_domain *domain = handle->domain;
	struct iommu_sva_ioas *ioas = handle->sva_ioas;

	mutex_lock(&iommu_sva_lock);
	if (refcount_dec_and_test(&handle->users)) {
		list_del(&handle->node);
		iommu_detach_device_pasid(domain, dev, ioas->pasid);
		iommu_domain_free(domain);
		iommu_sva_ioas_put(ioas);
		kfree(handle);
	}
	mutex_unlock(&iommu_sva_lock);
}
EXPORT_SYMBOL_GPL(iommu_sva_unbind_device);

u32 iommu_sva_get_pasid(struct iommu_sva *handle)
{
	return handle->sva_ioas->pasid;
}
EXPORT_SYMBOL_GPL(iommu_sva_get_pasid);

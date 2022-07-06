// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#include <linux/iommufd.h>
#include <linux/slab.h>
#include <linux/iommu.h>
#include <linux/file.h>
#include <linux/pci.h>
#include <linux/irqdomain.h>
#include <linux/dma-iommu.h>

#include "iommufd_private.h"

void iommufd_device_destroy(struct iommufd_object *obj)
{
	struct iommufd_device *idev =
		container_of(obj, struct iommufd_device, obj);

	WARN_ON(!xa_empty(&idev->hwpts));
	iommu_group_release_dma_owner(idev->group);
	iommu_group_put(idev->group);
	fput(idev->ictx->filp);
}

/**
 * iommufd_bind_pci_device - Bind a physical device to an iommu fd
 * @fd: iommufd file descriptor.
 * @pdev: Pointer to a physical PCI device struct
 * @id: Output ID number to return to userspace for this device
 *
 * A successful bind establishes an ownership over the device and returns
 * struct iommufd_device pointer, otherwise returns error pointer.
 *
 * A driver using this API must set driver_managed_dma and must not touch
 * the device until this routine succeeds and establishes ownership.
 *
 * Binding a PCI device places the entire RID under iommufd control.
 *
 * The caller must undo this with iommufd_unbind_device()
 */
struct iommufd_device *iommufd_bind_pci_device(int fd, struct pci_dev *pdev,
					       unsigned int flags, u32 *id)
{
	struct iommufd_device *idev;
	struct iommufd_ctx *ictx;
	struct iommu_group *group;
	int rc;

	ictx = iommufd_fget(fd);
	if (!ictx)
		return ERR_PTR(-EINVAL);

	group = iommu_group_get(&pdev->dev);
	if (!group) {
		rc = -ENODEV;
		goto out_file_put;
	}

	/*
	 * FIXME: Use a device-centric iommu api and this won't work with
	 * multi-device groups
	 */
	if ((!flags & IOMMUFD_BIND_FLAGS_BYPASS_DMA_OWNERSHIP)) {
		rc = iommu_group_claim_dma_owner(group, ictx->filp);
		if (rc)
			goto out_group_put;
	}

	idev = iommufd_object_alloc(ictx, idev, IOMMUFD_OBJ_DEVICE);
	if (IS_ERR(idev)) {
		rc = PTR_ERR(idev);
		goto out_release_owner;
	}
	idev->ictx = ictx;
	idev->dev = &pdev->dev;
	xa_init_flags(&idev->hwpts, XA_FLAGS_ALLOC1);
	init_rwsem(&idev->hwpts_rwsem);
	/* The calling driver is a user until iommufd_unbind_device() */
	refcount_inc(&idev->obj.users);
	/* group refcount moves into iommufd_device */
	idev->group = group;

	/*
	 * If the caller fails after this success it must call
	 * iommufd_unbind_device() which is safe since we hold this refcount.
	 * This also means the device is a leaf in the graph and no other object
	 * can take a reference on it.
	 */
	iommufd_object_finalize(ictx, &idev->obj);
	*id = idev->obj.id;
	return idev;

out_release_owner:
	if ((!flags & IOMMUFD_BIND_FLAGS_BYPASS_DMA_OWNERSHIP))
		iommu_group_release_dma_owner(group);
out_group_put:
	iommu_group_put(group);
out_file_put:
	fput(ictx->filp);
	return ERR_PTR(rc);
}
EXPORT_SYMBOL_GPL(iommufd_bind_pci_device);

void iommufd_unbind_device(struct iommufd_device *idev)
{
	bool was_destroyed;

	was_destroyed = iommufd_object_destroy_user(idev->ictx, &idev->obj);
	WARN_ON(!was_destroyed);
}
EXPORT_SYMBOL_GPL(iommufd_unbind_device);

int iommufd_device_get_info(struct iommufd_ucmd *ucmd)
{
	struct iommu_device_info *cmd = ucmd->cmd;
	struct iommufd_object *obj;
	struct iommufd_device *idev;
	struct iommu_hw_info hw_info;
	u32 user_length;
	int rc;

	if (cmd->flags || cmd->reserved || cmd->dev_id == IOMMUFD_INVALID_ID)
		return -EOPNOTSUPP;

	obj = iommufd_get_object(ucmd->ictx, cmd->dev_id, IOMMUFD_OBJ_DEVICE);
	if (IS_ERR(obj))
		return PTR_ERR(obj);

	idev = container_of(obj, struct iommufd_device, obj);

	rc = iommu_get_hw_info(idev->dev, &hw_info);
	if (rc < 0)
		goto out_put;

	cmd->iommu_hw_type = hw_info.type;

	if (hw_info.data_length <= cmd->hw_data_len &&
	    copy_to_user((void __user *)cmd->hw_data_ptr,
			 &hw_info.data, hw_info.data_length)) {
		rc = -EFAULT;
		goto out_put;
	}

	user_length = cmd->hw_data_len;
	cmd->hw_data_len = hw_info.data_length;

	rc = iommufd_ucmd_respond(ucmd, sizeof(*cmd));
	if (rc)
		goto out_put;

	if (hw_info.data_length > user_length) {
		rc = -EMSGSIZE;
	}

out_put:
	iommufd_put_object(obj);
	return rc;
}

unsigned int
iommufd_hw_pagetable_get_dev_id(struct iommufd_hw_pagetable *hwpt,
				struct device *dev, ioasid_t pasid)
{
	struct iommufd_hwpt_device *hdev = NULL;
	struct iommufd_device *idev = NULL;
	unsigned long index;
	bool check_pasid = pasid != INVALID_IOASID;

	mutex_lock(&hwpt->devices_lock);
	xa_for_each (&hwpt->devices, index, hdev)
		if (hdev->idev->dev == dev) {
			if (!check_pasid || (check_pasid &&
					     hdev->pasid_present &&
					     hdev->pasid == pasid)) {
				idev = hdev->idev;
				break;
			}
		}
	mutex_unlock(&hwpt->devices_lock);

	return idev ? idev->obj.id : IOMMUFD_INVALID_ID;
}

static bool iommufd_hw_pagetable_has_group(struct iommufd_hw_pagetable *hwpt,
					   struct iommu_group *group)
{
	struct iommufd_hwpt_device *hdev = NULL;
	unsigned long index;

	xa_for_each (&hwpt->devices, index, hdev)
		if (hdev->idev->group == group)
			return true;
	return false;
}

static int iommufd_device_setup_msi(struct iommufd_device *idev,
				    struct iommufd_hw_pagetable *hwpt,
				    phys_addr_t sw_msi_start,
				    unsigned int flags)
{
	int rc;

	/*
	 * IOMMU_CAP_INTR_REMAP means that the platform is isolating MSI,
	 * nothing further to do.
	 */
	if (iommu_capable(idev->dev->bus, IOMMU_CAP_INTR_REMAP))
		return 0;

	/*
	 * On ARM systems that set the global IRQ_DOMAIN_FLAG_MSI_REMAP every
	 * allocated iommu_domain will block interrupts by default and this
	 * special flow is needed to turn them back on.
	 */
	if (irq_domain_check_msi_remap()) {
		if (WARN_ON(!sw_msi_start))
			return -EPERM;
		/*
		 * iommu_get_msi_cookie() can only be called once per domain,
		 * it returns -EBUSY on later calls.
		 */
		if (hwpt->kernel.msi_cookie)
			return 0;
		rc = iommu_get_msi_cookie(hwpt->domain, sw_msi_start);
		if (rc && rc != -ENODEV)
			return rc;
		hwpt->kernel.msi_cookie = true;
		return 0;
	}

	/*
	 * Otherwise the platform has a MSI window that is not isolated. For
	 * historical compat with VFIO allow a module parameter to ignore the
	 * insecurity.
	 */
	if (!(flags & IOMMUFD_ATTACH_FLAGS_ALLOW_UNSAFE_INTERRUPT))
		return -EPERM;
	return 0;
}

static int
__iommufd_device_attach_kernel_hwpt(struct iommufd_device *idev,
				    struct iommufd_hw_pagetable *hwpt,
				    unsigned int flags)
{
	phys_addr_t sw_msi_start = 0;
	int rc;

	/*
	 * hwpt is now the exclusive owner of the group so this is the
	 * first time enforce is called for this group.
	 */
	rc = iopt_table_enforce_group_resv_regions(
		&hwpt->kernel.ioas->iopt, idev->group, &sw_msi_start);
	if (rc)
		return rc;

	rc = iommufd_device_setup_msi(idev, hwpt, sw_msi_start, flags);
	if (rc)
		iopt_remove_reserved_iova(&hwpt->kernel.ioas->iopt, idev->group);

	if (xa_empty(&hwpt->devices)) {
		rc = iopt_table_add_domain(&hwpt->kernel.ioas->iopt, hwpt->domain);
		if (rc)
			iopt_remove_reserved_iova(&hwpt->kernel.ioas->iopt, idev->group);
	}

	return rc;
}

static void
__iommufd_device_detach_kernel_hwpt(struct iommufd_device *idev,
				    struct iommufd_hw_pagetable *hwpt)
{
	iopt_remove_reserved_iova(&hwpt->kernel.ioas->iopt, idev->group);
	if (xa_empty(&hwpt->devices))
		iopt_table_remove_domain(&hwpt->kernel.ioas->iopt, hwpt->domain);
}

static struct iommufd_hwpt_device *
iommufd_alloc_hwpt_device(struct iommufd_hw_pagetable *hwpt,
			  struct iommufd_device *idev)
{
	struct iommufd_hwpt_device *hdev;

	hdev = kzalloc(sizeof(*hdev), GFP_KERNEL);
	if (!hdev)
		return NULL;

	hdev->hwpt = hwpt;
	hdev->idev = idev;

	return hdev;
}

static int iommufd_hwpt_device_finalize(struct iommufd_hwpt_device *hdev)
{

	return xa_alloc(&hdev->hwpt->devices, &hdev->id, hdev,
		     xa_limit_32b, GFP_KERNEL);
}

static struct iommufd_hwpt_device *
iommufd_device_attach_kernel_hwpt(struct iommufd_device *idev,
				  struct iommufd_hw_pagetable *hwpt,
				  unsigned int flags)
{
	struct iommufd_hwpt_device *hdev;
	int rc;

	hdev = iommufd_alloc_hwpt_device(hwpt, idev);
	if (!hdev) {
		rc = -ENOMEM;
		goto out;
	}

	if (!iommufd_hw_pagetable_has_group(hwpt, idev->group)) {
		/* FIXME: Use a device-centric iommu api. For now check if the
		 * hw_pagetable already has a device of the same group joined to tell if
		 * we are the first and need to attach the group. */
		rc = iommu_attach_group(hwpt->domain, idev->group);
		if (rc)
			goto out_free;;
		rc = __iommufd_device_attach_kernel_hwpt(idev, hwpt, flags);
		if (rc)
			goto out_detach;
	}

	rc = iommufd_hwpt_device_finalize(hdev);
	if (rc) {
		goto out_detach_hwpt;
	}

	return hdev;
out_detach_hwpt:
	if (!iommufd_hw_pagetable_has_group(hwpt, idev->group))
		__iommufd_device_detach_kernel_hwpt(idev, hwpt);
out_detach:
	if (!iommufd_hw_pagetable_has_group(hwpt, idev->group))
		iommu_detach_group(hwpt->domain, idev->group);
out_free:
	kfree(hdev);
out:
	return ERR_PTR(rc);
}

static struct iommufd_hwpt_device *
iommufd_device_attach_s1_hwpt(struct iommufd_device *idev,
			      struct iommufd_hw_pagetable *hwpt)
{
	struct iommufd_hwpt_device *hdev;
	int rc;

	hdev = iommufd_alloc_hwpt_device(hwpt, idev);
	if (!hdev) {
		rc = -ENOMEM;
		goto out;
	}

	rc = iommu_attach_device(hwpt->domain, idev->dev);
	if (rc)
		goto out_free;

	rc = iommufd_hwpt_device_finalize(hdev);
	if (rc) {
		goto out_detach;
	}

	return hdev;
out_detach:
	iommu_detach_device(hwpt->domain, idev->dev);
out_free:
	kfree(hdev);
out:
	return ERR_PTR(rc);
}

/**
 * iommufd_device_attach - Connect a device to an iommu_domain
 * @idev: device to attach
 * @pt_id: Input a IOMMUFD_OBJ_IOAS, or IOMMUFD_OBJ_HW_PAGETABLE
 *         Output the IOMMUFD_OBJ_HW_PAGETABLE ID
 * @flags: Optional flags
 *
 * This connects the device to an iommu_domain, either automatically or manually
 * selected. Once this completes the device could do DMA.
 *
 * The caller should return the resulting pt_id back to userspace.
 * This function is undone by calling iommufd_device_detach().
 */
int iommufd_device_attach(struct iommufd_device *idev, u32 *pt_id,
			  unsigned int flags)
{
	struct iommufd_hw_pagetable *hwpt;
	struct iommufd_hwpt_device *hdev;
	int rc;

	refcount_inc(&idev->obj.users);

	hwpt = iommufd_hw_pagetable_from_id(idev->ictx, *pt_id, idev->dev);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_users;
	}

	down_write(&idev->hwpts_rwsem);

	rc = xa_reserve(&idev->hwpts, hwpt->obj.id, GFP_KERNEL);
	if (rc)
		goto out_unlock_hwpts;

	mutex_lock(&hwpt->devices_lock);

	if (hwpt->type == IOMMUFD_HWPT_USER_S1) {
		hdev = iommufd_device_attach_s1_hwpt(idev, hwpt);
	} else if (hwpt->type == IOMMUFD_HWPT_KERNEL) {
		hdev = iommufd_device_attach_kernel_hwpt(idev, hwpt, flags);
	} else {
		rc = -EINVAL;
		goto out_unlock_devices;
	}

	if (IS_ERR(hdev)) {
		rc = PTR_ERR(hdev);
		goto out_unlock_devices;
	}

	mutex_unlock(&hwpt->devices_lock);
	xa_store(&idev->hwpts, hwpt->obj.id, hdev, GFP_KERNEL);
	up_write(&idev->hwpts_rwsem);

	*pt_id = hwpt->obj.id;
	return 0;

out_unlock_devices:
	mutex_unlock(&hwpt->devices_lock);
	xa_release(&idev->hwpts, hwpt->obj.id);
out_unlock_hwpts:
	up_write(&idev->hwpts_rwsem);
	iommufd_hw_pagetable_put(idev->ictx, hwpt);
out_users:
	refcount_dec(&idev->obj.users);
	return rc;
}
EXPORT_SYMBOL_GPL(iommufd_device_attach);

/**
 * iommufd_device_attach_pasid - Connect a device+pasid to an iommu_domain
 * @idev: device to attach
 * @pasid: pasid to attach
 * @pt_id: Input a IOMMUFD_OBJ_IOAS, or IOMMUFD_OBJ_HW_PAGETABLE
 *         Output the IOMMUFD_OBJ_HW_PAGETABLE ID
 * @flags: Optional flags
 *
 * This connects the device+pasid to an iommu_domain.
 *
 * The caller should return the resulting pt_id back to userspace.
 * This function is undone by calling iommufd_device_detach().
 */
int iommufd_device_attach_pasid(struct iommufd_device *idev, u32 *pt_id,
				ioasid_t pasid, unsigned int flags)
{
	struct iommufd_hw_pagetable *hwpt;
	struct iommufd_hwpt_device *hdev;
	int rc;

	refcount_inc(&idev->obj.users);

	hwpt = iommufd_hw_pagetable_from_id(idev->ictx, *pt_id, idev->dev);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_users;
	}

	down_write(&idev->hwpts_rwsem);

	rc = xa_reserve(&idev->hwpts, hwpt->obj.id, GFP_KERNEL);
	if (rc)
		goto out_unlock_hwpts;

	mutex_lock(&hwpt->devices_lock);

	hdev = iommufd_alloc_hwpt_device(hwpt, idev);
	if (!hdev) {
		rc = -ENOMEM;
		goto out_unlock_devices;
	}

	rc = iommu_attach_device_pasid(hwpt->domain,
				       idev->dev, pasid);
	if (rc)
		goto out_free;

	if (hwpt->type == IOMMUFD_HWPT_KERNEL) {
		rc = __iommufd_device_attach_kernel_hwpt(idev, hwpt, flags);
		if (rc)
			goto out_detach;
	}

	hdev->pasid = pasid;
	hdev->pasid_present = true;

	rc = iommufd_hwpt_device_finalize(hdev);
	if (rc) {
		goto out_detach_hwpt;
	}

	mutex_unlock(&hwpt->devices_lock);
	xa_store(&idev->hwpts, hwpt->obj.id, hdev, GFP_KERNEL);
	up_write(&idev->hwpts_rwsem);

	*pt_id = hwpt->obj.id;
	return 0;

out_detach_hwpt:
	if (hwpt->type == IOMMUFD_HWPT_KERNEL)
		__iommufd_device_detach_kernel_hwpt(idev, hwpt);
out_detach:
	iommu_detach_device_pasid(hwpt->domain, idev->dev, pasid);
out_free:
	kfree(hdev);
out_unlock_devices:
	mutex_unlock(&hwpt->devices_lock);
	xa_release(&idev->hwpts, hwpt->obj.id);
out_unlock_hwpts:
	up_write(&idev->hwpts_rwsem);
	iommufd_hw_pagetable_put(idev->ictx, hwpt);
out_users:
	refcount_dec(&idev->obj.users);
	return rc;
}
EXPORT_SYMBOL_GPL(iommufd_device_attach_pasid);

void iommufd_device_detach(struct iommufd_device *idev, u32 hwpt_id)
{
	struct iommufd_hw_pagetable *hwpt;
	struct iommufd_hwpt_device *hdev;

	down_write(&idev->hwpts_rwsem);

	hdev = xa_load(&idev->hwpts, hwpt_id);
	if (!hdev) {
		up_write(&idev->hwpts_rwsem);
		return;
	}

	hwpt = hdev->hwpt;
	mutex_lock(&hwpt->devices_lock);
	xa_erase(&hdev->hwpt->devices, hdev->id);
	if (!hdev->pasid_present) {
		if (hwpt->type == IOMMUFD_HWPT_KERNEL &&
		    !iommufd_hw_pagetable_has_group(hwpt, idev->group)) {
			__iommufd_device_detach_kernel_hwpt(idev, hwpt);
			iommu_detach_group(hwpt->domain, idev->group);
		} else if (hwpt->type == IOMMUFD_HWPT_USER_S1)
			iommu_detach_device(hwpt->domain, idev->dev);
	} else {
		if (hwpt->type == IOMMUFD_HWPT_KERNEL)
			__iommufd_device_detach_kernel_hwpt(idev, hwpt);
		iommu_detach_device_pasid(hwpt->domain, idev->dev, hdev->pasid);
		iommu_unregister_device_fault_handler(idev->dev);
	}

	kfree(hdev);
	mutex_unlock(&hwpt->devices_lock);
	xa_erase(&idev->hwpts, hwpt_id);
	up_write(&idev->hwpts_rwsem);

	iommufd_hw_pagetable_put(idev->ictx, hwpt);

	refcount_dec(&idev->obj.users);
}
EXPORT_SYMBOL_GPL(iommufd_device_detach);

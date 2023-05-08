// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#include <linux/vfio.h>
#include <linux/iommufd.h>

#include "vfio.h"

MODULE_IMPORT_NS(IOMMUFD);
MODULE_IMPORT_NS(IOMMUFD_VFIO);

int vfio_iommufd_compat_probe_noiommu(struct vfio_device *device,
				      struct iommufd_ctx *ictx)
{
	u32 ioas_id;

	if (!capable(CAP_SYS_RAWIO))
		return -EPERM;

	/*
	 * Require no compat ioas to be assigned to proceed.  The basic
	 * statement is that the user cannot have done something that
	 * implies they expected translation to exist
	 */
	if (!iommufd_vfio_compat_ioas_get_id(ictx, &ioas_id))
		return -EPERM;
	return 0;
}

int vfio_iommufd_bind(struct vfio_device_file *df)
{
	struct vfio_device *vdev = df->device;
	struct iommufd_ctx *ictx = df->iommufd;

	lockdep_assert_held(&vdev->dev_set->lock);

	if (vdev->noiommu)
		return vfio_iommufd_emulated_bind(vdev, ictx, &df->devid);

	return vdev->ops->bind_iommufd(vdev, ictx, &df->devid);
}

int vfio_iommufd_compat_attach_ioas(struct vfio_device *vdev,
				    struct iommufd_ctx *ictx)
{
	u32 ioas_id;
	int ret;

	lockdep_assert_held(&vdev->dev_set->lock);

	/* compat noiommu does not need to do ioas attach */
	if (vdev->noiommu)
		return 0;

	ret = iommufd_vfio_compat_ioas_get_id(ictx, &ioas_id);
	if (ret)
		return ret;

	/* The legacy path has no way to return the selected pt_id */
	return vdev->ops->attach_ioas(vdev, &ioas_id);
}

void vfio_iommufd_unbind(struct vfio_device_file *df)
{
	struct vfio_device *vdev = df->device;

	lockdep_assert_held(&vdev->dev_set->lock);

	if (vdev->noiommu) {
		vfio_iommufd_emulated_unbind(vdev);
		return;
	}

	if (vdev->ops->unbind_iommufd)
		vdev->ops->unbind_iommufd(vdev);
}

int vfio_iommufd_attach(struct vfio_device *vdev, u32 *pt_id)
{
	lockdep_assert_held(&vdev->dev_set->lock);

	if (vdev->noiommu)
		return vfio_iommufd_emulated_attach_ioas(vdev, pt_id);

	return vdev->ops->attach_ioas(vdev, pt_id);
}

void vfio_iommufd_detach(struct vfio_device *vdev)
{
	lockdep_assert_held(&vdev->dev_set->lock);

	if (vdev->noiommu)
		vfio_iommufd_emulated_detach_ioas(vdev);
	else
		vdev->ops->detach_ioas(vdev);
}

struct iommufd_ctx *vfio_iommufd_physical_ictx(struct vfio_device *vdev)
{
	if (vdev->iommufd_device)
		return iommufd_device_to_ictx(vdev->iommufd_device);
	if (vdev->iommufd_access)
		return iommufd_access_to_ictx(vdev->iommufd_access);
	return NULL;
}
EXPORT_SYMBOL_GPL(vfio_iommufd_physical_ictx);

/*
 * Return devid for devices that have been bound with iommufd,
 * returns 0 if not bound yet.
 */
u32 vfio_iommufd_physical_devid(struct vfio_device *vdev)
{
	if (WARN_ON(!vdev->iommufd_device && !vdev->iommufd_access))
		return 0;
	if (vdev->iommufd_device)
		return iommufd_device_to_id(vdev->iommufd_device);
	else
		return iommufd_access_to_id(vdev->iommufd_access);
}
EXPORT_SYMBOL_GPL(vfio_iommufd_physical_devid);
/*
 * The physical standard ops mean that the iommufd_device is bound to the
 * physical device vdev->dev that was provided to vfio_init_group_dev(). Drivers
 * using this ops set should call vfio_register_group_dev()
 */
int vfio_iommufd_physical_bind(struct vfio_device *vdev,
			       struct iommufd_ctx *ictx, u32 *out_device_id)
{
	struct iommufd_device *idev;

	idev = iommufd_device_bind(ictx, vdev->dev, out_device_id);
	if (IS_ERR(idev))
		return PTR_ERR(idev);
	vdev->iommufd_device = idev;
	return 0;
}
EXPORT_SYMBOL_GPL(vfio_iommufd_physical_bind);

void vfio_iommufd_physical_unbind(struct vfio_device *vdev)
{
	lockdep_assert_held(&vdev->dev_set->lock);

	if (vdev->iommufd_attached) {
		iommufd_device_detach(vdev->iommufd_device);
		vdev->iommufd_attached = false;
	}
	iommufd_device_unbind(vdev->iommufd_device);
	vdev->iommufd_device = NULL;
}
EXPORT_SYMBOL_GPL(vfio_iommufd_physical_unbind);

int vfio_iommufd_physical_attach_ioas(struct vfio_device *vdev, u32 *pt_id)
{
	int rc;

	lockdep_assert_held(&vdev->dev_set->lock);

	if (WARN_ON(!vdev->iommufd_device))
		return -EINVAL;

	if (vdev->iommufd_attached)
		return -EBUSY;

	rc = iommufd_device_attach(vdev->iommufd_device, pt_id);
	if (rc)
		return rc;
	vdev->iommufd_attached = true;
	return 0;
}
EXPORT_SYMBOL_GPL(vfio_iommufd_physical_attach_ioas);

void vfio_iommufd_physical_detach_ioas(struct vfio_device *vdev)
{
	lockdep_assert_held(&vdev->dev_set->lock);

	if (WARN_ON(!vdev->iommufd_device) || !vdev->iommufd_attached)
		return;

	iommufd_device_detach(vdev->iommufd_device);
	vdev->iommufd_attached = false;
}
EXPORT_SYMBOL_GPL(vfio_iommufd_physical_detach_ioas);

/*
 * The emulated standard ops can be used by below usages:
 * 1) The vfio_device that is going to use the "mdev path" and will call
 *    vfio_pin_pages()/vfio_dma_rw().  Such drivers using should call
 *    vfio_register_emulated_iommu_dev().  Drivers that do not call
 *    vfio_pin_pages()/vfio_dma_rw() have no need to provide dma_unmap.
 * 2) The noiommu device which doesn't have backend iommu but creating
 *    an iommufd_access allows generating a dev_id for it. noiommu device
 *    is not allowed to do map/unmap so this becomes a nop.
 */

static void vfio_emulated_unmap(void *data, unsigned long iova,
				unsigned long length)
{
	struct vfio_device *vdev = data;

	/* noiommu devices cannot do map/unmap */
	if (vdev->noiommu && vdev->ops->dma_unmap)
		vdev->ops->dma_unmap(vdev, iova, length);
}

static const struct iommufd_access_ops vfio_user_ops = {
	.needs_pin_pages = 1,
	.unmap = vfio_emulated_unmap,
};

int vfio_iommufd_emulated_bind(struct vfio_device *vdev,
			       struct iommufd_ctx *ictx, u32 *out_device_id)
{
	struct iommufd_access *user;

	lockdep_assert_held(&vdev->dev_set->lock);

	user = iommufd_access_create(ictx, &vfio_user_ops, vdev, out_device_id);
	if (IS_ERR(user))
		return PTR_ERR(user);
	vdev->iommufd_access = user;
	return 0;
}
EXPORT_SYMBOL_GPL(vfio_iommufd_emulated_bind);

void vfio_iommufd_emulated_unbind(struct vfio_device *vdev)
{
	lockdep_assert_held(&vdev->dev_set->lock);

	if (vdev->iommufd_access) {
		iommufd_access_destroy(vdev->iommufd_access);
		vdev->iommufd_attached = false;
		vdev->iommufd_access = NULL;
	}
}
EXPORT_SYMBOL_GPL(vfio_iommufd_emulated_unbind);

int vfio_iommufd_emulated_attach_ioas(struct vfio_device *vdev, u32 *pt_id)
{
	int rc;

	lockdep_assert_held(&vdev->dev_set->lock);

	if (vdev->iommufd_attached)
		return -EBUSY;
	rc = iommufd_access_attach(vdev->iommufd_access, *pt_id);
	if (rc)
		return rc;
	vdev->iommufd_attached = true;
	return 0;
}
EXPORT_SYMBOL_GPL(vfio_iommufd_emulated_attach_ioas);

void vfio_iommufd_emulated_detach_ioas(struct vfio_device *vdev)
{
	lockdep_assert_held(&vdev->dev_set->lock);

	if (WARN_ON(!vdev->iommufd_access) || !vdev->iommufd_attached)
		return;

	iommufd_access_detach(vdev->iommufd_access);
	vdev->iommufd_attached = false;
}
EXPORT_SYMBOL_GPL(vfio_iommufd_emulated_detach_ioas);

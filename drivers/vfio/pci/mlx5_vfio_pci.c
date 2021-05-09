// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, Mellanox Technologies. All rights reserved.
 *     Author: Max Gurtovoy <mgurtovoy@nvidia.com>
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/device.h>
#include <linux/eventfd.h>
#include <linux/file.h>
#include <linux/interrupt.h>
#include <linux/iommu.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/pci.h>
#include <linux/pm_runtime.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/vfio.h>
#include <linux/sched/mm.h>
#include <linux/mlx5/driver.h>

#include <linux/vfio_pci_core.h>

static int mlx5_vfio_pci_open(struct vfio_device *core_vdev)
{
	struct vfio_pci_core_device *vdev =
		container_of(core_vdev, struct vfio_pci_core_device, vdev);
	int ret;

	lockdep_assert_held(&core_vdev->reflck->lock);

	ret = vfio_pci_core_enable(vdev);
	if (ret)
		return ret;

	/* TODO: register migration region here for capable devices */

	vfio_pci_core_finish_enable(vdev);

	return 0;
}

static const struct vfio_device_ops mlx5_vfio_pci_ops = {
	.name		= "mlx5-vfio-pci",
	.open		= mlx5_vfio_pci_open,
	.release	= vfio_pci_core_release,
	.ioctl		= vfio_pci_core_ioctl,
	.read		= vfio_pci_core_read,
	.write		= vfio_pci_core_write,
	.mmap		= vfio_pci_core_mmap,
	.request	= vfio_pci_core_request,
	.match		= vfio_pci_core_match,
	.reflck_attach	= vfio_pci_core_reflck_attach,
};

static int mlx5_vfio_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct vfio_pci_core_device *vdev;
	int ret;

	vdev = kzalloc(sizeof(*vdev), GFP_KERNEL);
	if (!vdev)
		return -ENOMEM;

	ret = vfio_pci_core_register_device(vdev, pdev, &mlx5_vfio_pci_ops);
	if (ret)
		goto out_free;

	dev_set_drvdata(&pdev->dev, vdev);
	return 0;

out_free:
	kfree(vdev);
	return ret;
}

static void mlx5_vfio_pci_remove(struct pci_dev *pdev)
{
	struct vfio_pci_core_device *vdev = dev_get_drvdata(&pdev->dev);

	vfio_pci_core_unregister_device(vdev);
	kfree(vdev);
}

static const struct pci_device_id mlx5_vfio_pci_table[] = {
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0x101b) }, /* ConnectX-6 */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0x101c) }, /* ConnectX-6 VF */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0x101d) }, /* ConnectX-6 Dx */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0x101e) }, /* ConnectX Family mlx5Gen Virtual Function */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0x101f) }, /* ConnectX-6 LX */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0x1021) }, /* ConnectX-7 */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0xa2d2) }, /* BlueField integrated ConnectX-5 network controller */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0xa2d3) }, /* BlueField integrated ConnectX-5 network controller VF */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0xa2d6) }, /* BlueField-2 integrated ConnectX-6 Dx network controller */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_MELLANOX, 0xa2dc) }, /* BlueField-3 integrated ConnectX-7 network controller */
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, mlx5_vfio_pci_table);

static struct pci_driver mlx5_vfio_pci_driver = {
	.name			= "mlx5-vfio-pci",
	.id_table		= mlx5_vfio_pci_table,
	.probe			= mlx5_vfio_pci_probe,
	.remove			= mlx5_vfio_pci_remove,
#ifdef CONFIG_PCI_IOV
	.sriov_configure	= vfio_pci_core_sriov_configure,
#endif
	.err_handler		= &vfio_pci_core_err_handlers,
};

static void __exit mlx5_vfio_pci_cleanup(void)
{
	pci_unregister_driver(&mlx5_vfio_pci_driver);
}

static int __init mlx5_vfio_pci_init(void)
{
	return pci_register_driver(&mlx5_vfio_pci_driver);
}

module_init(mlx5_vfio_pci_init);
module_exit(mlx5_vfio_pci_cleanup);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Max Gurtovoy <mgurtovoy@nvidia.com>");
MODULE_DESCRIPTION("MLX5 VFIO PCI - User Level meta-driver for MLX5 device family");

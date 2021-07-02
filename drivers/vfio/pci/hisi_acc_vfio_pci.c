// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, HiSilicon Ltd.
 */

#include <linux/device.h>
#include <linux/eventfd.h>
#include <linux/file.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/vfio.h>
#include <linux/vfio_pci_core.h>

static long hisi_acc_vfio_pci_ioctl(struct vfio_device *core_vdev, unsigned int cmd,
				    unsigned long arg)
{
	struct vfio_pci_core_device *vdev =
		container_of(core_vdev, struct vfio_pci_core_device, vdev);

	if (cmd == VFIO_DEVICE_GET_REGION_INFO) {
		struct pci_dev *pdev = vdev->pdev;
		struct vfio_region_info info;
		unsigned long minsz;

		minsz = offsetofend(struct vfio_region_info, offset);

		if (copy_from_user(&info, (void __user *)arg, minsz))
			return -EFAULT;

		if (info.argsz < minsz)
			return -EINVAL;

		if (info.index == VFIO_PCI_BAR2_REGION_INDEX) {
			info.offset = VFIO_PCI_INDEX_TO_OFFSET(info.index);

			/*
			 * ACC VF dev BAR2 region(64K) consists of both functional
			 * register space and migration control register space.
			 * Report only the first 32K(functional region) to Guest.
			 */
			info.size = pci_resource_len(pdev, info.index) / 2;

			info.flags = VFIO_REGION_INFO_FLAG_READ |
					VFIO_REGION_INFO_FLAG_WRITE |
					VFIO_REGION_INFO_FLAG_MMAP;

			return copy_to_user((void __user *)arg, &info, minsz) ?
					    -EFAULT : 0;
		}
	}
	return vfio_pci_core_ioctl(core_vdev, cmd, arg);
}

static int hisi_acc_vfio_pci_open(struct vfio_device *core_vdev)
{
	struct vfio_pci_core_device *vdev =
		container_of(core_vdev, struct vfio_pci_core_device, vdev);
	int ret;

	lockdep_assert_held(&core_vdev->reflck->lock);

	ret = vfio_pci_core_enable(vdev);
	if (ret)
		return ret;

	vfio_pci_core_finish_enable(vdev);

	return 0;
}

static const struct vfio_device_ops hisi_acc_vfio_pci_ops = {
	.name		= "hisi-acc-vfio-pci",
	.open		= hisi_acc_vfio_pci_open,
	.release	= vfio_pci_core_release,
	.ioctl		= hisi_acc_vfio_pci_ioctl,
	.read		= vfio_pci_core_read,
	.write		= vfio_pci_core_write,
	.mmap		= vfio_pci_core_mmap,
	.request	= vfio_pci_core_request,
	.match		= vfio_pci_core_match,
	.reflck_attach	= vfio_pci_core_reflck_attach,
};

static int hisi_acc_vfio_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct vfio_pci_core_device *vdev;
	int ret;

	vdev = kzalloc(sizeof(*vdev), GFP_KERNEL);
	if (!vdev)
		return -ENOMEM;

	ret = vfio_pci_core_register_device(vdev, pdev, &hisi_acc_vfio_pci_ops);
	if (ret)
		goto out_free;

	dev_set_drvdata(&pdev->dev, vdev);

	return 0;

out_free:
	kfree(vdev);
	return ret;
}

static void hisi_acc_vfio_pci_remove(struct pci_dev *pdev)
{
	struct vfio_pci_core_device *vdev = dev_get_drvdata(&pdev->dev);

	vfio_pci_core_unregister_device(vdev);
	kfree(vdev);
}

static const struct pci_device_id hisi_acc_vfio_pci_table[] = {
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_HUAWEI, 0xa256) }, /* SEC VF */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_HUAWEI, 0xa259) }, /* HPRE VF */
	{ PCI_DRIVER_OVERRIDE_DEVICE_VFIO(PCI_VENDOR_ID_HUAWEI, 0xa251) }, /* ZIP VF */
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, hisi_acc_vfio_pci_table);

static struct pci_driver hisi_acc_vfio_pci_driver = {
	.name			= "hisi-acc-vfio-pci",
	.id_table		= hisi_acc_vfio_pci_table,
	.probe			= hisi_acc_vfio_pci_probe,
	.remove			= hisi_acc_vfio_pci_remove,
#ifdef CONFIG_PCI_IOV
	.sriov_configure	= vfio_pci_core_sriov_configure,
#endif
	.err_handler		= &vfio_pci_core_err_handlers,
};

module_pci_driver(hisi_acc_vfio_pci_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Liu Longfang <liulongfang@huawei.com>");
MODULE_AUTHOR("Shameer Kolothum <shameerali.kolothum.thodi@huawei.com>");
MODULE_DESCRIPTION("HiSilicon VFIO PCI - Generic VFIO PCI driver for HiSilicon ACC device family");

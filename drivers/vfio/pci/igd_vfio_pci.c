// SPDX-License-Identifier: GPL-2.0-only
/*
 * VFIO PCI Intel Graphics support
 *
 * Copyright (C) 2016 Red Hat, Inc.  All rights reserved.
 *	Author: Alex Williamson <alex.williamson@redhat.com>
 *
 * Register a device specific region through which to provide read-only
 * access to the Intel IGD opregion.  The register defining the opregion
 * address is also virtualized to prevent user modification.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/vfio.h>

#include "vfio_pci_core.h"
#include "igd_vfio_pci.h"

#define OPREGION_SIGNATURE	"IntelGraphicsMem"
#define OPREGION_SIZE		(8 * 1024)
#define OPREGION_PCI_ADDR	0xfc

#define DRIVER_VERSION  "0.1"
#define DRIVER_AUTHOR   "Alex Williamson <alex.williamson@redhat.com>"
#define DRIVER_DESC     "IGD VFIO PCI - User Level meta-driver for Intel Graphics Processing Unit"

struct igd_vfio_pci_device {
	struct vfio_pci_core_device	vdev;
};

static size_t vfio_pci_igd_rw(struct vfio_pci_core_device *vdev,
		char __user *buf, size_t count, loff_t *ppos, bool iswrite)
{
	unsigned int i = VFIO_PCI_OFFSET_TO_INDEX(*ppos) - VFIO_PCI_NUM_REGIONS;
	void *base = vdev->region[i].data;
	loff_t pos = *ppos & VFIO_PCI_OFFSET_MASK;

	if (pos >= vdev->region[i].size || iswrite)
		return -EINVAL;

	count = min(count, (size_t)(vdev->region[i].size - pos));

	if (copy_to_user(buf, base + pos, count))
		return -EFAULT;

	*ppos += count;

	return count;
}

static void vfio_pci_igd_release(struct vfio_pci_core_device *vdev,
				 struct vfio_pci_region *region)
{
	memunmap(region->data);
}

static const struct vfio_pci_regops vfio_pci_igd_regops = {
	.rw		= vfio_pci_igd_rw,
	.release	= vfio_pci_igd_release,
};

static int vfio_pci_igd_opregion_init(struct vfio_pci_core_device *vdev)
{
	__le32 *dwordp = (__le32 *)(vdev->vconfig + OPREGION_PCI_ADDR);
	u32 addr, size;
	void *base;
	int ret;

	ret = pci_read_config_dword(vdev->pdev, OPREGION_PCI_ADDR, &addr);
	if (ret)
		return ret;

	if (!addr || !(~addr))
		return -ENODEV;

	base = memremap(addr, OPREGION_SIZE, MEMREMAP_WB);
	if (!base)
		return -ENOMEM;

	if (memcmp(base, OPREGION_SIGNATURE, 16)) {
		memunmap(base);
		return -EINVAL;
	}

	size = le32_to_cpu(*(__le32 *)(base + 16));
	if (!size) {
		memunmap(base);
		return -EINVAL;
	}

	size *= 1024; /* In KB */

	if (size != OPREGION_SIZE) {
		memunmap(base);
		base = memremap(addr, size, MEMREMAP_WB);
		if (!base)
			return -ENOMEM;
	}

	ret = vfio_pci_register_dev_region(vdev,
		PCI_VENDOR_ID_INTEL | VFIO_REGION_TYPE_PCI_VENDOR_TYPE,
		VFIO_REGION_SUBTYPE_INTEL_IGD_OPREGION,
		&vfio_pci_igd_regops, size, VFIO_REGION_INFO_FLAG_READ, base);
	if (ret) {
		memunmap(base);
		return ret;
	}

	/* Fill vconfig with the hw value and virtualize register */
	*dwordp = cpu_to_le32(addr);
	memset(vdev->pci_config_map + OPREGION_PCI_ADDR,
	       PCI_CAP_ID_INVALID_VIRT, 4);

	return ret;
}

static size_t vfio_pci_igd_cfg_rw(struct vfio_pci_core_device *vdev,
				  char __user *buf, size_t count, loff_t *ppos,
				  bool iswrite)
{
	unsigned int i = VFIO_PCI_OFFSET_TO_INDEX(*ppos) - VFIO_PCI_NUM_REGIONS;
	struct pci_dev *pdev = vdev->region[i].data;
	loff_t pos = *ppos & VFIO_PCI_OFFSET_MASK;
	size_t size;
	int ret;

	if (pos >= vdev->region[i].size || iswrite)
		return -EINVAL;

	size = count = min(count, (size_t)(vdev->region[i].size - pos));

	if ((pos & 1) && size) {
		u8 val;

		ret = pci_user_read_config_byte(pdev, pos, &val);
		if (ret)
			return pcibios_err_to_errno(ret);

		if (copy_to_user(buf + count - size, &val, 1))
			return -EFAULT;

		pos++;
		size--;
	}

	if ((pos & 3) && size > 2) {
		u16 val;

		ret = pci_user_read_config_word(pdev, pos, &val);
		if (ret)
			return pcibios_err_to_errno(ret);

		val = cpu_to_le16(val);
		if (copy_to_user(buf + count - size, &val, 2))
			return -EFAULT;

		pos += 2;
		size -= 2;
	}

	while (size > 3) {
		u32 val;

		ret = pci_user_read_config_dword(pdev, pos, &val);
		if (ret)
			return pcibios_err_to_errno(ret);

		val = cpu_to_le32(val);
		if (copy_to_user(buf + count - size, &val, 4))
			return -EFAULT;

		pos += 4;
		size -= 4;
	}

	while (size >= 2) {
		u16 val;

		ret = pci_user_read_config_word(pdev, pos, &val);
		if (ret)
			return pcibios_err_to_errno(ret);

		val = cpu_to_le16(val);
		if (copy_to_user(buf + count - size, &val, 2))
			return -EFAULT;

		pos += 2;
		size -= 2;
	}

	while (size) {
		u8 val;

		ret = pci_user_read_config_byte(pdev, pos, &val);
		if (ret)
			return pcibios_err_to_errno(ret);

		if (copy_to_user(buf + count - size, &val, 1))
			return -EFAULT;

		pos++;
		size--;
	}

	*ppos += count;

	return count;
}

static void vfio_pci_igd_cfg_release(struct vfio_pci_core_device *vdev,
				     struct vfio_pci_region *region)
{
	struct pci_dev *pdev = region->data;

	pci_dev_put(pdev);
}

static const struct vfio_pci_regops vfio_pci_igd_cfg_regops = {
	.rw		= vfio_pci_igd_cfg_rw,
	.release	= vfio_pci_igd_cfg_release,
};

static int vfio_pci_igd_cfg_init(struct vfio_pci_core_device *vdev)
{
	struct pci_dev *host_bridge, *lpc_bridge;
	int ret;

	host_bridge = pci_get_domain_bus_and_slot(0, 0, PCI_DEVFN(0, 0));
	if (!host_bridge)
		return -ENODEV;

	if (host_bridge->vendor != PCI_VENDOR_ID_INTEL ||
	    host_bridge->class != (PCI_CLASS_BRIDGE_HOST << 8)) {
		pci_dev_put(host_bridge);
		return -EINVAL;
	}

	ret = vfio_pci_register_dev_region(vdev,
		PCI_VENDOR_ID_INTEL | VFIO_REGION_TYPE_PCI_VENDOR_TYPE,
		VFIO_REGION_SUBTYPE_INTEL_IGD_HOST_CFG,
		&vfio_pci_igd_cfg_regops, host_bridge->cfg_size,
		VFIO_REGION_INFO_FLAG_READ, host_bridge);
	if (ret) {
		pci_dev_put(host_bridge);
		return ret;
	}

	lpc_bridge = pci_get_domain_bus_and_slot(0, 0, PCI_DEVFN(0x1f, 0));
	if (!lpc_bridge)
		return -ENODEV;

	if (lpc_bridge->vendor != PCI_VENDOR_ID_INTEL ||
	    lpc_bridge->class != (PCI_CLASS_BRIDGE_ISA << 8)) {
		pci_dev_put(lpc_bridge);
		return -EINVAL;
	}

	ret = vfio_pci_register_dev_region(vdev,
		PCI_VENDOR_ID_INTEL | VFIO_REGION_TYPE_PCI_VENDOR_TYPE,
		VFIO_REGION_SUBTYPE_INTEL_IGD_LPC_CFG,
		&vfio_pci_igd_cfg_regops, lpc_bridge->cfg_size,
		VFIO_REGION_INFO_FLAG_READ, lpc_bridge);
	if (ret) {
		pci_dev_put(lpc_bridge);
		return ret;
	}

	return 0;
}

static int vfio_pci_igd_init(struct vfio_pci_core_device *vdev)
{
	int ret;

	ret = vfio_pci_igd_opregion_init(vdev);
	if (ret)
		return ret;

	ret = vfio_pci_igd_cfg_init(vdev);
	if (ret)
		return ret;

	return 0;
}

static void igd_vfio_pci_release(void *device_data)
{
	struct vfio_pci_core_device *vdev = device_data;

	mutex_lock(&vdev->reflck->lock);
	if (!(--vdev->refcnt)) {
		vfio_pci_vf_token_user_add(vdev, -1);
		vfio_pci_core_spapr_eeh_release(vdev);
		vfio_pci_core_disable(vdev);
	}
	mutex_unlock(&vdev->reflck->lock);

	module_put(THIS_MODULE);
}

static int igd_vfio_pci_open(void *device_data)
{
	struct vfio_pci_core_device *vdev = device_data;
	int ret = 0;

	if (!try_module_get(THIS_MODULE))
		return -ENODEV;

	mutex_lock(&vdev->reflck->lock);

	if (!vdev->refcnt) {
		ret = vfio_pci_core_enable(vdev);
		if (ret)
			goto error;

		ret = vfio_pci_igd_init(vdev);
		if (ret && ret != -ENODEV) {
			pci_warn(vdev->pdev, "Failed to setup Intel IGD regions\n");
			vfio_pci_core_disable(vdev);
			goto error;
		}
		ret = 0;
		vfio_pci_probe_mmaps(vdev);
		vfio_pci_core_spapr_eeh_open(vdev);
		vfio_pci_vf_token_user_add(vdev, 1);
	}
	vdev->refcnt++;
error:
	mutex_unlock(&vdev->reflck->lock);
	if (ret)
		module_put(THIS_MODULE);
	return ret;
}

static const struct vfio_device_ops igd_vfio_pci_ops = {
	.name		= "igd-vfio-pci",
	.open		= igd_vfio_pci_open,
	.release	= igd_vfio_pci_release,
	.ioctl		= vfio_pci_core_ioctl,
	.read		= vfio_pci_core_read,
	.write		= vfio_pci_core_write,
	.mmap		= vfio_pci_core_mmap,
	.request	= vfio_pci_core_request,
	.match		= vfio_pci_core_match,
};

static int igd_vfio_pci_probe(struct pci_dev *pdev,
		const struct pci_device_id *id)
{
	struct igd_vfio_pci_device *igvdev;
	int ret;

	igvdev = kzalloc(sizeof(*igvdev), GFP_KERNEL);
	if (!igvdev)
		return -ENOMEM;

	ret = vfio_pci_core_register_device(&igvdev->vdev, pdev,
			&igd_vfio_pci_ops);
	if (ret)
		goto out_free;

	return 0;

out_free:
	kfree(igvdev);
	return ret;
}

static void igd_vfio_pci_remove(struct pci_dev *pdev)
{
	struct vfio_device *vdev = dev_get_drvdata(&pdev->dev);
	struct vfio_pci_core_device *core_vpdev = vfio_device_data(vdev);
	struct igd_vfio_pci_device *igvdev;

	igvdev = container_of(core_vpdev, struct igd_vfio_pci_device, vdev);

	vfio_pci_core_unregister_device(core_vpdev);
	kfree(igvdev);
}

static const struct pci_device_id igd_vfio_pci_table[] = {
	{ PCI_VENDOR_ID_INTEL, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_DISPLAY_VGA << 8, 0xff0000, 0 },
	{ 0, }
};

static struct pci_driver igd_vfio_pci_driver = {
	.name			= "igd-vfio-pci",
	.id_table		= igd_vfio_pci_table,
	.probe			= igd_vfio_pci_probe,
	.remove			= igd_vfio_pci_remove,
#ifdef CONFIG_PCI_IOV
	.sriov_configure	= vfio_pci_core_sriov_configure,
#endif
	.err_handler		= &vfio_pci_core_err_handlers,
};

#ifdef CONFIG_VFIO_PCI_DRIVER_COMPAT
struct pci_driver *get_igd_vfio_pci_driver(struct pci_dev *pdev)
{
	if (pci_match_id(igd_vfio_pci_driver.id_table, pdev))
		return &igd_vfio_pci_driver;
	return NULL;
}
EXPORT_SYMBOL_GPL(get_igd_vfio_pci_driver);
#endif

module_pci_driver(igd_vfio_pci_driver);

MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

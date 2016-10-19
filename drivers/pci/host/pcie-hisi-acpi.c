/*
 * PCIe host controller driver for HiSilicon HipXX SoCs
 *
 * Copyright (C) 2016 HiSilicon Co., Ltd. http://www.hisilicon.com
 *
 * Author: Dongdong Liu <liudongdong3@huawei.com>
 *         Gabriele Paoloni <gabriele.paoloni@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/pci.h>
#include <linux/pci-acpi.h>
#include <linux/pci-ecam.h>

#define DEBUG0				0x728
#define PCIE_LTSSM_LINKUP_STATE		0x11
#define PCIE_LTSSM_STATE_MASK		0x3F

static const struct acpi_device_id hisi_pcie_rc_res_ids[] = {
	{"HISI0081", 0},
	{"", 0},
};

static int hisi_pcie_link_up_acpi(struct pci_config_window *cfg)
{
	u32 val;
	void __iomem *reg_base = cfg->priv;

	val = readl(reg_base + DEBUG0);
	return ((val & PCIE_LTSSM_STATE_MASK) == PCIE_LTSSM_LINKUP_STATE);
}

static int hisi_pcie_acpi_rd_conf(struct pci_bus *bus, u32 devfn, int where,
				  int size, u32 *val)
{
	struct pci_config_window *cfg = bus->sysdata;
	int dev = PCI_SLOT(devfn);

	if (bus->number == cfg->busr.start) {
		/* access only one slot on each root port */
		if (dev > 0)
			return PCIBIOS_DEVICE_NOT_FOUND;
		else
			return pci_generic_config_read32(bus, devfn, where,
							 size, val);
	}

	return pci_generic_config_read(bus, devfn, where, size, val);
}

static int hisi_pcie_acpi_wr_conf(struct pci_bus *bus, u32 devfn,
				  int where, int size, u32 val)
{
	struct pci_config_window *cfg = bus->sysdata;
	int dev = PCI_SLOT(devfn);

	if (bus->number == cfg->busr.start) {
		/* access only one slot on each root port */
		if (dev > 0)
			return PCIBIOS_DEVICE_NOT_FOUND;
		else
			return pci_generic_config_write32(bus, devfn, where,
							  size, val);
	}

	return pci_generic_config_write(bus, devfn, where, size, val);
}

static void __iomem *hisi_pcie_map_bus(struct pci_bus *bus, unsigned int devfn,
				       int where)
{
	struct pci_config_window *cfg = bus->sysdata;
	void __iomem *reg_base = cfg->priv;

	if (bus->number == cfg->busr.start)
		return reg_base + where;
	else
		return pci_ecam_map_bus(bus, devfn, where);
}

/*
 * Retrieve RC base and size from sub-device under the RC
 * Device (RES1)
 * {
 *	Name (_HID, "HISI0081")
 *	Name (_CID, "PNP0C02")
 *	Name (_CRS, ResourceTemplate (){
 *		Memory32Fixed (ReadWrite, 0xb0080000, 0x10000)
 *	})
 * }
 */
static int hisi_pcie_rc_addr_get(struct acpi_device *adev,
				 void __iomem **addr)
{
	struct acpi_device *child_adev;
	struct list_head list;
	struct resource *res;
	struct resource_entry *entry;
	unsigned long flags;
	int ret;

	list_for_each_entry(child_adev, &adev->children, node) {
		ret = acpi_match_device_ids(child_adev, hisi_pcie_rc_res_ids);
		if (ret)
			continue;

		INIT_LIST_HEAD(&list);
		flags = IORESOURCE_MEM;
		ret = acpi_dev_get_resources(child_adev, &list,
					     acpi_dev_filter_resource_type_cb,
					     (void *)flags);
		if (ret < 0) {
			dev_err(&child_adev->dev,
				"failed to parse _CRS method, error code %d\n",
				ret);
			return ret;
		} else if (ret == 0) {
			dev_err(&child_adev->dev,
				"no IO and memory resources present in _CRS\n");
			return -EINVAL;
		}

		entry = list_first_entry(&list, struct resource_entry, node);
		res = entry->res;
		*addr = devm_ioremap(&child_adev->dev,
				     res->start, resource_size(res));
		acpi_dev_free_resource_list(&list);
		if (IS_ERR(*addr)) {
			dev_err(&child_adev->dev, "error with ioremap\n");
			return -ENOMEM;
		}

		return 0;
	}

	return -EINVAL;
}

static int hisi_pcie_init(struct pci_config_window *cfg)
{
	int ret;
	struct acpi_device *adev = to_acpi_device(cfg->parent);
	void __iomem *reg_base;

	ret = hisi_pcie_rc_addr_get(adev, &reg_base);
	if (ret) {
		dev_err(&adev->dev, "can't get rc base address");
		return ret;
	}

	cfg->priv = reg_base;
	if (!hisi_pcie_link_up_acpi(cfg)) {
		dev_err(&adev->dev, "link status is down\n");
		return -EINVAL;
	}

	return 0;
}

struct pci_ecam_ops hisi_pcie_ops = {
	.bus_shift    = 20,
	.init         =  hisi_pcie_init,
	.pci_ops      = {
		.map_bus    = hisi_pcie_map_bus,
		.read       = hisi_pcie_acpi_rd_conf,
		.write      = hisi_pcie_acpi_wr_conf,
	}
};

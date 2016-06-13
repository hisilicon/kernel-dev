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
#include <linux/acpi.h>
#include <linux/pci.h>
#include <linux/pci-acpi.h>
#include <linux/pci-ecam.h>
#include "pcie-hisi.h"

#define GET_PCIE_LINK_STATUS  0x0

#define EFI_ACPI_HISI_OEM_ID "HISI"
#define EFI_ACPI_HISI_D02_OEM_TABLE_ID  "HISI-D02"
#define EFI_ACPI_HISI_D03_OEM_TABLE_ID  "HISI-D03"

struct hisi_pcie_acpi {
	struct acpi_device *device;
	void __iomem	*reg_base;
};

/* uuid 6d30f553-836c-408e-b6ad-45bccc957949 */
const u8 hisi_pcie_acpi_dsm_uuid[] = {
	0x53, 0xf5, 0x30, 0x6d, 0x6c, 0x83, 0x8e, 0x40,
	0xb6, 0xad, 0x45, 0xbc, 0xcc, 0x95, 0x79, 0x49
};

static const struct acpi_device_id hisi_pcie_rc_base_ids[] = {
	{"HISI0081", 0},
	{"", 0},
};


static int hisi_pcie_link_up_acpi(struct pci_config_window *cfg)
{
	u32 val;
	union acpi_object *obj;
	struct hisi_pcie_acpi *pcie_acpi = (struct hisi_pcie_acpi *)cfg->priv;
	struct acpi_device *device = pcie_acpi->device;

	obj = acpi_evaluate_dsm(device->handle,
				hisi_pcie_acpi_dsm_uuid, 0,
				GET_PCIE_LINK_STATUS, NULL);

	if (!obj  ||  obj->type != ACPI_TYPE_INTEGER)  {
		dev_err(&device->dev, "can't get link status from _DSM\n");
		return 0;
	}
	val = obj->integer.value;

	return ((val & PCIE_LTSSM_STATE_MASK) == PCIE_LTSSM_LINKUP_STATE);

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
static int hisi_pcie_rc_addr_get(struct acpi_device *device,
				 void __iomem **addr)
{
	struct acpi_device *child_device;
	struct list_head list;
	struct resource *res;
	struct resource_entry *entry;
	unsigned long flags;
	int ret;

	INIT_LIST_HEAD(&list);
	list_for_each_entry(child_device, &device->children, node) {
		ret = acpi_match_device_ids(child_device,
					    hisi_pcie_rc_base_ids);
		if (ret)
			continue;

		flags = IORESOURCE_MEM;
		ret = acpi_dev_get_resources(child_device, &list,
					     acpi_dev_filter_resource_type_cb,
					     (void *)flags);
		if (ret < 0) {
			dev_warn(&child_device->dev,
				 "failed to parse _CRS method, error code %d\n",
				 ret);
				return ret;
		} else if (ret == 0)
			dev_dbg(&child_device->dev,
				"no memory resources present in _CRS\n");

		entry = list_first_entry(&list, struct resource_entry, node);
		res = entry->res;

		*addr = devm_ioremap(&child_device->dev,
				     res->start, resource_size(res));
		if (IS_ERR(*addr)) {
			acpi_dev_free_resource_list(&list);
			dev_err(&child_device->dev, "error with ioremap\n");
			return -ENOMEM;
		}
	}
	acpi_dev_free_resource_list(&list);

	return 0;
}

static int hisi_pcie_init(struct pci_config_window *cfg)
{
	int ret;
	struct acpi_device *device = to_acpi_device(cfg->parent);
	void __iomem *reg_base = 0;
	struct hisi_pcie_acpi *pcie_acpi;

	pcie_acpi = devm_kzalloc(cfg->parent, sizeof(*pcie_acpi), GFP_KERNEL);
	if (!pcie_acpi)
		return -ENOMEM;

	ret = hisi_pcie_rc_addr_get(device, &reg_base);
	if (ret) {
		dev_err(&device->dev, "can't get rc base address");
		return ret;
	}

	pcie_acpi->reg_base = reg_base;
	pcie_acpi->device = device;
	cfg->priv = pcie_acpi;

	if (!hisi_pcie_link_up_acpi(cfg))
		dev_warn(cfg->parent, "link status is down\n");

	return 0;
}

static int hisi_pcie_acpi_valid_config(struct pci_config_window *cfg,
				       struct pci_bus *bus, int dev)
{
	/* If there is no link, then there is no device */
	if (bus->number != cfg->busr.start) {
		if (!hisi_pcie_link_up_acpi(cfg))
			return 0;
	}

	/* access only one slot on each root port */
	if (bus->number == cfg->busr.start && dev > 0)
		return 0;

	/*
	 * do not read more than one device on the bus directly attached
	 * to RC's (Virtual Bridge's) DS side.
	 */
	if (bus->primary == cfg->busr.start && dev > 0)
		return 0;

	return 1;
}

static int hisi_pcie_acpi_rd_conf(struct pci_bus *bus, u32 devfn, int where,
				  int size, u32 *val)
{
	struct pci_config_window *cfg = bus->sysdata;
	struct hisi_pcie_acpi *pcie_acpi = (struct hisi_pcie_acpi *)cfg->priv;
	void __iomem *reg_base = pcie_acpi->reg_base;

	if (hisi_pcie_acpi_valid_config(cfg, bus, PCI_SLOT(devfn)) == 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (bus->number == cfg->busr.start)
		return hisi_pcie_common_cfg_read(reg_base, where, size, val);

	return pci_generic_config_read(bus, devfn, where, size, val);
}

static int hisi_pcie_acpi_wr_conf(struct pci_bus *bus, u32 devfn,
				  int where, int size, u32 val)
{
	struct pci_config_window *cfg = bus->sysdata;
	struct hisi_pcie_acpi *pcie_acpi = (struct hisi_pcie_acpi *)cfg->priv;
	void __iomem *reg_base = pcie_acpi->reg_base;

	if (hisi_pcie_acpi_valid_config(cfg, bus, PCI_SLOT(devfn)) == 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (bus->number == cfg->busr.start)
		return hisi_pcie_common_cfg_write(reg_base, where, size, val);

	return pci_generic_config_write(bus, devfn, where, size, val);
}

static struct pci_ecam_ops hisi_pcie_ecam_ops = {
	.bus_shift	= 20,
	.init		= hisi_pcie_init,
	.pci_ops		= {
		.map_bus	= pci_ecam_map_bus,
		.read		= hisi_pcie_acpi_rd_conf,
		.write		= hisi_pcie_acpi_wr_conf,
	}
};

DECLARE_ACPI_MCFG_FIXUP(&hisi_pcie_ecam_ops, EFI_ACPI_HISI_OEM_ID,
       EFI_ACPI_HISI_D02_OEM_TABLE_ID, PCI_MCFG_DOMAIN_ANY, PCI_MCFG_BUS_ANY);

DECLARE_ACPI_MCFG_FIXUP(&hisi_pcie_ecam_ops, EFI_ACPI_HISI_OEM_ID,
       EFI_ACPI_HISI_D03_OEM_TABLE_ID, PCI_MCFG_DOMAIN_ANY, PCI_MCFG_BUS_ANY);

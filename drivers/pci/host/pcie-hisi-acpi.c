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
#define MAX_RC_NUM			16
#define PCIE_LTSSM_LINKUP_STATE		0x11
#define PCIE_LTSSM_STATE_MASK		0x3F

enum soc_type {
	HIP05,
	HIP06,
	HIP07,
};

struct hisi_rc_res {
	int soc_type;
	struct resource res[MAX_RC_NUM];
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

static struct hisi_rc_res rc_res[] = {
	{
		HIP05,
		{
			DEFINE_RES_MEM(0xb0070000, SZ_4K),
			DEFINE_RES_MEM(0xb0080000, SZ_4K),
			DEFINE_RES_MEM(0xb0090000, SZ_4K),
			DEFINE_RES_MEM(0xb00a0000, SZ_4K)
		}
	},
	{
		HIP06,
		{
			DEFINE_RES_MEM(0xa0090000, SZ_4K),
			DEFINE_RES_MEM(0xa0200000, SZ_4K),
			DEFINE_RES_MEM(0xa00a0000, SZ_4K),
			DEFINE_RES_MEM(0xa00b0000, SZ_4K)
		}
	},
	{
		HIP07,
		{
			DEFINE_RES_MEM(0xa0090000, SZ_4K),
			DEFINE_RES_MEM(0xa0200000, SZ_4K),
			DEFINE_RES_MEM(0xa00a0000, SZ_4K),
			DEFINE_RES_MEM(0xa00b0000, SZ_4K),
			DEFINE_RES_MEM(0x8a0090000UL, SZ_4K),
			DEFINE_RES_MEM(0x8a0200000UL, SZ_4K),
			DEFINE_RES_MEM(0x8a00a0000UL, SZ_4K),
			DEFINE_RES_MEM(0x8a00b0000UL, SZ_4K),
			DEFINE_RES_MEM(0x600a0090000UL, SZ_4K),
			DEFINE_RES_MEM(0x600a0200000UL, SZ_4K),
			DEFINE_RES_MEM(0x600a00a0000UL, SZ_4K),
			DEFINE_RES_MEM(0x600a00b0000UL, SZ_4K),
			DEFINE_RES_MEM(0x700a0090000UL, SZ_4K),
			DEFINE_RES_MEM(0x700a0200000UL, SZ_4K),
			DEFINE_RES_MEM(0x700a00a0000UL, SZ_4K),
			DEFINE_RES_MEM(0x700a00b0000UL, SZ_4K)
		}
	},
};

static int hisi_pcie_acpi_init(struct pci_config_window *cfg, int soc_type)
{
	struct acpi_device *adev = to_acpi_device(cfg->parent);
	struct acpi_pci_root *root = acpi_driver_data(adev);
	void __iomem *reg_base;
	struct resource *res;

	res = &rc_res[soc_type].res[root->segment];
	reg_base = devm_ioremap_resource(&adev->dev, res);
	if (!reg_base)
		return -ENOMEM;

	cfg->priv = reg_base;
	if (!hisi_pcie_link_up_acpi(cfg)) {
		dev_err(&adev->dev, "link status is down\n");
		return -EINVAL;
	}

	return 0;
}

static int hisi_pcie_hip05_init(struct pci_config_window *cfg)
{
	return hisi_pcie_acpi_init(cfg, HIP05);
}

static int hisi_pcie_hip06_init(struct pci_config_window *cfg)
{
	return hisi_pcie_acpi_init(cfg, HIP06);
}

static int hisi_pcie_hip07_init(struct pci_config_window *cfg)
{
	return hisi_pcie_acpi_init(cfg, HIP07);
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

struct pci_ecam_ops hisi_pcie_hip05_ops = {
	.bus_shift    = 20,
	.init         =  hisi_pcie_hip05_init,
	.pci_ops      = {
		.map_bus    = hisi_pcie_map_bus,
		.read       = hisi_pcie_acpi_rd_conf,
		.write      = hisi_pcie_acpi_wr_conf,
	}
};

struct pci_ecam_ops hisi_pcie_hip06_ops = {
	.bus_shift    = 20,
	.init         =  hisi_pcie_hip06_init,
	.pci_ops      = {
		.map_bus    = hisi_pcie_map_bus,
		.read       = hisi_pcie_acpi_rd_conf,
		.write      = hisi_pcie_acpi_wr_conf,
	}
};

struct pci_ecam_ops hisi_pcie_hip07_ops = {
	.bus_shift    = 20,
	.init         =  hisi_pcie_hip07_init,
	.pci_ops      = {
		.map_bus    = hisi_pcie_map_bus,
		.read       = hisi_pcie_acpi_rd_conf,
		.write      = hisi_pcie_acpi_wr_conf,
	}
};

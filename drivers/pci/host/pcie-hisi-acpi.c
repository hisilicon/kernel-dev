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

#include "mcfg-quirks.h"
#include "pcie-hisi.h"

#define DEBUG0          0x728
#define MAX_RC_NUM	16

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
	void __iomem *reg_base = cfg->priv;

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
	void __iomem *reg_base = cfg->priv;

	if (hisi_pcie_acpi_valid_config(cfg, bus, PCI_SLOT(devfn)) == 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (bus->number == cfg->busr.start)
		return hisi_pcie_common_cfg_write(reg_base, where, size, val);

	return pci_generic_config_write(bus, devfn, where, size, val);
}

static struct pci_ops hisi_pcie_ops = {
	.map_bus	= pci_ecam_map_bus,
	.read		= hisi_pcie_acpi_rd_conf,
	.write		= hisi_pcie_acpi_wr_conf,
};

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

struct pci_config_window *hisi_pcie_acpi_init(struct acpi_pci_root *root,
					      struct pci_ops *ops,
					      int soc_type)
{
	struct acpi_device *adev = root->device;
	void __iomem *reg_base;
	struct pci_config_window *cfg;
	struct resource *res;

	res = &rc_res[soc_type].res[root->segment];
	reg_base = devm_ioremap_resource(&adev->dev, res);
	if (!reg_base)
		return ERR_PTR(-ENOMEM);

	cfg = pci_acpi_setup_ecam_mapping(root, &hisi_pcie_ops);
	if (IS_ERR(cfg)) {
		dev_err(&adev->dev, "error %ld mapping ECAM\n", PTR_ERR(cfg));
		return NULL;
	}

	cfg->priv = reg_base;

	if (!hisi_pcie_link_up_acpi(cfg))
		dev_warn(&adev->dev, "link status is down\n");

	return cfg;
}

struct pci_config_window *hisi_pcie_acpi_hip05_init(struct acpi_pci_root *root,
						    struct pci_ops *ops)
{
	return hisi_pcie_acpi_init(root, ops, HIP05);
}

struct pci_config_window *hisi_pcie_acpi_hip06_init(struct acpi_pci_root *root,
						    struct pci_ops *ops)
{
	return hisi_pcie_acpi_init(root, ops, HIP06);
}

struct pci_config_window *hisi_pcie_acpi_hip07_init(struct acpi_pci_root *root,
						    struct pci_ops *ops)
{
	return hisi_pcie_acpi_init(root, ops, HIP07);
}

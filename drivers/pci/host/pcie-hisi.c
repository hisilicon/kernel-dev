/*
 * PCIe host controller driver for HiSilicon SoCs
 *
 * Copyright (C) 2015 HiSilicon Co., Ltd. http://www.hisilicon.com
 *
 * Authors: Zhou Wang <wangzhou1@hisilicon.com>
 *          Dacai Zhu <zhudacai@hisilicon.com>
 *          Gabriele Paoloni <gabriele.paoloni@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/mfd/syscon.h>
#include <linux/of_address.h>
#include <linux/of_pci.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/regmap.h>

#include "pcie-designware.h"
#include "pcie-hisi.h"

#define PCIE_SUBCTRL_SYS_STATE4_REG			0x6818
#define PCIE_SYS_STATE4						0x31c
#define PCIE_HIP06_CTRL_OFF					0x1000

#define to_hisi_pcie(x)	container_of(x, struct hisi_pcie, pp)

struct hisi_pcie {
	struct regmap *subctrl;
	void __iomem *reg_base;
	u32 port_id;
	struct pcie_port pp;
	struct pcie_soc_ops *soc_ops;
};

struct pcie_soc_ops {
	int (*hisi_pcie_link_up)(struct hisi_pcie *pcie);
};

static inline int hisi_rd_ecam_conf(struct pcie_port *pp, struct pci_bus *bus,
		unsigned int devfn, int where, int size, u32 *value)
{
	return pci_generic_config_read(bus, devfn, where, size, value);
}

static inline int hisi_wr_ecam_conf(struct pcie_port *pp, struct pci_bus *bus,
		unsigned int devfn, int where, int size, u32 value)
{
	return pci_generic_config_write(bus, devfn, where, size, value);
}

static inline int hisi_pcie_cfg_read(struct pcie_port *pp, int where,
		int size, u32 *val)
{
	struct hisi_pcie *pcie = to_hisi_pcie(pp);

	return hisi_pcie_common_cfg_read(pcie->reg_base, where, size, val);
}

static inline int hisi_pcie_cfg_write(struct pcie_port *pp, int where,
		int size, u32 val)
{
	struct hisi_pcie *pcie = to_hisi_pcie(pp);

	return hisi_pcie_common_cfg_write(pcie->reg_base, where, size, val);
}

static int hisi_pcie_link_up(struct pcie_port *pp)
{
	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);

	return hisi_pcie->soc_ops->hisi_pcie_link_up(hisi_pcie);
}

static struct pcie_host_ops hisi_pcie_host_ops = {
	.rd_own_conf = hisi_pcie_cfg_read,
	.wr_own_conf = hisi_pcie_cfg_write,
	.link_up = hisi_pcie_link_up,
};

static void __iomem *hisi_pci_map_cfg_bus_cam(struct pci_bus *bus,
					      unsigned int devfn,
					      int where)
{
	void __iomem *addr;
	struct pcie_port *pp = bus->sysdata;

	addr = pp->va_cfg1_base - (pp->busn->start << 20) +
			((bus->number << 20) | (devfn << 12)) +
			where;

	return addr;
}

static int hisi_add_pcie_port(struct pcie_port *pp,
				     struct platform_device *pdev)
{
	int ret;
	u32 port_id;
	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);

	if (of_property_read_u32(pdev->dev.of_node, "port-id", &port_id)) {
		dev_err(&pdev->dev, "failed to read port-id\n");
		return -EINVAL;
	}
	if (port_id > 3) {
		dev_err(&pdev->dev, "Invalid port-id: %d\n", port_id);
		return -EINVAL;
	}
	hisi_pcie->port_id = port_id;

	pp->ops = &hisi_pcie_host_ops;

	ret = dw_pcie_host_init(pp);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize host\n");
		return ret;
	}

	return 0;
}

static int hisi_pcie_probe(struct platform_device *pdev)
{
	struct hisi_pcie *hisi_pcie;
	struct pcie_port *pp;
	const struct of_device_id *match;
	struct resource *reg;
	struct device_driver *driver;
	int ret;

	hisi_pcie = devm_kzalloc(&pdev->dev, sizeof(*hisi_pcie), GFP_KERNEL);
	if (!hisi_pcie)
		return -ENOMEM;

	pp = &hisi_pcie->pp;
	pp->dev = &pdev->dev;
	driver = (pdev->dev).driver;

	match = of_match_device(driver->of_match_table, &pdev->dev);
	hisi_pcie->soc_ops = (struct pcie_soc_ops *) match->data;

	hisi_pcie->subctrl =
	syscon_regmap_lookup_by_compatible("hisilicon,pcie-sas-subctrl");
	if (IS_ERR(hisi_pcie->subctrl)) {
		dev_err(pp->dev, "cannot get subctrl base\n");
		return PTR_ERR(hisi_pcie->subctrl);
	}

	reg = platform_get_resource_byname(pdev, IORESOURCE_MEM, "rc_dbi");
	hisi_pcie->reg_base = devm_ioremap_resource(&pdev->dev, reg);
	if (IS_ERR(hisi_pcie->reg_base)) {
		dev_err(pp->dev, "cannot get rc_dbi base\n");
		return PTR_ERR(hisi_pcie->reg_base);
	}

	hisi_pcie->pp.dbi_base = hisi_pcie->reg_base;

	reg = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ecam-cfg");
	if (reg) {
		/* ECAM driver version */
		hisi_pcie->pp.va_cfg0_base =
				devm_ioremap_resource(&pdev->dev, reg);
		if (IS_ERR(hisi_pcie->pp.va_cfg0_base)) {
			dev_err(pp->dev, "cannot get ecam-cfg\n");
			return PTR_ERR(hisi_pcie->pp.va_cfg0_base);
		}
		hisi_pcie->pp.va_cfg1_base = hisi_pcie->pp.va_cfg0_base;

		dw_pcie_ops.map_bus = hisi_pci_map_cfg_bus_cam;

		hisi_pcie_host_ops.rd_other_conf = hisi_rd_ecam_conf;
		hisi_pcie_host_ops.wr_other_conf = hisi_wr_ecam_conf;
	}

	ret = hisi_add_pcie_port(pp, pdev);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, hisi_pcie);

	dev_warn(pp->dev, "only 32-bit config accesses supported; smaller writes may corrupt adjacent RW1C fields\n");

	return 0;
}

static int hisi_pcie_link_up_hip05(struct hisi_pcie *hisi_pcie)
{
	u32 val;

	regmap_read(hisi_pcie->subctrl, PCIE_SUBCTRL_SYS_STATE4_REG +
		    0x100 * hisi_pcie->port_id, &val);

	return ((val & PCIE_LTSSM_STATE_MASK) == PCIE_LTSSM_LINKUP_STATE);
}

static int hisi_pcie_link_up_hip06(struct hisi_pcie *hisi_pcie)
{
	u32 val;

	val = readl(hisi_pcie->reg_base + PCIE_HIP06_CTRL_OFF +
			PCIE_SYS_STATE4);

	return ((val & PCIE_LTSSM_STATE_MASK) == PCIE_LTSSM_LINKUP_STATE);
}

static struct pcie_soc_ops hip05_ops = {
		&hisi_pcie_link_up_hip05
};

static struct pcie_soc_ops hip06_ops = {
		&hisi_pcie_link_up_hip06
};

static const struct of_device_id hisi_pcie_of_match[] = {
	{
			.compatible = "hisilicon,hip05-pcie",
			.data	    = (void *) &hip05_ops,
	},
	{
			.compatible = "hisilicon,hip06-pcie",
			.data	    = (void *) &hip06_ops,
	},
	{},
};

static struct platform_driver hisi_pcie_driver = {
	.probe  = hisi_pcie_probe,
	.driver = {
		   .name = "hisi-pcie",
		   .of_match_table = hisi_pcie_of_match,
	},
};
builtin_platform_driver(hisi_pcie_driver);

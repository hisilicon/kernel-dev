/*
 * HISILICON PV660 AHCI SATA platform driver
 *
 * based on the AHCI SATA platform driver by Jeff Garzik and Anton Vorontsov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/libata.h>
#include <linux/ahci_platform.h>
#include "ahci.h"

#define DRV_NAME "ahci_p660"

#define HISI_ATA_PHY_NUM	(8)
#define SAS_CONFIG_REG		(0xD4)
#define PORT_PHY_BASE_REG	(0x800)
#define PORT_PROG_PHY_LINK_RATE	(PORT_PHY_BASE_REG + 0xC)
#define PORT_PHY_NEG_TIME_CFG	(PORT_PHY_BASE_REG + 0x30)
#define PORT_PHY_SETTING_CFG	(PORT_PHY_BASE_REG + 0x44)
#define PORT_PHY_SERDES_CFG	(PORT_PHY_BASE_REG + 0x1A8)

struct hisi_ahci_priv {
	struct platform_device *ahci_pdev;
	void __iomem *sas_base;
};

/**FIXME:
 * This part should be configured by UEFI, and I won't write a phy driver,
 * Because the UEFI will enable sata driver too. It will be droped someday,
 * use this for DEBUG!
 */
static void hisi_port_phy_set(struct hisi_ahci_priv *hipriv)
{
	int i;
	u32 val;
	void __iomem *base = hipriv->sas_base;
	struct device *dev = &hipriv->ahci_pdev->dev;

	for (i = 0; i < HISI_ATA_PHY_NUM; i++) {
		/*
		 * max&min set 6.0Gbps, it will fail on some disk,
		 * the phy can't support auto-negotiation.
		 */
		writel(0x8aa, base + PORT_PROG_PHY_LINK_RATE + i*0x400);
		val = readl(base + PORT_PROG_PHY_LINK_RATE + i*0x400);
		dev_dbg(dev, "Phy[%d] Rate Seting: 0x%x\n", i, val);

		writel(0x8187C084, base + PORT_PHY_SERDES_CFG + i*0x400);
		writel(0x415e600, base + PORT_PHY_NEG_TIME_CFG + i*0x400);
		writel(0x80a80000, base + PORT_PHY_SETTING_CFG + i*0x400);
	}

	writel(0x22000000, base + SAS_CONFIG_REG);
}

static void hisi_port_phy_restart(struct ata_port *ap)
{
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct hisi_ahci_priv *hisipriv = hpriv->plat_data;
	void __iomem *base = hisipriv->sas_base;
	struct device *dev = &hisipriv->ahci_pdev->dev;
	int i = ap->port_no;

	dev_dbg(dev, "Phy[%d] restart.\n", i);
	writel(0x6, base + PORT_PHY_BASE_REG + i*0x400);
	writel(0x7, base + PORT_PHY_BASE_REG + i*0x400);
}

static const struct ata_port_info ahci_port_info = {
	.flags		= AHCI_FLAG_COMMON,
	.pio_mask	= ATA_PIO4,
	.udma_mask	= ATA_UDMA6,
	.port_ops	= &ahci_platform_ops,
};

static struct scsi_host_template ahci_platform_sht = {
	AHCI_SHT(DRV_NAME),
};

static int hisi_ahci_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ahci_host_priv *hpriv;
	struct hisi_ahci_priv *hisipriv;
	struct resource *res;
	u32 port_mask;
	int rc;

	hpriv = ahci_platform_get_resources(pdev);
	if (IS_ERR(hpriv))
		return PTR_ERR(hpriv);

	hpriv->restart_port_phy = hisi_port_phy_restart;
	hisipriv = devm_kzalloc(dev, sizeof(*hisipriv), GFP_KERNEL);
	if (!hisipriv)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res)
		return -ENODEV;

	hisipriv->sas_base = devm_ioremap(dev, res->start, resource_size(res));
	if (!hisipriv->sas_base)
		return -ENOMEM;

	if (of_property_read_u32(pdev->dev.of_node, "port_mask", &port_mask))
		port_mask = 0;

	hisipriv->ahci_pdev = pdev;
	hpriv->plat_data = hisipriv;
	hpriv->mask_port_map = port_mask;
	hpriv->flags = AHCI_HFLAG_NO_FBS | AHCI_HFLAG_NO_NCQ | AHCI_HFLAG_NO_PMP;

	hisi_port_phy_set(hisipriv);

	rc = ahci_platform_enable_resources(hpriv);
	if (rc)
		return rc;

	rc = ahci_platform_init_host(pdev, hpriv, &ahci_port_info,
				     &ahci_platform_sht);
	if (rc)
		ahci_platform_disable_resources(hpriv);
	return rc;
}

static SIMPLE_DEV_PM_OPS(ahci_hisi660_pm_ops, ahci_platform_suspend,
			 ahci_platform_resume);

static const struct of_device_id ahci_of_match[] = {
	{ .compatible = "hisilicon,p660-ahci", },
	{},
};
MODULE_DEVICE_TABLE(of, ahci_of_match);

static struct platform_driver ahci_driver = {
	.probe = hisi_ahci_probe,
	.remove = ata_platform_remove_one,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = ahci_of_match,
		.pm = &ahci_hisi660_pm_ops,
	},
};
module_platform_driver(ahci_driver);

MODULE_DESCRIPTION("HISILICON PV660 AHCI SATA platform driver");
MODULE_AUTHOR("Kefeng Wang <wangkefeng.wang@huawei.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ahci_p660");

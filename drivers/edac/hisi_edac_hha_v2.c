// SPDX-License-Identifier: GPL-2.0+
/* Copyright (c) 2018-2019 Hisilicon Limited. */
/*
 * EDAC Driver for Hisilicon HCCS Home Agent.
 *
 * Copyright (c) 2015, Hisilicon Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */
#include <linux/delay.h>
#include <linux/edac.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_data/hisi-djtag.h>

#include "edac_mc.h"

#define HHA_EER_EQRAM			BIT(29)
#define HHA_ERR_SPECDATRAM		BIT(29)
#define HHA_ERR_SPECREQRAM		BIT(17)

#define HHA_ERR_CMDRAM			BIT(11)

#define HHA_INT_MSK			0x0080
#define HHA_RAWINT			0x0084
#define HHA_INT_CLR			0x008c
#define HHA_ECC_STAT0			0x0090
#define HHA_ECC_STAT1			0x0094

#define HHA_INVALID_ECC_ERR_MASK	BIT(1)
#define HHA_NUM				2
#define EDAC_HHA			"HHA"

/* err for 1616 and 1382 toterm hha */
#define HI1382_HHA_DIRERR			BIT(3)
#define HI1382_HHA_RXREQ_OPERR			BIT(4)
#define HI1382_HHA_RXRSP_OPERR			BIT(5)
#define HI1382_HHA_RXDAT_OPERR			BIT(6)

struct hha_edac_dev;

struct hha_edac {
	unsigned int		irqnum;
	struct hha_edac_dev	*hhadev;
};

struct hha_edac_dev {
	struct device		*dev;
	unsigned int		irq[HHA_NUM];
	void __iomem		*hha_base_c;
	struct hha_edac		hhaisr[HHA_NUM];
	struct device_node	*djtag_node;
	spinlock_t		hha_irq_lock;
	const struct peri_hha_handle *hha_handle;
};

struct peri_hha_handle {
	void (*hi_edac_hha_init)(struct hha_edac_dev *hha_dev);
	irqreturn_t (*hi_edac_hha_irq)(int irq, void *dev_id);

	/*
	 * when using djtag to access hha reg, modsel specify the
	 * location of the module on the djtag.
	 */
	unsigned int mod_sel[HHA_NUM][2];

	/* when using spi interrupt,the irq num must list from zero */
	unsigned int irq_shift;
};

static int edac_djtag_write(struct device_node *node, unsigned int reg,
			    unsigned int modaddr, unsigned int value,
			    unsigned int mod_index)
{
	return djtag_writel(node, reg, modaddr, 1 << mod_index, value);
}

static int edac_djtag_read(struct device_node *node, unsigned int reg,
			unsigned int modaddr, unsigned int *value,
			unsigned int reg_index)
{
	return djtag_readl(node, reg, modaddr, reg_index, value);
}

static void hip05_edac_hha_init(struct hha_edac_dev *hha_dev)
{
	unsigned int i;

	for (i = 0; i < HHA_NUM; i++) {
		/* clear wrong interruption */
		edac_djtag_write(hha_dev->djtag_node, HHA_INT_CLR,
				 hha_dev->hha_handle->mod_sel[i][0], 0xf,
				 hha_dev->hha_handle->mod_sel[i][1]);
		/* clear mask res */
		edac_djtag_write(hha_dev->djtag_node, HHA_INT_MSK,
				 hha_dev->hha_handle->mod_sel[i][0], 0xd,
				 hha_dev->hha_handle->mod_sel[i][1]);
	}
}

static void hi1382_edac_hha_init(struct hha_edac_dev *hha_dev)
{
	unsigned int i;

	for (i = 0; i < HHA_NUM; i++) {
		/* clear interruption */
		edac_djtag_write(hha_dev->djtag_node, HHA_INT_CLR,
				 hha_dev->hha_handle->mod_sel[i][0], 0xfffff,
				 hha_dev->hha_handle->mod_sel[i][1]);
		/* clear mask res */
		edac_djtag_write(hha_dev->djtag_node, HHA_INT_MSK,
				 hha_dev->hha_handle->mod_sel[i][0], 0xfffc5,
				 hha_dev->hha_handle->mod_sel[i][1]);
	}
}

static irqreturn_t hi1382_edac_hha_irq(int irq, void *dev_id)
{
	struct hha_edac_dev *hha_dev;
	struct hha_edac *hha_edac = (struct hha_edac *)dev_id;
	unsigned int mod_addr;
	unsigned int mod_index;
	unsigned int inter;
	unsigned long flags;
	unsigned int dir_ecc_err = 0;

	hha_dev = hha_edac->hhadev;
	spin_lock_irqsave(&hha_dev->hha_irq_lock, flags);

	mod_addr = hha_dev->hha_handle->mod_sel[hha_edac->irqnum][0];
	mod_index = hha_dev->hha_handle->mod_sel[hha_edac->irqnum][1];

	/* open mask */
	edac_djtag_write(hha_dev->djtag_node, HHA_INT_MSK,
			 mod_addr, 0xfffff, mod_index);
	/* read interruption */
	edac_djtag_read(hha_dev->djtag_node, HHA_RAWINT,
			mod_addr, &inter, mod_index);

	if (inter & HHA_INVALID_ECC_ERR_MASK) {
		/* The following ECC error use same one interruption */
		edac_djtag_read(hha_dev->djtag_node, HHA_ECC_STAT0,
				mod_addr, &dir_ecc_err, mod_index);

		if (dir_ecc_err & HHA_EER_EQRAM) {
			edac_printk(KERN_CRIT, EDAC_HHA,
				    "%s: HHA_NON_DIRCACHE_MERR_MASK Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
				    dev_name(hha_dev->dev), inter,
				    mod_addr, mod_index);
		}
	}

	if (inter & HI1382_HHA_DIRERR)
		edac_printk(KERN_CRIT, EDAC_HHA,
			    "%s: HHA_DIRERR Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(hha_dev->dev), inter, mod_addr, mod_index);
	if (inter & HI1382_HHA_RXREQ_OPERR)
		edac_printk(KERN_CRIT, EDAC_HHA,
			    "%s: HHA_RXREQ_OPERR Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(hha_dev->dev), inter, mod_addr, mod_index);
	if (inter & HI1382_HHA_RXRSP_OPERR)
		edac_printk(KERN_CRIT, EDAC_HHA,
			    "%s: HHA_RXRSP_OPERR Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(hha_dev->dev), inter, mod_addr, mod_index);
	if (inter & HI1382_HHA_RXDAT_OPERR)
		edac_printk(KERN_CRIT, EDAC_HHA,
			    "%s: HHA_DIRERR Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(hha_dev->dev), inter, mod_addr, mod_index);
	/* clear interruption */
	edac_djtag_write(hha_dev->djtag_node, HHA_INT_CLR,
			 mod_addr, inter, mod_index);
	/* clear mask */
	edac_djtag_write(hha_dev->djtag_node, HHA_INT_MSK,
			 mod_addr, 0xfffc5, mod_index);

	spin_unlock_irqrestore(&hha_dev->hha_irq_lock, flags);

	return IRQ_HANDLED;
}

static irqreturn_t hip05_edac_hha_irq(int irq, void *dev_id)
{
	struct hha_edac_dev *hha_dev;
	struct hha_edac *hha_edac = (struct hha_edac *)dev_id;
	unsigned int mod_addr;
	unsigned int mod_index;
	unsigned int inter;
	unsigned long flags;
	unsigned int dir_ecc_err = 0;
	unsigned int non_dir_ecc_err = 0;

	hha_dev = hha_edac->hhadev;
	spin_lock_irqsave(&hha_dev->hha_irq_lock, flags);

	mod_addr = hha_dev->hha_handle->mod_sel[hha_edac->irqnum][0];
	mod_index = hha_dev->hha_handle->mod_sel[hha_edac->irqnum][1];

	/* open mask */
	edac_djtag_write(hha_dev->djtag_node, HHA_INT_MSK,
			 mod_addr, 0xf, mod_index);
	/* read interruption */
	edac_djtag_read(hha_dev->djtag_node, HHA_RAWINT,
			mod_addr, &inter, mod_index);

	if (inter & HHA_INVALID_ECC_ERR_MASK) {
		/* The following ECC error use same one interruption */
		edac_djtag_read(hha_dev->djtag_node, HHA_ECC_STAT0,
				mod_addr, &dir_ecc_err, mod_index);
		edac_djtag_read(hha_dev->djtag_node, HHA_ECC_STAT1,
				mod_addr, &non_dir_ecc_err, mod_index);

		if (dir_ecc_err & HHA_EER_EQRAM || non_dir_ecc_err &
		    HHA_ERR_SPECDATRAM || non_dir_ecc_err &
		    HHA_ERR_SPECREQRAM || non_dir_ecc_err &
		    HHA_ERR_CMDRAM)
			edac_printk(KERN_CRIT, EDAC_HHA,
				    "%s: HHA_NON_DIRCACHE_MERR_MASK Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
				    dev_name(hha_dev->dev), inter,
				    mod_addr, mod_index);
	}

	/* clear interruption */
	edac_djtag_write(hha_dev->djtag_node, HHA_INT_CLR,
			 mod_addr, inter, mod_index);
	/* clear mask */
	edac_djtag_write(hha_dev->djtag_node, HHA_INT_MSK,
			 mod_addr, 0x0, mod_index);

	spin_unlock_irqrestore(&hha_dev->hha_irq_lock, flags);

	return IRQ_HANDLED;
}

static const struct peri_hha_handle hip05_peri_hha_handle = {
	.hi_edac_hha_init = hip05_edac_hha_init,
	.hi_edac_hha_irq = hip05_edac_hha_irq,
	.mod_sel = { { 5, 0 }, { 10, 0 } },
	.irq_shift = 0,
};

static const struct peri_hha_handle hi1382_peri_hha_handle = {
	.hi_edac_hha_init = hi1382_edac_hha_init,
	.hi_edac_hha_irq = hi1382_edac_hha_irq,
	.mod_sel = { { 16, 0 }, { 32, 0 } },
	.irq_shift = 11,
};

static const struct of_device_id hha_edac_of_match[] = {
	/* for 1610 and 1612 toterm hha */
	{
		.compatible = "hisilicon,hha-edac",
		.data = &hip05_peri_hha_handle
	},
	/* for 1616 and 1382 toterm hha */
	{
		.compatible = "hisilicon,hi1382-hha-edac",
		.data = &hi1382_peri_hha_handle
	},
	{},
};

MODULE_DEVICE_TABLE(of, hha_edac_of_match);

static int hi_hha_probe(struct platform_device *pdev)
{
	struct hha_edac_dev *hha_dev;
	const struct of_device_id *of_id;
	unsigned int count;
	int rc, i;

	of_id = of_match_device(hha_edac_of_match, &pdev->dev);
	if (!of_id)
		return -EINVAL;

	hha_dev = devm_kzalloc(&pdev->dev, sizeof(*hha_dev), GFP_KERNEL);
	if (!hha_dev)
		return -ENOMEM;

	hha_dev->dev = &pdev->dev;

	platform_set_drvdata(pdev, hha_dev);
	hha_dev->hha_handle = of_id->data;

	/*
	 *for writing or reading djtag on nimbus side
	 * whem getting the point of djtag
	 */
	hha_dev->djtag_node = of_parse_phandle(pdev->dev.of_node, "djtag", 0);
	spin_lock_init(&hha_dev->hha_irq_lock);
	hha_dev->hha_handle->hi_edac_hha_init(hha_dev);

	for (count = 0; count < HHA_NUM; count++) {
		hha_dev->irq[count] = platform_get_irq(pdev,
			count + hha_dev->hha_handle->irq_shift);
		hha_dev->hhaisr[count].irqnum = count;
		hha_dev->hhaisr[count].hhadev = hha_dev;

		if (hha_dev->irq[count] < 0) {
			dev_err(&pdev->dev, "No IRQ resource\n");
			rc = -EINVAL;
			goto out_err;
		}

		rc = devm_request_irq(&pdev->dev, hha_dev->irq[count],
				      hha_dev->hha_handle->hi_edac_hha_irq,
				      0, dev_name(&pdev->dev),
				      &hha_dev->hhaisr[count]);
		if (rc) {
			dev_err(&pdev->dev, "irq count:%d req failed\n", count);
			goto out_err;
		}
	}

	return 0;

out_err:
	for (i = 0; i < count; i++)
		devm_free_irq(&pdev->dev, hha_dev->irq[i],
			      &hha_dev->hhaisr[i]);

	return rc;
}

static int hi_hha_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver hha_edac_driver = {
	.probe = hi_hha_probe,
	.remove = hi_hha_remove,
	.driver = {
		.name = "hha-edac",
		.owner = THIS_MODULE,
		.of_match_table = hha_edac_of_match,
	},
};

module_platform_driver(hha_edac_driver);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("HISILICON EDAC driver");
MODULE_AUTHOR("Peter Chen <luck.chen@huawei.com>");
MODULE_AUTHOR("Fengying Wang <fy.wang@huawei.com>");

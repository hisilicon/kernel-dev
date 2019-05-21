// SPDX-License-Identifier: GPL-2.0+
/* Copyright (c) 2018-2019 Hisilicon Limited. */

/* EDAC Driver for Hisilicon Last Level Cache. */
#include <linux/delay.h>
#include <linux/edac.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_data/hisi-djtag.h>

#include "edac_mc.h"

#define LLC_IRQ_NUM			4
#define LLC_NUM				4

#define LLC_BANK_INTM			0x0100
#define LLC_BANK_INTC			0x010c
#define LLC_BANK_RINT			0x0104

#define LLC_TAG_MERR_MASK		BIT(8)
#define LLC_DATA_ERR_MASK		BIT(9)
#define LLC_RESP_ERR_MASK		BIT(10)
#define LLC_EDAC			"LLC"

/* err for 1616 and 1382 toterm llc */
#define HI1382_LLC_CURR_ST_ERROR		BIT(11)
#define HI1382_LLC_SNOOP_ERR_NORMAL		BIT(12)
#define HI1382_LLC_DIROVF_ERR			BIT(13)
#define HI1382_LLC_REQ_ERR			BIT(14)

struct llc_edac_dev;

struct llc_edac {
	unsigned int		irqnum;
	struct llc_edac_dev	*llcdev;
};

struct llc_edac_dev {
	struct device		*dev;
	unsigned int		irq[LLC_IRQ_NUM];
	void __iomem		*llc_base_c;
	struct llc_edac		llcisr[LLC_NUM];
	struct device_node	*djtag_node;
	spinlock_t		llc_irq_lock;
	const struct peri_llc_handle *llc_handle;
};

struct peri_llc_handle {
	void (*hi_edac_llc_init)(struct llc_edac_dev *llc_dev);
	irqreturn_t (*hi_edac_llc_irq)(int irq, void *dev_id);

	/*
	 * when using djtag to access llc reg, modsel specify the
	 * location of the module on the djtag.
	 */
	unsigned int mod_sel[LLC_NUM][2];

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

static void hip05_edac_llc_init(struct llc_edac_dev *llc_dev)
{
	unsigned int i;

	for (i = 0; i < LLC_NUM; i++) {
		/* clear wrong interruption */
		edac_djtag_write(llc_dev->djtag_node, LLC_BANK_INTC,
				 llc_dev->llc_handle->mod_sel[i][0], 0x7ff,
				 llc_dev->llc_handle->mod_sel[i][1]);
		/* clear mask res */
		edac_djtag_write(llc_dev->djtag_node, LLC_BANK_INTM,
				 llc_dev->llc_handle->mod_sel[i][0], 0xff,
				 llc_dev->llc_handle->mod_sel[i][1]);
	}
}

static void hi1382_edac_llc_init(struct llc_edac_dev *llc_dev)
{
	unsigned int i;

	for (i = 0; i < LLC_NUM; i++) {
		/* clear interruption */
		edac_djtag_write(llc_dev->djtag_node, LLC_BANK_INTC,
				 llc_dev->llc_handle->mod_sel[i][0], 0x3ffff,
				 llc_dev->llc_handle->mod_sel[i][1]);
		/* clear mask res */
		edac_djtag_write(llc_dev->djtag_node, LLC_BANK_INTM,
				 llc_dev->llc_handle->mod_sel[i][0], 0x380ff,
				 llc_dev->llc_handle->mod_sel[i][1]);
	}
}

static irqreturn_t hip05_edac_llc_irq(int irq, void *dev_id)
{
	struct llc_edac_dev *llc_dev;
	struct llc_edac *llc_edac = (struct llc_edac *)dev_id;
	unsigned int inter = 0;
	unsigned long flags;
	unsigned int mod_addr;
	unsigned int mod_index;

	llc_dev = llc_edac->llcdev;
	spin_lock_irqsave(&llc_dev->llc_irq_lock, flags);

	mod_addr = llc_dev->llc_handle->mod_sel[llc_edac->irqnum][0];
	mod_index = llc_dev->llc_handle->mod_sel[llc_edac->irqnum][1];

	/* Open mask res */
	edac_djtag_write(llc_dev->djtag_node, LLC_BANK_INTM,
			 mod_addr, 0x7ff, mod_index);
	edac_djtag_read(llc_dev->djtag_node, LLC_BANK_RINT,
			mod_addr, &inter, mod_index);
	if (inter & LLC_RESP_ERR_MASK)
		edac_printk(KERN_CRIT, LLC_EDAC,
			"%s: LLC_RESP_ERR Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			dev_name(llc_dev->dev), inter, mod_addr, mod_index);

	/* Internal data sram occurs multiple bits error */
	if (inter & LLC_DATA_ERR_MASK)
		edac_printk(KERN_CRIT, LLC_EDAC,
			    "%s: LLC_DATA_ERR_MASK Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(llc_dev->dev), inter, mod_addr, mod_index);

	/* Internal tag occurs ram multiple bits error */
	if (inter & LLC_TAG_MERR_MASK)
		edac_printk(KERN_CRIT, LLC_EDAC,
			    "%s: LLC_TAG_MERR_MASK Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(llc_dev->dev), inter, mod_addr, mod_index);

	/* clear interruption */
	edac_djtag_write(llc_dev->djtag_node, LLC_BANK_INTC,
			 mod_addr, inter, mod_index);
	/* clear mask res */
	edac_djtag_write(llc_dev->djtag_node, LLC_BANK_INTM,
			 mod_addr, 0x0, mod_index);
	spin_unlock_irqrestore(&llc_dev->llc_irq_lock, flags);

	return IRQ_HANDLED;
}

static void hi1382_edac_llc_handle(struct llc_edac_dev *llc_dev,
				   unsigned int inter, unsigned int mod_addr,
				   unsigned int mod_index)
{

	if (inter & LLC_RESP_ERR_MASK)
		edac_printk(KERN_CRIT, LLC_EDAC,
			"%s: LLC_RESP_ERR Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			dev_name(llc_dev->dev), inter, mod_addr, mod_index);

	/* Internal data sram occurs multiple bits error */
	if (inter & LLC_DATA_ERR_MASK)
		edac_printk(KERN_CRIT, LLC_EDAC,
			    "%s: LLC_DATA_ERR_MASK Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(llc_dev->dev), inter, mod_addr, mod_index);

	/* Internal tag occurs ram multiple bits error */
	if (inter & LLC_TAG_MERR_MASK)
		edac_printk(KERN_CRIT, LLC_EDAC,
			    "%s: LLC_TAG_MERR_MASK Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(llc_dev->dev), inter, mod_addr, mod_index);

	if (inter & HI1382_LLC_CURR_ST_ERROR)
		edac_printk(KERN_CRIT, LLC_EDAC,
			"%s: LLC_CURR_ST_ERROR Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			dev_name(llc_dev->dev), inter, mod_addr, mod_index);

	if (inter & HI1382_LLC_SNOOP_ERR_NORMAL)
		edac_printk(KERN_CRIT, LLC_EDAC,
			    "%s: LLC_SNOOP_ERR_NORMAL Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(llc_dev->dev), inter, mod_addr, mod_index);

	if (inter & HI1382_LLC_DIROVF_ERR)
		edac_printk(KERN_CRIT, LLC_EDAC,
			    "%s: LLC_TAG_MERR_MASK Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(llc_dev->dev), inter, mod_addr, mod_index);
	if (inter & HI1382_LLC_REQ_ERR)
		edac_printk(KERN_CRIT, LLC_EDAC,
			    "%s: LLC_REQ_ERR Error,inter:0x%x,mod_addr:0x%x,mod_index:0x%x\n",
			    dev_name(llc_dev->dev), inter, mod_addr, mod_index);
}

static irqreturn_t hi1382_edac_llc_irq(int irq, void *dev_id)
{
	struct llc_edac_dev *llc_dev;
	struct llc_edac *llc_edac = (struct llc_edac *)dev_id;
	unsigned int inter;
	unsigned long flags;
	unsigned int mod_addr;
	unsigned int mod_index;

	llc_dev = llc_edac->llcdev;
	spin_lock_irqsave(&llc_dev->llc_irq_lock, flags);

	mod_addr = llc_dev->llc_handle->mod_sel[llc_edac->irqnum][0];
	mod_index = llc_dev->llc_handle->mod_sel[llc_edac->irqnum][1];

	/* Open mask res */
	edac_djtag_write(llc_dev->djtag_node, LLC_BANK_INTM,
			 mod_addr, 0x3ffff, mod_index);
	edac_djtag_read(llc_dev->djtag_node, LLC_BANK_RINT,
			mod_addr, &inter, mod_index);

	hi1382_edac_llc_handle(llc_dev, inter, mod_addr, mod_index);

	/* clear interruption */
	edac_djtag_write(llc_dev->djtag_node, LLC_BANK_INTC,
			 mod_addr, inter, mod_index);
	/* clear mask res */
	edac_djtag_write(llc_dev->djtag_node, LLC_BANK_INTM,
			 mod_addr, 0x380ff, mod_index);
	spin_unlock_irqrestore(&llc_dev->llc_irq_lock, flags);

	return IRQ_HANDLED;
}

static const struct peri_llc_handle hip05_peri_llc_handle = {
	.hi_edac_llc_init = hip05_edac_llc_init,
	.hi_edac_llc_irq = hip05_edac_llc_irq,
	.mod_sel = { { 4, 1 }, { 4, 2 }, { 4, 0 }, { 4, 3 } },
	.irq_shift = 0,
};

static const struct peri_llc_handle hi1382_peri_llc_handle = {
	.hi_edac_llc_init = hi1382_edac_llc_init,
	.hi_edac_llc_irq = hi1382_edac_llc_irq,
	.mod_sel = { { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 } },
	.irq_shift = 13,
};

static const struct of_device_id llc_edac_of_match[] = {
	/* for 1610 and 1612 toterm llc */
	{
		.compatible = "hisilicon,llc-edac",
		.data = &hip05_peri_llc_handle
	},
	/* for 1616 and 1382 toterm llc */
	{
		.compatible = "hisilicon,hi1382-llc-edac",
		.data = &hi1382_peri_llc_handle
	},
	{},
};

MODULE_DEVICE_TABLE(of, llc_edac_of_match);

static int hi_llc_probe(struct platform_device *pdev)
{
	struct llc_edac_dev *llc_dev;
	struct device_node *edac_dev_node;
	const struct of_device_id *of_id;
	unsigned int count;
	int rc, i;

	of_id = of_match_device(llc_edac_of_match, &pdev->dev);
	if (!of_id)
		return -EINVAL;

	llc_dev = devm_kzalloc(&pdev->dev, sizeof(*llc_dev), GFP_KERNEL);
	if (!llc_dev)
		return -ENOMEM;

	llc_dev->dev = &pdev->dev;
	platform_set_drvdata(pdev, llc_dev);

	edac_dev_node = pdev->dev.of_node;

	llc_dev->llc_handle = of_id->data;

	llc_dev->djtag_node = of_parse_phandle(edac_dev_node, "djtag", 0);
	if (!llc_dev->djtag_node) {
		dev_err(&pdev->dev, "llc_dev djtag node is null\n");
		return -EFAULT;
	}
	spin_lock_init(&llc_dev->llc_irq_lock);

	llc_dev->llc_handle->hi_edac_llc_init(llc_dev);

	for (count = 0; count < LLC_IRQ_NUM; count++) {
		llc_dev->irq[count] = platform_get_irq(pdev,
			count + llc_dev->llc_handle->irq_shift);
		llc_dev->llcisr[count].irqnum = count;
		llc_dev->llcisr[count].llcdev = llc_dev;
		if (llc_dev->irq[count] < 0) {
			dev_err(&pdev->dev, "No IRQ resource\n");
			rc = -EINVAL;
			goto out_err;
		}

		rc = devm_request_irq(&pdev->dev, llc_dev->irq[count],
				      llc_dev->llc_handle->hi_edac_llc_irq,
				      0, dev_name(&pdev->dev),
				      &llc_dev->llcisr[count]);
		if (rc) {
			dev_err(&pdev->dev, "irq count:%d req failed\n", count);
			goto out_err;
		}
	}

	return 0;

out_err:
	for (i = 0; i < count; i++)
		devm_free_irq(&pdev->dev, llc_dev->irq[i],
			      &llc_dev->llcisr[i]);
	return rc;
}

static int hi_llc_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver llc_edac_driver = {
	.probe = hi_llc_probe,
	.remove = hi_llc_remove,
	.driver = {
		.name = "llc-edac",
		.owner = THIS_MODULE,
		.of_match_table = llc_edac_of_match,
	},
};

module_platform_driver(llc_edac_driver);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("HISILICON EDAC driver");
MODULE_AUTHOR("Fengying Wang <fy.wang@huawei.com>");
MODULE_AUTHOR("Peter Chen <luck.chen@huawei.com>");

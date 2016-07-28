/*
 * Driver for Hisilicon Djtag r/w via System Controller.
 *
 * Copyright (C) 2016 Hisilicon Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/bitops.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include <asm-generic/delay.h>
#include <linux/soc/hisilicon/djtag.h>

#define SC_DJTAG_TIMEOUT		100000	/* 100ms */

/* for djtag v1 */
#define SC_DJTAG_MSTR_EN		0x6800
#define DJTAG_NOR_CFG			BIT(1)	/* accelerate R,W */
#define DJTAG_MSTR_EN			BIT(0)
#define SC_DJTAG_MSTR_START_EN		0x6804
#define DJTAG_MSTR_START_EN		0x1
#define SC_DJTAG_DEBUG_MODULE_SEL	0x680c
#define SC_DJTAG_MSTR_WR		0x6810
#define DJTAG_MSTR_W			0x1
#define DJTAG_MSTR_R			0x0
#define SC_DJTAG_CHAIN_UNIT_CFG_EN	0x6814
#define CHAIN_UNIT_CFG_EN		0xFFFF
#define SC_DJTAG_MSTR_ADDR		0x6818
#define SC_DJTAG_MSTR_DATA		0x681c
#define SC_DJTAG_RD_DATA_BASE		0xe800

/* for djtag v2 */
#define SC_DJTAG_SEC_ACC_EN_EX		0xd800
#define DJTAG_SEC_ACC_EN_EX		0x1
#define SC_DJTAG_MSTR_CFG_EX		0xd818
#define DJTAG_MSTR_RW_SHIFT_EX		29
#define DJTAG_MSTR_RD_EX		(0x0 << DJTAG_MSTR_RW_SHIFT_EX)
#define DJTAG_MSTR_WR_EX		(0x1 << DJTAG_MSTR_RW_SHIFT_EX)
#define DEBUG_MODULE_SEL_SHIFT_EX	16
#define CHAIN_UNIT_CFG_EN_EX		0xFFFF
#define SC_DJTAG_MSTR_ADDR_EX		0xd810
#define SC_DJTAG_MSTR_DATA_EX		0xd814
#define SC_DJTAG_MSTR_START_EN_EX	0xd81c
#define DJTAG_MSTR_START_EN_EX		0x1
#define SC_DJTAG_RD_DATA_BASE_EX	0xe800
#define SC_DJTAG_OP_ST_EX		0xe828
#define DJTAG_OP_DONE_EX		BIT(8)

static LIST_HEAD(djtag_list);

struct djtag_data {
	spinlock_t lock;
	struct list_head list;
	struct regmap *scl_map;
	struct device_node *node;
	int (*djtag_readwrite)(struct regmap *map, u32 offset,
			u32 mod_sel, u32 mod_mask, bool is_w,
			u32 wval, int chain_id, u32 *rval);
};

/*
 * djtag_readwrite_v1/v2: djtag read/write interface
 * @regmap:	djtag base address
 * @offset:	register's offset
 * @mod_sel:	module selection
 * @mod_mask:	mask to select specific modules for write
 * @is_w:	write -> true, read -> false
 * @wval:	value to register for write
 * @chain_id:	which sub module for read
 * @rval:	value in register for read
 *
 * Return non-zero if error, else return 0.
 */
static int djtag_readwrite_v1(struct regmap *map, u32 offset, u32 mod_sel,
		u32 mod_mask, bool is_w, u32 wval, int chain_id, u32 *rval)
{
	u32 rd;
	int timeout = SC_DJTAG_TIMEOUT;

	if (!(mod_mask & CHAIN_UNIT_CFG_EN)) {
		pr_warn("djtag: do nothing.\n");
		return 0;
	}

	/* djtag mster enable & accelerate R,W */
	regmap_write(map, SC_DJTAG_MSTR_EN, DJTAG_NOR_CFG | DJTAG_MSTR_EN);

	/* select module */
	regmap_write(map, SC_DJTAG_DEBUG_MODULE_SEL, mod_sel);

	regmap_write(map, SC_DJTAG_CHAIN_UNIT_CFG_EN,
			mod_mask & CHAIN_UNIT_CFG_EN);

	if (is_w) {
		regmap_write(map, SC_DJTAG_MSTR_WR, DJTAG_MSTR_W);
		regmap_write(map, SC_DJTAG_MSTR_DATA, wval);
	} else
		regmap_write(map, SC_DJTAG_MSTR_WR, DJTAG_MSTR_R);

	/* address offset */
	regmap_write(map, SC_DJTAG_MSTR_ADDR, offset);

	/* start to write to djtag register */
	regmap_write(map, SC_DJTAG_MSTR_START_EN, DJTAG_MSTR_START_EN);

	/* ensure the djtag operation is done */
	do {
		regmap_read(map, SC_DJTAG_MSTR_START_EN, &rd);

		if (!(rd & DJTAG_MSTR_EN))
			break;

		udelay(1);
	} while (timeout--);

	if (timeout < 0) {
		pr_err("djtag: %s timeout!\n", is_w ? "write" : "read");
		return -EBUSY;
	}

	if (!is_w)
		regmap_read(map, SC_DJTAG_RD_DATA_BASE + chain_id * 0x4, rval);

	return 0;
}

static int djtag_readwrite_v2(struct regmap *map, u32 offset, u32 mod_sel,
		u32 mod_mask, bool is_w, u32 wval, int chain_id, u32 *rval)
{
	u32 rd;
	int timeout = SC_DJTAG_TIMEOUT;

	if (!(mod_mask & CHAIN_UNIT_CFG_EN_EX)) {
		pr_warn("djtag: do nothing.\n");
		return 0;
	}

	/* djtag mster enable */
	regmap_write(map, SC_DJTAG_SEC_ACC_EN_EX, DJTAG_SEC_ACC_EN_EX);

	if (is_w) {
		regmap_write(map, SC_DJTAG_MSTR_CFG_EX, DJTAG_MSTR_WR_EX
				| (mod_sel << DEBUG_MODULE_SEL_SHIFT_EX)
				| (mod_mask & CHAIN_UNIT_CFG_EN_EX));
		regmap_write(map, SC_DJTAG_MSTR_DATA_EX, wval);
	} else
		regmap_write(map, SC_DJTAG_MSTR_CFG_EX, DJTAG_MSTR_RD_EX
				| (mod_sel << DEBUG_MODULE_SEL_SHIFT_EX)
				| (mod_mask & CHAIN_UNIT_CFG_EN_EX));

	/* address offset */
	regmap_write(map, SC_DJTAG_MSTR_ADDR_EX, offset);

	/* start to write to djtag register */
	regmap_write(map, SC_DJTAG_MSTR_START_EN_EX, DJTAG_MSTR_START_EN_EX);

	/* ensure the djtag operation is done */
	do {
		regmap_read(map, SC_DJTAG_MSTR_START_EN_EX, &rd);

		if (!(rd & DJTAG_MSTR_START_EN_EX))
			break;

		udelay(1);
	} while (timeout--);

	if (timeout < 0)
		goto timeout;

	timeout = SC_DJTAG_TIMEOUT;
	do {
		regmap_read(map, SC_DJTAG_OP_ST_EX, &rd);

		if (rd & DJTAG_OP_DONE_EX)
			break;

		udelay(1);
	} while (timeout--);

	if (timeout < 0)
		goto timeout;

	if (!is_w)
		regmap_read(map, SC_DJTAG_RD_DATA_BASE_EX + chain_id * 0x4,
				rval);

	return 0;

timeout:
	pr_err("djtag: %s timeout!\n", is_w ? "write" : "read");
	return -EBUSY;
}


/**
 * djtag_writel - write registers via djtag
 * @node:	djtag node
 * @offset:	register's offset
 * @mod_sel:	module selection
 * @mod_mask:	mask to select specific modules
 * @val:	value to write to register
 *
 * If error return errno, otherwise return 0.
 */
int hisi_djtag_writel(struct device_node *node, u32 offset, u32 mod_sel,
			u32 mod_mask, u32 val)
{
	struct regmap *map;
	unsigned long flags;
	struct djtag_data *tmp, *p;
	int ret = 0;

	map = NULL;
	list_for_each_entry_safe(tmp, p, &djtag_list, list) {
		if (tmp->node == node) {
			map = tmp->scl_map;

			spin_lock_irqsave(&tmp->lock, flags);
			ret = tmp->djtag_readwrite(map, offset, mod_sel, mod_mask,
					true, val, 0, NULL);
			if (ret)
				pr_err("djtag_writel: %s: error!\n",
						node->full_name);
			spin_unlock_irqrestore(&tmp->lock, flags);
			break;
		}
	}

	if (!map)
		return -ENODEV;

	return ret;
}
EXPORT_SYMBOL_GPL(hisi_djtag_writel);

/**
 * djtag_readl - read registers via djtag
 * @node:	djtag node
 * @offset:	register's offset
 * @mod_sel:	module type selection
 * @chain_id:	chain_id number, mostly is 0
 * @val:	register's value
 *
 * If error return errno, otherwise return 0.
 */
int hisi_djtag_readl(struct device_node *node, u32 offset, u32 mod_sel,
		int chain_id, u32 *val)
{
	struct regmap *map;
	unsigned long flags;
	struct djtag_data *tmp, *p;
	int ret = 0;

	map = NULL;
	list_for_each_entry_safe(tmp, p, &djtag_list, list) {
		if (tmp->node == node) {
			map = tmp->scl_map;

			spin_lock_irqsave(&tmp->lock, flags);
			ret = tmp->djtag_readwrite(map, offset, mod_sel,
					0xffff, false, 0, chain_id, val);
			if (ret)
				pr_err("djtag_readl: %s: error!\n",
						node->full_name);
			spin_unlock_irqrestore(&tmp->lock, flags);
			break;
		}
	}

	if (!map)
		return -ENODEV;

	return ret;
}
EXPORT_SYMBOL_GPL(hisi_djtag_readl);

static const struct of_device_id djtag_of_match[] = {
	/* for hip05(D02) cpu die */
	{ .compatible = "hisilicon,hip05-cpu-djtag-v1",
		.data = (void *)djtag_readwrite_v1 },
	/* for hip05(D02) io die */
	{ .compatible = "hisilicon,hip05-io-djtag-v1",
		.data = (void *)djtag_readwrite_v1 },
	/* for hip06(D03) cpu die */
	{ .compatible = "hisilicon,hip06-cpu-djtag-v1",
		.data = (void *)djtag_readwrite_v1 },
	/* for hip06(D03) io die */
	{ .compatible = "hisilicon,hip06-io-djtag-v2",
		.data = (void *)djtag_readwrite_v2 },
	/* for hip07(D05) cpu die */
	{ .compatible = "hisilicon,hip07-cpu-djtag-v2",
		.data = (void *)djtag_readwrite_v2 },
	/* for hip07(D05) io die */
	{ .compatible = "hisilicon,hip07-io-djtag-v2",
		.data = (void *)djtag_readwrite_v2 },
	{},
};

MODULE_DEVICE_TABLE(of, djtag_of_match);

static int djtag_dev_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct djtag_data *dg_data;
	const struct of_device_id *of_id;

	of_id = of_match_device(djtag_of_match, dev);
	if (!of_id)
		return -EINVAL;

	dg_data = kzalloc(sizeof(struct djtag_data), GFP_KERNEL);
	if (!dg_data)
		return -ENOMEM;

	dg_data->node = dev->of_node;
	dg_data->djtag_readwrite = of_id->data;
	spin_lock_init(&dg_data->lock);

	INIT_LIST_HEAD(&dg_data->list);
	dg_data->scl_map = syscon_regmap_lookup_by_phandle(dg_data->node,
			"syscon");
	if (IS_ERR(dg_data->scl_map)) {
		dev_warn(dev, "wrong syscon register address.\n");
		kfree(dg_data);
		return -EINVAL;
	}

	list_add_tail(&dg_data->list, &djtag_list);
	dev_info(dev, "%s init successfully.\n", dg_data->node->name);
	return 0;
}

static int djtag_dev_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct djtag_data *tmp, *p;

	list_for_each_entry_safe(tmp, p, &djtag_list, list) {
		list_del(&tmp->list);
		dev_info(dev, "%s remove successfully.\n", tmp->node->name);
		kfree(tmp);
	}

	return 0;
}

static struct platform_driver djtag_dev_driver = {
	.driver = {
		.name = "hisi-djtag",
		.of_match_table = djtag_of_match,
	},
	.probe = djtag_dev_probe,
	.remove = djtag_dev_remove,
};

module_platform_driver(djtag_dev_driver);

MODULE_DESCRIPTION("Hisilicon djtag driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

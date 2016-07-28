/*
 * Driver for Hisilicon Djtag r/w which use CPU sysctrl.
 *
 * Copyright (C) 2016 Hisilicon Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/time64.h>

#include "djtag.h"

#define SC_DJTAG_TIMEOUT_US    (100 * USEC_PER_MSEC) /* 100ms */

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

#define DJTAG_PREFIX "hisi-djtag-dev-"

static DEFINE_IDR(djtag_hosts_idr);
static DEFINE_IDR(djtag_clients_idr);

struct hisi_djtag_ops {
	int (*djtag_read)(void __iomem *regs_base, u32 offset,
			  u32 mod_sel, u32 mod_mask, int chain_id, u32 *rval);
	int (*djtag_write)(void __iomem *regs_base, u32 offset,
			   u32 mod_sel, u32 mod_mask, u32 wval, int chain_id);
};

struct hisi_djtag_host {
	spinlock_t lock;
	int id;
	u32 scl_id;
	struct device dev;
	struct list_head client_list;
	void __iomem *sysctl_reg_map;
	struct device_node *of_node;
	const struct hisi_djtag_ops *djtag_ops;
};

#define to_hisi_djtag_client(d) container_of(d, struct hisi_djtag_client, dev)
#define to_hisi_djtag_driver(d) container_of(d, struct hisi_djtag_driver, \
					     driver)
#define MODULE_PREFIX "hisi_djtag:"

static void djtag_prepare_v1(void __iomem *regs_base, u32 offset,
			     u32 mod_sel, u32 mod_mask)
{
	/* djtag master enable & accelerate R,W */
	writel(DJTAG_NOR_CFG | DJTAG_MSTR_EN, regs_base + SC_DJTAG_MSTR_EN);

	/* select module */
	writel(mod_sel, regs_base + SC_DJTAG_DEBUG_MODULE_SEL);
	writel(mod_mask & CHAIN_UNIT_CFG_EN,
			regs_base + SC_DJTAG_CHAIN_UNIT_CFG_EN);

	/* address offset */
	writel(offset, regs_base + SC_DJTAG_MSTR_ADDR);
}

static int djtag_do_operation_v1(void __iomem *regs_base)
{
	u32 rd;
	int timeout = SC_DJTAG_TIMEOUT_US;

	/* start to write to djtag register */
	writel(DJTAG_MSTR_START_EN, regs_base + SC_DJTAG_MSTR_START_EN);

	/* ensure the djtag operation is done */
	do {
		rd = readl(regs_base + SC_DJTAG_MSTR_START_EN);
		if (!(rd & DJTAG_MSTR_EN))
			break;

		udelay(1);
	} while (timeout--);

	if (timeout < 0)
		return -EBUSY;

	return 0;
}

static void djtag_prepare_v2(void __iomem *regs_base, u32 offset,
			     u32 mod_sel, u32 mod_mask)
{
	/* djtag mster enable */
	writel(DJTAG_SEC_ACC_EN_EX, regs_base + SC_DJTAG_SEC_ACC_EN_EX);

	/* address offset */
	writel(offset, regs_base + SC_DJTAG_MSTR_ADDR_EX);
}

static int djtag_do_operation_v2(void __iomem *regs_base)
{
	u32 rd;
	int timeout = SC_DJTAG_TIMEOUT_US;

	/* start to write to djtag register */
	writel(DJTAG_MSTR_START_EN_EX, regs_base + SC_DJTAG_MSTR_START_EN_EX);

	/* ensure the djtag operation is done */
	do {
		rd = readl(regs_base + SC_DJTAG_MSTR_START_EN_EX);

		if (!(rd & DJTAG_MSTR_START_EN_EX))
			break;

		udelay(1);
	} while (timeout--);

	if (timeout < 0)
		goto timeout;

	timeout = SC_DJTAG_TIMEOUT_US;
	do {
		rd = readl(regs_base + SC_DJTAG_OP_ST_EX);

		if (rd & DJTAG_OP_DONE_EX)
			break;

		udelay(1);
	} while (timeout--);

	if (timeout < 0)
		goto timeout;

	return 0;

timeout:
	return -EBUSY;
}

/*
 * djtag_read_v1/v2: djtag read interface
 * @reg_base:	djtag register base address
 * @offset:	register's offset
 * @mod_sel:	module selection
 * @mod_mask:	mask to select specific modules for write
 * @chain_id:	which sub module for read
 * @rval:	value in register for read
 *
 * Return non-zero if error, else return 0.
 */
static int djtag_read_v1(void __iomem *regs_base, u32 offset, u32 mod_sel,
			 u32 mod_mask, int chain_id, u32 *rval)
{
	int ret;

	if (!(mod_mask & CHAIN_UNIT_CFG_EN)) {
		pr_warn("djtag: do nothing.\n");
		return 0;
	}

	djtag_prepare_v1(regs_base, offset, mod_sel, mod_mask);

	writel(DJTAG_MSTR_R, regs_base + SC_DJTAG_MSTR_WR);

	ret = djtag_do_operation_v1(regs_base);
	if (ret) {
		if (ret == EBUSY)
			pr_err("djtag: %s timeout!\n", "read");
		return ret;
	}

	*rval = readl(regs_base + SC_DJTAG_RD_DATA_BASE + chain_id * 0x4);

	return 0;
}

static int djtag_read_v2(void __iomem *regs_base, u32 offset, u32 mod_sel,
			 u32 mod_mask, int chain_id, u32 *rval)
{
	int ret;

	if (!(mod_mask & CHAIN_UNIT_CFG_EN_EX)) {
		pr_warn("djtag: do nothing.\n");
		return 0;
	}

	djtag_prepare_v2(regs_base, offset, mod_sel, mod_mask);

	writel(DJTAG_MSTR_RD_EX
			| (mod_sel << DEBUG_MODULE_SEL_SHIFT_EX)
			| (mod_mask & CHAIN_UNIT_CFG_EN_EX),
			regs_base + SC_DJTAG_MSTR_CFG_EX);

	ret = djtag_do_operation_v2(regs_base);
	if (ret) {
		if (ret == EBUSY)
			pr_err("djtag: %s timeout!\n", "read");
		return ret;
	}

	*rval = readl(regs_base + SC_DJTAG_RD_DATA_BASE_EX +
					      chain_id * 0x4);

	return 0;
}

/*
 * djtag_write_v1/v2: djtag write interface
 * @reg_base:	djtag register base address
 * @offset:	register's offset
 * @mod_sel:	module selection
 * @mod_mask:	mask to select specific modules for write
 * @wval:	value to register for write
 * @chain_id:	which sub module for read
 *
 * Return non-zero if error, else return 0.
 */
static int djtag_write_v1(void __iomem *regs_base, u32 offset, u32 mod_sel,
			  u32 mod_mask, u32 wval, int chain_id)
{
	int ret;

	if (!(mod_mask & CHAIN_UNIT_CFG_EN)) {
		pr_warn("djtag: do nothing.\n");
		return 0;
	}

	djtag_prepare_v1(regs_base, offset, mod_sel, mod_mask);

	writel(DJTAG_MSTR_W, regs_base + SC_DJTAG_MSTR_WR);
	writel(wval, regs_base + SC_DJTAG_MSTR_DATA);

	ret = djtag_do_operation_v1(regs_base);
	if (ret) {
		if (ret == EBUSY)
			pr_err("djtag: %s timeout!\n", "write");
		return ret;
	}

	return 0;
}

static int djtag_write_v2(void __iomem *regs_base, u32 offset, u32 mod_sel,
			  u32 mod_mask, u32 wval, int chain_id)
{
	int ret;

	if (!(mod_mask & CHAIN_UNIT_CFG_EN_EX)) {
		pr_warn("djtag: do nothing.\n");
		return 0;
	}

	djtag_prepare_v2(regs_base, offset, mod_sel, mod_mask);

	writel(DJTAG_MSTR_WR_EX
			| (mod_sel << DEBUG_MODULE_SEL_SHIFT_EX)
			| (mod_mask & CHAIN_UNIT_CFG_EN_EX),
			regs_base + SC_DJTAG_MSTR_CFG_EX);
	writel(wval, regs_base + SC_DJTAG_MSTR_DATA_EX);

	ret = djtag_do_operation_v2(regs_base);
	if (ret) {
		if (ret == EBUSY)
			pr_err("djtag: %s timeout!\n", "write");
		return ret;
	}

	return 0;
}

/**
 * djtag_writel - write registers via djtag
 * @client: djtag client handle
 * @offset:	register's offset
 * @mod_sel:	module selection
 * @mod_mask:	mask to select specific modules
 * @val:	value to write to register
 *
 * If error return errno, otherwise return 0.
 */
int hisi_djtag_writel(struct hisi_djtag_client *client, u32 offset,
		      u32 mod_sel, u32 mod_mask, u32 val)
{
	void __iomem *reg_map = client->host->sysctl_reg_map;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&client->host->lock, flags);
	ret = client->host->djtag_ops->djtag_write(reg_map, offset, mod_sel,
						   mod_mask, val, 0);
	if (ret)
		pr_err("djtag_writel: error! ret=%d\n", ret);
	spin_unlock_irqrestore(&client->host->lock, flags);

	return ret;
}

/**
 * djtag_readl - read registers via djtag
 * @client: djtag client handle
 * @offset:	register's offset
 * @mod_sel:	module type selection
 * @chain_id:	chain_id number, mostly is 0
 * @val:	register's value
 *
 * If error return errno, otherwise return 0.
 */
int hisi_djtag_readl(struct hisi_djtag_client *client, u32 offset,
		     u32 mod_sel, int chain_id, u32 *val)
{
	void __iomem *reg_map = client->host->sysctl_reg_map;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&client->host->lock, flags);
	ret = client->host->djtag_ops->djtag_read(reg_map, offset, mod_sel,
						  0xffff, chain_id, val);
	if (ret)
		pr_err("djtag_readl: error! ret=%d\n", ret);
	spin_unlock_irqrestore(&client->host->lock, flags);

	return ret;
}

u32 hisi_djtag_get_sclid(struct hisi_djtag_client *client)
{
	return client->host->scl_id;
}

static const struct hisi_djtag_ops djtag_v1_ops = {
	.djtag_read  = djtag_read_v1,
	.djtag_write  = djtag_write_v1,
};

static const struct hisi_djtag_ops djtag_v2_ops = {
	.djtag_read  = djtag_read_v2,
	.djtag_write  = djtag_write_v2,
};

static const struct of_device_id djtag_of_match[] = {
	/* for hip05 CPU die */
	{ .compatible = "hisilicon,hip05-cpu-djtag-v1",
		.data = &djtag_v1_ops },
	/* for hip05 IO die */
	{ .compatible = "hisilicon,hip05-io-djtag-v1",
		.data = &djtag_v1_ops },
	/* for hip06 CPU die */
	{ .compatible = "hisilicon,hip06-cpu-djtag-v1",
		.data = &djtag_v1_ops },
	/* for hip06 IO die */
	{ .compatible = "hisilicon,hip06-io-djtag-v2",
		.data = &djtag_v2_ops },
	/* for hip07 CPU die */
	{ .compatible = "hisilicon,hip07-cpu-djtag-v2",
		.data = &djtag_v2_ops },
	/* for hip07 IO die */
	{ .compatible = "hisilicon,hip07-io-djtag-v2",
		.data = &djtag_v2_ops },
	{},
};
MODULE_DEVICE_TABLE(of, djtag_of_match);

static ssize_t show_modalias(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct hisi_djtag_client *client = to_hisi_djtag_client(dev);

	return sprintf(buf, "%s%s\n", MODULE_PREFIX, client->name);
}
static DEVICE_ATTR(modalias, 0444, show_modalias, NULL);

static struct attribute *hisi_djtag_dev_attrs[] = {
	NULL,
	/* modalias helps coldplug:  modprobe $(cat .../modalias) */
	&dev_attr_modalias.attr,
	NULL
};
ATTRIBUTE_GROUPS(hisi_djtag_dev);

static struct device_type hisi_djtag_client_type = {
	.groups		= hisi_djtag_dev_groups,
};

static struct hisi_djtag_client *hisi_djtag_verify_client(struct device *dev)
{
	return (dev->type == &hisi_djtag_client_type)
			? to_hisi_djtag_client(dev)
			: NULL;
}

static int hisi_djtag_device_probe(struct device *dev)
{
	struct hisi_djtag_driver *driver;
	struct hisi_djtag_client *client;
	int rc;

	client = hisi_djtag_verify_client(dev);
	if (!client) {
		dev_err(dev, "could not find client\n");
		return -ENODEV;
	}

	driver = to_hisi_djtag_driver(dev->driver);
	if (!driver) {
		dev_err(dev, "could not find driver\n");
		return -ENODEV;
	}

	rc = driver->probe(client);
	if (rc < 0) {
		dev_err(dev, "client probe failed\n");
		return rc;
	}

	return 0;
}

static int hisi_djtag_device_remove(struct device *dev)
{
	struct hisi_djtag_driver *driver;
	struct hisi_djtag_client *client;
	int rc;

	client = hisi_djtag_verify_client(dev);
	if (!client) {
		dev_err(dev, "could not find client\n");
		return -ENODEV;
	}

	driver = to_hisi_djtag_driver(dev->driver);
	if (!driver) {
		dev_err(dev, "could not find driver\n");
		return -ENODEV;
	}

	rc = driver->remove(client);
	if (rc < 0) {
		dev_err(dev, "client probe failed\n");
		return rc;
	}

	return 0;
}

static int hisi_djtag_device_match(struct device *dev,
				   struct device_driver *drv)
{
	struct hisi_djtag_client *client = hisi_djtag_verify_client(dev);

	if (!client)
		return false;

	if (of_driver_match_device(dev, drv))
		return true;

	return false;
}

struct bus_type hisi_djtag_bus = {
	.name		= "hisi-djtag",
	.match		= hisi_djtag_device_match,
	.probe		= hisi_djtag_device_probe,
	.remove		= hisi_djtag_device_remove,
};

static struct hisi_djtag_client *
		hisi_djtag_client_alloc(struct hisi_djtag_host *host)
{
	struct hisi_djtag_client *client;

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (!client)
		return NULL;

	client->host = host;

	client->dev.parent = &client->host->dev;
	client->dev.bus = &hisi_djtag_bus;
	client->dev.type = &hisi_djtag_client_type;

	return client;
}

static int hisi_djtag_get_client_id(struct hisi_djtag_client *client)
{
	return idr_alloc(&djtag_clients_idr, client, 0, 0, GFP_KERNEL);
}

static int hisi_djtag_set_client_name(struct hisi_djtag_client *client,
				      const char *device_name)
{
	int id;

	id = hisi_djtag_get_client_id(client);
	if (id < 0)
		return id;

	client->id = id;

	snprintf(client->name, DJTAG_CLIENT_NAME_LEN, "%s%s_%d",
		 DJTAG_PREFIX, device_name, client->id);
	dev_set_name(&client->dev, "%s", client->name);

	return 0;
}

static int hisi_djtag_new_of_device(struct hisi_djtag_host *host,
				    struct device_node *node)
{
	struct hisi_djtag_client *client;
	int rc;

	client = hisi_djtag_client_alloc(host);
	if (!client) {
		dev_err(&host->dev, "DT: Client alloc fail!\n");
		return -ENOMEM;
	}

	client->dev.of_node = of_node_get(node);

	rc = hisi_djtag_set_client_name(client, node->name);
	if (rc < 0) {
		dev_err(&host->dev, "DT: Client set name fail!\n");
		goto fail;
	}

	rc = device_register(&client->dev);
	if (rc < 0) {
		dev_err(&client->dev,
			"DT: error adding new device, rc=%d\n", rc);
		idr_remove(&djtag_clients_idr, client->id);
		goto fail;
	}

	list_add(&client->next, &host->client_list);

	return 0;
fail:
	of_node_put(client->dev.of_node);
	kfree(client);
	return rc;
}

static void djtag_register_devices(struct hisi_djtag_host *host)
{
	struct device_node *node;

	if (host->of_node) {
		for_each_available_child_of_node(host->of_node, node) {
			if (of_node_test_and_set_flag(node, OF_POPULATED))
				continue;
			if (hisi_djtag_new_of_device(host, node))
				break;
		}
	}
}

static int hisi_djtag_add_host(struct hisi_djtag_host *host)
{
	int rc;

	host->dev.bus = &hisi_djtag_bus;

	rc = idr_alloc(&djtag_hosts_idr, host, 0, 0, GFP_KERNEL);
	if (rc < 0) {
		dev_err(&host->dev, "No available djtag host ID'!s\n");
		return rc;
	}
	host->id = rc;

	/* Suffix the unique ID and set djtag hostname */
	dev_set_name(&host->dev, "djtag-host-%d", host->id);
	rc = device_register(&host->dev);
	if (rc < 0) {
		dev_err(&host->dev,
			"add_host dev register failed, rc=%d\n", rc);
		idr_remove(&djtag_hosts_idr, host->id);
		return rc;
	}

	djtag_register_devices(host);

	return 0;
}

static int djtag_host_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hisi_djtag_host *host;
	struct resource *res;
	int rc;

	host = kzalloc(sizeof(*host), GFP_KERNEL);
	if (!host)
		return -ENOMEM;

	if (dev->of_node) {
		const struct of_device_id *of_id;

		of_id = of_match_device(djtag_of_match, dev);
		if (!of_id) {
			rc = -EINVAL;
			goto fail;
		}

		host->djtag_ops = of_id->data;
		host->of_node = of_node_get(dev->of_node);
	} else {
		rc = -EINVAL;
		goto fail;
	}

	/* Find the SCL ID */
	rc = device_property_read_u32(dev, "hisilicon,scl-id", &host->scl_id);
	if (rc < 0)
		goto fail;

	spin_lock_init(&host->lock);
	INIT_LIST_HEAD(&host->client_list);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "No reg resorces!\n");
		rc = -EINVAL;
		goto fail;
	}

	if (!resource_size(res)) {
		dev_err(dev, "Zero reg entry!\n");
		rc = -EINVAL;
		goto fail;
	}

	host->sysctl_reg_map = devm_ioremap_resource(dev, res);
	if (IS_ERR(host->sysctl_reg_map)) {
		dev_err(dev, "Unable to map sysctl registers!\n");
		rc = -EINVAL;
		goto fail;
	}

	platform_set_drvdata(pdev, host);

	rc = hisi_djtag_add_host(host);
	if (rc) {
		dev_err(dev, "add host failed, rc=%d\n", rc);
		goto fail;
	}

	return 0;
fail:
	of_node_put(dev->of_node);
	kfree(host);
	return rc;
}

static int djtag_host_remove(struct platform_device *pdev)
{
	struct hisi_djtag_host *host;
	struct hisi_djtag_client *client, *tmp;
	struct list_head *client_list;

	host = platform_get_drvdata(pdev);
	client_list = &host->client_list;

	list_for_each_entry_safe(client, tmp, client_list, next) {
		list_del(&client->next);
		device_unregister(&client->dev);
		idr_remove(&djtag_clients_idr, client->id);
		of_node_put(client->dev.of_node);
		kfree(client);
	}

	device_unregister(&host->dev);
	idr_remove(&djtag_hosts_idr, host->id);
	of_node_put(host->of_node);
	kfree(host);

	return 0;
}

static struct platform_driver djtag_dev_driver = {
	.driver = {
		.name = "hisi-djtag",
		.of_match_table = djtag_of_match,
	},
	.probe = djtag_host_probe,
	.remove = djtag_host_remove,
};
module_platform_driver(djtag_dev_driver);

int hisi_djtag_register_driver(struct module *owner,
			       struct hisi_djtag_driver *driver)
{
	int rc;

	driver->driver.owner = owner;
	driver->driver.bus = &hisi_djtag_bus;

	rc = driver_register(&driver->driver);
	if (rc < 0)
		pr_err("%s register failed, rc=%d\n", __func__, rc);

	return rc;
}

void hisi_djtag_unregister_driver(struct hisi_djtag_driver *driver)
{
	driver->driver.bus = &hisi_djtag_bus;
	driver_unregister(&driver->driver);
}

static int __init hisi_djtag_init(void)
{
	int rc;

	rc = bus_register(&hisi_djtag_bus);
	if (rc) {
		pr_err("hisi  djtag init failed, rc=%d\n", rc);
		return rc;
	}

	return 0;
}
module_init(hisi_djtag_init);

static void __exit hisi_djtag_exit(void)
{
	bus_unregister(&hisi_djtag_bus);
}
module_exit(hisi_djtag_exit);

MODULE_DESCRIPTION("Hisilicon djtag driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

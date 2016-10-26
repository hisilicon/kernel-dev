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
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
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

#define DJTAG_PREFIX "hisi-djtag-dev-"

DEFINE_IDR(djtag_hosts_idr);

struct hisi_djtag_host {
	spinlock_t lock;
	int dev_id;
	struct device dev;
	struct list_head client_list;
	void __iomem *sysctl_reg_map;
	struct device_node *of_node;
	int (*djtag_readwrite)(void __iomem *regs_base, u32 offset,
			u32 mod_sel, u32 mod_mask, bool is_w,
			u32 wval, int chain_id, u32 *rval);
};

#define to_hisi_djtag_client(d) container_of(d, struct hisi_djtag_client, dev)
#define to_hisi_djtag_driver(d) container_of(d, struct hisi_djtag_driver, \
								 driver)
#define MODULE_PREFIX "hisi_djtag:"

static void djtag_read32_relaxed(void __iomem *regs_base, u32 off, u32 *value)
{
	void __iomem *reg_addr = regs_base + off;

	*value = readl_relaxed(reg_addr);
}

static void djtag_write32(void __iomem *regs_base, u32 off, u32 val)
{
	void __iomem *reg_addr = regs_base + off;

	writel(val, reg_addr);
}

/*
 * djtag_readwrite_v1/v2: djtag read/write interface
 * @reg_base:	djtag register base address
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
static int djtag_readwrite_v1(void __iomem *regs_base, u32 offset, u32 mod_sel,
		u32 mod_mask, bool is_w, u32 wval, int chain_id, u32 *rval)
{
	u32 rd;
	int timeout = SC_DJTAG_TIMEOUT;

	if (!(mod_mask & CHAIN_UNIT_CFG_EN)) {
		pr_warn("djtag: do nothing.\n");
		return 0;
	}

	/* djtag mster enable & accelerate R,W */
	djtag_write32(regs_base, SC_DJTAG_MSTR_EN,
			DJTAG_NOR_CFG | DJTAG_MSTR_EN);

	/* select module */
	djtag_write32(regs_base, SC_DJTAG_DEBUG_MODULE_SEL, mod_sel);
	djtag_write32(regs_base, SC_DJTAG_CHAIN_UNIT_CFG_EN,
				mod_mask & CHAIN_UNIT_CFG_EN);

	if (is_w) {
		djtag_write32(regs_base, SC_DJTAG_MSTR_WR, DJTAG_MSTR_W);
		djtag_write32(regs_base, SC_DJTAG_MSTR_DATA, wval);
	} else
		djtag_write32(regs_base, SC_DJTAG_MSTR_WR, DJTAG_MSTR_R);

	/* address offset */
	djtag_write32(regs_base, SC_DJTAG_MSTR_ADDR, offset);

	/* start to write to djtag register */
	djtag_write32(regs_base, SC_DJTAG_MSTR_START_EN, DJTAG_MSTR_START_EN);

	/* ensure the djtag operation is done */
	do {
		djtag_read32_relaxed(regs_base, SC_DJTAG_MSTR_START_EN, &rd);
		if (!(rd & DJTAG_MSTR_EN))
			break;

		udelay(1);
	} while (timeout--);

	if (timeout < 0) {
		pr_err("djtag: %s timeout!\n", is_w ? "write" : "read");
		return -EBUSY;
	}

	if (!is_w)
		djtag_read32_relaxed(regs_base,
			SC_DJTAG_RD_DATA_BASE + chain_id * 0x4, rval);

	return 0;
}

static int djtag_readwrite_v2(void __iomem *regs_base, u32 offset, u32 mod_sel,
		u32 mod_mask, bool is_w, u32 wval, int chain_id, u32 *rval)
{
	u32 rd;
	int timeout = SC_DJTAG_TIMEOUT;

	if (!(mod_mask & CHAIN_UNIT_CFG_EN_EX)) {
		pr_warn("djtag: do nothing.\n");
		return 0;
	}

	/* djtag mster enable */
	djtag_write32(regs_base, SC_DJTAG_SEC_ACC_EN_EX, DJTAG_SEC_ACC_EN_EX);

	if (is_w) {
		djtag_write32(regs_base, SC_DJTAG_MSTR_CFG_EX, DJTAG_MSTR_WR_EX
				| (mod_sel << DEBUG_MODULE_SEL_SHIFT_EX)
				| (mod_mask & CHAIN_UNIT_CFG_EN_EX));
		djtag_write32(regs_base, SC_DJTAG_MSTR_DATA_EX, wval);
	} else
		djtag_write32(regs_base, SC_DJTAG_MSTR_CFG_EX, DJTAG_MSTR_RD_EX
				| (mod_sel << DEBUG_MODULE_SEL_SHIFT_EX)
				| (mod_mask & CHAIN_UNIT_CFG_EN_EX));

	/* address offset */
	djtag_write32(regs_base, SC_DJTAG_MSTR_ADDR_EX, offset);

	/* start to write to djtag register */
	djtag_write32(regs_base,
		      SC_DJTAG_MSTR_START_EN_EX, DJTAG_MSTR_START_EN_EX);

	/* ensure the djtag operation is done */
	do {
		djtag_read32_relaxed(regs_base, SC_DJTAG_MSTR_START_EN_EX, &rd);

		if (!(rd & DJTAG_MSTR_START_EN_EX))
			break;

		udelay(1);
	} while (timeout--);

	if (timeout < 0)
		goto timeout;

	timeout = SC_DJTAG_TIMEOUT;
	do {
		djtag_read32_relaxed(regs_base, SC_DJTAG_OP_ST_EX, &rd);

		if (rd & DJTAG_OP_DONE_EX)
			break;

		udelay(1);
	} while (timeout--);

	if (timeout < 0)
		goto timeout;

	if (!is_w)
		djtag_read32_relaxed(regs_base,
				     SC_DJTAG_RD_DATA_BASE_EX + chain_id * 0x4,
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
int hisi_djtag_writel(struct hisi_djtag_client *client, u32 offset, u32 mod_sel,
			u32 mod_mask, u32 val)
{
	void __iomem *reg_map = client->host->sysctl_reg_map;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&client->host->lock, flags);
	ret = client->host->djtag_readwrite(reg_map, offset, mod_sel, mod_mask,
					true, val, 0, NULL);
	if (ret)
		pr_err("djtag_writel: error! ret=%d\n", ret);
	spin_unlock_irqrestore(&client->host->lock, flags);

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
int hisi_djtag_readl(struct hisi_djtag_client *client, u32 offset, u32 mod_sel,
		int chain_id, u32 *val)
{
	void __iomem *reg_map = client->host->sysctl_reg_map;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&client->host->lock, flags);
	ret = client->host->djtag_readwrite(reg_map, offset, mod_sel,
			0xffff, false, 0, chain_id, val);
	if (ret)
		pr_err("djtag_readl: error! ret=%d\n", ret);
	spin_unlock_irqrestore(&client->host->lock, flags);

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

static ssize_t
show_modalias(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hisi_djtag_client *client = to_hisi_djtag_client(dev);

	return sprintf(buf, "%s%s\n", MODULE_PREFIX, client->name);
}
static DEVICE_ATTR(modalias, S_IRUGO, show_modalias, NULL);

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

struct hisi_djtag_client *hisi_djtag_verify_client(struct device *dev)
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
	const struct of_device_id *p;
	struct hisi_djtag_client *client = hisi_djtag_verify_client(dev);

	if (!client)
		return false;

	if (of_driver_match_device(dev, drv))
		return true;

	p = of_match_device(drv->of_match_table, dev);
	if (!p)
		return false;

	return true;
}

struct bus_type hisi_djtag_bus = {
	.name		= "hisi-djtag",
	.match		= hisi_djtag_device_match,
	.probe		= hisi_djtag_device_probe,
	.remove		= hisi_djtag_device_remove,
};

struct hisi_djtag_client *hisi_djtag_new_device(struct hisi_djtag_host *host,
						struct device_node *node)
{
	struct hisi_djtag_client *client;
	int status;

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (!client)
		return NULL;

	client->host = host;

	client->dev.parent = &client->host->dev;
	client->dev.bus = &hisi_djtag_bus;
	client->dev.type = &hisi_djtag_client_type;
	client->dev.of_node = node;
	snprintf(client->name, DJTAG_CLIENT_NAME_LEN, "%s%s",
					DJTAG_PREFIX, node->name);
	dev_set_name(&client->dev, "%s", client->name);

	status = device_register(&client->dev);
	if (status < 0) {
		pr_err("error adding new device, status=%d\n", status);
		kfree(client);
		return NULL;
	}

	return client;
}

static struct hisi_djtag_client *hisi_djtag_of_register_device(
						struct hisi_djtag_host *host,
						struct device_node *node)
{
	struct hisi_djtag_client *client;

	client = hisi_djtag_new_device(host, node);
	if (client == NULL) {
		dev_err(&host->dev, "error registering device %s\n",
			node->full_name);
		of_node_put(node);
		return ERR_PTR(-EINVAL);
	}

	return client;
}

static void djtag_register_devices(struct hisi_djtag_host *host)
{
	struct device_node *node;
	struct hisi_djtag_client *client;

	if (!host->of_node)
		return;

	for_each_available_child_of_node(host->of_node, node) {
		if (of_node_test_and_set_flag(node, OF_POPULATED))
			continue;
		client = hisi_djtag_of_register_device(host, node);
		list_add(&client->next, &host->client_list);
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
        host->dev_id = rc;

	/* Suffix the unique ID and set djtag hostname */
	dev_set_name(&host->dev, "djtag-host-%d", host->dev_id);
	rc = device_register(&host->dev);
	if (rc < 0) {
		dev_err(&host->dev, "add_host dev register failed, rc=%d\n",
									rc);
		idr_remove(&djtag_hosts_idr, host->dev_id);
		return rc;
	}

	djtag_register_devices(host);

	return 0;
}

static int djtag_host_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hisi_djtag_host *host;
	const struct of_device_id *of_id;
	struct resource *res;
	int rc;

	of_id = of_match_device(djtag_of_match, dev);
	if (!of_id)
		return -EINVAL;

	host = kzalloc(sizeof(*host), GFP_KERNEL);
	if (!host)
		return -ENOMEM;

	host->of_node = dev->of_node;
	host->djtag_readwrite = of_id->data;
	spin_lock_init(&host->lock);

	INIT_LIST_HEAD(&host->client_list);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "No reg resorces!\n");
		kfree(host);
		return -EINVAL;
	}

	if (!resource_size(res)) {
		dev_err(&pdev->dev, "Zero reg entry!\n");
		kfree(host);
		return -EINVAL;
	}

	host->sysctl_reg_map = devm_ioremap_resource(dev, res);
	if (IS_ERR(host->sysctl_reg_map)) {
		dev_warn(dev, "Unable to map sysctl registers.\n");
		kfree(host);
		return -EINVAL;
	}

	platform_set_drvdata(pdev, host);

	rc = hisi_djtag_add_host(host);
	if (rc) {
		dev_err(dev, "add host failed, rc=%d\n", rc);
		kfree(host);
		return rc;
	}

	return 0;
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
		kfree(client);
	}

	device_unregister(&host->dev);
	idr_remove(&djtag_hosts_idr, host->dev_id);
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

/*
 * Copyright (c) 2014-2015 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_mdio.h>
#include <linux/of_address.h>
#include <linux/mutex.h>
#include <linux/spinlock_types.h>

#define MDIO_DRV_NAME "Hi-MDIO"
#define MDIO_BUS_NAME "Hisilicon MII Bus"
#define MDIO_DRV_VERSION "1.1.0"
#define MDIO_COPYRIGHT "Copyright(c) 2015 - 2019 Huawei Corporation."
#define MDIO_DRV_STRING MDIO_BUS_NAME
#define MDIO_DEFAULT_DEVICE_DESCR MDIO_BUS_NAME

#define MDIO_CTL_DEV_ADDR(x)	(x & 0x1f)
#define MDIO_CTL_PORT_ADDR(x)	((x & 0x1f) << 5)

#define MDIO_BASE_ADDR			0x403C0000
#define MDIO_REG_ADDR_LEN		0x1000
#define MDIO_PHY_GRP_LEN		0x100
#define MDIO_REG_LEN			0x10
#define MDIO_PHY_ADDR_NUM		5
#define MDIO_MAX_PHY_ADDR		0x1F
#define MDIO_MAX_PHY_REG_ADDR		0xFFFF

#define MDIO_TIMEOUT			1000000

typedef u16 UINT16;
struct hns_mdio_device {
	struct device *dev;
	void *vbase;		/* mdio reg base address */
	u8 phy_class[PHY_MAX_ADDR];
	u8 index;
	u8 chip_id;
	u8 gidx;		/* global index */
};

#define MDIO_COMMAND_REG		0x0
#define MDIO_ADDR_REG			0x4
#define MDIO_WDATA_REG			0x8
#define MDIO_RDATA_REG			0xc
#define MDIO_STA_REG			0x10

#define MDIO_CMD_DEVAD_M	0x1f
#define MDIO_CMD_DEVAD_S	0
#define MDIO_CMD_PRTAD_M	0x1f
#define MDIO_CMD_PRTAD_S	5
#define MDIO_CMD_OP_M		0x3
#define MDIO_CMD_OP_S		10
#define MDIO_CMD_ST_M		0x3
#define MDIO_CMD_ST_S		12
#define MDIO_CMD_START_B	14

#define MDIO_ADDR_DATA_M	0xffff
#define MDIO_ADDR_DATA_S	0

#define MDIO_WDATA_DATA_M	0xffff
#define MDIO_WDATA_DATA_S	0

#define MDIO_RDATA_DATA_M	0xffff
#define MDIO_RDATA_DATA_S	0

#define MDIO_STATE_STA_B	0

enum mdio_st_clause {
	MDIO_ST_CLAUSE_45 = 0,
	MDIO_ST_CLAUSE_22
};

enum mdio_c22_op_seq {
	MDIO_C22_WRITE = 1,
	MDIO_C22_READ = 2
};

enum mdio_c45_op_seq {
	MDIO_C45_WRITE_ADDR = 0,
	MDIO_C45_WRITE_DATA,
	MDIO_C45_READ_INCREMENT,
	MDIO_C45_READ
};

static inline void mdio_write_reg(void *base, u32 reg, u32 value)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(base);

	writel(value, reg_addr + reg);
}

#define MDIO_WRITE_REG(a, reg, value) \
	mdio_write_reg((a)->vbase, (reg), (value))

static inline u32 mdio_read_reg(void *base, u32 reg)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(base);

	return readl(reg_addr + reg);
}

#define MDIO_READ_REG(a, reg) \
	mdio_read_reg((a)->vbase, (reg))

#define mdio_set_field(origin, mask, shift, val) \
	do { \
		(origin) &= (~((mask) << (shift))); \
		(origin) |= (((val) & (mask)) << (shift)); \
	} while (0)

#define mdio_get_field(origin, mask, shift) (((origin) >> (shift)) & (mask))

static void mdio_set_reg_field(void *base, u32 reg, u32 mask, u32 shift,
			       u32 val)
{
	u32 origin = mdio_read_reg(base, reg);

	mdio_set_field(origin, mask, shift, val);
	mdio_write_reg(base, reg, origin);
}

#define MDIO_SET_REG_FIELD(dev, reg, mask, shift, val) \
	mdio_set_reg_field((dev)->vbase, (reg), (mask), (shift), (val))

static u32 mdio_get_reg_field(void *base, u32 reg, u32 mask, u32 shift)
{
	u32 origin;

	origin = mdio_read_reg(base, reg);
	return mdio_get_field(origin, mask, shift);
}

#define MDIO_GET_REG_FIELD(dev, reg, mask, shift) \
		mdio_get_reg_field((dev)->vbase, (reg), (mask), (shift))

#define MDIO_SET_REG_BIT(dev, reg, bit, val) \
		mdio_set_reg_field((dev)->vbase, (reg), 0x1ull, (bit), (val))

#define MDIO_GET_REG_BIT(dev, reg, bit) \
		mdio_get_reg_field((dev)->vbase, (reg), 0x1ull, (bit))

/**
 *hns_mdio_read_hw - read phy regs
 *@mdio_dev: mdio device
 *@phy_addr:phy addr
 *@is_c45:
 *@page:
 *@reg: reg
 *@data:regs data
 *return status
 */
static int hns_mdio_read_hw(struct hns_mdio_device *mdio_dev, u8 phy_addr,
			    u8 is_c45, u8 page, u16 reg, u16 *data)
{
	u32 cmd_reg_value;
	u32 sta_reg_value;
	u32 time_cnt;

	if (phy_addr > MDIO_MAX_PHY_ADDR) {
		dev_err(mdio_dev->dev, "Wrong phy address: phy_addr(%x)\n",
			phy_addr);
		return -EINVAL;
	}

	dev_dbg(mdio_dev->dev, "mdio(%d) base is %p\n",
		mdio_dev->gidx, mdio_dev->vbase);
	dev_dbg(mdio_dev->dev,
		"phy_addr=%d, is_c45=%d, devad=%d, regnum=%#x!\n",
		phy_addr, is_c45, page, reg);

	/* Step 1; waitting for MDIO_COMMAND_REG 's mdio_start==0,	*/
	/*	after that can do read or wtrite*/
	for (time_cnt = MDIO_TIMEOUT; time_cnt; time_cnt--) {
		cmd_reg_value = MDIO_GET_REG_BIT(mdio_dev,
						 MDIO_COMMAND_REG,
						 MDIO_CMD_START_B);
		if (!cmd_reg_value)
			break;
	}
	if (cmd_reg_value) {
		dev_err(mdio_dev->dev, "MDIO is always busy!\n");
		return -EBUSY;
	}

	if (!is_c45) {
		cmd_reg_value = MDIO_ST_CLAUSE_22 << MDIO_CMD_ST_S;
		cmd_reg_value |= MDIO_C22_READ << MDIO_CMD_OP_S;
		cmd_reg_value |=
			(phy_addr & MDIO_CMD_PRTAD_M) << MDIO_CMD_PRTAD_S;
		cmd_reg_value |= (reg & MDIO_CMD_DEVAD_M) << MDIO_CMD_DEVAD_S;
		cmd_reg_value |= 1 << MDIO_CMD_START_B;

		MDIO_WRITE_REG(mdio_dev, MDIO_COMMAND_REG, cmd_reg_value);
	} else {
		MDIO_SET_REG_FIELD(mdio_dev, MDIO_ADDR_REG, MDIO_ADDR_DATA_M,
				   MDIO_ADDR_DATA_S, reg);

		/* Step 2; config the cmd-reg to write addr*/
		cmd_reg_value = MDIO_ST_CLAUSE_45 << MDIO_CMD_ST_S;
		cmd_reg_value |= MDIO_C45_WRITE_ADDR << MDIO_CMD_OP_S;

		/* mdio_st==2'b00: is configing port addr*/
		cmd_reg_value |=
			(phy_addr & MDIO_CMD_PRTAD_M) << MDIO_CMD_PRTAD_S;

		/* mdio_st == 2'b00: is configing dev_addr*/
		cmd_reg_value |= (page & MDIO_CMD_DEVAD_M) << MDIO_CMD_DEVAD_S;
		cmd_reg_value |= 1 << MDIO_CMD_START_B;
		MDIO_WRITE_REG(mdio_dev, MDIO_COMMAND_REG, cmd_reg_value);

		/* Step 3; waitting for MDIO_COMMAND_REG 's mdio_start==0,*/
		/*check for read or write opt is finished */
		for (time_cnt = MDIO_TIMEOUT; time_cnt; time_cnt--) {
			cmd_reg_value = MDIO_GET_REG_BIT(mdio_dev,
							 MDIO_COMMAND_REG,
							 MDIO_CMD_START_B);
			if (!cmd_reg_value)
				break;
		}
		if (cmd_reg_value) {
			dev_err(mdio_dev->dev, "MDIO is always busy\n");
			return -EBUSY;
		}

		/* Step 4; config cmd-reg, send read opt */
		cmd_reg_value = MDIO_ST_CLAUSE_45 << MDIO_CMD_ST_S;
		cmd_reg_value |= MDIO_C45_READ << MDIO_CMD_OP_S;

		/* mdio_st==2'b00: is configing port addr*/
		cmd_reg_value |=
			(phy_addr & MDIO_CMD_PRTAD_M) << MDIO_CMD_PRTAD_S;

		/* mdio_st == 2'b00: is configing dev_addr*/
		cmd_reg_value |= (page & MDIO_CMD_DEVAD_M) << MDIO_CMD_DEVAD_S;
		cmd_reg_value |= 1 << MDIO_CMD_START_B;
		MDIO_WRITE_REG(mdio_dev, MDIO_COMMAND_REG, cmd_reg_value);
	}

	/* Step 5; waitting for MDIO_COMMAND_REG 's mdio_start==0,*/
	/* check for read or write opt is finished */
	for (time_cnt = MDIO_TIMEOUT; time_cnt; time_cnt--) {
		cmd_reg_value = MDIO_GET_REG_BIT(mdio_dev,
						 MDIO_COMMAND_REG,
						 MDIO_CMD_START_B);
		if (!cmd_reg_value)
			break;
	}
	if (cmd_reg_value) {
		dev_err(mdio_dev->dev, "MDIO is always busy\n");
		return -EBUSY;
	}

	sta_reg_value = MDIO_GET_REG_BIT(mdio_dev,
					 MDIO_STA_REG, MDIO_STATE_STA_B);
	if (sta_reg_value) {
		dev_err(mdio_dev->dev, " ERROR! MDIO Read failed!\n");
		return -EBUSY;
	}

	/* Step 6; get out data*/
	*data = (u16)MDIO_GET_REG_FIELD(mdio_dev, MDIO_RDATA_REG,
					MDIO_RDATA_DATA_M, MDIO_RDATA_DATA_S);

	return 0;
}

/**
 *hns_mdio_write_hw - write phy regs
 *@mdio_dev: mdio device
 *@phy_addr: phy addr
 *@is_c45: 0 means the clause 22, none 0 means clause 45
 *@page: phy page addr
 *@reg:  phy reg
 *@data: regs data
 *return status
 */
static int hns_mdio_write_hw(struct hns_mdio_device *mdio_dev, u8 phy_addr,
			     u8 is_c45, u8 page, u16 reg, u16 data)
{
	u32 cmd_reg_value;
	u32 time_cnt;

	if (phy_addr > MDIO_MAX_PHY_ADDR) {
		dev_err(mdio_dev->dev, "Wrong phy address: phy_addr(%x)\n",
			phy_addr);
		return -EINVAL;
	}

	dev_dbg(mdio_dev->dev, "mdio(%d) base is %p,write data=%d\n",
		mdio_dev->gidx, mdio_dev->vbase, data);
	dev_dbg(mdio_dev->dev, "phy_addr=%d, is_c45=%d, devad=%d, regnum=%#x\n",
		phy_addr, is_c45, page, reg);

	/* Step 1; waitting for MDIO_COMMAND_REG 's mdio_start==0,*/
	/*	after that can do read or wtrite*/
	for (time_cnt = MDIO_TIMEOUT; time_cnt; time_cnt--) {
		cmd_reg_value = MDIO_GET_REG_BIT(mdio_dev,
						 MDIO_COMMAND_REG,
						 MDIO_CMD_START_B);
		if (!cmd_reg_value)
			break;
	}
	if (cmd_reg_value) {
		dev_err(mdio_dev->dev, "MDIO is always busy\n");
		return -EBUSY;
	}

	if (!is_c45) {
		MDIO_SET_REG_FIELD(mdio_dev, MDIO_WDATA_REG, MDIO_WDATA_DATA_M,
				   MDIO_WDATA_DATA_S, data);

		cmd_reg_value = MDIO_ST_CLAUSE_22 << MDIO_CMD_ST_S;
		cmd_reg_value |= MDIO_C22_WRITE << MDIO_CMD_OP_S;
		cmd_reg_value |=
			(phy_addr & MDIO_CMD_PRTAD_M) << MDIO_CMD_PRTAD_S;
		cmd_reg_value |= (reg & MDIO_CMD_DEVAD_M) << MDIO_CMD_DEVAD_S;
		cmd_reg_value |= 1 << MDIO_CMD_START_B;

		MDIO_WRITE_REG(mdio_dev, MDIO_COMMAND_REG, cmd_reg_value);
	} else {
		/* Step 2; config the cmd-reg to write addr*/
		MDIO_SET_REG_FIELD(mdio_dev, MDIO_ADDR_REG, MDIO_ADDR_DATA_M,
				   MDIO_ADDR_DATA_S, reg);

		cmd_reg_value = MDIO_ST_CLAUSE_45 << MDIO_CMD_ST_S;
		cmd_reg_value |= MDIO_C45_WRITE_ADDR << MDIO_CMD_OP_S;

		/* mdio_st==2'b00: is configing port addr*/
		cmd_reg_value |=
			(phy_addr & MDIO_CMD_PRTAD_M) << MDIO_CMD_PRTAD_S;

		/* mdio_st == 2'b00: is configing dev_addr*/
		cmd_reg_value |= (page & MDIO_CMD_DEVAD_M) << MDIO_CMD_DEVAD_S;
		cmd_reg_value |= 1 << MDIO_CMD_START_B;
		MDIO_WRITE_REG(mdio_dev, MDIO_COMMAND_REG, cmd_reg_value);

		/* Step 3; waitting for MDIO_COMMAND_REG 's mdio_start==0,*/
		/* check for read or write opt is finished */
		for (time_cnt = MDIO_TIMEOUT; time_cnt; time_cnt--) {
			cmd_reg_value = MDIO_GET_REG_BIT(mdio_dev,
							 MDIO_COMMAND_REG,
							 MDIO_CMD_START_B);
			if (!cmd_reg_value)
				break;
		}
		if (cmd_reg_value) {
			dev_err(mdio_dev->dev, "MDIO is always busy\n");
			return -EBUSY;
		}

		/* Step 4; config the data needed writing */
		MDIO_SET_REG_FIELD(mdio_dev, MDIO_WDATA_REG, MDIO_WDATA_DATA_M,
				   MDIO_WDATA_DATA_S, data);

		/* Step 5; config the cmd-reg for the write opt*/
		cmd_reg_value = MDIO_ST_CLAUSE_45 << MDIO_CMD_ST_S;
		cmd_reg_value |= MDIO_C45_WRITE_DATA << MDIO_CMD_OP_S;

		/* mdio_st==2'b00: is configing port addr*/
		cmd_reg_value |=
			(phy_addr & MDIO_CMD_PRTAD_M) << MDIO_CMD_PRTAD_S;

		/* mdio_st == 2'b00: is configing dev_addr*/
		cmd_reg_value |= (page & MDIO_CMD_DEVAD_M) << MDIO_CMD_DEVAD_S;
		cmd_reg_value |= 1 << MDIO_CMD_START_B;
		MDIO_WRITE_REG(mdio_dev, MDIO_COMMAND_REG, cmd_reg_value);
	}

	return 0;
}

/**
 * hns_mdio_get_chip_id - get parent node's chip-id attribute
 * @dpev: mdio platform device
 * return chip id if found the chip-id, or return 0
 */
static u32 hns_mdio_get_chip_id(struct platform_device *pdev)
{
	struct device_node *np;
	u32 chip_id;
	int ret;

	np = of_get_parent(pdev->dev.of_node);
	if (!np)
		return 0;

	ret = of_property_read_u32(np, "chip-id", &chip_id);
	if (ret)
		return 0;

	return chip_id;
}

/**
 * hns_mdio_write - access phy register
 * @bus: mdio bus
 * @phy_id: phy id
 * @regnum: register num
 * @value: register value
 *
 * Return 0 on success, negative on failure
 */
static int hns_mdio_write(struct mii_bus *bus,
			  int phy_id, int regnum, u16 value)
{
	int ret;
	struct hns_mdio_device *mdio_dev = (struct hns_mdio_device *)bus->priv;
	u8 devad = ((regnum >> 16) & 0x1f);
	u8 is_c45 = !!(regnum & MII_ADDR_C45);

	ret = hns_mdio_write_hw(mdio_dev, phy_id, is_c45, devad,
				(u16)(regnum & 0xffff), value);
	if (ret) {
		dev_err(&bus->dev, "write_phy_reg fail, phy_id=%d, devad=%d,regnum=%#x, value=%#x!\n",
			phy_id, devad, regnum, value);

		return ret;
	}

	return 0;
}

/**
 * hns_mdio_read - access phy register
 * @bus: mdio bus
 * @phy_id: phy id
 * @regnum: register num
 * @value: register value
 *
 * Return phy register value
 */
static int hns_mdio_read(struct mii_bus *bus, int phy_id, int regnum)
{
	int ret;
	u16 reg_val = 0;
	u8 devad = ((regnum >> 16) & 0x1f);
	u8 is_c45 = !!(regnum & MII_ADDR_C45);
	struct hns_mdio_device *mdio_dev = (struct hns_mdio_device *)bus->priv;

	ret = hns_mdio_read_hw(mdio_dev, phy_id, is_c45, devad,
			       (u16)(regnum&0xffff), &reg_val);
	if (ret) {
		dev_err(&bus->dev, "read_phy_reg fail, mdio_idx=%d, phy_id=%d, devad=%d,regnum=%#x!\n",
			mdio_dev->gidx,	phy_id, devad, regnum);

		return ret;
	}

	return reg_val;
}

/**
 * hns_mdio_reset - reset mdio bus
 * @bus: mdio bus
 *
 * Return 0 on success, negative on failure
 */
static int hns_mdio_reset(struct mii_bus *bus)
{
	return 0;
}

/**
 * hns_mdio_bus_name - get mdio bus name
 * @name: mdio bus name
 * @np: mdio device node pointer
 */
static void hns_mdio_bus_name(char *name, struct device_node *np)
{
	const u32 *addr;
	u64 taddr = OF_BAD_ADDR;

	addr = of_get_address(np, 0, NULL, NULL);
	if (addr)
		taddr = of_translate_address(np, addr);

	snprintf(name, MII_BUS_ID_SIZE, "%s@%llx", np->name,
		 (unsigned long long)taddr);
}

/**
 * hns_mdio_probe - probe mdio device
 * @pdev: mdio platform device
 *
 * Return 0 on success, negative on failure
 */
static int hns_mdio_probe(struct platform_device *pdev)
{
	struct device_node *np;
	struct hns_mdio_device *mdio_dev;
	struct mii_bus *new_bus;
	struct resource *res;
	int ret;

	if (!pdev) {
		dev_err(NULL, "pdev is NULL!\r\n");
		return -ENODEV;
	}
	np = pdev->dev.of_node;
	mdio_dev = devm_kzalloc(&pdev->dev, sizeof(*mdio_dev), GFP_KERNEL);
	if (!mdio_dev)
		return -ENOMEM;

	new_bus = devm_mdiobus_alloc(&pdev->dev);
	if (!new_bus) {
		dev_err(&pdev->dev, "mdiobus_alloc fail!\n");
		return -ENOMEM;
	}

	new_bus->name = MDIO_BUS_NAME;
	new_bus->read = hns_mdio_read;
	new_bus->write = hns_mdio_write;
	new_bus->reset = hns_mdio_reset;
	new_bus->priv = mdio_dev;
	hns_mdio_bus_name(new_bus->id, np);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mdio_dev->vbase = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(mdio_dev->vbase)) {
		ret = PTR_ERR(mdio_dev->vbase);
		return -EFAULT;
	}
	mdio_dev->index = 0;
	mdio_dev->chip_id = hns_mdio_get_chip_id(pdev);
	mdio_dev->gidx = mdio_dev->chip_id;
	mdio_dev->dev = &pdev->dev;

	new_bus->irq = devm_kcalloc(&pdev->dev, PHY_MAX_ADDR,
				    sizeof(int), GFP_KERNEL);
	if (!new_bus->irq)
		return -ENOMEM;

	new_bus->parent = &pdev->dev;
	platform_set_drvdata(pdev, new_bus);

	ret = of_mdiobus_register(new_bus, np);
	if (ret) {
		dev_err(&pdev->dev, "Cannot register as MDIO bus!\n");
		platform_set_drvdata(pdev, NULL);
		return ret;
	}

	return 0;
}

/**
 * hns_mdio_remove - remove mdio device
 * @pdev: mdio platform device
 *
 * Return 0 on success, negative on failure
 */
static int hns_mdio_remove(struct platform_device *pdev)
{
	struct hns_mdio_device *mdio_dev;
	struct mii_bus *bus;

	bus = platform_get_drvdata(pdev);
	mdio_dev = (struct hns_mdio_device *)bus->priv;

	mdiobus_unregister(bus);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id hns_mdio_match[] = {
	{.compatible = "hisilicon,mdio"},
	{}
};

static struct platform_driver hns_mdio_driver = {
	.probe = hns_mdio_probe,
	.remove = hns_mdio_remove,
	.driver = {
		   .name = MDIO_DRV_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = hns_mdio_match,
		   },
};

/**
 * hns_mdio_module_init - Driver Registration Routine
 *
 * hns_mdio_module_init is the first routine called when the driver is
 * loaded.
 **/
int __init hns_mdio_module_init(void)
{
	pr_info("%s - version %s\n", MDIO_DRV_STRING, MDIO_DRV_VERSION);
	pr_info("%s\n", MDIO_COPYRIGHT);

	return platform_driver_register(&hns_mdio_driver);
}

/**
 * hns_mdio_module_exit - Driver Exit Cleanup Routine
 *
 * hns_mdio_module_exit is called just before the driver is removed
 * from memory.
 **/
void hns_mdio_module_exit(void)
{
	platform_driver_unregister(&hns_mdio_driver);
}

subsys_initcall_sync(hns_mdio_module_init);
module_exit(hns_mdio_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("Hisilicon MDIO driver");

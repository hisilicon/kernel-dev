/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe486jA6OWvCqtz25NsRgVbi9ajPwUl1LWRVejglHN+CTtGUt2gOn4xfREdRgNtDOX1/nJz
F7SDulmSO1DdDEH5ggZO+R8aqloceR8NMkWuqJTvNOrTs7lV7/1+kiAdqAO+SQ==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/*********************************************************************

  Hisilicon MDIO driver
  Copyright(c) 2014 - 2019 Huawei Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:TBD

**********************************************************************/

#include "iware_error.h"
#include "iware_log.h"
#include "iware_mdio_main.h"
#include "iware_mdio_hal.h"

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

/**
 * mdio_get_chip_id - get parent node's chip-id attribute
 * @dpev: mdio platform device
 */
static int mdio_get_chip_id(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	u32 chip_id = 0;
	int ret = 0;

	np = of_get_parent(pdev->dev.of_node);
	if (np == NULL)
		return -EINVAL;

	ret = of_property_read_u32(np, "chip-id", &chip_id);
	if (ret != HRD_OK)
		return 0;	/* attribute not found, return 0 */

	if(chip_id >= DSAF_MAX_CHIP_NUM) {
		log_err(&pdev->dev, "chip_id error!\r\n");
		return 0;
	}
	else
		return chip_id;
}

/**
 * mdio_write - access phy register
 * @bus: mdio bus
 * @phy_id: phy id
 * @regnum: register num
 * @value: register value
 *
 * Return 0 on success, negative on failure
 */
static int mdio_write(struct mii_bus *bus, int phy_id, int regnum, u16 value)
{
	int ret = 0;
	struct mdio_device *mdio_dev = NULL;
	u8 devad = ((regnum >> 16) & 0x1f);
    u8 is_c45 = !!(regnum & MII_ADDR_C45);

	mdio_dev = (struct mdio_device *)bus->priv;

	mutex_lock(&mdio_dev->mdio_lock);
	ret = mdio_dev->ops.write_phy_reg(mdio_dev, phy_id, is_c45, devad,
		(u16)(regnum&0xffff), value);
	mutex_unlock(&mdio_dev->mdio_lock);
	log_dbg(&bus->dev, "bus=%#llx, phy_id=%d, regnum=%d, value=%#x!\n",
		(u64)bus, phy_id, regnum, value);
	if (ret) {
		log_err(&bus->dev,
			"write_phy_reg fail, phy_id=%d, devad=%d,regnum=%#x, value=%#x!\n",
			phy_id, devad, regnum, value);
		return ret;
	}
	return 0;
}

/**
 * mdio_read - access phy register
 * @bus: mdio bus
 * @phy_id: phy id
 * @regnum: register num
 * @value: register value
 *
 * Return phy register value
 */
static int mdio_read(struct mii_bus *bus, int phy_id, int regnum)
{
	int ret = 0;
	u16 reg_val = 0;
	u8 devad = ((regnum >> 16) & 0x1f);
    u8 is_c45 = !!(regnum & MII_ADDR_C45);

	struct mdio_device *mdio_dev = NULL;

	mdio_dev = (struct mdio_device *)bus->priv;

	mutex_lock(&mdio_dev->mdio_lock);
	ret = mdio_dev->ops.read_phy_reg(mdio_dev, phy_id, is_c45, devad,
		(u16)(regnum&0xffff), &reg_val);
	mutex_unlock(&mdio_dev->mdio_lock);
	log_dbg(&bus->dev, "bus=%#llx, phy_id=%d, regnum=%d,value=%#x!\n",
		(u64)bus, phy_id, regnum, reg_val);
	if (ret) {
		log_err(&bus->dev,
			"read_phy_reg fail, mdio_idx=%d, phy_id=%d,	devad=%d,regnum=%#x!\n",
			mdio_dev->gidx,	phy_id, devad, regnum);
		return ret;
	}
	return reg_val;
}

/**
 * mdio_reset - reset mdio bus
 * @bus: mdio bus
 *
 * Return 0 on success, negative on failure
 */
static int mdio_reset(struct mii_bus *bus)
{
	int ret = 0;
	struct mdio_device *mdio_dev = NULL;

	mdio_dev = (struct mdio_device *)bus->priv;

	mutex_lock(&mdio_dev->mdio_lock);
	ret = mdio_dev->ops.reset(mdio_dev);
	mutex_unlock(&mdio_dev->mdio_lock);
	log_info(&bus->dev, "reset mdio bus=%#llx!\n",(u64)bus);
	if (ret) {
		log_err(&bus->dev,
			"mdio reset fail, mdio_idx=%d!\n",mdio_dev->gidx);
		return ret;
	}

	return ret;
}

/**
 * mdio_bus_name - get mdio bus name
 * @name: mdio bus name
 * @np: mdio device node pointer
 */
static void mdio_bus_name(char *name, struct device_node *np)
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
 * mdio_probe - probe mdio device
 * @pdev: mdio platform device
 *
 * Return 0 on success, negative on failure
 */
static int mdio_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	struct mdio_device *mdio_dev = NULL;
	void __iomem *map;
	void __iomem *sys_map = NULL;
	struct mii_bus *new_bus;
	u64 phy_addr = 0;
	u64 tmp_addr = 0;
	u64 size = 0;
	int ret = 0;
	int chip_id = 0;

	log_dbg(&pdev->dev, "func begin\n");

	if (NULL == pdev) {
		log_err(NULL, "pdev is NULL!\r\n");
		return -ENODEV;
	}

	np = pdev->dev.of_node;

	mdio_dev = kzalloc(sizeof(*mdio_dev), GFP_KERNEL);
	if (NULL == mdio_dev) {
		log_err(&pdev->dev, "kzalloc fail!\n");
		return -ENOMEM;
	}

	mutex_init(&mdio_dev->mdio_lock);

	new_bus = mdiobus_alloc();
	if (NULL == new_bus) {
		log_err(&pdev->dev, "mdiobus_alloc fail!\n");
		ret = -ENOMEM;
		goto alloc_mdiobus_fail;
	}

	new_bus->name = "Hisilicon MII Bus", new_bus->read = mdio_read;
	new_bus->write = mdio_write;
	new_bus->reset = mdio_reset;
	new_bus->priv = mdio_dev;
	mdio_bus_name(new_bus->id, np);

	tmp_addr = (u64) of_get_address(np, 0, &size, NULL);
	if (tmp_addr == 0) {
		log_err(&pdev->dev, "of_get_address fail!\r\n");
		ret = -EINVAL;
		goto alloc_mdiobus_fail;
	}
	phy_addr = of_translate_address(np, (const void *)tmp_addr);
	if (OF_BAD_ADDR == phy_addr) {
		log_err(&pdev->dev, "of_translate_address fail!\r\n");
		ret = -EINVAL;
		goto alloc_mdiobus_fail;
	}
	map = (void __iomem *)ioremap(phy_addr, size);
	if (!map) {
		log_err(&pdev->dev, "of_iomap fail!\n");
		ret = -ENOMEM;
		goto iomap_fail;
	}
	/*map sys ctl addr**/
	tmp_addr = (u64) of_get_address(np, 1, &size, NULL);
	if (tmp_addr) {
		phy_addr = of_translate_address(np, (const void *)tmp_addr);
		if (OF_BAD_ADDR != phy_addr)
			sys_map = (void __iomem *)ioremap(phy_addr, size);
	}

	chip_id = mdio_get_chip_id(pdev);
	if(chip_id < 0) {
		ret = -EINVAL;
		goto iomap_fail;
	}
	else
		mdio_dev->chip_id = chip_id;

	mdio_dev->vbase = map;
	mdio_dev->sys_vbase = sys_map;
	mdio_dev->index = 0;	/* Multi-chip mode TBD */
	mdio_dev->gidx = mdio_dev->chip_id;
	mdio_dev->dev = &pdev->dev;

	new_bus->irq = kcalloc(PHY_MAX_ADDR, sizeof(int), GFP_KERNEL);
	if (NULL == new_bus->irq) {
		log_err(&pdev->dev, "kcalloc fail!\n");
		ret = -ENOMEM;
		goto alloc_irq_mem_fail;
	}

	new_bus->parent = &pdev->dev;
	platform_set_drvdata(pdev, new_bus);

	mdio_set_ops(&mdio_dev->ops);

	ret = of_mdiobus_register(new_bus, np);
	if (ret != HRD_OK) {
		log_err(&pdev->dev, "Cannot register as MDIO bus!\n");
		goto reg_mdiobus_fail;
	}

	return 0;

reg_mdiobus_fail:
	platform_set_drvdata(pdev, NULL);
	kfree(new_bus->irq);

alloc_irq_mem_fail:
	iounmap(mdio_dev->vbase);

iomap_fail:
	kfree(new_bus);

alloc_mdiobus_fail:
	kfree(mdio_dev);

	return ret;
}

/**
 * mdio_remove - remove mdio device
 * @pdev: mdio platform device
 *
 * Return 0 on success, negative on failure
 */
static int mdio_remove(struct platform_device *pdev)
{
	struct mdio_device *mdio_dev = NULL;
	struct mii_bus *bus;

	bus = platform_get_drvdata(pdev);
	mdio_dev = (struct mdio_device *)bus->priv;

	mdiobus_unregister(bus);
	platform_set_drvdata(pdev, NULL);
	kfree(bus->irq);
	iounmap(mdio_dev->vbase);
	kfree(bus);
	kfree(mdio_dev);

	return 0;
}

static const struct of_device_id mdio_match[] = {
	{.compatible = "hisilicon,mdio"},
	{}
};

static struct platform_driver mdio_driver = {
	.probe = mdio_probe,
	.remove = mdio_remove,
	.driver = {
		   .name = MDIO_DRV_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = mdio_match,
		   },
};

/**
 * mdio_module_init - Driver Registration Routine
 *
 * mdio_module_init is the first routine called when the driver is
 * loaded.
 **/
int __init mdio_module_init(void)
{
	return platform_driver_register(&mdio_driver);
}

/**
 * mdio_module_exit - Driver Exit Cleanup Routine
 *
 * mdio_module_exit is called just before the driver is removed
 * from memory.
 **/
void mdio_module_exit(void)
{
	platform_driver_unregister(&mdio_driver);
}

subsys_initcall_sync(mdio_module_init);
module_exit(mdio_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("Hisilicon MDIO driver");
MODULE_VERSION(MDIO_MOD_VERSION "," __DATE__ "," __TIME__);

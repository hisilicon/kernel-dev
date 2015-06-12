/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe4803ScZEptIRgbgyVkhOuBnV7zN6H8Hlwd4kh+5z8rzIgY+bJD1TCc4QTx6xpwOwubiks
4UWMn8suRzJDxHADo12UJv69GpvH7XWduUXl+b47MU6DB1hVfYqGKuJFXMsrMA==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/*******************************************************************************

  Hisilicon DSAF - support L2 switch
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

*******************************************************************************/

#include "iware_error.h"
#include "iware_log.h"
#include "iware_mac_main.h"
#include "iware_dsaf_main.h"
#include "iware_dsaf_drv_hw.h"
#include "iware_dsaf_sysfs.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
//#include <linux/irqchip/hisi-msi-gen.h>

struct dsaf_device *g_dsaf_dev[DSAF_MAX_CHIP_NUM] = {NULL, NULL};

/**
 * dsaf_register_mac_dev - register mac dev
 * @mac_dev: mac edv pointer
 */
struct dsaf_device *dsaf_register_mac_dev(struct mac_device *mac_dev)
{
	u8 chip_id = 0;
	u8 mac_id = 0;
	int ret;
	struct dsaf_device *dsaf_dev = NULL;

	if (NULL == mac_dev) {
		pr_err("dsaf_register_mac_dev faild, input invalid !\n");
		return NULL;
	}

	chip_id = mac_dev->chip_id;
	mac_id = mac_dev->mac_id;

	if ((chip_id >= DSAF_MAX_CHIP_NUM)
		|| (mac_id >= DSAF_MAX_PORT_NUM_PER_CHIP)) {
		pr_err("register_mac_dev faild, chip%d or mac%d invalid !\n",
			chip_id, mac_id);
		return NULL;
	}

	dsaf_dev = g_dsaf_dev[chip_id];

	if (NULL == dsaf_dev) {
		pr_err("dsaf_register_mac_dev faild, dsaf%d invalid !\n",
			chip_id);
		return NULL;
	}

	dsaf_dev->mac_dev[mac_id] = mac_dev;

	/*if phy mac, DSAF ser GE/XGE mode*/
	if ((NULL != dsaf_dev->fix_mac_mode)
		&& (mac_id <= DSAF_MAX_REAL_PORT_ID_PER_CHIP)) {
		ret = dsaf_dev->fix_mac_mode(dsaf_dev, mac_id);
		if (ret) {
			log_err(dsaf_dev->dev,
				"fix_mac_mode faild, dsaf%d mac%d ret = %#x!\n",
				dsaf_dev->chip_id, mac_id, ret);
			dsaf_dev->mac_dev[mac_id] = NULL;
			return NULL;
		}
	}

	return g_dsaf_dev[chip_id];
}
EXPORT_SYMBOL(dsaf_register_mac_dev);


/**
 * dsaf_register_mac_dev - unregister mac dev
 * @mac_dev: mac edv pointer
 */
int dsaf_unregister_mac_dev(struct mac_device *mac_dev)
{
	u8 chip_id = 0;
	u8 mac_id = 0;

	if (NULL == mac_dev) {
		pr_err("dsaf_unregister_mac_dev faild, input invalid !\n");
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	chip_id = mac_dev->chip_id;
	mac_id = mac_dev->mac_id;
	if ((chip_id >= DSAF_MAX_CHIP_NUM)
		|| (mac_id >= DSAF_MAX_PORT_NUM_PER_CHIP)) {
		pr_err("unregister_mac_dev faild, chip%d or mac%d invalid !\n",
			chip_id, mac_id);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	if (NULL == g_dsaf_dev[chip_id]) {
		pr_err("unregister_mac_dev faild, dsaf%d invalid !\n",
			chip_id);
		return HRD_COMMON_ERR_NULL_POINTER;
	}
	g_dsaf_dev[chip_id]->mac_dev[mac_id] = NULL;

	return 0;
}
EXPORT_SYMBOL(dsaf_unregister_mac_dev);

/**
 * dsaf_get_mac_dev - unregister mac dev
 * @dsaf_id: 0-1
 * @port_id: 0-5
 */
struct mac_device *dsaf_get_mac_dev(u8 dsaf_id, u8 port_id)
{
	u8 chip_id = dsaf_id;
	u8 mac_id = 0;

	/* DSAF-mode, port 0-5;	NON-DSAF-mode, just 1port, id = 0*/
	if ((chip_id >= DSAF_MAX_CHIP_NUM)
		|| (port_id >= (DSAF_MAX_PORT_NUM_PER_CHIP - 1))) {
		pr_err("bind_mac_dev faild, chip%d or port%d invalid !\n",
			chip_id, port_id);
		return NULL;
	}

	if (NULL == g_dsaf_dev[chip_id]) {
		pr_err("dsaf_bind_mac_dev faild, dsaf%d_dev invalid !\n",
			dsaf_id);
		return NULL;
	}

	if (g_dsaf_dev[chip_id]->dsaf_mode < DSAF_MODE_ENABLE)
		/* dsaf-mode : NIC ROCEE just vir mac, id is the lastone*/
		mac_id = DSAF_MAX_PORT_NUM_PER_CHIP - 1;
	else
		mac_id = port_id; /*non-dsaf-mode : mac_id == port_id*/

	if (NULL == g_dsaf_dev[chip_id]->mac_dev[mac_id]) {
		log_err(g_dsaf_dev[chip_id]->dev,
			"bind_mac_dev faild, dsaf%d mac%d_dev not register !\n",
			dsaf_id, mac_id);
		return NULL;
	}

	return g_dsaf_dev[chip_id]->mac_dev[mac_id];
}
EXPORT_SYMBOL(dsaf_get_mac_dev);

/**
 * dsaf_get_mac_vaddr - get mac addr
 * @dsaf_id: 0-1
 * @port_id: 0-5
 */
void *dsaf_get_mac_vaddr(u8 dsaf_id, u8 port_id)
{
	u8 chip_id = dsaf_id;
	u8 mac_id = port_id;

	/* DSAF-mode, port 0-5
		NON-DSAF-mode, just 1port, id = 0*/
	if ((chip_id >= DSAF_MAX_CHIP_NUM)
		|| (mac_id >= (DSAF_MAX_PORT_NUM_PER_CHIP - 1))) {
		pr_err("get_mac_vaddr faild, chip%d or port%d invalid !\n",
			chip_id, mac_id);
		return NULL;
	}

	if (NULL == g_dsaf_dev[chip_id]) {
		pr_err("get_mac_vaddr faild, dsaf%d_dev invalid !\n",
			dsaf_id);
		return NULL;
	}

	if (NULL == g_dsaf_dev[chip_id]->mac_dev[mac_id]) {
		log_err(g_dsaf_dev[chip_id]->dev,
			"get_mac_vaddr faild, dsaf%d mac%d_dev not register!\n",
			dsaf_id, mac_id);
		return NULL;
	}

	return g_dsaf_dev[chip_id]->mac_dev[mac_id]->vaddr;
}
EXPORT_SYMBOL(dsaf_get_mac_vaddr);

/**
 * dsaf_get_mac_vaddr - get work mode
 * @dsaf_id: 0-1
 * @mode: addr
 */
int dsaf_get_work_mode(u8 dsaf_id, enum dsaf_mode *mode)
{
	if (dsaf_id >= DSAF_MAX_CHIP_NUM) {
		pr_err("get_work_mode faild, dsaf_id%d invalid !\n", dsaf_id);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	if (NULL == g_dsaf_dev[dsaf_id]) {
		pr_err("get_work_mode faild, dsaf%d_dev invalid !\n", dsaf_id);
		return HRD_COMMON_ERR_NULL_POINTER;
	}

	*mode = g_dsaf_dev[dsaf_id]->dsaf_mode;

	return 0;
}
EXPORT_SYMBOL(dsaf_get_work_mode);

/**
 * dsaf_alloc_dev - alloc dev mem
 * @dev: struct device pointer
 * @sizeof_priv:
 */
static struct dsaf_device *dsaf_alloc_dev(struct device *dev,
	size_t sizeof_priv)
{
	struct dsaf_device *dsaf_dev;

	dsaf_dev = kzalloc(sizeof(*dsaf_dev) + sizeof_priv, GFP_KERNEL);
	if (unlikely(dsaf_dev == NULL))
		dsaf_dev = ERR_PTR(-ENOMEM);
	else {
		dsaf_dev->dev = dev;
		dev_set_drvdata(dev, dsaf_dev);
	}

	return dsaf_dev;
}

/**
 * dsaf_free_dev - free dev mem
 * @dev: struct device pointer
 */
static int dsaf_free_dev(struct dsaf_device *dsaf_dev)
{
	(void)dev_set_drvdata(dsaf_dev->dev, NULL);

	kfree(dsaf_dev);

	return 0;
}

/**
 * dsaf_drv_interrupt - int handle function
 * @irq: irq num
 * @dev_id: dev_id pointer
 */
irqreturn_t dsaf_drv_interrupt(int irq, void *dev_id)
{
	struct dsaf_device *dsaf_dev = (struct dsaf_device *)dev_id;

	if (NULL == (void *)dsaf_dev)
		return IRQ_NONE;

	if (NULL == dsaf_dev->dsaf_int_proc)
		return IRQ_NONE;

	dsaf_dev->dsaf_int_proc(dsaf_dev, irq);
	return IRQ_HANDLED;
}

/**
 * dsaf_get_chip_id -
 * @pdev: platform_device pointer
 */
int dsaf_get_chip_id(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	u32 chip_id = 0;
	int ret;

	/*find nic parent nood */
	np = of_get_parent(pdev->dev.of_node);
	if (np == NULL)
		return -EINVAL;

	/*fine chip-id, fi none return 0, singel chip*/
	ret = of_property_read_u32(np, "chip-id", &chip_id);
	if (ret != 0)
		return 0;

	if(chip_id >= DSAF_MAX_CHIP_NUM) {
		log_err(&pdev->dev, "chip_id error!\r\n");
		return -EINVAL;
	}
	else
		return chip_id;
}

/**
 * dsaf_irq_init -
 * @np: nood pointer
 * @dsaf_dev: dasf device pointer
 */
static int dsaf_irq_init(struct device_node *np, struct dsaf_device *dsaf_dev)
{
	int ret;
	
/** Modified by CHJ. hulk3.19 no ic_enable_msi*/
#if 0
	s32 irq_num = dsaf_dev->base_irq;
#endif

	u32 idx = 0;
	u32 fail_idx;

	for (idx = 0; idx < DSAF_IRQ_NUM; idx++) {

		/** Modified by CHJ. hulk3.19 no ic_enable_msi*/	
#if 0
		ret = ic_enable_msi(irq_num, &dsaf_dev->virq[idx]);
		if (ret ) {
			log_err(dsaf_dev->dev,
				"dsaf(%d) ic_enable_msi faild!irq=%d, ret=%d\n",
				dsaf_dev->chip_id, irq_num, ret);
			goto enable_msi_fail;
		}
#endif
		ret = dsaf_dev->virq[idx] = irq_of_parse_and_map(np, idx);
		if (!ret ) {
			log_err(dsaf_dev->dev,
				"dsaf(%d)irq_of_parse_and_map faild!irq=%d\n",
				dsaf_dev->chip_id, ret);
			goto enable_msi_fail;
		}

		ret = request_irq(dsaf_dev->virq[idx], dsaf_drv_interrupt,
			0, np->name, dsaf_dev);
		if (ret) {
			log_err(dsaf_dev->dev,
				"dsaf(%d) request_irq faild!  virq=%d, ret=%d\n",
				dsaf_dev->chip_id, 
				dsaf_dev->virq[idx], ret);
			goto request_irq_fail;
		}
		log_dbg(dsaf_dev->dev,
			"dsaf(%d) virq=%d idx=%d success!\n",
			dsaf_dev->chip_id,dsaf_dev->virq[idx], idx);
		//irq_num++;
	}

	return 0;

request_irq_fail:
/** Modified by CHJ. hulk3.19 no ic_enable_msi*/
#if 0
	ic_disable_msi(dsaf_dev->virq[idx]);
#endif
	dsaf_dev->virq[idx] = 0;

enable_msi_fail:
	for (fail_idx = 0; fail_idx < idx; fail_idx++) {
		free_irq(dsaf_dev->virq[fail_idx], dsaf_dev);
/** Modified by CHJ. hulk3.19 no ic_enable_msi*/
#if 0	
		ic_disable_msi(dsaf_dev->virq[fail_idx]);
#endif
		dsaf_dev->virq[fail_idx] = 0;
	}

	return ret;
}

/**
 * dsaf_irq_uninit -
 * @dsaf_dev: dasf device pointer
 */
static void dsaf_irq_uninit(struct dsaf_device *dsaf_dev)
{
	u32 idx = 0;

	log_info(dsaf_dev->dev, "func begin!\n");

	for (idx = 0; idx < DSAF_IRQ_NUM; idx++) {
		free_irq(dsaf_dev->virq[idx], dsaf_dev);
/** Modified by CHJ. hulk3.19 no ic_enable_msi*/		
#if 0		
		ic_disable_msi(dsaf_dev->virq[idx]);
#endif
		dsaf_dev->virq[idx] = 0;
	}
}

/**
 * dsaf_probe - probo dsaf dev
 * @pdev: dasf platform device
 */
static int dsaf_probe(struct platform_device *pdev)
{
	s32 irq = -1;
	int ret;
	s32 dev_id = -1;
	struct dsaf_device *dsaf_dev = NULL;
	u32 dsaf_mode;
	struct device_node *np = NULL;

	/*per chip 1 DSAF */
	dev_id = dsaf_get_chip_id(pdev);
	if(dev_id < 0)
		return -EINVAL;

	log_info(&pdev->dev, "dsaf(%d) func begin!\n", dev_id);

	np = pdev->dev.of_node;

	dsaf_dev = dsaf_alloc_dev(&pdev->dev, g_dsaf_sizeof_priv);
	if (IS_ERR(dsaf_dev)) {
		ret = PTR_ERR(dsaf_dev);
		log_err(&pdev->dev,
			"dsaf_probe dsaf_alloc_dev faild, dsaf%d ret = %#x!\n",
			dev_id, ret);
		return ret;
	}
	dsaf_dev->chip_id = dev_id;

	dsaf_dev->vaddr = of_iomap(np, 0);
	if (NULL == dsaf_dev->vaddr) {
		log_err(dsaf_dev->dev, "dsaf(%d) of_iomap fail!\n", dev_id);
		ret = -ENOMEM;
		goto iomap_fail;
	}

	/*int connect */
/**Modefied by CHJ. We do not need  dsaf_dev->base_irq  in hulk3.19.*/	
#if 0	
	ret = of_property_read_u32(np, "irq-num", (u32 *)&irq);
	if (ret != 0) {
		log_err(dsaf_dev->dev,
			"dsaf(%d) of_property_read_u32 irq-num fail, ret = %d!\n",
			dev_id, ret);
		goto read_irq_num_fail;
	}
	dsaf_dev->base_irq = irq;
#endif
	ret = dsaf_irq_init(np, dsaf_dev);
	if (ret) {
		log_err(dsaf_dev->dev,
		"dsaf_irq_init faild, dsaf%d ret = %#x\n", dev_id, ret);
		goto read_irq_num_fail;
	}

	ret = of_property_read_u32(np, "dsa-mode", &dsaf_mode);
	if (ret) {
		log_err(dsaf_dev->dev,
			"dsaf(%d) of_property_read_u32 dsa-mode fail, ret = %d!\n",
			dev_id, ret);
		goto read_dsa_mode_fail;
	}
	dsaf_dev->dsaf_mode = (enum dsaf_mode)dsaf_mode;
	log_info(dsaf_dev->dev, "dsaf(%d) dsaf_mode = %d\n", dev_id, dsaf_mode);

	/*init dev struct */
	ret = dsaf_config(dsaf_dev);
	if (ret) {
		log_err(dsaf_dev->dev,
			"dasf_probe dsaf_config faild, dsaf%d ret = %#x!\n",
			dsaf_dev->chip_id, ret);
		goto read_dsa_mode_fail;
	}

	if (NULL != dsaf_dev->dsaf_init) {
		/*hardware init */
		ret = dsaf_dev->dsaf_init(dsaf_dev);
		if (ret) {
			log_err(dsaf_dev->dev,
				"dsaf_probe dsaf_init faild, dsaf%d ret=%#x!\n",
				dsaf_dev->chip_id, ret);
			goto read_dsa_mode_fail;
		}
	}

	g_dsaf_dev[dsaf_dev->chip_id] = dsaf_dev;

	(void)dsaf_add_sysfs(dsaf_dev->dev);

	return 0;

read_dsa_mode_fail:
	dsaf_irq_uninit(dsaf_dev);

read_irq_num_fail:
	iounmap(dsaf_dev->vaddr);

iomap_fail:
	(void)dsaf_free_dev(dsaf_dev);

	return ret;
}

/**
 * dsaf_remove - remove dsaf dev
 * @pdev: dasf platform device
 */
static int dsaf_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct dsaf_device *dsaf_dev = NULL;

	dsaf_dev = (struct dsaf_device *)platform_get_drvdata(pdev);

	dsaf_del_sysfs(dsaf_dev->dev);

	if (NULL != dsaf_dev->dsaf_free) {
		/*if fail donnot return, continue free other*/
		ret = dsaf_dev->dsaf_free(dsaf_dev);
	}

	dsaf_irq_uninit(dsaf_dev);
	iounmap(dsaf_dev->vaddr);
	(void)dsaf_free_dev(dsaf_dev);
	return ret;
}

static const struct of_device_id g_dsaf_match[] = {
	{.compatible = "hisilicon,dsa"},
	{}
};

static struct platform_driver g_dsaf_driver = {
	.probe = dsaf_probe,
	.remove = dsaf_remove,
	.driver = {
		.name = DSAF_DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = g_dsaf_match,
	},
};

/**
 * dsaf_module_init -
 * void
 */
static int __init dsaf_module_init(void)
{
	int ret;

	pr_debug("enter dsaf_module_init!\n");

	ret = platform_driver_register(&g_dsaf_driver);
	if (ret) {
		pr_err("platform_driver_register faild, ret=%d!\n", ret);
		return ret;
	}

	return 0;
}

/**
 * dsaf_module_exit -
 * void
 */
static void __exit dsaf_module_exit(void)
{
	platform_driver_unregister(&g_dsaf_driver);

}

module_init(dsaf_module_init);
module_exit(dsaf_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DSAF main driver");
MODULE_VERSION(DSAF_MOD_VERSION);


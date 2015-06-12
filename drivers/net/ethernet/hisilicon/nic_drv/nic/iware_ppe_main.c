/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd														 */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfYF4ny0uVnnMBXVa8dqdK4VX8c+qg1bvdFi720Xwi4Y0ECwDivXsbEjfwBXoo5fltoDp
n1Xl2mCwKPVMSPchyhyHe5w+7I+H/pSHj97Lz8loFVga5kZmgS95niw1nDnBIA==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/*******************************************************************************

  Hisilicon network interface controller driver
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>

/* #include "hrd_module.h"
#include "hrd_typedef.h" */
#include "iware_log.h"
#include "iware_nic_main.h"
#include "iware_ppe_main.h"

static int ppe_common_probe(struct platform_device *pdev);
static int ppe_common_remove(struct platform_device *pdev);

static const struct of_device_id g_ppe_match[] = {
	{.compatible = "hisilicon,ppe"},
	{.compatible = "hisilicon,ppe-multi"},
	{.compatible = "hisilicon,ppe-single"},
	{}
};

static struct platform_driver g_ppe_common_driver = {
	.probe = ppe_common_probe,
	.remove = ppe_common_remove,
	.driver = {
		   .name = PPE_COMMON_DRIVER_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = g_ppe_match,
		   },
};

/**
 * ppe_common_probe - probe ppe common device
 * @pdev: platform device
 *
 * Return 0 on success, negative on failure
 */
static int ppe_common_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	int ret;
	u64 size = 0;
	phys_addr_t phy_addr;
	phys_addr_t tmp_addr;
	struct ppe_common_dev *ppe_common = NULL;
	int chip_id = 0;
	enum dsaf_mode dsaf_mode = DSAF_MODE_INVALID;
	u32 comm_index = 0;

	log_dbg(&pdev->dev, "func begin\n");

	chip_id = nic_get_chip_id(pdev);
	if(chip_id < 0) {
		return -EINVAL;
	}

	ret = dsaf_get_work_mode(chip_id, &dsaf_mode);
	if (ret) {
		log_err(&pdev->dev, "dsaf_get_work_mode fail, ret = %d!\r\n",
			ret);
		return -EINVAL;
	}

	np = pdev->dev.of_node;

	tmp_addr = (phys_addr_t) of_get_address(np, 0, &size, NULL);
	if (tmp_addr == 0) {
		log_err(&pdev->dev, "of_get_address fail!\r\n");
		return -EINVAL;
	}
	phy_addr = of_translate_address(np, (const void *)tmp_addr);
	if (OF_BAD_ADDR == phy_addr) {
		log_err(&pdev->dev, "of_translate_address fail!\r\n");
		return -EINVAL;
	}

	ppe_common =
	    (struct ppe_common_dev *) osal_kmalloc(sizeof(*ppe_common), GFP_KERNEL);
	if (NULL == ppe_common) {
		log_err(&pdev->dev, "osal_kmalloc fail!\r\n");
		return -ENOMEM;
	}
	ppe_common->base = phy_addr;
	ppe_common->vbase = ioremap(phy_addr, size);
	if (NULL == ppe_common->vbase) {
		log_err(&pdev->dev, "ioremap fail!\r\n");
		ret = -EINVAL;
		goto ioremap_fail;
	}

	ret = of_property_read_u32(np, "ppe-common-index", &comm_index);
	if (ret) {
		log_err(&pdev->dev, "no comm_index for ppe comm\n");
		goto init_fail;
	}
	ppe_common->comm_index = comm_index;

	if (of_device_is_compatible(np, "hisilicon,ppe-single"))
		ppe_common->ppe_mode = PPE_COMMON_MODE_SINGLE;
	else
		ppe_common->ppe_mode = PPE_COMMON_MODE_MULTI;

	log_info(&pdev->dev,
		 "ppe_mode=%d dsaf_mode=%d vbase=%#llx phy_addr=%#llx begin\n",
		 ppe_common->ppe_mode, dsaf_mode,
		 (u64) ppe_common->vbase, ppe_common->base);

	ppe_common->dsaf_mode = dsaf_mode;
	ppe_common->dev = &pdev->dev;
	spin_lock_init(&ppe_common->lock);

	ppe_set_common_ops(&ppe_common->ops);

	if (ppe_common->ops.init != NULL) {
		ret = ppe_common->ops.init(ppe_common);
		if (ret) {
			log_err(&pdev->dev, "ppe_common->ops.init fail!\r\n");
			goto init_fail;
		}
	}

	platform_set_drvdata(pdev, ppe_common);

	return 0;

init_fail:
	iounmap(ppe_common->vbase);

ioremap_fail:
	osal_kfree(ppe_common);
	return ret;
}

/**
 * ppe_common_remove - remove ppe common device
 * @pdev: platform device
 *
 * Return 0 on success, negative on failure
 */
static int ppe_common_remove(struct platform_device *pdev)
{
	struct ppe_common_dev *ppe_common = NULL;

	log_dbg(&pdev->dev, "func begin\n");

	ppe_common = platform_get_drvdata(pdev);
	if (NULL == ppe_common) {
		log_err(&pdev->dev, "ppe_common is NULL!\r\n");
		return -ENODEV;
	}

	if (ppe_common->ops.uninit != NULL)
		ppe_common->ops.uninit(ppe_common);

	platform_set_drvdata(pdev, NULL);

	iounmap(ppe_common->vbase);

	osal_kfree(ppe_common);

	return 0;
}

/**
 * ppe_commom_init - ppe common init
 *
 * Return 0 on success, negative on failure
 */
int ppe_commom_init(void)
{
	pr_debug("func begin\n");

	return platform_driver_register(&g_ppe_common_driver);
}

/**
 * ppe_get_commom_dev - get ppe common device
 * @nic_device: nic dev
 * @np: device node pointer
 *
 * Return 0 on success, negative on failure
 */
static int ppe_get_commom_dev(struct nic_device *nic_dev, struct device_node *np)
{
	struct platform_device *pdev = NULL;
	struct ppe_common_dev *ppe_common = NULL;

	log_dbg(&nic_dev->netdev->dev, "func begin\n");

	pdev = of_find_device_by_node(np);
	if (NULL == pdev) {
		log_err(&nic_dev->netdev->dev,
			"of_find_device_by_node fail!\r\n");
		return -ENODEV;
	}

	ppe_common = (struct ppe_common_dev *) platform_get_drvdata(pdev);
	if (NULL == ppe_common) {
		log_err(&nic_dev->netdev->dev,
			"platform_get_drvdata fail!\r\n");
		return -ENODATA;
	}
	nic_dev->ppe_common = ppe_common;

	return 0;
}

/**
 * ppe_commom_uninit - ppe common uninit
 */
void ppe_commom_uninit(void)
{
	platform_driver_unregister(&g_ppe_common_driver);
}

/**
 * ppe_init - ppe init
 * @pdev: platform device
 * @dsaf_mode: dsa fabric mode
 *
 * Return 0 on success, negative on failure
 */
int ppe_init(struct platform_device *pdev, enum dsaf_mode dsaf_mode)
{
	struct net_device *netdev = NULL;
	struct nic_device *nic_dev = NULL;
	struct ppe_device *ppe_device = NULL;
	struct ppe_device *new_ppe_dev = NULL;
	struct device_node *np = NULL;
	struct device_node *np_common = NULL;
	int ret;
	u32 irq_info = 0;
	u32 ppe_num = 0;
	u32 ppe_idx = 0;
	u32 fail_ppe_idx = 0;
	u64 size = 0;
	u64 phy_addr;
	u64 tmp_addr = 0;
	u64 ppe_common_addr;

	log_dbg(&pdev->dev, "func begin\n");

	netdev = platform_get_drvdata(pdev);
	nic_dev = (struct nic_device *)netdev_priv(netdev);

	np = of_parse_phandle(pdev->dev.of_node, "ppe-group-handle", 0);
	if (np == NULL) {
		log_err(&pdev->dev,
			"of_parse_phandle ppe-group-handle fail!\r\n");
		return -EINVAL;
	}

	np_common = of_get_parent(np);
	if (np_common == NULL) {
		log_err(&pdev->dev, "of_get_parent fail!\r\n");
		return -EINVAL;
	}

	ret = ppe_get_commom_dev(nic_dev, np_common);
	if (ret) {
		log_err(&pdev->dev, "ppe_commom_init fail, ret=%d!\r\n", ret);
		return ret;
	}
	nic_dev->ppe_common->ref_cnt++;

	ppe_common_addr = (u64) nic_dev->ppe_common->base;
	tmp_addr = (u64) of_get_address(np, 0, &size, NULL);
	if (tmp_addr == 0) {
		log_err(&pdev->dev, "of_get_address fail!\r\n");
		ret = -EINVAL;
		goto get_address_fail;
	}
	log_info(&pdev->dev,
		 "ppe_common_addr(%#llx) tmp_addr(%#llx) size(%#llx)!\r\n",
		 (u64) ppe_common_addr, tmp_addr, size);

	if (size < PPE_REG_OFFSET)
		ppe_num = 1;
	else
		ppe_num = size / PPE_REG_OFFSET;

	phy_addr = of_translate_address(np, (const void *)tmp_addr);
	if (OF_BAD_ADDR == phy_addr) {
		log_err(&pdev->dev, "of_translate_address fail!\r\n");
		ret = -EINVAL;
		goto get_address_fail;
	}

	new_ppe_dev = osal_kzalloc(sizeof(*new_ppe_dev), GFP_KERNEL);
	if (NULL == new_ppe_dev) {
		log_err(&pdev->dev, "osal_kzalloc fail!\r\n");
		ret = -ENOMEM;
		goto get_address_fail;
	}

	new_ppe_dev->comm_index = nic_dev->ppe_common->comm_index;
	new_ppe_dev->ppe_mode = nic_dev->ppe_common->ppe_mode;
	new_ppe_dev->netdev = netdev;
	nic_dev->ppe_device = new_ppe_dev;

	new_ppe_dev->vbase = ioremap(phy_addr, size);
	if (NULL == new_ppe_dev->vbase) {
		log_err(&pdev->dev,
			"ioremap phy_addr(%#llx) size(%#llx) fail!\r\n",
			phy_addr, size);
		ret = -EINVAL;
		goto loop_alloc_err;
	}

	ret = of_property_read_u32(np, "irq-num", &irq_info);
	if (ret) {
		log_err(&pdev->dev,
			"of_property_read_u32 irq-num fail, ret = %d!\r\n",
			ret);
		goto loop_alloc_err;
	}
	new_ppe_dev->base_irq = irq_info;
	new_ppe_dev->irq_num = PPE_REG_IRQ_OFFSET;

	new_ppe_dev->index
	    =
	    (phy_addr -
	     (ppe_common_addr - PPE_COMMON_REG_OFFSET)) / PPE_REG_OFFSET;

	log_info(&pdev->dev,
		 "index=%d irq=%d vbase=%#llx phy_addr=%#llx begin\n",
		 new_ppe_dev->index, irq_info, (u64) new_ppe_dev->vbase,
		 phy_addr);

	ppe_device = new_ppe_dev;

	for (ppe_idx = 1; ppe_idx < ppe_num; ppe_idx++) {
		new_ppe_dev = osal_kzalloc(sizeof(*new_ppe_dev), GFP_KERNEL);
		if (NULL == new_ppe_dev) {
			log_err(&pdev->dev, "osal_kzalloc fail!\r\n");
			ret = -ENOMEM;
			goto loop_alloc_err;
		}
		new_ppe_dev->vbase = (void __iomem *)((u64)ppe_device->vbase + PPE_REG_OFFSET);
		new_ppe_dev->base_irq = ppe_device->base_irq + PPE_REG_IRQ_OFFSET;
		new_ppe_dev->irq_num = PPE_REG_IRQ_OFFSET;
		new_ppe_dev->dsaf_mode = dsaf_mode;
		new_ppe_dev->index++;
		new_ppe_dev->ppe_mode = ppe_device->ppe_mode;
		new_ppe_dev->comm_index = ppe_device->comm_index;
		new_ppe_dev->netdev = netdev;
		ppe_device->next = new_ppe_dev;
		ppe_device = new_ppe_dev;
	}
	ppe_device->next = NULL;

	ppe_device = nic_dev->ppe_device;
	for (ppe_idx = 0; ppe_idx < ppe_num; ppe_idx++) {
		ppe_set_ops(&ppe_device->ops);

		if (ppe_device->ops.init != NULL) {
			ret = ppe_device->ops.init(ppe_device);
			if (ret) {
				log_err(&pdev->dev,
					"ppe_device->ops.init fail, nic_idx=%#x ppe_idx=%#x ret=%d!\r\n",
					nic_dev->index, ppe_idx, ret);
				goto init_fail;
			}
		}
		ppe_device = ppe_device->next;
	}

	return 0;

init_fail:
	ppe_device = nic_dev->ppe_device;
	for (fail_ppe_idx = 0; fail_ppe_idx < ppe_idx; fail_ppe_idx++) {
		if (ppe_device->ops.uninit != NULL)
			ppe_device->ops.uninit(ppe_device);

		ppe_device = ppe_device->next;
	}
	ppe_idx = ppe_num;

loop_alloc_err:
	ppe_device = nic_dev->ppe_device;
	if (ppe_device->vbase != NULL)
		iounmap(ppe_device->vbase);

	for (fail_ppe_idx = 0; fail_ppe_idx < ppe_idx; fail_ppe_idx++) {
		if (ppe_device != NULL) {
			new_ppe_dev = ppe_device->next;
			osal_kfree(ppe_device);
		}
		ppe_device = new_ppe_dev;
	}
	nic_dev->ppe_device = NULL;

get_address_fail:
	nic_dev->ppe_common->ref_cnt--;
	nic_dev->ppe_common = NULL;

	return ret;
}

/**
 * ppe_uninit - ppe uninit
 * @pdev: platform device
 */
void ppe_uninit(struct platform_device *pdev)
{
	struct net_device *netdev = NULL;
	struct nic_device *nic_dev = NULL;
	struct ppe_device *ppe_device = NULL;
	struct ppe_device *tmp_ppe_dev = NULL;
	void *vbase = NULL;

	log_dbg(&pdev->dev, "func begin\n");

	netdev = platform_get_drvdata(pdev);
	nic_dev = (struct nic_device *)netdev_priv(netdev);

	ppe_device = nic_dev->ppe_device;
	vbase = ppe_device->vbase;

	while (ppe_device) {
		if (ppe_device->ops.uninit != NULL)
			ppe_device->ops.uninit(ppe_device);

		tmp_ppe_dev = ppe_device->next;
		osal_kfree(ppe_device);

		ppe_device = tmp_ppe_dev;
	}
	if (vbase != NULL)
		iounmap(vbase);
}

/**
 * ppe_reset - ppe reset
 * @ppe_dev: ppe device
 */
void ppe_reset(struct ppe_device *ppe_dev)
{
	int idx;
	struct nic_device *nic_dev = NULL;
	struct ppe_common_dev *ppe_common = NULL;
	struct rcb_common_dev *rcb_common = NULL;
	struct nic_ring_pair * ring = NULL;

	nic_dev = (struct nic_device *)netdev_priv(ppe_dev->netdev);
	ppe_common = nic_dev->ppe_common;
	rcb_common = nic_dev->rcb_common;

	/* only single ppe mode support reset */
	if (PPE_COMMON_MODE_MULTI != ppe_dev->ppe_mode) {

		if (ppe_common->ops.init(ppe_common))
			log_err(nic_dev->dev, "reset ppe(%d) fail\n",
				nic_dev->gidx);

		if (rcb_common->ops.init(rcb_common))
			log_err(nic_dev->dev, "reset ppe(%d) fail\n",
				nic_dev->gidx);

		rcb_common->ops.set_port_coalesced_frames(rcb_common,
			nic_dev->index, nic_dev->coalesced_frames);

		rcb_common->ops.set_port_desc_cnt(rcb_common,
			nic_dev->index, nic_dev->desc_cnt);

		rcb_common->ops.set_port_timeout(rcb_common,
			nic_dev->index, nic_dev->time_out);

		if (ppe_dev->ops.init(ppe_dev))
			log_err(nic_dev->dev, "reset ppe(%d) fail\n",
				nic_dev->gidx);
	}

	for (idx=0; idx<nic_dev->ring_pair_num; idx++) {
		ring = nic_dev->ring[idx];
		rcb_reinit_ring(ring);
	}
	if (PPE_COMMON_MODE_MULTI != ppe_dev->ppe_mode)
		rcb_common->ops.init_commit(rcb_common);
}

/**
 * ppe_show_stat_by_port - show ppe statistics
 * @dev: device
 */
void ppe_show_stat_by_port(struct device *dev)
{
	struct net_device *netdev = (struct net_device *)dev_get_drvdata(dev);
	struct nic_device *nic_dev = NULL;
	struct ppe_device *ppe_device = NULL;

	nic_dev = netdev_priv(netdev);
	if (NULL == nic_dev) {
		log_err(dev, "nic_device is NULL!\n");
		return;
	}

	ppe_device = nic_dev->ppe_device;
	while (ppe_device) {
		if (ppe_device->ops.show_stat_by_port != NULL)
			ppe_device->ops.show_stat_by_port(ppe_device);
		ppe_device = ppe_device->next;
	}
}

/**
 * rcb_show_stat_by_port - show rcb statistics
 * @dev: device
 */
void rcb_show_stat_by_port(struct device *dev)
{
	struct net_device *netdev = (struct net_device *)dev_get_drvdata(dev);
	struct nic_device *nic_dev = NULL;

	struct nic_ring_pair *ring[NIC_MAX_RING_PAIR_NUM] = { 0 };
	int ring_pair_num = 0, ring_id = 0;

	nic_dev = netdev_priv(netdev);
	if (NULL == nic_dev) {
		log_err(dev, "nic_device is NULL!\n");
		return;
	}

	memcpy(ring, nic_dev->ring, sizeof(ring));
	ring_pair_num = nic_dev->ring_pair_num;
	for (ring_id = 0; ring_id < ring_pair_num; ring_id++) {
		rcb_show_tx_cnt(ring[ring_id]);
		rcb_show_rx_cnt(ring[ring_id]);
	}
}

/**
 * mac_show_stat_by_port - show mac statistics
 * @dev: device
 */
void mac_show_stat_by_port(struct device *dev)
{
	struct net_device *netdev = (struct net_device *)dev_get_drvdata(dev);
	struct nic_device *nic_dev = NULL;
	struct mac_device *mac_dev = NULL;

	nic_dev = netdev_priv(netdev);
	if (NULL == nic_dev) {
		log_err(dev, "nic_device is NULL!\n");
		return;
	}

	mac_dev = nic_dev->mac_dev;
	if (NULL == mac_dev) {
		log_err(dev, "mac_dev is NULL!\n");
		return;
	}

	(void)mac_show_statistics(mac_dev);
}

/**
 * ppe_show_sw_stat_by_port - show ppe software status
 * @ppe_device: ppe device
 */
void ppe_show_sw_stat_by_port(struct ppe_device *ppe_device)
{
	osal_printf("**********************ppe port info*********************");
	osal_printf("************************\r\n");
	osal_printf
	    ("index	         :%#18x  chip_id           :%#18x\r\n",
	     ppe_device->index, ppe_device->chip_id);
	osal_printf
	    ("vbase	         :%#18llx  base_irq	       :%#18x\r\n",
	     (u64) ppe_device->vbase, ppe_device->base_irq);
	osal_printf
	    ("virq           :%#18x  dsaf_mode         :%#18x\r\n",
	     ppe_device->virq, ppe_device->dsaf_mode);
	osal_printf
	    ("ppe_mode	     :%#18x  comm_index        :%#18x\r\n",
	     ppe_device->ppe_mode, ppe_device->comm_index);
	osal_printf
	    ("init	         :%#18llx  uninit          :%#18llx\r\n",
	     (u64) ppe_device->ops.init, (u64) ppe_device->ops.uninit);
	osal_printf
	    ("checksum_hw	 :%#18llx  get_regs		:%#18llx\r\n",
	     (u64) ppe_device->ops.checksum_hw, (u64) ppe_device->ops.get_regs);
	osal_printf
	    ("get_strings    :%#18llx  get_sset_count  :%#18llx\r\n",
	     (u64) ppe_device->ops.get_strings, (u64) ppe_device->ops.get_sset_count);
	osal_printf("show_stat_by_port :%#18llx  get_ethtool_stat:%#18llx\r\n",
		    (u64) ppe_device->ops.show_stat_by_port,
		    (u64) ppe_device->ops.get_ethtool_stats);
}

/**
 * ppe_common_show_sw_info - show ppe common software status
 * @ppe_commom: ppe common device
 */
void ppe_common_show_sw_info(struct ppe_common_dev *ppe_commom)
{
	osal_printf("**********************ppe commom info******************");
	osal_printf("*************************\r\n");
	osal_printf
	    ("base           :%#18llx  vbase          :%#18llx\r\n",
	     ppe_commom->base, (u64) ppe_commom->vbase);
	osal_printf
	    ("dsaf_mode      :%#18x  ppe_mode         :%#18x\r\n",
	     ppe_commom->dsaf_mode, ppe_commom->ppe_mode);
	osal_printf
	    ("ref_cnt        :%#18x  comm_index       :%#18x\r\n",
	     ppe_commom->ref_cnt, ppe_commom->comm_index);
	osal_printf
	    ("init           :%#18llx  uninit         :%#18llx\r\n",
	     (u64) ppe_commom->ops.init, (u64) ppe_commom->ops.uninit);
}

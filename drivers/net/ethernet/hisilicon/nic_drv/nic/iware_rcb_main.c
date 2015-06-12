/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe484J2KJSpUtf6QncJkB3pc32W8uIChje8Oojj6DX1+tva7XJa/G4BrHY9P5TrsPtnXkp9
2rvoGqyOzVrcA24eQln40ixytMkTWhot4h0Wkidu7+Z46dskgWwOztedtYngeQ==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/************************************************************************

  Hisilicon NIC driver
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

************************************************************************/
#include <linux/cdev.h>		/*struct cdev */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <asm/cacheflush.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
//#include <linux/irqchip/hisi-msi-gen.h>
#include <linux/moduleparam.h>

#include "iware_error.h"
#include "iware_log.h"
#include "iware_nic_main.h"
#include "iware_rcb_main.h"

int rcb_irq_init(struct nic_ring_pair *ring);
void rcb_irq_uninit(struct nic_ring_pair *ring);


static int rcb_common_probe(struct platform_device *pdev);
static int rcb_common_remove(struct platform_device *pdev);

static const struct of_device_id g_rcb_match[] = {
	{.compatible = "hisilicon,rcb"},
	{.compatible = "hisilicon,rcb-multi"},
	{.compatible = "hisilicon,rcb-single"},
	{}
};

static struct platform_driver g_rcb_common_driver = {
	.probe = rcb_common_probe,
	.remove = rcb_common_remove,
	.driver = {
		   .name = RCB_COMMON_DRIVER_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = g_rcb_match,
		   },
};

/**
 *rcb_irq_enable - enable rcb interrupt
 *@ring: rcb ring
 *@flag:ring flag fro tx or rx
 */
void rcb_irq_enable(struct nic_ring_pair *ring, u32 flag)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	if (rcb_dev->ops.int_ctrl != NULL)
		rcb_dev->ops.int_ctrl(rcb_dev, flag, ENABLE);
}

/**
 *rcb_irq_disable - disable rcb interrupt
 *@ring: rcb ring
 *@flag:ring flag fro tx or rx
 */
void rcb_irq_disable(struct nic_ring_pair *ring, u32 flag)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	if (rcb_dev->ops.int_ctrl != NULL)
		rcb_dev->ops.int_ctrl(rcb_dev, flag, DISABLE);

}

/**
 *rcb_common_probe - probe common rcb device and initialize
 *@pdev: platform device
 *retuen 0 - sucess , negative --fail
 */
static int rcb_common_probe(struct platform_device *pdev)
{
	struct device_node *np = NULL;
	int ret = 0;
	u64 size = 0;
	phys_addr_t phy_addr;
	phys_addr_t tmp_addr;
	struct rcb_common_dev *rcb_common = NULL;
	int chip_id = 0;
	enum dsaf_mode dsaf_mode = DSAF_MODE_INVALID;

	log_info(&pdev->dev, "func begin\n");

	chip_id = nic_get_chip_id(pdev);
	if(chip_id < 0) {
		return -EINVAL;
	}

	ret = dsaf_get_work_mode(chip_id, &dsaf_mode);
	if (ret != 0) {
		log_err(&pdev->dev,
			"dsaf_get_work_mode fail, ret = %d!\r\n", ret);
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

	rcb_common =
	    (struct rcb_common_dev *)osal_kmalloc(sizeof(*rcb_common),
						     GFP_KERNEL);
	if (NULL == rcb_common) {
		log_err(&pdev->dev, "osal_kmalloc fail!\r\n");
		return -ENOMEM;
	}
	ret = of_property_read_u32(np, "irq-num", (u32*)&rcb_common->base_irq);
	if (ret < 0) {
		log_err(&pdev->dev,
			"of_property_read_u32 irq-num fail, ret=%d!\r\n", ret);
		goto get_irp_num_fail;
	}
	rcb_common->irq_num = RCB_COMMON_IRQ_OFFSET;

	rcb_common->base = phy_addr;
	rcb_common->vbase = ioremap(phy_addr, size);
	if (NULL == rcb_common->vbase) {
		log_err(&pdev->dev, "ioremap fail!\r\n");
		ret = -EINVAL;
		goto ioremap_fail;
	}

	rcb_common->dsaf_mode = dsaf_mode;
	spin_lock_init(&rcb_common->lock);

	rcb_set_common_ops(&rcb_common->ops);

	if (rcb_common->ops.init != NULL) {
		ret = rcb_common->ops.init(rcb_common);
		if (ret != 0) {
			log_err(&pdev->dev, "rcb_common->ops.init fail!\r\n");
			goto init_fail;
		}
	}

	platform_set_drvdata(pdev, rcb_common);

	return 0;

init_fail:
	iounmap(rcb_common->vbase);

get_irp_num_fail:
ioremap_fail:
	osal_kfree(rcb_common);
	return ret;
}

/**
 *rcb_common_remove - remove common rcb
 *@pdev: platform device
 *retuen status
 */
static int rcb_common_remove(struct platform_device *pdev)
{
	struct rcb_common_dev *rcb_common = NULL;

	log_dbg(&pdev->dev, "func begin\n");

	rcb_common = (struct rcb_common_dev *)platform_get_drvdata(pdev);
	if (NULL == rcb_common) {
		log_err(&pdev->dev, "rcb_common is NULL!\r\n");
		return -ENODEV;
	}

	if (rcb_common->ops.uninit != NULL)
		rcb_common->ops.uninit(rcb_common);

	platform_set_drvdata(pdev, NULL);

	iounmap(rcb_common->vbase);

	osal_kfree(rcb_common);

	return 0;
}

/**
 *rcb_get_commom_dev - get commom rcb dev
 *@nic_device: nic device
 *@np : device node
 *retuen status
 */
static int rcb_get_commom_dev(struct nic_device *nic_dev,
	struct device_node *np)
{
	struct platform_device *pdev = NULL;
	struct rcb_common_dev *rcb_common = NULL;
	u32 port_idx;

	pdev = of_find_device_by_node(np);
	if (NULL == pdev) {
		log_err(&nic_dev->netdev->dev, "of_find_device_by_node fail!\r\n");
		return -ENODEV;
	}

	rcb_common = (struct rcb_common_dev *)platform_get_drvdata(pdev);
	if (NULL == rcb_common) {
		log_err(&nic_dev->netdev->dev, "platform_get_drvdata fail!\r\n");
		return -ENODATA;
	}

	port_idx = nic_dev->index;
	if (rcb_common->ops.set_port_desc_cnt != NULL)
		rcb_common->ops.set_port_desc_cnt(rcb_common, port_idx,
						  nic_dev->desc_cnt);

	if (rcb_common->ops.set_port_coalesced_frames != NULL)
		rcb_common->ops.set_port_coalesced_frames(rcb_common, port_idx,
							  nic_dev->coalesced_frames);

	if (rcb_common->ops.set_port_timeout != NULL)
		rcb_common->ops.set_port_timeout(rcb_common, port_idx,
						 nic_dev->time_out);

	nic_dev->rcb_common = rcb_common;

	return 0;
}

/**
 *rcb_commom_init - init rcb common
 *retuen status
 */
int rcb_commom_init(void)
{
	return platform_driver_register(&g_rcb_common_driver);
}

/**
 *rcb_commom_init_commit - get commom rcb dev
 *retuen status
 */
int rcb_commom_init_commit(void)
{
	struct device_node *np = NULL;
	struct platform_device *pdev = NULL;
	struct rcb_common_dev *rcb_common = NULL;

	for_each_compatible_node(np, NULL, "hisilicon,rcb") {
		pdev = of_find_device_by_node(np);
		if (NULL == pdev) {
			pr_err("of_find_device_by_node fail!\r\n");
			return -ENODEV;
		}
		rcb_common =
		    (struct rcb_common_dev *)platform_get_drvdata(pdev);
		if ((NULL != rcb_common)
		    && (rcb_common->ops.init_commit != NULL))
			rcb_common->ops.init_commit(rcb_common);
		else {
			pr_err("rcb_common is NULL!\r\n");
			return -ENODEV;
		}
	}

	return 0;
}

/**
 *rcb_commom_uninit - remove commom rcb dev
 *@nic_device: nic device
 *@np : device node
 *retuen status
 */
void rcb_commom_uninit(void)
{
	platform_driver_unregister(&g_rcb_common_driver);
}

/**
 *rcb_get_queue_mode - get max VM number and max ring number per VM
 *						accordding to dsaf mode
 *@dsaf_mode: dsaf mode
 *@max_vfn : max vfn number
 *@max_q_per_vf:max ring number per vm
 *retuen status
 */
static void rcb_get_queue_mode(enum dsaf_mode dsaf_mode, u16 *max_vfn,
			       u16 *max_q_per_vf)
{
	switch (dsaf_mode) {
	case DSAF_MODE_DISABLE_6PORT_0VM:
		*max_vfn = 1;
		*max_q_per_vf = 16;
		break;
	case DSAF_MODE_DISABLE_FIX:
		*max_vfn = 1;
		*max_q_per_vf = 1;
		break;
	default:
		*max_vfn = 1;
		*max_q_per_vf = 16;
		break;
	}
}

static int g_bd_num = 0;
static int g_buf_size = 0;
static int g_pkt_line = 0;
static int g_over_time = 0;
module_param(g_bd_num, int, S_IRUGO);
module_param(g_buf_size, int, S_IRUGO);
module_param(g_pkt_line, int, S_IRUGO);
module_param(g_over_time, int, S_IRUGO);
/**
 *rcb_get_info - get rcb information
 *@np:device node
 *@nic_device:nic device
 *@dsaf_mode: dsaf mode
 *@max_vfn : max vfn number
 *@max_q_per_vf:max ring number per vm
 *return status
 */
static int rcb_get_info(struct device_node *np, struct nic_device *nic_dev,
			struct nic_ring_pair *ring, enum dsaf_mode dsaf_mode)
{
	int ret = 0;
	u32 irq_info = 0;
	u32 ring_pair_num = 0;
	u64 size = 0;
	phys_addr_t phy_addr;
	phys_addr_t tmp_addr;
	u64 rcb_common_addr;
	u16 max_vfn = 0;
	u16 max_q_per_vf = 0;
	u32 desc_cnt = 0;
	u32 buf_size = 0;
	struct device_node *np_common = NULL;

	if (((u32)g_bd_num >= RCB_COMMON_MIN_DESC_CNT)
	    && ((u32)g_bd_num <= RCB_COMMON_MAX_DESC_CNT))
		desc_cnt = (u32)g_bd_num;
	else {
	ret = of_property_read_u32(np, "desc-num", &desc_cnt);
	if (ret < 0) {
		log_err(ring->dev,
			"of_property_read_u16 desc-num fail, ret=%d!\r\n", ret);
		return ret;
	}
	if ((desc_cnt < RCB_COMMON_MIN_DESC_CNT)
	    || (desc_cnt > RCB_COMMON_MAX_DESC_CNT)) {
		log_err(ring->dev,
			"of_property_read_u32 rcb-bd-num error, desc_cnt=%d!\r\n",
			desc_cnt);
		return -EINVAL;
	}
	}
	nic_dev->desc_cnt = (u16) desc_cnt;
	nic_dev->coalesced_frames = RCB_MAX_COALESCED_FRAMES;
	nic_dev->time_out = RCB_MAX_TIME_OUT;
	if (((u32)g_pkt_line) && ((u32)g_pkt_line <= 0x3FF))
		nic_dev->coalesced_frames = (u16)g_pkt_line;
	if ((u32)g_over_time)
		nic_dev->time_out = (u32)g_over_time;

	np_common = of_get_parent(np);
	if (np_common == NULL) {
		log_err(ring->dev, "of_get_parent fail!\r\n");
		return -EINVAL;
	}
	ret = rcb_get_commom_dev(nic_dev, np_common);
	if (ret != 0) {
		log_err(ring->dev,
			"rcb_get_commom_dev fail, ret=%d!\r\n", ret);
		return ret;
	}

	rcb_common_addr = nic_dev->rcb_common->base;
	tmp_addr = (phys_addr_t) of_get_address(np, 0, &size, NULL);
	if (tmp_addr == 0) {
		log_err(ring->dev, "of_get_address fail!\r\n");
		return -EINVAL;
	}
	phy_addr = of_translate_address(np, (const void *)tmp_addr);
	if (OF_BAD_ADDR == phy_addr) {
		log_err(ring->dev, "of_translate_address fail!\r\n");
		return -EINVAL;
	}
	log_info(ring->dev,
       "desc_cnt(%d) rcb_common_addr(%#llx) phy_addr(%#llx) size(%#llx)\n",
       desc_cnt, rcb_common_addr, phy_addr, size);

	rcb_get_queue_mode(dsaf_mode, &max_vfn, &max_q_per_vf);
	ring_pair_num = size / RCB_REG_OFFSET;
	if ((ring_pair_num > max_vfn * max_q_per_vf)
	    || ((ring_pair_num % max_q_per_vf) != 0)) {
		log_err(ring->dev,
			"ring_pair_num=%#x,max_vfn=%#x,max_q_per_vf=%#x,dsaf_mode=%#x\r\n",
			ring_pair_num, max_vfn, max_q_per_vf, dsaf_mode);
		return -EINVAL;
	}

	nic_dev->max_vfn = max_vfn;
	nic_dev->max_q_per_vf = max_q_per_vf;
	nic_dev->ring_pair_num = ring_pair_num;

	log_info(ring->dev,
		       "max_vfn(%d) max_q_per_vf(%d) ring_pair_num(%d) dsaf_mode(%d)\n",
		       max_vfn, max_q_per_vf, ring_pair_num, dsaf_mode);

	ring->rcb_dev.base = phy_addr;
	ring->tx_ring.count = desc_cnt;
	ring->rx_ring.count = desc_cnt;

	ret = of_property_read_u32(np, "irq-num", &irq_info);
	if (ret != 0) {
		log_err(ring->dev,
			"of_property_read_u32 irq-num fail, ret = %d!\r\n", ret);
		return ret;
	}
	if ((512 == (u32)g_buf_size)
		|| (1024 == (u32)g_buf_size)
		|| (2048 == (u32)g_buf_size)
		|| (4096 == (u32)g_buf_size))
		buf_size = (u32)g_buf_size;
	else {
	ret = of_property_read_u32(np, "buf-size", &buf_size);
	if (ret != 0) {
		log_err(ring->dev,
			"of_property_read_u32 buf-size fail, ret = %d!\r\n", ret);
		return ret;
		}
	}
	ring->rcb_dev.buf_size = (u16) buf_size;
	ring->rcb_dev.base_irq = irq_info;
	ring->rcb_dev.index = (phy_addr -
	     (rcb_common_addr + RCB_COMMON_REG_SIZE)) / RCB_REG_OFFSET;

	log_info(ring->dev, "buf_size(%d) irq_info(%d) index(%d)\n",
		buf_size, irq_info, ring->rcb_dev.index);

	if(ring->rcb_dev.index % ring_pair_num != 0)
		return -EINVAL;
	else
		return 0;
}

/**
 *rcb_init - init rcb
 *@pdev:platform_device
 *@dsaf_mode:dsaf mode
 *retuen status
 */
int rcb_init(struct platform_device *pdev, enum dsaf_mode dsaf_mode)
{
	struct net_device *netdev = NULL;
	struct nic_device *nic_dev = NULL;
	struct rcb_device *rcb_dev = NULL;
	struct nic_ring_pair *ring = NULL;
	struct nic_ring_pair *new_ring = NULL;
	struct device_node *np = NULL;
	int ret;
	u32 ring_pair_num = 0;
	u32 ring_id = 0;
	u32 fail_ring_id = 0;
	u32 port_idx;

	log_info(&pdev->dev, "rcb_init func begin\n");

	netdev = platform_get_drvdata(pdev);
	nic_dev = (struct nic_device *)netdev_priv(netdev);

	np = of_parse_phandle(pdev->dev.of_node, "rcb-group-handle", 0);
	if (np == NULL) {
		log_err(&pdev->dev,
			"of_parse_phandle rcb-group-handle fail!\r\n");
		return -EINVAL;
	}

	new_ring = osal_kzalloc(sizeof(struct nic_ring_pair), GFP_KERNEL);
	if (NULL == new_ring) {
		log_err(&pdev->dev, "osal_kzalloc fail!\r\n");
		ret = -ENOMEM;
		goto get_address_fail;
	}
	nic_dev->ring[ring_id] = new_ring;
	new_ring->netdev = netdev;
	new_ring->dev = &pdev->dev;
	new_ring->max_buf_num = RCB_RING_MAX_BD_PER_PKT;
	port_idx = (u8) nic_dev->index;

   /** Added by CHJ. hulk3.19 no ic_enable_msi.*/
	new_ring->rcb_dev.virq[RCB_IRQ_IDX_TX] = irq_of_parse_and_map(np, 0);
	new_ring->rcb_dev.virq[RCB_IRQ_IDX_RX] = irq_of_parse_and_map(np, 1);

	new_ring->rcb_dev.irq_init_flag = RCB_IRQ_NOT_INITED;

	if (nic_dev->index >= DSAF_PPE_INODE_BASE)
		port_idx = 0;

	new_ring->rcb_dev.port_id_in_dsa = port_idx;

	ret = rcb_get_info(np, nic_dev, new_ring, dsaf_mode);
	if (ret != 0) {
		log_err(new_ring->dev, "rcb_get_info fail, ret=%d!\r\n", ret);
		goto loop_alloc_err;
	}
	nic_dev->rcb_common->ref_cnt++;

	new_ring->rcb_dev.vbase =
	    ioremap(new_ring->rcb_dev.base, RCB_REG_OFFSET);
	if (NULL == new_ring->rcb_dev.vbase) {
		log_err(new_ring->dev, "ioremap fail, ret=%d!\r\n", ret);
		goto loop_alloc_err;
	}

	ring_pair_num = nic_dev->ring_pair_num;
	new_ring->rcb_dev.queue_index = 0;
	ring = new_ring;

	for (ring_id = 1; ring_id < ring_pair_num; ring_id++) {
		new_ring = osal_kzalloc(sizeof(*new_ring), GFP_KERNEL);
		if (NULL == new_ring) {
			log_err(&pdev->dev,
				"osal_kzalloc fail, nic_idx=%#x ring_id=%#x!\r\n",
				nic_dev->index, ring_id);
			ret = -ENOMEM;
			goto loop_alloc_err;
		}
		nic_dev->ring[ring_id] = new_ring;
		new_ring->netdev = netdev;
		new_ring->dev = &pdev->dev;
		new_ring->max_buf_num = RCB_RING_MAX_BD_PER_PKT;
		new_ring->rcb_dev.index = ring->rcb_dev.index + 1;
		new_ring->rcb_dev.queue_index = ring_id;
		new_ring->tx_ring.count = ring->tx_ring.count;
		new_ring->rx_ring.count = ring->rx_ring.count;
		new_ring->rcb_dev.buf_size = ring->rcb_dev.buf_size;
		new_ring->rcb_dev.port_id_in_dsa = port_idx;
		new_ring->rcb_dev.base = ring->rcb_dev.base + RCB_REG_OFFSET;
		new_ring->rcb_dev.vbase
			= ioremap(ring->rcb_dev.base + RCB_REG_OFFSET,
			RCB_REG_OFFSET);
		if (NULL == new_ring->rcb_dev.vbase) {
			log_err(new_ring->dev,
				"ioremap fail, nic_idx=%#x ring_id=%#x!\r\n",
				nic_dev->index, new_ring->rcb_dev.index);
			osal_kfree(new_ring);
			goto loop_alloc_err;
		}

#if 0
		new_ring->rcb_dev.base_irq =
		    ring->rcb_dev.base_irq + RCB_IRQ_OFFSET;
#endif
		/** Added by CHJ. hulk3.19 no ic_enable_msi.*/
		new_ring->rcb_dev.virq[RCB_IRQ_IDX_TX] = irq_of_parse_and_map(np, ring_id *2);
		new_ring->rcb_dev.virq[RCB_IRQ_IDX_RX] = irq_of_parse_and_map(np, ring_id *2 + 1);

		new_ring->rcb_dev.irq_init_flag = RCB_IRQ_NOT_INITED;

		ring = new_ring;
	}

	for (ring_id = 0; ring_id < ring_pair_num; ring_id++) {
		ring = nic_dev->ring[ring_id];
		rcb_dev = &ring->rcb_dev;

		rcb_set_ops(&rcb_dev->ops);

		if (rcb_dev->ops.init != NULL) {
			ret = rcb_dev->ops.init(ring);
			if (ret != 0) {
				log_err(&pdev->dev,
					"rcb_dev init fail, nic_idx=%#x rcb_idx=%#x ret=%d!\r\n",
					nic_dev->index, rcb_dev->index, ret);
				goto init_fail;
			}

			netif_napi_add(netdev, &ring->rx_ring.napi,
				       nic_rx_napi_poll, NIC_RX_CLEAN_MAX_NUM);
			netif_napi_add(netdev, &ring->tx_ring.napi,
				       nic_tx_napi_poll, 64);

		}
	}

	return 0;

init_fail:
	for (fail_ring_id = 0; fail_ring_id < ring_id; fail_ring_id++) {
		rcb_dev = &nic_dev->ring[fail_ring_id]->rcb_dev;
		if (rcb_dev->ops.uninit != NULL)
			rcb_dev->ops.uninit(ring);
	}
	ring_id = ring_pair_num;

loop_alloc_err:
	for (fail_ring_id = 0; fail_ring_id < ring_id; fail_ring_id++) {
		ring = nic_dev->ring[fail_ring_id];
		if (ring != NULL) {
			rcb_dev = &ring->rcb_dev;
			if (rcb_dev->vbase != NULL)
				iounmap(rcb_dev->vbase);

			netif_napi_del(&ring->rx_ring.napi);
			osal_kfree(ring);
			nic_dev->ring[fail_ring_id] = NULL;
		}
	}

get_address_fail:
	nic_dev->rcb_common->ref_cnt--;
	nic_dev->rcb_common = NULL;

	return ret;
}

/**
 *rcb_uninit - uninit rcb
 *@pdev:platform_device
 *retuen status
 */
void rcb_uninit(struct platform_device *pdev)
{
	u32 ring_id = 0;
	struct net_device *netdev = NULL;
	struct nic_device *nic_dev = NULL;
	struct rcb_device *rcb_dev = NULL;
	struct nic_ring_pair *ring = NULL;

	log_dbg(&pdev->dev, "func begin\n");

	netdev = platform_get_drvdata(pdev);
	nic_dev = (struct nic_device *)netdev_priv(netdev);

	for (ring_id = 0; ring_id < (u32)nic_dev->ring_pair_num; ring_id++) {
		ring = nic_dev->ring[ring_id];
		if (ring != NULL) {
			rcb_irq_uninit(ring);
			rcb_dev = &ring->rcb_dev;
			if (rcb_dev->ops.uninit != NULL)
				rcb_dev->ops.uninit(ring);

			if (rcb_dev->vbase != NULL)
				iounmap(rcb_dev->vbase);

			netif_napi_del(&ring->rx_ring.napi);
			osal_kfree(ring);
			nic_dev->ring[ring_id] = NULL;
		}
	}
}

void rcb_reinit_ring(struct nic_ring_pair *ring)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	rcb_dev->ops.uninit(ring);

	(void)rcb_dev->ops.init(ring);
}

void rcb_enable_test_ring(struct nic_ring_pair *ring)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	(void)rcb_dev->ops.enable(ring);
}
void rcb_disable_test_ring(struct nic_ring_pair *ring)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	(void)rcb_dev->ops.disable(ring);
}


/**
 *nic_tx_irq_handle - get nic tx interrupt handle
 *@irq:irq
 *@dev:irq dev
 *retuen irq dev
 */
irqreturn_t nic_tx_irq_handle(int irq, void *dev)
{
	struct nic_ring_pair *ring = (struct nic_ring_pair *)dev;

	log_dbg(ring->dev, "func begin\n");

	rcb_irq_disable(ring, RCB_INT_FLAG_TX);

	napi_schedule(&ring->tx_ring.napi);

	return IRQ_HANDLED;
}

/**
 *nic_rx_irq_handle - get nic rx interrupt handle
 *@irq:irq
 *@dev:irq dev
 *retuen irq dev
 */
irqreturn_t nic_rx_irq_handle(int irq, void *dev)
{
	struct nic_ring_pair *ring = (struct nic_ring_pair *)dev;

	rcb_irq_disable(ring, RCB_INT_FLAG_RX);

	napi_schedule(&ring->rx_ring.napi);

	return IRQ_HANDLED;
}

/**
 *rcb_irq_init - init rcb irq
 *@ring:rcb ring
 *retuen status
 */
int rcb_irq_init(struct nic_ring_pair *ring)
{
	int ret = 0;
	struct rcb_device *rcb_dev = &ring->rcb_dev;
	s32 irq_num = rcb_dev->base_irq;

/** Modified by CHJ. hulk3.19 no ic_enable_msi*/
#if 0
	ret = ic_enable_msi(irq_num, &rcb_dev->virq[RCB_IRQ_IDX_TX]);
	if (ret != 0) {
		log_err(ring->dev,
			"ic_enable_msi faild! irq = %d,ret = %d\n", irq_num, ret);
		return ret;
	}
#endif
	if (rcb_dev->irq_init_flag == RCB_IRQ_NOT_INITED) {

		memset(ring->rcb_dev.irq_name[RCB_IRQ_IDX_TX], 0, RCB_IRQ_NAME_LEN);

		sprintf(ring->rcb_dev.irq_name[RCB_IRQ_IDX_TX], "%sTx%d",
			ring->netdev->name, ring->rcb_dev.index);

		ret = request_irq(rcb_dev->virq[RCB_IRQ_IDX_TX], nic_tx_irq_handle,
				  0, ring->rcb_dev.irq_name[RCB_IRQ_IDX_TX], ring);
		if (ret != 0) {
			log_err(ring->dev,
				"request_irq faild! irq=%d, virq=%d, ret=%d\n",
				irq_num, rcb_dev->virq[RCB_IRQ_IDX_TX], ret);
			goto request_tx_irq_fail;
		}
		log_dbg(ring->dev, "request_irq %d virq=%d success!\n",
			irq_num, rcb_dev->virq[RCB_IRQ_IDX_TX]);

/** Modified by CHJ. hulk3.19 no ic_enable_msi*/
#if 0
	irq_num = irq_num + RCB_IRQ_TX_RX_OFFSET;
	ret = ic_enable_msi(irq_num, &rcb_dev->virq[RCB_IRQ_IDX_RX]);
	if (ret != 0) {
		log_err(ring->dev,
			"ic_enable_msi faild! irq = %d, ret = %d\n", irq_num, ret);
		goto enable_msi_rx_fail;
	}
#endif

		memset(ring->rcb_dev.irq_name[RCB_IRQ_IDX_RX], 0, RCB_IRQ_NAME_LEN);
		sprintf(ring->rcb_dev.irq_name[RCB_IRQ_IDX_RX], "%sRx%d",
			ring->netdev->name, ring->rcb_dev.index); /* TBD */
		ret =
		    request_irq(rcb_dev->virq[RCB_IRQ_IDX_RX], nic_rx_irq_handle, 0,
				ring->rcb_dev.irq_name[RCB_IRQ_IDX_RX], ring);
		if (ret != 0) {
			log_err(ring->dev,
				"request_irq faild! irq=%d, virq=%d, ret=%d\n",
				irq_num, rcb_dev->virq[RCB_IRQ_IDX_RX], ret);
			goto request_rx_irq_fail;
		}
		log_dbg(ring->dev, "request_irq %d virq=%d success!\n",
			irq_num, rcb_dev->virq[RCB_IRQ_IDX_RX]);

		rcb_dev->irq_init_flag = RCB_IRQ_INITED;
	}
	return 0;

request_rx_irq_fail:
/** Modified by CHJ. hulk3.19 no ic_enable_msi*/
#if 0
	ic_disable_msi(rcb_dev->virq[RCB_IRQ_IDX_RX]);
#endif
	rcb_dev->virq[RCB_IRQ_IDX_RX] = 0;
enable_msi_rx_fail:
	free_irq(rcb_dev->virq[RCB_IRQ_IDX_TX], ring);

request_tx_irq_fail:
/** Modified by CHJ. hulk3.19 no ic_enable_msi*/
#if 0
	ic_disable_msi(rcb_dev->virq[RCB_IRQ_IDX_TX]);
#endif
	rcb_dev->virq[RCB_IRQ_IDX_TX] = 0;

	return ret;
}

/**
 *rcb_irq_uninit - uninit rcb irq
 *@ring:rcb ring
 *retuen status
 */
void rcb_irq_uninit(struct nic_ring_pair *ring)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	log_dbg(ring->dev, "func begin\n");

	if (rcb_dev->virq[RCB_IRQ_IDX_TX])
		free_irq(rcb_dev->virq[RCB_IRQ_IDX_TX], ring);
/** Modified by CHJ. hulk3.19 no ic_enable_msi*/
#if 0
	ic_disable_msi(rcb_dev->virq[RCB_IRQ_IDX_TX]);
#endif
	//rcb_dev->virq[RCB_IRQ_IDX_TX] = 0;

	if (rcb_dev->virq[RCB_IRQ_IDX_RX])
		free_irq(rcb_dev->virq[RCB_IRQ_IDX_RX], ring);
/** Modified by CHJ. hulk3.19 no ic_enable_msi*/
#if 0
	ic_disable_msi(rcb_dev->virq[RCB_IRQ_IDX_RX]);
#endif
	//rcb_dev->virq[RCB_IRQ_IDX_RX] = 0;
}

/**
 *rcb_ring_enable - enable rcb ring
 *@ring:rcb ring
 *retuen status
 */
int rcb_ring_enable(struct nic_ring_pair *ring)
{
	int ret = 0;
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	ret = rcb_irq_init(ring);
	if (ret != 0) {
		log_err(ring->dev, "rcb_irq_init faild! ret=%d\n", ret);
		return ret;
	}

	napi_enable(&ring->tx_ring.napi);
	napi_enable(&ring->rx_ring.napi);

	rcb_irq_enable(ring,
		(enum rcb_int_flag)((u32)RCB_INT_FLAG_RX | RCB_INT_FLAG_TX));
	ret = rcb_dev->ops.enable(ring);
	if (ret != 0) {
		log_err(ring->dev, "ops.enable faild! ret=%d\n", ret);
		rcb_irq_uninit(ring);
		return ret;
	}
	return 0;
}

/**
 *rcb_ring_disable - disable rcb ring
 *@ring:rcb ring
 *retuen status
 */
void rcb_ring_disable(struct nic_ring_pair *ring)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	log_dbg(ring->dev, "func begin\n");

	rcb_dev->ops.disable(ring);
	rcb_irq_disable(ring,
		(enum rcb_int_flag)((u32)RCB_INT_FLAG_RX | RCB_INT_FLAG_TX));

	napi_disable(&ring->tx_ring.napi);
	napi_disable(&ring->rx_ring.napi);

	/*rcb_irq_uninit(ring);*/
	msleep(10);
	rcb_dev->ops.clean_tx_ring(ring);
}

/**
 *rcb_ring_disable_all - disable all rcb ring
 *@ring:rcb ring
 *retuen status
 */
void rcb_ring_disable_all(struct nic_device *nic_dev)
{
	int ring_id;
	struct nic_ring_pair *ring = NULL;
	struct rcb_device *rcb_dev = NULL;

	log_dbg(ring->dev, "func begin\n");

	for (ring_id=0; ring_id<nic_dev->ring_pair_num; ring_id++) {
		ring = nic_dev->ring[ring_id];
		rcb_dev = &ring->rcb_dev;
		rcb_dev->ops.disable(ring);
	}
	msleep(100);

	for (ring_id=0; ring_id<nic_dev->ring_pair_num; ring_id++) {
		ring = nic_dev->ring[ring_id];
		rcb_dev = &ring->rcb_dev;
		rcb_irq_disable(ring,
			(enum rcb_int_flag)((u32)RCB_INT_FLAG_RX | RCB_INT_FLAG_TX));

		napi_disable(&ring->tx_ring.napi);
		napi_disable(&ring->rx_ring.napi);

		/*rcb_irq_uninit(ring);*/

		rcb_dev->ops.clean_tx_ring(ring);
	}
}


/**
 *rcb_pkt_send - send pkt
 *@skb : sk_buff
 *@ring:rcb ring
 *retuen pkt num
 */
netdev_tx_t rcb_pkt_send(struct sk_buff *skb, struct nic_ring_pair *ring)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	return rcb_dev->ops.pkt_send(skb, ring);
}

/**
 *rcb_rx_poll - receive  pkt by poll
 *@ring:rcb ring
 *@budget: budget for receive pkt
 *retuen pkt num
 */
int rcb_rx_poll(struct nic_ring_pair *ring, int budget)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	return rcb_dev->ops.multi_pkt_recv(ring, budget);
}

/**
 *rcb_clean_tx_irq - clean tx irq
 *@ring:rcb ring
 *retuen status
 */
bool rcb_clean_tx_irq(struct nic_ring_pair *ring)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	return rcb_dev->ops.clean_tx_irq(ring);
}

/**
 *rcb_get_tx_ring_head - get tx ring head
 *@ring:rcb ring
 *retuen status
 */
u32 rcb_get_tx_ring_head(struct nic_ring_pair *ring)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	return rcb_dev->ops.get_tx_ring_head(rcb_dev);
}

/**
 *rcb_get_rx_ring_vld_bd_num - get ring valid bd num
 *@ring:rcb ring
 *retuen status
 */
u32 rcb_get_rx_ring_vld_bd_num(struct nic_ring_pair *ring)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;

	return rcb_dev->ops.get_rx_ring_vld_bd_num(rcb_dev);
}

/**
 *rcb_show_sw_stat_by_ring - get ring status and statistics
 *@ring:rcb ring
 *retuen status
 */
void rcb_show_sw_stat_by_ring(struct nic_ring_pair *ring)
{
	struct rcb_device *rcb_dev = &ring->rcb_dev;
	struct nic_tx_ring *tx_ring = &ring->tx_ring;
	struct nic_rx_ring *rx_ring = &ring->rx_ring;

	osal_printf("*-*-*-*-*-*-*-*-*-*-**rcb ring commom info**-*-");
	osal_printf("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\r\n");
	osal_printf("queue_index	   :%#18x  max_buf_num     :%#18x\r\n",
		    ring->queue_index, ring->max_buf_num);
	osal_printf("netdev	    :%#18llx  dev            :%#18llx\r\n",
		(u64)ring->netdev, (u64)ring->dev);
	osal_printf("**********************rcb ring info************");
	osal_printf("***********\r\n");
	osal_printf("base        :%#18llx  vbase        :%#18llx\r\n",
		rcb_dev->base, (u64)rcb_dev->vbase);
	osal_printf("index        :%#18x  buf_size    :%#18x\r\n",
	     rcb_dev->index, rcb_dev->buf_size);
	osal_printf("base_irq        :%#18x  queue_index:%#18x\r\n",
		rcb_dev->base_irq, rcb_dev->queue_index);
	osal_printf("max_buf_num    :%#18x  port_id_in_dsa:%#18x\r\n",
		rcb_dev->max_buf_num, rcb_dev->port_id_in_dsa);
	osal_printf("comm_index	    :%#18x\r\n", rcb_dev->comm_index);

	osal_printf("virq[%02d]      :%#18x  virq[%02d]:%#18x\r\n",
			0, rcb_dev->virq[0], 1,	rcb_dev->virq[1]);

	osal_printf("init        :%#18llx  uninit       :%#18llx\r\n",
		(u64)rcb_dev->ops.init,	(u64) rcb_dev->ops.uninit);
	osal_printf("enable	    :%#18llx  disable       :%#18llx\r\n",
		(u64)rcb_dev->ops.enable, (u64)rcb_dev->ops.disable);
	osal_printf("pkt_send       :%#18llx  pkt_recv       :%#18llx\r\n",
	     (u64)rcb_dev->ops.pkt_send, (u64) rcb_dev->ops.pkt_recv);
	osal_printf("multi_pkt_recv	:%#18llx  clean_tx_irq  :%#18llx\r\n",
		(u64)rcb_dev->ops.multi_pkt_recv, (u64) rcb_dev->ops.clean_tx_irq);
	osal_printf("get_tx_ring_head  :%#18llx  ",
		(u64)rcb_dev->ops.get_tx_ring_head);
	osal_printf("get_rx_ring_vld_bd_num:%#18llx\r\n",
		(u64)rcb_dev->ops.get_rx_ring_vld_bd_num);
	osal_printf("get_regs       :%#18llx  get_strings    :%#18llx\r\n",
		(u64)rcb_dev->ops.get_regs, (u64)rcb_dev->ops.get_strings);
	osal_printf("get_sset_count:%#18llx  get_ethtool_stats:%#18llx\r\n",
	     (u64)rcb_dev->ops.get_sset_count,
	     (u64)rcb_dev->ops.get_ethtool_stats);
	osal_printf("set_rx_coalesce_usecs:%#18llx  ",
		(u64)rcb_dev->ops.set_rx_coalesce_usecs);
	osal_printf("get_rx_coalesce_usecs:%#18llx\r\n",
	     (u64)rcb_dev->ops.get_rx_coalesce_usecs);
	osal_printf("set_rx_max_coalesced_frames:%#18llx",
	    (u64)rcb_dev->ops.set_rx_max_coalesced_frames);
	osal_printf("get_rx_max_coalesced_frames:%#18llx\r\n",
	    (u64)rcb_dev->ops.get_rx_max_coalesced_frames);
	osal_printf("set_use_adaptive_rx_coalesce:%#18llx ",
		(u64)rcb_dev->ops.set_use_adaptive_rx_coalesce);
	osal_printf("get_use_adaptive_rx_coalesce:%#18llx\r\n",
		(u64)rcb_dev->ops.get_use_adaptive_rx_coalesce);
	osal_printf("isr_handle	:%#18llx  int_ctrl	:%#18llx\r\n",
		(u64)rcb_dev->ops.isr_handle, (u64)rcb_dev->ops.int_ctrl);

	osal_printf("**********************rcb tx ring info*********");
	osal_printf("***********\r\n");
	osal_printf("desc     :%#18llx  tx_buffer_info   :%#18llx\r\n",
	    (u64)tx_ring->desc, (u64)tx_ring->tx_buffer_info);
	osal_printf("count	    :%#18x  numa_node        :%#18x\r\n",
	    tx_ring->count, tx_ring->numa_node);
	osal_printf("next_to_use    :%#18x  next_to_clean    :%#18x\r\n",
	    tx_ring->next_to_use, tx_ring->next_to_clean);
	osal_printf("tx_pkts    :%#18llx  tx_bytes     :%#18llx\r\n",
	    tx_ring->tx_pkts, tx_ring->tx_bytes);
	osal_printf("tx_err_cnt    :%#18llx  restart_queue :%#18llx\r\n",
		tx_ring->tx_err_cnt, tx_ring->tx_stats.restart_queue);
	osal_printf("tx_busy    :%#18llx  tx_done_old    :%#18llx\r\n",
	    tx_ring->tx_stats.tx_busy, tx_ring->tx_stats.tx_done_old);

	osal_printf("**********************rcb rx ring info******");
	osal_printf("**************\r\n");
	osal_printf("desc    :%#18llx  rx_buffer_info   :%#18llx\r\n",
	    (u64)rx_ring->desc, (u64)rx_ring->rx_buffer_info);
	osal_printf("count    :%#18x  numa_node	    :%#18x\r\n",
	    rx_ring->count, rx_ring->numa_node);
	osal_printf("next_to_use    :%#18x  next_to_clean    :%#18x\r\n",
	    rx_ring->next_to_use, rx_ring->next_to_clean);
	osal_printf("rx_pkts    :%#18llx  rx_bytes    :%#18llx\r\n",
	    rx_ring->rx_pkts, rx_ring->rx_bytes);
	osal_printf("rx_buff_err    :%#18llx  non_vld_descs :%#18llx\r\n",
	    rx_ring->rx_stats.rx_buff_err, rx_ring->rx_stats.non_vld_descs);
	osal_printf("err_pkt_len   :%#18llx alloc_rx_page_failed:%#18llx\r\n",
	    rx_ring->rx_stats.err_pkt_len,
	    rx_ring->rx_stats.alloc_rx_page_failed);
	osal_printf("alloc_rx_buff_failed:%#18llx csum_err :%#18llx\r\n",
	    rx_ring->rx_stats.alloc_rx_buff_failed,
	    rx_ring->rx_stats.csum_err);

}

/**
 *rcb_common_show_sw_info - get common rcb  status and statistics
 *@ring:rcb ring
 *retuen status
 */
void rcb_common_show_sw_info(struct rcb_common_dev *rcb_commom)
{
	osal_printf("**********************rcb commom info*******");
	osal_printf("************************************\r\n");
	osal_printf("base    :%#18llx  vbase    :%#18llx\r\n",
	    rcb_commom->base, (u64)rcb_commom->vbase);
	osal_printf("dsaf_mode      :%#18x  base_irq   :%#18x\r\n",
	    rcb_commom->dsaf_mode, rcb_commom->base_irq);
	osal_printf("virq    :%#18x  comm_index   :%#18x\r\n",
	    rcb_commom->virq, rcb_commom->comm_index);
	osal_printf("ref_cnt    :%#18x  init_commit    :%#18llx\r\n",
	    rcb_commom->ref_cnt, (u64)rcb_commom->ops.init_commit);
	osal_printf("init       :%#18llx  uninit    :%#18llx\r\n",
	    (u64)rcb_commom->ops.init, (u64)rcb_commom->ops.uninit);
	osal_printf("set_port_desc_cnt :%#18llx  ",
		(u64)rcb_commom->ops.set_port_desc_cnt);
	osal_printf("set_port_coalesced_frames:%#18llx\r\n",
	    (u64)rcb_commom->ops.set_port_coalesced_frames);
	osal_printf("set_port_timeout :%#18llx ",
	    (u64)rcb_commom->ops.set_port_timeout);
	osal_printf("get_rx_coalesce_usecs:%#18llx  ",
		(u64)rcb_commom->ops.get_rx_coalesce_usecs);
	osal_printf("get_rx_max_coalesced_frames:%#18llx",
		(u64)rcb_commom->ops.get_rx_max_coalesced_frames);
	osal_printf("set_use_adaptive_rx_coalesce:%#18llx\r\n",
		(u64)rcb_commom->ops.set_use_adaptive_rx_coalesce);
	osal_printf("get_use_adaptive_rx_coalesce:%#18llx",
		(u64)rcb_commom->ops.get_use_adaptive_rx_coalesce);
	osal_printf("get_regs:%#18llx\r\n",
	    (u64)rcb_commom->ops.get_regs);
}

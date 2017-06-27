/*
 * Copyright (c) 2016-2017 Hisilicon Limited.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/acpi.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <rdma/ib_umem.h>

#include "hnae3.h"
#include "hns_roce_common.h"
#include "hns_roce_device.h"
#include "hns_roce_cmd.h"
#include "hns_roce_hem.h"
#include "hns_roce_hw_v2.h"

#define HNS_ROCE_CMDQ_TX_TIMEOUT		200

int hns_roce_v2_post_send(struct ib_qp *ibqp, struct ib_send_wr *wr,
			  struct ib_send_wr **bad_wr)
{
	return 0;
}

int hns_roce_v2_post_recv(struct ib_qp *ibqp, struct ib_recv_wr *wr,
			  struct ib_recv_wr **bad_wr)
{
	return 0;
}

/**
 * hns_roce_v2_reset - reset RoCE
 * @hr_dev: RoCE device struct pointer
 * @enable: true -- drop reset, false -- reset
 * return 0 - success , negative --fail
 */
int hns_roce_v2_reset(struct hns_roce_dev *hr_dev, bool dereset)
{
	return 0;
}

static int hns_roce_alloc_cmd_desc(struct hns_roce_dev *hr_dev,
				   struct hns_roce_v2_cmdq *cmdq)
{
	int size = cmdq->desc_num * sizeof(struct hns_roce_cmdq_desc);

	cmdq->desc = kzalloc(size, GFP_KERNEL);
	if (!cmdq->desc)
		return -ENOMEM;

	cmdq->desc_dma_addr = dma_map_single(hr_dev->dev, cmdq->desc, size,
					     DMA_BIDIRECTIONAL);

	if (dma_mapping_error(hr_dev->dev, cmdq->desc_dma_addr)) {
		cmdq->desc_dma_addr = 0;
		kfree(cmdq->desc);
		cmdq->desc = NULL;
		return -ENOMEM;
	}

	return 0;
}

static int hns_roce_init_cmq(struct hns_roce_dev *hr_dev, bool cmdq_type)
{
	struct hns_roce_v2_priv *priv =
				    (struct hns_roce_v2_priv *)hr_dev->hw->priv;

	struct hns_roce_v2_cmdq *cmdq = (cmdq_type == TYPE_CSQ) ?
					 &priv->cmd.csq : &priv->cmd.crq;
	int ret;

	cmdq->flag = cmdq_type;

	ret = hns_roce_alloc_cmd_desc(hr_dev, cmdq);
	if (ret)
		goto err_alloc_desc;

	cmdq->next_to_clean = 0;
	cmdq->next_to_use = 0;

	return ret;

err_alloc_desc:

	return ret;
}

static void hns_roce_cmd_init_regs(struct hns_roce_dev *hr_dev, bool cmdq_type)
{
	struct hns_roce_v2_priv *priv =
				(struct hns_roce_v2_priv *)hr_dev->hw->priv;

	struct hns_roce_v2_cmdq *cmdq = (cmdq_type == TYPE_CSQ) ?
					 &priv->cmd.csq : &priv->cmd.crq;
	dma_addr_t dma = cmdq->desc_dma_addr;

	if (cmdq_type == TYPE_CSQ) {
		roce_write(hr_dev, RoCEE_TX_CMDQ_BASEADDR_L_REG, (u32)dma);
		roce_write(hr_dev, RoCEE_TX_CMDQ_BASEADDR_H_REG,
			  (u32)((dma >> 31) >> 1));
		roce_write(hr_dev, RoCEE_TX_CMDQ_DEPTH_REG,
			  (cmdq->desc_num >> HNS_ROCE_CMQ_DESC_NUM_S) |
			   HNS_ROCE_CMQ_ENABLE);
		roce_write(hr_dev, RoCEE_TX_CMDQ_HEAD_REG, 0);
		roce_write(hr_dev, ROCEE_TX_CMDQ_TAIL_REG, 0);
	} else {
		roce_write(hr_dev, RoCEE_RX_CMDQ_BASEADDR_L_REG, (u32)dma);
		roce_write(hr_dev, RoCEE_RX_CMDQ_BASEADDR_H_REG,
			  (u32)((dma >> 31) >> 1));
		roce_write(hr_dev, RoCEE_RX_CMDQ_DEPTH_REG,
			  (cmdq->desc_num >> HNS_ROCE_CMQ_DESC_NUM_S) |
			   HNS_ROCE_CMQ_ENABLE);
		roce_write(hr_dev, RoCEE_RX_CMDQ_HEAD_REG, 0);
		roce_write(hr_dev, RoCEE_RX_CMDQ_TAIL_REG, 0);
	}
}

static int hns_roce_v2_cmq_init(struct hns_roce_dev *hr_dev)
{
	int ret;

	struct hns_roce_v2_priv *priv =
				(struct hns_roce_v2_priv *)hr_dev->hw->priv;

	/* Setup the queue entries for use cmd queue */
	priv->cmd.csq.desc_num = 1024;
	priv->cmd.crq.desc_num = 1024;

	/* Setup the lock for command queue */
	mutex_init(&priv->cmd.csq.mutex);
	mutex_init(&priv->cmd.crq.mutex);

	/* Setup Tx write back timeout */
	priv->cmd.tx_timeout = HNS_ROCE_CMDQ_TX_TIMEOUT;

	/* Init CSQ */
	ret = hns_roce_init_cmq(hr_dev, TYPE_CSQ);
	if (ret) {
		dev_err(hr_dev->dev, "Init CSQ error, ret = %d.\n", ret);
		return ret;
	}

	/* Init CRQ */
	ret = hns_roce_init_cmq(hr_dev, TYPE_CRQ);
	if (ret) {
		dev_err(hr_dev->dev, "Init CRQ error, ret = %d.\n", ret);
		return ret;
	}

	/* Init CSQ REG */
	hns_roce_cmd_init_regs(hr_dev, TYPE_CSQ);

	/* Init CRQ REG */
	hns_roce_cmd_init_regs(hr_dev, TYPE_CRQ);

	return ret;
}

static void hns_roce_v2_cmq_exit(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_v2_priv *priv =
				(struct hns_roce_v2_priv *)hr_dev->hw->priv;

	kfree(priv->cmd.csq.desc_cb);
	kfree(priv->cmd.crq.desc_cb);
	priv->cmd.csq.desc_cb = NULL;
	priv->cmd.crq.desc_cb = NULL;

	mutex_destroy(&priv->cmd.csq.mutex);
	mutex_destroy(&priv->cmd.crq.mutex);
}

void hns_roce_v2_profile(struct hns_roce_dev *hr_dev)
{
}

struct hns_roce_v2_priv hr_v2_priv;

int hns_roce_v2_init(struct hns_roce_dev *hr_dev)
{
	return 0;
}

void hns_roce_v2_exit(struct hns_roce_dev *hr_dev)
{
}

void hns_roce_v2_set_gid(struct hns_roce_dev *hr_dev, u8 port, int gid_index,
			 union ib_gid *gid)
{
}

void hns_roce_v2_set_mac(struct hns_roce_dev *hr_dev, u8 phy_port, u8 *addr)
{
}

void hns_roce_v2_set_mtu(struct hns_roce_dev *hr_dev, u8 phy_port,
			 enum ib_mtu mtu)
{
}

int hns_roce_v2_write_mtpt(void *mb_buf, struct hns_roce_mr *mr,
			   unsigned long mtpt_idx)
{
	return 0;
}

void hns_roce_v2_write_cqc(struct hns_roce_dev *hr_dev,
			   struct hns_roce_cq *hr_cq, void *mb_buf, u64 *mtts,
			   dma_addr_t dma_handle, int nent, u32 vector)
{
}

int hns_roce_v2_req_notify_cq(struct ib_cq *ibcq, enum ib_cq_notify_flags flags)
{
	return 0;
}

int hns_roce_v2_poll_cq(struct ib_cq *ibcq, int num_entries, struct ib_wc *wc)
{
	return 0;
}

int hns_roce_v2_clear_hem(struct hns_roce_dev *hr_dev,
		struct hns_roce_hem_table *table, int obj)
{
	return 0;
}

int hns_roce_v2_modify_qp(struct ib_qp *ibqp, const struct ib_qp_attr *attr,
			  int attr_mask, enum ib_qp_state cur_state,
			  enum ib_qp_state new_state)
{
	return 0;
}

int hns_roce_v2_query_qp(struct ib_qp *ibqp, struct ib_qp_attr *qp_attr,
			 int qp_attr_mask, struct ib_qp_init_attr *qp_init_attr)
{
	return 0;
}

int hns_roce_v2_destroy_qp(struct ib_qp *ibqp)
{
	return 0;
}

static const struct hns_roce_hw hns_roce_hw_v2 = {
	.reset = hns_roce_v2_reset,
	.cmq_init = hns_roce_v2_cmq_init,
	.cmq_exit = hns_roce_v2_cmq_exit,
	.hw_profile = hns_roce_v2_profile,
	.hw_init = hns_roce_v2_init,
	.hw_exit = hns_roce_v2_exit,
	.set_gid = hns_roce_v2_set_gid,
	.set_mac = hns_roce_v2_set_mac,
	.set_mtu = hns_roce_v2_set_mtu,
	.write_mtpt = hns_roce_v2_write_mtpt,
	.write_cqc = hns_roce_v2_write_cqc,
	.clear_hem = hns_roce_v2_clear_hem,
	.modify_qp = hns_roce_v2_modify_qp,
	.query_qp = hns_roce_v2_query_qp,
	.destroy_qp = hns_roce_v2_destroy_qp,
	.post_send = hns_roce_v2_post_send,
	.post_recv = hns_roce_v2_post_recv,
	.req_notify_cq = hns_roce_v2_req_notify_cq,
	.poll_cq = hns_roce_v2_poll_cq,
	.priv = &hr_v2_priv,
};

static const struct pci_device_id hns_roce_pci_tbl[] = {
	{PCI_VDEVICE(HUAWEI, 0xA222), (kernel_ulong_t)&hns_roce_hw_v2},
	{PCI_VDEVICE(HUAWEI, 0xA223), (kernel_ulong_t)&hns_roce_hw_v2},
	{PCI_VDEVICE(HUAWEI, 0xA226), (kernel_ulong_t)&hns_roce_hw_v2},
	/* required last entry */
	{0, }
};

static int hns_roce_get_pci_cfg(struct hns_roce_dev *hr_dev,
				struct hnae3_handle *handle)
{
	const struct pci_device_id *id;

	id = pci_match_id(hns_roce_pci_tbl, hr_dev->pci_dev);
	if (!id) {
		dev_err(hr_dev->dev, "device is not compatible!\n");
		return -ENXIO;
	}

	hr_dev->hw = (struct hns_roce_hw *) id->driver_data;
	if (!hr_dev->hw) {
		dev_err(hr_dev->dev, "couldn't get H/W specific PCI data!\n");
		return -ENXIO;
	}

	/* Get info from NIC driver. */
	hr_dev->reg_base = handle->rinfo.roce_io_base;
	hr_dev->caps.num_ports = 1;
	hr_dev->iboe.netdevs[0] = handle->rinfo.netdev;
	hr_dev->iboe.phy_port[0] = 0;

	/* cmd issue mode: 0 is poll, 1 is event */
	hr_dev->cmd_mod = 0;
	hr_dev->loop_idc = 0;

	return 0;
}

static int hns_roce_pci_init_instance(struct hnae3_handle *handle)
{
	int ret;
	struct hns_roce_dev *hr_dev;

	hr_dev = (struct hns_roce_dev *)ib_alloc_device(sizeof(*hr_dev));
	if (!hr_dev)
		return -ENOMEM;

	hr_dev->pci_dev = handle->pdev;
	hr_dev->dev = &handle->pdev->dev;
	handle->priv = hr_dev;

	ret = hns_roce_get_pci_cfg(hr_dev, handle);
	if (ret) {
		dev_err(hr_dev->dev, "Get Configuration failed!\n");
		return ret;
	}

	return hns_roce_init(hr_dev);
}

static void hns_roce_pci_uninit_instance(struct hnae3_handle *handle,
					 bool reset)
{
	struct hns_roce_dev *hr_dev = (struct hns_roce_dev *)handle->priv;

	hns_roce_exit(hr_dev);
	ib_dealloc_device(&hr_dev->ib_dev);
}

static struct hnae3_client_ops hns_roce_ops = {
	.init_instance = hns_roce_pci_init_instance,
	.uninit_instance = hns_roce_pci_uninit_instance,
};

static struct hnae3_client hns_roce_client = {
	.name = "hns_roce",
	.type = HNAE3_CLIENT_ROCE,
	.ops = &hns_roce_ops,
};

static int __init hns_roce_pci_init(void)
{
	return hnae3_register_client(&hns_roce_client);
}

static void __exit hns_roce_pci_exit(void)
{
	hnae3_unregister_client(&hns_roce_client);
}

module_init(hns_roce_pci_init);
module_exit(hns_roce_pci_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Wei Hu <xavier.huwei@huawei.com>");
MODULE_AUTHOR("Lijun Ou <oulijun@huawei.com>");
MODULE_AUTHOR("Shaobo Xu <xushaobo2@huawei.com>");
MODULE_DESCRIPTION("HNS RoCE  hw v2 Driver based on PCI device");

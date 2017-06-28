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

void hns_roce_cmd_setup_basic_desc(struct hns_roce_cmdq_desc *desc,
				   enum hns_roce_opcode_type opcode,
				   bool is_read)
{
	memset((void *)desc, 0, sizeof(struct hns_roce_cmdq_desc));
	desc->opcode = cpu_to_le16(opcode);
	desc->flag =
		cpu_to_le16(HNS_ROCE_CMD_FLAG_NO_INTR | HNS_ROCE_CMD_FLAG_IN);
	if (is_read)
		desc->flag |= cpu_to_le16(HNS_ROCE_CMD_FLAG_WR);
	else
		desc->flag &= cpu_to_le16(~HNS_ROCE_CMD_FLAG_WR);
}

static int hns_roce_cmd_csq_done(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_v2_priv *priv =
				(struct hns_roce_v2_priv *)hr_dev->hw->priv;

	u32 head = roce_read(hr_dev, RoCEE_TX_CMDQ_HEAD_REG);
	return head == priv->cmd.csq.next_to_use;
}

static int hns_roce_cmd_csq_clean(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_cmdq_desc *desc;
	struct hns_roce_v2_priv *priv =
				(struct hns_roce_v2_priv *)hr_dev->hw->priv;
	struct hns_roce_v2_cmdq *csq = &priv->cmd.csq;
	u16 ntc = csq->next_to_clean;
	u32 head;
	int clean = 0;

	desc = &csq->desc[ntc];
	head = roce_read(hr_dev, RoCEE_TX_CMDQ_HEAD_REG);
	while (head != ntc) {
		memset(desc, 0, sizeof(*desc));
		ntc++;
		if (ntc == csq->desc_num)
			ntc = 0;
		desc = &csq->desc[ntc];
		clean++;
	}
	csq->next_to_clean = ntc;

	return clean;
}

enum hns_roce_cmd_status hns_roce_cmd_send(struct hns_roce_dev *hr_dev,
					   struct hns_roce_cmdq_desc *desc,
					   int num)
{
	struct hns_roce_v2_priv *priv =
				(struct hns_roce_v2_priv *)hr_dev->hw->priv;
	enum hns_roce_cmd_status status = 0;
	struct hns_roce_cmdq_desc *desc_to_use;
	u32 timeout = 0;
	bool complete = false;
	int handle = 0;
	int ntc;
	u16 retval;

	mutex_lock(&priv->cmd.csq.mutex);

	/**
	 * Record the location of desc in the cmq for this time
	 * which will be use for hardware to write back
	 */
	ntc = priv->cmd.csq.next_to_use;

	while (handle < num) {
		desc_to_use = &priv->cmd.csq.desc[priv->cmd.csq.next_to_use];
		*desc_to_use = desc[handle];
		dev_dbg(hr_dev->dev, "set cmd desc:\n");
		priv->cmd.csq.next_to_use++;
		if (priv->cmd.csq.next_to_use == priv->cmd.csq.desc_num)
			priv->cmd.csq.next_to_use = 0;
		handle++;
	}

	/* Write to hardware */
	roce_write(hr_dev, ROCEE_TX_CMDQ_TAIL_REG, priv->cmd.csq.next_to_use);

	/**
	 * If the command is sync, wait for the firmware to write back,
	 * if multi descriptors to be sent, use the first one to check
	 */
	if ((desc->flag) & HNS_ROCE_CMD_FLAG_NO_INTR) {
		do {
			if (hns_roce_cmd_csq_done(hr_dev))
				break;
			usleep_range(1000, 2000);
			timeout++;
		} while (timeout < priv->cmd.tx_timeout);
	}

	if (hns_roce_cmd_csq_done(hr_dev)) {
		complete = true;
		handle = 0;
		while (handle < num) {
			/* get the result of hardware write back */
			desc_to_use = &priv->cmd.csq.desc[ntc];
			desc[handle] = *desc_to_use;
			dev_dbg(hr_dev->dev, "Get cmd desc:\n");
			retval = desc[handle].retval;
			if ((enum hns_roce_cmd_return_status)retval ==
			    CMD_EXEC_SUCCESS)
				status = 0;
			else
				status = ERR_CSQ_ERROR;
			priv->cmd.last_status =
					(enum hns_roce_cmd_status)retval;
			ntc++;
			handle++;
			if (ntc == priv->cmd.csq.desc_num)
				ntc = 0;
		}
	}

	if (!complete)
		status = ERR_CSQ_TIMEOUT;

	/* clean the command send queue */
	handle = hns_roce_cmd_csq_clean(hr_dev);
	if (handle != num) {
		dev_warn(hr_dev->dev, "Cleaned %d, need to clean %d\n",
			 handle, num);
	}

	mutex_unlock(&priv->cmd.csq.mutex);
	return status;
}

enum hns_roce_cmd_status hns_roce_cmd_query_hw_info(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_cmdq_desc desc;
	struct hns_roce_query_version *resp =
				     (struct hns_roce_query_version *)desc.data;
	enum hns_roce_cmd_status status;

	hns_roce_cmd_setup_basic_desc(&desc, HNS_ROCE_OPC_QUERY_HW_VER, true);
	status = hns_roce_cmd_send(hr_dev, &desc, 1);
	if (status)
		return status;

	hr_dev->hw_rev = le32_to_cpu(resp->rocee_hw_version);
	hr_dev->vendor_id = le32_to_cpu(resp->rocee_vendor_id);

	return status;
}

static int hns_roce_config_global_param(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_cmdq_desc desc;
	struct hns_roce_cfg_global_param *req =
				(struct hns_roce_cfg_global_param *)desc.data;
	enum hns_roce_cmd_status status;

	hns_roce_cmd_setup_basic_desc(&desc, HNS_ROCE_OPC_CFG_GLOBAL_PARAM,
				      false);
	memset(req, 0, sizeof(*req));
	roce_set_field(req->data_0,
		       CFG_GLOBAL_PARAM_DATA_0_ROCEE_TIME_1US_CFG_M,
		       CFG_GLOBAL_PARAM_DATA_0_ROCEE_TIME_1US_CFG_S, 0x12b7);
	roce_set_field(req->data_0, CFG_GLOBAL_PARAM_DATA_0_ROCEE_UDP_PORT_M,
		       CFG_GLOBAL_PARAM_DATA_0_ROCEE_UDP_PORT_S, 0x3e8);
	status = hns_roce_cmd_send(hr_dev, &desc, 1);
	if (status)
		return status;

	return 0;
}

static int hns_roce_alloc_pf_resource(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_cmdq_desc desc[2];
	struct hns_roce_pf_res_a *req_a =
				(struct hns_roce_pf_res_a *)desc[0].data;
	struct hns_roce_pf_res_b *req_b =
				(struct hns_roce_pf_res_b *)desc[1].data;
	enum hns_roce_cmd_status status;
	int i;

	memset(req_a, 0, sizeof(*req_a));
	memset(req_b, 0, sizeof(*req_b));
	for (i = 0; i < 2; i++) {
		hns_roce_cmd_setup_basic_desc(&desc[i],
					      HNS_ROCE_OPC_ALLOC_PF_RES, false);

		if (i == 0)
			desc[i].flag |= cpu_to_le16(HNS_ROCE_CMD_FLAG_NEXT);
		else
			desc[i].flag &= ~cpu_to_le16(HNS_ROCE_CMD_FLAG_NEXT);

		if (i == 0) {
			roce_set_field(req_a->data_1,
				       PF_RES_A_DATA_1_PF_QPC_BT_IDX_M,
				       PF_RES_A_DATA_1_PF_QPC_BT_IDX_S, 0);
			roce_set_field(req_a->data_1,
				       PF_RES_A_DATA_1_PF_QPC_BT_NUM_M,
				       PF_RES_A_DATA_1_PF_QPC_BT_NUM_S,
				       HNS_ROCE_VF_QPC_BT_NUM);

			roce_set_field(req_a->data_2,
				       PF_RES_A_DATA_2_PF_SRQC_BT_IDX_M,
				       PF_RES_A_DATA_2_PF_SRQC_BT_IDX_S, 0);
			roce_set_field(req_a->data_2,
				       PF_RES_A_DATA_2_PF_SRQC_BT_NUM_M,
				       PF_RES_A_DATA_2_PF_SRQC_BT_NUM_S,
				       HNS_ROCE_VF_SRQC_BT_NUM);

			roce_set_field(req_a->data_3,
				       PF_RES_A_DATA_3_PF_CQC_BT_IDX_M,
				       PF_RES_A_DATA_3_PF_CQC_BT_IDX_S, 0);
			roce_set_field(req_a->data_3,
				       PF_RES_A_DATA_3_PF_CQC_BT_NUM_M,
				       PF_RES_A_DATA_3_PF_CQC_BT_NUM_S,
				       HNS_ROCE_VF_CQC_BT_NUM);

			roce_set_field(req_a->data_4,
				       PF_RES_A_DATA_4_PF_MPT_BT_IDX_M,
				       PF_RES_A_DATA_4_PF_MPT_BT_IDX_S, 0);
			roce_set_field(req_a->data_4,
				       PF_RES_A_DATA_4_PF_MPT_BT_NUM_M,
				       PF_RES_A_DATA_4_PF_MPT_BT_NUM_S,
				       HNS_ROCE_VF_MPT_BT_NUM);

			roce_set_field(req_a->data_5,
				       PF_RES_A_DATA_5_PF_EQC_BT_IDX_M,
				       PF_RES_A_DATA_5_PF_EQC_BT_IDX_S, 0);
			roce_set_field(req_a->data_5,
				       PF_RES_A_DATA_5_PF_EQC_BT_NUM_M,
				       PF_RES_A_DATA_5_PF_EQC_BT_NUM_S,
				       HNS_ROCE_VF_EQC_NUM);
		} else {
			roce_set_field(req_b->data_1,
				       PF_RES_B_DATA_1_PF_SMAC_IDX_M,
				       PF_RES_B_DATA_1_PF_SMAC_IDX_S, 0);
			roce_set_field(req_b->data_1,
				       PF_RES_B_DATA_1_PF_SMAC_NUM_M,
				       PF_RES_B_DATA_1_PF_SMAC_NUM_S,
				       HNS_ROCE_VF_SMAC_NUM);

			roce_set_field(req_b->data_2,
				       PF_RES_B_DATA_2_PF_SGID_IDX_M,
				       PF_RES_B_DATA_2_PF_SGID_IDX_S, 0);
			roce_set_field(req_b->data_2,
				       PF_RES_B_DATA_2_PF_SGID_NUM_M,
				       PF_RES_B_DATA_2_PF_SGID_NUM_S,
				       HNS_ROCE_VF_SGID_NUM);

			roce_set_field(req_b->data_3,
				       VF_RES_B_DATA_3_VF_QID_IDX_M,
				       VF_RES_B_DATA_3_VF_QID_IDX_S, 0);
			roce_set_field(req_b->data_3,
				       VF_RES_B_DATA_3_VF_SL_NUM_M,
				       VF_RES_B_DATA_3_VF_SL_NUM_S,
				       HNS_ROCE_VF_SL_NUM);
		}
	}

	status = hns_roce_cmd_send(hr_dev, desc, 2);
	if (status)
		return status;

	return 0;
}

static int hns_roce_alloc_vf_resource(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_cmdq_desc desc[2];
	struct hns_roce_vf_res_a *req_a =
				    (struct hns_roce_vf_res_a *)desc[0].data;
	struct hns_roce_vf_res_b *req_b =
				    (struct hns_roce_vf_res_b *)desc[1].data;
	enum hns_roce_cmd_status status;
	int i;

	memset(req_a, 0, sizeof(*req_a));
	memset(req_b, 0, sizeof(*req_b));
	for (i = 0; i < 2; i++) {
		hns_roce_cmd_setup_basic_desc(&desc[i],
					      HNS_ROCE_OPC_ALLOC_VF_RES, false);

		if (i == 0)
			desc[i].flag |= cpu_to_le16(HNS_ROCE_CMD_FLAG_NEXT);
		else
			desc[i].flag &= ~cpu_to_le16(HNS_ROCE_CMD_FLAG_NEXT);

		if (i == 0) {
			roce_set_field(req_a->data_1,
				       VF_RES_A_DATA_1_VF_QPC_BT_IDX_M,
				       VF_RES_A_DATA_1_VF_QPC_BT_IDX_S, 0);
			roce_set_field(req_a->data_1,
				       VF_RES_A_DATA_1_VF_QPC_BT_NUM_M,
				       VF_RES_A_DATA_1_VF_QPC_BT_NUM_S,
				       HNS_ROCE_VF_QPC_BT_NUM);

			roce_set_field(req_a->data_2,
				       VF_RES_A_DATA_2_VF_SRQC_BT_IDX_M,
				       VF_RES_A_DATA_2_VF_SRQC_BT_IDX_S, 0);
			roce_set_field(req_a->data_2,
				       VF_RES_A_DATA_2_VF_SRQC_BT_NUM_M,
				       VF_RES_A_DATA_2_VF_SRQC_BT_NUM_S,
				       HNS_ROCE_VF_SRQC_BT_NUM);

			roce_set_field(req_a->data_3,
				       VF_RES_A_DATA_3_VF_CQC_BT_IDX_M,
				       VF_RES_A_DATA_3_VF_CQC_BT_IDX_S, 0);
			roce_set_field(req_a->data_3,
				       VF_RES_A_DATA_3_VF_CQC_BT_NUM_M,
				       VF_RES_A_DATA_3_VF_CQC_BT_NUM_S,
				       HNS_ROCE_VF_CQC_BT_NUM);

			roce_set_field(req_a->data_4,
				       VF_RES_A_DATA_4_VF_MPT_BT_IDX_M,
				       VF_RES_A_DATA_4_VF_MPT_BT_IDX_S, 0);
			roce_set_field(req_a->data_4,
				       VF_RES_A_DATA_4_VF_MPT_BT_NUM_M,
				       VF_RES_A_DATA_4_VF_MPT_BT_NUM_S,
				       HNS_ROCE_VF_MPT_BT_NUM);

			roce_set_field(req_a->data_5,
				       VF_RES_A_DATA_5_VF_EQC_IDX_M,
				       VF_RES_A_DATA_5_VF_EQC_IDX_S, 0);
			roce_set_field(req_a->data_5,
				       VF_RES_A_DATA_5_VF_EQC_NUM_M,
				       VF_RES_A_DATA_5_VF_EQC_NUM_S,
				       HNS_ROCE_VF_EQC_NUM);
		} else {
			roce_set_field(req_b->data_1,
				       VF_RES_B_DATA_1_VF_SMAC_IDX_M,
				       VF_RES_B_DATA_1_VF_SMAC_IDX_S, 0);
			roce_set_field(req_b->data_1,
				       VF_RES_B_DATA_1_VF_SMAC_NUM_M,
				       VF_RES_B_DATA_1_VF_SMAC_NUM_S,
				       HNS_ROCE_VF_SMAC_NUM);

			roce_set_field(req_b->data_2,
				       VF_RES_B_DATA_2_VF_SGID_IDX_M,
				       VF_RES_B_DATA_2_VF_SGID_IDX_S, 0);
			roce_set_field(req_b->data_2,
				       VF_RES_B_DATA_2_VF_SGID_NUM_M,
				       VF_RES_B_DATA_2_VF_SGID_NUM_S,
				       HNS_ROCE_VF_SGID_NUM);

			roce_set_field(req_b->data_3,
				       VF_RES_B_DATA_3_VF_QID_IDX_M,
				       VF_RES_B_DATA_3_VF_QID_IDX_S, 0);
			roce_set_field(req_b->data_3,
				       VF_RES_B_DATA_3_VF_SL_NUM_M,
				       VF_RES_B_DATA_3_VF_SL_NUM_S,
				       HNS_ROCE_VF_SL_NUM);
		}
	}

	status = hns_roce_cmd_send(hr_dev, desc, 2);
	if (status)
		return status;

	return 0;
}

static int hns_roce_v2_set_bt(struct hns_roce_dev *hr_dev)
{
	struct hns_roce_cmdq_desc desc;
	struct hns_roce_cfg_bt_attr *req =
				    (struct hns_roce_cfg_bt_attr *)desc.data;
	enum hns_roce_cmd_status status;
	u8 qpc_hop_num = hr_dev->caps.qpc_hop_num;
	u8 srqc_hop_num = hr_dev->caps.srqc_hop_num;
	u8 cqc_hop_num = hr_dev->caps.cqc_hop_num;
	u8 mpt_hop_num = hr_dev->caps.mpt_hop_num;

	hns_roce_cmd_setup_basic_desc(&desc, HNS_ROCE_OPC_CFG_BT_ATTR, false);
	memset(req, 0, sizeof(*req));

	roce_set_field(req->data_0, CFG_BT_ATTR_DATA_0_VF_QPC_BA_PGSZ_M,
		       CFG_BT_ATTR_DATA_0_VF_QPC_BA_PGSZ_S,
		       hr_dev->caps.qpc_ba_pg_sz);
	roce_set_field(req->data_0, CFG_BT_ATTR_DATA_0_VF_QPC_BUF_PGSZ_M,
		       CFG_BT_ATTR_DATA_0_VF_QPC_BUF_PGSZ_S,
		       hr_dev->caps.qpc_buf_pg_sz);
	roce_set_field(req->data_0, CFG_BT_ATTR_DATA_0_VF_QPC_HOPNUM_M,
		       CFG_BT_ATTR_DATA_0_VF_QPC_HOPNUM_S,
		       qpc_hop_num == HNS_ROCE_HOP_NUM_0 ? 0 : qpc_hop_num);

	roce_set_field(req->data_1, CFG_BT_ATTR_DATA_1_VF_SRQC_BA_PGSZ_M,
		       CFG_BT_ATTR_DATA_1_VF_SRQC_BA_PGSZ_S,
		       hr_dev->caps.srqc_ba_pg_sz);
	roce_set_field(req->data_1, CFG_BT_ATTR_DATA_1_VF_SRQC_BUF_PGSZ_M,
		       CFG_BT_ATTR_DATA_1_VF_SRQC_BUF_PGSZ_S,
		       hr_dev->caps.srqc_buf_pg_sz);
	roce_set_field(req->data_1, CFG_BT_ATTR_DATA_1_VF_SRQC_HOPNUM_M,
		       CFG_BT_ATTR_DATA_1_VF_SRQC_HOPNUM_S,
		       srqc_hop_num == HNS_ROCE_HOP_NUM_0 ? 0 : srqc_hop_num);

	roce_set_field(req->data_2, CFG_BT_ATTR_DATA_2_VF_CQC_BA_PGSZ_M,
		       CFG_BT_ATTR_DATA_2_VF_CQC_BA_PGSZ_S,
		       hr_dev->caps.cqc_ba_pg_sz);
	roce_set_field(req->data_2, CFG_BT_ATTR_DATA_2_VF_CQC_BUF_PGSZ_M,
		       CFG_BT_ATTR_DATA_2_VF_CQC_BUF_PGSZ_S,
		       hr_dev->caps.cqc_buf_pg_sz);
	roce_set_field(req->data_2, CFG_BT_ATTR_DATA_2_VF_CQC_HOPNUM_M,
		       CFG_BT_ATTR_DATA_2_VF_CQC_HOPNUM_S,
		       cqc_hop_num == HNS_ROCE_HOP_NUM_0 ? 0 : cqc_hop_num);

	roce_set_field(req->data_3, CFG_BT_ATTR_DATA_3_VF_MPT_BA_PGSZ_M,
		       CFG_BT_ATTR_DATA_3_VF_MPT_BA_PGSZ_S,
		       hr_dev->caps.mpt_ba_pg_sz);
	roce_set_field(req->data_3, CFG_BT_ATTR_DATA_3_VF_MPT_BUF_PGSZ_M,
		       CFG_BT_ATTR_DATA_3_VF_MPT_BUF_PGSZ_S,
		       hr_dev->caps.mpt_buf_pg_sz);
	roce_set_field(req->data_3, CFG_BT_ATTR_DATA_3_VF_MPT_HOPNUM_M,
		       CFG_BT_ATTR_DATA_3_VF_MPT_HOPNUM_S,
		       mpt_hop_num == HNS_ROCE_HOP_NUM_0 ? 0 : mpt_hop_num);

	status = hns_roce_cmd_send(hr_dev, &desc, 1);
	if (status)
		return status;

	return 0;
}

int hns_roce_v2_profile(struct hns_roce_dev *hr_dev)
{
	int ret;
	struct hns_roce_caps *caps = &hr_dev->caps;

	ret = hns_roce_cmd_query_hw_info(hr_dev);
	if (ret) {
		dev_err(hr_dev->dev, "Query firmware version fail, ret = %d.\n",
			ret);
		return ret;
	}

	dev_err(hr_dev->dev, "the hw version is 0x%x\n", hr_dev->hw_rev);

	ret = hns_roce_config_global_param(hr_dev);
	if (ret) {
		dev_err(hr_dev->dev, "Configure global param fail, ret = %d.\n",
			ret);
	}

	ret = hns_roce_alloc_pf_resource(hr_dev);
	if (ret) {
		dev_err(hr_dev->dev, "Allocate pf resource fail, ret = %d.\n",
			ret);
		return ret;
	}

	ret = hns_roce_alloc_vf_resource(hr_dev);
	if (ret) {
		dev_err(hr_dev->dev, "Allocate vf resource fail, ret = %d.\n",
			ret);
		return ret;
	}

	hr_dev->vendor_part_id = 0;
	hr_dev->sys_image_guid = 0;

	caps->num_qps		= HNS_ROCE_V2_MAX_QP_NUM;
	caps->max_wqes		= HNS_ROCE_V2_MAX_WQE_NUM;
	caps->num_cqs		= HNS_ROCE_V2_MAX_CQ_NUM;
	caps->max_cqes		= HNS_ROCE_V2_MAX_CQE_NUM;
	caps->max_sq_sg		= HNS_ROCE_V2_MAX_SQ_SGE_NUM;
	caps->max_rq_sg		= HNS_ROCE_V2_MAX_RQ_SGE_NUM;
	caps->max_sq_inline	= HNS_ROCE_V2_MAX_SQ_INLINE;
	caps->num_uars		= HNS_ROCE_V2_UAR_NUM;
	caps->phy_num_uars	= HNS_ROCE_V2_PHY_UAR_NUM;
	caps->num_aeq_vectors	= 1;
	caps->num_comp_vectors	= 63;
	caps->num_other_vectors	= 0;
	caps->num_mtpts		= HNS_ROCE_V2_MAX_MTPT_NUM;
	caps->num_mtt_segs	= HNS_ROCE_V2_MAX_MTT_SEGS;/* need to confirm */
	caps->num_cqe_segs	= HNS_ROCE_V2_MAX_CQE_SEGS;
	caps->num_pds		= HNS_ROCE_V2_MAX_PD_NUM;
	caps->max_qp_init_rdma	= HNS_ROCE_V2_MAX_QP_INIT_RDMA;
	caps->max_qp_dest_rdma	= HNS_ROCE_V2_MAX_QP_DEST_RDMA;
	caps->max_sq_desc_sz	= HNS_ROCE_V2_MAX_SQ_DESC_SZ;
	caps->max_rq_desc_sz	= HNS_ROCE_V2_MAX_RQ_DESC_SZ;
	caps->max_srq_desc_sz	= HNS_ROCE_V2_MAX_SRQ_DESC_SZ;
	caps->qpc_entry_sz	= HNS_ROCE_V2_QPC_ENTRY_SZ;
	caps->irrl_entry_sz	= HNS_ROCE_V2_IRRL_ENTRY_SZ;
	caps->cqc_entry_sz	= HNS_ROCE_V2_CQC_ENTRY_SZ;
	caps->mtpt_entry_sz	= HNS_ROCE_V2_MTPT_ENTRY_SZ;
	caps->mtt_entry_sz	= HNS_ROCE_V2_MTT_ENTRY_SZ;
	caps->cq_entry_sz	= HNS_ROCE_V2_CQE_ENTRY_SIZE;
	caps->page_size_cap	= HNS_ROCE_V2_PAGE_SIZE_SUPPORTED;
	caps->reserved_lkey	= 0;
	caps->reserved_pds	= 0;
	caps->reserved_mrws	= 1;
	caps->reserved_uars	= 0;
	caps->reserved_cqs	= 0;
	caps->qpc_bt_num	= HNS_ROCE_VF_QPC_BT_NUM;
	caps->cqc_bt_num	= HNS_ROCE_VF_CQC_BT_NUM;
	caps->srqc_bt_num	= HNS_ROCE_VF_SRQC_BT_NUM;
	caps->mpt_bt_num	= HNS_ROCE_VF_MPT_BT_NUM;

	caps->qpc_ba_pg_sz	= 0;
	caps->qpc_buf_pg_sz	= 0;
	caps->qpc_hop_num	= HNS_ROCE_CONTEXT_HOP_NUM;
	caps->srqc_ba_pg_sz	= 0;
	caps->srqc_buf_pg_sz	= 0;
	caps->srqc_hop_num	= HNS_ROCE_HOP_NUM_0;
	caps->cqc_ba_pg_sz	= 0;
	caps->cqc_buf_pg_sz	= 0;
	caps->cqc_hop_num	= HNS_ROCE_CONTEXT_HOP_NUM;
	caps->mpt_ba_pg_sz	= 0;
	caps->mpt_buf_pg_sz	= 0;
	caps->mpt_hop_num	= HNS_ROCE_CONTEXT_HOP_NUM;
	caps->mtt_ba_pg_sz	= 0;
	caps->mtt_buf_pg_sz	= 0;
	caps->mtt_hop_num	= HNS_ROCE_MTT_HOP_NUM;
	caps->cqe_ba_pg_sz	= 0;
	caps->cqe_buf_pg_sz	= 0;
	caps->cqe_hop_num	= HNS_ROCE_CQE_HOP_NUM;

	caps->pkey_table_len[0] = 1;
	caps->gid_table_len[0] = 2;
	caps->local_ca_ack_delay = 0;
	caps->max_mtu = IB_MTU_4096;

	ret = hns_roce_v2_set_bt(hr_dev);
	if (ret) {
		dev_err(hr_dev->dev, "Configure bt attribute fail, ret = %d.\n",
			ret);
		return ret;
	}

	return 0;
}

struct hns_roce_v2_priv hr_v2_priv;

int hns_roce_v2_init(struct hns_roce_dev *hr_dev)
{
	return 0;
}

void hns_roce_v2_exit(struct hns_roce_dev *hr_dev)
{
}

static int hns_roce_v2_cmd_pending(struct hns_roce_dev *hr_dev)
{
	u32 status = readl(hr_dev->reg_base + ROCEE_VF_MB_STATUS_REG);

	return status >> HNS_ROCE_HW_RUN_BIT_SHIFT;
}
static int hns_roce_v2_cmd_complete(struct hns_roce_dev *hr_dev)
{
	u32 status = readl(hr_dev->reg_base + ROCEE_VF_MB_STATUS_REG);

	return status & HNS_ROCE_HW_MB_STATUS_MASK;
}

int hns_roce_v2_post_mbox(struct hns_roce_dev *hr_dev, u64 in_param,
			  u64 out_param, u32 in_modifier, u8 op_modifier,
			  u16 op, u16 token, int event)
{
	struct device *dev = hr_dev->dev;
	u32 *hcr = (u32 *)(hr_dev->reg_base + ROCEE_VF_MB_CFG0_REG);
	int ret = -EAGAIN;
	unsigned long end;
	u32 val0 = 0;
	u32 val1 = 0;

	end = msecs_to_jiffies(HNS_ROCE_V2_GO_BIT_TIMEOUT_MSECS) + jiffies;
	while (hns_roce_v2_cmd_pending(hr_dev)) {
		if (time_after(jiffies, end)) {
			dev_dbg(dev, "jiffies=%d end=%d\n", (int)jiffies,
				(int)end);
			goto out;
		}
		cond_resched();
	}

	roce_set_field(val0, HNS_ROCE_VF_MB4_TAG_MASK,
		       HNS_ROCE_VF_MB4_TAG_SHIFT, in_modifier);
	roce_set_field(val0, HNS_ROCE_VF_MB4_CMD_MASK,
		       HNS_ROCE_VF_MB4_CMD_SHIFT, op);
	roce_set_field(val1, HNS_ROCE_VF_MB5_EVENT_MASK,
		       HNS_ROCE_VF_MB5_EVENT_SHIFT, event);
	roce_set_field(val1, HNS_ROCE_VF_MB5_TOKEN_MASK,
		       HNS_ROCE_VF_MB5_TOKEN_SHIFT, token);

	__raw_writeq(cpu_to_le64(in_param), hcr + 0);
	__raw_writeq(cpu_to_le64(out_param), hcr + 2);

	/* Memory barrier */
	wmb();

	__raw_writel(cpu_to_le32(val0), hcr + 4);
	__raw_writel(cpu_to_le32(val1), hcr + 5);

	mmiowb();
	ret = 0;

out:
	return ret;
}

static int hns_roce_v2_chk_mbox(struct hns_roce_dev *hr_dev,
				unsigned long timeout)
{
	struct device *dev = hr_dev->dev;
	unsigned long end = 0;
	u32 status = 0;
	int ret = 0;

	end = msecs_to_jiffies(timeout) + jiffies;
	while (hns_roce_v2_cmd_pending(hr_dev) && time_before(jiffies, end))
		cond_resched();

	if (hns_roce_v2_cmd_pending(hr_dev)) {
		dev_err(dev, "[cmd_poll]hw run cmd TIMEDOUT!\n");
		ret = -ETIMEDOUT;
		goto out;
	}

	status = hns_roce_v2_cmd_complete(hr_dev);
	if (status != 0x1) {
		dev_err(dev, "mailbox status 0x%x!\n", status);
		ret = -EBUSY;
		goto out;
	}

out:
	return ret;
}

void hns_roce_v2_set_gid(struct hns_roce_dev *hr_dev, u8 port, int gid_index,
			 union ib_gid *gid)
{
	u32 *p;
	u32 val;

	p = (u32 *)&gid->raw[0];
	roce_raw_write(*p, hr_dev->reg_base + ROCEE_VF_SGID_CFG0_REG +
		       0x20 * gid_index);

	p = (u32 *)&gid->raw[4];
	roce_raw_write(*p, hr_dev->reg_base + ROCEE_VF_SGID_CFG1_REG +
		       0x20 * gid_index);

	p = (u32 *)&gid->raw[8];
	roce_raw_write(*p, hr_dev->reg_base + ROCEE_VF_SGID_CFG2_REG +
		       0x20 * gid_index);

	p = (u32 *)&gid->raw[0xc];
	roce_raw_write(*p, hr_dev->reg_base + ROCEE_VF_SGID_CFG3_REG +
		       0x20 * gid_index);

	val = roce_read(hr_dev, ROCEE_VF_SGID_CFG4_REG + 0x20 * gid_index);
	roce_set_field(val, ROCEE_VF_SGID_CFG4_SGID_TYPE_M,
		       ROCEE_VF_SGID_CFG4_SGID_TYPE_S, 0);

	roce_write(hr_dev, ROCEE_VF_SGID_CFG4_REG + 0x20 * gid_index, val);
}

void hns_roce_v2_set_mac(struct hns_roce_dev *hr_dev, u8 phy_port, u8 *addr)
{
	u16 reg_smac_h;
	u16 *p_h;
	u32 val;

	u32 *p = (u32 *)(&addr[0]);
	u32 reg_smac_l = *p;

	roce_raw_write(reg_smac_l, hr_dev->reg_base + ROCEE_VF_SMAC_CFG0_REG +
		       0x08 * phy_port);
	val = roce_read(hr_dev, ROCEE_VF_SMAC_CFG1_REG + 0x08 * phy_port);

	p_h = (u16 *)(&addr[4]);
	reg_smac_h  = *p_h;
	roce_set_field(val, ROCEE_VF_SMAC_CFG1_VF_SMAC_H_M,
		       ROCEE_VF_SMAC_CFG1_VF_SMAC_H_S, reg_smac_h);
	roce_write(hr_dev, ROCEE_VF_SMAC_CFG1_REG + 0x08 * phy_port,
		   val);
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

static void *get_cqe_v2(struct hns_roce_cq *hr_cq, int n)
{
	return hns_roce_buf_offset(&hr_cq->hr_buf.hr_buf,
				   n * HNS_ROCE_V2_CQE_ENTRY_SIZE);
}

static void *get_sw_cqe_v2(struct hns_roce_cq *hr_cq, int n)
{
	struct hns_roce_v2_cqe *cqe = get_cqe_v2(hr_cq, n & hr_cq->ib_cq.cqe);

	/* Get cqe when Owner bit is Conversely with the MSB of cons_idx */
	return (roce_get_bit(cqe->byte_4, V2_CQE_BYTE_4_OWNER_S) ^
		!!(n & (hr_cq->ib_cq.cqe + 1))) ? cqe : NULL;
}

static struct hns_roce_v2_cqe *next_cqe_sw_v2(struct hns_roce_cq *hr_cq)
{
	return get_sw_cqe_v2(hr_cq, hr_cq->cons_index);
}

void hns_roce_v2_cq_set_ci(struct hns_roce_cq *hr_cq, u32 cons_index)
{
	struct hns_roce_v2_cq_db cq_db;

	cq_db.byte_4 = 0;
	cq_db.parameter = 0;

	roce_set_field(cq_db.byte_4, V2_CQ_DB_BYTE_4_TAG_M,
		       V2_CQ_DB_BYTE_4_TAG_S, hr_cq->cqn);

	roce_set_field(cq_db.byte_4, V2_CQ_DB_BYTE_4_CMD_M,
		       V2_CQ_DB_BYTE_4_CMD_S, HNS_ROCE_V2_CQ_DB_PTR);

	roce_set_field(cq_db.parameter, V2_CQ_DB_PARAMETER_CONS_IDX_M,
		       V2_CQ_DB_PARAMETER_CONS_IDX_S,
		       cons_index & ((hr_cq->cq_depth << 1) - 1));
	roce_set_field(cq_db.parameter, V2_CQ_DB_PARAMETER_CMD_SN_M,
		       V2_CQ_DB_PARAMETER_CMD_SN_S, 1);
	roce_set_bit(cq_db.parameter, V2_CQ_DB_PARAMETER_NOTIFY_S, 0);

	hns_roce_write64_k((__be32 *)&cq_db, hr_cq->cq_db_l);

}

void hns_roce_v2_write_cqc(struct hns_roce_dev *hr_dev,
			   struct hns_roce_cq *hr_cq, void *mb_buf, u64 *mtts,
			   dma_addr_t dma_handle, int nent, u32 vector)
{
	struct hns_roce_v2_cq_context *cq_context;

	cq_context = mb_buf;
	memset(cq_context, 0, sizeof(*cq_context));

	/* Register cq_context members */
	roce_set_field(cq_context->byte_4, V2_CQC_BYTE_4_CQ_ST_M,
		       V2_CQC_BYTE_4_CQ_ST_S, V2_CQ_STATE_VALID);

	roce_set_bit(cq_context->byte_4, V2_CQC_BYTE_4_POLL_S, 0);

	roce_set_bit(cq_context->byte_4, V2_CQC_BYTE_4_SE_S, 0);

	roce_set_bit(cq_context->byte_4, V2_CQC_BYTE_4_CMD_SN_S, 0);

	roce_set_field(cq_context->byte_4, V2_CQC_BYTE_4_SHIFT_M,
		       V2_CQC_BYTE_4_SHIFT_S, ilog2((unsigned int)nent));

	roce_set_field(cq_context->byte_4, V2_CQC_BYTE_4_CEQN_M,
		       V2_CQC_BYTE_4_CEQN_S, vector);

	roce_set_field(cq_context->byte_4, V2_CQC_BYTE_4_PAGE_OFFSET_M,
		       V2_CQC_BYTE_4_PAGE_OFFSET_S, 0);

	cq_context->byte_4 = cpu_to_le32(cq_context->byte_4);

	roce_set_field(cq_context->byte_8, V2_CQC_BYTE_8_CQN_M,
		       V2_CQC_BYTE_8_CQN_S, hr_cq->cqn);

	cq_context->cqe_cur_blk_addr = (u32)(mtts[0] >> PAGE_ADDR_SHIFT);
	cq_context->cqe_cur_blk_addr =
				cpu_to_le32(cq_context->cqe_cur_blk_addr);

	roce_set_field(cq_context->byte_16, V2_CQC_BYTE_16_CQE_CUR_BLK_ADDR_M,
		       V2_CQC_BYTE_16_CQE_CUR_BLK_ADDR_S,
		       cpu_to_le32((mtts[0]) >> (32 + PAGE_ADDR_SHIFT)));

	roce_set_field(cq_context->byte_16, V2_CQC_BYTE_16_CQE_HOP_NUM_M,
		       V2_CQC_BYTE_16_CQE_HOP_NUM_S, hr_dev->caps.cqe_hop_num ==
		       HNS_ROCE_HOP_NUM_0 ? 0 : hr_dev->caps.cqe_hop_num);

	cq_context->cqe_nxt_blk_addr = (u32)(mtts[1] >> PAGE_ADDR_SHIFT);
	roce_set_field(cq_context->byte_24, V2_CQC_BYTE_24_CQE_NXT_BLK_ADDR_M,
		       V2_CQC_BYTE_24_CQE_NXT_BLK_ADDR_S,
		       cpu_to_le32((mtts[1]) >> (32 + PAGE_ADDR_SHIFT)));

	roce_set_field(cq_context->byte_24, V2_CQC_BYTE_24_CQE_BA_PG_SZ_M,
		       V2_CQC_BYTE_24_CQE_BA_PG_SZ_S,
		       hr_dev->caps.cqe_ba_pg_sz);
	roce_set_field(cq_context->byte_24, V2_CQC_BYTE_24_CQE_BUF_PG_SZ_M,
		       V2_CQC_BYTE_24_CQE_BUF_PG_SZ_S,
		       hr_dev->caps.cqe_buf_pg_sz);
	roce_set_field(cq_context->byte_28, V2_CQC_BYTE_28_CQ_PRODUCER_IDX_M,
		       V2_CQC_BYTE_28_CQ_PRODUCER_IDX_S, 0);

	roce_set_field(cq_context->byte_32, V2_CQC_BYTE_32_CQ_CONSUMER_IDX_M,
		       V2_CQC_BYTE_32_CQ_CONSUMER_IDX_S, 0);

	cq_context->cqe_ba = (u32)(dma_handle >> 3);

	roce_set_field(cq_context->byte_40, V2_CQC_BYTE_40_CQE_BA_M,
		       V2_CQC_BYTE_40_CQE_BA_S, (dma_handle >> (32 + 3)));
	roce_set_field(cq_context->byte_52, V2_CQC_BYTE_52_CQE_CNT_M,
		       V2_CQC_BYTE_52_CQE_CNT_S, 0);
	cq_context->cqe_report_timer = 0;
	roce_set_field(cq_context->byte_64, V2_CQC_BYTE_64_SE_CQE_IDX_M,
		       V2_CQC_BYTE_64_SE_CQE_IDX_S, 0);
}

int hns_roce_v2_req_notify_cq(struct ib_cq *ibcq, enum ib_cq_notify_flags flags)
{
	struct hns_roce_cq *hr_cq = to_hr_cq(ibcq);
	u32 notification_flag;
	u32 doorbell[2];

	doorbell[0] = 0;
	doorbell[1] = 0;

	notification_flag = (flags & IB_CQ_SOLICITED_MASK) == IB_CQ_SOLICITED ?
			     V2_CQ_DB_REQ_NOT : V2_CQ_DB_REQ_NOT_SOL;
	/*
	 * flags = 0; Notification Flag = 1, next
	 * flags = 1; Notification Flag = 0, solocited
	 */
	roce_set_field(doorbell[0], V2_CQ_DB_BYTE_4_TAG_M, V2_DB_BYTE_4_TAG_S,
		       hr_cq->cqn);
	roce_set_field(doorbell[0], V2_CQ_DB_BYTE_4_CMD_M, V2_DB_BYTE_4_CMD_S,
		       HNS_ROCE_V2_CQ_DB_NTR);
	roce_set_field(doorbell[1], V2_CQ_DB_PARAMETER_CONS_IDX_M,
		       V2_CQ_DB_PARAMETER_CONS_IDX_S,
		       hr_cq->cons_index & ((hr_cq->cq_depth << 1) - 1));
	roce_set_field(doorbell[1], V2_CQ_DB_PARAMETER_CMD_SN_M,
		       V2_CQ_DB_PARAMETER_CMD_SN_S, 1);
	roce_set_bit(doorbell[1], V2_CQ_DB_PARAMETER_NOTIFY_S,
		     notification_flag);

	hns_roce_write64_k(doorbell, hr_cq->cq_db_l);

	return 0;
}

static int hns_roce_v2_poll_one(struct hns_roce_cq *hr_cq,
				struct hns_roce_qp **cur_qp, struct ib_wc *wc)
{
	struct hns_roce_v2_cqe *cqe;
	struct hns_roce_wq *wq;
	int is_send;
	u16 wqe_ctr;
	u32 opcode;
	u32 status;
	int qpn;
	struct hns_roce_qp *hr_qp;
	struct hns_roce_dev *hr_dev = to_hr_dev(hr_cq->ib_cq.device);
	struct device *dev = hr_dev->dev;

	/* Find cqe according consumer index */
	cqe = next_cqe_sw_v2(hr_cq);
	if (!cqe)
		return -EAGAIN;

	++hr_cq->cons_index;
	/* Memory barrier */
	rmb();

	/* 0->SQ, 1->RQ */
	is_send = !roce_get_bit(cqe->byte_4, V2_CQE_BYTE_4_S_R_S);

	qpn = roce_get_field(cqe->byte_16, V2_CQE_BYTE_16_LCL_QPN_M,
				V2_CQE_BYTE_16_LCL_QPN_S);

	if (!*cur_qp || (qpn & HNS_ROCE_V2_CQE_QPN_MASK) != (*cur_qp)->qpn) {
		hr_qp = __hns_roce_qp_lookup(hr_dev, qpn);
		if (unlikely(!hr_qp)) {
			dev_err(dev, "CQ %06lx with entry for unknown QPN %06x\n",
				hr_cq->cqn, (qpn & HNS_ROCE_V2_CQE_QPN_MASK));
			return -EINVAL;
		}
		*cur_qp = hr_qp;
	}

	wc->qp = &(*cur_qp)->ibqp;
	wc->vendor_err = 0;

	status = roce_get_field(cqe->byte_4, V2_CQE_BYTE_4_STATUS_M,
				V2_CQE_BYTE_4_STATUS_S);
	switch (status & HNS_ROCE_V2_CQE_STATUS_MASK) {
	case HNS_ROCE_CQE_V2_SUCCESS:
		wc->status = IB_WC_SUCCESS;
		break;
	case HNS_ROCE_CQE_V2_LOCAL_LENGTH_ERR:
		wc->status = IB_WC_LOC_LEN_ERR;
		break;
	case HNS_ROCE_CQE_V2_LOCAL_QP_OP_ERR:
		wc->status = IB_WC_LOC_QP_OP_ERR;
		break;
	case HNS_ROCE_CQE_V2_LOCAL_PROT_ERR:
		wc->status = IB_WC_LOC_PROT_ERR;
		break;
	case HNS_ROCE_CQE_V2_WR_FLUSH_ERR:
		wc->status = IB_WC_WR_FLUSH_ERR;
		break;
	case HNS_ROCE_CQE_V2_MW_BIND_ERR:
		wc->status = IB_WC_MW_BIND_ERR;
		break;
	case HNS_ROCE_CQE_V2_BAD_RESP_ERR:
		wc->status = IB_WC_BAD_RESP_ERR;
		break;
	case HNS_ROCE_CQE_V2_LOCAL_ACCESS_ERR:
		wc->status = IB_WC_LOC_ACCESS_ERR;
		break;
	case HNS_ROCE_CQE_V2_REMOTE_INVAL_REQ_ERR:
		wc->status = IB_WC_REM_INV_REQ_ERR;
		break;
	case HNS_ROCE_CQE_V2_REMOTE_ACCESS_ERR:
		wc->status = IB_WC_REM_ACCESS_ERR;
		break;
	case HNS_ROCE_CQE_V2_REMOTE_OP_ERR:
		wc->status = IB_WC_REM_OP_ERR;
		break;
	case HNS_ROCE_CQE_V2_TRANSPORT_RETRY_EXC_ERR:
		wc->status = IB_WC_RETRY_EXC_ERR;
		break;
	case HNS_ROCE_CQE_V2_RNR_RETRY_EXC_ERR:
		wc->status = IB_WC_RNR_RETRY_EXC_ERR;
		break;
	case HNS_ROCE_CQE_V2_REMOTE_ABORT_ERR:
		wc->status = IB_WC_REM_ABORT_ERR;
		break;
	default:
		wc->status = IB_WC_GENERAL_ERR;
		break;
	}

	/* CQE status error, directly return */
	if (wc->status != IB_WC_SUCCESS)
		return 0;

	if (is_send) {
		wc->wc_flags = 0;
		/* SQ conresponding to CQE */
		switch (roce_get_field(cqe->byte_4, V2_CQE_BYTE_4_OPCODE_M,
				       V2_CQE_BYTE_4_OPCODE_S) & 0x1f) {
		case HNS_ROCE_SQ_OPCODE_SEND:
			wc->opcode = IB_WC_SEND;
			break;
		case HNS_ROCE_SQ_OPCODE_SEND_WITH_INV:
			wc->opcode = IB_WC_SEND;
			break;
		case HNS_ROCE_SQ_OPCODE_SEND_WITH_IMM:
			wc->opcode = IB_WC_SEND;
			wc->wc_flags |= IB_WC_WITH_IMM;
			break;
		case HNS_ROCE_SQ_OPCODE_RDMA_READ:
			wc->opcode = IB_WC_RDMA_READ;
			wc->byte_len = le32_to_cpu(cqe->byte_cnt);
			break;
		case HNS_ROCE_SQ_OPCODE_RDMA_WRITE:
			wc->opcode = IB_WC_RDMA_WRITE;
			break;
		case HNS_ROCE_SQ_OPCODE_RDMA_WRITE_WITH_IMM:
			wc->opcode = IB_WC_RDMA_WRITE;
			wc->wc_flags |= IB_WC_WITH_IMM;
			break;
		case HNS_ROCE_SQ_OPCODE_LOCAL_INV:
			wc->opcode = IB_WC_LOCAL_INV;
			wc->wc_flags |= IB_WC_WITH_INVALIDATE;
			break;
		case HNS_ROCE_SQ_OPCODE_ATOMIC_COMP_AND_SWAP:
			wc->opcode = IB_WC_COMP_SWAP;
			wc->byte_len  = 8;
			break;
		case HNS_ROCE_SQ_OPCODE_ATOMIC_FETCH_AND_ADD:
			wc->opcode = IB_WC_FETCH_ADD;
			wc->byte_len  = 8;
			break;
		case HNS_ROCE_SQ_OPCODE_ATOMIC_MASK_COMP_AND_SWAP:
			wc->opcode = IB_WC_MASKED_COMP_SWAP;
			wc->byte_len  = 8;
			break;
		case HNS_ROCE_SQ_OPCODE_ATOMIC_MASK_FETCH_AND_ADD:
			wc->opcode = IB_WC_MASKED_FETCH_ADD;
			wc->byte_len  = 8;
			break;
		case HNS_ROCE_SQ_OPCODE_FAST_REG_WR:
			wc->opcode = IB_WC_REG_MR;
			break;
		case HNS_ROCE_SQ_OPCODE_BIND_MW:
			wc->opcode = IB_WC_REG_MR;
			break;
		default:
			wc->status = IB_WC_GENERAL_ERR;
			break;
		}

		wq = &(*cur_qp)->sq;
		if ((*cur_qp)->sq_signal_bits) {
			/*
			 * If sg_signal_bit is 1,
			 * firstly tail pointer updated to wqe
			 * which current cqe correspond to
			 */
			wqe_ctr = (u16)roce_get_field(cqe->byte_4,
						      V2_CQE_BYTE_4_WQE_INDX_M,
						      V2_CQE_BYTE_4_WQE_INDX_S);
			wq->tail += (wqe_ctr - (u16)wq->tail) &
				    (wq->wqe_cnt - 1);
		}

		wc->wr_id = wq->wrid[wq->tail & (wq->wqe_cnt - 1)];
		++wq->tail;
	} else {
		/* RQ conrespond to CQE */
		wc->byte_len = le32_to_cpu(cqe->byte_cnt);

		opcode = roce_get_field(cqe->byte_4, V2_CQE_BYTE_4_OPCODE_M,
					V2_CQE_BYTE_4_OPCODE_S);
		switch (opcode & 0x1f) {
		case HNS_ROCE_V2_OPCODE_RDMA_WRITE_IMM:
			wc->opcode = IB_WC_RECV_RDMA_WITH_IMM;
			wc->wc_flags = IB_WC_WITH_IMM;
			wc->ex.imm_data = le32_to_cpu(cqe->rkey_immtdata);
			break;
		case HNS_ROCE_V2_OPCODE_SEND:
			wc->opcode = IB_WC_RECV;
			wc->wc_flags = 0;
		case HNS_ROCE_V2_OPCODE_SEND_WITH_IMM:
			wc->opcode = IB_WC_RECV;
			wc->wc_flags = IB_WC_WITH_IMM;
			wc->ex.imm_data = le32_to_cpu(cqe->rkey_immtdata);
			break;
		case HNS_ROCE_V2_OPCODE_SEND_WITH_INV:
			wc->opcode = IB_WC_RECV;
			wc->wc_flags = IB_WC_WITH_INVALIDATE;
			wc->ex.invalidate_rkey = cqe->rkey_immtdata;
			break;
		default:
			wc->status = IB_WC_GENERAL_ERR;
			break;
		}

		/* Update tail pointer, record wr_id */
		wq = &(*cur_qp)->rq;
		wc->wr_id = wq->wrid[wq->tail & (wq->wqe_cnt - 1)];
		++wq->tail;

		wc->sl = (u8)roce_get_field(cqe->byte_32, V2_CQE_BYTE_32_SL_M,
					    V2_CQE_BYTE_32_SL_S);
		wc->src_qp = (u8)roce_get_field(cqe->byte_32,
						V2_CQE_BYTE_32_RMT_QPN_M,
						V2_CQE_BYTE_32_RMT_QPN_S);
		wc->wc_flags |= (roce_get_bit(cqe->byte_32,
					      V2_CQE_BYTE_32_GRH_S) ?
					      IB_WC_GRH : 0);
	}

	return 0;
}

static int hns_roce_v2_poll_cq(struct ib_cq *ibcq, int num_entries,
			       struct ib_wc *wc)
{
	struct hns_roce_cq *hr_cq = to_hr_cq(ibcq);
	struct hns_roce_qp *cur_qp = NULL;
	int ret = 0;
	unsigned long flags;
	int npolled;

	spin_lock_irqsave(&hr_cq->lock, flags);

	for (npolled = 0; npolled < num_entries; ++npolled) {
		ret = hns_roce_v2_poll_one(hr_cq, &cur_qp, wc + npolled);
		if (ret)
			break;
	}

	if (npolled) {
		if ((to_hr_dev(ibcq->device)->hw_rev) == HNS_ROCE_HW_VER1)
			*hr_cq->tptr_addr = hr_cq->cons_index &
					    ((hr_cq->cq_depth << 1) - 1);

		/* Memroy barrier */
		wmb();
		hns_roce_v2_cq_set_ci(hr_cq, hr_cq->cons_index);
	}

	spin_unlock_irqrestore(&hr_cq->lock, flags);

	if (!ret || ret == -EAGAIN)
		return npolled;

	return ret;
}

int hns_roce_v2_set_hem(struct hns_roce_dev *hr_dev,
		struct hns_roce_hem_table *table, int obj, int step_idx)
{
	struct device *dev = hr_dev->dev;
	struct hns_roce_cmd_mailbox *mailbox;
	struct hns_roce_hem_iter iter;
	struct hns_roce_hem_mhop mhop;
	struct hns_roce_hem *hem;
	unsigned long mhop_obj = obj;
	int i, j, k;
	int ret = 0;
	u64 hem_idx = 0;
	u64 l1_idx = 0;
	u64 bt_ba = 0;
	u32 chunk_ba_num;
	u32 hop_num;
	u16 op = 0xff;

	if (!hns_roce_check_whether_mhop(hr_dev, table->type))
		return 0;

	hns_roce_calc_hem_mhop(hr_dev, table, &mhop_obj, &mhop);
	i = mhop.l0_idx;
	j = mhop.l1_idx;
	k = mhop.l2_idx;
	hop_num = mhop.hop_num;
	chunk_ba_num = mhop.bt_chunk_size / 8;

	if (hop_num == 2) {
		hem_idx = i * chunk_ba_num * chunk_ba_num + j * chunk_ba_num +
			  k;
		l1_idx = i * chunk_ba_num + j;
	} else if (hop_num == 1) {
		hem_idx = i * chunk_ba_num + j;
	} else if (hop_num == HNS_ROCE_HOP_NUM_0) {
		hem_idx = i;
	}

	switch (table->type) {
	case HEM_TYPE_QPC:
		op = HNS_ROCE_CMD_WRITE_QPC_BT0;
		break;
	case HEM_TYPE_MTPT:
		op = HNS_ROCE_CMD_WRITE_MPT_BT0;
		break;
	case HEM_TYPE_CQC:
		op = HNS_ROCE_CMD_WRITE_CQC_BT0;
		break;
	case HEM_TYPE_SRQC:
		op = HNS_ROCE_CMD_WRITE_SRQC_BT0;
		break;
	default:
		dev_warn(dev, "Table %d not to be written by mailbox!\n",
			 table->type);
		return 0;
	}
	op += step_idx;

	mailbox = hns_roce_alloc_cmd_mailbox(hr_dev);
	if (IS_ERR(mailbox))
		return PTR_ERR(mailbox);

	if (check_whether_last_step(hop_num, step_idx)) {
		hem = table->hem[hem_idx];
		for (hns_roce_hem_first(hem, &iter);
		     !hns_roce_hem_last(&iter); hns_roce_hem_next(&iter)) {
			bt_ba = hns_roce_hem_addr(&iter);

			/* configure the ba, tag, and op */
			ret = hns_roce_cmd_mbox(hr_dev, bt_ba, mailbox->dma,
						obj, 0, op,
						HNS_ROCE_CMD_TIMEOUT_MSECS);
		}
	} else {
		if (step_idx == 0)
			bt_ba = table->bt_l0_dma_addr[i];
		else if (step_idx == 1 && hop_num == 2)
			bt_ba = table->bt_l1_dma_addr[l1_idx];

		/* configure the ba, tag, and op */
		ret = hns_roce_cmd_mbox(hr_dev, bt_ba, mailbox->dma, obj,
					0, op, HNS_ROCE_CMD_TIMEOUT_MSECS);
	}

	hns_roce_free_cmd_mailbox(hr_dev, mailbox);
	return ret;
}

int hns_roce_v2_clear_hem(struct hns_roce_dev *hr_dev,
		struct hns_roce_hem_table *table, int obj, int step_idx)
{
	struct device *dev = hr_dev->dev;
	struct hns_roce_cmd_mailbox *mailbox;
	int ret = 0;
	u16 op = 0xff;

	if (!hns_roce_check_whether_mhop(hr_dev, table->type))
		return 0;

	switch (table->type) {
	case HEM_TYPE_QPC:
		op = HNS_ROCE_CMD_DESTROY_QPC_BT0;
		break;
	case HEM_TYPE_MTPT:
		op = HNS_ROCE_CMD_DESTROY_MPT_BT0;
		break;
	case HEM_TYPE_CQC:
		op = HNS_ROCE_CMD_DESTROY_CQC_BT0;
		break;
	case HEM_TYPE_SRQC:
		op = HNS_ROCE_CMD_DESTROY_SRQC_BT0;
		break;
	default:
		dev_warn(dev, "Table %d not to be destroyed by mailbox!\n",
			 table->type);
		return 0;
	}
	op += step_idx;

	mailbox = hns_roce_alloc_cmd_mailbox(hr_dev);
	if (IS_ERR(mailbox))
		return PTR_ERR(mailbox);

	/* configure the tag and op */
	ret = hns_roce_cmd_mbox(hr_dev, 0, mailbox->dma, obj, 0, op,
				HNS_ROCE_CMD_TIMEOUT_MSECS);

	hns_roce_free_cmd_mailbox(hr_dev, mailbox);
	return ret;
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
	.post_mbox = hns_roce_v2_post_mbox,
	.chk_mbox = hns_roce_v2_chk_mbox,
	.set_gid = hns_roce_v2_set_gid,
	.set_mac = hns_roce_v2_set_mac,
	.set_mtu = hns_roce_v2_set_mtu,
	.write_mtpt = hns_roce_v2_write_mtpt,
	.write_cqc = hns_roce_v2_write_cqc,
	.set_hem = hns_roce_v2_set_hem,
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

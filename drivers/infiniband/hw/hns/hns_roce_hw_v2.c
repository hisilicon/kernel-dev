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

static inline enum ib_qp_state to_ib_qp_st(enum hns_roce_v2_qp_state state)
{
	switch (state) {
	case HNS_ROCE_QP_ST_RST:	return IB_QPS_RESET;
	case HNS_ROCE_QP_ST_INIT:	return IB_QPS_INIT;
	case HNS_ROCE_QP_ST_RTR:	return IB_QPS_RTR;
	case HNS_ROCE_QP_ST_RTS:	return IB_QPS_RTS;
	case HNS_ROCE_QP_ST_SQ_DRAINING:
	case HNS_ROCE_QP_ST_SQD:	return IB_QPS_SQD;
	case HNS_ROCE_QP_ST_SQER:	return IB_QPS_SQE;
	case HNS_ROCE_QP_ST_ERR:	return IB_QPS_ERR;
	default:			return -1;
	}
}

static void set_data_seg_v2(struct hns_roce_v2_wqe_data_seg *dseg,
			    struct ib_sge *sg)
{
	dseg->lkey = cpu_to_le32(sg->lkey);
	dseg->addr = cpu_to_le64(sg->addr);
	dseg->len  = cpu_to_le32(sg->length);
}

int hns_roce_v2_post_send(struct ib_qp *ibqp, struct ib_send_wr *wr,
			  struct ib_send_wr **bad_wr)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ibqp->device);
	struct hns_roce_v2_rc_send_wqe *rc_sq_wqe;
	struct hns_roce_qp *qp = to_hr_qp(ibqp);
	struct hns_roce_v2_wqe_data_seg *dseg;
	struct device *dev = hr_dev->dev;
	struct hns_roce_v2_db sq_db;
	unsigned int sge_ind = 0;
	unsigned int wqe_sz = 0;
	unsigned long flags;
	unsigned int ind;
	void *wqe = NULL;
	int ret = 0;
	int nreq;
	int i;

	if (unlikely(ibqp->qp_type != IB_QPT_GSI &&
		     ibqp->qp_type != IB_QPT_RC && ibqp->qp_type != IB_QPT_UC &&
		     ibqp->qp_type != IB_QPT_UD) &&
		     ibqp->qp_type != IB_QPT_XRC_INI &&
		     ibqp->qp_type != IB_QPT_XRC_TGT) {
		dev_err(dev, "Not supported QP(0x%x)type\n", ibqp->qp_type);
		*bad_wr = NULL;
		return -EOPNOTSUPP;
	}

	if (unlikely(qp->state != IB_QPS_RTS && qp->state != IB_QPS_SQD)) {
		dev_err(dev, "Post WQE fail when QP is %d!\n", qp->state);
		*bad_wr = wr;
		return -EINVAL;
	}

	spin_lock_irqsave(&qp->sq.lock, flags);
	ind = qp->sq_next_wqe;
	sge_ind = qp->next_sge;

	for (nreq = 0; wr; ++nreq, wr = wr->next) {
		if (hns_roce_wq_overflow(&qp->sq, nreq, qp->ibqp.send_cq)) {
			ret = -ENOMEM;
			*bad_wr = wr;
			goto out;
		}

		if (unlikely(wr->num_sge > qp->sq.max_gs)) {
			dev_err(dev, "num_sge=%d > qp->sq.max_gs=%d\n",
				wr->num_sge, qp->sq.max_gs);
			ret = -EINVAL;
			*bad_wr = wr;
			goto out;
		}

		wqe = get_send_wqe(qp, ind & (qp->sq.wqe_cnt - 1));
		qp->sq.wrid[(qp->sq.head + nreq) & (qp->sq.wqe_cnt - 1)] =
								      wr->wr_id;

		/* Corresponding to the QP type, wqe process separately */
		if (ibqp->qp_type == IB_QPT_GSI) {
			spin_unlock_irqrestore(&qp->sq.lock, flags);
			dev_info(dev, "Not support post send for ud wr!");
			return ret;
		} else if (ibqp->qp_type == IB_QPT_RC) {
			rc_sq_wqe = wqe;
			memset(rc_sq_wqe, 0, sizeof(*rc_sq_wqe));
			for (i = 0; i < wr->num_sge; i++)
				rc_sq_wqe->msg_len += wr->sg_list[i].length;

			rc_sq_wqe->inv_key_immtdata = send_ieth(wr);

			switch (wr->opcode) {
			case IB_WR_RDMA_READ:
				roce_set_field(rc_sq_wqe->byte_4,
					       V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
					       V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
					       HNS_ROCE_V2_WQE_OP_RDMA_READ);
				rc_sq_wqe->rkey =
					cpu_to_le32(rdma_wr(wr)->rkey);
				rc_sq_wqe->va =
					cpu_to_le64(rdma_wr(wr)->remote_addr);
				break;

			case IB_WR_RDMA_WRITE:
				roce_set_field(rc_sq_wqe->byte_4,
					       V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
					       V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
					       HNS_ROCE_V2_WQE_OP_RDMA_WRITE);
				rc_sq_wqe->rkey =
					cpu_to_le32(rdma_wr(wr)->rkey);
				rc_sq_wqe->va =
					cpu_to_le64(rdma_wr(wr)->remote_addr);
				break;

			case IB_WR_RDMA_WRITE_WITH_IMM:
				roce_set_field(rc_sq_wqe->byte_4,
					V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
					V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
					HNS_ROCE_V2_WQE_OP_RDMA_WRITE_WITH_IMM);
				rc_sq_wqe->rkey =
					cpu_to_le32(rdma_wr(wr)->rkey);
				rc_sq_wqe->va =
					cpu_to_le64(rdma_wr(wr)->remote_addr);
				break;

			case IB_WR_SEND:
				roce_set_field(rc_sq_wqe->byte_4,
					       V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
					       V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
					       HNS_ROCE_V2_WQE_OP_SEND);
				break;

			case IB_WR_SEND_WITH_INV:
				roce_set_field(rc_sq_wqe->byte_4,
					V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
					V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
					HNS_ROCE_V2_WQE_OP_SEND_WITH_INV);
				break;

			case IB_WR_SEND_WITH_IMM:
				roce_set_field(rc_sq_wqe->byte_4,
					V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
					V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
					HNS_ROCE_V2_WQE_OP_SEND_WITH_IMM);
				break;

			case IB_WR_LOCAL_INV:
				roce_set_field(rc_sq_wqe->byte_4,
					V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
					V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
					HNS_ROCE_V2_WQE_OP_LOCAL_INV);
				break;

			case IB_WR_ATOMIC_CMP_AND_SWP:
				roce_set_field(rc_sq_wqe->byte_4,
					V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
					V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
					HNS_ROCE_V2_WQE_OP_ATOMIC_CMP_AND_SWAP);
				break;

			case IB_WR_ATOMIC_FETCH_AND_ADD:
				roce_set_field(rc_sq_wqe->byte_4,
				       V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
				       V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
				       HNS_ROCE_V2_WQE_OP_ATOMIC_FETCH_AND_ADD);
				break;

			case IB_WR_MASKED_ATOMIC_CMP_AND_SWP:
				roce_set_field(rc_sq_wqe->byte_4,
				 V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
				 V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
				 HNS_ROCE_V2_WQE_OP_ATOMIC_MASKED_CMP_AND_SWAP);
				break;

			case IB_WR_MASKED_ATOMIC_FETCH_AND_ADD:
				roce_set_field(rc_sq_wqe->byte_4,
				V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
				V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
				HNS_ROCE_V2_WQE_OP_ATOMIC_MASKED_FETCH_AND_ADD);
				break;

			default:
				roce_set_field(rc_sq_wqe->byte_4,
					       V2_RC_SEND_WQE_BYTE_4_OPCODE_M,
					       V2_RC_SEND_WQE_BYTE_4_OPCODE_S,
					       HNS_ROCE_V2_WQE_OP_MASK);
				break;
			}

			roce_set_bit(rc_sq_wqe->byte_4,
				     V2_RC_SEND_WQE_BYTE_4_CQE_S, 1);

			wqe += sizeof(struct hns_roce_v2_rc_send_wqe);
			dseg = wqe;

			if (wr->send_flags & IB_SEND_INLINE && wr->num_sge) {
				if (rc_sq_wqe->msg_len >
					hr_dev->caps.max_sq_inline) {
					ret = -EINVAL;
					*bad_wr = wr;
					dev_err(dev, "inline len(1-%d)=%d, illegal",
						rc_sq_wqe->msg_len,
						hr_dev->caps.max_sq_inline);
					goto out;
				}

				for (i = 0; i < wr->num_sge; i++) {
					memcpy(wqe, ((void *) (uintptr_t)
					       wr->sg_list[i].addr),
					       wr->sg_list[i].length);
					wqe += wr->sg_list[i].length;
					wqe_sz += wr->sg_list[i].length;
				}

				roce_set_bit(rc_sq_wqe->byte_4,
					     V2_RC_SEND_WQE_BYTE_4_INLINE_S, 1);
			} else {
				/*sqe num is two */
				if (wr->num_sge <= 2) {
					for (i = 0; i < wr->num_sge; i++)
						set_data_seg_v2(dseg + i,
							       wr->sg_list + i);
				} else {
					roce_set_field(rc_sq_wqe->byte_20,
				     V2_RC_SEND_WQE_BYTE_20_MSG_START_SGE_IDX_M,
				     V2_RC_SEND_WQE_BYTE_20_MSG_START_SGE_IDX_S,
				     sge_ind & (qp->sge.sge_cnt - 1));

					for (i = 0; i < 2; i++)
						set_data_seg_v2(dseg + i,
							       wr->sg_list + i);

					dseg = get_send_extend_sge(qp, sge_ind &
							 (qp->sge.sge_cnt - 1));

					for (i = 0; i < wr->num_sge - 2; i++) {
						set_data_seg_v2(dseg + i,
							   wr->sg_list + 2 + i);
						sge_ind++;
					}
				}

				roce_set_field(rc_sq_wqe->byte_16,
					V2_RC_SEND_WQE_BYTE_16_SGE_NUM_M,
					V2_RC_SEND_WQE_BYTE_16_SGE_NUM_S,
					wr->num_sge);
				wqe_sz += wr->num_sge *
				  sizeof(struct hns_roce_v2_wqe_data_seg);
			}
			ind++;
		} else if (ibqp->qp_type == IB_QPT_XRC_INI ||
			ibqp->qp_type == IB_QPT_XRC_TGT) {
			;
		} else {
			dev_err(dev, "Illegal qp_type!");
			return 0;
		}
	}

out:
	if (likely(nreq)) {
		qp->sq.head += nreq;
		/* Memory barrier */
		wmb();

		sq_db.byte_4 = 0;
		sq_db.parameter = 0;

		roce_set_field(sq_db.byte_4, V2_DB_BYTE_4_TAG_M,
			       V2_DB_BYTE_4_TAG_S, qp->doorbell_qpn);
		roce_set_field(sq_db.byte_4, V2_DB_BYTE_4_CMD_M,
			       V2_DB_BYTE_4_CMD_S, HNS_ROCE_V2_SQ_DB);
		roce_set_field(sq_db.parameter, V2_DB_PARAMETER_CONS_IDX_M,
			       V2_DB_PARAMETER_CONS_IDX_S,
			       qp->sq.head & ((qp->sq.wqe_cnt << 1) - 1));
		roce_set_field(sq_db.parameter, V2_DB_PARAMETER_SL_M,
			       V2_DB_PARAMETER_SL_S, qp->sl);

		hns_roce_write64_k((__be32 *)&sq_db, qp->sq.db_reg_l);

		qp->sq_next_wqe = ind;
		qp->next_sge = sge_ind;
	}

	spin_unlock_irqrestore(&qp->sq.lock, flags);

	return ret;
}

int hns_roce_v2_post_recv(struct ib_qp *ibqp, struct ib_recv_wr *wr,
			  struct ib_recv_wr **bad_wr)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ibqp->device);
	struct hns_roce_qp *hr_qp = to_hr_qp(ibqp);
	struct hns_roce_v2_wqe_data_seg *dseg;
	struct device *dev = hr_dev->dev;
	struct hns_roce_v2_db rq_db;
	unsigned long flags;
	void *wqe = NULL;
	int ret = 0;
	int nreq;
	int ind;
	int i;

	spin_lock_irqsave(&hr_qp->rq.lock, flags);
	ind = hr_qp->rq.head & (hr_qp->rq.wqe_cnt - 1);

	if (hr_qp->state == IB_QPS_RESET || hr_qp->state == IB_QPS_ERR) {
		spin_unlock_irqrestore(&hr_qp->rq.lock, flags);
		*bad_wr = wr;
		return -EINVAL;
	}

	for (nreq = 0; wr; ++nreq, wr = wr->next) {
		if (hns_roce_wq_overflow(&hr_qp->rq, nreq,
			hr_qp->ibqp.recv_cq)) {
			ret = -ENOMEM;
			*bad_wr = wr;
			goto out;
		}

		if (unlikely(wr->num_sge > hr_qp->rq.max_gs)) {
			dev_err(dev, "rq:num_sge=%d > qp->sq.max_gs=%d\n",
				wr->num_sge, hr_qp->rq.max_gs);
			ret = -EINVAL;
			*bad_wr = wr;
			goto out;
		}

		wqe = get_recv_wqe(hr_qp, ind);
		dseg = (struct hns_roce_v2_wqe_data_seg *)wqe;
		for (i = 0; i < wr->num_sge; i++)
			set_data_seg_v2(dseg + i, wr->sg_list + i);

		hr_qp->rq.wrid[ind] = wr->wr_id;

		ind = (ind + 1) & (hr_qp->rq.wqe_cnt - 1);
	}

out:
	if (likely(nreq)) {
		hr_qp->rq.head += nreq;
		/* Memory barrier */
		wmb();

		rq_db.byte_4 = 0;
		rq_db.parameter = 0;

		roce_set_field(rq_db.byte_4, V2_DB_BYTE_4_TAG_M,
			       V2_DB_BYTE_4_TAG_S, hr_qp->qpn);
		roce_set_field(rq_db.byte_4, V2_DB_BYTE_4_CMD_M,
			       V2_DB_BYTE_4_CMD_S, HNS_ROCE_V2_RQ_DB);
		roce_set_field(rq_db.parameter, V2_DB_PARAMETER_CONS_IDX_M,
			       V2_DB_PARAMETER_CONS_IDX_S, hr_qp->rq.head);

		hns_roce_write64_k((__be32 *)&rq_db, hr_qp->rq.db_reg_l);
	}
	spin_unlock_irqrestore(&hr_qp->rq.lock, flags);

	return ret;
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
	caps->pbl_ba_pg_sz	= 0;
	caps->pbl_buf_pg_sz	= 0;
	caps->pbl_hop_num	= HNS_ROCE_PBL_HOP_NUM;
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
	struct hns_roce_v2_mpt_entry *mpt_entry;
	struct scatterlist *sg;
	u64 *pages;
	int entry;
	int i;

	/* MPT filled into mailbox buf */
	mpt_entry = mb_buf;
	memset(mpt_entry, 0, sizeof(*mpt_entry));

	roce_set_field(mpt_entry->mpt_byte_4, V2_MPT_BYTE_4_MPT_ST_M,
		       V2_MPT_BYTE_4_MPT_ST_S, V2_MPT_ST_VALID);

	roce_set_field(mpt_entry->mpt_byte_4, V2_MPT_BYTE_4_PBL_HOP_NUM_M,
		       V2_MPT_BYTE_4_PBL_HOP_NUM_S, mr->pbl_hop_num ==
		       HNS_ROCE_HOP_NUM_0 ? 0 : mr->pbl_hop_num);

	roce_set_field(mpt_entry->mpt_byte_4, V2_MPT_BYTE_4_PBL_BA_PG_SZ_M,
		       V2_MPT_BYTE_4_PBL_BA_PG_SZ_S, mr->pbl_ba_pg_sz);

	roce_set_field(mpt_entry->mpt_byte_4, V2_MPT_BYTE_4_PD_M,
		       V2_MPT_BYTE_4_PD_S, mr->pd);
	mpt_entry->mpt_byte_4 = cpu_to_le32(mpt_entry->mpt_byte_4);

	roce_set_bit(mpt_entry->mpt_byte_8, V2_MPT_BYTE_8_RA_EN_S, 0);
	roce_set_bit(mpt_entry->mpt_byte_8, V2_MPT_BYTE_8_R_INV_EN_S, 1);
	roce_set_bit(mpt_entry->mpt_byte_8, V2_MPT_BYTE_8_L_INV_EN_S, 0);
	roce_set_bit(mpt_entry->mpt_byte_8, V2_MPT_BYTE_8_BIND_EN_S,
		     (mr->access & IB_ACCESS_MW_BIND ? 1 : 0));
	roce_set_bit(mpt_entry->mpt_byte_8, V2_MPT_BYTE_8_ATOMIC_EN_S, 0);
	roce_set_bit(mpt_entry->mpt_byte_8, V2_MPT_BYTE_8_RR_EN_S,
		     (mr->access & IB_ACCESS_REMOTE_READ ? 1 : 0));
	roce_set_bit(mpt_entry->mpt_byte_8, V2_MPT_BYTE_8_RW_EN_S,
		     (mr->access & IB_ACCESS_REMOTE_WRITE ? 1 : 0));
	roce_set_bit(mpt_entry->mpt_byte_8, V2_MPT_BYTE_8_LW_EN_S,
		     (mr->access & IB_ACCESS_LOCAL_WRITE ? 1 : 0));
	mpt_entry->mpt_byte_8 = cpu_to_le32(mpt_entry->mpt_byte_8);

	roce_set_bit(mpt_entry->mpt_byte_12, V2_MPT_BYTE_12_FRE_S, 0);
	roce_set_bit(mpt_entry->mpt_byte_12, V2_MPT_BYTE_12_PA_S,
		     mr->type == MR_TYPE_MR ? 0 : 1);
	roce_set_bit(mpt_entry->mpt_byte_12, V2_MPT_BYTE_12_ZBVA_S, 0);
	roce_set_bit(mpt_entry->mpt_byte_12, V2_MPT_BYTE_12_SHARE_S, 0);
	roce_set_bit(mpt_entry->mpt_byte_12, V2_MPT_BYTE_12_MR_MW_S, 0);
	roce_set_bit(mpt_entry->mpt_byte_12, V2_MPT_BYTE_12_BPD_S, 0);
	roce_set_bit(mpt_entry->mpt_byte_12, V2_MPT_BYTE_12_BQP_S, 0);
	mpt_entry->mpt_byte_12 = cpu_to_le32(mpt_entry->mpt_byte_12);

	mpt_entry->len_l = cpu_to_le32(lower_32_bits(mr->size));
	mpt_entry->len_h = cpu_to_le32(upper_32_bits(mr->size));
	mpt_entry->lkey = cpu_to_le32(mr->key);
	mpt_entry->va_l = cpu_to_le32(lower_32_bits(mr->iova));
	mpt_entry->va_h = cpu_to_le32(upper_32_bits(mr->iova));

	/* DMA memory register */
	if (mr->type == MR_TYPE_DMA)
		return 0;

	mpt_entry->pbl_size = cpu_to_le32(mr->pbl_size);

	mpt_entry->pbl_ba_l = cpu_to_le32(lower_32_bits(mr->pbl_ba >> 3));
	roce_set_field(mpt_entry->mpt_byte_48, V2_MPT_BYTE_48_PBL_BA_H_M,
		       V2_MPT_BYTE_48_PBL_BA_H_S,
		       upper_32_bits(mr->pbl_ba >> 3));
	mpt_entry->mpt_byte_48 = cpu_to_le32(mpt_entry->mpt_byte_48);

	pages = (u64 *)__get_free_page(GFP_KERNEL);
	if (!pages)
		return -ENOMEM;

	i = 0;
	for_each_sg(mr->umem->sg_head.sgl, sg, mr->umem->nmap, entry) {
		pages[i] = ((u64)sg_dma_address(sg)) >> 6;

		/* Directly record to MTPT table firstly 2 entry */
		if (i >= HNS_ROCE_V2_MAX_INNER_MTPT_NUM - 1)
			break;
		i++;
	}

	mpt_entry->pa0_l = cpu_to_le32(lower_32_bits(pages[0]));
	roce_set_field(mpt_entry->mpt_byte_56, V2_MPT_BYTE_56_PA0_H_M,
		       V2_MPT_BYTE_56_PA0_H_S,
		       upper_32_bits(pages[0]));
	mpt_entry->mpt_byte_56 = cpu_to_le32(mpt_entry->mpt_byte_56);

	mpt_entry->pa1_l = cpu_to_le32(lower_32_bits(pages[1]));
	roce_set_field(mpt_entry->mpt_byte_64, V2_MPT_BYTE_64_PA1_H_M,
		       V2_MPT_BYTE_64_PA1_H_S,
		       upper_32_bits(pages[1]));

	free_page((unsigned long)pages);

	roce_set_field(mpt_entry->mpt_byte_64, V2_MPT_BYTE_64_PBL_BUF_PG_SZ_M,
		       V2_MPT_BYTE_64_PBL_BUF_PG_SZ_S, mr->pbl_buf_pg_sz);
	mpt_entry->mpt_byte_64 = cpu_to_le32(mpt_entry->mpt_byte_64);

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

static void __hns_roce_v2_cq_clean(struct hns_roce_cq *hr_cq, u32 qpn,
				   struct hns_roce_srq *srq)
{
	struct hns_roce_v2_cqe *cqe, *dest;
	u32 prod_index;
	int nfreed = 0;
	u8 owner_bit;

	for (prod_index = hr_cq->cons_index; get_sw_cqe_v2(hr_cq, prod_index);
	     ++prod_index) {
		if (prod_index == hr_cq->cons_index + hr_cq->ib_cq.cqe)
			break;
	}

	/*
	 * Now backwards through the CQ, removing CQ entries
	 * that match our QP by overwriting them with next entries.
	 */
	while ((int) --prod_index - (int) hr_cq->cons_index >= 0) {
		cqe = get_cqe_v2(hr_cq, prod_index & hr_cq->ib_cq.cqe);
		if ((roce_get_field(cqe->byte_16, V2_CQE_BYTE_16_LCL_QPN_M,
				    V2_CQE_BYTE_16_LCL_QPN_S) &
				    HNS_ROCE_V2_CQE_QPN_MASK) == qpn) {
			/* In v1 engine, not support SRQ */
			++nfreed;
		} else if (nfreed) {
			dest = get_cqe_v2(hr_cq, (prod_index + nfreed) &
					  hr_cq->ib_cq.cqe);
			owner_bit = roce_get_bit(dest->byte_4,
						 V2_CQE_BYTE_4_OWNER_S);
			memcpy(dest, cqe, sizeof(*cqe));
			roce_set_bit(dest->byte_4, V2_CQE_BYTE_4_OWNER_S,
				     owner_bit);
		}
	}

	if (nfreed) {
		hr_cq->cons_index += nfreed;
		/*
		 * Make sure update of buffer contents is done before
		 * updating consumer index.
		 */
		wmb();

		hns_roce_v2_cq_set_ci(hr_cq, hr_cq->cons_index);
	}
}

static void hns_roce_v2_cq_clean(struct hns_roce_cq *hr_cq, u32 qpn,
				 struct hns_roce_srq *srq)
{
	spin_lock_irq(&hr_cq->lock);
	__hns_roce_v2_cq_clean(hr_cq, qpn, srq);
	spin_unlock_irq(&hr_cq->lock);
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

static int hns_roce_v2_qp_modify(struct hns_roce_dev *hr_dev,
				 struct hns_roce_mtt *mtt,
				 enum ib_qp_state cur_state,
				 enum ib_qp_state new_state,
				 struct hns_roce_v2_qp_context *context,
				 struct hns_roce_qp *hr_qp)
{
	struct hns_roce_cmd_mailbox *mailbox;
	int ret;

	mailbox = hns_roce_alloc_cmd_mailbox(hr_dev);
	if (IS_ERR(mailbox))
		return PTR_ERR(mailbox);

	memcpy(mailbox->buf, context, sizeof(*context) * 2);

	ret = hns_roce_cmd_mbox(hr_dev, mailbox->dma, 0, hr_qp->qpn, 0,
				HNS_ROCE_CMD_MODIFY_QPC,
				HNS_ROCE_CMD_TIMEOUT_MSECS);

	hns_roce_free_cmd_mailbox(hr_dev, mailbox);

	return ret;
}

static void modify_qp_reset_to_init(struct ib_qp *ibqp,
				    const struct ib_qp_attr *attr,
				    struct hns_roce_v2_qp_context *context,
				    struct hns_roce_v2_qp_context *qpc_mask)
{
	struct hns_roce_qp *hr_qp = to_hr_qp(ibqp);

	memset(qpc_mask, 0, sizeof(*qpc_mask));
	roce_set_field(context->byte_4, V2_QPC_BYTE_4_TST_M,
		       V2_QPC_BYTE_4_TST_S, to_hr_qp_type(hr_qp->ibqp.qp_type));
	roce_set_field(qpc_mask->byte_4, V2_QPC_BYTE_4_TST_M,
		       V2_QPC_BYTE_4_TST_S, 0);

	roce_set_field(context->byte_4, V2_QPC_BYTE_4_SGE_SHIFT_M,
		       V2_QPC_BYTE_4_SGE_SHIFT_S, hr_qp->sq.max_gs > 2 ?
		       ilog2((unsigned int)hr_qp->sge.sge_cnt) : 0);
	roce_set_field(qpc_mask->byte_4, V2_QPC_BYTE_4_SGE_SHIFT_M,
		       V2_QPC_BYTE_4_SGE_SHIFT_S, 0);

	roce_set_field(context->byte_4, V2_QPC_BYTE_4_SQPN_M,
		       V2_QPC_BYTE_4_SQPN_S, hr_qp->qpn);
	roce_set_field(qpc_mask->byte_4, V2_QPC_BYTE_4_SQPN_M,
		       V2_QPC_BYTE_4_SQPN_S, 0);

	roce_set_field(context->byte_20, V2_QPC_BYTE_20_RQWS_M,
		       V2_QPC_BYTE_20_RQWS_S, ilog2(hr_qp->rq.max_gs));
	roce_set_field(qpc_mask->byte_20, V2_QPC_BYTE_20_RQWS_M,
		       V2_QPC_BYTE_20_RQWS_S, 0);

	roce_set_field(context->byte_16, V2_QPC_BYTE_16_PD_M,
		       V2_QPC_BYTE_16_PD_S, to_hr_pd(ibqp->pd)->pdn);
	roce_set_field(qpc_mask->byte_16, V2_QPC_BYTE_16_PD_M,
		       V2_QPC_BYTE_16_PD_S, 0);

	roce_set_field(context->byte_20, V2_QPC_BYTE_20_SQ_SHIFT_M,
		       V2_QPC_BYTE_20_SQ_SHIFT_S,
		       ilog2((unsigned int)hr_qp->sq.wqe_cnt));
	roce_set_field(qpc_mask->byte_20, V2_QPC_BYTE_20_SQ_SHIFT_M,
		       V2_QPC_BYTE_20_SQ_SHIFT_S, 0);

	roce_set_field(context->byte_20, V2_QPC_BYTE_20_RQ_SHIFT_M,
		       V2_QPC_BYTE_20_RQ_SHIFT_S,
		       ilog2((unsigned int)hr_qp->rq.wqe_cnt));
	roce_set_field(qpc_mask->byte_20, V2_QPC_BYTE_20_RQ_SHIFT_M,
		       V2_QPC_BYTE_20_RQ_SHIFT_S, 0);

	/* Not VLAN need to set 0xFFF */
	roce_set_field(context->byte_24, V2_QPC_BYTE_24_VLAN_IDX_M,
		       V2_QPC_BYTE_24_VLAN_IDX_S, 0xfff);
	roce_set_field(qpc_mask->byte_24, V2_QPC_BYTE_24_VLAN_IDX_M,
		       V2_QPC_BYTE_24_VLAN_IDX_S, 0);

	roce_set_bit(context->byte_56, V2_QPC_BYTE_56_SQ_TX_ERR_S, 0);
	roce_set_bit(qpc_mask->byte_56, V2_QPC_BYTE_56_SQ_TX_ERR_S, 0);

	roce_set_bit(context->byte_56, V2_QPC_BYTE_56_SQ_RX_ERR_S, 0);
	roce_set_bit(qpc_mask->byte_56, V2_QPC_BYTE_56_SQ_RX_ERR_S, 0);

	roce_set_bit(context->byte_56, V2_QPC_BYTE_56_RQ_TX_ERR_S, 0);
	roce_set_bit(qpc_mask->byte_56, V2_QPC_BYTE_56_RQ_TX_ERR_S, 0);

	roce_set_bit(context->byte_56, V2_QPC_BYTE_56_RQ_RX_ERR_S, 0);
	roce_set_bit(qpc_mask->byte_56, V2_QPC_BYTE_56_RQ_RX_ERR_S, 0);

	roce_set_field(context->byte_60, V2_QPC_BYTE_60_MAPID_M,
		       V2_QPC_BYTE_60_MAPID_S, 0);
	roce_set_field(qpc_mask->byte_60, V2_QPC_BYTE_60_MAPID_M,
		       V2_QPC_BYTE_60_MAPID_S, 0);

	roce_set_bit(context->byte_60, V2_QPC_BYTE_60_INNER_MAP_IND_S, 0);
	roce_set_bit(qpc_mask->byte_60, V2_QPC_BYTE_60_INNER_MAP_IND_S, 0);

	roce_set_bit(context->byte_60, V2_QPC_BYTE_60_SQ_MAP_IND_S, 0);
	roce_set_bit(qpc_mask->byte_60, V2_QPC_BYTE_60_SQ_MAP_IND_S, 0);

	roce_set_bit(context->byte_60, V2_QPC_BYTE_60_RQ_MAP_IND_S, 0);
	roce_set_bit(qpc_mask->byte_60, V2_QPC_BYTE_60_RQ_MAP_IND_S, 0);

	roce_set_bit(context->byte_60, V2_QPC_BYTE_60_EXT_MAP_IND_S, 0);
	roce_set_bit(qpc_mask->byte_60, V2_QPC_BYTE_60_EXT_MAP_IND_S, 0);

	roce_set_bit(context->byte_60, V2_QPC_BYTE_60_SQ_RLS_IND_S, 0);
	roce_set_bit(qpc_mask->byte_60, V2_QPC_BYTE_60_SQ_RLS_IND_S, 0);

	roce_set_bit(context->byte_60, V2_QPC_BYTE_60_SQ_EXT_IND_S, 0);
	roce_set_bit(qpc_mask->byte_60, V2_QPC_BYTE_60_SQ_EXT_IND_S, 0);

	roce_set_bit(context->byte_28, V2_QPC_BYTE_28_CNP_TX_FLAG_S, 0);
	roce_set_bit(qpc_mask->byte_28, V2_QPC_BYTE_28_CNP_TX_FLAG_S, 0);

	roce_set_bit(context->byte_28, V2_QPC_BYTE_28_CE_FLAG_S, 0);
	roce_set_bit(qpc_mask->byte_28, V2_QPC_BYTE_28_CE_FLAG_S, 0);

	roce_set_bit(context->byte_76, V2_QPC_BYTE_76_RRE_S,
		     !!(attr->qp_access_flags & IB_ACCESS_REMOTE_READ));
	roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_RRE_S, 0);

	roce_set_bit(context->byte_76, V2_QPC_BYTE_76_RWE_S,
		     !!(attr->qp_access_flags & IB_ACCESS_REMOTE_WRITE));
	roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_RWE_S, 0);

	roce_set_bit(context->byte_76, V2_QPC_BYTE_76_ATE_S,
		     !!(attr->qp_access_flags & IB_ACCESS_REMOTE_ATOMIC));
	roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_ATE_S, 0);

	/* The RQIE is set for RQ inline while testing, 0 as default */
	roce_set_bit(context->byte_76, V2_QPC_BYTE_76_RQIE_S, 0);

	roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_RQIE_S, 0);

	roce_set_field(context->byte_80, V2_QPC_BYTE_80_RX_CQN_M,
		       V2_QPC_BYTE_80_RX_CQN_S, to_hr_cq(ibqp->recv_cq)->cqn);
	roce_set_field(qpc_mask->byte_80, V2_QPC_BYTE_80_RX_CQN_M,
		       V2_QPC_BYTE_80_RX_CQN_S, 0);
	if (ibqp->srq) {
		roce_set_field(context->byte_76, V2_QPC_BYTE_76_SRQN_M,
			       V2_QPC_BYTE_76_SRQN_S,
			       to_hr_srq(ibqp->srq)->srqn);
		roce_set_field(qpc_mask->byte_76, V2_QPC_BYTE_76_SRQN_M,
			       V2_QPC_BYTE_76_SRQN_S, 0);

		roce_set_bit(context->byte_76, V2_QPC_BYTE_76_SRQ_EN_S, 1);
		roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_SRQ_EN_S, 0);
	}

	roce_set_field(context->byte_84, V2_QPC_BYTE_84_RQ_PRODUCER_IDX_M,
		       V2_QPC_BYTE_84_RQ_PRODUCER_IDX_S, 0);
	roce_set_field(qpc_mask->byte_84, V2_QPC_BYTE_84_RQ_PRODUCER_IDX_M,
		       V2_QPC_BYTE_84_RQ_PRODUCER_IDX_S, 0);

	roce_set_field(context->byte_84, V2_QPC_BYTE_84_RQ_CONSUMER_IDX_M,
		       V2_QPC_BYTE_84_RQ_CONSUMER_IDX_S, 0);
	roce_set_field(qpc_mask->byte_84, V2_QPC_BYTE_84_RQ_CONSUMER_IDX_M,
		       V2_QPC_BYTE_84_RQ_CONSUMER_IDX_S, 0);

	roce_set_field(context->byte_92, V2_QPC_BYTE_92_SRQ_INFO_M,
		       V2_QPC_BYTE_92_SRQ_INFO_S, 0);
	roce_set_field(qpc_mask->byte_92, V2_QPC_BYTE_92_SRQ_INFO_M,
		       V2_QPC_BYTE_92_SRQ_INFO_S, 0);

	roce_set_field(context->byte_96, V2_QPC_BYTE_96_RX_REQ_MSN_M,
		       V2_QPC_BYTE_96_RX_REQ_MSN_S, 0);
	roce_set_field(qpc_mask->byte_96, V2_QPC_BYTE_96_RX_REQ_MSN_M,
		       V2_QPC_BYTE_96_RX_REQ_MSN_S, 0);

	roce_set_field(context->byte_104, V2_QPC_BYTE_104_RQ_CUR_WQE_SGE_NUM_M,
		       V2_QPC_BYTE_104_RQ_CUR_WQE_SGE_NUM_S, 0);
	roce_set_field(qpc_mask->byte_104, V2_QPC_BYTE_104_RQ_CUR_WQE_SGE_NUM_M,
		       V2_QPC_BYTE_104_RQ_CUR_WQE_SGE_NUM_S, 0);

	roce_set_bit(context->byte_108, V2_QPC_BYTE_108_RX_REQ_PSN_ERR_S, 0);
	roce_set_bit(qpc_mask->byte_108, V2_QPC_BYTE_108_RX_REQ_PSN_ERR_S, 0);

	roce_set_field(context->byte_108, V2_QPC_BYTE_108_RX_REQ_LAST_OPTYPE_M,
		       V2_QPC_BYTE_108_RX_REQ_LAST_OPTYPE_S, 0);
	roce_set_field(qpc_mask->byte_108, V2_QPC_BYTE_108_RX_REQ_LAST_OPTYPE_M,
		       V2_QPC_BYTE_108_RX_REQ_LAST_OPTYPE_S, 0);

	roce_set_bit(context->byte_108, V2_QPC_BYTE_108_RX_REQ_RNR_S, 0);
	roce_set_bit(qpc_mask->byte_108, V2_QPC_BYTE_108_RX_REQ_RNR_S, 0);

	context->rq_rnr_timer = 0;
	qpc_mask->rq_rnr_timer = 0;

	context->rx_msg_len = 0;
	qpc_mask->rx_msg_len = 0;

	context->rx_rkey_pkt_info = 0;
	qpc_mask->rx_rkey_pkt_info = 0;

	context->rx_va = 0;
	qpc_mask->rx_va = 0;

	roce_set_field(context->byte_132, V2_QPC_BYTE_132_TRRL_HEAD_MAX_M,
		       V2_QPC_BYTE_132_TRRL_HEAD_MAX_S, 0);
	roce_set_field(qpc_mask->byte_132, V2_QPC_BYTE_132_TRRL_HEAD_MAX_M,
		       V2_QPC_BYTE_132_TRRL_HEAD_MAX_S, 0);

	roce_set_field(context->byte_132, V2_QPC_BYTE_132_TRRL_TAIL_MAX_M,
		       V2_QPC_BYTE_132_TRRL_TAIL_MAX_S, 0);
	roce_set_field(qpc_mask->byte_132, V2_QPC_BYTE_132_TRRL_TAIL_MAX_M,
		       V2_QPC_BYTE_132_TRRL_TAIL_MAX_S, 0);

	roce_set_bit(context->byte_140, V2_QPC_BYTE_140_RSVD_RAQ_MAP_S, 0);
	roce_set_bit(qpc_mask->byte_140, V2_QPC_BYTE_140_RSVD_RAQ_MAP_S, 0);

	roce_set_field(context->byte_140, V2_QPC_BYTE_140_RAQ_TRRL_HEAD_M,
		       V2_QPC_BYTE_140_RAQ_TRRL_HEAD_S, 0);
	roce_set_field(qpc_mask->byte_140, V2_QPC_BYTE_140_RAQ_TRRL_HEAD_M,
		       V2_QPC_BYTE_140_RAQ_TRRL_HEAD_S, 0);

	roce_set_field(context->byte_140, V2_QPC_BYTE_140_RAQ_TRRL_TAIL_M,
		       V2_QPC_BYTE_140_RAQ_TRRL_TAIL_S, 0);
	roce_set_field(qpc_mask->byte_140, V2_QPC_BYTE_140_RAQ_TRRL_TAIL_M,
		       V2_QPC_BYTE_140_RAQ_TRRL_TAIL_S, 0);

	roce_set_field(context->byte_144, V2_QPC_BYTE_144_RAQ_RTY_INI_PSN_M,
		       V2_QPC_BYTE_144_RAQ_RTY_INI_PSN_S, 0);
	roce_set_field(qpc_mask->byte_144, V2_QPC_BYTE_144_RAQ_RTY_INI_PSN_M,
		       V2_QPC_BYTE_144_RAQ_RTY_INI_PSN_S, 0);

	roce_set_bit(context->byte_144, V2_QPC_BYTE_144_RAQ_RTY_INI_IND_S, 0);
	roce_set_bit(qpc_mask->byte_144, V2_QPC_BYTE_144_RAQ_RTY_INI_IND_S, 0);

	roce_set_field(context->byte_144, V2_QPC_BYTE_144_RAQ_CREDIT_M,
		       V2_QPC_BYTE_144_RAQ_CREDIT_S, 0);
	roce_set_field(qpc_mask->byte_144, V2_QPC_BYTE_144_RAQ_CREDIT_M,
		       V2_QPC_BYTE_144_RAQ_CREDIT_S, 0);

	roce_set_bit(context->byte_144, V2_QPC_BYTE_144_RESP_RTY_FLG_S, 0);
	roce_set_bit(qpc_mask->byte_144, V2_QPC_BYTE_144_RESP_RTY_FLG_S, 0);

	roce_set_field(context->byte_148, V2_QPC_BYTE_148_RQ_MSN_M,
		       V2_QPC_BYTE_148_RQ_MSN_S, 0);
	roce_set_field(qpc_mask->byte_148, V2_QPC_BYTE_148_RQ_MSN_M,
		       V2_QPC_BYTE_148_RQ_MSN_S, 0);

	roce_set_field(context->byte_148, V2_QPC_BYTE_148_RAQ_SYNDROME_M,
		       V2_QPC_BYTE_148_RAQ_SYNDROME_S, 0);
	roce_set_field(qpc_mask->byte_148, V2_QPC_BYTE_148_RAQ_SYNDROME_M,
		       V2_QPC_BYTE_148_RAQ_SYNDROME_S, 0);

	roce_set_field(context->byte_152, V2_QPC_BYTE_152_RAQ_PSN_M,
		       V2_QPC_BYTE_152_RAQ_PSN_S, 0);
	roce_set_field(qpc_mask->byte_152, V2_QPC_BYTE_152_RAQ_PSN_M,
		       V2_QPC_BYTE_152_RAQ_PSN_S, 0);

	roce_set_field(context->byte_152, V2_QPC_BYTE_152_RAQ_TRRL_RTY_HEAD_M,
		       V2_QPC_BYTE_152_RAQ_TRRL_RTY_HEAD_S, 0);
	roce_set_field(qpc_mask->byte_152, V2_QPC_BYTE_152_RAQ_TRRL_RTY_HEAD_M,
		       V2_QPC_BYTE_152_RAQ_TRRL_RTY_HEAD_S, 0);

	roce_set_field(context->byte_156, V2_QPC_BYTE_156_RAQ_USE_PKTN_M,
		       V2_QPC_BYTE_156_RAQ_USE_PKTN_S, 0);
	roce_set_field(qpc_mask->byte_156, V2_QPC_BYTE_156_RAQ_USE_PKTN_M,
		       V2_QPC_BYTE_156_RAQ_USE_PKTN_S, 0);

	roce_set_field(context->byte_160, V2_QPC_BYTE_160_SQ_PRODUCER_IDX_M,
		       V2_QPC_BYTE_160_SQ_PRODUCER_IDX_S, 0);
	roce_set_field(qpc_mask->byte_160, V2_QPC_BYTE_160_SQ_PRODUCER_IDX_M,
		       V2_QPC_BYTE_160_SQ_PRODUCER_IDX_S, 0);

	roce_set_field(context->byte_160, V2_QPC_BYTE_160_SQ_CONSUMER_IDX_M,
		       V2_QPC_BYTE_160_SQ_CONSUMER_IDX_S, 0);
	roce_set_field(qpc_mask->byte_160, V2_QPC_BYTE_160_SQ_CONSUMER_IDX_M,
		       V2_QPC_BYTE_160_SQ_CONSUMER_IDX_S, 0);

	roce_set_bit(context->byte_168, V2_QPC_BYTE_168_MSG_RTY_LP_FLG_S, 0);
	roce_set_bit(qpc_mask->byte_168, V2_QPC_BYTE_168_MSG_RTY_LP_FLG_S, 0);

	roce_set_field(context->byte_168, V2_QPC_BYTE_168_SQ_SHIFT_BAK_M,
		       V2_QPC_BYTE_168_SQ_SHIFT_BAK_S,
		       ilog2((unsigned int)hr_qp->sq.wqe_cnt));
	roce_set_field(qpc_mask->byte_168, V2_QPC_BYTE_168_SQ_SHIFT_BAK_M,
		       V2_QPC_BYTE_168_SQ_SHIFT_BAK_S, 0);

	roce_set_field(context->byte_168, V2_QPC_BYTE_168_IRRL_IDX_LSB_M,
		       V2_QPC_BYTE_168_IRRL_IDX_LSB_S, 0);
	roce_set_field(qpc_mask->byte_168, V2_QPC_BYTE_168_IRRL_IDX_LSB_M,
		       V2_QPC_BYTE_168_IRRL_IDX_LSB_S, 0);
	roce_set_field(context->byte_172, V2_QPC_BYTE_172_ACK_REQ_FREQ_M,
		       V2_QPC_BYTE_172_ACK_REQ_FREQ_S, 4);
	roce_set_field(qpc_mask->byte_172, V2_QPC_BYTE_172_ACK_REQ_FREQ_M,
		       V2_QPC_BYTE_172_ACK_REQ_FREQ_S, 0);

	roce_set_bit(context->byte_172, V2_QPC_BYTE_172_MSG_RNR_FLG_S, 0);
	roce_set_bit(qpc_mask->byte_172, V2_QPC_BYTE_172_MSG_RNR_FLG_S, 0);

	roce_set_field(context->byte_176, V2_QPC_BYTE_176_MSG_USE_PKTN_M,
		       V2_QPC_BYTE_176_MSG_USE_PKTN_S, 0);
	roce_set_field(qpc_mask->byte_176, V2_QPC_BYTE_176_MSG_USE_PKTN_M,
		       V2_QPC_BYTE_176_MSG_USE_PKTN_S, 0);

	roce_set_field(context->byte_176, V2_QPC_BYTE_176_IRRL_HEAD_PRE_M,
		       V2_QPC_BYTE_176_IRRL_HEAD_PRE_S, 0);
	roce_set_field(qpc_mask->byte_176, V2_QPC_BYTE_176_IRRL_HEAD_PRE_M,
		       V2_QPC_BYTE_176_IRRL_HEAD_PRE_S, 0);

	roce_set_field(context->byte_184, V2_QPC_BYTE_184_IRRL_IDX_MSB_M,
		       V2_QPC_BYTE_184_IRRL_IDX_MSB_S, 0);
	roce_set_field(qpc_mask->byte_184, V2_QPC_BYTE_184_IRRL_IDX_MSB_M,
		       V2_QPC_BYTE_184_IRRL_IDX_MSB_S, 0);

	context->cur_sge_offset = 0;
	qpc_mask->cur_sge_offset = 0;

	roce_set_field(context->byte_192, V2_QPC_BYTE_192_CUR_SGE_IDX_M,
		       V2_QPC_BYTE_192_CUR_SGE_IDX_S, 0);
	roce_set_field(qpc_mask->byte_192, V2_QPC_BYTE_192_CUR_SGE_IDX_M,
		       V2_QPC_BYTE_192_CUR_SGE_IDX_S, 0);

	roce_set_field(context->byte_192, V2_QPC_BYTE_192_EXT_SGE_NUM_LEFT_M,
		       V2_QPC_BYTE_192_EXT_SGE_NUM_LEFT_S, 0);
	roce_set_field(qpc_mask->byte_192, V2_QPC_BYTE_192_EXT_SGE_NUM_LEFT_M,
		       V2_QPC_BYTE_192_EXT_SGE_NUM_LEFT_S, 0);

	roce_set_field(context->byte_196, V2_QPC_BYTE_196_IRRL_HEAD_M,
		       V2_QPC_BYTE_196_IRRL_HEAD_S, 0);
	roce_set_field(qpc_mask->byte_196, V2_QPC_BYTE_196_IRRL_HEAD_M,
		       V2_QPC_BYTE_196_IRRL_HEAD_S, 0);

	roce_set_field(context->byte_200, V2_QPC_BYTE_200_SQ_MAX_IDX_M,
		       V2_QPC_BYTE_200_SQ_MAX_IDX_S, 0);
	roce_set_field(qpc_mask->byte_200, V2_QPC_BYTE_200_SQ_MAX_IDX_M,
		       V2_QPC_BYTE_200_SQ_MAX_IDX_S, 0);

	roce_set_field(context->byte_200, V2_QPC_BYTE_200_LCL_OPERATED_CNT_M,
		       V2_QPC_BYTE_200_LCL_OPERATED_CNT_S, 0);
	roce_set_field(qpc_mask->byte_200, V2_QPC_BYTE_200_LCL_OPERATED_CNT_M,
		       V2_QPC_BYTE_200_LCL_OPERATED_CNT_S, 0);

	roce_set_bit(context->byte_208, V2_QPC_BYTE_208_PKT_RNR_FLG_S, 0);
	roce_set_bit(qpc_mask->byte_208, V2_QPC_BYTE_208_PKT_RNR_FLG_S, 0);

	roce_set_bit(context->byte_208, V2_QPC_BYTE_208_PKT_RTY_FLG_S, 0);
	roce_set_bit(qpc_mask->byte_208, V2_QPC_BYTE_208_PKT_RTY_FLG_S, 0);

	roce_set_field(context->byte_212, V2_QPC_BYTE_212_CHECK_FLG_M,
		       V2_QPC_BYTE_212_CHECK_FLG_S, 0);
	roce_set_field(qpc_mask->byte_212, V2_QPC_BYTE_212_CHECK_FLG_M,
		       V2_QPC_BYTE_212_CHECK_FLG_S, 0);

	context->sq_timer = 0;
	qpc_mask->sq_timer = 0;

	roce_set_field(context->byte_220, V2_QPC_BYTE_220_RETRY_MSG_MSN_M,
		       V2_QPC_BYTE_220_RETRY_MSG_MSN_S, 0);
	roce_set_field(qpc_mask->byte_220, V2_QPC_BYTE_220_RETRY_MSG_MSN_M,
		       V2_QPC_BYTE_220_RETRY_MSG_MSN_S, 0);

	roce_set_field(context->byte_232, V2_QPC_BYTE_232_IRRL_SGE_IDX_M,
		       V2_QPC_BYTE_232_IRRL_SGE_IDX_S, 0);
	roce_set_field(qpc_mask->byte_232, V2_QPC_BYTE_232_IRRL_SGE_IDX_M,
		       V2_QPC_BYTE_232_IRRL_SGE_IDX_S, 0);

	context->irrl_cur_sge_offset = 0;
	qpc_mask->irrl_cur_sge_offset = 0;

	roce_set_field(context->byte_240, V2_QPC_BYTE_240_IRRL_TAIL_REAL_M,
		       V2_QPC_BYTE_240_IRRL_TAIL_REAL_S, 0);
	roce_set_field(qpc_mask->byte_240, V2_QPC_BYTE_240_IRRL_TAIL_REAL_M,
		       V2_QPC_BYTE_240_IRRL_TAIL_REAL_S, 0);

	roce_set_field(context->byte_240, V2_QPC_BYTE_240_IRRL_TAIL_RD_M,
		       V2_QPC_BYTE_240_IRRL_TAIL_RD_S, 0);
	roce_set_field(qpc_mask->byte_240, V2_QPC_BYTE_240_IRRL_TAIL_RD_M,
		       V2_QPC_BYTE_240_IRRL_TAIL_RD_S, 0);

	roce_set_field(context->byte_240, V2_QPC_BYTE_240_RX_ACK_MSN_M,
		       V2_QPC_BYTE_240_RX_ACK_MSN_S, 0);
	roce_set_field(qpc_mask->byte_240, V2_QPC_BYTE_240_RX_ACK_MSN_M,
		       V2_QPC_BYTE_240_RX_ACK_MSN_S, 0);

	roce_set_field(context->byte_248, V2_QPC_BYTE_248_IRRL_PSN_M,
		       V2_QPC_BYTE_248_IRRL_PSN_S, 0);
	roce_set_field(qpc_mask->byte_248, V2_QPC_BYTE_248_IRRL_PSN_M,
		       V2_QPC_BYTE_248_IRRL_PSN_S, 0);

	roce_set_bit(context->byte_248, V2_QPC_BYTE_248_ACK_PSN_ERR_S, 0);
	roce_set_bit(qpc_mask->byte_248, V2_QPC_BYTE_248_ACK_PSN_ERR_S, 0);

	roce_set_field(context->byte_248, V2_QPC_BYTE_248_ACK_LAST_OPTYPE_M,
		       V2_QPC_BYTE_248_ACK_LAST_OPTYPE_S, 0);
	roce_set_field(qpc_mask->byte_248, V2_QPC_BYTE_248_ACK_LAST_OPTYPE_M,
		       V2_QPC_BYTE_248_ACK_LAST_OPTYPE_S, 0);

	roce_set_bit(context->byte_248, V2_QPC_BYTE_248_IRRL_PSN_VLD_S, 0);
	roce_set_bit(qpc_mask->byte_248, V2_QPC_BYTE_248_IRRL_PSN_VLD_S, 0);

	roce_set_bit(context->byte_248, V2_QPC_BYTE_248_RNR_RETRY_FLAG_S, 0);
	roce_set_bit(qpc_mask->byte_248, V2_QPC_BYTE_248_RNR_RETRY_FLAG_S, 0);

	roce_set_bit(context->byte_248, V2_QPC_BYTE_248_CQ_ERR_IND_S, 0);
	roce_set_bit(qpc_mask->byte_248, V2_QPC_BYTE_248_CQ_ERR_IND_S, 0);

	hr_qp->access_flags = attr->qp_access_flags;
	hr_qp->pkey_index = attr->pkey_index;
	roce_set_field(context->byte_252, V2_QPC_BYTE_252_TX_CQN_M,
		       V2_QPC_BYTE_252_TX_CQN_S, to_hr_cq(ibqp->send_cq)->cqn);
	roce_set_field(qpc_mask->byte_252, V2_QPC_BYTE_252_TX_CQN_M,
		       V2_QPC_BYTE_252_TX_CQN_S, 0);

	roce_set_field(context->byte_252, V2_QPC_BYTE_252_ERR_TYPE_M,
		       V2_QPC_BYTE_252_ERR_TYPE_S, 0);
	roce_set_field(qpc_mask->byte_252, V2_QPC_BYTE_252_ERR_TYPE_M,
		       V2_QPC_BYTE_252_ERR_TYPE_S, 0);

	roce_set_field(context->byte_256, V2_QPC_BYTE_256_RQ_CQE_IDX_M,
		       V2_QPC_BYTE_256_RQ_CQE_IDX_S, 0);
	roce_set_field(qpc_mask->byte_256, V2_QPC_BYTE_256_RQ_CQE_IDX_M,
		       V2_QPC_BYTE_256_RQ_CQE_IDX_S, 0);

	roce_set_field(context->byte_256, V2_QPC_BYTE_256_SQ_FLUSH_IDX_M,
		       V2_QPC_BYTE_256_SQ_FLUSH_IDX_S, 0);
	roce_set_field(qpc_mask->byte_256, V2_QPC_BYTE_256_SQ_FLUSH_IDX_M,
		       V2_QPC_BYTE_256_SQ_FLUSH_IDX_S, 0);
}

static void modify_qp_init_to_init(struct ib_qp *ibqp,
				   const struct ib_qp_attr *attr, int attr_mask,
				   struct hns_roce_v2_qp_context *context,
				   struct hns_roce_v2_qp_context *qpc_mask)
{
	struct hns_roce_qp *hr_qp = to_hr_qp(ibqp);

	roce_set_field(context->byte_4, V2_QPC_BYTE_4_TST_M,
		       V2_QPC_BYTE_4_TST_S, to_hr_qp_type(hr_qp->ibqp.qp_type));
	roce_set_field(qpc_mask->byte_4, V2_QPC_BYTE_4_TST_M,
		       V2_QPC_BYTE_4_TST_S, 0);

	roce_set_field(context->byte_4, V2_QPC_BYTE_4_SGE_SHIFT_M,
		       V2_QPC_BYTE_4_SGE_SHIFT_S, hr_qp->sq.max_gs > 2 ?
		       ilog2((unsigned int)hr_qp->sge.sge_cnt) : 0);
	roce_set_field(qpc_mask->byte_4, V2_QPC_BYTE_4_SGE_SHIFT_M,
		       V2_QPC_BYTE_4_SGE_SHIFT_S, 0);

	if (attr_mask & IB_QP_ACCESS_FLAGS) {
		roce_set_bit(context->byte_76, V2_QPC_BYTE_76_RRE_S,
			     !!(attr->qp_access_flags & IB_ACCESS_REMOTE_READ));
		roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_RRE_S, 0);
		roce_set_bit(context->byte_76, V2_QPC_BYTE_76_RWE_S,
			     !!(attr->qp_access_flags &
			     IB_ACCESS_REMOTE_WRITE));
		roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_RWE_S, 0);

		roce_set_bit(context->byte_76, V2_QPC_BYTE_76_ATE_S,
			     !!(attr->qp_access_flags &
			     IB_ACCESS_REMOTE_ATOMIC));
		roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_ATE_S, 0);
	} else {
		roce_set_bit(context->byte_76, V2_QPC_BYTE_76_RRE_S,
			     !!(hr_qp->access_flags & IB_ACCESS_REMOTE_READ));
		roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_RRE_S, 0);

		roce_set_bit(context->byte_76, V2_QPC_BYTE_76_RWE_S,
			     !!(hr_qp->access_flags & IB_ACCESS_REMOTE_WRITE));
		roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_RWE_S, 0);

		roce_set_bit(context->byte_76, V2_QPC_BYTE_76_ATE_S,
			     !!(hr_qp->access_flags & IB_ACCESS_REMOTE_ATOMIC));
		roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_ATE_S, 0);
	}

	roce_set_field(context->byte_20, V2_QPC_BYTE_20_SQ_SHIFT_M,
		       V2_QPC_BYTE_20_SQ_SHIFT_S,
		       ilog2((unsigned int)hr_qp->sq.wqe_cnt));
	roce_set_field(qpc_mask->byte_20, V2_QPC_BYTE_20_SQ_SHIFT_M,
		       V2_QPC_BYTE_20_SQ_SHIFT_S, 0);

	roce_set_field(context->byte_20, V2_QPC_BYTE_20_RQ_SHIFT_M,
		       V2_QPC_BYTE_20_RQ_SHIFT_S,
		       ilog2((unsigned int)hr_qp->rq.wqe_cnt));
	roce_set_field(qpc_mask->byte_20, V2_QPC_BYTE_20_RQ_SHIFT_M,
		       V2_QPC_BYTE_20_RQ_SHIFT_S, 0);

	roce_set_field(context->byte_16, V2_QPC_BYTE_16_PD_M,
		       V2_QPC_BYTE_16_PD_S, to_hr_pd(ibqp->pd)->pdn);
	roce_set_field(qpc_mask->byte_16, V2_QPC_BYTE_16_PD_M,
		       V2_QPC_BYTE_16_PD_S, 0);

	roce_set_field(context->byte_80, V2_QPC_BYTE_80_RX_CQN_M,
		       V2_QPC_BYTE_80_RX_CQN_S, to_hr_cq(ibqp->recv_cq)->cqn);
	roce_set_field(qpc_mask->byte_80, V2_QPC_BYTE_80_RX_CQN_M,
		       V2_QPC_BYTE_80_RX_CQN_S, 0);

	roce_set_field(context->byte_252, V2_QPC_BYTE_252_TX_CQN_M,
		       V2_QPC_BYTE_252_TX_CQN_S, to_hr_cq(ibqp->recv_cq)->cqn);
	roce_set_field(qpc_mask->byte_252, V2_QPC_BYTE_252_TX_CQN_M,
		       V2_QPC_BYTE_252_TX_CQN_S, 0);

	if (ibqp->srq) {
		roce_set_bit(context->byte_76, V2_QPC_BYTE_76_SRQ_EN_S, 1);
		roce_set_bit(qpc_mask->byte_76, V2_QPC_BYTE_76_SRQ_EN_S, 0);
		roce_set_field(context->byte_76, V2_QPC_BYTE_76_SRQN_M,
			       V2_QPC_BYTE_76_SRQN_S,
			       to_hr_srq(ibqp->srq)->srqn);
		roce_set_field(qpc_mask->byte_76, V2_QPC_BYTE_76_SRQN_M,
			       V2_QPC_BYTE_76_SRQN_S, 0);
	}

	if (attr_mask & IB_QP_PKEY_INDEX)
		context->qkey_xrcd = attr->pkey_index;
	else
		context->qkey_xrcd = hr_qp->pkey_index;

	roce_set_field(context->byte_4, V2_QPC_BYTE_4_SQPN_M,
		       V2_QPC_BYTE_4_SQPN_S, hr_qp->qpn);
	roce_set_field(qpc_mask->byte_4, V2_QPC_BYTE_4_SQPN_M,
		       V2_QPC_BYTE_4_SQPN_S, 0);

	roce_set_field(context->byte_56, V2_QPC_BYTE_56_DQPN_M,
		       V2_QPC_BYTE_56_DQPN_S, hr_qp->qpn);
	roce_set_field(qpc_mask->byte_56, V2_QPC_BYTE_56_DQPN_M,
		       V2_QPC_BYTE_56_DQPN_S, 0);
	roce_set_field(context->byte_168, V2_QPC_BYTE_168_SQ_SHIFT_BAK_M,
		       V2_QPC_BYTE_168_SQ_SHIFT_BAK_S,
		       ilog2((unsigned int)hr_qp->sq.wqe_cnt));
	roce_set_field(qpc_mask->byte_168, V2_QPC_BYTE_168_SQ_SHIFT_BAK_M,
		       V2_QPC_BYTE_168_SQ_SHIFT_BAK_S, 0);
}

int modify_qp_init_to_rtr(struct ib_qp *ibqp, const struct ib_qp_attr *attr,
			  int attr_mask, struct hns_roce_v2_qp_context *context,
			  struct hns_roce_v2_qp_context *qpc_mask)
{
	const struct ib_global_route *grh = rdma_ah_read_grh(&attr->ah_attr);
	struct hns_roce_dev *hr_dev = to_hr_dev(ibqp->device);
	struct hns_roce_qp *hr_qp = to_hr_qp(ibqp);
	struct device *dev = hr_dev->dev;
	dma_addr_t dma_handle_2;
	dma_addr_t dma_handle;
	u32 page_size;
	u8 port_num;
	u64 *mtts_2;
	u64 *mtts;
	u8 *dmac;
	u8 *smac;
	int port;

	/* Search qp buf's mtts */
	mtts = hns_roce_table_find(hr_dev, &hr_dev->mr_table.mtt_table,
				   hr_qp->mtt.first_seg, &dma_handle);
	if (!mtts) {
		dev_err(dev, "qp buf pa find failed\n");
		return -EINVAL;
	}

	/* Search IRRL's mtts */
	mtts_2 = hns_roce_table_find(hr_dev, &hr_dev->qp_table.irrl_table,
				     hr_qp->qpn, &dma_handle_2);
	if (!mtts_2) {
		dev_err(dev, "qp irrl_table find failed\n");
		return -EINVAL;
	}

	if ((attr_mask & IB_QP_ALT_PATH) || (attr_mask & IB_QP_ACCESS_FLAGS) ||
	    (attr_mask & IB_QP_PKEY_INDEX) || (attr_mask & IB_QP_QKEY)) {
		dev_err(dev, "INIT2RTR attr_mask (0x%x) error\n", attr_mask);
		return -EINVAL;
	}

	dmac = (u8 *)attr->ah_attr.roce.dmac;
	context->wqe_sge_ba = (u32)(dma_handle >> 3);
	qpc_mask->wqe_sge_ba = 0;

	roce_set_field(context->byte_12, V2_QPC_BYTE_12_WQE_SGE_BA_M,
		       V2_QPC_BYTE_12_WQE_SGE_BA_S, dma_handle >> (32 + 3));
	roce_set_field(qpc_mask->byte_12, V2_QPC_BYTE_12_WQE_SGE_BA_M,
		       V2_QPC_BYTE_12_WQE_SGE_BA_S, 0);

	roce_set_field(context->byte_12, V2_QPC_BYTE_12_SQ_HOP_NUM_M,
		       V2_QPC_BYTE_12_SQ_HOP_NUM_S,
		       hr_dev->caps.mtt_hop_num == HNS_ROCE_HOP_NUM_0 ?
		       0 : hr_dev->caps.mtt_hop_num);
	roce_set_field(qpc_mask->byte_12, V2_QPC_BYTE_12_SQ_HOP_NUM_M,
		       V2_QPC_BYTE_12_SQ_HOP_NUM_S, 0);

	roce_set_field(context->byte_20, V2_QPC_BYTE_20_SGE_HOP_NUM_M,
		       V2_QPC_BYTE_20_SGE_HOP_NUM_S, hr_qp->sq.max_gs > 2 ?
		       hr_dev->caps.mtt_hop_num : 0);
	roce_set_field(qpc_mask->byte_20, V2_QPC_BYTE_20_SGE_HOP_NUM_M,
		       V2_QPC_BYTE_20_SGE_HOP_NUM_S, 0);

	roce_set_field(context->byte_20, V2_QPC_BYTE_20_RQ_HOP_NUM_M,
		       V2_QPC_BYTE_20_RQ_HOP_NUM_S,
		       hr_dev->caps.mtt_hop_num == HNS_ROCE_HOP_NUM_0 ?
		       0 : hr_dev->caps.mtt_hop_num);
	roce_set_field(qpc_mask->byte_20, V2_QPC_BYTE_20_RQ_HOP_NUM_M,
		       V2_QPC_BYTE_20_RQ_HOP_NUM_S, 0);

	roce_set_field(context->byte_16, V2_QPC_BYTE_16_WQE_SGE_BA_PG_SZ_M,
		       V2_QPC_BYTE_16_WQE_SGE_BA_PG_SZ_S,
		       hr_dev->caps.mtt_ba_pg_sz);
	roce_set_field(qpc_mask->byte_16, V2_QPC_BYTE_16_WQE_SGE_BA_PG_SZ_M,
		       V2_QPC_BYTE_16_WQE_SGE_BA_PG_SZ_S, 0);

	roce_set_field(context->byte_16, V2_QPC_BYTE_16_WQE_SGE_BUF_PG_SZ_M,
		       V2_QPC_BYTE_16_WQE_SGE_BUF_PG_SZ_S,
		       hr_dev->caps.mtt_buf_pg_sz);
	roce_set_field(qpc_mask->byte_16, V2_QPC_BYTE_16_WQE_SGE_BUF_PG_SZ_M,
		       V2_QPC_BYTE_16_WQE_SGE_BUF_PG_SZ_S, 0);

	roce_set_field(context->byte_80, V2_QPC_BYTE_80_MIN_RNR_TIME_M,
		       V2_QPC_BYTE_80_MIN_RNR_TIME_S, attr->min_rnr_timer);
	roce_set_field(qpc_mask->byte_80, V2_QPC_BYTE_80_MIN_RNR_TIME_M,
		       V2_QPC_BYTE_80_MIN_RNR_TIME_S, 0);

	page_size = 1 << (hr_dev->caps.mtt_buf_pg_sz + PAGE_SHIFT);
	context->rq_cur_blk_addr = (u32)(mtts[hr_qp->rq.offset / page_size]
				    >> PAGE_ADDR_SHIFT);
	qpc_mask->rq_cur_blk_addr = 0;

	roce_set_field(context->byte_92, V2_QPC_BYTE_92_RQ_CUR_BLK_ADDR_M,
		       V2_QPC_BYTE_92_RQ_CUR_BLK_ADDR_S,
		       mtts[hr_qp->rq.offset / page_size]
		       >> (32 + PAGE_ADDR_SHIFT));
	roce_set_field(qpc_mask->byte_92, V2_QPC_BYTE_92_RQ_CUR_BLK_ADDR_M,
		       V2_QPC_BYTE_92_RQ_CUR_BLK_ADDR_S, 0);

	context->rq_nxt_blk_addr = (u32)(mtts[hr_qp->rq.offset / page_size + 1]
				    >> PAGE_ADDR_SHIFT);
	qpc_mask->rq_nxt_blk_addr = 0;

	roce_set_field(context->byte_104, V2_QPC_BYTE_104_RQ_NXT_BLK_ADDR_M,
		       V2_QPC_BYTE_104_RQ_NXT_BLK_ADDR_S,
		       mtts[hr_qp->rq.offset / page_size + 1]
		       >> (32 + PAGE_ADDR_SHIFT));
	roce_set_field(qpc_mask->byte_104, V2_QPC_BYTE_104_RQ_NXT_BLK_ADDR_M,
		       V2_QPC_BYTE_104_RQ_NXT_BLK_ADDR_S, 0);

	roce_set_field(context->byte_108, V2_QPC_BYTE_108_RX_REQ_EPSN_M,
		       V2_QPC_BYTE_108_RX_REQ_EPSN_S, attr->rq_psn);
	roce_set_field(qpc_mask->byte_108, V2_QPC_BYTE_108_RX_REQ_EPSN_M,
		       V2_QPC_BYTE_108_RX_REQ_EPSN_S, 0);

	context->irrl_ba = (u32)dma_handle_2;
	qpc_mask->irrl_ba = 0;
	roce_set_field(context->byte_208, V2_QPC_BYTE_208_IRRL_BA_M,
		       V2_QPC_BYTE_208_IRRL_BA_S,
		      (dma_handle_2 >> 32) & V2_QPC_BYTE_208_IRRL_BA_M);
	roce_set_field(qpc_mask->byte_208, V2_QPC_BYTE_208_IRRL_BA_M,
		       V2_QPC_BYTE_208_IRRL_BA_S, 0);

	roce_set_bit(context->byte_208, V2_QPC_BYTE_208_RMT_E2E_S, 1);
	roce_set_bit(qpc_mask->byte_208, V2_QPC_BYTE_208_RMT_E2E_S, 0);

	roce_set_bit(context->byte_252, V2_QPC_BYTE_252_SIG_TYPE_S,
		     hr_qp->sq_signal_bits);
	roce_set_bit(qpc_mask->byte_252, V2_QPC_BYTE_252_SIG_TYPE_S, 0);

	port = (attr_mask & IB_QP_PORT) ? (attr->port_num - 1) : hr_qp->port;

	smac = (u8 *)hr_dev->dev_addr[port];
	/* when dmac equals smac or loop_idc is 1, it should loopback */
	if (ether_addr_equal_unaligned(dmac, smac) ||
	    hr_dev->loop_idc == 0x1) {
		roce_set_bit(context->byte_28, V2_QPC_BYTE_28_LBI_S, 1);
		roce_set_bit(qpc_mask->byte_28, V2_QPC_BYTE_28_LBI_S, 0);
	}

	roce_set_field(context->byte_140, V2_QPC_BYTE_140_RR_MAX_M,
		       V2_QPC_BYTE_140_RR_MAX_S,
		       ilog2((unsigned int)attr->max_dest_rd_atomic));
	roce_set_field(qpc_mask->byte_140, V2_QPC_BYTE_140_RR_MAX_M,
		       V2_QPC_BYTE_140_RR_MAX_S, 0);

	roce_set_field(context->byte_56, V2_QPC_BYTE_56_DQPN_M,
		       V2_QPC_BYTE_56_DQPN_S, attr->dest_qp_num);
	roce_set_field(qpc_mask->byte_56, V2_QPC_BYTE_56_DQPN_M,
		       V2_QPC_BYTE_56_DQPN_S, 0);

	/* Configure GID index */
	port_num = rdma_ah_get_port_num(&attr->ah_attr);
	roce_set_field(context->byte_20, V2_QPC_BYTE_20_SGID_IDX_M,
		       V2_QPC_BYTE_20_SGID_IDX_S,
		       hns_get_gid_index(hr_dev, port_num - 1,
					 grh->sgid_index));
	roce_set_field(qpc_mask->byte_20, V2_QPC_BYTE_20_SGID_IDX_M,
		       V2_QPC_BYTE_20_SGID_IDX_S, 0);
	memcpy(&(context->dmac), dmac, 4);
	roce_set_field(context->byte_52, V2_QPC_BYTE_52_DMAC_M,
		       V2_QPC_BYTE_52_DMAC_S, *((u16 *)(&dmac[4])));
	qpc_mask->dmac = 0;
	roce_set_field(qpc_mask->byte_52, V2_QPC_BYTE_52_DMAC_M,
		       V2_QPC_BYTE_52_DMAC_S, 0);

	roce_set_field(context->byte_56, V2_QPC_BYTE_56_LP_PKTN_INI_M,
		       V2_QPC_BYTE_56_LP_PKTN_INI_S, 4);
	roce_set_field(qpc_mask->byte_56, V2_QPC_BYTE_56_LP_PKTN_INI_M,
		       V2_QPC_BYTE_56_LP_PKTN_INI_S, 0);

	roce_set_field(context->byte_24, V2_QPC_BYTE_24_HOP_LIMIT_M,
		       V2_QPC_BYTE_24_HOP_LIMIT_S, grh->hop_limit);
	roce_set_field(qpc_mask->byte_24, V2_QPC_BYTE_24_HOP_LIMIT_M,
		       V2_QPC_BYTE_24_HOP_LIMIT_S, 0);

	roce_set_field(context->byte_28, V2_QPC_BYTE_28_FL_M,
		       V2_QPC_BYTE_28_FL_S, grh->flow_label);
	roce_set_field(qpc_mask->byte_28, V2_QPC_BYTE_28_FL_M,
		       V2_QPC_BYTE_28_FL_S, 0);

	roce_set_field(context->byte_24, V2_QPC_BYTE_24_TC_M,
		       V2_QPC_BYTE_24_TC_S, grh->traffic_class);
	roce_set_field(qpc_mask->byte_24, V2_QPC_BYTE_24_TC_M,
		       V2_QPC_BYTE_24_TC_S, 0);

	roce_set_field(context->byte_24, V2_QPC_BYTE_24_MTU_M,
		       V2_QPC_BYTE_24_MTU_S, attr->path_mtu);
	roce_set_field(qpc_mask->byte_24, V2_QPC_BYTE_24_MTU_M,
		       V2_QPC_BYTE_24_MTU_S, 0);

	memcpy(context->dgid, grh->dgid.raw, sizeof(grh->dgid.raw));
	memset(qpc_mask->dgid, 0, sizeof(grh->dgid.raw));

	roce_set_field(context->byte_84, V2_QPC_BYTE_84_RQ_PRODUCER_IDX_M,
		       V2_QPC_BYTE_84_RQ_PRODUCER_IDX_S, hr_qp->rq.head);
	roce_set_field(qpc_mask->byte_84, V2_QPC_BYTE_84_RQ_PRODUCER_IDX_M,
		       V2_QPC_BYTE_84_RQ_PRODUCER_IDX_S, 0);

	roce_set_field(context->byte_84, V2_QPC_BYTE_84_RQ_CONSUMER_IDX_M,
		       V2_QPC_BYTE_84_RQ_CONSUMER_IDX_S, 0);
	roce_set_field(qpc_mask->byte_84, V2_QPC_BYTE_84_RQ_CONSUMER_IDX_M,
		       V2_QPC_BYTE_84_RQ_CONSUMER_IDX_S, 0);

	roce_set_bit(context->byte_108, V2_QPC_BYTE_108_RX_REQ_PSN_ERR_S, 0);
	roce_set_bit(qpc_mask->byte_108, V2_QPC_BYTE_108_RX_REQ_PSN_ERR_S, 0);

	roce_set_field(context->byte_96, V2_QPC_BYTE_96_RX_REQ_MSN_M,
		       V2_QPC_BYTE_96_RX_REQ_MSN_S, 0);
	roce_set_field(qpc_mask->byte_96, V2_QPC_BYTE_96_RX_REQ_MSN_M,
		       V2_QPC_BYTE_96_RX_REQ_MSN_S, 0);

	roce_set_field(context->byte_108, V2_QPC_BYTE_108_RX_REQ_LAST_OPTYPE_M,
		       V2_QPC_BYTE_108_RX_REQ_LAST_OPTYPE_S, 0);
	roce_set_field(qpc_mask->byte_108, V2_QPC_BYTE_108_RX_REQ_LAST_OPTYPE_M,
		       V2_QPC_BYTE_108_RX_REQ_LAST_OPTYPE_S, 0);

	context->rq_rnr_timer = 0;
	qpc_mask->rq_rnr_timer = 0;

	roce_set_field(context->byte_152, V2_QPC_BYTE_152_RAQ_PSN_M,
		       V2_QPC_BYTE_152_RAQ_PSN_S, attr->rq_psn - 1);
	roce_set_field(qpc_mask->byte_152, V2_QPC_BYTE_152_RAQ_PSN_M,
		       V2_QPC_BYTE_152_RAQ_PSN_S, 0);

	roce_set_field(context->byte_132, V2_QPC_BYTE_132_TRRL_HEAD_MAX_M,
		       V2_QPC_BYTE_132_TRRL_HEAD_MAX_S, 0);
	roce_set_field(qpc_mask->byte_132, V2_QPC_BYTE_132_TRRL_HEAD_MAX_M,
		       V2_QPC_BYTE_132_TRRL_HEAD_MAX_S, 0);

	roce_set_field(context->byte_132, V2_QPC_BYTE_132_TRRL_TAIL_MAX_M,
		       V2_QPC_BYTE_132_TRRL_TAIL_MAX_S, 0);
	roce_set_field(qpc_mask->byte_132, V2_QPC_BYTE_132_TRRL_TAIL_MAX_M,
		       V2_QPC_BYTE_132_TRRL_TAIL_MAX_S, 0);
	roce_set_field(context->byte_168, V2_QPC_BYTE_168_LP_SGEN_INI_M,
		       V2_QPC_BYTE_168_LP_SGEN_INI_S, 3);
	roce_set_field(qpc_mask->byte_168, V2_QPC_BYTE_168_LP_SGEN_INI_M,
		       V2_QPC_BYTE_168_LP_SGEN_INI_S, 0);

	roce_set_field(context->byte_208, V2_QPC_BYTE_208_SR_MAX_M,
		       V2_QPC_BYTE_208_SR_MAX_S,
		       ilog2((unsigned int)attr->max_rd_atomic));
	roce_set_field(qpc_mask->byte_208, V2_QPC_BYTE_208_SR_MAX_M,
		       V2_QPC_BYTE_208_SR_MAX_S, 0);

	roce_set_field(context->byte_28, V2_QPC_BYTE_28_SL_M,
		       V2_QPC_BYTE_28_SL_S, rdma_ah_get_sl(&attr->ah_attr));
	roce_set_field(qpc_mask->byte_28, V2_QPC_BYTE_28_SL_M,
		       V2_QPC_BYTE_28_SL_S, 0);
	hr_qp->sl = rdma_ah_get_sl(&attr->ah_attr);

	return 0;
}

int modify_qp_rtr_to_rts(struct ib_qp *ibqp, const struct ib_qp_attr *attr,
			 int attr_mask, struct hns_roce_v2_qp_context *context,
			 struct hns_roce_v2_qp_context *qpc_mask)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ibqp->device);
	struct hns_roce_qp *hr_qp = to_hr_qp(ibqp);
	struct device *dev = hr_dev->dev;
	dma_addr_t dma_handle;
	u64 *mtts;

	/* Search qp buf's mtts */
	mtts = hns_roce_table_find(hr_dev, &hr_dev->mr_table.mtt_table,
				   hr_qp->mtt.first_seg, &dma_handle);
	if (!mtts) {
		dev_err(dev, "qp buf pa find failed\n");
		return -EINVAL;
	}

	/* If exist optional param, return error */
	if ((attr_mask & IB_QP_ALT_PATH) || (attr_mask & IB_QP_ACCESS_FLAGS) ||
	    (attr_mask & IB_QP_QKEY) || (attr_mask & IB_QP_PATH_MIG_STATE) ||
	    (attr_mask & IB_QP_CUR_STATE) ||
	    (attr_mask & IB_QP_MIN_RNR_TIMER)) {
		dev_err(dev, "RTR2RTS attr_mask (0x%x)error\n", attr_mask);
		return -EINVAL;
	}

	roce_set_field(context->byte_60, V2_QPC_BYTE_60_RTY_NUM_INI_BAK_M,
		       V2_QPC_BYTE_60_RTY_NUM_INI_BAK_S, attr->retry_cnt);
	roce_set_field(qpc_mask->byte_60, V2_QPC_BYTE_60_RTY_NUM_INI_BAK_M,
		       V2_QPC_BYTE_60_RTY_NUM_INI_BAK_S, 0);

	context->sq_cur_blk_addr = (u32)(mtts[0] >> PAGE_ADDR_SHIFT);
	roce_set_field(context->byte_168, V2_QPC_BYTE_168_SQ_CUR_BLK_ADDR_M,
		       V2_QPC_BYTE_168_SQ_CUR_BLK_ADDR_S,
		       mtts[0] >> (32 + PAGE_ADDR_SHIFT));
	qpc_mask->sq_cur_blk_addr = 0;
	roce_set_field(qpc_mask->byte_168, V2_QPC_BYTE_168_SQ_CUR_BLK_ADDR_M,
		       V2_QPC_BYTE_168_SQ_CUR_BLK_ADDR_S, 0);

	context->rx_sq_cur_blk_addr = (u32)(mtts[0] >> PAGE_ADDR_SHIFT);
	roce_set_field(context->byte_232, V2_QPC_BYTE_232_RX_SQ_CUR_BLK_ADDR_M,
		       V2_QPC_BYTE_232_RX_SQ_CUR_BLK_ADDR_S,
		       mtts[0] >> (32 + PAGE_ADDR_SHIFT));
	qpc_mask->rx_sq_cur_blk_addr = 0;
	roce_set_field(qpc_mask->byte_232, V2_QPC_BYTE_232_RX_SQ_CUR_BLK_ADDR_M,
		       V2_QPC_BYTE_232_RX_SQ_CUR_BLK_ADDR_S, 0);

	roce_set_field(context->byte_232, V2_QPC_BYTE_232_IRRL_SGE_IDX_M,
		       V2_QPC_BYTE_232_IRRL_SGE_IDX_S, 0);
	roce_set_field(qpc_mask->byte_232, V2_QPC_BYTE_232_IRRL_SGE_IDX_M,
		       V2_QPC_BYTE_232_IRRL_SGE_IDX_S, 0);

	roce_set_field(context->byte_240, V2_QPC_BYTE_240_RX_ACK_MSN_M,
		       V2_QPC_BYTE_240_RX_ACK_MSN_S, 0);
	roce_set_field(qpc_mask->byte_240, V2_QPC_BYTE_240_RX_ACK_MSN_M,
		       V2_QPC_BYTE_240_RX_ACK_MSN_S, 0);

	roce_set_field(context->byte_244, V2_QPC_BYTE_244_RX_ACK_EPSN_M,
		       V2_QPC_BYTE_244_RX_ACK_EPSN_S, attr->sq_psn);
	roce_set_field(qpc_mask->byte_244, V2_QPC_BYTE_244_RX_ACK_EPSN_M,
		       V2_QPC_BYTE_244_RX_ACK_EPSN_S, 0);

	roce_set_field(context->byte_248, V2_QPC_BYTE_248_ACK_LAST_OPTYPE_M,
		       V2_QPC_BYTE_248_ACK_LAST_OPTYPE_S, 0);
	roce_set_field(qpc_mask->byte_248, V2_QPC_BYTE_248_ACK_LAST_OPTYPE_M,
		       V2_QPC_BYTE_248_ACK_LAST_OPTYPE_S, 0);

	roce_set_bit(context->byte_248, V2_QPC_BYTE_248_IRRL_PSN_VLD_S, 0);
	roce_set_bit(qpc_mask->byte_248, V2_QPC_BYTE_248_IRRL_PSN_VLD_S, 0);

	roce_set_field(context->byte_248, V2_QPC_BYTE_248_IRRL_PSN_M,
		       V2_QPC_BYTE_248_IRRL_PSN_S, 0);
	roce_set_field(qpc_mask->byte_248, V2_QPC_BYTE_248_IRRL_PSN_M,
		       V2_QPC_BYTE_248_IRRL_PSN_S, 0);

	roce_set_field(context->byte_240, V2_QPC_BYTE_240_IRRL_TAIL_REAL_M,
		       V2_QPC_BYTE_240_IRRL_TAIL_REAL_S, 0);
	roce_set_field(qpc_mask->byte_240, V2_QPC_BYTE_240_IRRL_TAIL_REAL_M,
		       V2_QPC_BYTE_240_IRRL_TAIL_REAL_S, 0);

	roce_set_field(context->byte_220, V2_QPC_BYTE_220_RETRY_MSG_PSN_M,
		       V2_QPC_BYTE_220_RETRY_MSG_PSN_S, attr->sq_psn);
	roce_set_field(qpc_mask->byte_220, V2_QPC_BYTE_220_RETRY_MSG_PSN_M,
		       V2_QPC_BYTE_220_RETRY_MSG_PSN_S, 0);

	roce_set_field(context->byte_224, V2_QPC_BYTE_224_RETRY_MSG_PSN_M,
		       V2_QPC_BYTE_224_RETRY_MSG_PSN_S, attr->sq_psn >> 16);
	roce_set_field(qpc_mask->byte_224, V2_QPC_BYTE_224_RETRY_MSG_PSN_M,
		       V2_QPC_BYTE_224_RETRY_MSG_PSN_S, 0);

	roce_set_field(context->byte_224, V2_QPC_BYTE_224_RETRY_MSG_FPKT_PSN_M,
		       V2_QPC_BYTE_224_RETRY_MSG_FPKT_PSN_S, attr->sq_psn);
	roce_set_field(qpc_mask->byte_224, V2_QPC_BYTE_224_RETRY_MSG_FPKT_PSN_M,
		       V2_QPC_BYTE_224_RETRY_MSG_FPKT_PSN_S, 0);

	roce_set_field(context->byte_220, V2_QPC_BYTE_220_RETRY_MSG_MSN_M,
		       V2_QPC_BYTE_220_RETRY_MSG_MSN_S, 0);
	roce_set_field(qpc_mask->byte_220, V2_QPC_BYTE_220_RETRY_MSG_MSN_M,
		       V2_QPC_BYTE_220_RETRY_MSG_MSN_S, 0);

	roce_set_bit(context->byte_248, V2_QPC_BYTE_248_RNR_RETRY_FLAG_S, 0);
	roce_set_bit(qpc_mask->byte_248, V2_QPC_BYTE_248_RNR_RETRY_FLAG_S, 0);

	roce_set_field(context->byte_212, V2_QPC_BYTE_212_CHECK_FLG_M,
		       V2_QPC_BYTE_212_CHECK_FLG_S, 0);
	roce_set_field(qpc_mask->byte_212, V2_QPC_BYTE_212_CHECK_FLG_M,
		       V2_QPC_BYTE_212_CHECK_FLG_S, 0);

	roce_set_field(context->byte_212, V2_QPC_BYTE_212_RETRY_CNT_M,
		       V2_QPC_BYTE_212_RETRY_CNT_S, attr->retry_cnt);
	roce_set_field(qpc_mask->byte_212, V2_QPC_BYTE_212_RETRY_CNT_M,
		       V2_QPC_BYTE_212_RETRY_CNT_S, 0);

	roce_set_field(context->byte_212, V2_QPC_BYTE_212_RETRY_NUM_INIT_M,
		       V2_QPC_BYTE_212_RETRY_NUM_INIT_S, attr->retry_cnt);
	roce_set_field(qpc_mask->byte_212, V2_QPC_BYTE_212_RETRY_NUM_INIT_M,
		       V2_QPC_BYTE_212_RETRY_NUM_INIT_S, 0);

	roce_set_field(context->byte_244, V2_QPC_BYTE_244_RNR_NUM_INIT_M,
		       V2_QPC_BYTE_244_RNR_NUM_INIT_S, attr->rnr_retry);
	roce_set_field(qpc_mask->byte_244, V2_QPC_BYTE_244_RNR_NUM_INIT_M,
		       V2_QPC_BYTE_244_RNR_NUM_INIT_S, 0);

	roce_set_field(context->byte_244, V2_QPC_BYTE_244_RNR_CNT_M,
		       V2_QPC_BYTE_244_RNR_CNT_S, attr->rnr_retry);
	roce_set_field(qpc_mask->byte_244, V2_QPC_BYTE_244_RNR_CNT_M,
		       V2_QPC_BYTE_244_RNR_CNT_S, 0);

	roce_set_field(context->byte_212, V2_QPC_BYTE_212_LSN_M,
		       V2_QPC_BYTE_212_LSN_S, 0x100);
	roce_set_field(qpc_mask->byte_212, V2_QPC_BYTE_212_LSN_M,
		       V2_QPC_BYTE_212_LSN_S, 0);

	if (attr->timeout < 0xf)
		roce_set_field(context->byte_28, V2_QPC_BYTE_28_AT_M,
			       V2_QPC_BYTE_28_AT_S, 0xf);
	else
		roce_set_field(context->byte_28, V2_QPC_BYTE_28_AT_M,
			       V2_QPC_BYTE_28_AT_S, attr->timeout);

	roce_set_field(qpc_mask->byte_28, V2_QPC_BYTE_28_AT_M,
		       V2_QPC_BYTE_28_AT_S, 0);

	roce_set_field(context->byte_28, V2_QPC_BYTE_28_SL_M,
		       V2_QPC_BYTE_28_SL_S,
		       rdma_ah_get_sl(&attr->ah_attr));
	roce_set_field(qpc_mask->byte_28, V2_QPC_BYTE_28_SL_M,
		       V2_QPC_BYTE_28_SL_S, 0);
	hr_qp->sl = rdma_ah_get_sl(&attr->ah_attr);

	roce_set_field(context->byte_172, V2_QPC_BYTE_172_SQ_CUR_PSN_M,
		       V2_QPC_BYTE_172_SQ_CUR_PSN_S, attr->sq_psn);
	roce_set_field(qpc_mask->byte_172, V2_QPC_BYTE_172_SQ_CUR_PSN_M,
		       V2_QPC_BYTE_172_SQ_CUR_PSN_S, 0);

	roce_set_field(context->byte_196, V2_QPC_BYTE_196_IRRL_HEAD_M,
		       V2_QPC_BYTE_196_IRRL_HEAD_S, 0);
	roce_set_field(qpc_mask->byte_196, V2_QPC_BYTE_196_IRRL_HEAD_M,
		       V2_QPC_BYTE_196_IRRL_HEAD_S, 0);
	roce_set_field(context->byte_196, V2_QPC_BYTE_196_SQ_MAX_PSN_M,
		       V2_QPC_BYTE_196_SQ_MAX_PSN_S, attr->sq_psn);
	roce_set_field(qpc_mask->byte_196, V2_QPC_BYTE_196_SQ_MAX_PSN_M,
		       V2_QPC_BYTE_196_SQ_MAX_PSN_S, 0);

	return 0;
}

static int hns_roce_v2_m_qp(struct ib_qp *ibqp, const struct ib_qp_attr *attr,
			    int attr_mask, enum ib_qp_state cur_state,
			    enum ib_qp_state new_state)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ibqp->device);
	struct hns_roce_qp *hr_qp = to_hr_qp(ibqp);
	struct hns_roce_v2_qp_context *context;
	struct hns_roce_v2_qp_context *qpc_mask;
	struct device *dev = hr_dev->dev;
	int ret = -EINVAL;

	context = kzalloc(2 * sizeof(*context), GFP_KERNEL);
	if (!context)
		return -ENOMEM;

	qpc_mask = context + 1;

	memset(qpc_mask, 0xff, sizeof(*qpc_mask));
	/* Reset -> Init */
	if (cur_state == IB_QPS_RESET && new_state == IB_QPS_INIT) {
		modify_qp_reset_to_init(ibqp, attr, context, qpc_mask);
	} else if (cur_state == IB_QPS_INIT && new_state == IB_QPS_INIT) {
		modify_qp_init_to_init(ibqp, attr, attr_mask, context,
				       qpc_mask);
	} else if (cur_state == IB_QPS_INIT && new_state == IB_QPS_RTR) {
		ret = modify_qp_init_to_rtr(ibqp, attr, attr_mask, context,
					    qpc_mask);
		if (ret)
			goto out;
	} else if (cur_state == IB_QPS_RTR && new_state == IB_QPS_RTS) {
		ret = modify_qp_rtr_to_rts(ibqp, attr, attr_mask, context,
					   qpc_mask);
		if (ret)
			goto out;
	} else if ((cur_state == IB_QPS_RTS && new_state == IB_QPS_RTS) ||
		   (cur_state == IB_QPS_SQE && new_state == IB_QPS_RTS) ||
		   (cur_state == IB_QPS_RTS && new_state == IB_QPS_SQD) ||
		   (cur_state == IB_QPS_SQD && new_state == IB_QPS_SQD) ||
		   (cur_state == IB_QPS_SQD && new_state == IB_QPS_RTS) ||
		   (cur_state == IB_QPS_INIT && new_state == IB_QPS_RESET) ||
		   (cur_state == IB_QPS_RTR && new_state == IB_QPS_RESET) ||
		   (cur_state == IB_QPS_RTS && new_state == IB_QPS_RESET) ||
		   (cur_state == IB_QPS_ERR && new_state == IB_QPS_RESET) ||
		   (cur_state == IB_QPS_INIT && new_state == IB_QPS_ERR) ||
		   (cur_state == IB_QPS_RTR && new_state == IB_QPS_ERR) ||
		   (cur_state == IB_QPS_RTS && new_state == IB_QPS_ERR) ||
		   (cur_state == IB_QPS_SQD && new_state == IB_QPS_ERR) ||
		   (cur_state == IB_QPS_SQE && new_state == IB_QPS_ERR)) {
		/* Nothing */
		;
	} else {
		dev_err(dev, "Illegal state for QP!\n");
		goto out;
	}

	/* Every status migrate must change state */
	roce_set_field(context->byte_60, V2_QPC_BYTE_60_QP_ST_M,
		       V2_QPC_BYTE_60_QP_ST_S, new_state);
	roce_set_field(qpc_mask->byte_60, V2_QPC_BYTE_60_QP_ST_M,
		       V2_QPC_BYTE_60_QP_ST_S, 0);

	/* SW pass context to HW */
	ret = hns_roce_v2_qp_modify(hr_dev, &hr_qp->mtt, cur_state, new_state,
				    context, hr_qp);
	if (ret) {
		dev_err(dev, "hns_roce_qp_modify failed(%d)\n", ret);
		goto out;
	}

	hr_qp->state = new_state;

	if (attr_mask & IB_QP_MAX_DEST_RD_ATOMIC)
		hr_qp->resp_depth = attr->max_dest_rd_atomic;
	if (attr_mask & IB_QP_PORT) {
		hr_qp->port = attr->port_num - 1;
		hr_qp->phy_port = hr_dev->iboe.phy_port[hr_qp->port];
	}

	if (new_state == IB_QPS_RESET && !ibqp->uobject) {
		hns_roce_v2_cq_clean(to_hr_cq(ibqp->recv_cq), hr_qp->qpn,
				     ibqp->srq ? to_hr_srq(ibqp->srq) : NULL);
		if (ibqp->send_cq != ibqp->recv_cq)
			hns_roce_v2_cq_clean(to_hr_cq(ibqp->send_cq),
					     hr_qp->qpn, NULL);

		hr_qp->rq.head = 0;
		hr_qp->rq.tail = 0;
		hr_qp->sq.head = 0;
		hr_qp->sq.tail = 0;
		hr_qp->sq_next_wqe = 0;
		hr_qp->next_sge = 0;
	}

out:
	kfree(context);
	return ret;
}

int hns_roce_v2_modify_qp(struct ib_qp *ibqp, const struct ib_qp_attr *attr,
			  int attr_mask, enum ib_qp_state cur_state,
			  enum ib_qp_state new_state)
{
	return hns_roce_v2_m_qp(ibqp, attr, attr_mask, cur_state, new_state);
}

static int hns_roce_v2_query_qpc(struct hns_roce_dev *hr_dev,
				 struct hns_roce_qp *hr_qp,
				 struct hns_roce_v2_qp_context *hr_context)
{
	struct hns_roce_cmd_mailbox *mailbox;
	int ret;

	mailbox = hns_roce_alloc_cmd_mailbox(hr_dev);
	if (IS_ERR(mailbox))
		return PTR_ERR(mailbox);

	ret = hns_roce_cmd_mbox(hr_dev, 0, mailbox->dma, hr_qp->qpn, 0,
				HNS_ROCE_CMD_QUERY_QPC,
				HNS_ROCE_CMD_TIMEOUT_MSECS);
	if (!ret)
		memcpy(hr_context, mailbox->buf, sizeof(*hr_context));
	else
		dev_err(hr_dev->dev, "QUERY QP cmd process error\n");

	hns_roce_free_cmd_mailbox(hr_dev, mailbox);

	return ret;
}

static int hns_roce_v2_q_qp(struct ib_qp *ibqp, struct ib_qp_attr *qp_attr,
			    int qp_attr_mask,
			    struct ib_qp_init_attr *qp_init_attr)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ibqp->device);
	struct hns_roce_qp *hr_qp = to_hr_qp(ibqp);
	struct hns_roce_v2_qp_context *context;
	struct device *dev = hr_dev->dev;
	int state;
	int ret;

	context = kzalloc(sizeof(*context), GFP_KERNEL);
	if (!context)
		return -ENOMEM;

	memset(qp_attr, 0, sizeof(*qp_attr));
	memset(qp_init_attr, 0, sizeof(*qp_init_attr));

	mutex_lock(&hr_qp->mutex);

	if (hr_qp->state == IB_QPS_RESET) {
		qp_attr->qp_state = IB_QPS_RESET;
		goto done;
	}

	ret = hns_roce_v2_query_qpc(hr_dev, hr_qp, context);
	if (ret) {
		dev_err(dev, "query qpc error\n");
		ret = -EINVAL;
		goto out;
	}

	state = roce_get_field(context->byte_60, V2_QPC_BYTE_60_QP_ST_M,
			       V2_QPC_BYTE_60_QP_ST_S);

	hr_qp->state = (u8)to_ib_qp_st((enum hns_roce_v2_qp_state)state);
	if (hr_qp->state == -1) {
		dev_err(dev, "Illegal ib_qp_state\n");
		ret = -EINVAL;
		goto out;
	}
	qp_attr->qp_state = (enum ib_qp_state)hr_qp->state;
	qp_attr->path_mtu = (enum ib_mtu)roce_get_field(context->byte_24,
							V2_QPC_BYTE_24_MTU_M,
							V2_QPC_BYTE_24_MTU_S);
	qp_attr->path_mig_state = IB_MIG_ARMED;
	if (hr_qp->ibqp.qp_type == IB_QPT_UD)
		qp_attr->qkey = V2_QKEY_VAL;

	qp_attr->rq_psn = roce_get_field(context->byte_108,
					 V2_QPC_BYTE_108_RX_REQ_EPSN_M,
					 V2_QPC_BYTE_108_RX_REQ_EPSN_S);

	qp_attr->sq_psn = (u32)roce_get_field(context->byte_172,
					     V2_QPC_BYTE_172_SQ_CUR_PSN_M,
					     V2_QPC_BYTE_172_SQ_CUR_PSN_S);

	qp_attr->dest_qp_num = (u8)roce_get_field(context->byte_56,
						  V2_QPC_BYTE_56_DQPN_M,
						  V2_QPC_BYTE_56_DQPN_S);

	qp_attr->qp_access_flags = ((roce_get_bit(context->byte_76,
						  V2_QPC_BYTE_76_RRE_S)) << 2) |
				   ((roce_get_bit(context->byte_76,
						  V2_QPC_BYTE_76_RWE_S)) << 1) |
				   ((roce_get_bit(context->byte_76,
						  V2_QPC_BYTE_76_ATE_S)) << 3);
	if (hr_qp->ibqp.qp_type == IB_QPT_RC ||
	    hr_qp->ibqp.qp_type == IB_QPT_UC) {
		struct ib_global_route *grh =
				rdma_ah_retrieve_grh(&qp_attr->ah_attr);

		rdma_ah_set_sl(&qp_attr->ah_attr,
			       roce_get_field(context->byte_28,
					      V2_QPC_BYTE_28_SL_M,
					      V2_QPC_BYTE_28_SL_S));
		grh->flow_label = roce_get_field(context->byte_28,
						 V2_QPC_BYTE_28_FL_M,
						 V2_QPC_BYTE_28_FL_S);
		grh->sgid_index = roce_get_field(context->byte_20,
						 V2_QPC_BYTE_20_SGID_IDX_M,
						 V2_QPC_BYTE_20_SGID_IDX_S);
		grh->hop_limit = roce_get_field(context->byte_24,
						V2_QPC_BYTE_24_HOP_LIMIT_M,
						V2_QPC_BYTE_24_HOP_LIMIT_S);
		grh->traffic_class = roce_get_field(context->byte_24,
						    V2_QPC_BYTE_24_TC_M,
						    V2_QPC_BYTE_24_TC_S);

		memcpy(grh->dgid.raw, context->dgid, sizeof(grh->dgid.raw));
	}

	qp_attr->port_num = hr_qp->port + 1;
	qp_attr->sq_draining = 0;
	qp_attr->max_rd_atomic = 1 << roce_get_field(context->byte_208,
						     V2_QPC_BYTE_208_SR_MAX_M,
						     V2_QPC_BYTE_208_SR_MAX_S);
	qp_attr->max_dest_rd_atomic = 1 << roce_get_field(context->byte_140,
						     V2_QPC_BYTE_140_RR_MAX_M,
						     V2_QPC_BYTE_140_RR_MAX_S);
	qp_attr->min_rnr_timer = (u8)roce_get_field(context->byte_80,
						 V2_QPC_BYTE_80_MIN_RNR_TIME_M,
						 V2_QPC_BYTE_80_MIN_RNR_TIME_S);

	qp_attr->timeout = (u8)roce_get_field(context->byte_28,
					      V2_QPC_BYTE_28_AT_M,
					      V2_QPC_BYTE_28_AT_S);
	qp_attr->retry_cnt = roce_get_field(context->byte_212,
					    V2_QPC_BYTE_212_RETRY_CNT_M,
					    V2_QPC_BYTE_212_RETRY_CNT_S);
	qp_attr->rnr_retry = context->rq_rnr_timer;

done:
	qp_attr->cur_qp_state = qp_attr->qp_state;
	qp_attr->cap.max_recv_wr = hr_qp->rq.wqe_cnt;
	qp_attr->cap.max_recv_sge = hr_qp->rq.max_gs;

	if (!ibqp->uobject) {
		qp_attr->cap.max_send_wr = hr_qp->sq.wqe_cnt;
		qp_attr->cap.max_send_sge = hr_qp->sq.max_gs;
	} else {
		qp_attr->cap.max_send_wr = 0;
		qp_attr->cap.max_send_sge = 0;
	}

	qp_init_attr->cap = qp_attr->cap;

out:
	mutex_unlock(&hr_qp->mutex);
	kfree(context);
	return ret;
}

int hns_roce_v2_query_qp(struct ib_qp *ibqp, struct ib_qp_attr *qp_attr,
			 int qp_attr_mask, struct ib_qp_init_attr *qp_init_attr)
{
	return hns_roce_v2_q_qp(ibqp, qp_attr, qp_attr_mask, qp_init_attr);
}

static void hns_roce_v2_destroy_qp_common(struct hns_roce_dev *hr_dev,
					  struct hns_roce_qp *hr_qp,
					  int is_user)
{
	struct hns_roce_cq *send_cq, *recv_cq;
	struct device *dev = hr_dev->dev;
	int ret;

	if (hr_qp->ibqp.qp_type == IB_QPT_RC) {
		if (hr_qp->state != IB_QPS_RESET) {
			/* Modify qp to reset before destroying qp */
			ret = hns_roce_v2_modify_qp(&hr_qp->ibqp, NULL, 0,
						    hr_qp->state, IB_QPS_RESET);
			if (ret)
				dev_err(dev, "modify QP %06lx to ERR failed.\n",
					hr_qp->qpn);
		}
	}

	send_cq = to_hr_cq(hr_qp->ibqp.send_cq);
	recv_cq = to_hr_cq(hr_qp->ibqp.recv_cq);

	hns_roce_lock_cqs(send_cq, recv_cq);

	if (!is_user) {
		__hns_roce_v2_cq_clean(recv_cq, hr_qp->qpn, hr_qp->ibqp.srq ?
				       to_hr_srq(hr_qp->ibqp.srq) : NULL);
		if (send_cq != recv_cq)
			__hns_roce_v2_cq_clean(send_cq, hr_qp->qpn, NULL);
	}

	hns_roce_qp_remove(hr_dev, hr_qp);

	hns_roce_unlock_cqs(send_cq, recv_cq);

	hns_roce_qp_free(hr_dev, hr_qp);

	/* Not special_QP, free their QPN */
	if ((hr_qp->ibqp.qp_type == IB_QPT_RC) ||
	    (hr_qp->ibqp.qp_type == IB_QPT_UC) ||
	    (hr_qp->ibqp.qp_type == IB_QPT_UD))
		hns_roce_release_range_qp(hr_dev, hr_qp->qpn, 1);

	hns_roce_mtt_cleanup(hr_dev, &hr_qp->mtt);

	if (is_user) {
		ib_umem_release(hr_qp->umem);
	} else {
		kfree(hr_qp->sq.wrid);
		kfree(hr_qp->rq.wrid);
		hns_roce_buf_free(hr_dev, hr_qp->buff_size, &hr_qp->hr_buf);
	}
}

int hns_roce_v2_destroy_qp(struct ib_qp *ibqp)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ibqp->device);
	struct hns_roce_qp *hr_qp = to_hr_qp(ibqp);

	hns_roce_v2_destroy_qp_common(hr_dev, hr_qp, !!ibqp->pd->uobject);

	if (hr_qp->ibqp.qp_type == IB_QPT_GSI)
		kfree(hr_to_hr_sqp(hr_qp));
	else
		kfree(hr_qp);

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

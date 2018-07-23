/*
 * Copyright (c) 2018 Hisilicon Limited.
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

#include <rdma/ib_umem.h>
#include <rdma/hns-abi.h>
#include "hns_roce_device.h"
#include "hns_roce_cmd.h"
#include "hns_roce_hem.h"

static void hns_roce_ib_srq_event(struct hns_roce_srq *srq,
				  enum hns_roce_event event_type)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(srq->ibsrq.device);
	struct ib_srq *ibsrq = &srq->ibsrq;
	struct ib_event event;

	if (ibsrq->event_handler) {
		event.device      = ibsrq->device;
		event.element.srq = ibsrq;
		switch (event_type) {
		case HNS_ROCE_EVENT_TYPE_SRQ_LIMIT_REACH:
			event.event = IB_EVENT_SRQ_LIMIT_REACHED;
			break;
		case HNS_ROCE_EVENT_TYPE_SRQ_CATAS_ERROR:
			event.event = IB_EVENT_SRQ_ERR;
			break;
		default:
			dev_err(hr_dev->dev,
			   "hns_roce:Unexpected event type 0x%x on SRQ %06lx\n",
			   event_type, srq->srqn);
			return;
		}

		ibsrq->event_handler(&event, ibsrq->srq_context);
	}
}

static int hns_roce_sw2hw_srq(struct hns_roce_dev *dev,
			      struct hns_roce_cmd_mailbox *mailbox,
			      unsigned long srq_num)
{
	return hns_roce_cmd_mbox(dev, mailbox->dma, 0, srq_num, 0,
				 HNS_ROCE_CMD_SW2HW_SRQ,
				 HNS_ROCE_CMD_TIMEOUT_MSECS);
}

static int hns_roce_hw2sw_srq(struct hns_roce_dev *dev,
			     struct hns_roce_cmd_mailbox *mailbox,
			     unsigned long srq_num)
{
	return hns_roce_cmd_mbox(dev, 0, mailbox ? mailbox->dma : 0, srq_num,
				 mailbox ? 0 : 1, HNS_ROCE_CMD_HW2SW_SRQ,
				 HNS_ROCE_CMD_TIMEOUT_MSECS);
}

static int hns_roce_arm_srq(struct hns_roce_dev *dev, int srq_num, int lm)
{
	return hns_roce_cmd_mbox(dev, lm, 0, srq_num, 0,
				 HNS_ROCE_CMD_MODIFY_SRQC,
				 HNS_ROCE_CMD_TIMEOUT_MSECS);
}

int hns_roce_srq_alloc(struct hns_roce_dev *hr_dev, u32 pdn, u32 cqn, u16 xrcd,
		       struct hns_roce_mtt *hr_mtt, u64 db_rec_addr,
		       struct hns_roce_srq *srq)
{
	struct hns_roce_srq_table *srq_table = &hr_dev->srq_table;
	struct hns_roce_cmd_mailbox *mailbox;
	dma_addr_t dma_handle_wqe;
	dma_addr_t dma_handle_idx;
	u64 *mtts_wqe;
	u64 *mtts_idx;
	int ret;

	/* Get the physical address of srq buf */
	mtts_wqe = hns_roce_table_find(hr_dev,
				       &hr_dev->mr_table.mtt_srqwqe_table,
				       srq->mtt.first_seg,
				       &dma_handle_wqe);
	if (!mtts_wqe) {
		dev_err(hr_dev->dev,
			"SRQ alloc.Failed to find srq buf addr.\n");
		return -EINVAL;
	}

	/* Get physical address of idx que buf */
	mtts_idx = hns_roce_table_find(hr_dev, &hr_dev->mr_table.mtt_idx_table,
				       srq->idx_que.mtt.first_seg,
				       &dma_handle_idx);
	if (!mtts_idx) {
		dev_err(hr_dev->dev,
			"SRQ alloc.Failed to find idx que buf addr.\n");
		return -EINVAL;
	}

	ret = hns_roce_bitmap_alloc(&srq_table->bitmap, &srq->srqn);
	if (ret == -1) {
		dev_err(hr_dev->dev, "SRQ alloc.Failed to alloc index.\n");
		return -ENOMEM;
	}

	ret = hns_roce_table_get(hr_dev, &srq_table->table, srq->srqn);
	if (ret)
		goto err_out;

	spin_lock_irq(&srq_table->lock);
	ret = radix_tree_insert(&srq_table->tree, srq->srqn, srq);
	spin_unlock_irq(&srq_table->lock);
	if (ret)
		goto err_put;

	mailbox = hns_roce_alloc_cmd_mailbox(hr_dev);
	if (IS_ERR(mailbox)) {
		ret = PTR_ERR(mailbox);
		goto err_radix;
	}

	hr_dev->hw->write_srqc(hr_dev, srq, pdn, xrcd, cqn, mailbox->buf,
			       mtts_wqe, mtts_idx, dma_handle_wqe,
			       dma_handle_idx);

	ret = hns_roce_sw2hw_srq(hr_dev, mailbox, srq->srqn);
	hns_roce_free_cmd_mailbox(hr_dev, mailbox);
	if (ret)
		goto err_radix;

	refcount_set(&srq->refcount, 1);
	init_completion(&srq->free);
	return ret;

err_radix:
	spin_lock_irq(&srq_table->lock);
	radix_tree_delete(&srq_table->tree, srq->srqn);
	spin_unlock_irq(&srq_table->lock);

err_put:
	hns_roce_table_put(hr_dev, &srq_table->table, srq->srqn);

err_out:
	hns_roce_bitmap_free(&srq_table->bitmap, srq->srqn, BITMAP_NO_RR);
	return ret;
}

void hns_roce_srq_free(struct hns_roce_dev *hr_dev, struct hns_roce_srq *srq)
{
	struct hns_roce_srq_table *srq_table = &hr_dev->srq_table;
	int ret;

	ret = hns_roce_hw2sw_srq(hr_dev, NULL, srq->srqn);
	if (ret)
		dev_err(hr_dev->dev, "HW2SW_SRQ failed (%d) for CQN %06lx\n",
			ret, srq->srqn);

	spin_lock_irq(&srq_table->lock);
	radix_tree_delete(&srq_table->tree, srq->srqn);
	spin_unlock_irq(&srq_table->lock);

	if (refcount_dec_and_test(&srq->refcount))
		complete(&srq->free);
	wait_for_completion(&srq->free);

	hns_roce_table_put(hr_dev, &srq_table->table, srq->srqn);
	hns_roce_bitmap_free(&srq_table->bitmap, srq->srqn, BITMAP_NO_RR);
}

struct ib_srq *hns_roce_create_srq(struct ib_pd *pd,
				   struct ib_srq_init_attr *srq_init_attr,
				   struct ib_udata *udata)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(pd->device);
	struct hns_roce_srq *srq;
	int srq_desc_size;
	int srq_buf_size;
	u32 page_shift;
	int ret = 0;
	u32 npages;
	u16 xrcdn;
	u32 cqn;

	/* Check the actual SRQ wqe and SRQ sge num */
	if (srq_init_attr->attr.max_wr >= hr_dev->caps.max_srq_wrs ||
	    srq_init_attr->attr.max_sge > hr_dev->caps.max_srq_sges)
		return ERR_PTR(-EINVAL);

	srq = kzalloc(sizeof(*srq), GFP_KERNEL);
	if (!srq)
		return ERR_PTR(-ENOMEM);

	mutex_init(&srq->mutex);
	spin_lock_init(&srq->lock);

	srq->max = roundup_pow_of_two(srq_init_attr->attr.max_wr + 1);
	srq->max_gs = srq_init_attr->attr.max_sge;

	srq_desc_size = 16 * srq->max_gs;

	srq->wqe_shift = ilog2(srq_desc_size);

	srq_buf_size = srq->max * srq_desc_size;

	srq->idx_que.entry_sz = HNS_ROCE_IDX_QUE_ENTRY_SZ;
	srq->idx_que.buf_size = srq->max * srq->idx_que.entry_sz;

	if (udata) {
		struct hns_roce_ib_create_srq  ucmd;

		if (ib_copy_from_udata(&ucmd, udata, sizeof(ucmd))) {
			ret = -EFAULT;
			goto err_srq;
		}

		srq->umem = ib_umem_get(pd->uobject->context, ucmd.buf_addr,
					srq_buf_size, 0, 0);
		if (IS_ERR(srq->umem)) {
			ret = PTR_ERR(srq->umem);
			goto err_srq;
		}

		srq->mtt.mtt_type = MTT_TYPE_SRQWQE;
		if (hr_dev->caps.srqwqe_buf_pg_sz) {
			npages = (ib_umem_page_count(srq->umem) +
				  (1 << hr_dev->caps.srqwqe_buf_pg_sz) - 1) /
				  (1 << hr_dev->caps.srqwqe_buf_pg_sz);
			page_shift = PAGE_SHIFT + hr_dev->caps.srqwqe_buf_pg_sz;
			ret = hns_roce_mtt_init(hr_dev, npages,
						page_shift,
						&srq->mtt);
		} else
			ret = hns_roce_mtt_init(hr_dev,
						ib_umem_page_count(srq->umem),
						srq->umem->page_shift,
						&srq->mtt);
		if (ret)
			goto err_buf;

		ret = hns_roce_ib_umem_write_mtt(hr_dev, &srq->mtt, srq->umem);
		if (ret)
			goto err_mtt;

		/* config index queue BA */
		srq->idx_que.umem = ib_umem_get(pd->uobject->context,
						ucmd.que_addr,
						srq->idx_que.buf_size, 0, 0);
		if (IS_ERR(srq->idx_que.umem)) {
			dev_err(hr_dev->dev,
				"ib_umem_get error for index queue\n");
			goto err_mtt;
		}

		srq->idx_que.mtt.mtt_type = MTT_TYPE_IDX;
		if (hr_dev->caps.idx_buf_pg_sz) {
			npages = (ib_umem_page_count(srq->idx_que.umem) +
				  (1 << hr_dev->caps.idx_buf_pg_sz) - 1) /
				  (1 << hr_dev->caps.idx_buf_pg_sz);
			page_shift = PAGE_SHIFT + hr_dev->caps.idx_buf_pg_sz;
			ret = hns_roce_mtt_init(hr_dev, npages,
						page_shift, &srq->idx_que.mtt);
		} else {
			ret = hns_roce_mtt_init(hr_dev,
				       ib_umem_page_count(srq->idx_que.umem),
				       srq->idx_que.umem->page_shift,
				       &srq->idx_que.mtt);
		}

		if (ret) {
			dev_err(hr_dev->dev,
				"hns_roce_mtt_init error for idx que\n");
			goto err_idx_buf;
		}

		ret = hns_roce_ib_umem_write_mtt(hr_dev, &srq->idx_que.mtt,
						 srq->idx_que.umem);
		if (ret) {
			dev_err(hr_dev->dev,
			      "hns_roce_ib_umem_write_mtt error for idx que\n");
			goto err_write_mtt;
		}
	} else {
		u32 page_shift = PAGE_SHIFT + hr_dev->caps.mtt_buf_pg_sz;

		if (hns_roce_buf_alloc(hr_dev, srq_buf_size, PAGE_SIZE * 2,
				       &srq->buf, page_shift)) {
			ret = -ENOMEM;
			goto err_buf;
		}

		srq->head = 0;
		srq->tail = srq->max - 1;
		srq->wqe_ctr = 0;

		ret = hns_roce_mtt_init(hr_dev, srq->buf.npages,
					srq->buf.page_shift, &srq->mtt);
		if (ret)
			goto err_buf;

		ret = hns_roce_buf_write_mtt(hr_dev, &srq->mtt, &srq->buf);
		if (ret)
			goto err_mtt;

		srq->wrid = kvmalloc_array(srq->max, sizeof(u64), GFP_KERNEL);
		if (!srq->wrid) {
			ret = -ENOMEM;
			goto err_mtt;
		}
	}

	cqn = ib_srq_has_cq(srq_init_attr->srq_type) ?
	      to_hr_cq(srq_init_attr->ext.cq)->cqn : 0;
	xrcdn = (srq_init_attr->srq_type == IB_SRQT_XRC) ?
		to_hr_xrcd(srq_init_attr->ext.xrc.xrcd)->xrcdn : 0;

	srq->db_reg_l = hr_dev->reg_base + DB_REG_OFFSET;

	ret = hns_roce_srq_alloc(hr_dev, to_hr_pd(pd)->pdn, cqn, xrcdn,
				 &srq->mtt, 0, srq);
	if (ret)
		goto err_wrid;

	srq->event = hns_roce_ib_srq_event;
	srq->ibsrq.ext.xrc.srq_num = srq->srqn;

	return &srq->ibsrq;

err_wrid:
	kvfree(srq->wrid);

err_write_mtt:
	if (udata)
		hns_roce_mtt_cleanup(hr_dev, &srq->idx_que.mtt);
err_idx_buf:
	if (udata)
		ib_umem_release(srq->idx_que.umem);

	if (udata)
		hns_roce_mtt_cleanup(hr_dev, &srq->idx_que.mtt);

err_mtt:
	hns_roce_mtt_cleanup(hr_dev, &srq->mtt);

err_buf:
	if (udata)
		ib_umem_release(srq->umem);
	else
		hns_roce_buf_free(hr_dev, srq_buf_size, &srq->buf);

err_srq:
	kfree(srq);
	return ERR_PTR(ret);
}

int hns_roce_modify_srq(struct ib_srq *ibsrq, struct ib_srq_attr *srq_attr,
			enum ib_srq_attr_mask srq_attr_mask,
			struct ib_udata *udata)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ibsrq->device);
	struct hns_roce_srq *srq = to_hr_srq(ibsrq);
	int ret;

	if (srq_attr_mask & IB_SRQ_LIMIT) {
		if (srq_attr->srq_limit >= srq->max)
			return -EINVAL;

		mutex_lock(&srq->mutex);
		ret = hns_roce_arm_srq(hr_dev, srq->srqn, srq_attr->srq_limit);
		mutex_unlock(&srq->mutex);

		if (ret)
			return ret;
	}

	return 0;
}

int hns_roce_destroy_srq(struct ib_srq *ibsrq)
{
	struct hns_roce_dev *hr_dev = to_hr_dev(ibsrq->device);
	struct hns_roce_srq *srq = to_hr_srq(ibsrq);

	hns_roce_srq_free(hr_dev, srq);
	hns_roce_mtt_cleanup(hr_dev, &srq->mtt);

	if (ibsrq->uobject) {
		ib_umem_release(srq->umem);
	} else {
		kvfree(srq->wrid);
		hns_roce_buf_free(hr_dev, srq->max << srq->wqe_shift,
				  &srq->buf);
	}

	kfree(srq);

	return 0;
}

struct hns_roce_srq *hns_roce_srq_lookup(struct hns_roce_dev *hr_dev, u32 srqn)
{
	struct hns_roce_srq_table *srq_table = &hr_dev->srq_table;
	struct hns_roce_srq *srq;

	rcu_read_lock();
	srq = radix_tree_lookup(&srq_table->tree,
				srqn & (hr_dev->caps.max_srqs - 1));
	rcu_read_unlock();

	return srq;
}
EXPORT_SYMBOL_GPL(hns_roce_srq_lookup);

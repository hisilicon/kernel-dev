/*
 * Copyright (c) 2018 HiSilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <asm/io.h>
#include <linux/irqreturn.h>
#include "qm.h"

/**
 * true: busy
 * false: ready
 */
static inline bool hacc_qm_mb_is_busy(struct qm_info *qm)
{
    u32 val;
    void *base = qm->fun_base;

    val = readl(base + MAILBOX_CMD_SEND_BASE);
    return ((val >> MAILBOX_BUSY_SHIFT) & 0x1) ? true : false;
}

static inline void mb_write(struct qm_info *qm, void *src)
{
	void __iomem *fun_base = qm->fun_base + MAILBOX_CMD_SEND_BASE;
	unsigned long tmp0 = 0, tmp1 = 0;

	asm volatile ("ldp %0, %1, %3     \n"
		      "stp %0, %1, %2     \n"
		      "dsb sy             \n"
		      : "=&r" (tmp0),
		        "=&r" (tmp1),
			"+Q" (*((char *)fun_base))
		      : "Q" (*((char *)src))
		      : "memory");
}

/**
 * hacc_mb - Send HiSilicon accelarator mailbox command.
 * @qm: Queue Management struct
 * @cmd: Mailbox command
 * @phys_addr: ...
 * @queue: Queue number for SQC/CQC, function number for SQC_BT/CQC_BT
 * @op: 0 for writing, 1 for reading
 * @event: 0 for polling mode, 1 for event mode
 */
/* fix: how to do read mb */
static int hacc_mb(struct qm_info *qm, u8 cmd, u64 phys_addr, u16 queue,
                   bool op, bool event)
{
        struct mailbox mailbox;

        memset(&mailbox, 0, sizeof(struct mailbox));

	/* to do: prepare mb date */
	mailbox.w0 = cmd |
		     (event ? 0x1 << MAILBOX_EVENT_SHIFT : 0) |
		     (op ? 0x1 << MAILBOX_OP_SHIFT : 0) |
		     (0x1 << MAILBOX_BUSY_SHIFT);
        mailbox.queue_num = queue;
	mailbox.mb_base_l = lower_32_bits(phys_addr);
	mailbox.mb_base_h = upper_32_bits(phys_addr);
	mailbox.rsvd = 0;

	pr_err("in %s\n", __FUNCTION__);
	pr_err("in %x\n", mailbox.queue_num);
	pr_err("in %x\n", mailbox.mb_base_l);
	pr_err("in %x\n", mailbox.mb_base_h);
	pr_err("in %x\n", mailbox.rsvd);

	spin_lock(&qm->mailbox_lock);

	while(hacc_qm_mb_is_busy(qm));
	mb_write(qm, &mailbox);
	while(hacc_qm_mb_is_busy(qm));

    	spin_unlock(&qm->mailbox_lock);

	return 0;
}

/**
 * hacc_db - Send HiSilicon accelarator doorbell command.
 * @qm:
 * @qn:
 * @cmd:
 * @index:
 * @prority:
 */
static int hacc_db(struct qm_info *qm, u16 qn, u8 cmd, u16 index, u8 priority)
{
	/* fix me: check input params */
	//pr_err("in %s: qn: %d, cmd: %d, index: %d, priority: %d", __FUNCTION__,
			//qn, cmd, index, priority);

	void *base = qm->fun_base;
	u64 doorbell = 0;

	doorbell = (u64)qn | ((u64)cmd << 16);
	doorbell |= ((u64)index | ((u64)priority << 16)) << 32;

	//pr_err("in %llx\n", doorbell);

	/* to do: start doorbell, fix me */
	writeq(doorbell, base + DOORBELL_CMD_SEND_BASE);

	return 0;
}

irqreturn_t hacc_irq_thread(int irq, void *data)
{
	struct qm_info *qm_info = (struct qm_info *)data;
        struct eqc *eqc = qm_info->eqc;
	struct cqc *cqc_base = qm_info->cqc_base;
	struct eqe *eq_base = qm_info->eq_base;
	struct eqe *eqe = eq + qm_info->eq_head;
	char *cqe_h, *cq;
        struct cqc *cqc;
	u16 eq_index = EQC_HEAD_INDEX(eqc);
	u16 cq_index;

	/* to do: if no new eqe, there is no irq, do nothing or reture error */

	/* handle all eqs from eqe_h */
	while (eqe->phase == eqc->phase) {

                qp = find_hisi_acc_qp;

                if (qp->type == CRYPTO_QUEUE)

                        cqc = cqc_base + eqe_to_cqn;
                        cq = phys_to_virt(CQC_CQ_ADDRESS(cqc_current));
                        cq_index = CQC_HEAD_INDEX(cqc_current); /* fix me: wrong here */
                        cqe_h = cq + cq_index * QM_CQE_SIZE;

                        cqe = find_cqe;

                        update cq;
                        if sync: wakeup sync receive interface;
                        if async: call async callback;

                        /* for each eqe, handle related cqs */
                        while (CQE_PHASE(cqe_h) == CQC_PHASE(cqc_current)) {
                                /* ? */
                                dma_rmb();

                                /* handle each cqe */
                                qm_info->sqe_handler(qm_info, cqe_h);
                                /* irq error report */

                                cqe_h += QM_CQE_SIZE;
                                cq_index++;
                        }

                        /* update cached cqc head index */
                        cqc_current->cq_head = cq_index;
                        hacc_db(qm_info, EQE_CQN(eqe_h), DOORBELL_CMD_CQ, cq_index, 0);
                        /* set c_flag */
                        hacc_db(qm_info, EQE_CQN(eqe_h), DOORBELL_CMD_CQ, cq_index, 1);

                if (qp->type == WD_QUEUE)
                        do not update cq;
                        wakeup

		eqe_h++;
		eq_index++;
	}

	/* update cached eqc head index */
        eqc->eq_head = eq_index;
	hacc_db(qm_info, 0, DOORBELL_CMD_EQ, eq_index, 0);

	return IRQ_HANDLED;
}

/* v1 qm hw ops */
static int vft_config_v1(struct qm_info *qm, u32 base, u32 number)
{
	/* fix: init qm memory */
	hisi_zip_write(hisi_zip, 0x1, QM_MEM_START_INIT);
	hisi_zip_check(hisi_zip, QM_MEM_INIT_DONE, 0);

	/* fix: init sq number for each pf and vf */
	hisi_zip_check(hisi_zip, QM_VFT_CFG_RDY, 0);
	for (i = 0; i <= 0; i++) {	
		hisi_zip_write(hisi_zip, 0x0, QM_VFT_CFG_OP_WR);
		hisi_zip_write(hisi_zip, QM_SQC_VFT, QM_VFT_CFG_TYPE);
		hisi_zip_write(hisi_zip, i, QM_VFT_CFG_ADDRESS);
		
		/* 64(queus) x 32B(sqc size) = 2048B */
		tmp = QM_SQC_VFT_BUF_SIZE		|
		      QM_SQC_VFT_SQC_SIZE		|
		      QM_SQC_VFT_INDEX_NUMBER		|
		      i << QM_SQC_VFT_BT_INDEX_SHIFT	|
		      QM_SQC_VFT_VALID			|
		      i * HZIP_FUN_QUEUE_NUM << QM_SQC_VFT_START_SQN_SHIFT;


		hisi_zip_write(hisi_zip, tmp & 0xffffffff, QM_VFT_CFG_DATA_L);
		hisi_zip_write(hisi_zip, tmp >> 32, QM_VFT_CFG_DATA_H);

		hisi_zip_write(hisi_zip, 0x0, QM_VFT_CFG_RDY);
		hisi_zip_write(hisi_zip, 0x1, QM_VFT_CFG_OP_ENABLE);
		hisi_zip_check(hisi_zip, QM_VFT_CFG_RDY, 0);
	}

	tmp = 0;

	/* fix: init cq number for each pf and vf */
	for (i = 0; i <= 0; i++) {	
		hisi_zip_write(hisi_zip, 0x0, QM_VFT_CFG_OP_WR);
		hisi_zip_write(hisi_zip, QM_CQC_VFT, QM_VFT_CFG_TYPE);
		hisi_zip_write(hisi_zip, i, QM_VFT_CFG_ADDRESS);

		tmp = QM_CQC_VFT_BUF_SIZE		|
		      QM_CQC_VFT_SQC_SIZE		|
		      QM_CQC_VFT_INDEX_NUMBER		|
		      i << QM_CQC_VFT_BT_INDEX_SHIFT	|
		      QM_CQC_VFT_VALID;


		hisi_zip_write(hisi_zip, tmp & 0xffffffff, QM_VFT_CFG_DATA_L);
		hisi_zip_write(hisi_zip, tmp >> 32, QM_VFT_CFG_DATA_H);

		hisi_zip_write(hisi_zip, 0x0, QM_VFT_CFG_RDY);
		hisi_zip_write(hisi_zip, 0x1, QM_VFT_CFG_OP_ENABLE);
		hisi_zip_check(hisi_zip, QM_VFT_CFG_RDY, 0);
	}

	return 0;
}

struct hisi_acc_qm_hw_ops qm_hw_ops_v1 = {
        .vft_config = vft_config_v1,
};

/* v2 qm hw ops */
static int alloc_aeqc_v2() {} /* v1 = NULL */
static int vft_config_v2(struct qm_info *qm, u32 base, u32 number) {}

struct hisi_acc_qm_hw_ops qm_hw_ops_v2 = {
        .vft_config = vft_config_v2,
};


int hisi_acc_qm_info_create(struct device *dev, void __iomem *base,
                            struct hisi_acc_qm_hw_ops *ops, u32 number,
                            struct qm_info **res)
{
        struct qm_info *qm;
        size_t size;
        int ret;

        qm = (struct qm_info *)devm_kzalloc(dev, sizeof(*qm), GFP_KERNEL);
	if (!qm) {
		ret = -ENOME;
                goto err_out;
        }
        
        qm->fun_base = base;
        qm->fun_num = number;
        qm->eq_head = 0;
        qm->node_id = dev->numa_node;
        qm->ops = ops;
        qm->dev = dev;
        spin_lock_init(&qm->mailbox_lock);

        size = max_t(size_t, sizeof(struct eqc), sizeof(struct aeqc));
	qm->eqc_aeqc_pool = dma_pool_create("eqc_aeqc", dev, size, 32, 0);
        if (!qm->eqc_aeqc_pool) {
		ret = -ENOME;
                goto err_out;
        }

	qm->eqc = dma_pool_alloc(qm->eqc_aeqc_pool, GFP_ATOMIC, &qm->eqc_dma);
	if (!qm->eqc) {
		ret = -ENOME;
                return err_out;
        }

        size = sizeof(struct eq) * QM_Q_DEPTH;
	qm->eq_base = dma_alloc_coherent(dev, size, &qm->eq_base_dma,
                                         GFP_KERNEL);
	if (!qm->eq_base) {
		ret = -ENOME;
                goto err_eq;
        }

        qm->eqc->eq_base_l = lower_32_bits(qm->eq_base_dma);
        qm->eqc->eq_base_h = upper_32_bits(qm->eq_base_dma);
        qm->eqc->dw3 = 2 << MB_EQC_EQE_SHIFT;
        qm->eqc->dw6 = (QM_Q_DEPTH - 1) | (1 << MB_EQC_PHASE_SHIFT);

	hacc_mb(qm, MAILBOX_CMD_EQC, qm->eqc_dma, QM_Q_DEPTH, 0, 0);

        /* fix me: qm->alloc_aeqc(); */

        *res = qm;
	return 0;
err_eq:
	dma_pool_free(qm->eqc_aeqc_pool, qm->eqc, qm->eqc_dma);
err_out:
        return ret;
}

/* only can be called in PF */
int hisi_acc_qm_info_add_queue(struct qm_info *qm, u32 base, u32 number)
{
        int ret;
        size_t size;

        if (!number)
                return -EINVAL;

        if (qm->qp_bitmap)
                kfree(qm->qp_bitmap);

        size = BITS_TO_LONGS(number) * sizeof(long);
	qm->qp_bitmap = (unsigned long *)kzalloc(size, GFP_KERNEL);

        qm->qp_base = base;
        qm->num = number;

        ret = qm->ops->vft_config(qm, base, number);
        if (ret)
		goto err_vft_config;
        
	/* Init sqc_bt */
        size = sizeof(struct sqc) * number;
	qm->sqc_base = dma_alloc_coherent(qm->dev, size, &qm->sqc_base_dma,
                                          GFP_KERNEL);
	if (!qm->sqc_base) {
		ret = -ENOMEM;
                goto err_vft_config;
        }
	hacc_mb(qm, MAILBOX_CMD_SQC_BT, qm->sqc_base_dma, 0, 0, 0);

	/* Init cqc_bt */
        size = sizeof(struct cqc) * number;
	qm->cqc_base = dma_alloc_coherent(qm->dev, size, &qm->cqc_base_dma,
                                          GFP_KERNEL);
	if (!qm->cqc_base) {
		ret = -ENOMEM;
                goto err_cqc;
        }
	hacc_mb(qm, MAILBOX_CMD_SQC_BT, qm->cqc_base_dma, 0, 0, 0);

        return 0;
err_cqc:
	dma_free_coherent(qm->dev, sizeof(struct sqc) * number, qm->sqc_base,
                          qm->sqc_base_dma);
err_vft_config:
        kfree(qm->qp_bitmap);
        return ret;
}

void hisi_acc_qm_info_release(struct qm_info *qm)
{
	dma_pool_free(qm->eqc_aeqc_pool, qm->eqc, qm->eqc_dma);
	dma_free_coherent(qm->dev, sizeof(struct eqe) * QM_Q_DEPTH, qm->eq_base,
                          qm->eq_base_dma);

	dma_pool_destroy(qm->eqc_aeqc_pool);

        if (qm->qp_bitmap)
                kfree(qm->qp_bitmap);
        if (qm->sqc_base)
	        dma_free_coherent(qm->dev, sizeof(struct sqc) * QM_Q_DEPTH,
                                  qm->sqc_base, qm->sqc_base_dma);
        if (qm->cqc_base)
	        dma_free_coherent(qm->dev, sizeof(struct cqc) * QM_Q_DEPTH,
                                  qm->cqc_base, qm->cqc_base_dma);
}

int hisi_acc_create_qp(struct qm_info *qm, struct hisi_acc_qp **res,
                       u32 sqe_size)
{
        struct hisi_acc_qp *qp;
        struct sqc *sqc;
        struct cqc *cqc;
        struct cqe *cq_base;
        void *sq_base;
        int qp_index;
        size_t size;

	spin_lock(&qm->qp_bitmap_lock);
        qp_index = find_first_zero_bit(qm->qp_bitmap, qm->qp_num);
        set_bits(qp_index, qm->qp_bitmap);
    	spin_unlock(&qm->qp_bitmap_lock);

        qp = (struct hisi_acc_qp *)kzalloc(sizeof(*qp), GFP_KERNEL);
	if (!qp)
		return -ENOMEM;
        qp->queue_id = qp_index;
        qp->sq_tail = 0;
        qp->cq_head = 0;
        qp->sqe_size = sqe_size;
        qp->parent = qm;

        sqc = qm->sqc_base + qp_index;
        qp->sqc = sqc;
       
        size = sqe_size * QM_Q_DEPTH;
	sq_base = dma_alloc_coherent(qm->dev, size, &qm->sq_base_dma,
                                     GFP_KERNEL);
	if (!sq_base)
		return -ENOMEM;

        sqc->sq_head = 0;
        sqc->sq_tail = 0;
        sqc->sq_base_l = lower_32_bits(qm->sq_base_dma);
        sqc->sq_base_h = upper_32_bits(qm->sq_base_dma);
        sqc->dw3 = (0 << SQ_HOP_NUM_SHIFT)      |
                   (0 << SQ_PAGE_SIZE_SHIFT)    | 
                   (0 << SQ_BUF_SIZE_SHIFT)     | 
                   (7 << SQ_SQE_SIZE_SHIFT);
        sqc->qes = SQ_DEPTH - 1;
        sqc->pasid = 0;
        sqc->w11 = 0; /* fix me */
        sqc->cq_num = qp_index;
        sqc->w13 = 0 << SQ_PRIORITY_SHIFT	|
		   1 << SQ_ORDERS_SHIFT		|
		   alg_type << SQ_TYPE_SHIFT;
        sqc->rsvd1 = 0;

	hacc_mb(qm, MAILBOX_CMD_SQC, virt_to_phys(sqc), qp_index, 0, 0);
        qp->sq_base = sq_base;

        cqc = qm->cqc_base + qp_index;
        qp->cqc = cqc;

        size = sizeof(struct cqe) * QM_Q_DEPTH;
	cq_base = dma_alloc_coherent(qm->dev, size, &qm->cq_base_dma,
                                     GFP_KERNEL);
	if (!cq_base)
		return -ENOMEM;

        cqc->cq_head = 0;
        cqc->cq_tail = 0;
        cqc->cq_base_l = lower_32_bits(virt_to_phys(vadd));
        cqc->cq_base_h = upper_32_bits(virt_to_phys(vadd));
        cqc->dw3 = (0 << CQ_HOP_NUM_SHIFT)      |
                   (0 << CQ_PAGE_SIZE_SHIFT)    | 
                   (0 << CQ_BUF_SIZE_SHIFT)     | 
                   (4 << CQ_SQE_SIZE_SHIFT);
        cqc->qes = CQ_DEPTH - 1;
        cqc->pasid = 0;
        cqc->w11 = 0; /* fix me */
        cqc->dw6 = 1 << CQ_PHASE_SHIFT | 1 << CQ_FLAG_SHIFT;
        cqc->rsvd1 = 0;

	hacc_mb(hisi_zip->qm_info, MAILBOX_CMD_CQC, virt_to_phys(cqc),
		q_num, 0, 0);

        qp->cq_base = cq_base;

        *res = qp;

        return 0;
}

int hisi_acc_release_qp(struct qm_info *qm, struct hisi_acc_qp *qp)
{
	dma_free_coherent(qm->dev, qp->sqe_size * QM_Q_DEPTH, qp->sq_base,
                          qp->sq_base_dma);
	dma_free_coherent(qm->dev, sizeof(struct cqe) * QM_Q_DEPTH, qp->cq_base,
                          qp->cq_base_dma);

	spin_lock(&qm->qp_bitmap_lock);
        bitmap_clear(qm->qp_bitmap, qp->queue_id, 1);
    	spin_unlock(&qm->qp_bitmap_lock);

        kfree(qp);

        return 0;
}

int hisi_acc_set_pasid(struct hisi_acc_qp *qp, u16 pasid)
{
        qp->sqc->pasid = pasid;

	hacc_mb(qp->parent, MAILBOX_CMD_SQC, virt_to_phys(qp->sqc),
                qp->queue_id, 0, 0);

        return 0;
}

int hisi_acc_unset_pasid(struct hisi_acc_qp *qp)
{
        qp->sqc->pasid = 0;

	hacc_mb(qp->parent, MAILBOX_CMD_SQC, virt_to_phys(qp->sqc),
                qp->queue_id, 0, 0);

        return 0;
}

u16 hisi_acc_get_sq_tail(struct hisi_acc_qp *qp)
{
        return qp->sq_tail;
}

/* fix me */
int hisi_acc_send(struct hisi_acc_qp *qp, u16 sq_tail, void *priv)
{
        hacc_db(qp->parent, qp->qp_num, DOORBELL_CMD_SQ, sq_tail, 0);

        qp->sq_tail++; /* fix me: wrap */

        return 0;
}

/* fix me */
int hisi_acc_receive(struct hisi_acc_qp *qp, void *priv)
{
        return 0;
}

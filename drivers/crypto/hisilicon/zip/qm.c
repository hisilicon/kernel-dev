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
#include <linux/bitmap.h>
#include <linux/dmapool.h>
#include <linux/dma-mapping.h>
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
int hacc_mb(struct qm_info *qm, u8 cmd, u64 phys_addr, u16 queue,
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
int hacc_db(struct qm_info *qm, u16 qn, u8 cmd, u16 index, u8 priority)
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

static inline struct hisi_acc_qp *to_hisi_acc_qp(struct qm_info *qm,
                                                    struct eqe *eqe)
{
        u16 cqn = eqe->dw0 & QM_EQE_CQN_MASK;

        return qm->qp_array[cqn];
}

static inline struct cqe *to_current_cqe(struct hisi_acc_qp *qp)
{
       return qp->cq_base + qp->cq_head; 
}

irqreturn_t hacc_irq_thread(int irq, void *data)
{
	struct qm_info *qm = (struct qm_info *)data;
	struct eqe *eqe = qm->eq_base + qm->eq_head;
        struct eqc *eqc = qm->eqc;
        struct hisi_acc_qp *qp;
        struct cqe *cqe;

	/* to do: if no new eqe, there is no irq, do nothing or reture error */

	while (EQE_PHASE(eqe) == EQC_PHASE(eqc)) {

                qp = to_hisi_acc_qp(qm, eqe);

                if (qp->type == CRYPTO_QUEUE) {

                        cqe = to_current_cqe(qp);

                        while (CQE_PHASE(cqe) == CQC_PHASE(qp->cqc)) {
                                /* ? */
                                dma_rmb();

                                /* crypto sync interface: wakeup */

                                /* crypto async interface: callback */
                                /* handle each cqe */
                                qp->sqe_handler(qp, cqe);

                                if (qp->cq_head == QM_Q_DEPTH - 1) {
                                        qp->cqc->dw6 = qp->cqc->dw6 ^ CQC_PHASE_BIT;
                                        cqe = qp->cq_base;
                                        qp->cq_head = 0;
                                } else {
                                        cqe++;
                                        qp->cq_head++;
                                }
                        }

                        hacc_db(qm, qp->queue_id, DOORBELL_CMD_CQ, qp->cq_head, 0);
                        /* set c_flag */
                        hacc_db(qm, qp->queue_id, DOORBELL_CMD_CQ, qp->cq_head, 1);
                }

                if (qp->type == WD_QUEUE) {
                        /* wd sync interface: if cq_head finished, wakeup;
                         *                    update cq head which is used both
                         *                    in user space and kernel. 
                         */
                }

                if (qm->eq_head == QM_Q_DEPTH - 1) {
                        eqc->dw6 = eqc->dw6 ^ EQC_PHASE_BIT;
                        eqe = qm->eq_base;
                        qm->eq_head = 0;
                } else {
                        eqe++;
		        qm->eq_head++;
                }
	}

	hacc_db(qm, 0, DOORBELL_CMD_EQ, qm->eq_head, 0);

	return IRQ_HANDLED;
}

/* check if bit in regs is 1 */
static inline void hisi_acc_check(struct qm_info *qm, u32 offset, u32 bit)
{
	int val;

	do {
                val = readl(qm->fun_base + offset);
	} while ((BIT(bit) & val) == 0);
}

/* init qm memory will erase configure in vft */
int hisi_acc_init_qm_mem(struct qm_info *qm)
{
	writel(0x1, qm->fun_base + QM_MEM_START_INIT);
        hisi_acc_check(qm, QM_MEM_INIT_DONE, 0);

        return 0;
}

/* v1 qm hw ops */
/* before call this at first time, please call hisi_acc_init_qm_mem */
static int vft_config_v1(struct qm_info *qm, u16 base, u32 number)
{
        u64 tmp;

        hisi_acc_check(qm, QM_VFT_CFG_RDY, 0);

	writel(0x0, qm->fun_base + QM_VFT_CFG_OP_WR);
	writel(QM_SQC_VFT, qm->fun_base + QM_VFT_CFG_TYPE);
	writel(qm->fun_num, qm->fun_base + QM_VFT_CFG_ADDRESS);

	tmp = QM_SQC_VFT_SQC_SIZE		        |
	      QM_SQC_VFT_INDEX_NUMBER		        |
	      QM_SQC_VFT_VALID			        |
              (u64)base << QM_SQC_VFT_START_SQN_SHIFT;

	writel(tmp & 0xffffffff, qm->fun_base + QM_VFT_CFG_DATA_L);
	writel(tmp >> 32, qm->fun_base + QM_VFT_CFG_DATA_H);

	writel(0x0, qm->fun_base + QM_VFT_CFG_RDY);
	writel(0x1, qm->fun_base + QM_VFT_CFG_OP_ENABLE);
        hisi_acc_check(qm, QM_VFT_CFG_RDY, 0);

	tmp = 0;

	writel(0x0, qm->fun_base + QM_VFT_CFG_OP_WR);
	writel(QM_CQC_VFT, qm->fun_base + QM_VFT_CFG_TYPE);
	writel(qm->fun_num, qm->fun_base + QM_VFT_CFG_ADDRESS);

	tmp = QM_CQC_VFT_SQC_SIZE		        |
	      QM_CQC_VFT_INDEX_NUMBER		        |
	      QM_CQC_VFT_VALID;

	writel(tmp & 0xffffffff, qm->fun_base + QM_VFT_CFG_DATA_L);
	writel(tmp >> 32, qm->fun_base + QM_VFT_CFG_DATA_H);

	writel(0x0, qm->fun_base + QM_VFT_CFG_RDY);
	writel(0x1, qm->fun_base + QM_VFT_CFG_OP_ENABLE);
        hisi_acc_check(qm, QM_VFT_CFG_RDY, 0);

	return 0;
}

struct hisi_acc_qm_hw_ops qm_hw_ops_v1 = {
        .vft_config = vft_config_v1,
        .aeq_config = NULL,
};

/* v2 qm hw ops */
static int aeq_config_v2(struct qm_info *qm) {return 0;} /* v1 = NULL */
static int vft_config_v2(struct qm_info *qm, u16 base, u32 number) {return 0;}

struct hisi_acc_qm_hw_ops qm_hw_ops_v2 = {
        .vft_config = vft_config_v2,
        .aeq_config = aeq_config_v2,
};

int hisi_acc_qm_info_create(struct device *dev, void __iomem *base, u32 number,
                            struct hisi_acc_qm_hw_ops *ops,
                            struct qm_info **res)
{
        struct qm_info *qm;
        size_t size;

        qm = (struct qm_info *)devm_kzalloc(dev, sizeof(*qm), GFP_KERNEL);
	if (!qm)
                goto err_out;
        
        qm->fun_base = base;
        qm->fun_num = number;
        qm->eq_head = 0;
        qm->node_id = dev->numa_node;
        qm->ops = ops;
        qm->dev = dev;
        spin_lock_init(&qm->mailbox_lock);

        size = max_t(size_t, sizeof(struct eqc), sizeof(struct aeqc));
	qm->eqc_aeqc_pool = dma_pool_create("eqc_aeqc", dev, size, 32, 0);
        if (!qm->eqc_aeqc_pool)
                goto err_out;

	qm->eqc = dma_pool_alloc(qm->eqc_aeqc_pool, GFP_ATOMIC, &qm->eqc_dma);
	if (!qm->eqc)
                goto err_out;

        size = sizeof(struct eqe) * QM_Q_DEPTH;
	qm->eq_base = dma_alloc_coherent(dev, size, &qm->eq_base_dma,
                                         GFP_KERNEL);
	if (!qm->eq_base)
                goto err_eq;

        qm->eqc->eq_base_l = lower_32_bits(qm->eq_base_dma);
        qm->eqc->eq_base_h = upper_32_bits(qm->eq_base_dma);
        qm->eqc->dw3 = 2 << MB_EQC_EQE_SHIFT;
        qm->eqc->dw6 = (QM_Q_DEPTH - 1) | (1 << MB_EQC_PHASE_SHIFT);

        /* to check */
	hacc_mb(qm, MAILBOX_CMD_EQC, qm->eqc_dma, QM_Q_DEPTH, 0, 0);

        /* fix me: qm->alloc_aeqc(); */

        *res = qm;
	return 0;
err_eq:
	dma_pool_free(qm->eqc_aeqc_pool, qm->eqc, qm->eqc_dma);
err_out:
        return -ENOMEM;
}

/* only can be called in PF */
int hisi_acc_qm_info_add_queue(struct qm_info *qm, u32 base, u32 number)
{
        size_t size;
        int ret;

        if (!number)
                return -EINVAL;

        if (qm->qp_bitmap && qm->qp_array) {
                kfree(qm->qp_bitmap);
                kfree(qm->qp_array);
        }

        size = BITS_TO_LONGS(number) * sizeof(long);
	qm->qp_bitmap = (unsigned long *)kzalloc(size, GFP_KERNEL);
        if (!qm->qp_bitmap) 
                goto err_bitmap;

        size = number * sizeof(struct hisi_acc_qp *);
        qm->qp_array = (struct hisi_acc_qp **)kzalloc(size, GFP_KERNEL);
        if (!qm->qp_array)
                goto err_qp_array;

        qm->qp_base = base;
        qm->qp_num = number;

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
        kfree(qm->qp_array);
err_qp_array:
        kfree(qm->qp_bitmap);
err_bitmap:
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
        if (qm->qp_array)
                kfree(qm->qp_array);
        if (qm->sqc_base)
	        dma_free_coherent(qm->dev, sizeof(struct sqc) * QM_Q_DEPTH,
                                  qm->sqc_base, qm->sqc_base_dma);
        if (qm->cqc_base)
	        dma_free_coherent(qm->dev, sizeof(struct cqc) * QM_Q_DEPTH,
                                  qm->cqc_base, qm->cqc_base_dma);
}

int hisi_acc_create_qp(struct qm_info *qm, struct hisi_acc_qp **res,
                       u32 sqe_size, u8 alg_type)
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
        set_bit(qp_index, qm->qp_bitmap);
    	spin_unlock(&qm->qp_bitmap_lock);

        qp = (struct hisi_acc_qp *)kzalloc(sizeof(*qp), GFP_KERNEL);
	if (!qp)
                goto err_qp;

        qp->queue_id = qp_index;
        qp->sq_tail = 0;
        qp->cq_head = 0;
        qp->sqe_size = sqe_size;
        qp->parent = qm;

        sqc = qm->sqc_base + qp_index;
        qp->sqc = sqc;
       
        size = sqe_size * QM_Q_DEPTH;
	sq_base = dma_alloc_coherent(qm->dev, size, &qp->sq_base_dma,
                                     GFP_KERNEL);
	if (!sq_base)
                goto err_sq_base;
        qp->sq_base = sq_base;

        sqc->sq_head = 0;
        sqc->sq_tail = 0;
        sqc->sq_base_l = lower_32_bits(qp->sq_base_dma);
        sqc->sq_base_h = upper_32_bits(qp->sq_base_dma);
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
		   (alg_type & SQ_TYPE_MASK) << SQ_TYPE_SHIFT;
        sqc->rsvd1 = 0;

	hacc_mb(qm, MAILBOX_CMD_SQC, virt_to_phys(sqc), qp_index, 0, 0);

        cqc = qm->cqc_base + qp_index;
        qp->cqc = cqc;

        size = sizeof(struct cqe) * QM_Q_DEPTH;
	cq_base = dma_alloc_coherent(qm->dev, size, &qp->cq_base_dma,
                                     GFP_KERNEL);
	if (!cq_base)
                goto err_cq_base;
        qp->cq_base = cq_base;

        cqc->cq_head = 0;
        cqc->cq_tail = 0;
        cqc->cq_base_l = lower_32_bits(qp->cq_base_dma);
        cqc->cq_base_h = upper_32_bits(qp->cq_base_dma);
        cqc->dw3 = (0 << CQ_HOP_NUM_SHIFT)      |
                   (0 << CQ_PAGE_SIZE_SHIFT)    | 
                   (0 << CQ_BUF_SIZE_SHIFT)     | 
                   (4 << CQ_SQE_SIZE_SHIFT);
        cqc->qes = CQ_DEPTH - 1;
        cqc->pasid = 0;
        cqc->w11 = 0; /* fix me */
        cqc->dw6 = 1 << CQ_PHASE_SHIFT | 1 << CQ_FLAG_SHIFT;
        cqc->rsvd1 = 0;

	hacc_mb(qm, MAILBOX_CMD_CQC, virt_to_phys(cqc), qp_index, 0, 0);

        qm->qp_array[qp_index] = qp;
        *res = qp;
        
        return 0;
err_cq_base:
	dma_free_coherent(qm->dev, qp->sqe_size * QM_Q_DEPTH, qp->sq_base,
                          qp->sq_base_dma);
err_sq_base:
        kfree(qp);
err_qp:
	return -ENOMEM;
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

        /* to check */
	hacc_mb(qp->parent, MAILBOX_CMD_SQC, virt_to_phys(qp->sqc),
                qp->queue_id, 0, 0);

        return 0;
}

int hisi_acc_unset_pasid(struct hisi_acc_qp *qp)
{
        qp->sqc->pasid = 0;

        /* to check */
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
        hacc_db(qp->parent, qp->queue_id, DOORBELL_CMD_SQ, sq_tail, 0);

        qp->sq_tail++; /* fix me: wrap */

        return 0;
}

/* fix me */
int hisi_acc_receive(struct hisi_acc_qp *qp, void *priv)
{
        return 0;
}

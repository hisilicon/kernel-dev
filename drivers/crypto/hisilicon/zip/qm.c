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
int hacc_mb(struct qm_info *qm, u8 cmd, u64 phys_addr, u16 queue, bool op,
	    bool event)
{
	u32 mailbox[4] = {0};

	/* to do: prepare mb date */
	mailbox[0] = cmd |
		     (event ? 0x1 << MAILBOX_EVENT_SHIFT : 0) |
		     (op ? 0x1 << MAILBOX_OP_SHIFT : 0) |
		     (queue << MAILBOX_QUEUE_SHIFT) |
		     (0x1 << MAILBOX_BUSY_SHIFT);
	mailbox[1] = lower_32_bits(phys_addr);
	mailbox[2] = upper_32_bits(phys_addr);
	mailbox[3] = 0;

	pr_err("in %s\n", __FUNCTION__);
	pr_err("in %x\n", mailbox[0]);
	pr_err("in %x\n", mailbox[1]);
	pr_err("in %x\n", mailbox[2]);
	pr_err("in %x\n", mailbox[3]);

	spin_lock(&qm->mailbox_lock);

	while(hacc_qm_mb_is_busy(qm));
	mb_write(qm, mailbox);
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
/* we may remove db to user space */
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

irqreturn_t hacc_irq_thread(int irq, void *data)
{
	//pr_err("in %s\n", __FUNCTION__);

	struct qm_info *qm_info = (struct qm_info *)data;
	struct eqc *eqc = qm_info->eqc;
	struct cqc *cqc = qm_info->cqc_base;
	struct eqe *eq = qm_info->eq_base;
	struct eqe *eqe_h = eq + EQC_HEAD_INDEX(eqc);
	char *cqe_h, *cq;
        struct cqc *cqc_current;
	u16 eq_index = EQC_HEAD_INDEX(eqc);
	u16 cq_index;

	/* to do: if no new eqe, there is no irq, do nothing or reture error */

	/* handle all eqs from eqe_h */
	while (EQE_PHASE(eqe_h) == EQC_PHASE(eqc)) {

		cqc_current = cqc + EQE_CQN(eqe_h);
		cq = phys_to_virt(CQC_CQ_ADDRESS(cqc_current));
		cq_index = CQC_HEAD_INDEX(cqc_current); /* fix me: wrong here */
		cqe_h = cq + cq_index * QM_CQE_SIZE;

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

		eqe_h++;
		eq_index++;
	}

	/* update cached eqc head index */
        eqc->eq_head = eq_index;
	hacc_db(qm_info, 0, DOORBELL_CMD_EQ, eq_index, 0);

	return IRQ_HANDLED;
}

struct qm_info *hisi_acc_qm_info_create(struct hisi_acc_qm_hw_ops *ops)
{
        return NULL;
}

int hisi_acc_qm_info_add_queue(struct qm_info *qm, u32 base, u32 number)
{
        return 0;
}

void hisi_acc_qm_info_release(struct qm_info *qm)
{

}

int hisi_acc_create_qp(struct qm_info *qm, struct hisi_acc_qp *qp)
{
        return 0;
}

int hisi_acc_release_qp(struct qm_info *qm, struct hisi_acc_qp *qp)
{
        return 0;
}

int hisi_acc_set_pasid(struct hisi_acc_qp *qp, u16 pasid)
{
        return 0;
}

int hisi_acc_unset_pasid(struct hisi_acc_qp *qp)
{
        return 0;
}

int hisi_acc_get_sq_tail(struct hisi_acc_qp *qp)
{
        return 0;
}

int hisi_acc_send(struct hisi_acc_qp *qp, u16 sq_tail, void *priv)
{
        return 0;
}

int hisi_acc_receive(struct hisi_acc_qp *qp, void *priv)
{
        return 0;
}

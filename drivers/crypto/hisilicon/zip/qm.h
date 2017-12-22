#ifndef HISI_ACC_QM_H
#define HISI_ACC_QM_H

#include <linux/init.h>
#include <linux/irqreturn.h>
#include <linux/kernel.h>

struct sqc {
	u32 content[8];
} __attribute__ ((aligned(32)));

struct cqc {
	u32 content[8];
} __attribute__ ((aligned(32)));

struct eqc {
	u32 content[8];
} __attribute__ ((aligned(32)));

struct qm_info {
	void __iomem * fun_base;

	u32 sqn_base;
	u32 sqn_num;
	u32 cqn_base;
	u32 cqn_num;

	char *sqc_cache;
	char *cqc_cache;
	char *eqc_cache;
	char *eq;
	char *aeqc_cache;
	char *aeq;

	/* sqc mb can be triggered in create function from user space */
	spinlock_t mailbox_lock;

	void *priv;

	int (*sqe_handler)(struct qm_info *qm_info, char *cqe);
};

#define QM_SQC_SIZE			32
#define QM_CQC_SIZE			32
#define QM_CQE_SIZE			16
#define QM_EQC_SIZE			28
#define QM_EQE_SIZE			4
#define QM_AEQC_SIZE			32
#define QM_AEQE_SIZE			12

/* qm 0x0000: mailbox, doorbell, irq */
/* eq/aeq irq enable */
#define QM_VF_AEQ_INT_SOURCE		0x0
#define QM_VF_AEQ_INT_MASK		0x4
#define QM_VF_EQ_INT_SOURCE		0x8
#define QM_VF_EQ_INT_MASK		0xc

/* mailbox */
#define MAILBOX_CMD_SQC			0x0
#define MAILBOX_CMD_CQC			0x1
#define MAILBOX_CMD_EQC			0x2
#define MAILBOX_CMD_AEQC		0x3
#define MAILBOX_CMD_SQC_BT		0x4
#define MAILBOX_CMD_CQC_BT		0x5

#define MAILBOX_CMD_SEND_BASE		0x300
#define MAILBOX_EVENT_SHIFT		8
#define MAILBOX_STATUS_SHIFT		9
#define MAILBOX_BUSY_SHIFT		13
#define MAILBOX_OP_SHIFT		14
#define MAILBOX_QUEUE_SHIFT		16

/* sqc shift */
#define SQ_HEAD_SHIFT			0
#define SQ_TAIL_SHIFI			16
#define SQ_HOP_NUM_SHIFT		0
#define SQ_PAGE_SIZE_SHIFT		4
#define SQ_BUF_SIZE_SHIFT		8
#define SQ_SQE_SIZE_SHIFT		12
#define SQ_DEPTH			1024
#define SQ_PASID			0 //
#define SQ_HEAD_IDX_SIG_SHIFT		0 //
#define SQ_TAIL_IDX_SIG_SHIFT		0 //
#define SQ_CQN_SHIFT			0 //
#define SQ_PRIORITY_SHIFT		16//
#define SQ_ORDERS_SHIFT			20//
#define SQ_TYPE_SHIFT			24// #

/* cqc shift */
#define CQ_HEAD_SHIFT			0
#define CQ_TAIL_SHIFI			16
#define CQ_HOP_NUM_SHIFT		0
#define CQ_PAGE_SIZE_SHIFT		4
#define CQ_BUF_SIZE_SHIFT		8
#define CQ_SQE_SIZE_SHIFT		12
#define CQ_DEPTH			1024
#define CQ_PASID			0 //
#define CQ_HEAD_IDX_SIG_SHIFT		0 //
#define CQ_TAIL_IDX_SIG_SHIFT		0 //
#define CQ_CQN_SHIFT			0 //
#define CQ_PRIORITY_SHIFT		16//
#define CQ_ORDERS_SHIFT			0 //
#define CQ_TYPE_SHIFT			0

#define CQ_PHASE_SHIFT			0
#define CQ_FLAG_SHIFT			1

#define CQC_HEAD_INDEX(cqc)		((*(u32 *)(cqc)) & 0xffff)
#define CQC_PHASE(cqc)			((*((u32 *)(cqc) + 6)) & 0x1)
#define CQC_CQ_ADDRESS(cqc)		(((u64)(*((u32 *)(cqc) + 2)) << 32) |    \
					 (*((u32 *)(cqc) + 1)))

/* cqe shift */
#define CQE_PHASE(cq)			(((*((u32 *)(cq) + 3)) >> 16) & 0x1)
#define CQE_SQ_NUM(cq)			((*((u32 *)(cq) + 2)) >> 16)
#define CQE_SQ_HEAD_INDEX(cq)		((*((u32 *)(cq) + 2)) & 0xffff)


/* eqc shift */
#define MB_EQC_EQE_SHIFT		12
#define MB_EQC_PHASE_SHIFT		16

#define EQC_HEAD_INDEX(eqc) 		((*(u32 *)(eqc)) & 0xffff)
#define EQC_TAIL_INDEX(eqc) 		((*(u32 *)(eqc)) >> 16)
#define EQC_PHASE(eqc)			((*((u32 *)(eqc) + 6)) >> 16 & 0x1)

/* eqe shift */

#define EQE_PHASE(eq)			((*(u32 *)(eq)) >> 16 & 0x1)
#define EQE_CQN(eq)			((*(u32 *)(eq)) & 0xffff)

/* aeqc shift */
#define MB_AEQC_AEQE_SHIFT		12
#define MB_AEQC_PHASE_SHIFT		16

/* aeqe shift */

int hacc_qm_mb_check_busy(struct qm_info *qm);
int hacc_mb(struct qm_info *qm, u8 cmd, u64 phys_addr, u16 num, bool rw, bool event);

/* doorbell */
#define DOORBELL_CMD_SQ			0
#define DOORBELL_CMD_CQ			1
#define DOORBELL_CMD_EQ			2
#define DOORBELL_CMD_AEQ		3

#define DOORBELL_CMD_SEND_BASE		0x340

/* user space interface */
struct hacc_qm_db {
	__u16 tag;
	__u8 cmd;
	__u16 index;
	__u8 priority;
};

int hacc_db(struct qm_info *qm, u16 qn, u8 cmd, u16 index, u8 priority);

#define QM_EQ_DEPTH			1024
/* qm 0x100000: cfg registers */
#define QM_MEM_START_INIT		0x100040
#define QM_MEM_INIT_DONE		0x100044
#define QM_VFT_CFG_RDY			0x10006c
#define QM_VFT_CFG_OP_WR		0x100058
#define QM_VFT_CFG_TYPE			0x10005c
#define QM_SQC_VFT			0x0
#define QM_CQC_VFT			0x1
#define QM_VFT_CFG_ADDRESS		0x100060
#define QM_VFT_CFG_OP_ENABLE		0x100054

#define QM_VFT_CFG_DATA_L		0x100064
#define QM_VFT_CFG_DATA_H		0x100068
#define QM_SQC_VFT_BUF_SIZE		0x7 << 8
#define QM_SQC_VFT_SQC_SIZE		0x5 << 12
#define QM_SQC_VFT_INDEX_NUMBER		0x1 << 16
#define QM_SQC_VFT_BT_INDEX_SHIFT	22
#define QM_SQC_VFT_START_SQN_SHIFT	28
#define QM_SQC_VFT_VALID		(u64)0x1 << 44
#define QM_CQC_VFT_BUF_SIZE		0x7 << 8
#define QM_CQC_VFT_SQC_SIZE		0x5 << 12
#define QM_CQC_VFT_INDEX_NUMBER		0x1 << 16
#define QM_CQC_VFT_BT_INDEX_SHIFT	22
#define QM_CQC_VFT_VALID            	0x1 << 28

/* qm user domain */
#define QM_ARUSER_M_CFG_1		0x100088
#define QM_ARUSER_M_CFG_ENABLE		0x100090
#define QM_AWUSER_M_CFG_1		0x100098
#define QM_AWUSER_M_CFG_ENABLE		0x1000a0
#define QM_WUSER_M_CFG_ENABLE		0x1000a8
/* qm cache */
#define QM_AXI_M_CFG			0x1000ac
#define QM_AXI_M_CFG_ENABLE		0x1000b0
#define QM_PEH_AXUSER_CFG_ENABLE	0x1000d0


irqreturn_t hacc_irq_thread(int irq, void *data);

#endif

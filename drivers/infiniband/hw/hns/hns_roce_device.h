/*
 * Copyright (c) 2016 Hisilicon Limited.
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

#ifndef _HNS_ROCE_DEVICE_H
#define _HNS_ROCE_DEVICE_H

#include <rdma/ib_verbs.h>

#define DRV_NAME "hns_roce"

#define HNS_ROCE_BA_SIZE			(32 * 4096)

#define HNS_ROCE_MAX_IRQ_NUM			34

#define HNS_ROCE_COMP_VEC_NUM			32

#define HNS_ROCE_AEQE_VEC_NUM			1
#define HNS_ROCE_AEQE_OF_VEC_NUM		1

#define HNS_ROCE_MAX_PORTS			6

enum hns_roce_event {
	HNS_ROCE_EVENT_TYPE_PATH_MIG                  = 0x01,
	HNS_ROCE_EVENT_TYPE_PATH_MIG_FAILED           = 0x02,
	HNS_ROCE_EVENT_TYPE_COMM_EST                  = 0x03,
	HNS_ROCE_EVENT_TYPE_SQ_DRAINED                = 0x04,
	HNS_ROCE_EVENT_TYPE_WQ_CATAS_ERROR            = 0x05,
	HNS_ROCE_EVENT_TYPE_INV_REQ_LOCAL_WQ_ERROR    = 0x06,
	HNS_ROCE_EVENT_TYPE_LOCAL_WQ_ACCESS_ERROR     = 0x07,
	HNS_ROCE_EVENT_TYPE_SRQ_LIMIT_REACH           = 0x08,
	HNS_ROCE_EVENT_TYPE_SRQ_LAST_WQE_REACH        = 0x09,
	HNS_ROCE_EVENT_TYPE_SRQ_CATAS_ERROR           = 0x0a,
	HNS_ROCE_EVENT_TYPE_CQ_ACCESS_ERROR           = 0x0b,
	HNS_ROCE_EVENT_TYPE_CQ_OVERFLOW               = 0x0c,
	HNS_ROCE_EVENT_TYPE_CQ_ID_INVALID             = 0x0d,
	HNS_ROCE_EVENT_TYPE_PORT_CHANGE               = 0x0f,
	/* 0x10 and 0x11 is unused in currently application case */
	HNS_ROCE_EVENT_TYPE_DB_OVERFLOW               = 0x12,
	HNS_ROCE_EVENT_TYPE_MB                        = 0x13,
	HNS_ROCE_EVENT_TYPE_CEQ_OVERFLOW              = 0x14,
};

/* Local Work Queue Catastrophic Error,SUBTYPE 0x5 */
enum {
	HNS_ROCE_LWQCE_QPC_ERROR		= 1,
	HNS_ROCE_LWQCE_MTU_ERROR		= 2,
	HNS_ROCE_LWQCE_WQE_BA_ADDR_ERROR	= 3,
	HNS_ROCE_LWQCE_WQE_ADDR_ERROR		= 4,
	HNS_ROCE_LWQCE_SQ_WQE_SHIFT_ERROR	= 5,
	HNS_ROCE_LWQCE_SL_ERROR			= 6,
	HNS_ROCE_LWQCE_PORT_ERROR		= 7,
};

/* Local Access Violation Work Queue Error,SUBTYPE 0x7 */
enum {
	HNS_ROCE_LAVWQE_R_KEY_VIOLATION		= 1,
	HNS_ROCE_LAVWQE_LENGTH_ERROR		= 2,
	HNS_ROCE_LAVWQE_VA_ERROR		= 3,
	HNS_ROCE_LAVWQE_PD_ERROR		= 4,
	HNS_ROCE_LAVWQE_RW_ACC_ERROR		= 5,
	HNS_ROCE_LAVWQE_KEY_STATE_ERROR		= 6,
	HNS_ROCE_LAVWQE_MR_OPERATION_ERROR	= 7,
};

/* DOORBELL overflow subtype */
enum {
	HNS_ROCE_DB_SUBTYPE_SDB_OVF		= 1,
	HNS_ROCE_DB_SUBTYPE_SDB_ALM_OVF		= 2,
	HNS_ROCE_DB_SUBTYPE_ODB_OVF		= 3,
	HNS_ROCE_DB_SUBTYPE_ODB_ALM_OVF		= 4,
	HNS_ROCE_DB_SUBTYPE_SDB_ALM_EMP		= 5,
	HNS_ROCE_DB_SUBTYPE_ODB_ALM_EMP		= 6,
};

#define HNS_ROCE_CMD_SUCCESS			1

#define HNS_ROCE_PORT_DOWN			0
#define HNS_ROCE_PORT_UP			1

struct hns_roce_uar {
	u64		pfn;
	unsigned long	index;
};

struct hns_roce_bitmap {
	/* Bitmap Traversal last a bit which is 1 */
	unsigned long		last;
	unsigned long		top;
	unsigned long		max;
	unsigned long		reserved_top;
	unsigned long		mask;
	spinlock_t		lock;
	unsigned long		*table;
};

/* Order bitmap length -- bit num compute formula: 1 << (max_order - order) */
/* Order = 0: bitmap is biggest, order = max bitmap is least (only a bit) */
/* Every bit repesent to a partner free/used status in bitmap */
/*
* Initial, bits of other bitmap are all 0 except that a bit of max_order is 1
* Bit = 1 represent to idle and available; bit = 0: not available
*/
struct hns_roce_buddy {
	/* Members point to every order level bitmap */
	unsigned long **bits;
	/* Represent to avail bits of the order level bitmap */
	u32            *num_free;
	int             max_order;
	spinlock_t      lock;
};

/* For Hardware Entry Memory */
struct hns_roce_hem_table {
	/* HEM type: 0 = qpc, 1 = mtt, 2 = cqc, 3 = srq, 4 = other */
	u32		type;
	/* HEM array elment num */
	unsigned long	num_hem;
	/* HEM entry record obj total num */
	unsigned long	num_obj;
	/*Single obj size */
	unsigned long	obj_size;
	int		lowmem;
	struct mutex	mutex;
	struct hns_roce_hem **hem;
};

struct hns_roce_mr_table {
	struct hns_roce_bitmap		mtpt_bitmap;
	struct hns_roce_buddy		mtt_buddy;
	struct hns_roce_hem_table	mtt_table;
	struct hns_roce_hem_table	mtpt_table;
};

struct hns_roce_buf_list {
	void		*buf;
	dma_addr_t	map;
};

struct hns_roce_cq {
	void (*comp)(struct hns_roce_cq *);
	void (*event)(struct hns_roce_cq *, enum hns_roce_event);

	atomic_t			refcount;
	struct completion		free;
};

struct hns_roce_uar_table {
	struct hns_roce_bitmap bitmap;
};

struct hns_roce_qp_table {
	struct hns_roce_bitmap		bitmap;
	spinlock_t			lock;
	struct hns_roce_hem_table	qp_table;
	struct hns_roce_hem_table	irrl_table;
};

struct hns_roce_cq_table {
	struct hns_roce_bitmap		bitmap;
	spinlock_t			lock;
	struct radix_tree_root		tree;
	struct hns_roce_hem_table	table;
};

struct hns_roce_raq_table {
	struct hns_roce_buf_list	*e_raq_buf;
};

struct hns_roce_cmd_context {
	struct completion	done;
	int			result;
	int			next;
	u64			out_param;
	u16			token;
};

struct hns_roce_cmdq {
	struct dma_pool		*pool;
	u8 __iomem		*hcr;
	struct mutex		hcr_mutex;
	struct semaphore	poll_sem;
	/*
	* Event mode: cmd register mutex protection,
	* ensure to not exceed max_cmds and user use limit region
	*/
	struct semaphore	event_sem;
	int			max_cmds;
	spinlock_t		context_lock;
	int			free_head;
	struct hns_roce_cmd_context *context;
	/*
	* Result of get integer part
	* which max_comds compute according a power of 2
	*/
	u16			token_mask;
	/*
	* Process whether use event mode, init default non-zero
	* After the event queue of cmd event ready,
	* can switch into event mode
	* close device, switch into poll mode(non event mode)
	*/
	u8			use_events;
	u8			toggle;
};

struct hns_roce_dev;

struct hns_roce_qp {
	void			(*event)(struct hns_roce_qp *,
					 enum hns_roce_event);

	atomic_t		refcount;
	struct completion	free;
};

struct hns_roce_ib_iboe {
	struct net_device      *netdevs[HNS_ROCE_MAX_PORTS];
	u8			phy_port[HNS_ROCE_MAX_PORTS];
};

struct hns_roce_eq {
	struct hns_roce_dev		*hr_dev;
	void __iomem			*doorbell;

	int				type_flag;/* Aeq:1 ceq:0 */
	int				eqn;
	u32				entries;
	int				log_entries;
	int				eqe_size;
	int				irq;
	int				log_page_size;
	int				cons_index;
	struct hns_roce_buf_list	*buf_list;
};

struct hns_roce_eq_table {
	struct hns_roce_eq	*eq;
	void __iomem		**eqc_base;
};

struct hns_roce_caps {
	u8		num_ports;
	int		gid_table_len[HNS_ROCE_MAX_PORTS];
	int		pkey_table_len[HNS_ROCE_MAX_PORTS];
	int		local_ca_ack_delay;
	int		num_uars;
	u32		phy_num_uars;
	u32		max_sq_sg;	/* 2 */
	u32		max_sq_inline;	/* 32 */
	u32		max_rq_sg;	/* 2 */
	int		num_qps;	/* 256k */
	u32		max_wqes;	/* 16k */
	u32		max_sq_desc_sz;	/* 64 */
	u32		max_rq_desc_sz;	/* 64 */
	int		max_qp_init_rdma;
	int		max_qp_dest_rdma;
	int		sqp_start;
	int		num_cqs;
	int		max_cqes;
	int		reserved_cqs;
	int		num_aeq_vectors;	/* 1 */
	int		num_comp_vectors;	/* 32 ceq */
	int		num_other_vectors;
	int		num_mtpts;
	u32		num_mtt_segs;
	int		reserved_mrws;
	int		reserved_uars;
	int		num_pds;
	int		reserved_pds;
	u32		mtt_entry_sz;
	u32		cq_entry_sz;
	u32		page_size_cap;
	u32		reserved_lkey;
	int		mtpt_entry_sz;
	int		qpc_entry_sz;
	int		irrl_entry_sz;
	int		cqc_entry_sz;
	int		aeqe_depth;
	int		ceqe_depth[HNS_ROCE_COMP_VEC_NUM];
	enum ib_mtu	max_mtu;
};

struct hns_roce_hw {
	int (*reset)(struct hns_roce_dev *hr_dev, bool enable);
	void (*hw_profile)(struct hns_roce_dev *hr_dev);
	int (*hw_init)(struct hns_roce_dev *hr_dev);
	void (*hw_exit)(struct hns_roce_dev *hr_dev);
	void	*priv;
};

struct hns_roce_dev {
	struct ib_device	ib_dev;
	struct platform_device  *pdev;
	struct hns_roce_uar     priv_uar;
	const char		*irq_names;
	spinlock_t		sm_lock;
	spinlock_t		cq_db_lock;
	spinlock_t		bt_cmd_lock;
	struct hns_roce_ib_iboe iboe;

	int			irq[HNS_ROCE_MAX_IRQ_NUM];
	u8 __iomem		*reg_base;
	struct hns_roce_caps	caps;
	struct radix_tree_root  qp_table_tree;

	u64			sys_image_guid;
	u32                     vendor_id;
	u32                     vendor_part_id;
	u32                     hw_rev;

	struct hns_roce_cmdq	cmd;
	struct hns_roce_bitmap    pd_bitmap;
	struct hns_roce_uar_table uar_table;
	struct hns_roce_mr_table  mr_table;
	struct hns_roce_cq_table  cq_table;
	struct hns_roce_qp_table  qp_table;
	struct hns_roce_eq_table  eq_table;

	int			cmd_mod;
	int			loop_idc;
	struct hns_roce_hw	*hw;
};

static inline void hns_roce_write64_k(__be32 val[2], void __iomem *dest)
{
	__raw_writeq(*(u64 *) val, dest);
}

static inline struct hns_roce_qp
	*__hns_roce_qp_lookup(struct hns_roce_dev *hr_dev, u32 qpn)
{
	return radix_tree_lookup(&hr_dev->qp_table_tree,
				 qpn & (hr_dev->caps.num_qps - 1));
}

int hns_roce_init_uar_table(struct hns_roce_dev *dev);
int hns_roce_uar_alloc(struct hns_roce_dev *dev, struct hns_roce_uar *uar);
void hns_roce_uar_free(struct hns_roce_dev *dev, struct hns_roce_uar *uar);
void hns_roce_cleanup_uar_table(struct hns_roce_dev *dev);

int hns_roce_cmd_init(struct hns_roce_dev *hr_dev);
void hns_roce_cmd_cleanup(struct hns_roce_dev *hr_dev);
void hns_roce_cmd_event(struct hns_roce_dev *hr_dev, u16 token, u8 status,
			u64 out_param);
int hns_roce_cmd_use_events(struct hns_roce_dev *hr_dev);
void hns_roce_cmd_use_polling(struct hns_roce_dev *hr_dev);

int hns_roce_init_pd_table(struct hns_roce_dev *hr_dev);
int hns_roce_init_mr_table(struct hns_roce_dev *hr_dev);
int hns_roce_init_eq_table(struct hns_roce_dev *hr_dev);
int hns_roce_init_cq_table(struct hns_roce_dev *hr_dev);
int hns_roce_init_qp_table(struct hns_roce_dev *hr_dev);

void hns_roce_cleanup_pd_table(struct hns_roce_dev *hr_dev);
void hns_roce_cleanup_mr_table(struct hns_roce_dev *hr_dev);
void hns_roce_cleanup_eq_table(struct hns_roce_dev *hr_dev);
void hns_roce_cleanup_cq_table(struct hns_roce_dev *hr_dev);
void hns_roce_cleanup_qp_table(struct hns_roce_dev *hr_dev);

int hns_roce_bitmap_alloc(struct hns_roce_bitmap *bitmap, unsigned long *obj);
void hns_roce_bitmap_free(struct hns_roce_bitmap *bitmap, unsigned long obj);
int hns_roce_bitmap_init(struct hns_roce_bitmap *bitmap, u32 num, u32 mask,
			 u32 reserved_bot, u32 resetrved_top);
void hns_roce_bitmap_cleanup(struct hns_roce_bitmap *bitmap);
void hns_roce_cleanup_bitmap(struct hns_roce_dev *hr_dev);
int hns_roce_bitmap_alloc_range(struct hns_roce_bitmap *bitmap, int cnt,
				int align, unsigned long *obj);
void hns_roce_bitmap_free_range(struct hns_roce_bitmap *bitmap,
				unsigned long obj, int cnt);

void hns_roce_cq_completion(struct hns_roce_dev *hr_dev, u32 cqn);
void hns_roce_cq_event(struct hns_roce_dev *hr_dev, u32 cqn, int event_type);
void hns_roce_qp_event(struct hns_roce_dev *hr_dev, u32 qpn, int event_type);

extern struct hns_roce_hw hns_roce_hw_v1;

#endif /* _HNS_ROCE_DEVICE_H */

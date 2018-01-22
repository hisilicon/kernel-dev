/*
 * Copyright (c) 2017. Hisilicon Tech Co. Ltd. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __HZIP_DRV_H__
#define __HZIP_DRV_H__

#include <linux/types.h>
#include "../wd.h"

#define HZIP_SQE_SIZE		128
#define QM_CQE_SIZE		16
#define QM_EQ_DEPTH		1024

/* cqe shift */
#define CQE_PHASE(cq)			(((*((__u32 *)(cq) + 3)) >> 16) & 0x1)
#define CQE_SQ_NUM(cq)			((*((__u32 *)(cq) + 2)) >> 16)
#define CQE_SQ_HEAD_INDEX(cq)		((*((__u32 *)(cq) + 2)) & 0xffff)

#define HZIP_BAR2_SIZE          (4 * 1024 * 1024)

#define HZIP_DOORBELL_OFFSET    0x340;


/* add as private date in wd_queue of user space */
struct hzip_queue_info {
	void *sq_base;
	void *cq_base;
        void *doorbell_base;
        __u16 sq_tail_index;
        __u16 cq_head_index;
	__u16 sqn;
        bool cqc_phase;
};

struct hisi_acc_qm_db {
	__u16 tag;
	__u8 cmd;
	__u16 index;
	__u8 priority;
};

struct hisi_acc_qm_sqc {
	__u16 sq_head_index;
	__u16 sq_tail_index;

	__u16 sqn;

	/* now we don't export other info in sqc */
};

#define HACC_QM_DB_SQ		_IOW('d', 0, unsigned long)
#define HACC_QM_MB_SQC		_IOR('d', 1, struct hisi_acc_qm_sqc *)
#define HACC_QM_SET_PASID	_IOW('d', 2, unsigned long)
#define HACC_QM_DB_CQ		_IOW('d', 3, unsigned long)

/* fix me */
#define DOORBELL_CMD_SQ 	0
#define DOORBELL_CMD_CQ 	1

int hisi_zip_set_queue_dio(struct wd_queue *q);
int hisi_zip_unset_queue_dio(struct wd_queue *q);
int hisi_zip_add_to_dio_q(struct wd_queue *q, void *req);
int hisi_zip_get_from_dio_q(struct wd_queue *q, void **resp);
int hisi_zip_set_pasid(struct wd_queue *q);
int hisi_zip_unset_pasid(struct wd_queue *q);

#endif

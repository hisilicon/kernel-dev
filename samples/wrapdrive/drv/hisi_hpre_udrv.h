/*
 * Copyright (c) 2017. Hisilicon Tech Co. Ltd. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef HISI_HPRE_UDRV_H__
#define HISI_HPRE_UDRV_H__

#include <linux/types.h>
#include "../wd.h"

#define HPRE_SQE_SIZE		64
#define QM_CQE_SIZE		16
#define QM_EQ_DEPTH		1024

/* cqe shift */
#define CQE_PHASE(cq)			(((*((__u32 *)(cq) + 3)) >> 16) & 0x1)
#define CQE_SQ_NUM(cq)			((*((__u32 *)(cq) + 2)) >> 16)
#define CQE_SQ_HEAD_INDEX(cq)		((*((__u32 *)(cq) + 2)) & 0xffff)

#define HPRE_BAR2_SIZE			(4 * 1024 * 1024)

#define HPRE_DOORBELL_OFFSET		0x340

#define SQE_DONE_FLAG_SHIFT 		30
enum hpre_alg_type {
 	HPRE_ALG_NC_NCRT = 0x0,
	HPRE_ALG_NC_CRT = 0x1,
	HPRE_ALG_KG_STD = 0x2,
	HPRE_ALG_KG_CRT = 0x3,
	HPRE_ALG_DH_G2 = 0x4,
	HPRE_ALG_DH = 0x5,
	HPRE_ALG_PRIME = 0x6,
	HPRE_ALG_MOD = 0x7,
	HPRE_ALG_MOD_INV = 0x8,
	HPRE_ALG_MUL = 0x9,
	HPRE_ALG_COPRIME = 0xA
};


struct hpre_cqe {
	__le32 rsvd0;
	__le16 cmd_id;
	__le16 rsvd1;
	__le16 sq_head;
	__le16 sq_num;
	__le16 rsvd2;
	__le16 w7; /* phase, status */
};

struct hpre_sqe {
	__u32 alg 	: 5;

	/* error type */
	__u32 etype 	:11;
	__u32 resv0 	: 14;
	__u32 done	: 2;

	__u32 task_len1	: 8;
	__u32 task_len2	: 8;
	__u32 mrttest_num : 8;
	__u32 resv1 	: 8;

	__u32 low_key;
	__u32 hi_key;
	__u32 low_in;
	__u32 hi_in;
	__u32 low_out;
	__u32 hi_out;

	__u32 tag	:16;
	__u32 resv2	:16;
	__u32 rsvd1[7];
};

struct hpre_queue_info {
	struct hpre_sqe cache_sqe;
	void *dma_buf;
	void *sq_base;
	void *cq_base;
	void *doorbell_base;
	__u16 sq_tail_index;
	__u16 sq_head_index;
	__u16 cq_head_index;
	__u16 sqn;
	bool cqc_phase;
	void *recv;
	int is_sq_full;
};

struct acc_qm_sqc {
	__u16 sqn;
};

#define HPRE_QM_MB_SQC		_IOR('d', 1, struct acc_qm_sqc *)
#define HPRE_QM_SET_PASID	_IOW('d', 2, unsigned long)

#define DOORBELL_CMD_SQ		0
#define DOORBELL_CMD_CQ		1

int hpre_set_queue_dio(struct wd_queue *q);
int hpre_unset_queue_dio(struct wd_queue *q);
int hpre_add_to_dio_q(struct wd_queue *q, void *req);
int hpre_get_from_dio_q(struct wd_queue *q, void **resp);
int hpre_set_pasid(struct wd_queue *q);
int hpre_unset_pasid(struct wd_queue *q);

#endif

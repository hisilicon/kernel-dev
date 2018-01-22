/*
 * Copyright (c) 2017. Hisilicon Tech Co. Ltd. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "./hisi_zip_udrv.h"
#include "../wd_util.h"
#include "../wd_comp.h"

/* fix me */
struct hisi_zip_q_priv {
	/* local mirror of the register space */
	int head;		/* queue head */
	int resp_tail;		/* resp tail in the queue */
	/* so in the user side: when add to queue, head++ but don't exceed
	 * resp_tail. when get back from the queue, resp_tail++ but don't
	 * exceed tail. in the kernel side: when get from queue,
	 * tail++ but don't exceed head-1
	 */

	struct hisi_zip_hw_queue_reg *reg;
};

int hisi_zip_set_pasid(struct wd_queue *q)
{
        int ret;

	ret = ioctl(q->device, HACC_QM_SET_PASID, q->pasid);
        if (ret == -1) {
		printf("HACC_QM_SET_PASID ioctl fail!\n");
                return -1;
        }

	return 0;
}

int hisi_zip_unset_pasid(struct wd_queue *q)
{
        int ret;

	ret = ioctl(q->device, HACC_QM_SET_PASID, 0);
        if (ret == -1) {
		printf("HACC_QM_SET_PASID(unset) ioctl fail!\n");
                return -1;
        }

	return 0;
}

/* user date: msg, sq date: info, sqe offet: i */
static int hisi_zip_fill_sqe(struct wd_comp_msg *msg,
			     struct hzip_queue_info *info, __u16 i)
{
	__u64 src = msg->src;
	__u64 dst = msg->dst;
	__u32 aflags = msg->aflags;
	__u32 size = msg->in_bytes;
	void *sqe = info->sq_base + HZIP_SQE_SIZE * i;


	*((__u32 *)sqe + 4) = size;   // input date length
	*((__u32 *)sqe + 9) = 2; // request type 8bit
	//*((__u32 *)sqe + 9) = 0;      // buffer type 4bit
	*((__u32 *)sqe + 14) = 0x800; // define out buffer size, 2k now
	*((__u32 *)sqe + 18) = src & 0xffffffff;     // src
	*((__u32 *)sqe + 19) = src >> 32;
	*((__u32 *)sqe + 20) = dst & 0xffffffff;     // dst
	*((__u32 *)sqe + 21) = dst >> 32;
#if 0
	printf("in fill: dump sqe:\n");
	printf("         sqe base: %p\n", sqe);
	printf("         sqe  [4]: %lx\n", *((__u32 *)sqe + 4));
	printf("         sqe  [9]: %lx\n", *((__u32 *)sqe + 9));
	printf("         sqe [18]: %lx\n", *((__u32 *)sqe + 18));
	printf("         sqe [19]: %lx\n", *((__u32 *)sqe + 19));
	printf("         sqe [20]: %lx\n", *((__u32 *)sqe + 20));
	printf("         sqe [21]: %lx\n", *((__u32 *)sqe + 21));
#endif
	return 0;
}

int hisi_zip_set_queue_dio(struct wd_queue *q)
{
	struct hzip_queue_info *info;
	struct hisi_acc_qm_sqc qm_sqc;
	void *vaddr;
        int ret;

	info = malloc(sizeof(struct hzip_queue_info));
	if (!info)
		return -1;

	q->priv = info;

	/* mmap hisi zip sq and cq to user space */
	vaddr = mmap(NULL,
                     HZIP_SQE_SIZE * QM_EQ_DEPTH + QM_CQE_SIZE * QM_EQ_DEPTH,
                     PROT_READ | PROT_WRITE, MAP_SHARED, q->device, 0);
	if (vaddr == MAP_FAILED || vaddr == NULL)
		return -EIO;

	info->sq_base = vaddr;
	info->cq_base = vaddr + HZIP_SQE_SIZE * QM_EQ_DEPTH;
        info->sq_tail_index = 0;
        info->cq_head_index = 0;
        info->cqc_phase = 1;

	ret = ioctl(q->device, HACC_QM_MB_SQC, &qm_sqc);
	if (ret == -1) {
		printf("open erro: %s\n", strerror(errno));
		printf("HACC_QM_MB_SQC ioctl fail!\n");
		return -1;
	}

        info->sqn = qm_sqc.sqn;

	return hisi_zip_set_pasid(q);
}

int hisi_zip_unset_queue_dio(struct wd_queue *q)
{
	struct hzip_queue_info *info = (struct hzip_queue_info *)q->priv;

	munmap(info->sq_base, HZIP_SQE_SIZE * QM_EQ_DEPTH + QM_CQE_SIZE * QM_EQ_DEPTH);
	free(info);
	q->priv = NULL;

	return hisi_zip_unset_pasid(q);
}

int hisi_zip_add_to_dio_q(struct wd_queue *q, void *req)
{
	//printf("in send\n");
	struct wd_comp_msg *msg = (struct wd_comp_msg *)req;
	struct hzip_queue_info *info = (struct hzip_queue_info *)q->priv;
	struct hisi_acc_qm_db qm_db;
	int ret;
	__u16 i;

	i = info->sq_tail_index;

	/* to do: fill sqe */
	hisi_zip_fill_sqe(msg, q->priv, i);

	//printf("in send: tag: %d, cmd: %d, index: %d\n", info->sqn, DOORBELL_CMD_SQ, i);

	/* fill doorbell struct */
	qm_db.tag = info->sqn;
	qm_db.cmd = DOORBELL_CMD_SQ;
	qm_db.index = ++i;
	qm_db.priority = 0;

	/* to do: ioctl to trigger sq doorbell */
	ret = ioctl(q->device, HACC_QM_DB_SQ, &qm_db);
	if (ret == -1) {
		printf("HACC_QM_DB_SQ ioctl fail!\n");
		return -1;
	}

        info->sq_tail_index = i;

	return 0;
}

int hisi_zip_get_from_dio_q(struct wd_queue *q, void **resp)
{
	struct hzip_queue_info *info = (struct hzip_queue_info *)q->priv;
	__u16 i = info->cq_head_index;
	void *cq_base = info->cq_base;
	void *sq_base = info->sq_base;
        char *cqe = cq_base + i * QM_CQE_SIZE;
        char *sqe = sq_base + CQE_SQ_HEAD_INDEX(cqe) * HZIP_SQE_SIZE;
	struct hisi_acc_qm_db qm_db;
	int ret;

        if (info->cqc_phase == CQE_PHASE(cqe)) {
                /* to do: handle_sqe(sqe); */
        } else {
                return -EBUSY;
        }

        if (i == (QM_EQ_DEPTH - 1)) {
                i = 0;
                info->cqc_phase != (info->cqc_phase);
        } else {
                i++;
        }

        qm_db.tag = info->sqn;
        qm_db.cmd = DOORBELL_CMD_CQ;
        qm_db.index = i;
        qm_db.priority = 0;

        /* ioctl to trigger cq doorbell */
        ret = ioctl(q->device, HACC_QM_DB_CQ, &qm_db);
        if (ret == -1) {
                printf("HACC_QM_DB_CQ ioctl fail!\n");
                return -1;
        }

        info->cq_head_index = i;

	return 0;
}

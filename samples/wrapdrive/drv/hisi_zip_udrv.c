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

char *hzip_request_type[] = {
        "none",
        "resv",                 /* 0x01 */
        "zlib_comp",            /* 0x02 */
};

int hacc_db(struct hzip_queue_info *q, __u8 cmd, __u16 index, __u8 priority)
{
	void *base = q->doorbell_base;
        __u16 sqn = q->sqn;
	__u64 doorbell = 0;

	doorbell = (__u64)sqn | ((__u64)cmd << 16);
	doorbell |= ((__u64)index | ((__u64)priority << 16)) << 32;

        *((__u64 *)base) = doorbell;

	return 0;
}


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

static int hisi_zip_fill_sqe(struct wd_comp_msg *msg,
			     struct hzip_queue_info *info, __u16 i)
{
	__u64 src = msg->src;
	__u64 dst = msg->dst;
	__u32 size = msg->in_bytes;
	void *sqe = info->sq_base + HZIP_SQE_SIZE * i;

	*((__u32 *)sqe + 4) = size;
	*((__u32 *)sqe + 9) = 2;
	*((__u32 *)sqe + 14) = 0x800;
	*((__u32 *)sqe + 18) = src & 0xffffffff;
	*((__u32 *)sqe + 19) = src >> 32;
	*((__u32 *)sqe + 20) = dst & 0xffffffff;
	*((__u32 *)sqe + 21) = dst >> 32;

	return 0;
}

int hisi_zip_recv_sqe(char *sqe, struct wd_comp_msg *recv_msg)
{
        int status = *((__u32 *)sqe + 3) & 0xff;
        int type = *((__u32 *)sqe + 9) & 0xff;

        if (status != 0) {
                printf("hisi zip %s fail!\n", hzip_request_type[type]);
                return -1;
        }

        recv_msg->alg = hzip_request_type[type];
        recv_msg->aflags = 0;
        recv_msg->in_coms = *(__u32 *)sqe;
        recv_msg->in_bytes = *((__u32 *)sqe + 4);
        recv_msg->out_bytes = *((__u32 *)sqe + 1);
        recv_msg->src = ((__u64)*((__u32 *)sqe + 19) << 32) | *((__u32 *)sqe + 18);
        recv_msg->dst = ((__u64)*((__u32 *)sqe + 21) << 32) | *((__u32 *)sqe + 20);
        recv_msg->comp_lv = 0;
        recv_msg->file_type = 0;
        recv_msg->humm_type = 0;
        recv_msg->op_type = 0;
        recv_msg->win_size = 0;
        recv_msg->cflags = 0;
        recv_msg->status = status;
        recv_msg->udata = 0;

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
                     HZIP_SQE_SIZE * QM_EQ_DEPTH + QM_CQE_SIZE * QM_EQ_DEPTH + HZIP_BAR2_SIZE,
                     PROT_READ | PROT_WRITE, MAP_SHARED, q->device, 0);
	if (vaddr == MAP_FAILED || vaddr == NULL)
		return -EIO;

	info->sq_base = vaddr;
	info->cq_base = vaddr + HZIP_SQE_SIZE * QM_EQ_DEPTH;
        info->doorbell_base = info->cq_base + QM_CQE_SIZE * QM_EQ_DEPTH +
                              HZIP_DOORBELL_OFFSET;
        info->sq_tail_index = 0;
        info->cq_head_index = 0;
        info->cqc_phase = 1;

	ret = ioctl(q->device, HACC_QM_MB_SQC, &qm_sqc);
	if (ret == -1) {
		printf("HACC_QM_MB_SQC ioctl fail!\n");
		return -1;
	}

        info->sqn = qm_sqc.sqn;

	info->recv = malloc(sizeof(struct wd_comp_msg) * QM_EQ_DEPTH);
	if (!info->recv)
		return -1;

	return hisi_zip_set_pasid(q);
}

int hisi_zip_unset_queue_dio(struct wd_queue *q)
{
	struct hzip_queue_info *info = (struct hzip_queue_info *)q->priv;

	munmap(info->sq_base, HZIP_SQE_SIZE * QM_EQ_DEPTH +
               QM_CQE_SIZE * QM_EQ_DEPTH + HZIP_BAR2_SIZE);
        free(info->recv);
	free(info);
	q->priv = NULL;

	return hisi_zip_unset_pasid(q);
}

int hisi_zip_add_to_dio_q(struct wd_queue *q, void *req)
{
	struct wd_comp_msg *msg = (struct wd_comp_msg *)req;
	struct hzip_queue_info *info = (struct hzip_queue_info *)q->priv;
	int ret;
	__u16 i;

	i = info->sq_tail_index;

	hisi_zip_fill_sqe(msg, q->priv, i);

        if (i == (QM_EQ_DEPTH - 1))
                i = 0;
        else
                i++;

        hacc_db(info, DOORBELL_CMD_SQ, i, 0);

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
        struct wd_comp_msg *recv_msg = info->recv +
                                       i * sizeof(struct wd_comp_msg);
	int ret;

        if (info->cqc_phase == CQE_PHASE(cqe)) {
                ret = hisi_zip_recv_sqe(sqe, recv_msg);
                if (ret != 0)
                        return -EIO;
        } else {
                return -EBUSY;
        }

        if (i == (QM_EQ_DEPTH - 1)) {
                i = 0;
                info->cqc_phase != (info->cqc_phase);
        } else {
                i++;
        }

        hacc_db(info, DOORBELL_CMD_CQ, i, 0);

        info->cq_head_index = i;

	return 0;
}

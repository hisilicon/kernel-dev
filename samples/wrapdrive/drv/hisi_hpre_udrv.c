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

#include "./hisi_hpre_udrv.h"
#include "../wd_util.h"
#include "../wd_rsa.h"

#define mb() asm volatile("dsb sy" : : : "memory")

char *hpre_request_type[] = {
	"none",
	"resv",			/* 0x01 */
	"zlib_comp",		/* 0x02 */
};

int hpre_db(struct hpre_queue_info *q, __u8 cmd, __u16 index, __u8 priority)
{
	void *base = q->doorbell_base;
	__u16 sqn = q->sqn;
	__u64 doorbell = 0;

	doorbell = (__u64)sqn | ((__u64)cmd << 16);
	doorbell |= ((__u64)index | ((__u64)priority << 16)) << 32;

	*((__u64 *)base) = doorbell;

	return 0;
}


int hpre_set_pasid(struct wd_queue *q)
{
	int ret;

	ret = ioctl(q->device, HPRE_QM_SET_PASID, q->pasid);
	if (ret == -1) {
		printf("HACC_QM_SET_PASID ioctl fail!\n");
		return -1;
	}

	return 0;
}

int hpre_unset_pasid(struct wd_queue *q)
{
	int ret;

	ret = ioctl(q->device, HPRE_QM_SET_PASID, 0);
	if (ret == -1) {
		printf("HACC_QM_SET_PASID(unset) ioctl fail!\n");
		return -1;
	}

	return 0;
}

static int hpre_fill_sqe(void *msg, struct wd_queue *q, __u16 i)
{
	struct hpre_queue_info *info = q->priv;
	struct hpre_sqe *cc_sqe = &info->cache_sqe;
	struct hpre_sqe *sqe = (struct hpre_sqe *)info->sq_base + i;
	char *alg = q->capa.alg;
	struct wd_rsa_msg *rsa_msg = msg;
	void *dma_buf = info->dma_buf;
	int k, j = 0;
	__u8 *tmp;

	if (!strncmp(alg, rsa, 3)) {
		if (rsa_msg->prikey_type == WD_RSA_PRIKEY2)
			cc_sqe->alg = HPRE_ALG_NC_CRT;
		else if (rsa_msg->prikey_type == WD_RSA_PRIKEY1)
			cc_sqe->alg = HPRE_ALG_NC_NCRT;
		else
			return -1;
		cc_sqe->task_len1 = rsa_msg->nbytes /8 - 1;
		if (rsa_msg->op_type == WD_RSA_SIGN) {
			/* Since SVA and key SGLs is not supported now, we should copy */
			if (cc_sqe->alg == HPRE_ALG_NC_CRT) {
				struct wd_rsa_prikey2 *prikey2 = (void *)rsa_msg->prikey;

				for (k = rsa_msg->nbytes / 16 - 1; k >= 0; k--) {
					tmp = prikey2->dq + k * 8;
					endian_swap_in_dword(tmp, 8);
					*((__u64 *)dma_buf + j) = *(__u64 *)tmp;

					tmp = prikey2->dp + k * 8;
					endian_swap_in_dword(tmp, 8);
					*((__u64 *)dma_buf + j + rsa_msg->nbytes /16) = *(__u64 *)tmp;

					tmp = prikey2->q + k * 8;
					endian_swap_in_dword(tmp, 8);
					*((__u64 *)dma_buf + j + rsa_msg->nbytes /8) = *(__u64 *)tmp;

					tmp = prikey2->p + k * 8;
					endian_swap_in_dword(tmp, 8);
					*((__u64 *)dma_buf + j + rsa_msg->nbytes * 3 /16) = *(__u64 *)tmp;

					tmp = prikey2->qinv + k * 8;
					endian_swap_in_dword(tmp, 8);
					*((__u64 *)dma_buf + j + rsa_msg->nbytes /4) = *(__u64 *)tmp;

					j++;
				}
			} else {
				struct wd_rsa_prikey1 *prikey1 = (void *)rsa_msg->prikey;
				for (k = rsa_msg->nbytes / 8 - 1; k >= 0; k--) {
					tmp = prikey1->d + k * 8;
					endian_swap_in_dword(tmp, 8);
					*((__u64 *)dma_buf + j) = *(__u64 *)tmp;

					tmp = prikey1->n + k * 8;
					endian_swap_in_dword(tmp, 8);
					*((__u64 *)dma_buf + j + rsa_msg->nbytes /8) = *(__u64 *)tmp;

					j++;
				}
			} 
		}else if (rsa_msg->op_type == WD_RSA_VERIFY) {
			struct wd_rsa_pubkey *pubkey = (void *)rsa_msg->pubkey;

			for (k = rsa_msg->nbytes / 8 - 1; k >= 0; k--) {
				tmp = pubkey->e + k * 8;
				endian_swap_in_dword(tmp, 8);
				*((__u64 *)dma_buf + j) = *(__u64 *)tmp;

				tmp = pubkey->n + k * 8;
				endian_swap_in_dword(tmp, 8);
				*((__u64 *)dma_buf + j + rsa_msg->nbytes /8) = *(__u64 *)tmp;

				j++;
			}
		} else {
			WD_ERR("\nrsa algorithm support only sign and verify now!");
			return -1;
		}
		cc_sqe->low_key = (__u32)(((__u64)dma_buf) & 0xffffffff);
		cc_sqe->hi_key = (__u32)((((__u64)dma_buf) >> 32) & 0xffffffff);
	}else {
		WD_ERR("\nalg=%s,rsa algorithm support only now!", alg);
		return -1;
	}
	dma_buf += (rsa_msg->nbytes * 4);
	for (k = rsa_msg->nbytes / 8 - 1; k >= 0; k--) {
		tmp = (__u8 *)rsa_msg->in + k * 8;
		endian_swap_in_dword(tmp, 8);
		*((__u64 *)dma_buf + j) = *(__u64 *)tmp;
		j++;
	}
	cc_sqe->low_in = (__u32)((__u64)dma_buf & 0xffffffff);
	cc_sqe->hi_in = (__u32)(((__u64)dma_buf >> 32) & 0xffffffff);

	dma_buf += (rsa_msg->nbytes * 5);
	cc_sqe->low_out = (__u32)((__u64)dma_buf & 0xffffffff);
	cc_sqe->hi_out = (__u32)(((__u64)dma_buf >> 32) & 0xffffffff);

	/* This need more processing logic. to do more */
	cc_sqe->tag = (__u32)rsa_msg->udata;
	cc_sqe->done = 0x1;

	memcpy((void *)sqe, (void *)cc_sqe, sizeof(*cc_sqe));

	return 0;
}

int hpre_recv_sqe(struct hpre_sqe *sqe, struct wd_rsa_msg *recv_msg)
{
	__u32 status = sqe->done;
	__u8 *tmp;

	if (status != 0x3) {
		WD_ERR("HPRE do %s fail!\n", rsa);
		return -1;
	}

	recv_msg->alg = rsa;
	recv_msg->aflags = 0;
	recv_msg->out = ((__u64)(sqe->hi_out) << 32) | (sqe->low_out);
	recv_msg->outbytes = (__u16)((sqe->task_len1 + 1) << 6);
	tmp = (__u8 *)recv_msg->out;
	endian_swap_in_dword(tmp, recv_msg->outbytes);
	recv_msg->udata = (__u64)sqe->tag;

	return 1;
}
#define HPRE_DMA_PAGE 		4096
static int hpre_init_cache_sqe(struct wd_queue *q)
{
	void *dma_buf;
	struct hpre_queue_info *info = q->priv;
	int ret;

	dma_buf = mmap((void *)0x0, HPRE_DMA_PAGE , PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (!dma_buf) {
		printf("\nmmap dma buf fail!");
		return -1;
	}
	memset(dma_buf, 0, HPRE_DMA_PAGE);

	ret = wd_mem_share(q, (const void *)dma_buf, HPRE_DMA_PAGE, 0);
	if (ret) {
		printf("\nwd_mem_share dma buf fail!");
		return ret;
	}
	info->cache_sqe.low_key = (__u32)(((__u64)dma_buf) & 0xffffffff);
	info->cache_sqe.hi_key = (__u32)((((__u64)dma_buf) >> 32) & 0xffffffff);
	info->dma_buf = dma_buf;

	return 0;
}
int hpre_set_queue_dio(struct wd_queue *q)
{
	struct hpre_queue_info *info;
	struct acc_qm_sqc qm_sqc;
	void *vaddr;
	int ret;

	info = malloc(sizeof(struct hpre_queue_info));
	if (!info)
		return -1;

	q->priv = info;

	vaddr = mmap(NULL, HPRE_SQE_SIZE * QM_EQ_DEPTH,
		     PROT_READ | PROT_WRITE, MAP_SHARED, q->device, 0);
	if (vaddr == MAP_FAILED || vaddr == NULL)
		return -EIO;
	info->sq_base = vaddr;

	vaddr = mmap(NULL, QM_CQE_SIZE * QM_EQ_DEPTH,
		     PROT_READ | PROT_WRITE, MAP_SHARED, q->device,
		     HPRE_SQE_SIZE * QM_EQ_DEPTH);
	if (vaddr == MAP_FAILED || vaddr == NULL)
		return -EIO;
	info->cq_base = vaddr;

	vaddr = mmap(NULL, HPRE_BAR2_SIZE,
		     PROT_READ | PROT_WRITE, MAP_SHARED, q->device,
		     HPRE_SQE_SIZE * QM_EQ_DEPTH + QM_CQE_SIZE * QM_EQ_DEPTH);
	if (vaddr == MAP_FAILED || vaddr == NULL)
		return -EIO;
	info->doorbell_base = vaddr + HPRE_DOORBELL_OFFSET;

	info->sq_tail_index = 0;
	info->sq_head_index = 0;
	info->cq_head_index = 0;
	info->cqc_phase = 1;

	info->is_sq_full = 0;

	ret = ioctl(q->device, HPRE_QM_MB_SQC, &qm_sqc);
	if (ret == -1) {
		printf("HACC_QM_MB_SQC ioctl fail!\n");
		return -1;
	}

	info->sqn = qm_sqc.sqn;

	info->recv = malloc(sizeof(struct wd_rsa_msg) * QM_EQ_DEPTH);
	if (!info->recv)
		return -1;

	ret = hpre_set_pasid(q);
	if (ret) {
		printf("hpre_set_pasid fail!\n");
		return -1;
	}

	return hpre_init_cache_sqe(q);
}

int hpre_unset_queue_dio(struct wd_queue *q)
{
	struct hpre_queue_info *info = (struct hpre_queue_info *)q->priv;

	munmap(info->sq_base, HPRE_SQE_SIZE * QM_EQ_DEPTH);
	munmap(info->cq_base, QM_CQE_SIZE * QM_EQ_DEPTH);
	munmap(info->doorbell_base - HPRE_DOORBELL_OFFSET, HPRE_BAR2_SIZE);

	free(info->recv);
	free(info);
	q->priv = NULL;

	return hpre_unset_pasid(q);
}

int hpre_add_to_dio_q(struct wd_queue *q, void *req)
{
	struct wd_rsa_msg *msg = req;
	struct hpre_queue_info *info = (struct hpre_queue_info *)q->priv;
	__u16 i;

	if (info->is_sq_full)
		return -EBUSY;

	i = info->sq_tail_index;

	hpre_fill_sqe(msg, q->priv, i);

	mb();

	if (i == (QM_EQ_DEPTH - 1))
		i = 0;
	else
		i++;

	hpre_db(info, DOORBELL_CMD_SQ, i, 0);

	info->sq_tail_index = i;

	if (i == info->sq_head_index)
		info->is_sq_full = 1;

	return 0;
}

int hpre_get_from_dio_q(struct wd_queue *q, void **resp)
{
	struct hpre_queue_info *info = (struct hpre_queue_info *)q->priv;
	__u16 i = info->cq_head_index;
	struct hpre_cqe *cq_base = info->cq_base;
	struct hpre_sqe *sq_base = info->sq_base;
	struct cqe *cqe = (void *)(cq_base + i);
	struct hpre_sqe *sqe = sq_base + CQE_SQ_HEAD_INDEX(cqe);
	struct wd_rsa_msg *recv_msg = info->recv +
		i * sizeof(struct wd_rsa_msg);
	int ret;

	if (info->cqc_phase == CQE_PHASE(cqe)) {
		ret = hpre_recv_sqe(sqe, recv_msg);
		if (ret < 0)
			return -EIO;

		if (info->is_sq_full)
			info->is_sq_full = 0;
	} else {
		return 0;
	}

	if (i == (QM_EQ_DEPTH - 1)) {
		info->cqc_phase = !(info->cqc_phase);
		i = 0;
	} else {
		i++;
	}

	hpre_db(info, DOORBELL_CMD_CQ, i, 0);

	info->cq_head_index = i;
	info->sq_head_index = i;

	*resp = recv_msg;

	return ret;
}

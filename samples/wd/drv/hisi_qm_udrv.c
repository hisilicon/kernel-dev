/* SPDX-License-Identifier: Apache-2.0 */
#include "config.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "wd_drv.h"
#include "hisi_qm_udrv.h"

#define QM_SQE_SIZE		128 /* todo: get it from sysfs */
#define QM_CQE_SIZE		16

#define DOORBELL_CMD_SQ		0
#define DOORBELL_CMD_CQ		1

/* cqe shift */
#define CQE_PHASE(cq)	(((*((__u32 *)(cq) + 3)) >> 16) & 0x1)
#define CQE_SQ_NUM(cq)	((*((__u32 *)(cq) + 2)) >> 16)
#define CQE_SQ_HEAD_INDEX(cq)	((*((__u32 *)(cq) + 2)) & 0xffff)

struct hisi_qm_queue_info {
	void *sq_base;
	void *cq_base;
	int sqe_size;
	void *mmio_base;
	void *doorbell_base;
	int (*db)(struct hisi_qm_queue_info *q, __u8 cmd,
		  __u16 idx, __u8 priority);
	void *dko_base;
	void *ds_base;
	__u16 sq_tail_index;
	__u16 cq_head_index;
	__u16 sqn;
	int cqc_phase;
	void *req_cache[QM_Q_DEPTH];
	int used;
};

int hacc_db_v1(struct hisi_qm_queue_info *q, __u8 cmd,
	       __u16 idx, __u8 priority)
{
	void *base = q->doorbell_base;
	__u16 sqn = q->sqn;
	__u64 doorbell;

	doorbell = (__u64)sqn | ((__u64)cmd << 16);
	doorbell |= ((__u64)idx | ((__u64)priority << 16)) << 32;

	*((__u64 *)base) = doorbell;

	return 0;
}

/* Only Hi1620 CS, we just need version 2 doorbell. */
static int hacc_db_v2(struct hisi_qm_queue_info *q, __u8 cmd,
		      __u16 idx, __u8 priority)
{
	void *base = q->doorbell_base;
	__u16 sqn = q->sqn & 0x3ff;
	__u64 doorbell;

	doorbell = (__u64)sqn | ((__u64)(cmd & 0xf) << 12);
	doorbell |= ((__u64)idx | ((__u64)priority << 16)) << 32;

	*((__u64 *)base) = doorbell;

	return 0;
}

static int hisi_qm_fill_sqe(void *sqe, struct hisi_qm_queue_info *info, __u16 i)
{
	void *sq_offset_base = (void *)((uintptr_t)info->sq_base +
			i * info->sqe_size);

	memcpy(sq_offset_base, sqe, info->sqe_size);

	ASSERT(!info->req_cache[i]);
	info->req_cache[i] = sqe;

	return 0;
}

static int hisi_qm_recv_sqe(const void *sqe,
			    struct hisi_qm_queue_info *info, __u16 i)
{
	ASSERT(info->req_cache[i]);
	dbg("hisi_qm_recv_sqe: %p, %p, %d\n", info->req_cache[i], sqe,
	    info->sqe_size);
	memcpy(info->req_cache[i], sqe, info->sqe_size);
	return 0;
}

static int hisi_qm_set_queue_regions(struct wd_queue *q)
{
	unsigned int has_dko = !(q->dev_flags &
				(UACCE_DEV_NOIOMMU | UACCE_DEV_PASID));
	struct hisi_qm_queue_info *info = q->priv;

	info->sq_base = wd_drv_mmap_qfr(q, UACCE_QFRT_DUS, UACCE_QFRT_SS, 0);
	if (info->sq_base == MAP_FAILED) {
		info->sq_base = NULL;
		WD_ERR("mmap dus fail\n");
		return -ENOMEM;
	}

	info->mmio_base = wd_drv_mmap_qfr(q, UACCE_QFRT_MMIO, has_dko ?
					UACCE_QFRT_DKO : UACCE_QFRT_DUS, 0);
	if (info->mmio_base == MAP_FAILED) {
		info->mmio_base = NULL;
		WD_ERR("mmap mmio fail\n");
		return -ENOMEM;
	}

	if (has_dko) {
		info->dko_base = wd_drv_mmap_qfr(q, UACCE_QFRT_DKO,
						UACCE_QFRT_DUS, 0);
		if (info->dko_base == MAP_FAILED) {
			info->dko_base = NULL;
			WD_ERR("mmap dko fail!\n");
			return -ENOMEM;
		}
	}

	return 0;
}

static void hisi_qm_unset_queue_regions(struct wd_queue *q)
{
	struct hisi_qm_queue_info *info = q->priv;

	if (info->dko_base) {
		wd_drv_unmmap_qfr(q, info->dko_base,
				  UACCE_QFRT_DKO, UACCE_QFRT_DUS, 0);
		wd_drv_unmmap_qfr(q, info->mmio_base, UACCE_QFRT_MMIO,
				  UACCE_QFRT_DKO, 0);
	} else
		wd_drv_unmmap_qfr(q, info->mmio_base, UACCE_QFRT_MMIO,
				  UACCE_QFRT_DUS, 0);
	wd_drv_unmmap_qfr(q, info->sq_base, UACCE_QFRT_DUS, UACCE_QFRT_SS, 0);
}

static int hisi_qm_set_queue_info(struct wd_queue *q)
{
	struct hisi_qm_priv *priv = (struct hisi_qm_priv *)q->capa.priv;
	struct hisi_qm_queue_info *info = q->priv;
	struct hisi_qp_ctx qp_ctx;
	int ret;

	ret = hisi_qm_set_queue_regions(q);
	if (ret)
		return -EINVAL;
	info->sqe_size = priv->sqe_size;
	if (!info->sqe_size) {
		WD_ERR("sqe size =%d err!\n", info->sqe_size);
		return -EINVAL;
	}
	info->cq_base = (void *)((uintptr_t)info->sq_base +
			info->sqe_size * QM_Q_DEPTH);

	/* The last 32 bits of DUS show device or qp statuses */
	info->ds_base = info->sq_base + q->qfrs_offset[UACCE_QFRT_SS] -
				q->qfrs_offset[UACCE_QFRT_DUS] - sizeof(uint32_t);
	if (strstr(q->hw_type, HISI_QM_API_VER2_BASE)) {
		info->db = hacc_db_v2;
		info->doorbell_base = info->mmio_base + QM_V2_DOORBELL_OFFSET;
	} else if (strstr(q->hw_type, HISI_QM_API_VER_BASE)) {
		info->db = hacc_db_v1;
		info->doorbell_base = info->mmio_base + QM_DOORBELL_OFFSET;
	} else {
		WD_ERR("hw version mismatch!\n");
		return -EINVAL;
	}
	info->sq_tail_index = 0;
	info->cq_head_index = 0;
	info->cqc_phase = 1;
	info->used = 0;
	qp_ctx.qc_type = priv->op_type;
	qp_ctx.id = 0;
	ret = ioctl(q->fd, UACCE_CMD_QM_SET_QP_CTX, &qp_ctx);
	if (ret < 0) {
		WD_ERR("hisi qm set qc_type fail, use default!\n");
		return ret;
	}
	info->sqn = qp_ctx.id;

	return ret;
}

int hisi_qm_set_queue_dio(struct wd_queue *q)
{
	struct hisi_qm_queue_info *info;
	int ret = -ENOMEM;

	info = calloc(1, sizeof(*info));
	if (!info) {
		WD_ERR("no mem!\n");
		return -ENOMEM;
	}
	q->priv = info;
	ret = hisi_qm_set_queue_info(q);
	if (ret < 0)
		goto err_with_regions;

	return 0;

err_with_regions:
	hisi_qm_unset_queue_regions(q);
	free(q->priv);
	q->priv = NULL;
	return ret;
}

void hisi_qm_unset_queue_dio(struct wd_queue *q)
{
	hisi_qm_unset_queue_regions(q);
	free(q->priv);
	q->priv = NULL;
}

int hisi_qm_add_to_dio_q(struct wd_queue *q, void *req)
{
	struct hisi_qm_queue_info *info = (struct hisi_qm_queue_info *)q->priv;
	__u16 i;

	if (wd_reg_read(info->ds_base) == 1) {
		WD_ERR("wd queue hw error happened before qm send!\n");
		return -WD_HW_ERR;
	}

	if (__atomic_load_n(&info->used, __ATOMIC_RELAXED) == QM_Q_DEPTH) {
		WD_ERR("queue is full!\n");
		return -EBUSY;
	}

	i = info->sq_tail_index;

	hisi_qm_fill_sqe(req, q->priv, i);

	mb(); /* make sure the request is all in memory before doorbell */

	if (i == (QM_Q_DEPTH - 1))
		i = 0;
	else
		i++;

	info->db(info, DOORBELL_CMD_SQ, i, 0);

	info->sq_tail_index = i;
	__atomic_add_fetch(&info->used, 1, __ATOMIC_RELAXED);

	if (wd_reg_read(info->ds_base) == 1) {
		WD_ERR("wd queue hw error happened in qm send!\n");
		return -WD_HW_ERR;
	}

	return 0;
}

int hisi_qm_get_from_dio_q(struct wd_queue *q, void **resp)
{
	struct hisi_qm_queue_info *info = (struct hisi_qm_queue_info *)q->priv;
	__u16 i = info->cq_head_index, j;
	struct cqe *cqe = info->cq_base + i * sizeof(struct cqe);
	void *sq_offset_base = NULL;
	int ret;

	if (wd_reg_read(info->ds_base) == 1) {
		WD_ERR("wd queue hw error happened before qm receive!\n");
		return -WD_HW_ERR;
	}

	if (info->cqc_phase == CQE_PHASE(cqe)) {
		mb();
		j = CQE_SQ_HEAD_INDEX(cqe);
		if (j >= QM_Q_DEPTH) {
			WD_ERR("CQE_SQ_HEAD_INDEX(%d) error\n", j);
			return -EIO;
		}
		sq_offset_base = (void *)((uintptr_t)info->sq_base +
					j * info->sqe_size);
		ret = hisi_qm_recv_sqe(sq_offset_base, info, i);
		if (ret < 0) {
			WD_ERR("recv sqe error %d\n", j);
			return -EIO;
		}
	} else
		return -EAGAIN;

	*resp = info->req_cache[i];
	info->req_cache[i] = NULL;

	if (i == (QM_Q_DEPTH - 1)) {
		info->cqc_phase = !(info->cqc_phase);
		i = 0;
	} else
		i++;

	info->db(info, DOORBELL_CMD_CQ, i, 0);

	info->cq_head_index = i;
	__atomic_sub_fetch(&info->used, 1, __ATOMIC_RELAXED);

	if (wd_reg_read(info->ds_base) == 1) {
		WD_ERR("wd queue hw error happened in qm receive!\n");
		return -WD_HW_ERR;
	}

	return ret;
}

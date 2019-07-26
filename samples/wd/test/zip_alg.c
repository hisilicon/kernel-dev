// SPDX-License-Identifier: Apache-2.0
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <sys/poll.h>

#include "../wd.h"
#include "../wd_sched.h"
#include "drv/hisi_qm_udrv.h"
#include "zip_usr_if.h"

#include "zip_alg.h"
#include "smm.h"

#define ZLIB_HEADER "\x78\x9c"
#define ZLIB_HEADER_SZ 2

/*
 * We use a extra field for gzip block length. So the fourth byte is \x04.
 * This is necessary because our software don't know the size of block when
 * using an hardware decompresser (It is known by hardware). This help our
 * decompresser to work and helpfully, compatible with gzip.
 */
#define GZIP_HEADER "\x1f\x8b\x08\x04\x00\x00\x00\x00\x00\x03"
#define GZIP_HEADER_SZ 10
#define GZIP_EXTRA_SZ 10
#define GZIP_TAIL_SZ 8

/* bytes of data for a request */
#define BLOCK_SIZE 512000
#define REQ_CACHE_NUM 4
#define Q_NUM 1

struct hizip_priv {
	int alg_type;
	int op_type;
	int block_size;
	int dw9;
	int total_len;
	int out_len;
	struct hisi_zip_sqe *msgs;
	void *src, *dst;
	int is_fd;
};

enum alg_op_type {
	COMPRESS,
	DECOMPRESS,
};

/* block mode api use wd_schedule interface */

static void hizip_wd_sched_init_cache(struct wd_scheduler *sched, int i)
{
	struct wd_msg *wd_msg = &sched->msgs[i];
	struct hisi_zip_sqe *msg;
	struct hizip_priv *priv = sched->priv;
	void *data_in, *data_out;

	msg = wd_msg->msg = &priv->msgs[i];
	msg->dw9 = priv->dw9;
	msg->dest_avail_out = sched->msg_data_size;
	if (sched->qs[0].dev_flags & UACCE_DEV_NOIOMMU) {
		data_in = wd_get_pa_from_va(&sched->qs[0], wd_msg->data_in);
		data_out = wd_get_pa_from_va(&sched->qs[0], wd_msg->data_out);
	} else {
		data_in = wd_msg->data_in;
		data_out = wd_msg->data_out;
	}
	msg->source_addr_l = (__u64)data_in & 0xffffffff;
	msg->source_addr_h = (__u64)data_in >> 32;
	msg->dest_addr_l = (__u64)data_out & 0xffffffff;
	msg->dest_addr_h = (__u64)data_out >> 32;

	dbg("init sched cache %d: %p, %p\n", i, wd_msg, msg);
}

static int hizip_wd_sched_input(struct wd_msg *msg, void *priv)
{
	size_t ilen, templen, real_len;
	struct hisi_zip_sqe *m = msg->msg;
	struct hizip_priv *zip_priv = priv;

	ilen = zip_priv->total_len > zip_priv->block_size ?
		zip_priv->block_size : zip_priv->total_len;
	templen = ilen;
	zip_priv->total_len -= ilen;
	if (zip_priv->op_type == INFLATE) {
		if (zip_priv->alg_type == ZLIB) {
			zip_priv->src += ZLIB_HEADER_SZ;
			ilen -= ZLIB_HEADER_SZ;
		} else {
			ilen -= GZIP_HEADER_SZ;
			if (*((char *)zip_priv->src + 3) == 0x04) {
				zip_priv->src += GZIP_HEADER_SZ;
				memcpy(&ilen, zip_priv->src + 6, 4);
				zip_priv->src += GZIP_EXTRA_SZ;
				dbg("gzip iuput len %ld\n", ilen);
				SYS_ERR_COND(ilen > zip_priv->block_size * 2,
				"gzip protocol_len(%ld) > dmabuf_size(%d)\n",
				ilen, zip_priv->block_size);
				real_len = GZIP_HEADER_SZ +
							GZIP_EXTRA_SZ + ilen;
				zip_priv->total_len = zip_priv->total_len +
					      templen - real_len;
			} else
				zip_priv->src += GZIP_HEADER_SZ;
		}
	}

	memcpy(msg->data_in, zip_priv->src, ilen);
	zip_priv->src += ilen;

	m->input_data_length = ilen;
	dbg("zip input ilen= %d, block_size= %d, total_len= %d\n",
	    ilen, zip_priv->block_size, zip_priv->total_len);

	dbg("zip input(%p, %p): %x, %x, %x, %x, %d, %d\n",
	    msg, m,
	    m->source_addr_l, m->source_addr_h,
	    m->dest_addr_l, m->dest_addr_h,
	    m->dest_avail_out, m->input_data_length);

	return 0;
}

static int hizip_wd_sched_output(struct wd_msg *msg, void *priv)
{
	struct hisi_zip_sqe *m = msg->msg;
	__u32 status = m->dw3 & 0xff;
	__u32 type = m->dw9 & 0xff;
	struct hizip_priv *zip_priv = priv;
	char gzip_extra[GZIP_EXTRA_SZ] = {0x00, 0x07, 0x48, 0x69, 0x00, 0x04,
					  0x00, 0x00, 0x00, 0x00};

	dbg("zip output(%p, %p): %x, %x, %x, %x, %d, %d, consume=%d, out=%d\n",
	    msg, m,
	    m->source_addr_l, m->source_addr_h,
	    m->dest_addr_l, m->dest_addr_h,
	    m->dest_avail_out, m->input_data_length, m->consumed, m->produced);

	if (status != 0 && status != 0x0d) {
		WD_ERR("bad status (s=%d, t=%d)\n", status, type);
		return -EIO;
	}
	if (zip_priv->op_type == DEFLATE) {

		if (zip_priv->alg_type == ZLIB) {
			memcpy(zip_priv->dst,
				       ZLIB_HEADER, ZLIB_HEADER_SZ);
			zip_priv->dst += ZLIB_HEADER_SZ;
			zip_priv->out_len += ZLIB_HEADER_SZ;
		} else {
			memcpy(gzip_extra + 6, &m->produced, 4);
			memcpy(zip_priv->dst, GZIP_HEADER,
				      GZIP_HEADER_SZ);
			zip_priv->dst += GZIP_HEADER_SZ;
			zip_priv->out_len += GZIP_HEADER_SZ;
			memcpy(zip_priv->dst, gzip_extra,
					GZIP_EXTRA_SZ);
			zip_priv->dst += GZIP_EXTRA_SZ;
			zip_priv->out_len += GZIP_EXTRA_SZ;
		}
	}

	memcpy(zip_priv->dst, msg->data_out, m->produced);
	zip_priv->dst += m->produced;
	zip_priv->out_len += m->produced;

	return 0;
}

static int hizip_init(struct wd_scheduler *sched, int alg_type, int op_type,
		      int blk_size)
{
	int ret = -ENOMEM, i;
	char *alg;
	struct hisi_qm_priv *priv;
	struct hizip_priv *zip_priv;

	sched->q_num = Q_NUM;
	sched->ss_region_size = 0; /* let system make decision */
	sched->msg_cache_num = REQ_CACHE_NUM;
	/* use twice size of the input data, hope it is engouth for output */
	sched->msg_data_size = blk_size * 2;
	sched->init_cache = hizip_wd_sched_init_cache;
	sched->input = hizip_wd_sched_input;
	sched->output = hizip_wd_sched_output;

	sched->qs = calloc(sched->q_num, sizeof(struct wd_queue));
	if (!sched->qs)
		return -ENOMEM;

	zip_priv = calloc(1, sizeof(struct hizip_priv));
	if (!zip_priv)
		goto err_with_qs;

	zip_priv->msgs = calloc(sched->msg_cache_num,
				sizeof(struct hisi_zip_sqe));
	if (!zip_priv->msgs)
		goto err_with_priv;

	zip_priv->alg_type = alg_type;
	zip_priv->op_type = op_type;
	zip_priv->block_size = blk_size;
	if (alg_type == ZLIB) {
		alg = "zlib";
		zip_priv->dw9 = 2;
	} else {
		alg = "gzip";
		zip_priv->dw9 = 3;
	}

	for (i = 0; i < sched->q_num; i++) {
		sched->qs[i].capa.alg = alg;
		priv = (struct hisi_qm_priv *)sched->qs[i].capa.priv;
		priv->sqe_size = sizeof(struct hisi_zip_sqe);
		priv->op_type = zip_priv->op_type;
	}

	sched->priv = zip_priv;
	ret = wd_sched_init(sched);
	if (ret)
		goto err_with_msgs;

	return 0;

err_with_msgs:
	free(zip_priv->msgs);
err_with_priv:
	free(zip_priv);
err_with_qs:
	free(sched->qs);
	return ret;
}

static void hizip_fini(struct wd_scheduler *sched)
{
	struct hizip_priv *zip_priv = sched->priv;

	wd_sched_fini(sched);
	free(zip_priv->msgs);
	free(zip_priv);
	free(sched->qs);
}

/**
 * compress() - compress memory buffer.
 * @alg_type: alg_type.
 *
 * This function compress memory buffer.
 */
int hw_blk_compress(int alg_type, int blksize,
		    unsigned char *dst, ulong *dstlen,
		    unsigned char *src, ulong srclen)
{
	int ret;
	struct wd_scheduler sched;
	struct hizip_priv *zip_priv;

	if (blksize < 0 || dst == NULL || src == NULL)
		return -EINVAL;
	memset(&sched, 0, sizeof(struct wd_scheduler));
	ret = hizip_init(&sched, alg_type, DEFLATE, blksize);
	if (ret) {
		WD_ERR("fail to hizip init!\n");
		return ret;
	}
	zip_priv = sched.priv;
	zip_priv->total_len = srclen;
	zip_priv->src = src;
	zip_priv->dst = dst;
	zip_priv->is_fd = 0;

	while (zip_priv->total_len || !wd_sched_empty(&sched)) {
		dbg("request loop: total_len=%d\n", zip_priv->total_len);
		ret = wd_sched_work(&sched, zip_priv->total_len);
		if (ret < 0) {
			WD_ERR("wd_sched_work fail, ret=%d!\n", ret);
			return ret;
		}
	}

	*dstlen = zip_priv->out_len;
	hizip_fini(&sched);

	return ret;
}

int hw_blk_decompress(int alg_type, int blksize,
		      unsigned char *dst, ulong *dstlen,
		      unsigned char *src, ulong srclen)
{
	int ret;
	struct wd_scheduler sched;
	struct hizip_priv *zip_priv;

	if (blksize < 0 || dst == NULL || src == NULL)
		return -EINVAL;
	memset(&sched, 0, sizeof(struct wd_scheduler));
	ret = hizip_init(&sched, alg_type, INFLATE, blksize);
	if (ret) {
		WD_ERR("fail to hizip init!\n");
		return ret;
	}
	zip_priv = sched.priv;
	zip_priv->total_len = srclen;
	zip_priv->src = src;
	zip_priv->dst = dst;
	zip_priv->is_fd = 0;

	while (zip_priv->total_len || !wd_sched_empty(&sched)) {
		dbg("request loop: total_len=%d\n", zip_priv->total_len);
		ret = wd_sched_work(&sched, zip_priv->total_len);
		if (ret < 0) {
			WD_ERR("wd_sched_work fail, ret=%d!\n", ret);
			return ret;
		}
	}

	*dstlen = zip_priv->out_len;
	hizip_fini(&sched);

	return ret;
}

/* stream api  */
#define ST_ZLIB_HEADER "\x78\x9c"
#define ST_GZIP_HEADER "\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x03"

#define ST_ZLIB_HEADER_SZ 2
#define ST_GZIP_HEADER_SZ 10

#define EMPTY_ZLIB_APPEND "\x03\x00\x00\x00\x00\x01"
#define EMPTY_ZLIB_SZ 6
#define EMPTY_GZIP_APPEND "\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00"
#define EMPTY_GZIP_SZ 10

#define DMEMSIZE (1024 * 1024)	/* 1M */
#define HW_CTX_SIZE (64 * 1024)
#define Z_OK            0
#define Z_STREAM_END    1
#define Z_ERRNO (-1)
#define Z_STREAM_ERROR (-EIO)

#define swab32(x) \
	((((x) & 0x000000ff) << 24) | \
	(((x) & 0x0000ff00) <<  8) | \
	(((x) & 0x00ff0000) >>  8) | \
	(((x) & 0xff000000) >> 24))

#define cpu_to_be32(x) swab32(x)

/*wrap as zlib basic interface */
#define HZLIB_VERSION "1.0.1"

#ifndef MAX_WBITS
#define MAX_WBITS   15 /* 32K LZ77 window */
#endif

/* compression levels */
#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)


/* Maximum value for memLevel in deflateInit2 */
#ifndef MAX_MEM_LEVEL
#ifdef MAXSEG_64K
#define MAX_MEM_LEVEL 8
#else
#define MAX_MEM_LEVEL 9
#endif
#endif

/* default memLevel */
#ifndef DEF_MEM_LEVEL
#if MAX_MEM_LEVEL >= 8
#define DEF_MEM_LEVEL 8
#else
#define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif

/* compression strategy; see deflateInit2() below for details */
#define Z_DEFAULT_STRATEGY    0

/* default windowBits for decompression. MAX_WBITS is for compression only */
#ifndef DEF_WBITS
#define DEF_WBITS MAX_WBITS
#endif

/* The deflate compression method (the only one supported in this version) */
#define Z_DEFLATED   8

struct hw_ctl {
	struct wd_queue *q;
	int alg_type;
	int stream_pos;
	void *ctx_buf;
	int ctx_dw0;
	int ctx_dw1;
	int ctx_dw2;
	void *next_in_pa;   /* next input byte */
	void *temp_in_pa;   /* temp input byte */
	void *next_out_pa;  /* next output byte should be put there */
	void *ss_buf;
	int isize;
	int checksum;
	int resend;
	int recv;
};

struct zip_stream {
	void *next_in;   /* next input byte */
	unsigned long  avail_in;  /* number of bytes available at next_in */
	unsigned long  total_in;  /* total nb of input bytes read so far */
	void  *next_out;  /* next output byte should be put there */
	unsigned long avail_out; /* remaining free space at next_out */
	unsigned long    total_out; /* total nb of bytes output so far */
	char     *msg;      /* last error message, NULL if no error */
	void     *workspace; /* memory allocated for this stream */
	int     data_type;  /*the data type: ascii or binary */
	unsigned long   adler;      /* adler32 value of the uncompressed data */
	void *reserved;   /* reserved for future use */
};

#define hw_deflateInit(strm, level) \
	hw_deflateInit_(strm, level, HZLIB_VERSION, sizeof(struct zip_stream))
#define hw_inflateInit(strm) \
	hw_inflateInit_(strm, HZLIB_VERSION, (int)sizeof(struct zip_stream))
#define hw_deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
	hw_deflateInit2_(strm, level, method, windowBits, memLevel,\
		     (strategy), HZLIB_VERSION, (int)sizeof(struct zip_stream))
#define hw_inflateInit2(strm, windowBits) \
	hw_inflateInit2_(strm, windowBits, HZLIB_VERSION, \
		     (int)sizeof(struct zip_stream))

static int stream_chunk = 1024 * 64;

static int hw_init(struct zip_stream *zstrm, int alg_type, int comp_optype)
{
	int ret;
	size_t ss_region_size;
	struct hisi_qm_priv *priv;
	struct hw_ctl *hw_ctl;

	hw_ctl = calloc(1, sizeof(struct hw_ctl));
	if (hw_ctl == NULL)
		return -ENOMEM;
	hw_ctl->q = calloc(1, sizeof(struct wd_queue));
	if (hw_ctl->q == NULL) {
		ret = -ENOMEM;
		fprintf(stderr, "alloc hw_ctl->q fail, ret =%d\n", ret);
		goto hw_free;
	}

	switch (alg_type) {
	case 0:
		hw_ctl->alg_type = HW_ZLIB;
		hw_ctl->q->capa.alg = "zlib";
		break;
	case 1:
		hw_ctl->alg_type = HW_GZIP;
		hw_ctl->q->capa.alg = "gzip";
		break;
	default:
		hw_ctl->alg_type = HW_ZLIB;
		hw_ctl->q->capa.alg = "zlib";
	}
	hw_ctl->q->capa.latency = 0;   /*todo..*/
	hw_ctl->q->capa.throughput = 0;
	priv = (struct hisi_qm_priv *)hw_ctl->q->capa.priv;
	priv->sqe_size = sizeof(struct hisi_zip_sqe);
	priv->op_type = comp_optype;
	ret = wd_request_queue(hw_ctl->q);
	if (ret) {
		fprintf(stderr, "wd_request_queue fail, ret =%d\n", ret);
		goto hw_q_free;
	}
	SYS_ERR_COND(ret, "wd_request_queue");

	ss_region_size = 4096 + DMEMSIZE * 2 + HW_CTX_SIZE;

#ifdef CONFIG_IOMMU_SVA
		hw_ctl->ss_buf = calloc(1, ss_region_size);
#else
		hw_ctl->ss_buf = wd_reserve_memory(hw_ctl->q, ss_region_size);
#endif
	if (!hw_ctl->ss_buf) {
		fprintf(stderr, "fail to reserve %ld dmabuf\n", ss_region_size);
		ret = -ENOMEM;
		goto release_q;
	}

	ret = smm_init(hw_ctl->ss_buf, ss_region_size, 0xF);
	if (ret)
		goto buf_free;

	zstrm->next_in = NULL;
	zstrm->next_in = NULL;
	hw_ctl->ctx_buf = NULL;

	zstrm->next_in = smm_alloc(hw_ctl->ss_buf, DMEMSIZE);
	zstrm->next_out = smm_alloc(hw_ctl->ss_buf, DMEMSIZE);
	hw_ctl->ctx_buf = smm_alloc(hw_ctl->ss_buf, HW_CTX_SIZE);

	if (zstrm->next_in == NULL || zstrm->next_out == NULL ||
	    hw_ctl->ctx_buf == NULL) {
		dbg("not enough data ss_region memory for cache (bs=%d)\n",
		    DMEMSIZE);
		goto buf_free;
	}

	if (hw_ctl->q->dev_flags & UACCE_DEV_NOIOMMU) {
		hw_ctl->next_in_pa = wd_get_pa_from_va(hw_ctl->q,
						       zstrm->next_in);
		hw_ctl->next_out_pa = wd_get_pa_from_va(hw_ctl->q,
							zstrm->next_out);
		hw_ctl->ctx_buf = wd_get_pa_from_va(hw_ctl->q, hw_ctl->ctx_buf);
	} else {
		hw_ctl->next_in_pa = zstrm->next_in;
		hw_ctl->next_out_pa = zstrm->next_out;
	}

	hw_ctl->temp_in_pa = hw_ctl->next_in_pa;
	hw_ctl->stream_pos = STREAM_NEW;
	zstrm->reserved = hw_ctl;

	return Z_OK;
buf_free:
#ifdef CONFIG_IOMMU_SVA
		if (hw_ctl->ss_buf)
			free(hw_ctl->ss_buf);
#endif
release_q:
	wd_release_queue(hw_ctl->q);
hw_q_free:
	free(hw_ctl->q);
hw_free:
	free(hw_ctl);

	return ret;
}

static void hw_end(struct zip_stream *zstrm)
{
	struct hw_ctl *hw_ctl = (struct hw_ctl *)zstrm->reserved;

#ifdef CONFIG_IOMMU_SVA
	if (hw_ctl->ss_buf)
		free(hw_ctl->ss_buf);
#endif

	wd_release_queue(hw_ctl->q);
	free(hw_ctl->q);
	free(hw_ctl);
}

static unsigned int bit_reverse(register unsigned int x)
{
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));

	return((x >> 16) | (x << 16));
}

/* output an empty store block */
static int append_store_block(struct zip_stream *zstrm, int flush)
{
	char store_block[5] = {0x1, 0x00, 0x00, 0xff, 0xff};
	struct hw_ctl *hw_ctl = (struct hw_ctl *)zstrm->reserved;
	__u32 checksum = hw_ctl->checksum;
	__u32 isize = hw_ctl->isize;

	memcpy(zstrm->next_out, store_block, 5);
	zstrm->total_out += 5;
	zstrm->avail_out -= 5;
	if (flush != WD_FINISH)
		return Z_STREAM_END;

	if (hw_ctl->alg_type == HW_ZLIB) { /*if zlib, ADLER32*/
		checksum = (__u32) cpu_to_be32(checksum);
		memcpy(zstrm->next_out + 5, &checksum, 4);
		zstrm->total_out += 4;
		zstrm->avail_out -= 4;
	} else if (hw_ctl->alg_type == HW_GZIP) {  /*if gzip, CRC32 and ISIZE*/
		checksum = ~checksum;
		checksum = bit_reverse(checksum);
		memcpy(zstrm->next_out + 5, &checksum, 4);
		memcpy(zstrm->next_out + 9, &isize, 4);
		zstrm->total_out += 8;
		zstrm->avail_out -= 8;
	} else
		fprintf(stderr, "in append store block, wrong alg type %d.\n",
			hw_ctl->alg_type);

	return Z_STREAM_END;
}

static int hw_send_and_recv(struct zip_stream *zstrm, int flush)
{
	struct hisi_zip_sqe *msg, *recv_msg;
	struct hw_ctl *hw_ctl = (struct hw_ctl *)zstrm->reserved;
	int ret = 0;
	__u32 status, type;
	__u64 stream_mode, stream_new, flush_type;

	if (zstrm->avail_in == 0)
		return append_store_block(zstrm, flush);

	msg = calloc(1, sizeof(*msg));
	if (!msg) {
		fputs("alloc msg fail!\n", stderr);
		goto msg_free;
	}
	hw_ctl->resend = 0;
	hw_ctl->recv = 0;
	stream_mode = STATEFUL;
	stream_new = hw_ctl->stream_pos;
	flush_type = (flush == WD_FINISH) ? HZ_FINISH : HZ_SYNC_FLUSH;

	memset((void *)msg, 0, sizeof(*msg));
	msg->dw9 = hw_ctl->alg_type;
	msg->dw7 |= ((stream_new << 2 | stream_mode << 1 |
			flush_type)) << STREAM_FLUSH_SHIFT;
	msg->source_addr_l = (__u64)hw_ctl->next_in_pa & 0xffffffff;
	msg->source_addr_h = (__u64)hw_ctl->next_in_pa >> 32;
	msg->dest_addr_l = (__u64)hw_ctl->next_out_pa & 0xffffffff;
	msg->dest_addr_h = (__u64)hw_ctl->next_out_pa >> 32;
	msg->input_data_length = zstrm->avail_in;
	if (zstrm->avail_out > 4096)
		msg->dest_avail_out = zstrm->avail_out;
	else
		msg->dest_avail_out = 4096;
	msg->stream_ctx_addr_l = (__u64)hw_ctl->ctx_buf & 0xffffffff;
	msg->stream_ctx_addr_h = (__u64)hw_ctl->ctx_buf >> 32;
	msg->ctx_dw0 = hw_ctl->ctx_dw0;
	msg->ctx_dw1 = hw_ctl->ctx_dw1;
	msg->ctx_dw2 = hw_ctl->ctx_dw2;
	msg->isize = hw_ctl->isize;
	msg->checksum = hw_ctl->checksum;
	if (hw_ctl->stream_pos == STREAM_NEW) {
		hw_ctl->stream_pos = STREAM_OLD;
		zstrm->total_out = 0;
	}
resend:
	ret = wd_send(hw_ctl->q, msg);
	if (ret == -EBUSY) {
		usleep(1);
		if (++hw_ctl->resend > 1000) {
			fputs(" wd_resend, timeout fail!\n", stderr);
			goto msg_free;
		}
		goto resend;
	}

	SYS_ERR_COND(ret, "send fail!\n");
recv_again:
	ret = wd_recv(hw_ctl->q, (void **)&recv_msg);
	if (ret == -EIO) {
		fputs(" wd_recv fail!\n", stderr);
		goto msg_free;
	/* synchronous mode, if get none, then get again */
	} else if (ret == -EAGAIN) {
		usleep(1);
		if (++hw_ctl->recv > 1000) {
			fputs(" wd_recv timeout, fail!\n", stderr);
			goto msg_free;
		}
		goto recv_again;
	}
	status = recv_msg->dw3 & 0xff;
	type = recv_msg->dw9 & 0xff;

	if (status != 0 && status != 0x0d && status != 0x13) {
		WD_ERR("bad status (s=%d, t=%d)\n", status, type);
		return -EIO;
	}
	zstrm->avail_out -= recv_msg->produced;
	zstrm->total_out += recv_msg->produced;
	zstrm->avail_in -= recv_msg->consumed;
	hw_ctl->ctx_dw0 = recv_msg->ctx_dw0;
	hw_ctl->ctx_dw1 = recv_msg->ctx_dw1;
	hw_ctl->ctx_dw2 = recv_msg->ctx_dw2;
	hw_ctl->isize = recv_msg->isize;
	hw_ctl->checksum = recv_msg->checksum;
	if (zstrm->avail_in > 0)
		hw_ctl->next_in_pa +=  recv_msg->consumed;
	if (zstrm->avail_in == 0)
		hw_ctl->next_in_pa = hw_ctl->temp_in_pa;

	if (ret == 0 && flush == WD_FINISH)
		ret = Z_STREAM_END;
	else if (ret == 0 &&  (recv_msg->dw3 & 0x1ff) == 0x113)
		ret = Z_STREAM_END;    /* decomp_is_end  region */

msg_free:
	free(msg);
	return ret;
}

int hw_deflateInit2_(struct zip_stream *zstrm, int level, int method,
		  int windowBits, int memLevel, int strategy,
		  const char *version, int stream_size)
{
	int alg_type;
	int wrap = 0;

	if (windowBits < 0) { /* suppress zlib wrapper */
		wrap = 0;
		windowBits = -windowBits;
	}
#ifdef GZIP
	else if (windowBits > 15) {
		wrap = 2;		/* write gzip wrapper instead */
		windowBits -= 16;
	}
#endif
	if (wrap & 0x02)
		alg_type = GZIP;
	else
		alg_type = ZLIB;

	return hw_init(zstrm, alg_type, HW_DEFLATE);

}

int hw_deflateInit_(struct zip_stream *zstrm, int level, const char *version,
		 int stream_size)
{
	if (zstrm == NULL)
		return -EINVAL;

	return hw_deflateInit2_(zstrm, level, Z_DEFLATED,
				MAX_WBITS, DEF_MEM_LEVEL,
				Z_DEFAULT_STRATEGY, version, stream_size);
}

int hw_deflate(struct zip_stream *zstrm, int flush)
{
	int ret;

	if (zstrm == NULL)
		return -EINVAL;
	ret = hw_send_and_recv(zstrm, flush);
	if (ret < 0)
		return Z_STREAM_ERROR;
	return ret;
}

int hw_deflateEnd(struct zip_stream *zstrm)
{
	if (zstrm == NULL)
		return -EINVAL;
	hw_end(zstrm);
	return 0;
}

int hw_inflateInit2_(struct zip_stream *zstrm, int windowBits,
		  const char *version, int stream_size)
{
	int wrap, alg_type;

	/* extract wrap request from windowBits parameter */
	if (windowBits < 0) {
		wrap = 0;
		windowBits = -windowBits;
	} else {
		wrap = (windowBits >> 4) + 5;

	}
	if (wrap & 0x01)
		alg_type = ZLIB;
	if (wrap & 0x02)
		alg_type = GZIP;

	return hw_init(zstrm, alg_type, HW_INFLATE);
}

int hw_inflateInit_(struct zip_stream *zstrm,
		    const char *version, int stream_size)
{
	if (zstrm == NULL)
		return -EINVAL;
	return hw_inflateInit2_(zstrm, DEF_WBITS, version, stream_size);
}

int hw_inflate(struct zip_stream *zstrm, int flush)
{
	int ret;

	if (zstrm == NULL)
		return -EINVAL;
	ret = hw_send_and_recv(zstrm, flush);
	if (ret < 0)
		return Z_STREAM_ERROR;
	return ret;
}

int hw_inflateEnd(struct zip_stream *zstrm)
{
	if (zstrm == NULL)
		return -EINVAL;
	hw_end(zstrm);
	return 0;
}

int hw_stream_compress(int alg_type, int blksize,
		       unsigned char *dst, ulong *dstlen,
		       unsigned char *src, ulong srclen)
{
	int flush, have;
	int ret;
	int level = 0;
	struct zip_stream zstrm;
	int windowBits = 15;
	int GZIP_ENCODING = 16;

	if (blksize < 0 || dst == NULL || src == NULL)
		return -EINVAL;
	stream_chunk = blksize;
	*dstlen = 0;

	/* add zlib compress head and write head + compressed date to a file */
	if (alg_type == ZLIB) {
		ret = hw_deflateInit(&zstrm, level);
		if (ret != Z_OK)
			return ret;
		memcpy(dst, ST_ZLIB_HEADER, ST_ZLIB_HEADER_SZ);
		dst += ST_ZLIB_HEADER_SZ;
		*dstlen += ST_ZLIB_HEADER_SZ;
	} else {
		/* deflate for gzip data */
		ret = hw_deflateInit2(&zstrm, Z_DEFAULT_COMPRESSION,
				   Z_DEFLATED, windowBits | GZIP_ENCODING, 8,
				   Z_DEFAULT_STRATEGY);
		if (ret != Z_OK)
			return ret;
		memcpy(dst, ST_GZIP_HEADER, ST_GZIP_HEADER_SZ);
		dst += ST_GZIP_HEADER_SZ;
		*dstlen += ST_GZIP_HEADER_SZ;
	}
	do {
		if (srclen > stream_chunk) {
			memcpy(zstrm.next_in, src, stream_chunk);
			src += stream_chunk;
			zstrm.avail_in = stream_chunk;
			srclen -= stream_chunk;
		} else {
			memcpy(zstrm.next_in, src, srclen);
			zstrm.avail_in = srclen;
			srclen = 0;
		}
		flush = srclen ? WD_SYNC_FLUSH : WD_FINISH;
		do {
			zstrm.avail_out = stream_chunk;
			ret = hw_deflate(&zstrm, flush);
			ASSERT(ret != Z_STREAM_ERROR);
			have = stream_chunk - zstrm.avail_out;
			memcpy(dst, zstrm.next_out, have);
			dst += have;
			*dstlen += have;
		} while (zstrm.avail_in > 0);
		ASSERT(zstrm.avail_in == 0);   /* all input will be used */

		/* done when last data in file processed */
	} while (flush != WD_FINISH);

	ASSERT(ret == Z_STREAM_END);       /* stream will be complete */
	hw_end(&zstrm);

	return Z_OK;
}

int hw_stream_decompress(int alg_type, int blksize,
			 unsigned char *dst, ulong *dstlen,
			 unsigned char *src, ulong srclen)
{
	int have;
	int ret;
	struct zip_stream zstrm;

	if (blksize < 0 || dst == NULL || src == NULL)
		return -EINVAL;
	stream_chunk = blksize;
	*dstlen = 0;

	if (alg_type == ZLIB) {
		ret = hw_inflateInit(&zstrm);
		if (ret != Z_OK)
			return ret;
		src += ST_ZLIB_HEADER_SZ;
		srclen -= ST_ZLIB_HEADER_SZ;
	} else {
		ret = hw_inflateInit2(&zstrm, 16 + MAX_WBITS);
		if (ret != Z_OK)
			return ret;
		src += ST_GZIP_HEADER_SZ;
		srclen -= ST_GZIP_HEADER_SZ;
	}
	do {
		if (srclen > stream_chunk) {
			memcpy(zstrm.next_in, src, stream_chunk);
			src += stream_chunk;
			zstrm.avail_in = stream_chunk;
			srclen -= stream_chunk;
		} else {
			memcpy(zstrm.next_in, src, srclen);
			zstrm.avail_in = srclen;
			srclen = 0;
		}

		if (zstrm.avail_in == 0) {
			ret = Z_STREAM_END;
			break;
		}
		/* finish compression if all of source has been read in */
		do {
			zstrm.avail_out = stream_chunk;
			ret = hw_inflate(&zstrm, WD_SYNC_FLUSH);
			ASSERT(ret != Z_STREAM_ERROR);
			have = stream_chunk - zstrm.avail_out;
			memcpy(dst, zstrm.next_out, have);
			dst += have;
			*dstlen += have;
		} while (zstrm.avail_in > 0);
		ASSERT(zstrm.avail_in == 0);    /* all input will be used */

		/* done when last data in file processed */
	} while (ret != Z_STREAM_END);

	ASSERT(ret == Z_STREAM_END);            /* stream will be complete */
	hw_end(&zstrm);
	return Z_OK;
}

int hw_stream_def(FILE *source, FILE *dest,  int alg_type)
{
	int flush, have;
	int ret;
	int level = 0;
	struct zip_stream zstrm;
	int windowBits = 15;
	int GZIP_ENCODING = 16;
	int fd, file_len;
	struct stat s;

	fd = fileno(source);
	SYS_ERR_COND(fstat(fd, &s) < 0, "fstat");
	file_len = s.st_size;
	if (!file_len) {
		if (alg_type == ZLIB) {
			fwrite(ST_ZLIB_HEADER, 1, ST_ZLIB_HEADER_SZ, dest);
			fwrite(EMPTY_ZLIB_APPEND, 1, EMPTY_ZLIB_SZ, dest);
			return Z_OK;
		} else if (alg_type == GZIP) {
			fwrite(ST_GZIP_HEADER, 1, ST_GZIP_HEADER_SZ, dest);
			fwrite(EMPTY_GZIP_APPEND, 1, EMPTY_GZIP_SZ, dest);
			return Z_OK;
		} else
			return -EINVAL;
	}
	/* add zlib compress head and write head + compressed date to a file */
	if (alg_type == ZLIB) {
		ret = hw_deflateInit(&zstrm, level);
		if (ret != Z_OK)
			return ret;
		fwrite(ST_ZLIB_HEADER, 1, ST_ZLIB_HEADER_SZ, dest);
	} else {
		/* deflate for gzip data */
		ret = hw_deflateInit2(&zstrm, Z_DEFAULT_COMPRESSION,
				   Z_DEFLATED, windowBits | GZIP_ENCODING, 8,
				   Z_DEFAULT_STRATEGY);
		if (ret != Z_OK)
			return ret;
		fwrite(ST_GZIP_HEADER, 1, ST_GZIP_HEADER_SZ, dest);
	}
	do {

		zstrm.avail_in =  fread(zstrm.next_in, 1, stream_chunk, source);
		flush = feof(source) ? WD_FINISH : WD_SYNC_FLUSH;
		do {
			zstrm.avail_out = stream_chunk;
			ret = hw_deflate(&zstrm, flush);
			ASSERT(ret != Z_STREAM_ERROR);
			have = stream_chunk - zstrm.avail_out;
			if (fwrite(zstrm.next_out, 1, have, dest) != have ||
				ferror(dest)) {
				fprintf(stderr, "errno =%d\n", errno);
				(void)hw_end(&zstrm);
				return Z_ERRNO;
			}
		} while (zstrm.avail_in > 0);
		ASSERT(zstrm.avail_in == 0);   /* all input will be used */

		/* done when last data in file processed */
	} while (flush != WD_FINISH);

	ASSERT(ret == Z_STREAM_END);       /* stream will be complete */
	hw_end(&zstrm);

	return Z_OK;
}

int hw_stream_inf(FILE *source, FILE *dest,  int alg_type)
{
	int have;
	int ret;
	struct zip_stream zstrm;

	if (alg_type == ZLIB) {
		ret = hw_inflateInit(&zstrm);
		if (ret != Z_OK)
			return ret;
		fseek(source, ST_ZLIB_HEADER_SZ, SEEK_SET);
	} else {
		ret = hw_inflateInit2(&zstrm, 16 + MAX_WBITS);
		if (ret != Z_OK)
			return ret;
		fseek(source, ST_GZIP_HEADER_SZ, SEEK_SET);
	}
	do {
		zstrm.avail_in = fread(zstrm.next_in, 1, stream_chunk, source);
		if (ferror(source)) {
			hw_end(&zstrm);
			return Z_ERRNO;
		}
		if (zstrm.avail_in == 0)
			break;
		/* finish compression if all of source has been read in */
		do {
			zstrm.avail_out = stream_chunk;
			ret = hw_inflate(&zstrm, WD_SYNC_FLUSH);
			ASSERT(ret != Z_STREAM_ERROR);
			have = stream_chunk - zstrm.avail_out;
			if (fwrite(zstrm.next_out, 1, have, dest) != have ||
				ferror(dest)) {
				hw_end(&zstrm);
				return Z_ERRNO;
			}

		} while (zstrm.avail_in > 0);
		ASSERT(zstrm.avail_in == 0);    /* all input will be used */

		/* done when last data in file processed */
	} while (ret != Z_STREAM_END);

	ASSERT(ret == Z_STREAM_END);            /* stream will be complete */
	hw_end(&zstrm);
	return Z_OK;
}


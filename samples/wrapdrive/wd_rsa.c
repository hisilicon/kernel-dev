/*
 * Copyright (c) 2017. Hisilicon Tech Co. Ltd. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "wd.h"
#include "wd_rsa.h"
#include "wd_util.h"


struct wd_rsa_udata {
	void *tag;
	struct wd_rsa_op_data *opdata;
};

struct wd_rsa_ctx {
	struct wd_rsa_msg cache_msg;
	struct wd_queue *q;
	char  alg[32];
	wd_rsa_cb cb;
	__u32 kbits;
};


/* Before initiate this context, we should get a queue from WD */
void *wd_create_rsa_ctx(struct wd_queue *q, struct wd_rsa_ctx_setup *setup)
{
	struct wd_rsa_ctx *ctx;

	if (!q || !setup) {
		WD_ERR("%s(): input param err!\n", __func__);
		return NULL;
	}
	ctx = malloc(sizeof(*ctx));
	if (!ctx) {
		WD_ERR("Alloc ctx memory fail!\n");
		return ctx;
	}
	memset(ctx, 0, sizeof(*ctx));
	ctx->q = q;
	strncpy(ctx->alg, q->capa.alg, strlen(q->capa.alg));
	ctx->cache_msg.aflags = setup->aflags;

	ctx->cache_msg.alg = ctx->alg;
	ctx->cb = setup->cb;
	q->ctx = ctx;

	return ctx;
}

int wd_do_rsa(void *ctx, struct wd_rsa_op_data *opdata)
{
	struct wd_rsa_ctx *ctxt = ctx;
	struct wd_rsa_msg *resp;
	int ret;

	if (opdata->op_type == WD_RSA_SIGN ||
	    opdata->op_type == WD_RSA_VERIFY) {
		ctxt->cache_msg.in = (__u64)opdata->in;
		ctxt->cache_msg.inbytes = (__u16)opdata->in_bytes;
		ctxt->cache_msg.out = (__u64)opdata->out;
	}
	ctxt->cache_msg.pubkey = (__u64)opdata->pubkey;
	ctxt->cache_msg.prikey = (__u64)opdata->prikey;
	ctxt->cache_msg.op_type = (__u8)opdata->op_type;
	ret = wd_send(ctxt->q, &ctxt->cache_msg);
	if (ret) {
		WD_ERR("%s():wd_send err!\n", __func__);
		return ret;
	}
	ret = wd_recv_sync(ctxt->q, (void **)&resp, 0);
	if (ret != 1 || resp->status) {
		WD_ERR("%s():wd_recv_sync err!ret=%d\n", __func__, ret);
		return -1;
	} else {
		*(opdata->out_bytes) = resp->outbytes;
	}

	return 0;
}

int wd_rsa_op(void *ctx, struct wd_rsa_op_data *opdata, void *tag)
{
	struct wd_rsa_ctx *context = ctx;
	struct wd_rsa_msg *msg = &context->cache_msg;
	int ret;
	struct wd_rsa_udata *udata;

	if (!ctx || !opdata) {
		WD_ERR("param err!\n");
		return -1;
	}
	msg->status = 0;

	/* malloc now, as need performance we should rewrite mem management */
	udata = malloc(sizeof(*udata));
	if (!udata) {
		WD_ERR("malloc udata fail!\n");
		return -1;
	}
	udata->tag = tag;
	udata->opdata = opdata;
	if (opdata->op_type == WD_RSA_SIGN ||
	    opdata->op_type == WD_RSA_VERIFY) {
		msg->in = (__u64)opdata->in;
		msg->inbytes = (__u16)opdata->in_bytes;
		msg->out = (__u64)opdata->out;
	}
	msg->pubkey = (__u64)opdata->pubkey;
	msg->prikey = (__u64)opdata->prikey;
	msg->udata = (__u64)udata;
	msg->op_type = (__u8)opdata->op_type;
	ret = wd_send(context->q, (void *)msg);
	if (ret < 0) {
		WD_ERR("wd send request fail!\n");
		return -1;
	}

	return 0;
}

int wd_rsa_poll(struct wd_queue *q, int num)
{
	int ret, count = 0;
	struct wd_rsa_msg *resp;
	struct wd_rsa_ctx *ctx = q->ctx;
	unsigned int status;
	struct wd_rsa_udata *udata;

	do {
		ret = wd_recv(q, (void **)&resp);
		if (ret < 1)
			break;
		count++;
		udata = (void *)resp->udata;
		*(udata->opdata->out_bytes) = (__u32)resp->outbytes;
		status = resp->status;
		ctx->cb(udata->tag, status, udata->opdata);
		free(udata);
	} while (--num);

	return count;
}

void wd_del_rsa_ctx(void *ctx)
{
	if (ctx)
		free(ctx);
}

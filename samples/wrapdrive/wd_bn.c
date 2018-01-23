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
#include "wd_bn.h"
#include "wd_util.h"


struct wd_bn_udata {
	void *tag;
	struct wd_bn_op_data *opdata;
};

struct wd_bn_ctx {
	struct wd_bn_msg cache_msg;
	struct wd_queue *q;
	wd_bn_cb cb;
	char  alg[32];
};


/* Before initiate this context, we should get a queue from WD */
void *wd_create_bn_ctx(struct wd_queue *q, struct wd_bn_ctx_setup *setup)
{
	struct wd_bn_ctx *ctx;

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

int wd_do_bn(void *ctx, struct wd_bn_op_data *opdata)
{
	struct wd_bn_ctx *ctxt = ctx;
	struct wd_bn_msg *resp;
	int ret;

	if (!ctx || !opdata) {
		WD_ERR("%s(): input param err!\n", __func__);
		return -1;
	}
	ctxt->cache_msg.m = (__u64)opdata->m;
	ctxt->cache_msg.mbytes= (__u16)opdata->mbytes;
	ctxt->cache_msg.e = (__u64)opdata->e;
	ctxt->cache_msg.ebytes= (__u16)opdata->ebytes;
	ctxt->cache_msg.b = (__u64)opdata->b;
	ctxt->cache_msg.bbytes= (__u16)opdata->bbytes;
	ctxt->cache_msg.r = (__u64)opdata->r;
	ctxt->cache_msg.op_type = (__u8)opdata->op_type;

	ret = wd_send(ctxt->q, &ctxt->cache_msg);
	if (ret) {
		WD_ERR("%s():wd_send err!\n", __func__);
		return ret;
	}
	ret = wd_recv_sync(ctxt->q, (void **)&resp, 0);
	if (ret != 1) {
		WD_ERR("%s():wd_recv_sync err!ret=%d\n", __func__, ret);
		return ret;
	} else {
		*(opdata->rbytes) = resp->rbytes;
	}

	return 0;
}

int wd_bn_op(void *ctx, struct wd_bn_op_data *opdata, void *tag)
{
	struct wd_bn_ctx *context = ctx;
	struct wd_bn_msg *msg = &context->cache_msg;
	int ret;
	struct wd_bn_udata *udata;

	if (!ctx || !opdata) {
		WD_ERR("%s(): input param err!\n", __func__);
		return -1;
	}
	msg->m = (__u64)opdata->m;
	msg->mbytes= (__u16)opdata->mbytes;
	msg->e = (__u64)opdata->e;
	msg->ebytes= (__u16)opdata->ebytes;
	msg->b = (__u64)opdata->b;
	msg->bbytes= (__u16)opdata->bbytes;
	msg->r = (__u64)opdata->r;
	msg->op_type = (__u8)opdata->op_type;
	msg->status = 0;

	/* malloc now, as need performance we should rewrite mem management */
	udata = malloc(sizeof(*udata));
	if (!udata) {
		WD_ERR("malloc udata fail!\n");
		return -1;
	}
	udata->tag = tag;


	msg->udata = (__u64)udata;
	ret = wd_send(context->q, (void *)msg);
	if (ret < 0) {
		WD_ERR("wd send request fail!\n");
		return -1;
	}

	return 0;
}

int wd_bn_poll(struct wd_queue *q, int num)
{
	int ret, count = 0;
	struct wd_bn_msg *resp;
	struct wd_bn_ctx *ctx = q->ctx;
	unsigned int status;
	struct wd_bn_udata *udata;

	do {
		ret = wd_recv(q, (void **)&resp);
		if (ret < 1)
			break;
		count++;
		udata = (void *)resp->udata;
		*(udata->opdata->rbytes) = (int)resp->rbytes;
		status = resp->status;
		ctx->cb(udata->tag, status, (void *)udata->opdata);
		free(udata);
	} while (--num);

	return count;
}

void wd_del_bn_ctx(void *ctx)
{
	if (ctx)
		free(ctx);
}

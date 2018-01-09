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
#include "wd_cipher.h"
#include "wd_util.h"

struct wd_comp_ctx {
	struct wd_comp_msg cache_msg;
	struct wd_queue *q;
	char  alg[32];
};

/* Before initiate this context, we should get a queue from WD */
void *wd_create_comp_ctx(struct wd_queue *q, struct wd_comp_ctx_setup *setup)
{
	struct wd_comp_ctx *ctx;

	if (!q || !setup) {
		WD_ERR("%s(): input param err!\n", __func__);
		return NULL;
	}
	ctx = malloc(sizeof(*ctx));
	if (!ctx) {
		WD_ERR("Alloc ctx memory fail!\n");
		return ctx
	}
	memset(ctx, 0, sizeof(*ctx));
	ctx->q = q;
	strncpy(ctx->alg, q->capa.alg, strlen(q->capa.alg));
	ctx->cache_msg.aflags = setup->aflags;
	ctx->cache_msg.comp_lv = setup->comp_lv;
	ctx->cache_msg.humm_type = setup->humm_type;
	ctx->cache_msg.win_size = setup->win_size;
	ctx->cache_msg.file_type = setup->file_type;
	ctx->cache_msg.alg = ctx->alg;

	return ctx;
}

int wd_do_comp(void *ctx, __u32 *cflush, char *in, int in_bytes,
		         int *comsumed, char *out, int *out_bytes)
{
	struct wd_comp_ctx *ctxt = ctx;
	struct wd_comp_msg *resp;
	int ret;

	ctxt->cache_msg.cflags = *cflush;
	ctxt->cache_msg.src = (__u64)in;
	ctxt->cache_msg.dst = (__u64)out;
	ctxt->cache_msg.in_bytes = (__u32)in_bytes;

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
		*out_bytes = resp->out_bytes;
		*comsumed = resp->in_coms;
		*cflush = resp->cflags;
	}

	return 0;
}

void wd_del_comp_ctx(void *ctx)
{
	if (ctx)
		free(ctx);
}

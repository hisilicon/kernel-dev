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
	struct wd_comp_msg msg;
	struct wd_queue *q;
};

/* Before initiate this context, we should get a queue from WD */
void *wd_create_comp_ctx(struct wd_queue *q, __u32 alg , __u32 wdw_size,
					enum wd_comp_op op)
{
	return NULL;
}

int wd_do_comp(void *ctx, char *in, char *out, int size)
{
	return -1;
}

void wd_del_comp_ctx(void *ctx)
{
	free(ctx);
}


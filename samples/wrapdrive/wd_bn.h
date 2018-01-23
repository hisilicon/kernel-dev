/*
 * Copyright (c) 2017. Hisilicon Tech Co. Ltd. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef __WD_BN_H
#define __WD_BN_H

#include <stdlib.h>
#include <errno.h>

#include "../../drivers/crypto/hisilicon/wd/wd_usr_if.h"
#include "../../drivers/crypto/hisilicon/wd/wd_bn_usr_if.h"

enum wd_bn_op {
	WD_BN_INVALID,

	/* result = (base ^ exponent) mod modulus*/
	WD_BN_MOD_EXP,
};

typedef void (*wd_bn_cb)(void *tag, int status,  void *opdata);

struct wd_bn_ctx_setup {
	char  *alg;
	wd_bn_cb cb;
	enum wd_bn_op op_type;
	__u16 aflags;
};

struct wd_bn_op_data {
	void *m;
	void *e;
	void *b;

	/* result address */
	void *r;
	__u16 *rbytes;

	__u16 mbytes;
	__u16 ebytes;
	__u16 bbytes;
	enum wd_bn_op op_type;
};

struct wd_bn_msg {

	/* First 8 bytes of the message must indicate algorithm */
	union {
		char  *alg;
		__u64 pading;
	};

	/* address type */
	__u16 aflags;
	__u8 op_type;
	__u8 resv;
	__u32 status;

	/* Comsumed bytes of input data */
	__u64 m;
	__u64 e;
	__u64 b;

	/* result address */
	__u64 r;

	__u16 mbytes;
	__u16 ebytes;
	__u16 bbytes;
	__u16 rbytes;
	__u64 udata;
};

void *wd_create_bn_ctx(struct wd_queue *q, struct wd_bn_ctx_setup *setup);
int wd_do_bn(void *ctx, struct wd_bn_op_data *opdata);
int wd_bn_op(void *ctx, struct wd_bn_op_data *opdata, void *tag);
int wd_bn_poll(struct wd_queue *q, int num);
void wd_del_bn_ctx(void *ctx);
#endif

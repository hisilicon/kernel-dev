/*
 * Copyright (c) 2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

//#include "../aarch64_build/openssl/bn.h"
#include "./openssl/bn.h"
#include "./openssl/rsa.h"
#include "../wd.h"
#include "../wd_rsa.h"

#define ASIZE (16*4096)

#define SYS_ERR_COND(cond, msg)		\
do {					\
	if (cond) {			\
		perror(msg);		\
		exit(EXIT_FAILURE);	\
	}				\
} while (0)

#define OP_NUMBER	1

struct test_wd_bn {
	unsigned long long  *d;
	int top;

	/* The next are internal book keeping for bn_expand. */
	int dmax;
	int neg;
	int flags;
};

int test_rsa_key_gen(void *ctx)
{
	BIGNUM *p, *q, *e_value, *n, *e, *d, *dmp1, *dmq1, *iqmp;
	int ret, bits;
	RSA *test_rsa;
	wd_rsa_prikey *prikey;
	struct wd_rsa_pubkey *pubkey;
	
	bits = wd_rsa_key_bits(ctx);

	test_rsa = RSA_new();
	if (!test_rsa || !bits) {
		WD_ERR("\n RSA new fail!");
		return -1;
	}
	e_value = BN_new();
	if (!e_value){
		RSA_free(test_rsa);
		WD_ERR("\n BN new e fail!");
		ret = -1;
		return ret;
	}
	ret = BN_set_word(e_value, 65537);
	if (1 != ret) {
		WD_ERR("\n BN_set_word fail!");
		ret = -1;
		goto gen_fail;
	}

	ret = RSA_generate_key_ex(test_rsa, 4096, e_value, NULL);
	if (1 != ret) {
		WD_ERR("\n RSA_generate_key_ex fail!");
		ret = -1;
		goto gen_fail;
	}
	RSA_get0_key((const RSA *)test_rsa, (const BIGNUM **)&n,
		       (const BIGNUM **)&e, (const BIGNUM **)&d);
	RSA_get0_factors((const RSA *)test_rsa, (const BIGNUM **)&p, (const BIGNUM **)&q);
	RSA_get0_crt_params((const RSA *)test_rsa, (const BIGNUM **)&dmp1,
			     (const BIGNUM **)&dmq1, (const BIGNUM **)&iqmp);
	printf("\nn: d[0] = 0x%llx, top=%d, dmax=%d", 
	((struct test_wd_bn *)n)->d[0], ((struct test_wd_bn *)n)->top, ((struct test_wd_bn *)n)->dmax);
	printf("\ne: d[0] = 0x%llx, top=%d, dmax=%d", 
	((struct test_wd_bn *)e)->d[0], ((struct test_wd_bn *)e)->top, ((struct test_wd_bn *)e)->dmax);
	printf("\nd: d[0] = 0x%llx, top=%d, dmax=%d", 
	((struct test_wd_bn *)d)->d[0], ((struct test_wd_bn *)d)->top, ((struct test_wd_bn *)d)->dmax);
	printf("\np: d[0] = 0x%llx, top=%d, dmax=%d", 
	((struct test_wd_bn *)p)->d[0], ((struct test_wd_bn *)p)->top, ((struct test_wd_bn *)p)->dmax);
	printf("\nq: d[0] = 0x%llx, top=%d, dmax=%d", 
	((struct test_wd_bn *)q)->d[0], ((struct test_wd_bn *)q)->top, ((struct test_wd_bn *)q)->dmax);
	printf("\ndmp1: d[0] = 0x%llx, top=%d, dmax=%d", 
	((struct test_wd_bn *)dmp1)->d[0], ((struct test_wd_bn *)dmp1)->top, ((struct test_wd_bn *)dmp1)->dmax);
	printf("\ndmq1: d[0] = 0x%llx, top=%d, dmax=%d", 
	((struct test_wd_bn *)dmq1)->d[0], ((struct test_wd_bn *)dmq1)->top, ((struct test_wd_bn *)dmq1)->dmax);
	printf("\niqmp: d[0] = 0x%llx, top=%d, dmax=%d", 
	((struct test_wd_bn *)iqmp)->d[0], ((struct test_wd_bn *)iqmp)->top, ((struct test_wd_bn *)iqmp)->dmax);
	prikey = malloc(sizeof(wd_rsa_prikey));
	pubkey = malloc(sizeof(struct wd_rsa_pubkey));
	if (!prikey || !pubkey) {
		WD_ERR("\n malloc key fail!");
		ret = -1;
		goto gen_fail;
	}
		
	if (wd_rsa_is_crt(ctx)) {
		prikey->pkey2.dp = (__u8 *)((struct test_wd_bn *)dmp1)->d;
		prikey->pkey2.dq = (__u8 *)((struct test_wd_bn *)dmq1)->d;
		prikey->pkey2.p = (__u8 *)((struct test_wd_bn *)p)->d;
		prikey->pkey2.q = (__u8 *)((struct test_wd_bn *)q)->d;
		prikey->pkey2.qinv = (__u8 *)((struct test_wd_bn *)iqmp)->d;
	} else {
		prikey->pkey1.d = (__u8 *)((struct test_wd_bn *)d)->d;
		prikey->pkey1.n = (__u8 *)((struct test_wd_bn *)n)->d;
	}

	pubkey->e = (__u8 *)((struct test_wd_bn *)e)->d;
	pubkey->n = (__u8 *)((struct test_wd_bn *)n)->d;

	ret = wd_set_rsa_pubkey(ctx, pubkey);
	if (ret) {
		WD_ERR("\n wd set pubkey fail!");
		goto set_fail;
	}
	ret = wd_set_rsa_prikey(ctx, prikey);
	if (ret) {
		WD_ERR("\n wd set prikey fail!");
		goto set_fail;
	}
	BN_free(e_value);

	return ret;
set_fail:
	free(prikey);
	free(pubkey);
gen_fail:
	RSA_free(test_rsa);
	BN_free(e_value);

	return ret;
}

#define RSA_KEY_BITS		4096
int main(int argc, char *argv[])
{
	struct wd_capa capa;
	struct wd_queue q;
	struct wd_rsa_msg *msg;
	void *a, *src, *dst;
	int ret, i;
	int output_num;
	struct timeval start_tval, end_tval;
	float time, speed;
	int mode;
	unsigned int pkt_len = (RSA_KEY_BITS >> 3);
	void *ctx = NULL;
	struct wd_rsa_ctx_setup setup;
	struct wd_rsa_op_data opdata;

	if (argv[1])
		mode = strtoul(argv[1], NULL, 10);
	else
		mode = 0;
	memset(&q, 0, sizeof(q));
	memset(&capa, 0, sizeof(capa));
	capa.alg = rsa;
	capa.throughput = 10;
	capa.latency = 10;

	ret = wd_request_queue(&q, &capa);
	SYS_ERR_COND(ret, "wd_request_queue");
	printf("\npasid=%d, dma_flag=%d", q.pasid, q.dma_flag);

	setup.aflags = _WD_AATTR_IOVA;
	setup.alg = rsa;
	setup.key_bits = RSA_KEY_BITS;
	setup.is_crt = mode;
	setup.cb = NULL;

	ctx = wd_create_rsa_ctx(&q, &setup);
	if (!ctx) {
		WD_ERR("\ncreate rsa ctx fail!");
		goto release_q;
	}
	ret = test_rsa_key_gen(ctx);
	if (ret) {
		wd_del_rsa_ctx(ctx);
		goto release_q;
	}

	/* Allocate some space and setup a DMA mapping */
	a = mmap((void *)0x0, ASIZE, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (!a) {
		wd_del_rsa_ctx(ctx);
		printf("\nmmap fail!");
		goto release_q;
	}
	memset(a, 0, ASIZE);
	sleep(5);
	ret = wd_mem_share(&q, a, ASIZE, 0);
	SYS_ERR_COND(ret, "wd_mem_share err\n");
	printf("WD dma map VA=IOVA=%p successfully!\n", a);

	src = a;
	dst = (char *)a + (ASIZE / 2);

	msg = malloc(sizeof(*msg));
	if (!msg) {
		printf("\nalloc msg fail!");
		goto alloc_msg_fail;
	}
	memset((void *)msg, 0, sizeof(*msg));
	gettimeofday(&start_tval, NULL);
	for (i = 0; i < OP_NUMBER; i++) {
		opdata.in_bytes = pkt_len;
		opdata.op_type = WD_RSA_SIGN;
		opdata.in = src;
		opdata.out = dst;
		ret = wd_do_rsa(ctx, &opdata);
		if (ret) {
			free(msg);
			printf("\nwd_do_rsa fail!");
			goto alloc_msg_fail;
		}
	}
	output_num = opdata.out_bytes;
	if (output_num != pkt_len) {
		free(msg);
		printf("\nout put err!");
		goto alloc_msg_fail;
	}
	gettimeofday(&end_tval, NULL);
	time = (float)((end_tval.tv_sec-start_tval.tv_sec) * 1000000 +
		end_tval.tv_usec - start_tval.tv_usec);
	speed = 1 / (time / OP_NUMBER);
	if (mode)
		printf("\r\n%s CRT mode sign time %0.0f us, pkt len ="
		         " %d bytes, %0.3f Mpps", rsa, time, pkt_len, speed);
	else
		printf("\r\n%s NCRT mode sign time %0.0f us, pkt len ="
		         " %d bytes, %0.3f Mpps", rsa, time, pkt_len, speed);
	free(msg);
alloc_msg_fail:
	wd_del_rsa_ctx(ctx);
	wd_mem_unshare(&q, a, ASIZE);
	munmap(a, ASIZE);
release_q:
	wd_release_queue(&q);

	return EXIT_SUCCESS;
}

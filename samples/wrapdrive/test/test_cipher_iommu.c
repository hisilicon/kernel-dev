/*
 * Copyright (c) 2016-2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "wd.h"
#include "test_cipher_common.h"

#define ASIZE (1024 * 1024)
int main(int argc, char *argv[])
{
	int ret, size;
	unsigned char *src, *dst, *iv, *key;
	struct wd_capa capa;
	struct wd_queue q;
	struct wd_cipher_msg *msg, *resp;
	void *a;
	int i, loops;
	int pkt_len;

	if (argv[2]) {
		pkt_len = strtoul(argv[2], NULL, 10);
		if (pkt_len == 0 || pkt_len > 8196) {
			printf("pkt_len error!\n");
			return -1;
		}
	} else
		pkt_len = sizeof(aucDataIn_aes_cbc_128);
	if (argv[1]) {
		loops = strtoul(argv[1], NULL, 10);
		if (loops == 0)
			loops = 1;
		if (loops > ASIZE / pkt_len)
			loops = ASIZE / pkt_len;
	} else
		loops = 1;
	init_capa(&capa, cbc_aes_128);

	ret = wd_request_queue(&q, &capa);
	SYS_ERR_COND(ret, "wd_request_queue");

	/* Allocate some space and setup a DMA mapping */
	a = mmap(0, ASIZE,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	ret = wd_mem_share(&q, a, ASIZE, 0);
	SYS_ERR_COND(ret, "wd_mem_share err\n");
	printf("WD dma map VA=IOVA=%p successfully!\n", a);

	size = pkt_len;
	src = a;
	msg = malloc(sizeof(*msg) * loops);
	SYS_ERR_COND(!msg, "malloc msg fail!\n");
	for (i = 0; i < loops; i++) {
		src = a + i * (size * 2 + 32);
		memcpy(src, aucDataIn_aes_cbc_128,
			sizeof(aucDataIn_aes_cbc_128));
		dst = src + size;
		memset(dst, 0, size);
		key = dst + size;
		memcpy(key, aucKey_aes_cbc_128, sizeof(aucKey_aes_cbc_128));
		iv = key + sizeof(aucKey_aes_cbc_128);
		memcpy(iv, aucIvIn_aes_cbc_128, sizeof(aucIvIn_aes_cbc_128));

		init_msg(&msg[i], &capa);
		msg[i].iv = (__u64)iv;
		msg[i].src = (__u64)src;
		msg[i].dst = (__u64)dst;
		msg[i].key = (__u64)key;
	}

	for (i = 0; i < loops; i++) {
		ret = wd_send(&q, (void *)&msg[i]);
		SYS_ERR_COND(ret,
			"send fail(release queue should be done auto)\n");
		ret = wd_recv_sync(&q, (void **)&resp, 0);
		SYS_ERR_COND(ret != 1,
			"recv fail(release queue should be done auto)\n");
		ret = check_result((void *)resp->dst);
		if (ret) {
			printf("dst=0x%llx:0x%llx\n",resp->dst, *(__u64 *)resp->dst);
			printf("%dth time cipher fail!\n", i);
			break;
		}
	}
	free(msg);
	wd_mem_unshare(&q, a, ASIZE);
	munmap(a, ASIZE);
	wd_release_queue(&q);

	return EXIT_SUCCESS;
}


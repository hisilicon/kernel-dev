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
#include "../wd.h"
#include "../wd_comp.h"

#define ASIZE (16*4096)

#define SYS_ERR_COND(cond, msg)		\
do {					\
	if (cond) {			\
		perror(msg);		\
		exit(EXIT_FAILURE);	\
	}				\
} while (0)

char zlib_test[1024] = {
0x20, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72, 0x61, 0x6d,
0x20, 0x69, 0x73, 0x20, 0x66, 0x72, 0x65, 0x65, 0x20, 0x73, 0x6f, 0x66, 0x74,
0x77, 0x61, 0x72, 0x65, 0x3b, 0x20, 0x79, 0x6f, 0x75, 0x20, 0x63, 0x61, 0x6e,
0x20, 0x72, 0x65, 0x64, 0x69, 0x73, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65,
0x20, 0x69, 0x74, 0x20, 0x61, 0x6e, 0x64, 0x2f, 0x6f, 0x72, 0x20, 0x6d, 0x6f,
0x64, 0x69, 0x66, 0x79, 0x0a, 0x20, 0x69, 0x74, 0x20, 0x75, 0x6e, 0x64, 0x65,
0x72, 0x20, 0x74, 0x68, 0x65, 0x20, 0x74, 0x65, 0x72, 0x6d, 0x73, 0x20, 0x6f,
0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x47, 0x4e, 0x55, 0x20, 0x47, 0x65, 0x6e,
0x65, 0x72, 0x61, 0x6c, 0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20, 0x4c,
0x69, 0x63, 0x65, 0x6e, 0x73, 0x65, 0x20, 0x61, 0x73, 0x20, 0x70, 0x75, 0x62,
0x6c, 0x69, 0x73, 0x68, 0x65, 0x64, 0x20, 0x62, 0x79, 0x0a, 0x20, 0x74, 0x68,
0x65, 0x20, 0x46, 0x72, 0x65, 0x65, 0x20, 0x53, 0x6f, 0x66, 0x74, 0x77, 0x61,
0x72, 0x65, 0x20, 0x46, 0x6f, 0x75, 0x6e, 0x64, 0x61, 0x74, 0x69, 0x6f, 0x6e,
0x3b, 0x20, 0x65, 0x69, 0x74, 0x68, 0x65, 0x72, 0x20, 0x76, 0x65, 0x72, 0x73,
0x69, 0x6f, 0x6e, 0x20, 0x32, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20,
0x4c, 0x69, 0x63, 0x65, 0x6e, 0x73, 0x65, 0x2c, 0x20, 0x6f, 0x72, 0x0a, 0x20,
0x28, 0x61, 0x74, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x6f, 0x70, 0x74, 0x69,
0x6f, 0x6e, 0x29, 0x20, 0x61, 0x6e, 0x79, 0x20, 0x6c, 0x61, 0x74, 0x65, 0x72,
0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x2e, 0x0a};

#define COMP_FILE	"/root/compress_data"
#define OP_NUMBER	1024000

int main(int argc, char *argv[])
{
	struct wd_capa capa;
	struct wd_queue q;
	struct wd_comp_msg *msg, *recv_msg;
	void *a, *src, *dst;
	int ret, i;
	int output_num;
	FILE *fp;
	int proc_tag;
	char file[64];
	struct timeval start_tval, end_tval;
	float time, speed;
	int data_size = 244;
	int mode;

	if (argv[1])
		proc_tag = strtoul(argv[1], NULL, 10);
	else
		proc_tag = 0;
	if (proc_tag)
		sprintf(file, COMP_FILE"%d", proc_tag);
	else
		sprintf(file, COMP_FILE);
	if (argv[2])
		mode = strtoul(argv[2], NULL, 10);
	else
		mode = 0;
	memset(&q, 0, sizeof(q));
	memset(&capa, 0, sizeof(capa));
	capa.alg = zlib;
	capa.throughput = 10;
	capa.latency = 10;

	ret = wd_request_queue(&q, &capa);
	SYS_ERR_COND(ret, "wd_request_queue");
	printf("\npasid=%d, dma_flag=%d", q.pasid, q.dma_flag);

	/* Allocate some space and setup a DMA mapping */
	a = mmap((void *)0xffffa9001000, ASIZE, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 0, 0);
	if (a != (void *)0xffffa9001000) {
		printf("\nmmap fail!");
		goto release_q;
	}
	memset(a, 0, ASIZE);
	sleep(5);
	ret = wd_mem_share(&q, a, ASIZE, 0);
	SYS_ERR_COND(ret, "wd_mem_share err\n");
	printf("WD dma map VA=IOVA=%p successfully!\n", a);

	src = a;
	dst = (char *)a + (ASIZE/2);

	for (i = 0; i < 128; i++)
		memcpy(src + i * 256, zlib_test, data_size);

	msg = malloc(sizeof(*msg));
	if (!msg) {
		printf("\nalloc msg fail!");
		goto alloc_msg_fail;
	}
	memset((void *)msg, 0, sizeof(*msg));
	gettimeofday(&start_tval, NULL);
	for (i = 0; i < OP_NUMBER; i++) {
		msg->alg = zlib;
		msg->src = (__u64)src + (i % 128) * 256;
		msg->dst = (__u64)dst + (i % 128) * 256;
		msg->in_bytes = data_size;

		/* now we only support pbuffer */
		msg->aflags |= _WD_AATTR_IOVA;
		ret = wd_send(&q, msg);
		SYS_ERR_COND(ret, "send fail!\n");
recv_again:
		ret = wd_recv(&q, (void **)&recv_msg);
		if (ret < 0) {
			printf("\n wd_recv fail!");
			goto alloc_msg_fail;
		/* synchronous mode, if get none, then get again */
		} else if (ret == 0 && mode)
			goto recv_again;
		/* asynchronous mode, if get one then get again */
		else if (ret == 1 && !mode)
			goto recv_again;

		output_num = recv_msg->out_bytes;
		/* printf("output_num: %d\n", output_num); */
	}
	gettimeofday(&end_tval, NULL);
	time = (float)((end_tval.tv_sec-start_tval.tv_sec) * 1000000 +
		end_tval.tv_usec - start_tval.tv_usec);
	speed = 1 / (time / OP_NUMBER);
	printf("\r\n%s compressing time %0.0f us, pkt len = %d bytes, %0.3f Mpps",
	       "zlib", time, data_size, speed);

	/* add zlib compress head and write head + compressed date to a file */
	char zip_head[2] = {0x78, 0x9c};

	fp = fopen(file, "wb");

	fwrite(zip_head, 1, 2, fp);
	fwrite((char *)msg->dst, 1, output_num, fp);

	fclose(fp);

	free(msg);
alloc_msg_fail:
	wd_mem_unshare(&q, a, ASIZE);
	munmap(a, ASIZE);
release_q:
	wd_release_queue(&q);

	return EXIT_SUCCESS;
}

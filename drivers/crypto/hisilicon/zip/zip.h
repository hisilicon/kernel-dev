/*
 * Copyright (c) 2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef HISI_ZIP_H
#define HISI_ZIP_H

#define HZIP_SQE_SIZE			128

enum hisi_zip_alg_type {
	HZIP_ZLIB = 0,
	/* add more algorithm type here */
};

enum hisi_zip_queue_status {
	/* queue does not exit */
	HZIP_QUEUE_NONE = 0,
	/* create queue pair: allocate sq/cq, pasid not set */
	HZIP_QUEUE_INITED,
	/*
	 * now queue can be used at any time, just need a sq doorbell.
	 * After Using status, queue also can enter idel status.
	 */
	HZIP_QUEUE_IDEL,
	/* now queue is using */
	HZIP_QUEUE_USING,
};

struct hisi_acc_qm_db {
	__u16 tag;
	__u8 cmd;
	__u16 index;
	__u8 priority;
};

struct hisi_acc_qm_sqc {
	__u16 sq_head_index;
	__u16 sq_tail_index;

	__u16 sqn;

	/* now we don't export other info in sqc */
};

struct hisi_zip_sqe {
        __u32 consumed;
        __u32 produced;
        __u32 comp_date_length;
        __u32 dw3;
        __u32 input_date_length;
        __u32 lba_l;
        __u32 lba_h;
        __u32 dw7; /* ... */
        __u32 dw8; /* ... */
        __u32 dw9; /* ... */
        __u32 dw10; /* ... */
        __u32 priv_info;
        __u32 dw12; /* ... */
        __u32 tag;
        __u32 dest_avail_out;
        __u32 rsvd0;
        __u32 comp_head_addr_l;
        __u32 comp_head_addr_h;
        __u32 source_addr_l;
        __u32 source_addr_h;
        __u32 dest_addr_l;
        __u32 dest_addr_h;
        __u32 stream_ctx_addr_l;
        __u32 stream_ctx_addr_h;
        __u32 cipher_key1_addr_l;
        __u32 cipher_key1_addr_h;
        __u32 cipher_key2_addr_l;
        __u32 cipher_key2_addr_h;
        __u32 rsvd1[4];
};

#define HACC_QM_DB_SQ		_IOW('d', 0, unsigned long)
#define HACC_QM_MB_SQC		_IOR('d', 1, struct hisi_acc_qm_sqc *)
#define HACC_QM_SET_PASID	_IOW('d', 2, unsigned long)
#define HACC_QM_DB_CQ		_IOW('d', 3, unsigned long)

#endif

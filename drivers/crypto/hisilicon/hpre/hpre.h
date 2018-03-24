/*
 * Copyright (c) 2018 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __HPRE_H
#define __HPRE_H

#include <linux/iopoll.h>
#include "../zip/qm.h"

#define HPRE_SQE_SIZE			64
#define HPRE_SQ_SIZE			(HPRE_SQE_SIZE * QM_Q_DEPTH)
#define QM_CQ_SIZE			(QM_CQE_SIZE * QM_Q_DEPTH)

struct hpre {
	struct pci_dev *pdev;
	resource_size_t phys_base;
	resource_size_t size;
	void __iomem *io_base;
	struct qm_info *qm_info;
	struct wd_dev *wdev;
};

enum hpre_alg_type {
	HPRE_INVLD, 
	HPRE_RSA,
	HPRE_DH,
	/* add more algorithm type here */
};

struct acc_qm_sqc {
	__u16 sqn;

	/* now we don't export other info in sqc */
};

struct hpre_sqe {
	__u32 alg 	: 5;

	/* error type */
	__u32 etype 	:11;
	__u32 resv0 	: 14;
	__u32 done	: 2;
	__u32 task_len1	: 8;
	__u32 task_len2	: 8;
	__u32 mrttest_num : 8;
	__u32 resv1 	: 8;
	__u32 low_key;
	__u32 hi_key;
	__u32 low_in;
	__u32 hi_in;
	__u32 low_out;
	__u32 hi_out;
	__u32 tag	:16;
	__u32 resv2	:16;
	__u32 rsvd1[7];
};

#define HACC_QM_DB_SQ		_IOW('d', 0, unsigned long)
#define HACC_QM_MB_SQC		_IOR('d', 1, struct hisi_acc_qm_sqc *)
#define HACC_QM_SET_PASID	_IOW('d', 2, unsigned long)
#define HACC_QM_DB_CQ		_IOW('d', 3, unsigned long)

#endif

/*
 * Copyright (c) 2016-2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _HISI_SEC_MAIN_H
#define _HISI_SEC_MAIN_H

#include <linux/interrupt.h>
#include <linux/list.h>


#include "../zip/qm.h"


#define SEC_DEBUG
/*#define CONFIG_WD*/


#define HSEC_VF_NUM			63
#define HSEC_QUEUE_NUM			1024
#define HSEC_FUN_QUEUE_NUM  (HSEC_QUEUE_NUM / (HSEC_VF_NUM + 1))

#define HISI_SEC_SQE_SIZE                128

#define SEC_PF_DEF_Q_BASE		 0
#define SEC_PF_DEF_Q_NUM		 64

#define HISI_SEC_CONTROL		0x301200
#define HISI_SEC_SAA_CLK_EN		0x301260
#define HISI_SEC_SAA_EN		0x301270
#define HISI_SEC_NAME_SIZE		64
#define HISI_SEC_DRV_VERSION		"sec-v2.0"

struct hisi_sec {
	struct pci_dev *pdev;
	void __iomem *io_base;

	int sec_id;
	char name[HISI_SEC_NAME_SIZE];
	struct qm_info *qm_info;

#ifdef CONFIG_WD
	struct wd_dev *wdev;
#endif
	struct hisi_acc_qp qp[HSEC_FUN_QUEUE_NUM];
	spinlock_t qp_lock;
};

#define HISI_SEC_Q_DEV(Q) ((Q)->p_dev)
#define HISI_SEC_DEV(SEC)	((SEC)->dev)


extern irqreturn_t hacc_irq_thread(int irq, void *data);
extern int hisi_acc_init_qm_mem(struct qm_info *qm);
extern void hisi_acc_set_user_domain(struct qm_info *qm);
extern void hisi_acc_set_cache(struct qm_info *qm);
extern int hisi_acc_qm_info_create(struct device *dev,
	void __iomem *base, u32 number,
	enum hw_version hw_v, struct qm_info **res);
extern int hisi_acc_qm_info_add_queue(struct qm_info *qm, u32 base, u32 number);
extern void hisi_acc_qm_info_release(struct qm_info *qm);
extern int hisi_acc_create_qp(struct qm_info *qm, struct hisi_acc_qp **res,
	u32 sqe_size, u8 alg_type);
extern int hisi_acc_release_qp(struct hisi_acc_qp *qp);
extern int hisi_acc_set_pasid(struct hisi_acc_qp *qp, u16 pasid);
extern int hisi_acc_unset_pasid(struct hisi_acc_qp *qp);
extern u16 hisi_acc_get_sq_tail(struct hisi_acc_qp *qp);
extern int hisi_acc_send(struct hisi_acc_qp *qp, u16 sq_tail, void *priv);
extern int hisi_acc_receive(struct hisi_acc_qp *qp, void *priv);
extern int hisi_acc_unset_pasid(struct hisi_acc_qp *qp);
extern u16 hisi_acc_get_sq_tail(struct hisi_acc_qp *qp);
extern int hisi_acc_send(struct hisi_acc_qp *qp, u16 sq_tail, void *priv);
extern int hisi_acc_receive(struct hisi_acc_qp *qp, void *priv);

struct hisi_sec  *sec_device_get(int cpu);
struct hisi_acc_qp *sec_crypto_queue_alloc(struct hisi_sec *sec_dev);
int sec_queue_send(void *queue, void *msg);
int sec_crypto_queue_release(struct hisi_acc_qp *sec_queue);

#endif /* _HISI_SEC_MAIN_H */

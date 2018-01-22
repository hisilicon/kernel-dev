/*
 * Copyright 2017 (c) HiSilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include "./qm.h"
#include "../wd/wd.h"
#include "./zip.h"

#define HZIP_VF_NUM			63
#define HZIP_QUEUE_NUM			4096
#define HZIP_FUN_QUEUE_NUM  (HZIP_QUEUE_NUM / (HZIP_VF_NUM + 1))

#define HZIP_FSM_MAX_CNT		0x301008
#define HZIP_CONTROL			0x30100c /* not use */
#define SGE_OFFSET_REG_VAL 0x0
#define ZIP_SGL_CONTROL 0x0
#define PRP_PAGE_SIZE 0x0
#define ZIP_PAGE_CONTROL 0x0
#define T10_DIF_CRC_INITIAL 0x0
#define ZIP_DIF_CRC_INIT 0x0
#define STORE_COMP_HEAD_LEN 0x0
#define ZIP_COM_HEAD_LENGTH 0x0

#define HZIP_PORT_ARCA_CHE_0		0x301040
#define HZIP_PORT_ARCA_CHE_1		0x301044
#define HZIP_PORT_AWCA_CHE_0		0x301060
#define HZIP_PORT_AWCA_CHE_1		0x301064

#define HZIP_BD_RUSER_32_63		0x301110
#define HZIP_SGL_RUSER_32_63		0x30111c
#define HZIP_DATA_RUSER_32_63		0x301128
#define HZIP_DATA_WUSER_32_63		0x301134
#define HZIP_BD_WUSER_32_63		0x301140

#define HZIP_SQE_STATUS(sqe)		((*((u32 *)(sqe) + 3)) & 0xff)
#define HZIP_SQC_PASID(sqc)		((*((u32 *)(sqc) + 5)) & 0xffff)

struct hzip_qp {
	u32 queue_id;
	u32 alg_type;
	/* 128B x 1024 = 32 4k pages, will map this sq to user space */
	char *sq;
	char *sqc;
	/* all through cq is same for all QM based acc, here we still copy a
	 * point of cq to indicate the cq in one zip function queue pair
	 */
	char *cq;

	enum hisi_zip_queue_status status;

	/* use q status */
	struct wd_queue *wdq;
	struct hisi_zip *hzip;
};

struct hisi_zip {
	struct pci_dev *pdev;
	void __iomem *io_base;

	struct qm_info *qm_info;
	struct wd_dev *wdev;
	struct hzip_qp qp[HZIP_FUN_QUEUE_NUM];
	spinlock_t qp_lock;
};

static inline void hisi_zip_write(struct hisi_zip *hzip, u32 val, u32 offset)
{
	writel(val, hzip->io_base + offset);
}

static inline u32 hisi_zip_read(struct hisi_zip *hzip, u32 offset)
{
	return readl(hzip->io_base + offset);
}

/* check if bit in regs is 1 */
static inline void hisi_zip_check(struct hisi_zip *hzip, u32 offset, u32 bit)
{
	int val;

	do {
		val = hisi_zip_read(hzip, offset);
	} while ((BIT(bit) & val) == 0);
}

char hisi_zip_name[] = "hisi_zip";

#define to_hisi_zip(p_wdev) container_of(p_wdev, struct hisi_zip, wdev)
#define to_hisi_zip_qp(p_wdq) container_of(p_wdq, struct hzip_qp, wdq)

static struct pci_device_id zip_dev_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_HUAWEI, 0xa250) },
	/* we plan to reuse zip.c for vf */
	{ PCI_DEVICE(PCI_VENDOR_ID_HUAWEI, 0xa251) },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, zip_dev_ids);

static irqreturn_t hisi_zip_irq(int irq, void *data)
{
//	pr_err("in %s\n", __FUNCTION__);
	struct qm_info *qm_info = (struct qm_info *)data;
	struct hisi_zip *hzip = qm_info->priv;
	u32 int_source;

	/* There is an interrupt or not */
	int_source = hisi_zip_read(hzip, QM_VF_EQ_INT_SOURCE);	

	if (int_source) {
		pr_err("in %s a\n", __FUNCTION__);
		return IRQ_WAKE_THREAD;
	}
	else {
		pr_err("in %s a\n", __FUNCTION__);
		return IRQ_HANDLED;
	}
}

static int hisi_zip_sqe_handler(struct qm_info *qm_info, char *cqe)
{
	struct hisi_zip *hzip = qm_info->priv;
	struct hzip_qp *qp = hzip->qp;
	char *sq = qp[CQE_SQ_NUM(cqe)].sq;
	char *sqe_h = sq + CQE_SQ_HEAD_INDEX(cqe) * HZIP_SQE_SIZE;
	int status = HZIP_SQE_STATUS(sqe_h);

	if (!status) {
		/* set flag */
		qp->status = HZIP_QUEUE_IDEL;
		return IRQ_HANDLED;
	} else {
		/* to handle err */
		return -1;
	}
	
	return 0;
}

static int hisi_zip_init_qm(struct hisi_zip *hisi_zip)
{
	u64 tmp = 0;
	u64 i;
	u32 val;

	/* fix: init qm user domain and cache */
	/* user domain */
	hisi_zip_write(hisi_zip, 0x40000070, QM_ARUSER_M_CFG_1);
	hisi_zip_write(hisi_zip, 0xfffffffe, QM_ARUSER_M_CFG_ENABLE); //
	hisi_zip_write(hisi_zip, 0x40000070, QM_AWUSER_M_CFG_1);
	hisi_zip_write(hisi_zip, 0xfffffffe, QM_AWUSER_M_CFG_ENABLE); //
	hisi_zip_write(hisi_zip, 0xffffffff, QM_WUSER_M_CFG_ENABLE);
	/* cache */
	hisi_zip_write(hisi_zip, 0xffff,     QM_AXI_M_CFG);
	hisi_zip_write(hisi_zip, 0xffffffff, QM_AXI_M_CFG_ENABLE);
	hisi_zip_write(hisi_zip, 0xffffffff, QM_PEH_AXUSER_CFG_ENABLE);

	/* add to debug */
	hisi_zip_write(hisi_zip, 0x4893, 0x100050);
	val = hisi_zip_read(hisi_zip, 0x1000cc);
	val |= (1 << 11);
	hisi_zip_write(hisi_zip, val, 0x1000cc);

	/* fix: init qm memory */
	hisi_zip_write(hisi_zip, 0x1, QM_MEM_START_INIT);
	hisi_zip_check(hisi_zip, QM_MEM_INIT_DONE, 0);

	/* fix: init sq number for each pf and vf */
	hisi_zip_check(hisi_zip, QM_VFT_CFG_RDY, 0);
	for (i = 0; i <= 0; i++) {	
		hisi_zip_write(hisi_zip, 0x0, QM_VFT_CFG_OP_WR);
		hisi_zip_write(hisi_zip, QM_SQC_VFT, QM_VFT_CFG_TYPE);
		hisi_zip_write(hisi_zip, i, QM_VFT_CFG_ADDRESS);
		
		/* 64(queus) x 32B(sqc size) = 2048B */
		tmp = QM_SQC_VFT_BUF_SIZE		|   //
		      QM_SQC_VFT_SQC_SIZE		|
		      QM_SQC_VFT_INDEX_NUMBER		|
		      i << QM_SQC_VFT_BT_INDEX_SHIFT	|
		      QM_SQC_VFT_VALID			|
		      i * HZIP_FUN_QUEUE_NUM << QM_SQC_VFT_START_SQN_SHIFT;


		hisi_zip_write(hisi_zip, tmp & 0xffffffff, QM_VFT_CFG_DATA_L);
		hisi_zip_write(hisi_zip, tmp >> 32, QM_VFT_CFG_DATA_H);

		hisi_zip_write(hisi_zip, 0x0, QM_VFT_CFG_RDY);
		hisi_zip_write(hisi_zip, 0x1, QM_VFT_CFG_OP_ENABLE);
		hisi_zip_check(hisi_zip, QM_VFT_CFG_RDY, 0);
	}

	/* dump sqc vft */
	tmp = 0;
	hisi_zip_check(hisi_zip, QM_VFT_CFG_RDY, 0);
	for (i = 0; i <= 0; i++) {	
		hisi_zip_write(hisi_zip, 0x1, QM_VFT_CFG_OP_WR);
		hisi_zip_write(hisi_zip, QM_SQC_VFT, QM_VFT_CFG_TYPE);
		hisi_zip_write(hisi_zip, i, QM_VFT_CFG_ADDRESS);
		
		tmp = hisi_zip_read(hisi_zip, QM_VFT_CFG_DATA_L);
		tmp = hisi_zip_read(hisi_zip, QM_VFT_CFG_DATA_H);

		hisi_zip_write(hisi_zip, 0x0, QM_VFT_CFG_RDY);
		hisi_zip_write(hisi_zip, 0x1, QM_VFT_CFG_OP_ENABLE);
		hisi_zip_check(hisi_zip, QM_VFT_CFG_RDY, 0);
	}

	tmp = 0;

	/* fix: init cq number for each pf and vf */
	for (i = 0; i <= 0; i++) {	
		hisi_zip_write(hisi_zip, 0x0, QM_VFT_CFG_OP_WR);
		hisi_zip_write(hisi_zip, QM_CQC_VFT, QM_VFT_CFG_TYPE);
		hisi_zip_write(hisi_zip, i, QM_VFT_CFG_ADDRESS);

		tmp = QM_CQC_VFT_BUF_SIZE		|
		      QM_CQC_VFT_SQC_SIZE		|
		      QM_CQC_VFT_INDEX_NUMBER		|
		      i << QM_CQC_VFT_BT_INDEX_SHIFT	|
		      QM_CQC_VFT_VALID;


		hisi_zip_write(hisi_zip, tmp & 0xffffffff, QM_VFT_CFG_DATA_L);
		hisi_zip_write(hisi_zip, tmp >> 32, QM_VFT_CFG_DATA_H);

		hisi_zip_write(hisi_zip, 0x0, QM_VFT_CFG_RDY);
		hisi_zip_write(hisi_zip, 0x1, QM_VFT_CFG_OP_ENABLE);
		hisi_zip_check(hisi_zip, QM_VFT_CFG_RDY, 0);
	}

	return 0;
}

static int hisi_zip_init_zip(struct hisi_zip *hisi_zip)
{
	/* to do: init zip user domain and cache */
	/* cache */
	hisi_zip_write(hisi_zip, 0xffffffff, HZIP_PORT_ARCA_CHE_0);
	hisi_zip_write(hisi_zip, 0xffffffff, HZIP_PORT_ARCA_CHE_1);
	hisi_zip_write(hisi_zip, 0xffffffff, HZIP_PORT_AWCA_CHE_0);
	hisi_zip_write(hisi_zip, 0xffffffff, HZIP_PORT_AWCA_CHE_1);
	/* user domain configurations */
	hisi_zip_write(hisi_zip, 0x40000070, HZIP_BD_RUSER_32_63);
	hisi_zip_write(hisi_zip, 0x40001070, HZIP_SGL_RUSER_32_63);
	hisi_zip_write(hisi_zip, 0x40001071, HZIP_DATA_RUSER_32_63);
	hisi_zip_write(hisi_zip, 0x40001071, HZIP_DATA_WUSER_32_63);
	hisi_zip_write(hisi_zip, 0x40000070, HZIP_BD_WUSER_32_63);

	/* fsm count */	
	hisi_zip_write(hisi_zip, 0xfffffff, HZIP_FSM_MAX_CNT); 

	/* to do: big/little endian configure: default: 32bit little */

	/* to do: SGL offset, later to do */
	/* hisi_zip_write(hisi_zip, SGE_OFFSET_REG_VAL, ZIP_SGL_CONTROL); */

	/* to do: PRP page size */
	/* hisi_zip_write(hisi_zip, PRP_PAGE_SIZE, ZIP_PAGE_CONTROL); */

	/* CRC initial*/
	/* hisi_zip_write(hisi_zip, T10_DIF_CRC_INITIAL, ZIP_DIF_CRC_INIT); */

	/* Compress head length */
	/* hisi_zip_write(hisi_zip, STORE_COMP_HEAD_LEN, ZIP_COM_HEAD_LENGTH); */

	/* to check: clock gating, core, decompress verify enable */
	hisi_zip_write(hisi_zip, 0x10005, 0x301004);

	/* to check: enable counters */

	/* to check: configure mastooo dfx & configure larger packet. */
		
	return 0;
}

static int hisi_zip_init_queue(struct hisi_zip *hisi_zip)
{
	struct device *dev = &hisi_zip->pdev->dev;
	struct qm_info *qm;
	void *vadd;

	qm = devm_kzalloc(dev, sizeof(struct qm_info), GFP_KERNEL);
	if (!qm)
		return -ENOMEM;

	hisi_zip->qm_info = qm;
	qm->fun_base = hisi_zip->io_base;
	qm->sqn_base = 0;
	qm->sqn_num = 64;
	qm->cqn_base = 0;
	qm->cqn_num = 64;
	qm->sqe_handler = hisi_zip_sqe_handler;
	qm->priv = hisi_zip;
        spin_lock_init(&qm->mailbox_lock);

	/* Init sqc_bt */
	qm->sqc_cache = kzalloc(QM_SQC_SIZE * HZIP_FUN_QUEUE_NUM, GFP_KERNEL);
	if (!qm->sqc_cache)
		return -ENOMEM;
	/* here we configure sqc_bt for PF, so queue = 0 */
	hacc_mb(qm, MAILBOX_CMD_SQC_BT, virt_to_phys(qm->sqc_cache),
		0, 0, 0);

	unsigned long sqc_bt;
	hacc_mb(qm, MAILBOX_CMD_SQC_BT, virt_to_phys(&sqc_bt),
		0, 1, 0);

	/* Init cqc_bt */
	qm->cqc_cache = kzalloc(QM_CQC_SIZE * HZIP_FUN_QUEUE_NUM, GFP_KERNEL);
	if (!qm->cqc_cache)
		return -ENOMEM;
	/* here we configure cqc_bt for PF, so queue = 0 */
	hacc_mb(qm, MAILBOX_CMD_CQC_BT, virt_to_phys(qm->cqc_cache),
		0, 0, 0);

	/* to do: queue status? */

	/* Init eqc */
	qm->eqc_cache = kzalloc(QM_EQC_SIZE, GFP_KERNEL);
	if (!qm->eqc_cache)
		return -ENOMEM;

	vadd = kzalloc(QM_EQE_SIZE * QM_EQ_DEPTH, GFP_KERNEL);
	if (!vadd)
		return -ENOMEM;
	qm->eq = vadd;

	((u32 *)qm->eqc_cache)[1] =  lower_32_bits(virt_to_phys(vadd));
	((u32 *)qm->eqc_cache)[2] =  upper_32_bits(virt_to_phys(vadd));
	((u32 *)qm->eqc_cache)[3] =  2 << MB_EQC_EQE_SHIFT;
	((u32 *)qm->eqc_cache)[6] =  (QM_EQ_DEPTH - 1) |
				     (1 << MB_EQC_PHASE_SHIFT);
	hacc_mb(qm, MAILBOX_CMD_EQC, virt_to_phys(qm->eqc_cache), QM_EQ_DEPTH,
		0, 0);
#if 0   /* ES version does not support aeqc */
	/* Init aeqc */
	qm->aeqc_cache  = devm_kzalloc(dev, QM_AEQC_SIZE, GFP_KERNEL);
	if (!qm->aeqc_cache)
		return -ENOMEM;

	vadd = devm_kzalloc(dev, QM_AEQE_SIZE * QM_EQ_DEPTH, GFP_KERNEL);
	if (!vadd)
		return -ENOMEM;
	qm->aeq = vadd;

	((u32 *)qm->aeqc_cache)[1] =  lower_32_bits(virt_to_phys(vadd));
	((u32 *)qm->aeqc_cache)[2] =  upper_32_bits(virt_to_phys(vadd));
	((u32 *)qm->aeqc_cache)[3] =  2 << MB_AEQC_AEQE_SHIFT;
	((u32 *)qm->aeqc_cache)[6] =  (QM_EQ_DEPTH - 1) |
				      (1 << MB_AEQC_PHASE_SHIFT);
	hacc_mb(qm, MAILBOX_CMD_AEQC, virt_to_phys(qm->aeqc_cache), QM_EQ_DEPTH,
		0, 0);
#endif
	/* for what? */
	hisi_zip_write(hisi_zip, 0x0, QM_VF_EQ_INT_MASK);

	return 0;
}

static ssize_t
pasid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct wd_queue *q = wd_queue(dev);
	struct hzip_qp *qp = (struct hzip_qp *)q->priv;
	char *sqc = qp->sqc;

	return sprintf(buf, "%d\n", HZIP_SQC_PASID(sqc));
}

static ssize_t
pasid_store(struct device *dev, struct device_attribute *attr, const char *buf,
	    size_t len)
{
	struct wd_queue *q = wd_queue(dev);
	struct hzip_qp *qp = (struct hzip_qp *)q->priv;
	struct wd_dev *wdev = q->wdev;
	struct hisi_zip *hzip = (struct hisi_zip *)wdev->priv;
	char *sqc = qp->sqc;
	long value;
	int ret;

	ret = kstrtol(buf, 10, &value);
	if (ret)
		return -EINVAL;

	*((u32 *)sqc + 5) &= 0xffff0000;
	*((u32 *)sqc + 5) |= value & 0xffff;

	hacc_mb(hzip->qm_info, MAILBOX_CMD_SQC, virt_to_phys(sqc),
		qp->queue_id, 0, 0);

	qp->status = HZIP_QUEUE_IDEL;

	return len;
}
DEVICE_ATTR_RW(pasid);

static ssize_t
qid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct wd_queue *q = wd_queue(dev);
	struct hzip_qp *qp = (struct hzip_qp *)q->priv;

	return sprintf(buf, "%d\n", qp->queue_id);
}
static DEVICE_ATTR_RO(qid);

static struct attribute *mdev_dev_attrs[] = {
	WD_DEFAULT_MDEV_DEV_ATTRS
	&dev_attr_pasid.attr,
	&dev_attr_qid.attr,
	NULL,
};

static const struct attribute_group mdev_dev_group = {
	.name  = WD_QUEUE_PARAM_GRP_NAME,
	.attrs = mdev_dev_attrs,
};

/* this will be showed under virtual device's directory */
static const  struct attribute_group *mdev_dev_groups[] = {
	&mdev_dev_group,
	NULL,
};

static ssize_t name_show(struct kobject *kobj, struct device *dev, char *buf)
{
	return 0;
}
MDEV_TYPE_ATTR_RO(name);

static ssize_t
device_api_show(struct kobject *kobj, struct device *dev, char *buf)
{
	return 0;
}
MDEV_TYPE_ATTR_RO(device_api);

static ssize_t
available_instances_show(struct kobject *kobj, struct device *dev, char *buf)
{
	/* how many free queues now one function can support */
	return 0;
}
MDEV_TYPE_ATTR_RO(available_instances);

/* fix me: add zlib here. and why we add name, device_api, available_instances */
static struct attribute *hzip_zlib_type_attrs[] = {
	WD_DEFAULT_MDEV_TYPE_ATTRS
	&mdev_type_attr_name.attr,
	&mdev_type_attr_device_api.attr,
	&mdev_type_attr_available_instances.attr,
	NULL,
};

static struct attribute_group hzip_zlib_type_group = {
	.name  = "zlib",
	.attrs = hzip_zlib_type_attrs,
};

/* this will be showed under physical device's supported_type_groups directory */
static struct attribute_group *mdev_type_groups[] = {
	&hzip_zlib_type_group,
	NULL,
};

static int hisi_zip_create_qp(struct hisi_zip *hisi_zip,
			      enum hisi_zip_alg_type alg_type, int q_num)
{
	char *sqc = hisi_zip->qm_info->sqc_cache + QM_SQC_SIZE * q_num;
	char *cqc = hisi_zip->qm_info->cqc_cache + QM_CQC_SIZE * q_num;
	void *vadd;

	hisi_zip->qp[q_num].queue_id = q_num;
	hisi_zip->qp[q_num].sqc = sqc;

	/* create wd_queue */
	hisi_zip->qp[q_num].wdq = kzalloc(sizeof(struct wd_queue), GFP_KERNEL);
	if (!hisi_zip->qp[q_num].wdq)
		return -ENOMEM;
	hisi_zip->qp[q_num].wdq->priv = hisi_zip->qp + q_num;
	hisi_zip->qp[q_num].wdq->wdev = hisi_zip->wdev;

	/* will map to user space */
	vadd = __get_free_pages(GFP_KERNEL, get_order(HZIP_SQE_SIZE * QM_EQ_DEPTH));
	if (!vadd)
		return -ENOMEM;
	memset(vadd, 0, PAGE_SIZE << get_order(HZIP_SQE_SIZE * QM_EQ_DEPTH));
	hisi_zip->qp[q_num].sq = vadd;

	pr_err("in %s: sq base: %p\n", __FUNCTION__, vadd);

	/* to fill sqc mb format, how to add lock? */
	((u32 *)sqc)[0] = 0 << SQ_HEAD_SHIFT | 0 << SQ_TAIL_SHIFI;
	((u32 *)sqc)[1] = lower_32_bits(virt_to_phys(vadd));
	((u32 *)sqc)[2] = upper_32_bits(virt_to_phys(vadd));
	((u32 *)sqc)[3] = (0 << SQ_HOP_NUM_SHIFT)	|
		          (0 << SQ_PAGE_SIZE_SHIFT)	|
		          (0 << SQ_BUF_SIZE_SHIFT)	|
		          (7 << SQ_SQE_SIZE_SHIFT);
	((u32 *)sqc)[4] = SQ_DEPTH - 1;
	((u32 *)sqc)[5] = 0; //SQ_PASID, need add interface ?
	((u32 *)sqc)[6] = q_num				|
		          0 << SQ_PRIORITY_SHIFT	|
		          1 << SQ_ORDERS_SHIFT		|
		          alg_type << SQ_TYPE_SHIFT;
	((u32 *)sqc)[7] = 0;

	hacc_mb(hisi_zip->qm_info, MAILBOX_CMD_SQC, virt_to_phys(sqc),
		q_num, 0, 0);

//	struct sqc sqc_dump __attribute__ ((aligned(32)));
	void *sqc_dump = kzalloc(32, GFP_KERNEL);
//	pr_err("in %s: sqc base: %p\n", __FUNCTION__, &sqc_dump);
//	pr_err("in %s: sqc basep: %p\n", __FUNCTION__, virt_to_phys(&sqc_dump));
	pr_err("in %s: sqc base: %p\n", __FUNCTION__, sqc_dump);
	pr_err("in %s: sqc basep: %p\n", __FUNCTION__, virt_to_phys(sqc_dump));
	hacc_mb(hisi_zip->qm_info, MAILBOX_CMD_SQC, virt_to_phys(sqc_dump),
		q_num, 1, 0);
	pr_err("in %s: dump sqc: 0: %lx\n", __FUNCTION__, ((u32 *)sqc_dump)[0]);
	pr_err("in %s: dump sqc: 1: %lx\n", __FUNCTION__, ((u32 *)sqc_dump)[1]);
	pr_err("in %s: dump sqc: 2: %lx\n", __FUNCTION__, ((u32 *)sqc_dump)[2]);
	pr_err("in %s: dump sqc: 3: %lx\n", __FUNCTION__, ((u32 *)sqc_dump)[3]);
	pr_err("in %s: dump sqc: 4: %lx\n", __FUNCTION__, ((u32 *)sqc_dump)[4]);
	pr_err("in %s: dump sqc: 5: %lx\n", __FUNCTION__, ((u32 *)sqc_dump)[5]);
	pr_err("in %s: dump sqc: 6: %lx\n", __FUNCTION__, ((u32 *)sqc_dump)[6]);
	pr_err("in %s: dump sqc: 7: %lx\n", __FUNCTION__, ((u32 *)sqc_dump)[7]);

	/* better to page align? */
	vadd = kzalloc(QM_CQE_SIZE * QM_EQ_DEPTH, GFP_KERNEL);
	if (!vadd)
		return -ENOMEM;
	hisi_zip->qp[q_num].cq = vadd;

	/* to fill cqc mb format */
	((u32 *)cqc)[0] = 0 << CQ_HEAD_SHIFT | 0 << CQ_TAIL_SHIFI;
	((u32 *)cqc)[1] = lower_32_bits(virt_to_phys(vadd));
	((u32 *)cqc)[2] = upper_32_bits(virt_to_phys(vadd));
	((u32 *)cqc)[3] = (0 << CQ_HOP_NUM_SHIFT)	|
		          (0 << CQ_PAGE_SIZE_SHIFT)	|
		          (0 << CQ_BUF_SIZE_SHIFT)	|
		          (4 << CQ_SQE_SIZE_SHIFT);
	((u32 *)cqc)[4] = CQ_DEPTH - 1;
	((u32 *)cqc)[5] = 0; //CQ_PASID, need add interface ?
	((u32 *)cqc)[6] = 1 << CQ_PHASE_SHIFT | 1 << CQ_FLAG_SHIFT;
	((u32 *)cqc)[7] =  0;

	hacc_mb(hisi_zip->qm_info, MAILBOX_CMD_CQC, virt_to_phys(cqc),
		q_num, 0, 0);

	hisi_zip->qp[q_num].status = HZIP_QUEUE_INITED;

	return 0;
}

static int hzip_get_queue(struct wd_dev *wdev, const char *devalgo_name,
			  struct wd_queue **q)
{
	/* the meaning of this inferface is to build a new virtual device, which
	 * will be add to mdev_bus_type later. so we can use sqc/cqc mailbox to
	 * create sq/cq
	 */
	/* when we are getting a queue, we can get a unused sqc and need to
	 * create a queue; or there already has a queue used, but its status is
	 * idle.
	 *
	 * seems in same process, we can use the idle queue, as we need not to
	 * allocate a queue. but for different processes, we'd better get a new
	 * queue with no sqe before we can see, otherwise, there is a safe risk.
	 *
	 */
	/* get the alg type from devalo_name */
	/* how about adding a para to pass pasid to hardware */

	struct hisi_zip *hzip = (struct hisi_zip *)wdev->priv;
	unsigned long flags;
	int i;

	// 1. parse qp to find a NULL wd_queue
	// 2. allocate memory and mb to create sq and cq
	// 3. create wd_queue struct

//	spin_lock_irqsave(&hzip->qp_lock, flags);
	for (i = 0; i < HZIP_FUN_QUEUE_NUM; i++) {
		if (hzip->qp[i].status != HZIP_QUEUE_NONE) {
			continue;
		} else {
			hzip->qp[i].status = HZIP_QUEUE_INITED;
			break;
		}
	}
//	spin_unlock_irqrestore(&hzip->qp_lock, flags);

	if (i < HZIP_FUN_QUEUE_NUM && hzip->qp[i].status == HZIP_QUEUE_INITED) {
		if (!hisi_zip_create_qp(hzip, 0, i)) {
			*q = hzip->qp[i].wdq;
			hzip->qp[i].hzip = hzip;
			pr_err("in %s\n", __FUNCTION__);
			return 0;
		} else {
			dev_err(wdev->dev, "Can't create zip queue pair!\n");
			hzip->qp[i].status = HZIP_QUEUE_NONE;
			return -ENOMEM;
		}
	} else {
		dev_err(wdev->dev, "all queue are using!\n");
		return -ENODEV;
	}
}

/* mdev_fops->remove */
static int hzip_put_queue(struct wd_queue *q)
{
	/* the meaning of this inferface is to remove a new virtual device, so
	 * free sq/cq related memory, update sqc and cq related info
	 */
	/* to do: sqc mailbox to release one queue? */

	struct hzip_qp *qp = (struct hzip_qp *)q->priv;

	// 1. stop queue first or return fail, hardware can not be stopped in ES :(
	// 2. free sq and cq
	free_pages((unsigned long)qp->sq, get_order(HZIP_SQE_SIZE * QM_EQ_DEPTH));
	kfree(qp->cq);
	kfree(qp->wdq);

	qp->status = HZIP_QUEUE_NONE;

	return 0;
}

/* mdev_fops->open */
static int hzip_open_queue(struct wd_queue *q)
{
	/* to do: to start this queue, after this, at any time, you can send a
	 * sqc doorbeel to put sqe to sq of zip
	 */
	/* after sending a sqc/cqc mb, hardware queue can work */
	return 0;
}

/* mdev_fops->release */
static int hzip_close_queue(struct wd_queue *q)
{
	/* to do: to stop operating the work of zip core? and clear sq/cq memory?
	 * set sqc/cqc head
	 */
	/* hardware can not be closed :( */

	/* to do: set qp status here */

	return 0;
}

static int hzip_is_q_updated(struct wd_queue *q)
{
	struct hzip_qp *qp = (struct hzip_qp *)q->priv;
	/* to do: one q is updated, update related flag, here we check if
	 * this flag is updated.
	 *
	 * user process will sleep to wait queue to updated.
	 */

	return (qp->status == HZIP_QUEUE_IDEL) ? 1 : 0;
}

/* map sq to user space. doorbell register also to user space? */
static int hzip_mmap(struct wd_queue *q, struct vm_area_struct *vma)
{
	struct hzip_qp *qp = (struct hzip_qp *)q->priv;
	unsigned long sq_size = HZIP_SQE_SIZE * SQ_DEPTH;
	unsigned long cq_size = QM_CQE_SIZE * QM_EQ_DEPTH;
	char *sq = qp->sq;
	char *cq = qp->cq;
        int ret;

	vma->vm_flags |= (VM_IO | VM_LOCKED | VM_DONTEXPAND | VM_DONTDUMP);

	/* fix me */
	if (vma->vm_pgoff != 0)
		return -EINVAL;

        /* map sq */
        ret = remap_pfn_range(vma, vma->vm_start, __pa(sq) >> PAGE_SHIFT,
			      sq_size, PAGE_SHARED);
        if (ret < 0)
                return ret;

        /* map cq */
        return remap_pfn_range(vma, vma->vm_start + sq_size,
                               __pa(cq) >> PAGE_SHIFT,
			       cq_size, PAGE_SHARED);

}

static void dump_sqe(void *sqe)
{
	pr_err("in %s\n", __FUNCTION__);
	pr_err("         sqe base: %p\n", sqe);
	pr_err("         sqe  [4]: %lx\n", *((__u32 *)sqe + 4));
	pr_err("         sqe  [9]: %lx\n", *((__u32 *)sqe + 9));
	pr_err("         sqe [18]: %lx\n", *((__u32 *)sqe + 18));
	pr_err("         sqe [19]: %lx\n", *((__u32 *)sqe + 19));
	pr_err("         sqe [20]: %lx\n", *((__u32 *)sqe + 20));
	pr_err("         sqe [21]: %lx\n", *((__u32 *)sqe + 21));

}

static long hzip_ioctl(struct wd_queue *q, unsigned int cmd, unsigned long arg)
{
	struct hzip_qp *qp = (struct hzip_qp *)q->priv;
	struct qm_info *qm = qp->hzip->qm_info;
	struct hisi_acc_qm_sqc qm_sqc;
	struct hacc_qm_db qm_db;
	char *sqc = qp->sqc;

	switch (cmd) {
	/* we can offer sq doorbell */
	case HACC_QM_DB_SQ:
		if (copy_from_user(&qm_db, (void __user *)arg,
				   sizeof(struct hacc_qm_db)))
			return -EFAULT;

		//dump_sqe(qp->sq + (qm_db.index - 1) * HZIP_SQE_SIZE);

		hacc_db(qm, qm_db.tag, qm_db.cmd, qm_db.index, qm_db.priority);
		qp->status = HZIP_QUEUE_USING;
		break;
	/* user to read the data in SQC cache */
	case HACC_QM_MB_SQC:
		qm_sqc.sq_head_index = 0xffff & *((u32 *)qp->sqc);
		qm_sqc.sq_tail_index = 0xffff & (*((u32 *)qp->sqc) >> 16);
		qm_sqc.sqn = qp->queue_id;
                if (copy_to_user((struct hisi_acc_qm_sqc *)arg, &qm_sqc,
				 sizeof(struct hisi_acc_qm_sqc)))
                        return -EFAULT;
		break;
	case HACC_QM_SET_PASID:

		*((u32 *)sqc + 5) &= 0xffff0000;
		*((u32 *)sqc + 5) |= arg & 0xffff;

		hacc_mb(qm, MAILBOX_CMD_SQC, virt_to_phys(sqc),
			qp->queue_id, 0, 0);

		qp->status = HZIP_QUEUE_IDEL;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static const struct wd_dev_ops hzip_ops = {
	.get_queue = hzip_get_queue,
	.put_queue = hzip_put_queue,
	.open = hzip_open_queue,
	.close = hzip_close_queue,
	.is_q_updated = hzip_is_q_updated,
	.mmap = hzip_mmap,
	.ioctl = hzip_ioctl,
};


static int zip_probe (struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct hisi_zip *hisi_zip;
	struct wd_dev *wdev;
	resource_size_t base;
	resource_size_t size;
	int ret;
	u16 ecam_val16;

	pci_set_power_state(pdev, PCI_D0);
	ecam_val16 = (PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY);
	pci_write_config_word(pdev, PCI_COMMAND, ecam_val16);

	ret = pci_enable_device_mem(pdev);
	if(ret < 0) {
        	dev_err(&pdev->dev, "Can't enable device mem!\n");
		return ret;
	}

	ret = pci_request_mem_regions(pdev, hisi_zip_name);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't request mem regions!\n");
		goto err_pci_reg;
	}

	/* to do: zip ras */

	/* init hisi_zip */
	hisi_zip  = devm_kzalloc(&pdev->dev, sizeof(*hisi_zip), GFP_KERNEL);
	if (!hisi_zip) {
		ret = -ENOMEM;
		goto err_pci_reg;
	}

	base = pci_resource_start(pdev, 2);
	size = pci_resource_len(pdev, 2);
	hisi_zip->io_base = devm_ioremap(&pdev->dev, base, size);
	if (!hisi_zip->io_base) {
		ret = -EIO;;
		goto err_pci_reg;
	}
	hisi_zip->pdev = pdev;
        spin_lock_init(&hisi_zip->qp_lock);

	dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	pci_set_master(pdev);

	/* to do: request irq */
	ret = pci_alloc_irq_vectors(pdev, 1, 2, PCI_IRQ_MSI);
	if (ret < 2) {
		dev_err(&pdev->dev, "Enable MSI vectors fail!\n");
		if (ret > 0)
			goto err_pci_irq;
		else
			goto err_pci_reg;
	}

	/* to do: exception irq handler register */
	
	if (pdev->is_physfn) {
		/* to do: init zip's QM */
		ret = hisi_zip_init_qm(hisi_zip);
		if (ret) {
			dev_err(&pdev->dev, "Fail to init QM!\n");
			goto err_pci_irq;
		}
			
		/* to do: init zip itself */
		hisi_zip_init_zip(hisi_zip);
		if (ret) {
			dev_err(&pdev->dev, "Fail to init ZIP!\n");
			goto err_pci_irq;
		}
	}

	/* to do: init zip's queue */
	ret = hisi_zip_init_queue(hisi_zip);
	if (ret) {
		dev_err(&pdev->dev, "Fail to init queues!\n");
		goto err_pci_irq;
	}

	ret = devm_request_threaded_irq(&pdev->dev, pci_irq_vector(pdev, 0),
					hisi_zip_irq, hacc_irq_thread,
					IRQF_SHARED, hisi_zip_name, hisi_zip->qm_info);
	if (ret)
		goto err_pci_irq;

	/* to do: init wd related structure */
	wdev = devm_kzalloc(&pdev->dev, sizeof(struct wd_dev), GFP_KERNEL);
	if (!wdev) {
		ret = -ENOMEM;
		goto err_pci_irq;
	}

	hisi_zip->wdev = wdev;
	pci_set_drvdata(pdev, wdev);

	wdev->iommu_type = VFIO_TYPE1_IOMMU;
	wdev->dma_flag = WD_DMA_MULTI_PROC_MAP;
	//wdev->dma_flag = WD_DMA_SVM_NO_FAULT;
	wdev->owner = THIS_MODULE;
	wdev->name = hisi_zip_name;
	wdev->dev = &pdev->dev;
	wdev->is_vf = pdev->is_virtfn;
	wdev->priv = hisi_zip;
	wdev->node_id = pdev->dev.numa_node;
	wdev->priority = 0;
	wdev->api_ver = "wd_hzip_v1";
	wdev->throughput_level = 10;
	wdev->latency_level = 10;
	wdev->flags = 0;

	wdev->mdev_fops.mdev_attr_groups = mdev_dev_groups;
	wdev->mdev_fops.supported_type_groups = mdev_type_groups;
	wdev->ops = &hzip_ops;

	ret = wd_dev_register(wdev);
	if (ret) {
		dev_err(&pdev->dev, "Fail to register ZIP to WD system!\n");
		goto err_pci_irq;
	}

	return 0;

err_pci_irq:
	pci_free_irq_vectors(pdev);
err_pci_reg:
	pci_disable_device(pdev);
	/* release region? */
	return ret;
}

static void zip_remove(struct pci_dev *pdev)
{
	struct wd_dev *wdev = pci_get_drvdata(pdev);

	wd_dev_unregister(wdev);
}

static int zip_pci_sriov_configure(struct pci_dev *pdev, int num_vfs)
{
	return 0;
}

static struct pci_driver zip_pci_driver = {
	.name 		= "hisi_zip",
	.id_table 	= zip_dev_ids,
	.probe 		= zip_probe,
	.remove 	= zip_remove,
	.sriov_configure = zip_pci_sriov_configure
};

static int __init zip_init(void)
{
	int ret;

	ret = pci_register_driver(&zip_pci_driver);
	if (ret < 0)
		pr_err("zip: can't register hisi zip driver.\n");

	return ret;
}

static void __exit zip_exit(void)
{
	pci_unregister_driver(&zip_pci_driver);
}

module_init(zip_init);
module_exit(zip_exit);

MODULE_DESCRIPTION("Driver for HiSilicon ZIP accelerator");
MODULE_AUTHOR("Zhou Wang <wangzhou1@hisilicon.com>");
MODULE_AUTHOR("Dong Bo <dongbo4@huawei.com>");
MODULE_LICENSE("GPL");

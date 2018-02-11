/*
 * Copyright (c) 2017-2018 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/atomic.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/iommu.h>
#include <linux/mdev.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/pci.h>

#include "sec_main.h"
#include "sec_algs.h"
#include "sec_common.h"
#include "../wd/wd.h"

#define DRV_NAME "hisi_sec"

static int sec_num;

/*todo: static table should not be a good way to manage*/
static struct hisi_sec *sec_devices[HISI_MAX_SEC_DEVICES];


/*todo: hw debug interface*/
#ifdef SEC_HW_DEBUG
static int hisi_sec_qm_rd_bd_cfg(struct wd_dev *dev, int bd_index,
				 int offset, int bit_index)
{
	__u32 var = 0;

	if (bd_index > 19 || offset > 3 || bit_index > 3) {
		pr_err("bd index = %d offset = %d, bit_index=%derror\n",
		       bd_index, offset, bit_index);
		return 0;
	}
	/* Which bd among 20 bd from 0 to 19 */
	var |= (bd_index << 4);

	/* which 128bits in that bd from 0-3 */
	var |= (offset << 2);

	/* which 32bits in that 128bits from 0-3 */
	var |= bit_index;
	writel_relaxed(var,
		       DEV_IO_ADDR(dev, 1) + HISI_SEC_QM_BD_DFX_CFG_REG);

	udelay(100);

	var = readl_relaxed(DEV_IO_ADDR(dev, 1) +
			    HISI_SEC_QM_BD_DFX_RESULT_REG);

	pr_info("bd index = %d offset = %d, bit_index=%d, var=0x%x\n",
		bd_index, offset, bit_index, var);

	return 0;

}

static int hisi_sec_ooo_err_stat(struct wd_dev *dev)
{
	__u32 var1, var2;

	var1 = readl_relaxed(DEV_IO_ADDR(dev, 0) + 0x180);
	var2 = readl_relaxed(DEV_IO_ADDR(dev, 0) + 0x184);
	pr_info("warning 0x%x from read chanel 0x%x from write channel\n",
	       var1, var2);

	return 0;
}

static void hisi_sec_queue_inner_hw_buf_print(struct wd_queue *q)
{
	hisi_sec_ooo_err_stat(q->dev);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 0, 0);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 0, 1);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 0, 2);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 0, 3);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 1, 0);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 1, 1);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 1, 2);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 1, 3);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 2, 0);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 2, 1);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 2, 2);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 2, 3);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 3, 0);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 3, 1);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 3, 2);
	hisi_sec_qm_rd_bd_cfg(q->dev, 0, 3, 3);
}
#endif


struct hisi_sec *sec_device_get(int cpu)
{
	if ((cpu < sec_num) && (cpu >= 0))
		return sec_devices[cpu];
	pr_err("sec device not found for cpu %d\n", cpu);

	return NULL;
}

int hisi_sec_sqe_handler(struct hisi_acc_qp *queue, void *element)
{
	struct sec_bd_info *msg = (struct sec_bd_info *)element;
#ifdef SEC_DEBUG
	pr_info("sec msg->done=%d\n", msg->done);
#endif
	/* atomic todo...*/
	while (msg->done) {
		sec_alg_handler(msg);
		msg->done = 0;
	}

	return 0;
}

static irqreturn_t hisi_sec_irq(int irq, void *data)
{
	struct qm_info *qm = (struct qm_info *)data;
	u32 int_source;

	/* There is an interrupt or not */
	int_source = hisi_acc_get_irq_source(qm);

	if (int_source)
		return IRQ_WAKE_THREAD;
	else
		return IRQ_HANDLED;
}


int sec_queue_send(void *queue, void *msg)
{
	struct hisi_acc_qp *sec_queue = queue;
	int ret;

	/*lock need   todo...*/
	/*spin_lock_bh(&sec_queue->lock);*/
	sec_queue->sq_tail = hisi_acc_get_sq_tail(sec_queue);

	memcpy(sec_queue->sq_base + sec_queue->sq_tail * sec_queue->sqe_size,
				msg,  sec_queue->sqe_size);

	/*mb here*/
	wmb();

	/* fixme....*/
	ret = hisi_acc_send(sec_queue, sec_queue->sq_tail, NULL);
	/*spin_unlock_bh(&sec_queue->lock);*/
	return ret;

}

struct hisi_acc_qp *sec_crypto_queue_alloc(struct hisi_sec *sec_dev)
{

	struct hisi_acc_qp *sec_queue = NULL;
	struct qm_info *qm =  sec_dev->qm_info;
	struct device *dev = &sec_dev->pdev->dev;
	int ret;
	u8 task_type = 0; /* 0:normal   1: PBKDF2*/

	ret = hisi_acc_create_qp(qm, &sec_queue, HISI_SEC_SQE_SIZE, task_type);
	if (ret) {
		dev_err(dev, "alloc sec queue fai!\n");
		return NULL;
	}
	sec_queue->type = CRYPTO_QUEUE;
	sec_queue->sqe_handler = hisi_sec_sqe_handler;

	/*spin_lock_init(&sec_queue->lock); todo... */

	/* debug: dump sqc */
	hisi_acc_qm_read_sqc(sec_queue);
#if 0
	pr_err("---> in %s: sq_base_l: %x\n",
		__FUNCTION__, sec_queue->sqc->sq_base_l);
	pr_err("---> in %s: sq_base_h: %x\n",
		__FUNCTION__, sec_queue->sqc->sq_base_h);
#endif
	return sec_queue;
}

int sec_crypto_queue_release(struct hisi_acc_qp *sec_queue)
{
	int ret = 0;

	ret = hisi_acc_release_qp(sec_queue);
	if (ret) {
		dev_err(sec_queue->p_dev,  "free sec queue fail!\n");
		return ret;
	}
	return 0;
}

static inline void hisi_sec_write(struct hisi_sec *hsec, u32 val, u32 offset)
{
	writel(val, hsec->io_base + offset);
}

static inline u32 hisi_sec_read(struct hisi_sec *hsec, u32 offset)
{
	return readl(hsec->io_base + offset);
}

/* check if bit in regs is 1 */
static inline void hisi_sec_check(struct hisi_sec *hsec, u32 offset, u32 bit)
{
	int val;

	do {
		val = hisi_sec_read(hsec, offset);
	} while ((BIT(bit) & val) == 0);
}



static int hisi_sec_init_qm(struct hisi_sec *hisi_sec)
{
	struct qm_info *qm = hisi_sec->qm_info;

	/* fix: init qm user domain and cache */
	/* user domain */
	hisi_acc_set_user_domain(qm);
	/* cache */
	hisi_acc_set_cache(qm);

	/* fix: init qm memory */
	hisi_acc_init_qm_mem(qm);

	hisi_acc_qm_set_priv(qm, hisi_sec);

	return 0;
}

static int hisi_sec_reg_test(struct hisi_sec *hisi_sec)
{
	pr_info("hisi_sec_register rdtest HISI_SEC_CONTROL=%x",
			hisi_sec_read(hisi_sec, HISI_SEC_CONTROL));
	pr_info("hisi_sec_register rdtest HISI_SEC_SAA_CLK_EN=%x",
			hisi_sec_read(hisi_sec, HISI_SEC_SAA_CLK_EN));
	pr_info("hisi_sec_register rdtest HISI_SEC_SAA_EN=%x",
			hisi_sec_read(hisi_sec, HISI_SEC_SAA_EN));

	return 0;
}

static int hisi_sec_init_sec(struct hisi_sec *hisi_sec)
{

	/*todo...*/
	/*little endian*/
	hisi_sec_write(hisi_sec, 0x00000458, HISI_SEC_CONTROL);
	hisi_sec_write(hisi_sec, 0x000001FF, HISI_SEC_SAA_CLK_EN);
	hisi_sec_write(hisi_sec, 0x000001FF, HISI_SEC_SAA_EN);
	mdelay(1);

	pr_info("hisi_sec_register rdtest HISI_SEC_CONTROL=%x",
			hisi_sec_read(hisi_sec, HISI_SEC_CONTROL));
	pr_info("hisi_sec_register rdtest HISI_SEC_SAA_CLK_EN=%x",
			hisi_sec_read(hisi_sec, HISI_SEC_SAA_CLK_EN));
	pr_info("hisi_sec_register rdtest HISI_SEC_SAA_EN=%x",
			hisi_sec_read(hisi_sec, HISI_SEC_SAA_EN));

	return 0;
}



static int hisi_sec_init_queue(struct hisi_sec *hisi_sec, u32 qbase, u32 qnum)
{
	struct device *dev = &hisi_sec->pdev->dev;
	struct qm_info *qm = hisi_sec->qm_info;
	int ret;

	/* 0-64 queues for PF */
	ret = hisi_acc_qm_info_add_queue(qm, qbase, qnum);
	if (ret) {
		dev_err(dev, "Fail to add queue to QM!\n");
		return -ENODEV;
	}

	return 0;
}

static int sec_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct hisi_sec *hisi_sec;
	resource_size_t base;
	resource_size_t size;
	int ret;
	u16 ecam_val16;
	u32 sec_id;
	u32 q_base, q_num;

	/* init pci state */
	pci_set_power_state(pdev, PCI_D0);
	ecam_val16 = (PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY);
	pci_write_config_word(pdev, PCI_COMMAND, ecam_val16);

	ret = pci_enable_device_mem(pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't enable device mem!\n");
		return ret;
	}

	pci_set_master(pdev);

	ret = pci_request_mem_regions(pdev, DRV_NAME);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't request mem regions!\n");
		goto err_pci_reg;
	}

	/* to do: sec ras */

	/* Find a free id */
	for (sec_id = 0; sec_id < HISI_MAX_SEC_DEVICES; sec_id++)
		if (!sec_devices[sec_id])
			break;

	/* To ensure that the index is within the limit */
	if (sec_id == HISI_MAX_SEC_DEVICES) {
		dev_err(&pdev->dev, "sec device list full\n");
		goto err_pci_reg;
	}

	/* init hisi_sec mem,res*/
	hisi_sec  = devm_kzalloc(&pdev->dev, sizeof(*hisi_sec), GFP_KERNEL);
	if (!hisi_sec) {
		ret = -ENOMEM;
		goto err_pci_reg;
	}

	base = pci_resource_start(pdev, 2);
	size = pci_resource_len(pdev, 2);
	hisi_sec->io_base = devm_ioremap(&pdev->dev, base, size);
	if (!hisi_sec->io_base) {
		ret = -EIO;
		goto err_pci_reg;
	}
	hisi_sec->pdev = pdev;
	hisi_sec->sec_id = sec_id;
	spin_lock_init(&hisi_sec->qp_lock);


	/* set hisi_sec DMA mask */
	if (!dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)))
		dev_dbg(&pdev->dev, "set mask to 64bit\n");
	else {
		ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
		if (ret) {
			dev_err(&pdev->dev, "No usable DMA configuration, aborting\n");
			ret = -EIO;
			goto err_pci_reg;
		}
	}

	hisi_sec_reg_test(hisi_sec);

	ret = hisi_acc_qm_info_create(&pdev->dev, hisi_sec->io_base,
					pdev->devfn, CS, &hisi_sec->qm_info);
	if (ret) {
		dev_err(&pdev->dev, "Fail to create QM!\n");
		goto err_pci_irq;
	}
	q_base = SEC_PF_DEF_Q_BASE;
	q_num = SEC_PF_DEF_Q_NUM;

	dev_info(&pdev->dev, "sec probe  is_physfn=%d!", pdev->is_physfn);

	/*pdev->is_physfn = 1;*/

	/*PF hardware init*/
	if (pdev->is_physfn) {
		dev_info(&pdev->dev, "sec probe  sec is physfn\n");
		/* init sec's QM hardware , fixme */
		ret = hisi_sec_init_qm(hisi_sec);
		if (ret) {
			dev_err(&pdev->dev, "Fail to init QM!\n");
			goto err_pci_irq;
		}

		hisi_acc_qm_info_vft_config(hisi_sec->qm_info, q_base, q_num);

		/* to do: init sec hardware, fixme */
		hisi_sec_init_sec(hisi_sec);
		if (ret) {
			dev_err(&pdev->dev, "Fail to init SEC!\n");
			goto err_pci_irq;
		}
	} else if (pdev->is_virtfn) {
		dev_info(&pdev->dev, "sec probe  sec is virtfn\n");
		hisi_acc_get_vft_info(hisi_sec->qm_info, &q_base, &q_num);
	} else
		dev_err(&pdev->dev, "sec probe no pcie physfn and virtfn!\n");

#if 0
	 /* debug: see vft config */
	 u64 debug_vft;

	 debug_vft = vft_read_v1(hisi_sec->qm_info);
	 pr_err("---> in %s: vft: %llx\n", __func__, debug_vft);
#endif

	/* sec's queue init and create, fixme*/
	ret = hisi_sec_init_queue(hisi_sec, q_base, q_num);
	if (ret) {
		dev_err(&pdev->dev, "Fail to alloc queues!\n");
		goto err_pci_irq;
	}

	dev_info(&pdev->dev, "sec probe  hisi_sec_init_queue  success\n");

	/* todo max_msi: request irq */
	ret = pci_alloc_irq_vectors(pdev, 1, 2, PCI_IRQ_MSI);
	if (ret < 2) {
		dev_err(&pdev->dev, "Enable MSI vectors fail!\n");
		if (ret > 0)
			goto err_pci_irq;
		else
			goto err_pci_irq;
	}
	ret = devm_request_threaded_irq(&pdev->dev, pci_irq_vector(pdev, 0),
		hisi_sec_irq, hacc_irq_thread,
		IRQF_SHARED, DRV_NAME, hisi_sec->qm_info);
	if (ret) {
		dev_err(&pdev->dev, "request_threaded_irq fail!\n");
		goto err_pci_irq;

	}

	/* to do: exception irq handler register */

#ifdef CONFIG_WD

	ret = hisi_sec_register_to_wd(hisi_sec);
	if (ret) {
		dev_err(&pdev->dev,
			"Fail to register %s to WD system!\n", DRV_NAME);
		goto err_pci_irq;
	}
#endif

	/*sec_algs register*/
	ret = sec_algs_register();
	if (ret != 0) {
		dev_err(&pdev->dev, "sec register algorithms to crypto fail!\n");
		goto sec_algs_register_fail;
	}

	sec_devices[sec_id] = hisi_sec;
	sec_num++;

	dev_info(&pdev->dev, "sec driver probe success!\n");
	return 0;

sec_algs_register_fail:
	sec_algs_unregister();
err_pci_irq:
	pci_free_irq_vectors(pdev);
err_pci_reg:
	pci_disable_device(pdev);
	/* release region, fixme*/

	return ret;
}

static void sec_remove(struct pci_dev *pdev)
{
	struct device *dev = &pdev->dev;
#ifdef CONFIG_WD
	struct wd_dev *wdev = pci_get_drvdata(pdev);

	wd_dev_unregister(wdev);
#endif
	sec_algs_unregister();
	pci_free_irq_vectors(pdev);
	pci_disable_device(pdev);

	dev_dbg(dev, "hisilicon sec remove finishing!!!\n");
}

static int sec_pci_sriov_configure(struct pci_dev *pdev, int num_vfs)
{
	return 0;
}

static struct pci_device_id sec_dev_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_HUAWEI, 0xa255) },
	/* we plan to reuse sec_main.c for vf */
	{ PCI_DEVICE(PCI_VENDOR_ID_HUAWEI, 0xa256) },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, sec_dev_ids);

static struct pci_driver sec_pci_driver = {
	.name    = DRV_NAME,
	.id_table = sec_dev_ids,
	.probe    = sec_probe,
	.remove = sec_remove,
	.sriov_configure = sec_pci_sriov_configure
};

static int __init sec_init(void)
{
	int ret;

	ret = pci_register_driver(&sec_pci_driver);
	if (ret < 0)
		pr_err("sec: can't register hisi sec driver.\n");

	return ret;
}

static void __exit sec_exit(void)
{
	pci_unregister_driver(&sec_pci_driver);
}

module_init(sec_init);
module_exit(sec_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION(HISI_SEC_DRV_VERSION);
MODULE_DESCRIPTION("Hisilicon Security Accelerators v2 hw driver based on pci device");
MODULE_AUTHOR("Hisilicon, Inc.");
MODULE_ALIAS("platform:" DRV_NAME);

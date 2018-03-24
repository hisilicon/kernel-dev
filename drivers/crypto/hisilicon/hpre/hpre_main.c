/*
 * Copyright 2018 (c) HiSilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/io.h>
#include <linux/bitops.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include "../wd/wd.h"
#include "hpre.h"
#include "hpre_wd.h"

#define HPRE_VF_NUM			63
#define HPRE_QUEUE_NUM_V1		4096
#define HPRE_QUEUE_NUM_V2		1024

#define HPRE_COMM_CNT_CLR_CE		0x0
#define HPRE_FSM_MAX_CNT		0x301008
#define HPRE_VFG_AXQOS			0x30100c
#define HPRE_VFG_AXCACHE		0x301010
#define HPRE_RDCHN_INI_CFG		0x301014
#define HPRE_BD_ENDIAN			0x301020
#define HPRE_ECC_BYPASS			0x301024
#define HPRE_POISON_BYPASS		0x30102c
#define HPRE_ARUSR_CFG			0x301030
#define HPRE_AWUSR_CFG			0x301034
#define HPRE_INT_MASK			0x301400
#define HPRE_RAS_ECC_1BIT_TH		0x30140c

#define HPRE_TYPES_ENB			0x301038
#define HPRE_PORT_ARCA_CHE_0		0x301040
#define HPRE_PORT_ARCA_CHE_1		0x301044
#define HPRE_PORT_AWCA_CHE_0		0x301060
#define HPRE_PORT_AWCA_CHE_1		0x301064

#define HPRE_BD_RUSER_32_63		0x301110
#define HPRE_SGL_RUSER_32_63		0x30111c
#define HPRE_DATA_RUSER_32_63		0x301128
#define HPRE_DATA_WUSER_32_63		0x301134
#define HPRE_BD_WUSER_32_63		0x301140
#define HPRE_RDCHN_INI_ST		0x301a00
#define HPRE_CORE_ENB			0x302004
#define HPRE_CORE_INI_CFG		0x302020
#define HPRE_CORE_INI_STATUS		0x302080

#define HPRE_PF_DEF_Q_NUM	       	64
#define HPRE_PF_DEF_Q_BASE	      	0

char hpre_name[] = "hpre";

static const struct pci_device_id hpre_dev_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_HUAWEI, 0xa258) },

	/* HPRE VF device ID */
	{ PCI_DEVICE(PCI_VENDOR_ID_HUAWEI, 0xa259) },
	{ 0, }
};

static irqreturn_t hpre_irq(int irq, void *data)
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

static int hpre_sqe_handler(struct hisi_acc_qp *qp, void *sqe)
{

	u32 status = ((struct hpre_sqe *)sqe)->done & 0xff;

	if (!status) {
		/* fix me */
		return IRQ_HANDLED;
	}

	/* to handle err */
	return -1;
}

static int hpre_eng_init(struct hpre *hpre)
{
	int ret;
	u32 val;

	writel(0x1, hpre->io_base + HPRE_TYPES_ENB);
	writel(0x0, hpre->io_base + HPRE_VFG_AXQOS);
	writel(0xff, hpre->io_base + HPRE_VFG_AXCACHE);
	writel(0x0, hpre->io_base + HPRE_BD_ENDIAN);
	writel(0x0, hpre->io_base + HPRE_INT_MASK);
	writel(0x0, hpre->io_base + HPRE_RAS_ECC_1BIT_TH);
	writel(0x0, hpre->io_base + HPRE_POISON_BYPASS);
	writel(0x0, hpre->io_base + HPRE_COMM_CNT_CLR_CE);
	writel(0x0, hpre->io_base + HPRE_ECC_BYPASS);
	writel(0x0, hpre->io_base + HPRE_ARUSR_CFG);
	writel(0x0, hpre->io_base + HPRE_AWUSR_CFG);
	writel(0x0, hpre->io_base + HPRE_RDCHN_INI_CFG);
	ret = readl_relaxed_poll_timeout(hpre->io_base + HPRE_RDCHN_INI_ST,
				    val, val & BIT(0), 10, 1000);
	if (ret)
		return -ETIMEDOUT;

	/* First cluster initiating */
	writel(0xf, hpre->io_base + HPRE_CORE_ENB);
	writel(0x1, hpre->io_base + HPRE_CORE_INI_CFG);
	ret = readl_relaxed_poll_timeout(hpre->io_base + HPRE_CORE_INI_STATUS,
				    val, ((val & 0xf) == 0xf), 10, 1000);
	if (ret)
		return -ETIMEDOUT;

	/* Second cluster initiating, reg's address is 0x1000 more*/
	writel(0xf, hpre->io_base + 0x1000 + HPRE_CORE_ENB);
	writel(0x1, hpre->io_base + 0x1000 + HPRE_CORE_INI_CFG);
	ret = readl_relaxed_poll_timeout(hpre->io_base + 0x1000 +
				    HPRE_CORE_INI_STATUS,
				    val, ((val & 0xf) == 0xf), 10, 1000);
	if (ret)
		return -ETIMEDOUT;

	return ret;
}

static int hpre_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct hpre *hpre;
	struct qm_info *qm;
	int ret;
	u16 ecam_val16;
	u32 q_base, q_num;

	pci_set_power_state(pdev, PCI_D0);
	ecam_val16 = (PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY);
	pci_write_config_word(pdev, PCI_COMMAND, ecam_val16);

	ret = pci_enable_device_mem(pdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't enable device mem!\n");
		return ret;
	}

	ret = pci_request_mem_regions(pdev, hpre_name);
	if (ret < 0) {
		dev_err(&pdev->dev, "Can't request mem regions!\n");
		goto err_pci_reg;
	}

	/* to do: hpre ras */

	/* init hpre */
	hpre = devm_kzalloc(&pdev->dev, sizeof(*hpre), GFP_KERNEL);
	if (!hpre) {
		ret = -ENOMEM;
		goto err_hpre;
	}

	hpre->phys_base = pci_resource_start(pdev, 2);
	hpre->size = pci_resource_len(pdev, 2);
	hpre->io_base = devm_ioremap(&pdev->dev, hpre->phys_base,
					 hpre->size);
	if (!hpre->io_base) {
		ret = -EIO;
		goto err_hpre;
	}
	hpre->pdev = pdev;

	dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	pci_set_master(pdev);

	ret = pci_alloc_irq_vectors(pdev, 1, 2, PCI_IRQ_MSI);
	if (ret < 2) {
		dev_err(&pdev->dev, "Enable MSI vectors fail!\n");
		if (ret > 0)
			goto err_pci_irq;
		else
			goto err_hpre;
	}

	ret = hisi_acc_qm_info_create(&pdev->dev, hpre->io_base,
				      pdev->devfn, CS, &qm);
	if (ret) {
		dev_err(&pdev->dev, "Fail to create QM!\n");
		goto err_pci_irq;
	}

	if (pdev->is_physfn) {
		hisi_acc_set_user_domain(qm);
		hisi_acc_set_cache(qm);
		hisi_acc_init_qm_mem(qm);
		ret = hpre_eng_init(hpre);
		if (ret)
			goto err_pci_irq;

		q_base = HPRE_PF_DEF_Q_BASE;
		q_num = HPRE_PF_DEF_Q_NUM;
		hisi_acc_qm_info_vft_config(qm, q_base, q_num);
	} else if (pdev->is_virtfn) {
		/* get queue base and number, ES did not support to get this
		 * from mailbox. so fix me...
		 */
		hisi_acc_get_vft_info(qm, &q_base, &q_num);
	}

	ret = hisi_acc_qm_info_create_eq(qm);
	if (ret) {
		dev_err(&pdev->dev, "Fail to create eq!\n");
		goto err_pci_irq;
	}

	ret = hisi_acc_qm_info_add_queue(qm, q_base, q_num);
	if (ret) {
		dev_err(&pdev->dev, "Fail to add queue to QM!\n");
		goto err_pci_irq;
	}

	hpre->qm_info = qm;
	hisi_acc_qm_set_priv(qm, hpre);

	ret = devm_request_threaded_irq(&pdev->dev, pci_irq_vector(pdev, 0),
					hpre_irq, hacc_irq_thread,
					IRQF_SHARED, hpre_name,
					hpre->qm_info);
	if (ret)
		goto err_pci_irq;

	/* to do: exception irq handler register, ES did not support */

	ret = hpre_register_to_wd(hpre);
	if (ret)
		goto err_pci_irq;

	/* to do: register to crypto */

	return 0;

err_pci_irq:
	pci_free_irq_vectors(pdev);
err_hpre:
	pci_release_mem_regions(pdev);
err_pci_reg:
	pci_disable_device(pdev);

	return ret;
}

static void hpre_remove(struct pci_dev *pdev)
{
	struct wd_dev *wdev = pci_get_drvdata(pdev);

	wd_dev_unregister(wdev);
}

static int hpre_pci_sriov_configure(struct pci_dev *pdev, int num_vfs)
{
	/* to do: set queue number for VFs */

	return 0;
}

static struct pci_driver hpre_pci_driver = {
	.name		= "hisi_hpre",
	.id_table	= hpre_dev_ids,
	.probe		= hpre_probe,
	.remove		= hpre_remove,
	.sriov_configure = hpre_pci_sriov_configure
};

static int __init hpre_init(void)
{
	int ret;

	ret = pci_register_driver(&hpre_pci_driver);
	if (ret < 0)
		pr_err("hpre: can't register hisi hpre driver.\n");

	return ret;
}

static void __exit hpre_exit(void)
{
	pci_unregister_driver(&hpre_pci_driver);
}

module_init(hpre_init);
module_exit(hpre_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Zhou Wang <wangzhou1@hisilicon.com>");
MODULE_DESCRIPTION("Driver for HiSilicon ZIP accelerator");
MODULE_DEVICE_TABLE(pci, hpre_dev_ids);

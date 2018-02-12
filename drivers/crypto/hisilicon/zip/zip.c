/*
 * Copyright 2018 (c) HiSilicon Limited.
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
#define HZIP_QUEUE_NUM_V1		4096
#define HZIP_QUEUE_NUM_V2		1024

#define HZIP_FSM_MAX_CNT		0x301008

#define HZIP_PORT_ARCA_CHE_0		0x301040
#define HZIP_PORT_ARCA_CHE_1		0x301044
#define HZIP_PORT_AWCA_CHE_0		0x301060
#define HZIP_PORT_AWCA_CHE_1		0x301064

#define HZIP_BD_RUSER_32_63		0x301110
#define HZIP_SGL_RUSER_32_63		0x30111c
#define HZIP_DATA_RUSER_32_63		0x301128
#define HZIP_DATA_WUSER_32_63		0x301134
#define HZIP_BD_WUSER_32_63		0x301140

#define HZIP_PF_DEF_Q_NUM               64
#define HZIP_PF_DEF_Q_BASE              0
#define HZIP_SQE_SIZE			128

struct hisi_zip {
	struct pci_dev *pdev;

	resource_size_t phys_base;
	resource_size_t size;
	void __iomem *io_base;

	struct qm_info *qm_info;
	struct wd_dev *wdev;
};

char hisi_zip_name[] = "hisi_zip";

static const struct pci_device_id hisi_zip_dev_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_HUAWEI, 0xa250) },
	{ PCI_DEVICE(PCI_VENDOR_ID_HUAWEI, 0xa251) },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, hisi_zip_dev_ids);

static irqreturn_t hisi_zip_irq(int irq, void *data)
{
	struct qm_info *qm_info = (struct qm_info *)data;
	struct hisi_zip *hzip = qm_info->priv;
	u32 int_source;

	/* There is an interrupt or not */
	int_source = readl(hzip->io_base + QM_VF_EQ_INT_SOURCE);

	if (int_source) {
		return IRQ_WAKE_THREAD;
	}
	else {
		return IRQ_HANDLED;
	}
}

static int hisi_zip_sqe_handler(struct hisi_acc_qp *qp, struct cqe *cqe)
{
	struct hisi_zip_sqe *sqe = (struct hisi_zip_sqe *)qp->sq_base +
                                   CQE_SQ_HEAD_INDEX(cqe);
	u32 status = sqe->dw3 & 0xff;

	if (!status) {
                /* fix me */
		return IRQ_HANDLED;
	} else {
		/* to handle err */
		return -1;
	}
	
	return 0;
}

static void hisi_zip_set_user_domain_and_cache(struct hisi_zip *hisi_zip)
{
	/* to do: init zip user domain and cache */
	/* cache */
	writel(0xffffffff, hisi_zip->io_base + HZIP_PORT_ARCA_CHE_0);
	writel(0xffffffff, hisi_zip->io_base + HZIP_PORT_ARCA_CHE_1);
	writel(0xffffffff, hisi_zip->io_base + HZIP_PORT_AWCA_CHE_0);
	writel(0xffffffff, hisi_zip->io_base + HZIP_PORT_AWCA_CHE_1);
	/* user domain configurations */
	writel(0x40001070, hisi_zip->io_base + HZIP_BD_RUSER_32_63);
	writel(0x40001070, hisi_zip->io_base + HZIP_SGL_RUSER_32_63);
	writel(0x40001071, hisi_zip->io_base + HZIP_DATA_RUSER_32_63);
	writel(0x40001071, hisi_zip->io_base + HZIP_DATA_WUSER_32_63);
	writel(0x40001070, hisi_zip->io_base + HZIP_BD_WUSER_32_63);

	/* fsm count */	
	writel(0xfffffff, hisi_zip->io_base + HZIP_FSM_MAX_CNT); 

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
	writel(0x10005, hisi_zip->io_base + 0x301004);

	/* to check: enable counters */

	/* to check: configure mastooo dfx & configure larger packet. */
}

static ssize_t
pasid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct wd_queue *q = wd_queue(dev);
        struct hisi_acc_qp *qp = (struct hisi_acc_qp *)q->priv;
	struct sqc *sqc = qp->sqc;

	return sprintf(buf, "%d\n", sqc->pasid);
}

static ssize_t
pasid_store(struct device *dev, struct device_attribute *attr, const char *buf,
	    size_t len)
{
	struct wd_queue *q = wd_queue(dev);
        struct hisi_acc_qp *qp = (struct hisi_acc_qp *)q->priv;
	long value;
	int ret;

	ret = kstrtol(buf, 10, &value);
	if (ret)
		return -EINVAL;

        hisi_acc_set_pasid(qp, cpu_to_le16((u16)value));

	return len;
}
DEVICE_ATTR_RW(pasid);

static ssize_t
qid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct wd_queue *q = wd_queue(dev);
        struct hisi_acc_qp *qp = (struct hisi_acc_qp *)q->priv;

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

static int hzip_get_queue(struct wd_dev *wdev, const char *devalgo_name,
			  struct wd_queue **q)
{
	struct hisi_zip *hzip = (struct hisi_zip *)wdev->priv;
        struct qm_info *qm = hzip->qm_info;
        struct hisi_acc_qp *qp;
        struct wd_queue *wd_q;
        int ret;

        u8 alg_type = 0; /* fix me here */

        ret = hisi_acc_create_qp(qm, &qp, HZIP_SQE_SIZE, alg_type);
        if (ret) {
		dev_err(wdev->dev, "Can't create zip queue pair!\n");
                goto err_create_qp;
        }
        qp->type = WD_QUEUE;

        /* debug: dump sqc */
        hisi_acc_qm_read_sqc(qp);
        pr_err("---> in %s: sq_base_l: %x\n", __FUNCTION__, qp->sqc->sq_base_l);
        pr_err("---> in %s: sq_base_h: %x\n", __FUNCTION__, qp->sqc->sq_base_h);

	wd_q = kzalloc(sizeof(struct wd_queue), GFP_KERNEL);
	if (!wd_q) {
                ret = -ENOMEM;
                goto err_wd_q;
        }
        wd_q->priv = qp;

        *q = wd_q;

        return 0;

err_wd_q:
        hisi_acc_release_qp(qp);
err_create_qp:
        return ret;
}

static int hzip_put_queue(struct wd_queue *q)
{
        struct hisi_acc_qp *qp = (struct hisi_acc_qp *)q->priv;

        /* need first stop harware queue(don't support in ES), then release
         * resources.
         */
        hisi_acc_release_qp(qp);

	kfree(q);

	return 0;
}

static int hzip_open_queue(struct wd_queue *q)
{
	/* after sending a sqc/cqc mb, hardware queue can work */
	return 0;
}

static int hzip_close_queue(struct wd_queue *q)
{
	/* hardware queue can not be closed in ES :( */
	return 0;
}

static int hzip_is_q_updated(struct wd_queue *q)
{
        /* to do: now we did not support wd sync interface yet */
	return 0;
}

/* map sq/cq/doorbell to user space */
static int hzip_mmap(struct wd_queue *q, struct vm_area_struct *vma)
{
        struct hisi_acc_qp *qp = (struct hisi_acc_qp *)q->priv;
	unsigned long sq_size = HZIP_SQE_SIZE * SQ_DEPTH;
	unsigned long cq_size = QM_CQE_SIZE * QM_Q_DEPTH;
	void *sq = qp->sq_base;
	void *cq = qp->cq_base;
        struct hisi_zip *hzip = (struct hisi_zip *)qp->parent->priv;
        int ret;

	vma->vm_flags |= (VM_IO | VM_LOCKED | VM_DONTEXPAND | VM_DONTDUMP);

	/* fix me */
	if (vma->vm_pgoff != 0)
		return -EINVAL;

        ret = remap_pfn_range(vma, vma->vm_start, __pa(sq) >> PAGE_SHIFT,
			      sq_size, vma->vm_page_prot);
        if (ret < 0)
                return ret;

        ret = remap_pfn_range(vma, vma->vm_start + sq_size,
                              __pa(cq) >> PAGE_SHIFT,
			      cq_size, vma->vm_page_prot);
        if (ret < 0) {
                /* to do: unmap sq */
                return ret;
        }

        return remap_pfn_range(vma, vma->vm_start + sq_size + cq_size,
                               hzip->phys_base >> PAGE_SHIFT, hzip->size,
                               pgprot_noncached(vma->vm_page_prot));
        /* fix me: err handle */
}

static long hzip_ioctl(struct wd_queue *q, unsigned int cmd, unsigned long arg)
{
        struct hisi_acc_qp *qp = (struct hisi_acc_qp *)q->priv;
	struct hisi_acc_qm_sqc qm_sqc;

	switch (cmd) {
	/* user to read the data in SQC cache */
	case HACC_QM_MB_SQC:
		qm_sqc.sq_tail_index = qp->sq_tail;
		qm_sqc.sqn = qp->queue_id;
                if (copy_to_user((struct hisi_acc_qm_sqc *)arg, &qm_sqc,
				 sizeof(struct hisi_acc_qm_sqc)))
                        return -EFAULT;
		break;
	case HACC_QM_SET_PASID:
                hisi_acc_set_pasid(qp, (u16)(arg & 0xffff));
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

static int hisi_zip_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct hisi_zip *hisi_zip;
        struct qm_info *qm;
	struct wd_dev *wdev;
	int ret;
	u16 ecam_val16;
        u32 q_base, q_num;

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
	hisi_zip = devm_kzalloc(&pdev->dev, sizeof(*hisi_zip), GFP_KERNEL);
	if (!hisi_zip) {
		ret = -ENOMEM;
		goto err_hisi_zip;
	}

	hisi_zip->phys_base = pci_resource_start(pdev, 2);
	hisi_zip->size = pci_resource_len(pdev, 2);
	hisi_zip->io_base = devm_ioremap(&pdev->dev, hisi_zip->phys_base,
                                         hisi_zip->size);
	if (!hisi_zip->io_base) {
		ret = -EIO;;
		goto err_hisi_zip;
	}
	hisi_zip->pdev = pdev;

	dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	pci_set_master(pdev);

	ret = pci_alloc_irq_vectors(pdev, 1, 2, PCI_IRQ_MSI);
	if (ret < 2) {
		dev_err(&pdev->dev, "Enable MSI vectors fail!\n");
		if (ret > 0)
			goto err_pci_irq;
		else
			goto err_hisi_zip;
	}

        ret = hisi_acc_qm_info_create(&pdev->dev, hisi_zip->io_base,
                                      pdev->devfn, ES, &qm);
        if (ret) {
		dev_err(&pdev->dev, "Fail to create QM!\n");
		goto err_pci_irq;
        }

	if (pdev->is_physfn) {
                hisi_acc_set_user_domain(qm);
                hisi_acc_set_cache(qm);
                hisi_acc_init_qm_mem(qm);
                hisi_zip_set_user_domain_and_cache(hisi_zip);

                q_base = HZIP_PF_DEF_Q_BASE;
                q_num = HZIP_PF_DEF_Q_NUM;
                hisi_acc_qm_info_vft_config(qm, q_base, q_num);
	} else if (pdev->is_virtfn) {
                /* get queue base and number, ES did not support to get this
                 * from mailbox. so fix me...
                 */
                hisi_acc_get_vft_info(qm, &q_base, &q_num);
        }

        /* debug: see vft config */
        u64 debug_vft;
        debug_vft = vft_read_v1(qm);
        pr_err("---> in %s: vft: %llx\n", __FUNCTION__, debug_vft);

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

	hisi_zip->qm_info = qm;
        qm->priv = hisi_zip;

	ret = devm_request_threaded_irq(&pdev->dev, pci_irq_vector(pdev, 0),
					hisi_zip_irq, hacc_irq_thread,
					IRQF_SHARED, hisi_zip_name,
                                        hisi_zip->qm_info);
	if (ret)
		goto err_pci_irq;

	/* to do: exception irq handler register, ES did not support */

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
err_hisi_zip:
        pci_release_mem_regions(pdev);
err_pci_reg:
	pci_disable_device(pdev);

	return ret;
}

static void hisi_zip_remove(struct pci_dev *pdev)
{
	struct wd_dev *wdev = pci_get_drvdata(pdev);

	wd_dev_unregister(wdev);
}

static int hisi_zip_pci_sriov_configure(struct pci_dev *pdev, int num_vfs)
{
        /* to do: set queue number for VFs */

	return 0;
}

static struct pci_driver hisi_zip_pci_driver = {
	.name 		= "hisi_zip",
	.id_table 	= hisi_zip_dev_ids,
	.probe 		= hisi_zip_probe,
	.remove 	= hisi_zip_remove,
	.sriov_configure = hisi_zip_pci_sriov_configure
};

static int __init hisi_zip_init(void)
{
	int ret;

	ret = pci_register_driver(&hisi_zip_pci_driver);
	if (ret < 0)
		pr_err("zip: can't register hisi zip driver.\n");

	return ret;
}

static void __exit hisi_zip_exit(void)
{
	pci_unregister_driver(&hisi_zip_pci_driver);
}

module_init(hisi_zip_init);
module_exit(hisi_zip_exit);

MODULE_DESCRIPTION("Driver for HiSilicon ZIP accelerator");
MODULE_AUTHOR("Zhou Wang <wangzhou1@hisilicon.com>");
MODULE_LICENSE("GPL");

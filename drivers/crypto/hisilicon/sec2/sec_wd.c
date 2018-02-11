#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/pci.h>

#include "sec_main.h"
#include "../zip/qm.h"

#include "../wd/wd_usr_if.h"
#include "../wd/wd_cipher_usr_if.h"
#include "../wd/wd.h"

/*todo need recode */

/* Status of devices/queues under SEC drv */
enum _queue_status {
	WDQ_STATE_IDLE,
	WDQ_STATE_INITED,
	WDQ_STATE_USING,
	WDQ_STATE_SCHED,
};

/* This is inner data of SEC, please don't care */
struct _sec_wd_capa {
	__u8 ver;
	__u8 alg_type;
	wd_throughput_level_t throughput_level;
	wd_latency_level_t latency_level;
	__u32 flags;
	const char *alg_name;
	void *priv;
};

#define SEC_ALG_FLAGS	WD_CAPA_SGL

#define SEC_CALG_PARAM(key, iv, name)				\
	static struct wd_calg_param sec_##name##_param = {	\
		.key_size = key,				\
		.iv_size = iv,					\
	}

#define SEC_AALG_PARAM(key, mac, min, max, inc, name)		\
	static struct wd_aalg_param sec_##name##_param = {	\
		.key_size = key,				\
		.mac_size = mac,				\
		.aad_ssize = {					\
			.min_size = min,			\
			.max_ssize = max,			\
			.inc_ssize = inc,			\
		},						\
	}

#define SEC_CAPA(atype, thrghpt, latency, name)			\
	static const struct _sec_wd_capa sec_##name##_capa = {	\
		.ver = WD_VER,					\
		.alg_type = atype,				\
		.throughput_level = thrghpt,			\
		.latency_level = latency,			\
		.flags = SEC_ALG_FLAGS,				\
		.alg_name = name,				\
		.priv = &sec_##name##_param,			\
	}

SEC_CALG_PARAM(16, 16, cbc_aes_128);
SEC_CAPA(WD_AT_CY_SYM,  10, 10, cbc_aes_128);
SEC_CALG_PARAM(24, 16, cbc_aes_192);
SEC_CAPA(WD_AT_CY_SYM,  10, 10, cbc_aes_192);
SEC_CALG_PARAM(32, 16, cbc_aes_256);
SEC_CAPA(WD_AT_CY_SYM,  10, 10, cbc_aes_256);

SEC_CALG_PARAM(16, 16, ctr_aes_128);
SEC_CAPA(WD_AT_CY_SYM, 10, 10, ctr_aes_128);
SEC_CALG_PARAM(24, 16, ctr_aes_192);
SEC_CAPA(WD_AT_CY_SYM, 10, 10, ctr_aes_192);
SEC_CALG_PARAM(32, 16, ctr_aes_256);
SEC_CAPA(WD_AT_CY_SYM, 10, 10, ctr_aes_256);

SEC_CALG_PARAM(16, 16, ecb_aes_128);
SEC_CAPA(WD_AT_CY_SYM,  10, 10, ecb_aes_128);
SEC_CALG_PARAM(24, 16, ecb_aes_192);
SEC_CAPA(WD_AT_CY_SYM,  10, 10, ecb_aes_192);
SEC_CALG_PARAM(32, 16, ecb_aes_256);
SEC_CAPA(WD_AT_CY_SYM,  10, 10, ecb_aes_256);

static const struct _sec_wd_capa  *sec_sym_capa_tbl[] = {
	&sec_cbc_aes_128_capa,
	&sec_cbc_aes_192_capa,
	&sec_cbc_aes_256_capa,
	&sec_ctr_aes_128_capa,
	&sec_ctr_aes_192_capa,
	&sec_ctr_aes_256_capa,
	&sec_ecb_aes_128_capa,
	&sec_ecb_aes_192_capa,
	&sec_ecb_aes_256_capa,
	/* To be extended */
};

SEC_AALG_PARAM(-1, 16, -1, -1, -1, md5);
SEC_CAPA(WD_AT_CY_AUTH,  10, 10, md5);
SEC_AALG_PARAM(-1, 20, -1, -1, -1, sha160);
SEC_CAPA(WD_AT_CY_AUTH,  10, 10, sha160);
SEC_AALG_PARAM(-1, 28, -1, -1, -1, sha224);
SEC_CAPA(WD_AT_CY_AUTH,  10, 10, sha224);
SEC_AALG_PARAM(-1, 32, -1, -1, -1, sha256);
SEC_CAPA(WD_AT_CY_AUTH,  10, 10, sha256);

static const struct _sec_wd_capa *sec_dgst_capa_tbl[] = {
	&sec_md5_capa,
	&sec_sha160_capa,
	&sec_sha224_capa,
	&sec_sha256_capa,
	/* To be extended */
};

#define SEC_CAPA_NUM (ARRAY_SIZE(sec_dgst_capa_tbl) +	\
			ARRAY_SIZE(sec_sym_capa_tbl))


#ifdef CONFIG_WD

static int _alloc_queue(struct wd_dev *sec_dev,
			const char *alg, struct wd_queue **q)
{
	struct sec_dev_info *dev_info = sec_dev->priv;
	int i;

	/* For SEC, all the queue can supply the algorithms it supports */

	spin_lock(&dev_info->dev_lock);

	/* Get the first idle queue in SEC device */
	for (i = 0; i < HISI_SEC_V1_Q_NUM; i++) {
		if (dev_info->queue[i]->status == WDQ_STATE_USING)
			continue;
		if (dev_info->queue[i]->status == WDQ_STATE_IDLE) {
			dev_info->queue[i]->status = WDQ_STATE_USING;
			spin_unlock(&dev_info->dev_lock);

			*q = dev_info->queue[i];

			return 0;
		}
	}
	spin_unlock(&dev_info->dev_lock);

	return -ENODEV;
}

static int _free_queue(struct wd_queue *q)
{
	struct wd_dev *dev = q->wdev;
	struct sec_queue_info *info = q->priv;
	struct sec_dev_info *dev_info = dev->priv;
	spinlock_t *lock = &dev_info->dev_lock;
	int i;

	if (info->queue_id >= HISI_SEC_V1_Q_NUM ||
		!dev_info->queue[info->queue_id]) {
		dev_err(dev->dev, "no queue %d in %s\n",
			info->queue_id, dev_info->name);
		return -ENODEV;
	}

	if (dev_info->queue[info->queue_id]->status == WDQ_STATE_IDLE) {
		dev_err(dev->dev, "queue %d in %s is idle\n",
			info->queue_id, dev_info->name);
		return -ENODEV;
	}

	spin_lock(lock);
	for (i = 0; i < HISI_SEC_V1_Q_NUM; i++) {
		if (q == dev_info->queue[info->queue_id]) {
			q->status = WDQ_STATE_IDLE;
			spin_unlock(lock);
			return 0;
		}

	}
	spin_unlock(lock);

	return -ENODEV;
}

static irqreturn_t sec_irq_handler(int irq, void *dev_id)
{
	struct wd_queue *q = dev_id;

	hisi_sec_queue_irq_disable(q);
	wd_wake_up(q);
	return IRQ_HANDLED;
}

static int _open_queue(struct wd_queue *q)
{
	struct wd_dev *sec = q->wdev;
	struct sec_queue_info *qinfo = q->priv;
	int ret;

	/* This is used for checking whether task is finished */
	qinfo->ring[SEC_OUTORDER_RING].write = 0;
	hisi_sec_queue_irq_disable(q);
	hisi_sec_queue_init_set(q, 0x3);
	hisi_sec_queue_enable(q, 0x1);
	ret = request_irq(qinfo->task_irq, sec_irq_handler, 0,
			  qinfo->name, q);
	if (ret) {
		dev_err(sec->dev, "request irq(%d) fail\n",
			qinfo->task_irq);
		return ret;
	}

	return ret;
}

static int _close_queue(struct wd_queue *q)
{
	struct sec_queue_info *qinfo = q->priv;

	hisi_sec_queue_irq_disable(q);
	hisi_sec_queue_enable(q, 0x0);
	irq_clear_status_flags(qinfo->task_irq, IRQ_TYPE_EDGE_RISING);
	free_irq(qinfo->task_irq, q);

	return 0;
}

/* Based on DRV, which fixs the algorithm type from algorithm name */
static int hisi_sec_get_alg_type(const char *alg_name)
{
	/* fix me */

	return WD_AT_CY_SYM;
}

static int _get_alg_info(struct wd_dev *wdev, const char *alg_name,
			 struct _sec_wd_capa ***info)
{
	struct sec_dev_info *dinfo;
	int alg_type, i;
	const char *aname;

	if (!wdev || !info)
		return -EINVAL;

	dinfo = wdev->priv;
	if (!alg_name) {
		info[0] = dinfo->capa[WD_AT_CY_SYM];
		info[1] = dinfo->capa[WD_AT_CY_AUTH];

		/* 2 kinds of algorithms are supported now */
		return HISI_SEC_ALG_TYPES - 2;
	}

	/* This alg_name has alg_type at the beginning */
	alg_type = hisi_sec_get_alg_type(alg_name);
	if (alg_type < 0)
		return alg_type;

	for (i = 0; i < HISI_SEC_V1_MAX_CAP; i++) {
		aname = dinfo->capa[alg_type][i]->alg_name;
		if (aname[0] == '\0')
			break;
		if (!strncmp(alg_name, aname, strlen(aname))) {
			*info = &dinfo->capa[alg_type][i];
			return 0;
		}
	}

	return -ENODEV;
}

static int _get_free_q_num(struct wd_dev *dev)
{
	int i, count = 0;
	struct sec_dev_info *dev_info = dev->priv;

	spin_lock(&dev_info->dev_lock);
	for (i = 0; i < HISI_SEC_V1_Q_NUM; i++) {
		if (dev_info->queue[i]->status == WDQ_STATE_USING)
			continue;
		count++;
	}
	spin_unlock(&dev_info->dev_lock);

	return count;
}

static int _queue_mmap(struct wd_queue *q, struct vm_area_struct *vma)
{
	struct device *dev = HISI_SEC_Q_DEV(q);
	struct sec_queue_info *qinfo = q->priv;
	struct sec_azone *z = qinfo->info;
	unsigned long req_len;
	int ret, i, sz = 0;
	pgprot_t vm_page_prot = vma->vm_page_prot;

	req_len = vma->vm_end - vma->vm_start;

	/* the io space is provided as a whole, no bargain */
	if (vma->vm_end < vma->vm_start ||
	    !(vma->vm_flags & VM_SHARED) ||
	    vma->vm_start & ~PAGE_MASK ||
	    vma->vm_end & ~PAGE_MASK ||
	    vma->vm_pgoff != 0 || req_len != HISI_SEC_IOSPACE_SIZE) {
		dev_err(dev, "sec map vm error!\n");
		return -EINVAL;
	}

	vma->vm_private_data = q;

	for (i = 0; i < HISI_SEC_HW_RING_NUM; i++) {
		if (z[i].size & ~PAGE_MASK)
			z[i].size = (z[i].size & PAGE_MASK) + PAGE_SIZE;
		if (z[i].addr & ~PAGE_MASK) {
			dev_err(dev, "sec map zone param error!\n");
			return -EINVAL;
		}
		vma->vm_pgoff = (z[i].addr >> PAGE_SHIFT);
		if (i == SEC_Q_REGS)
			vma->vm_page_prot =
			pgprot_noncached(vma->vm_page_prot);
		else
			vma->vm_page_prot = vm_page_prot;
		ret = remap_pfn_range(vma, vma->vm_start + sz, vma->vm_pgoff,
				z[i].size, vma->vm_page_prot);
		if (ret) {
			/* no unmap can be done */
			dev_err(dev, "map SEC_Q_REGS fail (%d)\n", ret);
			return ret;
		}
		sz += z[i].size;
	}

	return 0;
}

static int _reset_queue(struct wd_queue *wdev)
{
	/* Fixe me */
	return 0;
}

long _queue_ioctl(struct wd_queue *q, unsigned int cmd, unsigned long arg)
{
	struct device *dev = HISI_SEC_Q_DEV(q);

	switch (cmd) {

	default:
		dev_err(dev, "SEC ioctl cmd error!cmd=0x%x\n", cmd);
		return -EINVAL;
	}
}

static int sec_q_updated(struct wd_queue *q)
{
	u32 wr;
	struct sec_queue_info *qinfo = q->priv;
	struct sec_out_bd_info *out_bd = Q_IO_ADDR(q, SEC_OUTORDER_RING);
	struct sec_bd_info *bd = Q_IO_ADDR(q, SEC_CMD_RING);

	wr = readl_relaxed(Q_IO_ADDR(q, 0) +
			   HISI_SEC_Q_OUTORDER_WR_PTR_REG);
	if (wr > 0)
		out_bd = out_bd + wr - 1;
	else
		out_bd = out_bd + HISI_SEC_QUEUE_LEN - 1;

	bd = bd + out_bd->q_id;

	if (wr != qinfo->ring[SEC_OUTORDER_RING].write && bd->done) {
		qinfo->ring[SEC_OUTORDER_RING].write = wr;
		return 1;
	}

	return 0;
}

static void sec_mask_notification(struct wd_queue *q, int mask)
{
	if (mask & _WD_EVENT_NOTIFY)
		hisi_sec_queue_irq_enable(q);
}

static const char *get_alg_from_kobj(struct device *dev, struct kobject *kobj)
{
	const char *drv_name = dev_driver_string(dev);
	int len;

	len = strlen(drv_name);

	return kobj->name + len + 1;
}

static ssize_t iv_size_show(struct kobject *kobj, struct device *dev, char *buf)
{
	struct wd_dev *wdev = dev->driver_data;
	struct wd_calg_param *param;
	const char *alg_name;
	int ret;
	struct _sec_wd_capa **capa;

	if (!wdev)
		return sprintf(buf, "no device!\n");
	alg_name = get_alg_from_kobj(dev, kobj);
	if (!alg_name)
		return sprintf(buf, "no alg!\n");

	ret = _get_alg_info(wdev, alg_name, &capa);
	if (ret)
		return sprintf(buf, "no alg info!\n");
	param = capa[0]->priv;

	return sprintf(buf, "%d\n", param->iv_size);
}
MDEV_TYPE_ATTR_RO(iv_size);

static ssize_t key_size_show(struct kobject *kobj,
			     struct device *dev, char *buf)
{
	struct wd_dev *wdev = dev->driver_data;
	struct wd_calg_param *param;
	const char *alg_name;
	struct _sec_wd_capa **capa;
	int ret;

	if (!wdev)
		return sprintf(buf, "no device!\n");

	alg_name = get_alg_from_kobj(dev, kobj);
	if (!alg_name)
		return sprintf(buf, "no alg!\n");

	ret = _get_alg_info(wdev, alg_name, &capa);
	if (ret)
		return sprintf(buf, "no alg info!\n");

	param = capa[0]->priv;

	/* We take this 'keysize' as cipher key size now */
	return sprintf(buf, "%d\n", param->key_size);
}
MDEV_TYPE_ATTR_RO(key_size);

static ssize_t
available_instances_show(struct kobject *kobj, struct device *dev, char *buf)
{
	int num = -1;
	struct wd_dev *wdev = dev->driver_data;

	if (wdev)
		num = _get_free_q_num(wdev);
	if (num >= 0)
		return sprintf(buf, "%d\n", num);
	else
		return sprintf(buf, "error!\n");
}
MDEV_TYPE_ATTR_RO(available_instances);

static ssize_t device_api_show(struct kobject *kobj, struct device *dev,
			       char *buf)
{
	struct wd_dev *wdev = dev->driver_data;

	return sprintf(buf, "%s\n", HISI_SEC_DRV_NAME);
}
MDEV_TYPE_ATTR_RO(device_api);

static ssize_t name_show(struct kobject *kobj, struct device *dev, char *buf)
{
	const char *alg_name;
	struct wd_dev *wdev = dev->driver_data;

	if (!wdev)
		return sprintf(buf, "Get name fali!\n");

	alg_name = get_alg_from_kobj(dev, kobj);
	if (!alg_name)
		return sprintf(buf, "no alg!\n");

	return sprintf(buf, "%s\n", alg_name);
}
MDEV_TYPE_ATTR_RO(name);

static ssize_t mac_size_show(struct kobject *kobj,
			     struct device *dev, char *buf)
{
	struct wd_dev *wdev = dev->driver_data;
	struct wd_aalg_param *param;
	const char *alg_name;
	struct _sec_wd_capa **capa;
	int ret;

	if (!wdev)
		return sprintf(buf, "no device!\n");

	alg_name = get_alg_from_kobj(dev, kobj);
	if (!alg_name)
		return sprintf(buf, "no alg!\n");

	ret = _get_alg_info(wdev, alg_name, &capa);
	if (ret)
		return sprintf(buf, "no alg info!\n");

	param = capa[0]->priv;

	return sprintf(buf, "%d\n", param->mac_size);
}
MDEV_TYPE_ATTR_RO(mac_size);

#endif

static struct attribute *sec_mdev_attrs[] = {
	WD_DEFAULT_MDEV_DEV_ATTRS
	NULL,
};

static const struct attribute_group sec_mdev_group = {
	.name  = WD_QUEUE_PARAM_GRP_NAME,
	.attrs = sec_mdev_attrs,
};

static const  struct attribute_group *sec_mdev_groups[] = {
	&sec_mdev_group,
	NULL,
};

static struct attribute *sec_cbc_aes_128_type_attrs[] = {
	WD_DEFAULT_MDEV_TYPE_ATTRS
	&mdev_type_attr_name.attr,
	&mdev_type_attr_device_api.attr,
	&mdev_type_attr_iv_size.attr,
	&mdev_type_attr_key_size.attr,
	NULL,
};

static struct attribute *sec_md5_type_attrs[] = {
	WD_DEFAULT_MDEV_TYPE_ATTRS
	&mdev_type_attr_name.attr,
	&mdev_type_attr_device_api.attr,
	&mdev_type_attr_mac_size.attr,
	NULL,
};

/* one cipher and one hash algorithm is suported now */
static struct attribute_group sec_cbc_aes_128_type_group = {
	.name  = cbc_aes_128,
	.attrs = sec_cbc_aes_128_type_attrs,
};

static struct attribute_group sec_md5_type_group = {
	.name  = md5,
	.attrs = sec_md5_type_attrs,
};

static struct attribute_group *sec_mdev_type_groups[] = {
	&sec_cbc_aes_128_type_group,
	&sec_md5_type_group,
	NULL,
};



static const struct wd_dev_ops hisi_wd_sec_ops = {
	.get_queue = _alloc_queue,
	.put_queue = _free_queue,
	.is_q_updated = sec_q_updated,
	.mask_notification = sec_mask_notification,   /*todo...*/
	.mmap = _queue_mmap,
	.open = _open_queue,
	.close = _close_queue,
	.reset_queue = _reset_queue,         /*todo...*/
	.ioctl = _queue_ioctl,
};


int sec_register_to_wd(struct hisi_sec *hisi_sec)
{
	struct pci_dev *pdev = hisi_sec->pdev;
	struct wd_dev *wdev;
	int  ret;

	/* init wd related structure */
	wdev = devm_kzalloc(&pdev->dev, sizeof(struct wd_dev), GFP_KERNEL);
	if (!wdev) {
		ret = -ENOMEM;
		goto err_pci_irq;
	}

#ifdef CONFIG_WD
	hisi_sec->wdev = wdev;
#endif
	pci_set_drvdata(pdev, wdev);

	wdev->iommu_type = VFIO_TYPE1_IOMMU;
	wdev->dma_flag = WD_DMA_MULTI_PROC_MAP;
	wdev->owner = THIS_MODULE;
	wdev->name = DRV_NAME;
	wdev->dev = &pdev->dev;
	wdev->is_vf = pdev->is_virtfn;
	wdev->priv = hisi_sec;
	wdev->node_id = pdev->dev.numa_node;
	wdev->priority = 0;
	wdev->api_ver = "wd_hsec_v1";
	wdev->throughput_level = 10;
	wdev->latency_level = 10;
	wdev->flags = 0;

	wdev->mdev_fops.mdev_attr_groups = sec_mdev_type_groups;
	wdev->mdev_fops.supported_type_groups = sec_mdev_groups;
	wdev->ops = &hisi_wd_sec_ops;

	ret = wd_dev_register(wdev);

	return ret;
}

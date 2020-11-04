// SPDX-License-Identifier: GPL-2.0-or-later
#include <linux/compat.h>
#include <linux/dma-mapping.h>
#include <linux/iommu.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/uacce.h>

#define UACCE_MAX_PIN_PAGE		(SZ_1G / PAGE_SIZE)

static struct class *uacce_class;
static dev_t uacce_devt;
static DEFINE_MUTEX(uacce_mutex);
static DEFINE_XARRAY_ALLOC(uacce_xa);

static bool uacce_nosva;
module_param(uacce_nosva, bool, 0444);

struct uacce_pin_container {
	struct xarray array;
};

struct pin_pages {
	unsigned long first;
	unsigned long nr_pages;
	struct page **pages;
};

static int uacce_start_queue(struct uacce_queue *q)
{
	int ret = 0;

	mutex_lock(&uacce_mutex);

	if (q->state != UACCE_Q_INIT) {
		ret = -EINVAL;
		goto out_with_lock;
	}

	if (q->uacce->ops->start_queue) {
		ret = q->uacce->ops->start_queue(q);
		if (ret < 0)
			goto out_with_lock;
	}

	q->state = UACCE_Q_STARTED;

out_with_lock:
	mutex_unlock(&uacce_mutex);

	return ret;
}

static int uacce_put_queue(struct uacce_queue *q)
{
	struct uacce_device *uacce = q->uacce;
	int i;

	mutex_lock(&uacce_mutex);

	if (q->state == UACCE_Q_ZOMBIE)
		goto out;

	if ((q->state == UACCE_Q_STARTED) && uacce->ops->stop_queue)
		uacce->ops->stop_queue(q);

	if ((q->state == UACCE_Q_INIT || q->state == UACCE_Q_STARTED) &&
	     uacce->ops->put_queue)
		uacce->ops->put_queue(q);

	for (i = 0; i < UACCE_MAX_REGION; i++) {
		struct uacce_qfile_region *qfr = q->qfrs[i];

		if (qfr && qfr->kaddr) {
			dma_free_coherent(uacce->parent,
					  qfr->nr_pages << PAGE_SHIFT,
					  qfr->kaddr, qfr->dma);
			qfr->kaddr = NULL;
		}
	}

	q->state = UACCE_Q_ZOMBIE;
out:
	mutex_unlock(&uacce_mutex);

	return 0;
}

static long uacce_get_ss_dma(struct uacce_queue *q, void __user *arg)
{
	struct uacce_device *uacce = q->uacce;
	long ret = 0;
	unsigned long dma = 0;

	if ((uacce->flags & UACCE_DEV_SVA))
		return -EINVAL;

	mutex_lock(&uacce_mutex);
	if (q->qfrs[UACCE_QFRT_SS])
		dma = q->qfrs[UACCE_QFRT_SS]->dma;
	else
		ret = -EINVAL;
	mutex_unlock(&uacce_mutex);

	if (copy_to_user(arg, &dma, sizeof(dma)))
		ret = -EFAULT;

	return ret;
}

static long uacce_fops_unl_ioctl(struct file *filep,
				 unsigned int cmd, unsigned long arg)
{
	struct uacce_queue *q = filep->private_data;
	struct uacce_device *uacce = q->uacce;

	switch (cmd) {
	case UACCE_CMD_START_Q:
		return uacce_start_queue(q);

	case UACCE_CMD_PUT_Q:
		return uacce_put_queue(q);

	case UACCE_CMD_GET_SS_DMA:
		return uacce_get_ss_dma(q, (void __user *)arg);

	default:
		if (!uacce->ops->ioctl)
			return -EINVAL;

		return uacce->ops->ioctl(q, cmd, arg);
	}
}

#ifdef CONFIG_COMPAT
static long uacce_fops_compat_ioctl(struct file *filep,
				   unsigned int cmd, unsigned long arg)
{
	arg = (unsigned long)compat_ptr(arg);

	return uacce_fops_unl_ioctl(filep, cmd, arg);
}
#endif

static int uacce_bind_queue(struct uacce_device *uacce, struct uacce_queue *q)
{
	u32 pasid;
	struct iommu_sva *handle;

	if (!(uacce->flags & UACCE_DEV_SVA))
		return 0;

	handle = iommu_sva_bind_device(uacce->parent, current->mm, NULL);
	if (IS_ERR(handle))
		return PTR_ERR(handle);

	pasid = iommu_sva_get_pasid(handle);
	if (pasid == IOMMU_PASID_INVALID) {
		iommu_sva_unbind_device(handle);
		return -ENODEV;
	}

	q->handle = handle;
	q->pasid = pasid;
	return 0;
}

static void uacce_unbind_queue(struct uacce_queue *q)
{
	if (!q->handle)
		return;
	iommu_sva_unbind_device(q->handle);
	q->handle = NULL;
}

static int uacce_fops_open(struct inode *inode, struct file *filep)
{
	struct uacce_device *uacce;
	struct uacce_queue *q;
	int ret = 0;

	uacce = xa_load(&uacce_xa, iminor(inode));
	if (!uacce)
		return -ENODEV;

	q = kzalloc(sizeof(struct uacce_queue), GFP_KERNEL);
	if (!q)
		return -ENOMEM;

	ret = uacce_bind_queue(uacce, q);
	if (ret)
		goto out_with_mem;

	q->uacce = uacce;

	if (uacce->ops->get_queue) {
		ret = uacce->ops->get_queue(uacce, q->pasid, q);
		if (ret < 0)
			goto out_with_bond;
	}

	init_waitqueue_head(&q->wait);
	filep->private_data = q;
	uacce->inode = inode;
	q->state = UACCE_Q_INIT;

	mutex_lock(&uacce->queues_lock);
	list_add(&q->list, &uacce->queues);
	mutex_unlock(&uacce->queues_lock);

	return 0;

out_with_bond:
	uacce_unbind_queue(q);
out_with_mem:
	kfree(q);
	return ret;
}

static int uacce_fops_release(struct inode *inode, struct file *filep)
{
	struct uacce_queue *q = filep->private_data;

	mutex_lock(&q->uacce->queues_lock);
	list_del(&q->list);
	mutex_unlock(&q->uacce->queues_lock);
	uacce_put_queue(q);
	uacce_unbind_queue(q);
	kfree(q);

	return 0;
}

static void uacce_vma_close(struct vm_area_struct *vma)
{
	struct uacce_queue *q = vma->vm_private_data;
	struct uacce_device *uacce = q->uacce;
	struct uacce_qfile_region *qfr = NULL;
	int nr_pages = vma_pages(vma);

	if (vma->vm_pgoff < UACCE_MAX_REGION)
		qfr = q->qfrs[vma->vm_pgoff];

	if (qfr->kaddr) {
		dma_free_coherent(uacce->parent, nr_pages << PAGE_SHIFT,
				  qfr->kaddr, qfr->dma);
		qfr->kaddr = NULL;
	}

	kfree(qfr);
	q->qfrs[vma->vm_pgoff] = NULL;
}

static const struct vm_operations_struct uacce_vm_ops = {
	.close = uacce_vma_close,
};

static int uacce_fops_mmap(struct file *filep, struct vm_area_struct *vma)
{
	struct uacce_queue *q = filep->private_data;
	struct uacce_device *uacce = q->uacce;
	struct uacce_qfile_region *qfr;
	enum uacce_qfrt type = UACCE_MAX_REGION;
	int nr_pages = vma_pages(vma);
	unsigned long vm_pgoff;
	int ret = 0;

	if (vma->vm_pgoff < UACCE_MAX_REGION)
		type = vma->vm_pgoff;
	else
		return -EINVAL;

	qfr = kzalloc(sizeof(*qfr), GFP_KERNEL);
	if (!qfr)
		return -ENOMEM;

	vma->vm_flags |= VM_DONTCOPY | VM_DONTEXPAND | VM_WIPEONFORK;
	vma->vm_ops = &uacce_vm_ops;
	vma->vm_private_data = q;
	qfr->type = type;

	mutex_lock(&uacce_mutex);

	if (q->state != UACCE_Q_INIT && q->state != UACCE_Q_STARTED) {
		ret = -EINVAL;
		goto out_with_lock;
	}

	if (q->qfrs[type]) {
		ret = -EEXIST;
		goto out_with_lock;
	}

	switch (type) {
	case UACCE_QFRT_MMIO:
		if (!uacce->ops->mmap) {
			ret = -EINVAL;
			goto out_with_lock;
		}

		ret = uacce->ops->mmap(q, vma, qfr);
		if (ret)
			goto out_with_lock;

		break;

	case UACCE_QFRT_DUS:
		if (!uacce->ops->mmap) {
			ret = -EINVAL;
			goto out_with_lock;
		}

		ret = uacce->ops->mmap(q, vma, qfr);
		if (ret)
			goto out_with_lock;
		break;

	case UACCE_QFRT_SS:
		if (q->state != UACCE_Q_STARTED) {
			ret = -EINVAL;
			goto out_with_lock;
		}

		if (uacce->flags & UACCE_DEV_SVA) {
			ret = -EINVAL;
			goto out_with_lock;
		}

		qfr->kaddr = dma_alloc_coherent(uacce->parent,
						nr_pages << PAGE_SHIFT,
						&qfr->dma, GFP_KERNEL);
		if (!qfr->kaddr) {
			ret = -ENOMEM;
			goto out_with_lock;
		}
		qfr->nr_pages = nr_pages;

		/*
		 * dma_mmap_coherent() requires vm_pgoff as 0
		 * restore vm_pfoff to initial value for mmap()
		 */
		vm_pgoff = vma->vm_pgoff;
		vma->vm_pgoff = 0;
		ret = dma_mmap_coherent(uacce->parent, vma, qfr->kaddr,
					qfr->dma,
					nr_pages << PAGE_SHIFT);
		vma->vm_pgoff = vm_pgoff;
		if (ret)
			goto err_with_pages;
		break;

	default:
		ret = -EINVAL;
		goto out_with_lock;
	}

	q->qfrs[type] = qfr;
	mutex_unlock(&uacce_mutex);

	return ret;

err_with_pages:
	if (qfr->kaddr)
		dma_free_coherent(uacce->parent, nr_pages << PAGE_SHIFT,
				  qfr->kaddr, qfr->dma);
out_with_lock:
	mutex_unlock(&uacce_mutex);
	kfree(qfr);
	return ret;
}

static __poll_t uacce_fops_poll(struct file *file, poll_table *wait)
{
	struct uacce_queue *q = file->private_data;
	struct uacce_device *uacce = q->uacce;

	poll_wait(file, &q->wait, wait);
	if (uacce->ops->is_q_updated && uacce->ops->is_q_updated(q))
		return EPOLLIN | EPOLLRDNORM;

	return 0;
}

static const struct file_operations uacce_fops = {
	.owner		= THIS_MODULE,
	.open		= uacce_fops_open,
	.release	= uacce_fops_release,
	.unlocked_ioctl	= uacce_fops_unl_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= uacce_fops_compat_ioctl,
#endif
	.mmap		= uacce_fops_mmap,
	.poll		= uacce_fops_poll,
};

#define to_uacce_device(dev) container_of(dev, struct uacce_device, dev)

static ssize_t api_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct uacce_device *uacce = to_uacce_device(dev);

	return sprintf(buf, "%s\n", uacce->api_ver);
}

static ssize_t flags_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct uacce_device *uacce = to_uacce_device(dev);

	return sprintf(buf, "%u\n", uacce->flags);
}

static ssize_t available_instances_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct uacce_device *uacce = to_uacce_device(dev);

	if (!uacce->ops->get_available_instances)
		return -ENODEV;

	return sprintf(buf, "%d\n",
		       uacce->ops->get_available_instances(uacce));
}

static ssize_t algorithms_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct uacce_device *uacce = to_uacce_device(dev);

	return sprintf(buf, "%s\n", uacce->algs);
}

static ssize_t region_mmio_size_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct uacce_device *uacce = to_uacce_device(dev);

	return sprintf(buf, "%lu\n",
		       uacce->qf_pg_num[UACCE_QFRT_MMIO] << PAGE_SHIFT);
}

static ssize_t region_dus_size_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct uacce_device *uacce = to_uacce_device(dev);

	return sprintf(buf, "%lu\n",
		       uacce->qf_pg_num[UACCE_QFRT_DUS] << PAGE_SHIFT);
}

static DEVICE_ATTR_RO(api);
static DEVICE_ATTR_RO(flags);
static DEVICE_ATTR_RO(available_instances);
static DEVICE_ATTR_RO(algorithms);
static DEVICE_ATTR_RO(region_mmio_size);
static DEVICE_ATTR_RO(region_dus_size);

static struct attribute *uacce_dev_attrs[] = {
	&dev_attr_api.attr,
	&dev_attr_flags.attr,
	&dev_attr_available_instances.attr,
	&dev_attr_algorithms.attr,
	&dev_attr_region_mmio_size.attr,
	&dev_attr_region_dus_size.attr,
	NULL,
};

static umode_t uacce_dev_is_visible(struct kobject *kobj,
				    struct attribute *attr, int n)
{
	struct device *dev = kobj_to_dev(kobj);
	struct uacce_device *uacce = to_uacce_device(dev);

	if (((attr == &dev_attr_region_mmio_size.attr) &&
	    (!uacce->qf_pg_num[UACCE_QFRT_MMIO])) ||
	    ((attr == &dev_attr_region_dus_size.attr) &&
	    (!uacce->qf_pg_num[UACCE_QFRT_DUS])))
		return 0;

	return attr->mode;
}

static struct attribute_group uacce_dev_group = {
	.is_visible	= uacce_dev_is_visible,
	.attrs		= uacce_dev_attrs,
};

__ATTRIBUTE_GROUPS(uacce_dev);

static void uacce_release(struct device *dev)
{
	struct uacce_device *uacce = to_uacce_device(dev);

	kfree(uacce);
}

/**
 * uacce_alloc() - alloc an accelerator
 * @parent: pointer of uacce parent device
 * @interface: pointer of uacce_interface for register
 *
 * Returns uacce pointer if success and ERR_PTR if not
 * Need check returned negotiated uacce->flags
 */
struct uacce_device *uacce_alloc(struct device *parent,
				 struct uacce_interface *interface)
{
	unsigned int flags = interface->flags;
	struct uacce_device *uacce;
	int ret;

	uacce = kzalloc(sizeof(struct uacce_device), GFP_KERNEL);
	if (!uacce)
		return ERR_PTR(-ENOMEM);

	if (uacce_nosva)
		flags &= ~UACCE_DEV_SVA;

	if (flags & UACCE_DEV_SVA) {
		ret = iommu_dev_enable_feature(parent, IOMMU_DEV_FEAT_SVA);
		if (ret)
			flags &= ~UACCE_DEV_SVA;
	}

	uacce->parent = parent;
	uacce->flags = flags;
	uacce->ops = interface->ops;

	ret = xa_alloc(&uacce_xa, &uacce->dev_id, uacce, xa_limit_32b,
		       GFP_KERNEL);
	if (ret < 0)
		goto err_with_uacce;

	INIT_LIST_HEAD(&uacce->queues);
	mutex_init(&uacce->queues_lock);
	device_initialize(&uacce->dev);
	uacce->dev.devt = MKDEV(MAJOR(uacce_devt), uacce->dev_id);
	uacce->dev.class = uacce_class;
	uacce->dev.groups = uacce_dev_groups;
	uacce->dev.parent = uacce->parent;
	uacce->dev.release = uacce_release;
	dev_set_name(&uacce->dev, "%s-%d", interface->name, uacce->dev_id);

	return uacce;

err_with_uacce:
	if (flags & UACCE_DEV_SVA)
		iommu_dev_disable_feature(uacce->parent, IOMMU_DEV_FEAT_SVA);
	kfree(uacce);
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(uacce_alloc);

/**
 * uacce_register() - add the accelerator to cdev and export to user space
 * @uacce: The initialized uacce device
 *
 * Return 0 if register succeeded, or an error.
 */
int uacce_register(struct uacce_device *uacce)
{
	if (!uacce)
		return -ENODEV;

	uacce->cdev = cdev_alloc();
	if (!uacce->cdev)
		return -ENOMEM;

	uacce->cdev->ops = &uacce_fops;
	uacce->cdev->owner = THIS_MODULE;

	return cdev_device_add(uacce->cdev, &uacce->dev);
}
EXPORT_SYMBOL_GPL(uacce_register);

/**
 * uacce_remove() - remove the accelerator
 * @uacce: the accelerator to remove
 */
void uacce_remove(struct uacce_device *uacce)
{
	struct uacce_queue *q, *next_q;

	if (!uacce)
		return;
	/*
	 * unmap remaining mapping from user space, preventing user still
	 * access the mmaped area while parent device is already removed
	 */
	if (uacce->inode)
		unmap_mapping_range(uacce->inode->i_mapping, 0, 0, 1);

	/* ensure no open queue remains */
	mutex_lock(&uacce->queues_lock);
	list_for_each_entry_safe(q, next_q, &uacce->queues, list) {
		uacce_put_queue(q);
		uacce_unbind_queue(q);
	}
	mutex_unlock(&uacce->queues_lock);

	/* disable sva now since no opened queues */
	if (uacce->flags & UACCE_DEV_SVA)
		iommu_dev_disable_feature(uacce->parent, IOMMU_DEV_FEAT_SVA);

	if (uacce->cdev)
		cdev_device_del(uacce->cdev, &uacce->dev);
	xa_erase(&uacce_xa, uacce->dev_id);
	put_device(&uacce->dev);
}
EXPORT_SYMBOL_GPL(uacce_remove);


int uacce_ctrl_open(struct inode *inode, struct file *file)
{
	struct uacce_pin_container *p;

	p = kzalloc(sizeof(*p), GFP_KERNEL);
	if (!p)
		return -ENOMEM;
	file->private_data = p;

	xa_init(&p->array);

	return 0;
}

int uacce_ctrl_release(struct inode *inode, struct file *file)
{
	struct uacce_pin_container *priv = file->private_data;
	struct pin_pages *p;
	unsigned long idx;

	xa_for_each(&priv->array, idx, p) {
		unpin_user_pages(p->pages, p->nr_pages);
		xa_erase(&priv->array, p->first);
		kfree(p->pages);
		kfree(p);
	}

	xa_destroy(&priv->array);
	kfree(priv);

	return 0;
}

static int uacce_pin_page(struct uacce_pin_container *priv,
			  struct uacce_pin_address *addr)
{
	unsigned int flags = FOLL_FORCE | FOLL_WRITE;
	unsigned long first, last, nr_pages;
	struct page **pages;
	struct pin_pages *p;
	int ret;

	first = (addr->addr & PAGE_MASK) >> PAGE_SHIFT;
	last = ((addr->addr + addr->size - 1) & PAGE_MASK) >> PAGE_SHIFT;
	nr_pages = last - first + 1;

	pages = kmalloc_array(nr_pages, sizeof(struct page *), GFP_KERNEL);
	if (!pages)
		return -ENOMEM;

	p = kzalloc(sizeof(struct pin_pages), GFP_KERNEL);
	if (!p) {
		ret = -ENOMEM;
		goto free;
	}

	ret = pin_user_pages(addr->addr & PAGE_MASK, nr_pages,
			     flags | FOLL_LONGTERM, pages, NULL);
	if (ret != nr_pages) {
		pr_err("uacce: Failed to pin page\n");
		goto free_p;
	}
	p->first = first;
	p->nr_pages = nr_pages;
	p->pages = pages;

	ret = xa_err(xa_store(&priv->array, p->first, p, GFP_KERNEL));
	if (ret)
		goto unpin_pages;

	return 0;

unpin_pages:
	unpin_user_pages(pages, nr_pages);
free_p:
	kfree(p);
free:
	kfree(pages);
	return ret;
}

static int uacce_unpin_page(struct uacce_pin_container *priv,
			    struct uacce_pin_address *addr)
{
	unsigned long first, last, nr_pages;
	struct pin_pages *p;

	first = (addr->addr & PAGE_MASK) >> PAGE_SHIFT;
	last = ((addr->addr + addr->size - 1) & PAGE_MASK) >> PAGE_SHIFT;
	nr_pages = last - first + 1;

	/* find pin_pages */
	p = xa_load(&priv->array, first);
	if (!p)
		return -ENODEV;

	if (p->nr_pages != nr_pages)
		return -EINVAL;

	/* unpin */
	unpin_user_pages(p->pages, p->nr_pages);

	/* release resource */
	xa_erase(&priv->array, first);
	kfree(p->pages);
	kfree(p);

	return 0;
}

static int uacce_pin_check_param(struct uacce_pin_address *addr)
{
	if (addr->size > UACCE_MAX_PIN_PAGE * PAGE_SIZE)
		return -EINVAL;

	return 0;
}

static long uacce_ctrl_unl_ioctl(struct file *filep, unsigned int cmd,
				 unsigned long arg)
{
	struct uacce_pin_container *p = filep->private_data;
	struct uacce_pin_address addr;
	int ret;

	if (copy_from_user(&addr, (void __user *)arg,
			   sizeof(struct uacce_pin_address)))
		return -EFAULT;

	ret = uacce_pin_check_param(&addr);
	if (ret) {
		pr_err("uacce: Invalid pin input\n");
		return -EINVAL;
	}

	switch (cmd) {
	case UACCE_CMD_PIN:
		return uacce_pin_page(p, &addr);

	case UACCE_CMD_UNPIN:
		return uacce_unpin_page(p, &addr);

	default:
		return -EINVAL;
	}
}

const struct file_operations uacce_ctrl_fops = {
	.owner = THIS_MODULE,
	.open = uacce_ctrl_open,
	.release = uacce_ctrl_release,
	.unlocked_ioctl	= uacce_ctrl_unl_ioctl,
};

static struct miscdevice uacce_ctrl_miscdev = {
	.name = "uacce_ctrl",
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &uacce_ctrl_fops,
};

static int __init uacce_init(void)
{
	int ret;

	uacce_class = class_create(THIS_MODULE, UACCE_NAME);
	if (IS_ERR(uacce_class))
		return PTR_ERR(uacce_class);

	ret = alloc_chrdev_region(&uacce_devt, 0, MINORMASK, UACCE_NAME);
	if (ret)
		goto destroy_class;

	ret = misc_register(&uacce_ctrl_miscdev);
	if (ret) {
		pr_err("uacce: Pin misc registration failed\n");
		goto unregister_cdev;
	}

	return 0;

unregister_cdev:
	unregister_chrdev_region(uacce_devt, MINORMASK);
destroy_class:
	class_destroy(uacce_class);
	return ret;
}

static __exit void uacce_exit(void)
{
	misc_deregister(&uacce_ctrl_miscdev);
	unregister_chrdev_region(uacce_devt, MINORMASK);
	class_destroy(uacce_class);
}

subsys_initcall(uacce_init);
module_exit(uacce_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hisilicon Tech. Co., Ltd.");
MODULE_DESCRIPTION("Accelerator interface for Userland applications");

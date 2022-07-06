// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#include <linux/iommu.h>
#include <uapi/linux/iommufd.h>
#include <linux/file.h>
#include <linux/anon_inodes.h>
#include <linux/circ_buf.h>

#include "iommufd_private.h"

static void
iommufd_hw_pagetable_dma_fault_destroy(struct iommufd_hw_pagetable_s1 *s1);

void iommufd_hw_pagetable_destroy(struct iommufd_object *obj)
{
	struct iommufd_hw_pagetable *hwpt =
		container_of(obj, struct iommufd_hw_pagetable, obj);

	WARN_ON(!xa_empty(&hwpt->devices));
	if (hwpt->type == IOMMUFD_HWPT_KERNEL) {
		struct iommufd_ioas *ioas = hwpt->kernel.ioas;

		mutex_lock(&ioas->mutex);
		list_del(&hwpt->kernel.auto_domains_item);
		mutex_unlock(&ioas->mutex);
		WARN_ON(!list_empty(&hwpt->kernel.stage1_domains));
		mutex_destroy(&hwpt->kernel.mutex);
		refcount_dec(&ioas->obj.users);
	} else {
		struct iommufd_hw_pagetable *stage2 = hwpt->s1.stage2;

		mutex_lock(&stage2->kernel.mutex);
		list_del(&hwpt->s1.stage1_domains_item);
		mutex_unlock(&stage2->kernel.mutex);
		refcount_dec(&stage2->obj.users);
		iommufd_hw_pagetable_dma_fault_destroy(&hwpt->s1);
	}

	iommu_domain_free(hwpt->domain);
	mutex_destroy(&hwpt->devices_lock);
}

/*
 * When automatically managing the domains we search for a compatible domain in
 * the iopt and if one is found use it, otherwise create a new domain.
 * Automatic domain selection will never pick a manually created domain.
 */
static struct iommufd_hw_pagetable *
iommufd_hw_pagetable_auto_get(struct iommufd_ctx *ictx,
			      struct iommufd_ioas *ioas, struct device *dev)
{
	struct iommufd_hw_pagetable *hwpt;
	struct iommufd_hw_pagetable_kernel *kernel;
	int rc;

	/*
	 * There is no differentiation when domains are allocated, so any domain
	 * from the right ops is interchangeable with any other.
	 */
	mutex_lock(&ioas->mutex);
	list_for_each_entry (kernel, &ioas->auto_domains, auto_domains_item) {
		hwpt = container_of(kernel, struct iommufd_hw_pagetable, kernel);
		/*
		 * FIXME: We really need an op from the driver to test if a
		 * device is compatible with a domain. This thing from VFIO
		 * works sometimes.
		 */
		if (hwpt->domain->ops == dev_iommu_ops(dev)->default_domain_ops) {
			if (refcount_inc_not_zero(&hwpt->obj.users)) {
				mutex_unlock(&ioas->mutex);
				return hwpt;
			}
		}
	}

	hwpt = iommufd_object_alloc(ictx, hwpt, IOMMUFD_OBJ_HW_PAGETABLE);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_unlock;
	}

	hwpt->domain = iommu_domain_alloc(dev->bus);
	if (!hwpt->domain) {
		rc = -ENOMEM;
		goto out_abort;
	}

	/* FixMe: We need proper interface to enable nesting for SMMUv3. */
	rc = iommu_enable_nesting(hwpt->domain);
	if (rc)
		goto out_abort;

	xa_init_flags(&hwpt->devices, XA_FLAGS_ALLOC1);
	mutex_init(&hwpt->devices_lock);
	hwpt->type = IOMMUFD_HWPT_KERNEL;
	kernel = &hwpt->kernel;
	kernel->ioas = ioas;
	INIT_LIST_HEAD(&kernel->stage1_domains);
	mutex_init(&kernel->mutex);

	/* The calling driver is a user until iommufd_hw_pagetable_put() */
	refcount_inc(&ioas->obj.users);

	list_add_tail(&kernel->auto_domains_item, &ioas->auto_domains);
	/*
	 * iommufd_object_finalize() consumes the refcount, get one for the
	 * caller. This pairs with the first put in
	 * iommufd_object_destroy_user()
	 */
	refcount_inc(&hwpt->obj.users);
	iommufd_object_finalize(ictx, &hwpt->obj);

	mutex_unlock(&ioas->mutex);
	return hwpt;

out_abort:
	iommufd_object_abort(ictx, &hwpt->obj);
out_unlock:
	mutex_unlock(&ioas->mutex);
	return ERR_PTR(rc);
}

/**
 * iommufd_hw_pagetable_from_id() - Get an iommu_domain for a device
 * @ictx: iommufd context
 * @pt_id: ID of the IOAS or hw_pagetable object
 * @dev: Device to get an iommu_domain for
 *
 * Turn a general page table ID into an iommu_domain contained in a
 * iommufd_hw_pagetable object. If a hw_pagetable ID is specified then that
 * iommu_domain is used, otherwise a suitable iommu_domain in the IOAS is found
 * for the device, creating one automatically if necessary.
 */
struct iommufd_hw_pagetable *
iommufd_hw_pagetable_from_id(struct iommufd_ctx *ictx, u32 pt_id,
			     struct device *dev)
{
	struct iommufd_object *obj;

	obj = iommufd_get_object(ictx, pt_id, IOMMUFD_OBJ_ANY);
	if (IS_ERR(obj))
		return ERR_CAST(obj);

	switch (obj->type) {
	case IOMMUFD_OBJ_HW_PAGETABLE:
	case IOMMUFD_OBJ_HW_PAGETABLE_S1:
		iommufd_put_object_keep_user(obj);
		return container_of(obj, struct iommufd_hw_pagetable, obj);
	case IOMMUFD_OBJ_IOAS: {
		struct iommufd_ioas *ioas =
			container_of(obj, struct iommufd_ioas, obj);
		struct iommufd_hw_pagetable *hwpt;

		hwpt = iommufd_hw_pagetable_auto_get(ictx, ioas, dev);
		iommufd_put_object(obj);
		return hwpt;
	}
	default:
		iommufd_put_object(obj);
		return ERR_PTR(-EINVAL);
	}
}

void iommufd_hw_pagetable_put(struct iommufd_ctx *ictx,
			      struct iommufd_hw_pagetable *hwpt)
{
	if (hwpt->type == IOMMUFD_HWPT_USER_S1) {
		/* Manually created hw_pagetables just keep going */
		refcount_dec(&hwpt->obj.users);
		return;
	}
	iommufd_object_destroy_user(ictx, &hwpt->obj);
}

static int iommufd_hw_pagetable_eventfd_setup(struct eventfd_ctx **ctx, int fd)
{
	struct eventfd_ctx *efdctx;

	efdctx = eventfd_ctx_fdget(fd);
	if (IS_ERR(efdctx))
		return PTR_ERR(efdctx);
	if (*ctx)
		eventfd_ctx_put(*ctx);
	*ctx = efdctx;
	return 0;
}

static void iommufd_hw_pagetable_eventfd_destroy(struct eventfd_ctx **ctx)
{
	eventfd_ctx_put(*ctx);
	*ctx = NULL;
}

static ssize_t hwpt_fault_fops_read(struct file *filep, char __user *buf,
				    size_t count, loff_t *ppos)
{
	struct iommufd_hw_pagetable_s1 *s1 = filep->private_data;
	loff_t pos = *ppos;
	void *base = s1->fault_pages;
	size_t size = s1->fault_region_size;
	int ret = -EFAULT;

	if (pos >= size)
		return -EINVAL;

	count = min(count, (size_t)(size - pos));

	mutex_lock(&s1->fault_queue_lock);
	if (!copy_to_user(buf, base + pos, count)) {
		*ppos += count;
		ret = count;
	}
	mutex_unlock(&s1->fault_queue_lock);

	return ret;
}

static ssize_t hwpt_fault_fops_write(struct file *filep,
				     const char __user *buf,
				     size_t count, loff_t *ppos)
{
	struct iommufd_hw_pagetable_s1 *s1 = filep->private_data;
	loff_t pos = *ppos;
	void *base = s1->fault_pages;
	struct iommufd_stage1_dma_fault *header =
			(struct iommufd_stage1_dma_fault *)base;
	size_t size = s1->fault_region_size;
	u32 new_tail;
	int ret = -EFAULT;

	if (pos >= size)
		return -EINVAL;

	count = min(count, (size_t)(size - pos));

	mutex_lock(&s1->fault_queue_lock);

	/* Only allows write to the tail which locates at offset 0 */
	if (pos != 0 || count != 4) {
		ret = -EINVAL;
		goto unlock;
	}

	if (copy_from_user((void *)&new_tail, buf, count))
		goto unlock;

	/* new tail should not exceed the maximum index */
	if (new_tail > header->nb_entries) {
		ret = -EINVAL;
		goto unlock;
	}

	/* update the tail value */
	header->tail = new_tail;
	ret = count;

unlock:
	mutex_unlock(&s1->fault_queue_lock);
	return ret;
}

static const struct file_operations hwpt_fault_fops = {
	.owner		= THIS_MODULE,
	.read		= hwpt_fault_fops_read,
	.write		= hwpt_fault_fops_write,
};

static int iommufd_hw_pagetable_get_fault_fd(struct iommufd_hw_pagetable_s1 *s1)
{
	struct file *filep;
	int fdno, ret;

	fdno = ret = get_unused_fd_flags(O_CLOEXEC);
	if (ret < 0)
		return ret;

	filep = anon_inode_getfile("[hwpt-fault]", &hwpt_fault_fops,
				   s1, O_RDWR);
	if (IS_ERR(filep)) {
		put_unused_fd(fdno);
		return PTR_ERR(filep);
	}

	filep->f_mode |= (FMODE_LSEEK | FMODE_PREAD | FMODE_PWRITE);
	fd_install(fdno, filep);

	s1->fault_file = filep;
	s1->fault_fd = fdno;

	return 0;
}

static enum iommu_page_response_code
iommufd_hw_pagetable_iopf_handler(struct iommu_fault *fault,
				  struct device *dev, void *data)
{
	struct iommufd_hw_pagetable_s1 *s1 =
				(struct iommufd_hw_pagetable_s1 *)data;
	struct iommufd_hw_pagetable *hwpt =
		container_of(s1, struct iommufd_hw_pagetable, s1);
	struct iommufd_stage1_dma_fault *header =
		(struct iommufd_stage1_dma_fault *)s1->fault_pages;
	struct iommu_fault *new;
	int head, tail, size;
	u32 dev_id;
	ioasid_t pasid = (fault->prm.flags & IOMMU_FAULT_PAGE_REQUEST_PASID_VALID) ?
			 fault->prm.pasid : INVALID_IOASID;
	enum iommu_page_response_code resp = IOMMU_PAGE_RESP_ASYNC;

	if (WARN_ON(!header))
		return IOMMU_PAGE_RESP_FAILURE;

	if (dev) {
		dev_id = iommufd_hw_pagetable_get_dev_id(hwpt, dev, pasid);
		if (dev_id == IOMMUFD_INVALID_ID)
			return IOMMU_PAGE_RESP_FAILURE;

		fault->dev_id = dev_id;
	}
	mutex_lock(&s1->fault_queue_lock);

	new = (struct iommu_fault *)(s1->fault_pages + header->offset +
				     header->head * header->entry_size);

	pr_debug("%s, enque fault event\n", __func__);
	head = header->head;
	tail = header->tail;
	size = header->nb_entries;

	if (CIRC_SPACE(head, tail, size) < 1) {
		resp = IOMMU_PAGE_RESP_FAILURE;
		goto unlock;
	}

	*new = *fault;
	header->head = (head + 1) % size;
unlock:
	mutex_unlock(&s1->fault_queue_lock);
	if (resp != IOMMU_PAGE_RESP_ASYNC)
		return resp;

	mutex_lock(&s1->notify_gate);
	pr_debug("%s, signal userspace!\n", __func__);
	if (s1->trigger)
		eventfd_signal(s1->trigger, 1);
	mutex_unlock(&s1->notify_gate);

	return resp;
}

static int
iommufd_hw_pagetable_dev_fault_handler(struct iommu_fault *fault, void *data)
{
	enum iommu_page_response_code resp;

	/* Hack: FixMe: for now we are just using the iopf fault handler */
	resp = iommufd_hw_pagetable_iopf_handler(fault, NULL, data);
	if (resp == IOMMU_PAGE_RESP_FAILURE) {
		printk("%s: resp %d\n", __func__, resp);
		return resp;
	}

	return 0;
}

#define DMA_FAULT_RING_LENGTH 512

static int
iommufd_hw_pagetable_dma_fault_init(struct iommufd_hw_pagetable_s1 *s1,
				    int eventfd)
{
	struct iommufd_stage1_dma_fault *header;
	size_t size;
	int rc;

	mutex_init(&s1->fault_queue_lock);
	mutex_init(&s1->notify_gate);

	/*
	 * We provision 1 page for the header and space for
	 * DMA_FAULT_RING_LENGTH fault records in the ring buffer.
	 */
	size = ALIGN(sizeof(struct iommu_fault) *
		     DMA_FAULT_RING_LENGTH, PAGE_SIZE) + PAGE_SIZE;

	s1->fault_pages = kzalloc(size, GFP_KERNEL);
	if (!s1->fault_pages)
		return -ENOMEM;

	header = (struct iommufd_stage1_dma_fault *)s1->fault_pages;
	header->entry_size = sizeof(struct iommu_fault);
	header->nb_entries = DMA_FAULT_RING_LENGTH;
	header->offset = PAGE_SIZE;
	s1->fault_region_size = size;

	rc = iommufd_hw_pagetable_eventfd_setup(&s1->trigger, eventfd);
	if (rc)
		goto out_free;

	rc = iommufd_hw_pagetable_get_fault_fd(s1);
	if (rc)
		goto out_destroy_eventfd;

	return rc;

out_destroy_eventfd:
	iommufd_hw_pagetable_eventfd_destroy(&s1->trigger);
out_free:
	kfree(s1->fault_pages);
	return rc;
}

static void
iommufd_hw_pagetable_dma_fault_destroy(struct iommufd_hw_pagetable_s1 *s1)
{
	struct iommufd_stage1_dma_fault *header =
		(struct iommufd_stage1_dma_fault *)s1->fault_pages;

	WARN_ON(header->tail != header->head);
	iommufd_hw_pagetable_eventfd_destroy(&s1->trigger);
	kfree(s1->fault_pages);
	mutex_destroy(&s1->fault_queue_lock);
	mutex_destroy(&s1->notify_gate);
}

int iommufd_alloc_s1_hwpt(struct iommufd_ucmd *ucmd)
{
	struct iommu_alloc_s1_hwpt *cmd = ucmd->cmd;
	struct iommufd_object *stage2_obj;
	struct iommufd_object *dev_obj;
	struct iommufd_device *idev;
	struct iommufd_hw_pagetable *hwpt;
	struct iommufd_hw_pagetable *stage2;
	struct iommufd_hw_pagetable_s1 *s1;
	union iommu_stage1_config s1_config;
	int rc;

	if (cmd->flags)
		return -EOPNOTSUPP;

	if (cmd->eventfd < 0)
		return -EINVAL;

	rc = copy_struct_from_user(&s1_config, sizeof(s1_config),
				   (void __user *)cmd->stage1_config_uptr,
				   cmd->stage1_config_len);
	if (rc)
		return rc;

	stage2_obj = iommufd_get_object(ucmd->ictx, cmd->stage2_hwpt_id,
					IOMMUFD_OBJ_HW_PAGETABLE);
	if (IS_ERR(stage2_obj))
		return PTR_ERR(stage2_obj);

	stage2 = container_of(stage2_obj, struct iommufd_hw_pagetable, obj);
	if (stage2->type != IOMMUFD_HWPT_KERNEL) {
		rc = -EINVAL;
		goto out_put_stage2;
	}

	dev_obj = iommufd_get_object(ucmd->ictx, cmd->dev_id,
				     IOMMUFD_OBJ_DEVICE);
	if (IS_ERR(dev_obj)) {
		rc = PTR_ERR(dev_obj);
		goto out_put_stage2;
	}

	idev = container_of(dev_obj, struct iommufd_device, obj);

	hwpt = iommufd_object_alloc(ucmd->ictx, hwpt, IOMMUFD_OBJ_HW_PAGETABLE_S1);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_put_dev;
	}

	xa_init_flags(&hwpt->devices, XA_FLAGS_ALLOC1);
	mutex_init(&hwpt->devices_lock);
	hwpt->type = IOMMUFD_HWPT_USER_S1;

	hwpt->domain = iommu_alloc_nested_domain(idev->dev->bus,
						 stage2->domain,
						 cmd->stage1_ptr,
						 &s1_config);
	if (!hwpt->domain) {
		rc = -ENOMEM;
		goto out_abort;
	}

	s1 = &hwpt->s1;
	s1->stage2 = stage2;

	rc = iommufd_hw_pagetable_dma_fault_init(&hwpt->s1, cmd->eventfd);
	if (rc)
		goto out_free_domain;

	cmd->out_hwpt_id = hwpt->obj.id;
	cmd->out_fault_fd = s1->fault_fd;

	rc = iommufd_ucmd_respond(ucmd, sizeof(*cmd));
	if (rc)
		goto out_destroy_dma;

	/*Hack: FixMe: This is for ARM SMMUv3 nested mode */
	rc = iommu_register_device_fault_handler(idev->dev,
						 iommufd_hw_pagetable_dev_fault_handler, s1);
	if (rc)
		goto out_destroy_dma;

	mutex_lock(&stage2->kernel.mutex);
	list_add_tail(&s1->stage1_domains_item, &stage2->kernel.stage1_domains);
	mutex_unlock(&stage2->kernel.mutex);
	iommufd_object_finalize(ucmd->ictx, &hwpt->obj);
	/* No need to hold refcount on dev_obj per hwpt allocation */
	iommufd_put_object(dev_obj);
	/* Caller is a user of stage2 until detach */
	iommufd_put_object_keep_user(stage2_obj);

	return 0;
out_destroy_dma:
	iommufd_hw_pagetable_dma_fault_destroy(&hwpt->s1);
out_free_domain:
	iommu_domain_free(hwpt->domain);
out_abort:
	iommufd_object_abort(ucmd->ictx, &hwpt->obj);
out_put_dev:
	iommufd_put_object(dev_obj);
out_put_stage2:
	iommufd_put_object(stage2_obj);
	return rc;
}

int iommufd_hwpt_invalidate_cache(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_invalidate_s1_cache *cmd = ucmd->cmd;
	struct iommufd_object *obj;
	struct iommufd_hw_pagetable *hwpt;
	int rc = 0;

	if (cmd->flags)
		return -EOPNOTSUPP;

	/* TODO: more sanity check when the struct is finalized */
	obj = iommufd_get_object(ucmd->ictx, cmd->hwpt_id,
				 IOMMUFD_OBJ_HW_PAGETABLE_S1);
	if (IS_ERR(obj))
		return PTR_ERR(obj);

	hwpt = container_of(obj, struct iommufd_hw_pagetable, obj);
	if (hwpt->type != IOMMUFD_HWPT_USER_S1) {
		rc = -EINVAL;
		goto out_put;
	}

	iommu_domain_cache_inv(hwpt->domain, &cmd->info);

out_put:
	iommufd_put_object(obj);
	return rc;
}

int iommufd_hwpt_page_response(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_page_response *cmd = ucmd->cmd;
	struct iommufd_object *obj, *dev_obj;
	struct iommufd_hw_pagetable *hwpt;
	struct iommufd_device *idev;
	int rc = 0;

	if (cmd->flags)
		return -EOPNOTSUPP;

	/* TODO: more sanity check when the struct is finalized */
	obj = iommufd_get_object(ucmd->ictx, cmd->hwpt_id,
				 IOMMUFD_OBJ_HW_PAGETABLE_S1);
	if (IS_ERR(obj))
		return PTR_ERR(obj);

	hwpt = container_of(obj, struct iommufd_hw_pagetable, obj);
	if (hwpt->type != IOMMUFD_HWPT_USER_S1) {
		rc = -EINVAL;
		goto out_put_hwpt;
	}

	dev_obj = iommufd_get_object(ucmd->ictx,
				     cmd->dev_id, IOMMUFD_OBJ_DEVICE);
	if (IS_ERR(dev_obj)) {
		rc = PTR_ERR(obj);
		goto out_put_hwpt;
	}

	idev = container_of(dev_obj, struct iommufd_device, obj);
	rc = iommu_page_response(idev->dev, &cmd->resp);
	iommufd_put_object(dev_obj);
out_put_hwpt:
	iommufd_put_object(obj);
	return rc;
}

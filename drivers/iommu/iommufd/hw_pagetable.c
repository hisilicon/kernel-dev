// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#include <linux/iommu.h>
#include <uapi/linux/iommufd.h>
#include <linux/eventfd.h>
#include <linux/file.h>
#include <linux/anon_inodes.h>
#include <linux/circ_buf.h>

#include "iommufd_private.h"
#include "iommufd_test.h"

static void iommufd_hw_pagetable_eventfd_destroy(struct eventfd_ctx **ctx)
{
	eventfd_ctx_put(*ctx);
	*ctx = NULL;
}

static void
iommufd_hw_pagetable_dma_fault_destroy(struct iommufd_hw_pagetable *hwpt)
{
	struct iommufd_hw_s1_fault_data *s1 = &hwpt->s1_fault_data;
	struct iommufd_stage1_dma_fault *header =
		(struct iommufd_stage1_dma_fault *)s1->fault_pages;

	WARN_ON(header->tail != header->head);
	iommufd_hw_pagetable_eventfd_destroy(&s1->trigger);
	kfree(s1->fault_pages);
	mutex_destroy(&s1->fault_queue_lock);
	mutex_destroy(&s1->notify_gate);
}

void iommufd_hw_pagetable_destroy(struct iommufd_object *obj)
{
	struct iommufd_hw_pagetable *hwpt =
		container_of(obj, struct iommufd_hw_pagetable, obj);

	WARN_ON(!list_empty(&hwpt->devices));

	iommufd_hw_pagetable_dma_fault_destroy(hwpt);
	iommu_domain_free(hwpt->domain);
	refcount_dec(&hwpt->ioas->obj.users);
	if (hwpt->parent) {
		refcount_dec(&hwpt->parent->obj.users);
	} else {
		WARN_ON(!refcount_dec_if_one(hwpt->devices_users));
		mutex_destroy(hwpt->devices_lock);
		kfree(hwpt->devices_lock);
	}
}

static struct iommufd_hw_pagetable *
__iommufd_hw_pagetable_alloc(struct iommufd_ctx *ictx,
			     struct iommufd_ioas *ioas,
			     struct device *dev,
			     struct iommufd_hw_pagetable *parent,
			     void *user_data)
{
	const struct iommu_ops *ops;
	struct iommu_domain *parent_domain = NULL;
	struct iommufd_hw_pagetable *hwpt;
	int rc;

	if (WARN_ON(!ioas && !parent))
		return ERR_PTR(-EINVAL);

	hwpt = iommufd_object_alloc(ictx, hwpt, IOMMUFD_OBJ_HW_PAGETABLE);
	if (IS_ERR(hwpt))
		return hwpt;

	ops = dev_iommu_ops(dev);
	if (!ops || !ops->domain_alloc_user) {
		rc = -EOPNOTSUPP;
		goto out_abort;
	}

	if (parent)
		parent_domain = parent->domain;

	hwpt->domain = ops->domain_alloc_user(dev, parent_domain, user_data);
	if (!hwpt->domain) {
		rc = -ENOMEM;
		goto out_abort;
	}

	if (!hwpt->domain->ops)
		hwpt->domain->ops = ops->default_domain_ops;

	if (parent)
		hwpt->domain->type = IOMMU_DOMAIN_NESTED;
	else
		hwpt->domain->type = IOMMU_DOMAIN_UNMANAGED;

	INIT_LIST_HEAD(&hwpt->devices);
	INIT_LIST_HEAD(&hwpt->hwpt_item);
	hwpt->parent = parent;
	if (parent) {
		/* Always reuse parent's devices_lock and devices_users... */
		hwpt->devices_lock = parent->devices_lock;
		hwpt->devices_users = parent->devices_users;
		refcount_inc(&parent->obj.users);
	} else {
		/* ...otherwise, allocate a new pair */
		hwpt->devices_lock = kzalloc(sizeof(*hwpt->devices_lock) +
					     sizeof(*hwpt->devices_users),
					     GFP_KERNEL);
		if (!hwpt->devices_lock) {
			rc = -ENOMEM;
			goto out_free_domain;
		}
		mutex_init(hwpt->devices_lock);
		hwpt->devices_users = (refcount_t *)&hwpt->devices_lock[1];
		refcount_set(hwpt->devices_users, 1);
	}

	/* Pairs with iommufd_hw_pagetable_destroy() */
	refcount_inc(&ioas->obj.users);
	hwpt->ioas = ioas;
	return hwpt;

out_free_domain:
	iommu_domain_free(hwpt->domain);
out_abort:
	iommufd_object_abort(ictx, &hwpt->obj);
	return ERR_PTR(rc);
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

static ssize_t hwpt_fault_fops_read(struct file *filep, char __user *buf,
				    size_t count, loff_t *ppos)
{
	struct iommufd_hw_s1_fault_data *s1 = filep->private_data;
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
	struct iommufd_hw_s1_fault_data *s1 = filep->private_data;
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

static int iommufd_hw_pagetable_get_fault_fd(struct iommufd_hw_s1_fault_data *s1)
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
	struct iommufd_hw_pagetable *hwpt =
				(struct iommufd_hw_pagetable *)data;
	struct iommufd_hw_s1_fault_data *s1 = &hwpt->s1_fault_data;
	struct iommufd_stage1_dma_fault *header =
		(struct iommufd_stage1_dma_fault *)s1->fault_pages;
	struct iommu_fault *new;
	int head, tail, size;
	enum iommu_page_response_code resp = IOMMU_PAGE_RESP_ASYNC;

	if (WARN_ON(!header))
		return IOMMU_PAGE_RESP_FAILURE;

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

	resp = iommufd_hw_pagetable_iopf_handler(fault, NULL, data);
	if (resp == IOMMU_PAGE_RESP_FAILURE)
		return resp;

	return 0;
}

#define DMA_FAULT_RING_LENGTH 512

static int
iommufd_hw_pagetable_dma_fault_init(struct iommufd_hw_pagetable *hwpt,
				    int eventfd)
{
	struct iommufd_stage1_dma_fault *header;
	struct iommufd_hw_s1_fault_data *s1 = &hwpt->s1_fault_data;
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
/**
 * iommufd_hw_pagetable_alloc() - Get an iommu_domain for a device
 * @ictx: iommufd context
 * @ioas: IOAS to associate the domain with
 * @dev: Device to get an iommu_domain for
 *
 * Allocate a new iommu_domain and return it as a hw_pagetable.
 */
struct iommufd_hw_pagetable *
iommufd_hw_pagetable_alloc(struct iommufd_ctx *ictx, struct iommufd_ioas *ioas,
			   struct device *dev)
{
	return __iommufd_hw_pagetable_alloc(ictx, ioas, dev, NULL, NULL);
}

/*
 * size of page table type specific data, indexed by
 * enum iommu_pgtbl_types.
 */
static const size_t iommufd_hwpt_info_size[] = {
	[IOMMU_PGTBL_TYPE_NONE] = 0,
	[IOMMU_PGTBL_TYPE_VTD_S1] = sizeof(struct iommu_hwpt_intel_vtd),
	[IOMMU_PGTBL_TYPE_SMMUV3_S1] = sizeof(struct iommu_hwpt_arm_smmuv3),
};

/* Return true if type is supported, otherwise false */
static inline bool
iomufd_hwpt_type_check(enum iommu_device_data_type driver_type,
		       enum iommu_pgtbl_types type)
{
	return ((1 << type) &
			iommufd_supported_pgtbl_types[driver_type]);
}

int iommufd_hwpt_alloc(struct iommufd_ucmd *ucmd)
{
	struct iommufd_hw_pagetable *hwpt, *parent = NULL;
	struct iommu_hwpt_alloc *cmd = ucmd->cmd;
	struct iommufd_ctx *ictx = ucmd->ictx;
	struct iommufd_object *pt_obj = NULL;
	struct iommufd_ioas *ioas = NULL;
	struct iommufd_object *dev_obj;
	struct device *dev;
	const struct iommu_ops *ops;
	void *data = NULL;
	u32 driver_type, klen;
	int rc;
	bool support;

	if (cmd->__reserved || cmd->flags)
		return -EOPNOTSUPP;

	dev_obj = iommufd_get_object(ucmd->ictx, cmd->dev_id,
				     IOMMUFD_OBJ_ANY);
	if (IS_ERR(dev_obj))
		return PTR_ERR(dev_obj);

	dev = iommufd_obj_dev(dev_obj);
	if (!dev) {
		rc = -EINVAL;
		goto out_put_dev;
	}

	ops = dev_iommu_ops(dev);
	if (!ops) {
		rc = -EOPNOTSUPP;
		goto out_put_dev;
	}

	driver_type = ops->driver_type;

	/* data_type should be a supported type by the hardware */
	if (cmd->data_type != IOMMU_PGTBL_TYPE_SELFTTEST)
		support = iomufd_hwpt_type_check(driver_type,
						 cmd->data_type);
#ifdef CONFIG_IOMMUFD_TEST
	else
		support = true; /* selftest pretend to support all types */
#endif
	if (!support) {
		rc = -EINVAL;
		goto out_put_dev;
	}

	pt_obj = iommufd_get_object(ictx, cmd->pt_id, IOMMUFD_OBJ_ANY);
	if (IS_ERR(pt_obj)) {
		rc = -EINVAL;
		goto out_put_dev;
	}

	switch (pt_obj->type) {
	case IOMMUFD_OBJ_HW_PAGETABLE:
		parent = container_of(pt_obj, struct iommufd_hw_pagetable, obj);
		/*
		 * Cannot allocate user-managed hwpt linking to auto_created
		 * hwpt. If the parent hwpt is already a user-managed hwpt,
		 * don't allocate another user-managed hwpt linking to it.
		 */
		if (parent->auto_domain || parent->parent) {
			rc = -EINVAL;
			goto out_put_pt;
		}
		ioas = parent->ioas;
		break;
	case IOMMUFD_OBJ_IOAS:
		ioas = container_of(pt_obj, struct iommufd_ioas, obj);
		break;
	default:
		rc = -EINVAL;
		goto out_put_pt;
	}

	if (cmd->data_type != IOMMU_PGTBL_TYPE_SELFTTEST)
		klen = iommufd_hwpt_info_size[cmd->data_type];
#ifdef CONFIG_IOMMUFD_TEST
	else
		klen = sizeof(struct iommu_hwpt_selftest);
#endif
	if (klen) {
		if (!cmd->data_len) {
			rc = -EINVAL;
			goto out_put_pt;
		}

		data = kzalloc(klen, GFP_KERNEL);
		if (!data) {
			rc = -ENOMEM;
			goto out_put_pt;
		}

		rc = copy_struct_from_user(data, klen,
					   u64_to_user_ptr(cmd->data_uptr),
					   cmd->data_len);
		if (rc)
			goto out_free_data;
	}

	mutex_lock(&ioas->mutex);
	hwpt = __iommufd_hw_pagetable_alloc(ictx, ioas, dev, parent, data);
	mutex_unlock(&ioas->mutex);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_free_data;
	}

	/* Hack: Register device fault handler if userspace sets eventfd */
	if (cmd->eventfd >= 0) {
		rc = iommufd_hw_pagetable_dma_fault_init(hwpt, cmd->eventfd);
		if (rc)
			goto out_destroy_hwpt;

		cmd->out_fault_fd = hwpt->s1_fault_data.fault_fd;

		rc = iommu_register_device_fault_handler(dev,
				iommufd_hw_pagetable_dev_fault_handler, hwpt);
		if (rc)
			goto out_destroy_hwpt;
	}

	cmd->out_hwpt_id = hwpt->obj.id;

	rc = iommufd_ucmd_respond(ucmd, sizeof(*cmd));
	if (rc)
		goto out_destroy_hwpt;

	kfree(data);
	iommufd_object_finalize(ucmd->ictx, &hwpt->obj);
	iommufd_put_object(pt_obj);
	iommufd_put_object(dev_obj);
	return 0;
out_destroy_hwpt:
	iommufd_object_abort_and_destroy(ucmd->ictx, &hwpt->obj);
out_free_data:
	kfree(data);
out_put_pt:
	iommufd_put_object(pt_obj);
out_put_dev:
	iommufd_put_object(dev_obj);
	return rc;
}

static u32 iommufd_hwpt_invalidate_info_size[] = {
	[IOMMU_PGTBL_TYPE_VTD_S1] = sizeof(struct iommu_hwpt_invalidate_intel_vtd),
	[IOMMU_PGTBL_TYPE_SMMUV3_S1] = sizeof(struct iommu_hwpt_invalidate_arm_smmuv3),
};

int iommufd_hwpt_invalidate(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_invalidate *cmd = ucmd->cmd;
	struct iommufd_hw_pagetable *hwpt;
	u64 user_ptr;
	u32 user_data_len, klen;
	int rc = 0;

	/*
	 * No invalidation needed for type==IOMMU_PGTBL_TYPE_NONE.
	 * data_len should not exceed the size of iommufd_invalidate_buffer.
	 */
	if (cmd->data_type == IOMMU_PGTBL_TYPE_NONE || !cmd->data_len)
		return -EOPNOTSUPP;

	hwpt = iommufd_get_hwpt(ucmd, cmd->hwpt_id);
	if (IS_ERR(hwpt))
		return PTR_ERR(hwpt);

	/* Do not allow any kernel-managed hw_pagetable */
	if (!hwpt->parent) {
		rc = -EINVAL;
		goto out_put_hwpt;
	}

	if (cmd->data_type != IOMMU_PGTBL_TYPE_SELFTTEST)
		klen = iommufd_hwpt_invalidate_info_size[cmd->data_type];
#ifdef CONFIG_IOMMUFD_TEST
	else
		klen = sizeof(struct iommu_hwpt_invalidate_selftest);
#endif
	if (!klen) {
		rc = -EINVAL;
		goto out_put_hwpt;
	}

	/*
	 * copy the needed fields before reusing the ucmd buffer, this
	 * avoids memory allocation in this path.
	 */
	user_ptr = cmd->data_uptr;
	user_data_len = cmd->data_len;

	rc = copy_struct_from_user(cmd, klen,
				   u64_to_user_ptr(user_ptr), user_data_len);
	if (rc)
		goto out_put_hwpt;

	hwpt->domain->ops->iotlb_sync_user(hwpt->domain, cmd);
out_put_hwpt:
	iommufd_put_object(&hwpt->obj);
	return rc;
}

int iommufd_hwpt_page_response(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_page_response *cmd = ucmd->cmd;
	struct iommufd_object *dev_obj;
	struct device *dev;
	int rc = 0;

	if (cmd->flags)
		return -EOPNOTSUPP;

	dev_obj = iommufd_get_object(ucmd->ictx, cmd->dev_id,
				     IOMMUFD_OBJ_ANY);
	if (IS_ERR(dev_obj))
		return PTR_ERR(dev_obj);

	dev = iommufd_obj_dev(dev_obj);
	if (!dev) {
		rc = -EINVAL;
		goto out;
	}

	rc = iommu_page_response(dev, &cmd->resp);
out:
	iommufd_put_object(dev_obj);
	return rc;
}

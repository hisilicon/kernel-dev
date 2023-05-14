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

#include "../iommu-priv.h"
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

	if (!header)
		return;

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

	if (!list_empty(&hwpt->hwpt_item)) {
		mutex_lock(&hwpt->ioas->mutex);
		list_del(&hwpt->hwpt_item);
		mutex_unlock(&hwpt->ioas->mutex);

		iopt_table_remove_domain(&hwpt->ioas->iopt, hwpt->domain);
	}

	iommufd_hw_pagetable_dma_fault_destroy(hwpt);

	if (hwpt->domain)
		iommu_domain_free(hwpt->domain);

	if (hwpt->parent)
		refcount_dec(&hwpt->parent->obj.users);
	refcount_dec(&hwpt->ioas->obj.users);
}

void iommufd_hw_pagetable_abort(struct iommufd_object *obj)
{
	struct iommufd_hw_pagetable *hwpt =
		container_of(obj, struct iommufd_hw_pagetable, obj);

	/* The ioas->mutex must be held until finalize is called. */
	lockdep_assert_held(&hwpt->ioas->mutex);

	if (!list_empty(&hwpt->hwpt_item)) {
		list_del_init(&hwpt->hwpt_item);
		iopt_table_remove_domain(&hwpt->ioas->iopt, hwpt->domain);
	}
	iommufd_hw_pagetable_destroy(obj);
}

int iommufd_hw_pagetable_enforce_cc(struct iommufd_hw_pagetable *hwpt)
{
	if (hwpt->enforce_cache_coherency)
		return 0;

	if (hwpt->domain->ops->enforce_cache_coherency)
		hwpt->enforce_cache_coherency =
			hwpt->domain->ops->enforce_cache_coherency(
				hwpt->domain);
	if (!hwpt->enforce_cache_coherency)
		return -EINVAL;
	return 0;
}

static int iommufd_hw_pagetable_link_ioas(struct iommufd_hw_pagetable *hwpt)
{
	int rc;

	/*
	 * Only a parent hwpt needs to be linked to the IOAS. And a hwpt->parent
	 * must be linked to the IOAS already, when it's being allocated.
	 */
	if (hwpt->parent)
		return 0;

	rc = iopt_table_add_domain(&hwpt->ioas->iopt, hwpt->domain);
	if (rc)
		return rc;
	list_add_tail(&hwpt->hwpt_item, &hwpt->ioas->hwpt_list);
	return 0;
}

/**
 * iommufd_hw_pagetable_alloc() - Get an iommu_domain for a device
 * @ictx: iommufd context
 * @ioas: IOAS to associate the domain with
 * @idev: Device to get an iommu_domain for
 * @parent: Optional parent HWPT to associate with the domain with
 * @user_data: Optional user_data pointer
 * @immediate_attach: True if idev should be attached to the hwpt
 *
 * Allocate a new iommu_domain and return it as a hw_pagetable. The HWPT
 * will be linked to the given ioas and upon return the underlying iommu_domain
 * is fully popoulated.
 *
 * The caller must hold the ioas->mutex until after
 * iommufd_object_abort_and_destroy() or iommufd_object_finalize() is called on
 * the returned hwpt.
 */
struct iommufd_hw_pagetable *
iommufd_hw_pagetable_alloc(struct iommufd_ctx *ictx, struct iommufd_ioas *ioas,
			   struct iommufd_device *idev,
			   struct iommufd_hw_pagetable *parent,
			   void *user_data, bool immediate_attach)
{
	const struct iommu_ops *ops = dev_iommu_ops(idev->dev);
	struct iommu_domain *parent_domain = NULL;
	struct iommufd_hw_pagetable *hwpt;
	int rc;

	lockdep_assert_held(&ioas->mutex);

	if ((user_data || parent) && !ops->domain_alloc_user)
		return ERR_PTR(-EOPNOTSUPP);

	hwpt = iommufd_object_alloc(ictx, hwpt, IOMMUFD_OBJ_HW_PAGETABLE);
	if (IS_ERR(hwpt))
		return hwpt;

	INIT_LIST_HEAD(&hwpt->hwpt_item);
	/* Pairs with iommufd_hw_pagetable_destroy() */
	refcount_inc(&ioas->obj.users);
	hwpt->ioas = ioas;
	if (parent) {
		hwpt->parent = parent;
		parent_domain = parent->domain;
		refcount_inc(&parent->obj.users);
	}

	if (ops->domain_alloc_user)
		hwpt->domain = ops->domain_alloc_user(idev->dev,
						      parent_domain, user_data);
	else
		hwpt->domain = iommu_domain_alloc(idev->dev->bus);
	if (!hwpt->domain) {
		rc = -ENOMEM;
		goto out_abort;
	}

	/* It must be either NESTED or UNMANAGED, depending on parent_domain */
	if (WARN_ON((parent_domain && hwpt->domain->type != IOMMU_DOMAIN_NESTED) ||
		    (!parent_domain && hwpt->domain->type != IOMMU_DOMAIN_UNMANAGED))) {
		rc = -EINVAL;
		goto out_abort;
	}

	/*
	 * Set the coherency mode before we do iopt_table_add_domain() as some
	 * iommus have a per-PTE bit that controls it and need to decide before
	 * doing any maps. It is an iommu driver bug to report
	 * IOMMU_CAP_ENFORCE_CACHE_COHERENCY but fail enforce_cache_coherency on
	 * a new domain.
	 */
	if (idev->enforce_cache_coherency) {
		rc = iommufd_hw_pagetable_enforce_cc(hwpt);
		if (WARN_ON(rc))
			goto out_abort;
	}

	/*
	 * immediate_attach exists only to accommodate iommu drivers that cannot
	 * directly allocate a domain. These drivers do not finish creating the
	 * domain until attach is completed. Thus we must have this call
	 * sequence. Once those drivers are fixed this should be removed.
	 */
	if (immediate_attach) {
		rc = iommufd_hw_pagetable_attach(hwpt, idev);
		if (rc)
			goto out_abort;
	}

	rc = iommufd_hw_pagetable_link_ioas(hwpt);
	if (rc)
		goto out_detach;
	return hwpt;

out_detach:
	if (immediate_attach)
		iommufd_hw_pagetable_detach(idev);
out_abort:
	iommufd_object_abort_and_destroy(ictx, &hwpt->obj);
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

int iommufd_hwpt_alloc(struct iommufd_ucmd *ucmd)
{
	struct iommufd_hw_pagetable *hwpt, *parent = NULL;
	struct iommu_hwpt_alloc *cmd = ucmd->cmd;
	struct iommufd_object *pt_obj;
	const struct iommu_ops *ops;
	struct iommufd_device *idev;
	struct iommufd_ioas *ioas;
	void *data = NULL;
	u32 klen = 0;
	int rc = 0;

	if (cmd->flags || cmd->__reserved)
		return -EOPNOTSUPP;

	idev = iommufd_get_device(ucmd, cmd->dev_id);
	if (IS_ERR(idev))
		return PTR_ERR(idev);

	ops = dev_iommu_ops(idev->dev);

	/*
	 * All drivers support IOMMU_HWPT_TYPE_DEFAULT, so pass it through.
	 * For any other cmd->hwpt_type, check the ops->hwpt_type_bitmap and
	 * the ops->domain_alloc_user_data_len array.
	 */
	if (cmd->hwpt_type != IOMMU_HWPT_TYPE_DEFAULT) {
		if (!(BIT_ULL(cmd->hwpt_type) & ops->hwpt_type_bitmap)) {
			rc = -EINVAL;
			goto out_put_idev;
		}
		if (!ops->domain_alloc_user_data_len) {
			rc = -EOPNOTSUPP;
			goto out_put_idev;
		}
	}

	pt_obj = iommufd_get_object(ucmd->ictx, cmd->pt_id, IOMMUFD_OBJ_ANY);
	if (IS_ERR(pt_obj)) {
		rc = -EINVAL;
		goto out_put_idev;
	}

	switch (pt_obj->type) {
	case IOMMUFD_OBJ_IOAS:
		ioas = container_of(pt_obj, struct iommufd_ioas, obj);
		break;
	case IOMMUFD_OBJ_HW_PAGETABLE:
		/* pt_id points HWPT only when hwpt_type is !IOMMU_HWPT_TYPE_DEFAULT */
		if (cmd->hwpt_type == IOMMU_HWPT_TYPE_DEFAULT) {
			rc = -EINVAL;
			goto out_put_pt;
		}

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
	default:
		rc = -EINVAL;
		goto out_put_pt;
	}

	if (cmd->hwpt_type != IOMMU_HWPT_TYPE_DEFAULT)
		klen = ops->domain_alloc_user_data_len[cmd->hwpt_type];

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
	hwpt = iommufd_hw_pagetable_alloc(ucmd->ictx, ioas, idev,
					  parent, data, false);
	if (IS_ERR(hwpt)) {
		rc = PTR_ERR(hwpt);
		goto out_unlock;
	}

	/* Hack: Register device fault handler if userspace sets eventfd */
	if (cmd->eventfd >= 0) {
		rc = iommufd_hw_pagetable_dma_fault_init(hwpt, cmd->eventfd);
		if (rc)
			goto out_hwpt;

		cmd->out_fault_fd = hwpt->s1_fault_data.fault_fd;

		/*
		 * FixMe: This is a hack to unregister the existing fault handler if any.
		 * We need to do this on VM exit/reboot path ideally.
		 */
		iommu_unregister_device_fault_handler(idev->dev);

		rc = iommu_register_device_fault_handler(idev->dev,
				iommufd_hw_pagetable_dev_fault_handler, hwpt);
		if (rc)
			goto out_hwpt;
	}

	cmd->out_hwpt_id = hwpt->obj.id;
	rc = iommufd_ucmd_respond(ucmd, sizeof(*cmd));
	if (rc)
		goto out_hwpt;
	iommufd_object_finalize(ucmd->ictx, &hwpt->obj);
	goto out_unlock;

out_hwpt:
	iommufd_object_abort_and_destroy(ucmd->ictx, &hwpt->obj);
out_unlock:
	mutex_unlock(&ioas->mutex);
out_free_data:
	kfree(data);
out_put_pt:
	iommufd_put_object(pt_obj);
out_put_idev:
	iommufd_put_object(&idev->obj);
	return rc;
}

int iommufd_hwpt_invalidate(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_invalidate *cmd = ucmd->cmd;
	struct iommufd_hw_pagetable *hwpt;
	u32 user_data_len, klen;
	u64 user_ptr;
	int rc = 0;

	if (!cmd->data_len || cmd->__reserved)
		return -EOPNOTSUPP;

	hwpt = iommufd_get_hwpt(ucmd, cmd->hwpt_id);
	if (IS_ERR(hwpt))
		return PTR_ERR(hwpt);

	/* Do not allow any kernel-managed hw_pagetable */
	if (!hwpt->parent) {
		rc = -EINVAL;
		goto out_put_hwpt;
	}

	klen = hwpt->domain->ops->cache_invalidate_user_data_len;
	if (!hwpt->domain->ops->cache_invalidate_user || !klen) {
		rc = -EOPNOTSUPP;
		goto out_put_hwpt;
	}

	/*
	 * Copy the needed fields before reusing the ucmd buffer, this
	 * avoids memory allocation in this path.
	 */
	user_ptr = cmd->data_uptr;
	user_data_len = cmd->data_len;

	rc = copy_struct_from_user(cmd, klen,
				   u64_to_user_ptr(user_ptr), user_data_len);
	if (rc)
		goto out_put_hwpt;

	rc = hwpt->domain->ops->cache_invalidate_user(hwpt->domain, cmd);
out_put_hwpt:
	iommufd_put_object(&hwpt->obj);
	return rc;
}

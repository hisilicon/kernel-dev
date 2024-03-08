// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (C) 2024 Intel Corporation
 */
#define pr_fmt(fmt) "iommufd: " fmt

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/iommufd.h>
#include <linux/poll.h>
#include <linux/anon_inodes.h>
#include <uapi/linux/iommufd.h>

#include "iommufd_private.h"

static int device_add_fault(struct iopf_group *group)
{
	struct iommufd_device *idev = group->cookie->private;
	void *curr;

	curr = xa_cmpxchg(&idev->faults, group->last_fault.fault.prm.grpid,
			  NULL, group, GFP_KERNEL);

	return curr ? xa_err(curr) : 0;
}

static void device_remove_fault(struct iopf_group *group)
{
	struct iommufd_device *idev = group->cookie->private;

	xa_store(&idev->faults, group->last_fault.fault.prm.grpid,
		 NULL, GFP_KERNEL);
}

static struct iopf_group *device_get_fault(struct iommufd_device *idev,
					   unsigned long grpid)
{
	return xa_load(&idev->faults, grpid);
}

void iommufd_fault_destroy(struct iommufd_object *obj)
{
	struct iommufd_fault *fault = container_of(obj, struct iommufd_fault, obj);
	struct iopf_group *group, *next;

	mutex_lock(&fault->mutex);
	list_for_each_entry_safe(group, next, &fault->deliver, node) {
		list_del(&group->node);
		iopf_group_response(group, IOMMU_PAGE_RESP_INVALID);
		iopf_free_group(group);
	}
	list_for_each_entry_safe(group, next, &fault->response, node) {
		list_del(&group->node);
		device_remove_fault(group);
		iopf_group_response(group, IOMMU_PAGE_RESP_INVALID);
		iopf_free_group(group);
	}
	mutex_unlock(&fault->mutex);

	mutex_destroy(&fault->mutex);
}

static void iommufd_compose_fault_message(struct iommu_fault *fault,
					  struct iommu_hwpt_pgfault *hwpt_fault,
					  struct iommufd_device *idev)
{
	hwpt_fault->size = sizeof(*hwpt_fault);
	hwpt_fault->flags = fault->prm.flags;
	hwpt_fault->dev_id = idev->obj.id;
	hwpt_fault->pasid = fault->prm.pasid;
	hwpt_fault->grpid = fault->prm.grpid;
	hwpt_fault->perm = fault->prm.perm;
	hwpt_fault->addr = fault->prm.addr;
}

static ssize_t iommufd_fault_fops_read(struct file *filep, char __user *buf,
				       size_t count, loff_t *ppos)
{
	size_t fault_size = sizeof(struct iommu_hwpt_pgfault);
	struct iommufd_fault *fault = filep->private_data;
	struct iommu_hwpt_pgfault data;
	struct iommufd_device *idev;
	struct iopf_group *group;
	struct iopf_fault *iopf;
	size_t done = 0;
	int rc;

	if (*ppos || count % fault_size)
		return -ESPIPE;

	mutex_lock(&fault->mutex);
	while (!list_empty(&fault->deliver) && count > done) {
		group = list_first_entry(&fault->deliver,
					 struct iopf_group, node);

		if (list_count_nodes(&group->faults) * fault_size > count - done)
			break;

		idev = (struct iommufd_device *)group->cookie->private;
		list_for_each_entry(iopf, &group->faults, list) {
			iommufd_compose_fault_message(&iopf->fault, &data, idev);
			rc = copy_to_user(buf + done, &data, fault_size);
			if (rc)
				goto err_unlock;
			done += fault_size;
		}

		rc = device_add_fault(group);
		if (rc)
			goto err_unlock;

		list_move_tail(&group->node, &fault->response);
	}
	mutex_unlock(&fault->mutex);

	return done;
err_unlock:
	mutex_unlock(&fault->mutex);
	return rc;
}

static ssize_t iommufd_fault_fops_write(struct file *filep, const char __user *buf,
					size_t count, loff_t *ppos)
{
	size_t response_size = sizeof(struct iommu_hwpt_page_response);
	struct iommufd_fault *fault = filep->private_data;
	struct iommu_hwpt_page_response response;
	struct iommufd_device *idev;
	struct iopf_group *group;
	size_t done = 0;
	int rc;

	if (*ppos || count % response_size)
		return -ESPIPE;

	while (!list_empty(&fault->response) && count > done) {
		rc = copy_from_user(&response, buf + done, response_size);
		if (rc)
			break;

		idev = container_of(iommufd_get_object(fault->ictx,
						       response.dev_id,
						       IOMMUFD_OBJ_DEVICE),
				    struct iommufd_device, obj);
		if (IS_ERR(idev))
			break;

		group = device_get_fault(idev, response.grpid);
		if (!group ||
		    response.addr != group->last_fault.fault.prm.addr ||
		    ((group->last_fault.fault.prm.flags & IOMMU_FAULT_PAGE_REQUEST_PASID_VALID) &&
		      response.pasid != group->last_fault.fault.prm.pasid)) {
			iommufd_put_object(fault->ictx, &idev->obj);
			break;
		}

		iopf_group_response(group, response.code);

		mutex_lock(&fault->mutex);
		list_del(&group->node);
		mutex_unlock(&fault->mutex);

		device_remove_fault(group);
		iopf_free_group(group);
		done += response_size;

		iommufd_put_object(fault->ictx, &idev->obj);
	}

	return done;
}

static __poll_t iommufd_fault_fops_poll(struct file *filep,
					struct poll_table_struct *wait)
{
	struct iommufd_fault *fault = filep->private_data;
	__poll_t pollflags = 0;

	poll_wait(filep, &fault->wait_queue, wait);
	mutex_lock(&fault->mutex);
	if (!list_empty(&fault->deliver))
		pollflags = EPOLLIN | EPOLLRDNORM;
	mutex_unlock(&fault->mutex);

	return pollflags;
}

static const struct file_operations iommufd_fault_fops = {
	.owner		= THIS_MODULE,
	.open		= nonseekable_open,
	.read		= iommufd_fault_fops_read,
	.write		= iommufd_fault_fops_write,
	.poll		= iommufd_fault_fops_poll,
	.llseek		= no_llseek,
};

static int get_fault_fd(struct iommufd_fault *fault)
{
	struct file *filep;
	int fdno;

	fdno = get_unused_fd_flags(O_CLOEXEC);
	if (fdno < 0)
		return fdno;

	filep = anon_inode_getfile("[iommufd-pgfault]", &iommufd_fault_fops,
				   fault, O_RDWR);
	if (IS_ERR(filep)) {
		put_unused_fd(fdno);
		return PTR_ERR(filep);
	}

	fd_install(fdno, filep);

	return fdno;
}

int iommufd_fault_alloc(struct iommufd_ucmd *ucmd)
{
	struct iommu_fault_alloc *cmd = ucmd->cmd;
	struct iommufd_fault *fault;
	int rc;

	if (cmd->flags)
		return -EOPNOTSUPP;

	fault = iommufd_object_alloc(ucmd->ictx, fault, IOMMUFD_OBJ_FAULT);
	if (IS_ERR(fault))
		return PTR_ERR(fault);

	fault->ictx = ucmd->ictx;
	INIT_LIST_HEAD(&fault->deliver);
	INIT_LIST_HEAD(&fault->response);
	mutex_init(&fault->mutex);
	init_waitqueue_head(&fault->wait_queue);

	rc = get_fault_fd(fault);
	if (rc < 0)
		goto out_abort;

	cmd->out_fault_id = fault->obj.id;
	cmd->out_fault_fd = rc;

	rc = iommufd_ucmd_respond(ucmd, sizeof(*cmd));
	if (rc)
		goto out_abort;
	iommufd_object_finalize(ucmd->ictx, &fault->obj);

	return 0;
out_abort:
	iommufd_object_abort_and_destroy(ucmd->ictx, &fault->obj);

	return rc;
}

int iommufd_fault_iopf_handler(struct iopf_group *group)
{
	struct iommufd_hw_pagetable *hwpt = group->cookie->domain->fault_data;
	struct iommufd_fault *fault = hwpt->fault;

	mutex_lock(&fault->mutex);
	list_add_tail(&group->node, &fault->deliver);
	mutex_unlock(&fault->mutex);

	wake_up_interruptible(&fault->wait_queue);

	return 0;
}

static void release_attach_cookie(struct iopf_attach_cookie *cookie)
{
	struct iommufd_hw_pagetable *hwpt = cookie->domain->fault_data;
	struct iommufd_device *idev = cookie->private;

	refcount_dec(&idev->obj.users);
	refcount_dec(&hwpt->obj.users);
	kfree(cookie);
}

static int iommufd_fault_iopf_enable(struct iommufd_device *idev)
{
	int ret;

	if (idev->iopf_enabled)
		return 0;

	ret = iommu_dev_enable_feature(idev->dev, IOMMU_DEV_FEAT_IOPF);
	if (ret)
		return ret;

	idev->iopf_enabled = true;

	return 0;
}

static void iommufd_fault_iopf_disable(struct iommufd_device *idev)
{
	if (!idev->iopf_enabled)
		return;

	iommu_dev_disable_feature(idev->dev, IOMMU_DEV_FEAT_IOPF);
	idev->iopf_enabled = false;
}

int iommufd_fault_domain_attach_dev(struct iommufd_hw_pagetable *hwpt,
				    struct iommufd_device *idev)
{
	struct iopf_attach_cookie *cookie;
	int ret;

	cookie = kzalloc(sizeof(*cookie), GFP_KERNEL);
	if (!cookie)
		return -ENOMEM;

	refcount_inc(&hwpt->obj.users);
	refcount_inc(&idev->obj.users);
	cookie->release = release_attach_cookie;
	cookie->private = idev;

	if (!idev->iopf_enabled) {
		ret = iommufd_fault_iopf_enable(idev);
		if (ret)
			goto out_put_cookie;
	}

	ret = iopf_domain_attach(hwpt->domain, idev->dev, IOMMU_NO_PASID, cookie);
	if (ret)
		goto out_disable_iopf;

	return 0;
out_disable_iopf:
	iommufd_fault_iopf_disable(idev);
out_put_cookie:
	release_attach_cookie(cookie);

	return ret;
}

void iommufd_fault_domain_detach_dev(struct iommufd_hw_pagetable *hwpt,
				     struct iommufd_device *idev)
{
	iopf_domain_detach(hwpt->domain, idev->dev, IOMMU_NO_PASID);
	iommufd_fault_iopf_disable(idev);
}

int iommufd_fault_domain_replace_dev(struct iommufd_hw_pagetable *hwpt,
				     struct iommufd_device *idev)
{
	bool iopf_enabled_originally = idev->iopf_enabled;
	struct iopf_attach_cookie *cookie = NULL;
	int ret;

	if (hwpt->fault_capable) {
		cookie = kzalloc(sizeof(*cookie), GFP_KERNEL);
		if (!cookie)
			return -ENOMEM;

		refcount_inc(&hwpt->obj.users);
		refcount_inc(&idev->obj.users);
		cookie->release = release_attach_cookie;
		cookie->private = idev;

		if (!idev->iopf_enabled) {
			ret = iommufd_fault_iopf_enable(idev);
			if (ret) {
				release_attach_cookie(cookie);
				return ret;
			}
		}
	}

	ret = iopf_domain_replace(hwpt->domain, idev->dev, IOMMU_NO_PASID, cookie);
	if (ret) {
		goto out_put_cookie;
	}

	if (iopf_enabled_originally && !hwpt->fault_capable)
		iommufd_fault_iopf_disable(idev);

	return 0;
out_put_cookie:
	if (hwpt->fault_capable)
		release_attach_cookie(cookie);
	if (iopf_enabled_originally && !idev->iopf_enabled)
		iommufd_fault_iopf_enable(idev);
	else if (!iopf_enabled_originally && idev->iopf_enabled)
		iommufd_fault_iopf_disable(idev);

	return ret;
}

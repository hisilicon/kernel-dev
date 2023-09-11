// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#include <linux/iommu.h>
#include <linux/file.h>
#include <linux/anon_inodes.h>
#include <uapi/linux/iommufd.h>

#include "../iommu-priv.h"
#include "iommufd_private.h"

static int iommufd_compose_fault_message(struct iommu_fault *fault,
					 struct iommu_hwpt_pgfault *hwpt_fault,
					 struct device *dev)
{
	struct iommufd_device *idev = iopf_pasid_cookie_get(dev, IOMMU_NO_PASID);

	if (!idev)
		return -ENODEV;

	if (IS_ERR(idev))
		return PTR_ERR(idev);

	hwpt_fault->size = sizeof(*hwpt_fault);
	hwpt_fault->flags = fault->prm.flags;
	hwpt_fault->dev_id = idev->obj.id;
	hwpt_fault->pasid = fault->prm.pasid;
	hwpt_fault->grpid = fault->prm.grpid;
	hwpt_fault->perm = fault->prm.perm;
	hwpt_fault->addr = fault->prm.addr;
	hwpt_fault->private_data[0] = fault->prm.private_data[0];
	hwpt_fault->private_data[1] = fault->prm.private_data[1];

	return 0;
}

static ssize_t hwpt_fault_fops_read(struct file *filep, char __user *buf,
				    size_t count, loff_t *ppos)
{
	size_t fault_size = sizeof(struct iommu_hwpt_pgfault);
	struct hw_pgtable_fault *fault = filep->private_data;
	struct iommu_hwpt_pgfault data;
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

		list_for_each_entry(iopf, &group->faults, list) {
			rc = iommufd_compose_fault_message(&iopf->fault,
							   &data, group->dev);
			if (rc)
				goto err_unlock;
			rc = copy_to_user(buf + done, &data, fault_size);
			if (rc)
				goto err_unlock;
			done += fault_size;
		}

		list_move_tail(&group->node, &fault->response);
	}
	mutex_unlock(&fault->mutex);

	return done;
err_unlock:
	mutex_unlock(&fault->mutex);
	return rc;
}

static ssize_t hwpt_fault_fops_write(struct file *filep,
				     const char __user *buf,
				     size_t count, loff_t *ppos)
{
	size_t response_size = sizeof(struct iommu_hwpt_page_response);
	struct hw_pgtable_fault *fault = filep->private_data;
	struct iommu_hwpt_page_response response;
	struct iommufd_hw_pagetable *hwpt;
	struct iopf_group *iter, *group;
	struct iommufd_device *idev;
	size_t done = 0;
	int rc = 0;

	if (*ppos || count % response_size)
		return -ESPIPE;

	mutex_lock(&fault->mutex);
	while (!list_empty(&fault->response) && count > done) {
		rc = copy_from_user(&response, buf + done, response_size);
		if (rc)
			break;

		/* Get the device that this response targets at. */
		idev = container_of(iommufd_get_object(fault->ictx,
						       response.dev_id,
						       IOMMUFD_OBJ_DEVICE),
				    struct iommufd_device, obj);
		if (IS_ERR(idev)) {
			rc = PTR_ERR(idev);
			break;
		}

		/*
		 * Get the hw page table that this response was generated for.
		 * It must match the one stored in the fault data.
		 */
		hwpt = container_of(iommufd_get_object(fault->ictx,
						       response.hwpt_id,
						       IOMMUFD_OBJ_ANY),
				    struct iommufd_hw_pagetable, obj);
		if (IS_ERR(hwpt)) {
			iommufd_put_object(&idev->obj);
			rc = PTR_ERR(hwpt);
			break;
		}

		if (hwpt != fault->hwpt) {
			rc = -EINVAL;
			goto put_obj;
		}

		group = NULL;
		list_for_each_entry(iter, &fault->response, node) {
			if (response.grpid != iter->last_fault.fault.prm.grpid)
				continue;

			if (idev->dev != iter->dev)
				continue;

			group = iter;
			break;
		}

		if (!group) {
			rc = -ENODEV;
			goto put_obj;
		}

		rc = iopf_group_response(group, response.code);
		if (rc)
			goto put_obj;

		list_del(&group->node);
		iopf_free_group(group);
		done += response_size;
put_obj:
		iommufd_put_object(&hwpt->obj);
		iommufd_put_object(&idev->obj);
		if (rc)
			break;
	}
	mutex_unlock(&fault->mutex);

	return (rc < 0) ? rc : done;
}

static const struct file_operations hwpt_fault_fops = {
	.owner		= THIS_MODULE,
	.read		= hwpt_fault_fops_read,
	.write		= hwpt_fault_fops_write,
};

static int hw_pagetable_get_fault_fd(struct hw_pgtable_fault *fault)
{
	struct file *filep;
	int fdno;

	fdno = get_unused_fd_flags(O_CLOEXEC);
	if (fdno < 0)
		return fdno;

	filep = anon_inode_getfile("[iommufd-pgfault]", &hwpt_fault_fops,
				   fault, O_RDWR);
	if (IS_ERR(filep)) {
		put_unused_fd(fdno);
		return PTR_ERR(filep);
	}

	fd_install(fdno, filep);
	fault->fault_file = filep;
	fault->fault_fd = fdno;

	return 0;
}

static struct hw_pgtable_fault *hw_pagetable_fault_alloc(void)
{
	struct hw_pgtable_fault *fault;
	int rc;

	fault = kzalloc(sizeof(*fault), GFP_KERNEL);
	if (!fault)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&fault->deliver);
	INIT_LIST_HEAD(&fault->response);
	mutex_init(&fault->mutex);

	rc = hw_pagetable_get_fault_fd(fault);
	if (rc) {
		kfree(fault);
		return ERR_PTR(rc);
	}

	return fault;
}

static void hw_pagetable_fault_free(struct hw_pgtable_fault *fault)
{
	WARN_ON(!list_empty(&fault->deliver));
	WARN_ON(!list_empty(&fault->response));

	fput(fault->fault_file);
	kfree(fault);
}

void iommufd_hwpt_paging_destroy(struct iommufd_object *obj)
{
	struct iommufd_hwpt_paging *hwpt_paging =
		container_of(obj, struct iommufd_hwpt_paging, common.obj);

	if (!list_empty(&hwpt_paging->hwpt_item)) {
		mutex_lock(&hwpt_paging->ioas->mutex);
		list_del(&hwpt_paging->hwpt_item);
		mutex_unlock(&hwpt_paging->ioas->mutex);

		iopt_table_remove_domain(&hwpt_paging->ioas->iopt,
					 hwpt_paging->common.domain);
	}

	if (hwpt_paging->common.domain)
		iommu_domain_free(hwpt_paging->common.domain);

	if (hwpt_paging->common.fault) {
		hw_pagetable_fault_free(hwpt_paging->common.fault);
		hwpt_paging->common.fault = NULL;
	}

	refcount_dec(&hwpt_paging->ioas->obj.users);
}

void iommufd_hwpt_paging_abort(struct iommufd_object *obj)
{
	struct iommufd_hwpt_paging *hwpt_paging =
		container_of(obj, struct iommufd_hwpt_paging, common.obj);

	/* The ioas->mutex must be held until finalize is called. */
	lockdep_assert_held(&hwpt_paging->ioas->mutex);

	if (!list_empty(&hwpt_paging->hwpt_item)) {
		list_del_init(&hwpt_paging->hwpt_item);
		iopt_table_remove_domain(&hwpt_paging->ioas->iopt,
					 hwpt_paging->common.domain);
	}
	iommufd_hwpt_paging_destroy(obj);
}

void iommufd_hwpt_nested_destroy(struct iommufd_object *obj)
{
	struct iommufd_hwpt_nested *hwpt_nested =
		container_of(obj, struct iommufd_hwpt_nested, common.obj);

	if (hwpt_nested->common.domain)
		iommu_domain_free(hwpt_nested->common.domain);

	if (hwpt_nested->common.fault) {
		hw_pagetable_fault_free(hwpt_nested->common.fault);
		hwpt_nested->common.fault = NULL;
	}

	refcount_dec(&hwpt_nested->parent->common.obj.users);
}

void iommufd_hwpt_nested_abort(struct iommufd_object *obj)
{
	iommufd_hwpt_nested_destroy(obj);
}

static int
iommufd_hwpt_paging_enforce_cc(struct iommufd_hwpt_paging *hwpt_paging)
{
	struct iommu_domain *paging_domain = hwpt_paging->common.domain;

	if (hwpt_paging->enforce_cache_coherency)
		return 0;

	if (paging_domain->ops->enforce_cache_coherency)
		hwpt_paging->enforce_cache_coherency =
			paging_domain->ops->enforce_cache_coherency(
				paging_domain);
	if (!hwpt_paging->enforce_cache_coherency)
		return -EINVAL;
	return 0;
}

/**
 * iommufd_hwpt_paging_alloc() - Get a PAGING iommu_domain for a device
 * @ictx: iommufd context
 * @ioas: IOAS to associate the domain with
 * @idev: Device to get an iommu_domain for
 * @flags: Flags from userspace
 * @immediate_attach: True if idev should be attached to the hwpt
 * @user_data: The user provided driver specific data describing the domain to
 *             create
 *
 * Allocate a new iommu_domain and return it as a hw_pagetable. The HWPT
 * will be linked to the given ioas and upon return the underlying iommu_domain
 * is fully popoulated.
 *
 * The caller must hold the ioas->mutex until after
 * iommufd_object_abort_and_destroy() or iommufd_object_finalize() is called on
 * the returned hwpt.
 */
struct iommufd_hwpt_paging *
iommufd_hwpt_paging_alloc(struct iommufd_ctx *ictx, struct iommufd_ioas *ioas,
			  struct iommufd_device *idev, u32 flags,
			  bool immediate_attach,
			  const struct iommu_user_data *user_data)
{
	const u32 valid_flags = IOMMU_HWPT_ALLOC_NEST_PARENT |
				IOMMU_HWPT_ALLOC_DIRTY_TRACKING;
	const struct iommu_ops *ops = dev_iommu_ops(idev->dev);
	struct iommufd_hwpt_paging *hwpt_paging;
	struct iommufd_hw_pagetable *hwpt;
	int rc;

	lockdep_assert_held(&ioas->mutex);

	if ((flags || user_data) && !ops->domain_alloc_user)
		return ERR_PTR(-EOPNOTSUPP);
	if (flags & ~valid_flags)
		return ERR_PTR(-EOPNOTSUPP);

	hwpt_paging = __iommufd_object_alloc(
		ictx, hwpt_paging, IOMMUFD_OBJ_HWPT_PAGING, common.obj);
	if (IS_ERR(hwpt_paging))
		return ERR_CAST(hwpt_paging);
	hwpt = &hwpt_paging->common;

	INIT_LIST_HEAD(&hwpt_paging->hwpt_item);
	/* Pairs with iommufd_hw_pagetable_destroy() */
	refcount_inc(&ioas->obj.users);
	hwpt_paging->ioas = ioas;
	hwpt_paging->nest_parent = flags & IOMMU_HWPT_ALLOC_NEST_PARENT;

	if (ops->domain_alloc_user) {
		hwpt->domain = ops->domain_alloc_user(idev->dev, flags, NULL,
						      ictx->kvm, user_data);
		if (IS_ERR(hwpt->domain)) {
			rc = PTR_ERR(hwpt->domain);
			hwpt->domain = NULL;
			goto out_abort;
		}
		hwpt->domain->owner = ops;
	} else {
		hwpt->domain = iommu_domain_alloc(idev->dev->bus);
		if (!hwpt->domain) {
			rc = -ENOMEM;
			goto out_abort;
		}
	}

	/*
	 * Set the coherency mode before we do iopt_table_add_domain() as some
	 * iommus have a per-PTE bit that controls it and need to decide before
	 * doing any maps. It is an iommu driver bug to report
	 * IOMMU_CAP_ENFORCE_CACHE_COHERENCY but fail enforce_cache_coherency on
	 * a new domain.
	 *
	 * The cache coherency mode must be configured here and unchanged later.
	 * Note that a HWPT (non-CC) created for a device (non-CC) can be later
	 * reused by another device (either non-CC or CC). However, A HWPT (CC)
	 * created for a device (CC) cannot be reused by another device (non-CC)
	 * but only devices (CC). Instead user space in this case would need to
	 * allocate a separate HWPT (non-CC).
	 */
	if (idev->enforce_cache_coherency) {
		rc = iommufd_hwpt_paging_enforce_cc(hwpt_paging);
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

	rc = iopt_table_add_domain(&ioas->iopt, hwpt->domain);
	if (rc)
		goto out_detach;
	list_add_tail(&hwpt_paging->hwpt_item, &ioas->hwpt_list);
	return hwpt_paging;

out_detach:
	if (immediate_attach)
		iommufd_hw_pagetable_detach(idev);
out_abort:
	iommufd_object_abort_and_destroy(ictx, &hwpt->obj);
	return ERR_PTR(rc);
}

static int iommufd_hw_pagetable_iopf_handler(struct iopf_group *group)
{
	struct iommufd_hw_pagetable *hwpt = group->domain->fault_data;

	mutex_lock(&hwpt->fault->mutex);
	list_add_tail(&group->node, &hwpt->fault->deliver);
	mutex_unlock(&hwpt->fault->mutex);

	return 0;
}

/**
 * iommufd_hwpt_nested_alloc() - Get a NESTED iommu_domain for a device
 * @ictx: iommufd context
 * @parent: Parent PAGING-type hwpt to associate the domain with
 * @idev: Device to get an iommu_domain for
 * @flags: Flags from userspace
 * @user_data: user_data pointer. Must be valid
 *
 * Allocate a new iommu_domain (must be IOMMU_DOMAIN_NESTED) and return it as
 * a NESTED hw_pagetable. The given parent PAGING-type hwpt must be capable of
 * being a parent.
 */
static struct iommufd_hwpt_nested *
iommufd_hwpt_nested_alloc(struct iommufd_ctx *ictx,
			  struct iommufd_hwpt_paging *parent,
			  struct iommufd_device *idev, u32 flags,
			  const struct iommu_user_data *user_data)
{
	const u32 valid_flags = IOMMU_HWPT_ALLOC_IOPF_CAPABLE;
	const struct iommu_ops *ops = dev_iommu_ops(idev->dev);
	struct iommufd_hwpt_nested *hwpt_nested;
	struct iommufd_hw_pagetable *hwpt;
	int rc;

	if (!user_data->len || !ops->domain_alloc_user)
		return ERR_PTR(-EOPNOTSUPP);
	if (flags & ~valid_flags)
		return ERR_PTR(-EOPNOTSUPP);

	if (parent->auto_domain || !parent->nest_parent)
		return ERR_PTR(-EINVAL);

	hwpt_nested = __iommufd_object_alloc(
		ictx, hwpt_nested, IOMMUFD_OBJ_HWPT_NESTED, common.obj);
	if (IS_ERR(hwpt_nested))
		return ERR_CAST(hwpt_nested);
	hwpt = &hwpt_nested->common;

	refcount_inc(&parent->common.obj.users);
	hwpt_nested->parent = parent;

	hwpt->domain = ops->domain_alloc_user(idev->dev, flags,
					      parent->common.domain,
					      ictx->kvm, user_data);
	if (IS_ERR(hwpt->domain)) {
		rc = PTR_ERR(hwpt->domain);
		hwpt->domain = NULL;
		goto out_abort;
	}
	hwpt->domain->owner = ops;

	if (WARN_ON_ONCE(hwpt->domain->type != IOMMU_DOMAIN_NESTED)) {
		rc = -EINVAL;
		goto out_abort;
	}
	/* Driver is buggy by missing cache_invalidate_user in domain_ops */
	if (WARN_ON_ONCE(!hwpt->domain->ops->cache_invalidate_user)) {
		rc = -EINVAL;
		goto out_abort;
	}
	return hwpt_nested;

out_abort:
	iommufd_object_abort_and_destroy(ictx, &hwpt->obj);
	return ERR_PTR(rc);
}

int iommufd_hwpt_alloc(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_alloc *cmd = ucmd->cmd;
	const struct iommu_user_data user_data = {
		.type = cmd->data_type,
		.uptr = u64_to_user_ptr(cmd->data_uptr),
		.len = cmd->data_len,
	};
	struct iommufd_hw_pagetable *hwpt;
	struct iommufd_ioas *ioas = NULL;
	struct iommufd_object *pt_obj;
	struct iommufd_device *idev;
	int rc;

	if (cmd->__reserved)
		return -EOPNOTSUPP;
	if (cmd->data_type == IOMMU_HWPT_DATA_NONE && cmd->data_len)
		return -EINVAL;

	idev = iommufd_get_device(ucmd, cmd->dev_id);
	if (IS_ERR(idev))
		return PTR_ERR(idev);

	pt_obj = iommufd_get_object(ucmd->ictx, cmd->pt_id, IOMMUFD_OBJ_ANY);
	if (IS_ERR(pt_obj)) {
		rc = -EINVAL;
		goto out_put_idev;
	}

	if (pt_obj->type == IOMMUFD_OBJ_IOAS) {
		struct iommufd_hwpt_paging *hwpt_paging;

		ioas = container_of(pt_obj, struct iommufd_ioas, obj);
		mutex_lock(&ioas->mutex);
		hwpt_paging = iommufd_hwpt_paging_alloc(
			ucmd->ictx, ioas, idev, cmd->flags, false,
			user_data.len ? &user_data : NULL);
		if (IS_ERR(hwpt_paging)) {
			rc = PTR_ERR(hwpt_paging);
			goto out_unlock;
		}
		hwpt = &hwpt_paging->common;
	} else if (pt_obj->type == IOMMUFD_OBJ_HWPT_PAGING) {
		struct iommufd_hwpt_nested *hwpt_nested;

		hwpt_nested = iommufd_hwpt_nested_alloc(
			ucmd->ictx,
			container_of(pt_obj, struct iommufd_hwpt_paging,
				     common.obj),
			idev, cmd->flags, &user_data);
		if (IS_ERR(hwpt_nested)) {
			rc = PTR_ERR(hwpt_nested);
			goto out_unlock;
		}
		hwpt = &hwpt_nested->common;
	} else {
		rc = -EINVAL;
		goto out_put_pt;
	}

	if (cmd->flags & IOMMU_HWPT_ALLOC_IOPF_CAPABLE) {
		hwpt->fault = hw_pagetable_fault_alloc();
		if (IS_ERR(hwpt->fault)) {
			rc = PTR_ERR(hwpt->fault);
			hwpt->fault = NULL;
			goto out_hwpt;
		}

		hwpt->fault->ictx = ucmd->ictx;
		hwpt->fault->hwpt = hwpt;
		hwpt->domain->iopf_handler = iommufd_hw_pagetable_iopf_handler;
		hwpt->domain->fault_data = hwpt;
		cmd->out_fault_fd = hwpt->fault->fault_fd;
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
	if (ioas)
		mutex_unlock(&ioas->mutex);
out_put_pt:
	iommufd_put_object(pt_obj);
out_put_idev:
	iommufd_put_object(&idev->obj);
	return rc;
}

int iommufd_hwpt_set_dirty_tracking(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_set_dirty_tracking *cmd = ucmd->cmd;
	struct iommufd_hwpt_paging *hwpt_paging;
	struct iommufd_ioas *ioas;
	int rc = -EOPNOTSUPP;
	bool enable;

	if (cmd->flags & ~IOMMU_HWPT_DIRTY_TRACKING_ENABLE)
		return rc;

	hwpt_paging = iommufd_get_hwpt_paging(ucmd, cmd->hwpt_id);
	if (IS_ERR(hwpt_paging))
		return PTR_ERR(hwpt_paging);

	ioas = hwpt_paging->ioas;
	enable = cmd->flags & IOMMU_HWPT_DIRTY_TRACKING_ENABLE;

	rc = iopt_set_dirty_tracking(&ioas->iopt, hwpt_paging->common.domain,
				     enable);

	iommufd_put_object(&hwpt_paging->common.obj);
	return rc;
}

int iommufd_hwpt_get_dirty_bitmap(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_get_dirty_bitmap *cmd = ucmd->cmd;
	struct iommufd_hwpt_paging *hwpt_paging;
	struct iommufd_ioas *ioas;
	int rc = -EOPNOTSUPP;

	if ((cmd->flags & ~(IOMMU_HWPT_GET_DIRTY_BITMAP_NO_CLEAR)) ||
	    cmd->__reserved)
		return -EOPNOTSUPP;

	hwpt_paging = iommufd_get_hwpt_paging(ucmd, cmd->hwpt_id);
	if (IS_ERR(hwpt_paging))
		return PTR_ERR(hwpt_paging);

	ioas = hwpt_paging->ioas;
	rc = iopt_read_and_clear_dirty_data(
		&ioas->iopt, hwpt_paging->common.domain, cmd->flags, cmd);

	iommufd_put_object(&hwpt_paging->common.obj);
	return rc;
};

int iommufd_hwpt_invalidate(struct iommufd_ucmd *ucmd)
{
	struct iommu_hwpt_invalidate *cmd = ucmd->cmd;
	struct iommu_user_data_array data_array = {
		.type = cmd->req_type,
		.uptr = u64_to_user_ptr(cmd->reqs_uptr),
		.entry_len = cmd->req_len,
		.entry_num = cmd->req_num,
	};
	struct iommufd_hw_pagetable *hwpt;
	int rc = 0;

	if (cmd->req_type == IOMMU_HWPT_DATA_NONE)
		return -EINVAL;
	if (!cmd->reqs_uptr || !cmd->req_len || !cmd->req_num)
		return -EINVAL;

	hwpt = iommufd_hw_pagetable_get_nested(ucmd, cmd->hwpt_id);
	if (IS_ERR(hwpt))
		return PTR_ERR(hwpt);

	rc = hwpt->domain->ops->cache_invalidate_user(hwpt->domain, &data_array,
						      &cmd->out_driver_error_code);
	cmd->req_num = data_array.entry_num;
	if (iommufd_ucmd_respond(ucmd, sizeof(*cmd)))
		return -EFAULT;
	iommufd_put_object(&hwpt->obj);
	return rc;
}

// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (C) 2021 Intel Corporation
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 *
 * iommfd provides control over the IOMMU HW objects created by IOMMU kernel
 * drivers. IOMMU HW objects revolve around IO page tables that map incoming DMA
 * addresses (IOVA) to CPU addresses.
 *
 * The API is divided into a general portion that is intended to work with any
 * kernel IOMMU driver, and a device specific portion that  is intended to be
 * used with a userspace HW driver paired with the specific kernel driver. This
 * mechanism allows all the unique functionalities in individual IOMMUs to be
 * exposed to userspace control.
 */
#define pr_fmt(fmt) "iommufd: " fmt

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/mutex.h>
#include <linux/bug.h>
#include <uapi/linux/iommufd.h>

#include "iommufd_private.h"

struct iommufd_object_ops {
	void (*destroy)(struct iommufd_object *obj);
};
static struct iommufd_object_ops iommufd_object_ops[];

struct iommufd_object *_iommufd_object_alloc(struct iommufd_ctx *ictx,
					     size_t size,
					     enum iommufd_object_type type)
{
	struct iommufd_object *obj;
	int rc;

	obj = kzalloc(size, GFP_KERNEL);
	if (!obj)
		return ERR_PTR(-ENOMEM);
	obj->type = type;
	init_rwsem(&obj->destroy_rwsem);
	refcount_set(&obj->users, 1);

	/*
	 * Reserve an ID in the xarray but do not publish the pointer yet since
	 * the caller hasn't initialized it yet. Once the pointer is published
	 * in the xarray and visible to other threads we can't reliably destroy
	 * it anymore, so the caller must complete all errorable operations
	 * before calling iommufd_object_finalize().
	 */
	rc = xa_alloc(&ictx->objects, &obj->id, XA_ZERO_ENTRY,
		      xa_limit_32b, GFP_KERNEL);
	if (rc)
		goto out_free;
	return obj;
out_free:
	kfree(obj);
	return ERR_PTR(rc);
}

/*
 * Allow concurrent access to the object. This should only be done once the
 * system call that created the object is guaranteed to succeed.
 */
void iommufd_object_finalize(struct iommufd_ctx *ictx,
			     struct iommufd_object *obj)
{
	void *old;

	old = xa_store(&ictx->objects, obj->id, obj, GFP_KERNEL);
	/* obj->id was returned from xa_alloc() so the xa_store() cannot fail */
	WARN_ON(old);
}

/* Undo _iommufd_object_alloc() if iommufd_object_finalize() was not called */
void iommufd_object_abort(struct iommufd_ctx *ictx, struct iommufd_object *obj)
{
	void *old;

	old = xa_erase(&ictx->objects, obj->id);
	WARN_ON(old);
	kfree(obj);
}

struct iommufd_object *iommufd_get_object(struct iommufd_ctx *ictx, u32 id,
					  enum iommufd_object_type type)
{
	struct iommufd_object *obj;

	xa_lock(&ictx->objects);
	obj = xa_load(&ictx->objects, id);
	if (!obj || (type != IOMMUFD_OBJ_ANY && obj->type != type) ||
	    !iommufd_lock_obj(obj))
		obj = ERR_PTR(-ENOENT);
	xa_unlock(&ictx->objects);
	return obj;
}

/*
 * The caller holds a users refcount and wants to destroy the object. Returns
 * true if the object was destroyed. In all cases the caller no longer has a
 * reference on obj.
 */
bool iommufd_object_destroy_user(struct iommufd_ctx *ictx,
				 struct iommufd_object *obj)
{
	/*
	 * The purpose of the destroy_rwsem is to ensure deterministic
	 * destruction of objects used by external drivers and destroyed by this
	 * function. Any temporary increment of the refcount must hold the read
	 * side of this, such as during ioctl execution.
	 */
	down_write(&obj->destroy_rwsem);
	xa_lock(&ictx->objects);
	refcount_dec(&obj->users);
	if (!refcount_dec_if_one(&obj->users)) {
		xa_unlock(&ictx->objects);
		up_write(&obj->destroy_rwsem);
		return false;
	}
	__xa_erase(&ictx->objects, obj->id);
	if (ictx->vfio_ioas && &ictx->vfio_ioas->obj == obj)
		ictx->vfio_ioas = NULL;
	xa_unlock(&ictx->objects);

	iommufd_object_ops[obj->type].destroy(obj);
	up_write(&obj->destroy_rwsem);
	kfree(obj);
	return true;
}

static int iommufd_destroy(struct iommufd_ucmd *ucmd)
{
	struct iommu_destroy *cmd = ucmd->cmd;
	struct iommufd_object *obj;

	obj = iommufd_get_object(ucmd->ictx, cmd->id, IOMMUFD_OBJ_ANY);
	if (IS_ERR(obj))
		return PTR_ERR(obj);
	iommufd_put_object_keep_user(obj);
	if (!iommufd_object_destroy_user(ucmd->ictx, obj))
		return -EBUSY;
	return 0;
}

static int iommufd_fops_open(struct inode *inode, struct file *filp)
{
	struct iommufd_ctx *ictx;

	ictx = kzalloc(sizeof(*ictx), GFP_KERNEL);
	if (!ictx)
		return -ENOMEM;

	xa_init_flags(&ictx->objects, XA_FLAGS_ALLOC1);
	ictx->filp = filp;
	filp->private_data = ictx;
	return 0;
}

static int iommufd_fops_release(struct inode *inode, struct file *filp)
{
	struct iommufd_ctx *ictx = filp->private_data;
	struct iommufd_object *obj;
	unsigned long index = 0;
	int cur = 0;

	/* Destroy the graph from depth first */
	while (cur < IOMMUFD_OBJ_MAX) {
		xa_for_each(&ictx->objects, index, obj) {
			if (obj->type != cur)
				continue;
			xa_erase(&ictx->objects, index);
			if (WARN_ON(!refcount_dec_and_test(&obj->users)))
				continue;
			iommufd_object_ops[obj->type].destroy(obj);
			kfree(obj);
		}
		cur++;
	}
	WARN_ON(!xa_empty(&ictx->objects));
	kfree(ictx);
	return 0;
}

union ucmd_buffer {
	struct iommu_ioas_alloc alloc;
	struct iommu_ioas_iova_ranges iova_ranges;
	struct iommu_ioas_map map;
	struct iommu_ioas_unmap unmap;
	struct iommu_destroy destroy;
};

struct iommufd_ioctl_op {
	unsigned int size;
	unsigned int min_size;
	unsigned int ioctl_num;
	int (*execute)(struct iommufd_ucmd *ucmd);
};

#define IOCTL_OP(_ioctl, _fn, _struct, _last)                                  \
	[_IOC_NR(_ioctl) - IOMMUFD_CMD_BASE] = {                               \
		.size = sizeof(_struct) +                                      \
			BUILD_BUG_ON_ZERO(sizeof(union ucmd_buffer) <          \
					  sizeof(_struct)),                    \
		.min_size = offsetofend(_struct, _last),                       \
		.ioctl_num = _ioctl,                                           \
		.execute = _fn,                                                \
	}
static struct iommufd_ioctl_op iommufd_ioctl_ops[] = {
	IOCTL_OP(IOMMU_DESTROY, iommufd_destroy, struct iommu_destroy, id),
	IOCTL_OP(IOMMU_IOAS_ALLOC, iommufd_ioas_alloc_ioctl,
		 struct iommu_ioas_alloc, out_ioas_id),
	IOCTL_OP(IOMMU_IOAS_COPY, iommufd_ioas_copy, struct iommu_ioas_copy,
		 src_iova),
	IOCTL_OP(IOMMU_IOAS_IOVA_RANGES, iommufd_ioas_iova_ranges,
		 struct iommu_ioas_iova_ranges, __reserved),
	IOCTL_OP(IOMMU_IOAS_MAP, iommufd_ioas_map, struct iommu_ioas_map,
		 __reserved),
	IOCTL_OP(IOMMU_IOAS_UNMAP, iommufd_ioas_unmap, struct iommu_ioas_unmap,
		 length),
	IOCTL_OP(IOMMU_VFIO_IOAS, iommufd_vfio_ioas, struct iommu_vfio_ioas,
		 __reserved),
};

static long iommufd_fops_ioctl(struct file *filp, unsigned int cmd,
			       unsigned long arg)
{
	struct iommufd_ctx *ictx = filp->private_data;
	struct iommufd_ucmd ucmd = {};
	struct iommufd_ioctl_op *op;
	union ucmd_buffer buf;
	unsigned int nr;
	int ret;

	nr = _IOC_NR(cmd);
	if (nr < IOMMUFD_CMD_BASE ||
	    (nr - IOMMUFD_CMD_BASE) >= ARRAY_SIZE(iommufd_ioctl_ops))
		return iommufd_vfio_ioctl(ictx, cmd, arg);

	ucmd.ictx = ictx;
	ucmd.ubuffer = (void __user *)arg;
	ret = get_user(ucmd.user_size, (u32 __user *)ucmd.ubuffer);
	if (ret)
		return ret;

	op = &iommufd_ioctl_ops[nr - IOMMUFD_CMD_BASE];
	if (op->ioctl_num != cmd)
		return -ENOIOCTLCMD;
	if (ucmd.user_size < op->min_size)
		return -EOPNOTSUPP;

	ucmd.cmd = &buf;
	ret = copy_struct_from_user(ucmd.cmd, op->size, ucmd.ubuffer,
				    ucmd.user_size);
	if (ret)
		return ret;
	ret = op->execute(&ucmd);
	return ret;
}

static const struct file_operations iommufd_fops = {
	.owner = THIS_MODULE,
	.open = iommufd_fops_open,
	.release = iommufd_fops_release,
	.unlocked_ioctl = iommufd_fops_ioctl,
};

/**
 * iommufd_fget - Acquires a reference to the iommufd file.
 * @fd: file descriptor
 *
 * Returns a pointer to the iommufd_ctx, otherwise NULL;
 */
struct iommufd_ctx *iommufd_fget(int fd)
{
	struct file *filp;

	filp = fget(fd);
	if (!filp)
		return NULL;

	if (filp->f_op != &iommufd_fops) {
		fput(filp);
		return NULL;
	}
	return filp->private_data;
}

static struct iommufd_object_ops iommufd_object_ops[] = {
	[IOMMUFD_OBJ_DEVICE] = {
		.destroy = iommufd_device_destroy,
	},
	[IOMMUFD_OBJ_IOAS] = {
		.destroy = iommufd_ioas_destroy,
	},
	[IOMMUFD_OBJ_HW_PAGETABLE] = {
		.destroy = iommufd_hw_pagetable_destroy,
	},
};

static struct miscdevice iommu_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "iommu",
	.fops = &iommufd_fops,
	.nodename = "iommu",
	.mode = 0660,
};

static int __init iommufd_init(void)
{
	int ret;

	ret = misc_register(&iommu_misc_dev);
	if (ret) {
		pr_err("Failed to register misc device\n");
		return ret;
	}

	return 0;
}

static void __exit iommufd_exit(void)
{
	misc_deregister(&iommu_misc_dev);
}

module_init(iommufd_init);
module_exit(iommufd_exit);

MODULE_DESCRIPTION("I/O Address Space Management for passthrough devices");
MODULE_LICENSE("GPL v2");

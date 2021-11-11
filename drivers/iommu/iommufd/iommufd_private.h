/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#ifndef __IOMMUFD_PRIVATE_H
#define __IOMMUFD_PRIVATE_H

#include <linux/rwsem.h>
#include <linux/xarray.h>
#include <linux/refcount.h>
#include <linux/uaccess.h>

struct iommufd_ctx {
	struct file *filp;
	struct xarray objects;
};

struct iommufd_ctx *iommufd_fget(int fd);

struct iommufd_ucmd {
	struct iommufd_ctx *ictx;
	void __user *ubuffer;
	u32 user_size;
	void *cmd;
};

/* Copy the response in ucmd->cmd back to userspace. */
static inline int iommufd_ucmd_respond(struct iommufd_ucmd *ucmd,
				       size_t cmd_len)
{
	if (copy_to_user(ucmd->ubuffer, ucmd->cmd,
			 min_t(size_t, ucmd->user_size, cmd_len)))
		return -EFAULT;
	return 0;
}

/*
 * The objects for an acyclic graph through the users refcount. This enum must
 * be sorted by type depth first so that destruction completes lower objects and
 * releases the users refcount before reaching higher objects in the graph.
 */
enum iommufd_object_type {
	IOMMUFD_OBJ_NONE,
	IOMMUFD_OBJ_ANY = IOMMUFD_OBJ_NONE,
	IOMMUFD_OBJ_MAX,
};

/* Base struct for all objects with a userspace ID handle. */
struct iommufd_object {
	struct rw_semaphore destroy_rwsem;
	refcount_t users;
	enum iommufd_object_type type;
	unsigned int id;
};

static inline bool iommufd_lock_obj(struct iommufd_object *obj)
{
	if (!down_read_trylock(&obj->destroy_rwsem))
		return false;
	if (!refcount_inc_not_zero(&obj->users)) {
		up_read(&obj->destroy_rwsem);
		return false;
	}
	return true;
}

struct iommufd_object *iommufd_get_object(struct iommufd_ctx *ictx, u32 id,
					  enum iommufd_object_type type);
static inline void iommufd_put_object(struct iommufd_object *obj)
{
	refcount_dec(&obj->users);
	up_read(&obj->destroy_rwsem);
}
static inline void iommufd_put_object_keep_user(struct iommufd_object *obj)
{
	up_read(&obj->destroy_rwsem);
}
void iommufd_object_abort(struct iommufd_ctx *ictx, struct iommufd_object *obj);
void iommufd_object_finalize(struct iommufd_ctx *ictx,
			     struct iommufd_object *obj);
bool iommufd_object_destroy_user(struct iommufd_ctx *ictx,
				 struct iommufd_object *obj);
struct iommufd_object *_iommufd_object_alloc(struct iommufd_ctx *ictx,
					     size_t size,
					     enum iommufd_object_type type);

#define iommufd_object_alloc(ictx, ptr, type)                                  \
	container_of(_iommufd_object_alloc(                                    \
			     ictx,                                             \
			     sizeof(*(ptr)) + BUILD_BUG_ON_ZERO(               \
						      offsetof(typeof(*(ptr)), \
							       obj) != 0),     \
			     type),                                            \
		     typeof(*(ptr)), obj)

#endif

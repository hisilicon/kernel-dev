/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2021 Intel Corporation
 * Copyright (c) 2021-2022, NVIDIA CORPORATION & AFFILIATES
 */
#ifndef __LINUX_IOMMUFD_H
#define __LINUX_IOMMUFD_H

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/device.h>

#define IOMMUFD_INVALID_ID  0

struct pci_dev;
struct iommufd_device;
struct iommufd_ctx;

#if IS_ENABLED(CONFIG_IOMMUFD)
struct iommufd_device *iommufd_bind_pci_device(int fd, struct pci_dev *pdev,
					       u32 *id);
void iommufd_unbind_device(struct iommufd_device *idev);

enum {
	IOMMUFD_ATTACH_FLAGS_ALLOW_UNSAFE_INTERRUPT = 1 << 0,
};
int iommufd_device_attach(struct iommufd_device *idev, u32 *pt_id,
			  unsigned int flags);
void iommufd_device_detach(struct iommufd_device *idev);

struct iommufd_ctx *vfio_group_set_iommufd(int fd, struct list_head *device_list);
void vfio_group_unset_iommufd(void *iommufd, struct list_head *device_list);
int iommufd_vfio_check_extension(unsigned long type);
#else /* !CONFIG_IOMMUFD */
static inline struct iommufd_device *
iommufd_bind_pci_device(int fd, struct pci_dev *pdev, u32 *id)
{
	return ERR_PTR(-EOPNOTSUPP);
}

static inline void iommufd_unbind_device(struct iommufd_device *idev)
{
}

static inline int iommufd_device_attach(struct iommufd_device *idev,
					u32 ioas_id)
{
	return -EOPNOTSUPP;
}

static inline void iommufd_device_detach(struct iommufd_device *idev)
{
}

static inline struct iommufd_ctx *
vfio_group_set_iommufd(int fd, struct list_head *device_list)
{
	return NULL;
}

static inline void vfio_group_unset_iommufd(void *iommufd,
					    struct list_head *device_list)
{
}

static int iommufd_vfio_check_extension(unsigned long type)
{
	return -EOPNOTSUPP;
}
#endif /* CONFIG_IOMMUFD */
#endif

/*
 * Copyright (c) 2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __WD_H
#define __WD_H

#include <linux/device.h>
#include <linux/vfio.h>
#include <linux/mdev.h>
#include <linux/vfio.h>
#include <linux/iommu.h>

#include "wd_usr_if.h"

struct wd_queue;
struct wd_dev;

/**
 * struct wd_dev_ops - WD device operations
 * @get_queue:  get a queue from the device according to algorithm
 * @put_queue:  free a queue to the device
 * @is_q_updated:  check wether the task is finished
 * @mask_notification: mask the task irq of queue
 * @mmap:  mmap adresses of queue to user space
 * @open:  open queue to start it
 * @close:  close queue to stop it
 * @reset:  reset the WD device
 * @reset_queue:  reset the queue
 * @ioctl:   ioctl for user space users of the queue
 */
struct wd_dev_ops {
	int (*get_queue)(struct wd_dev *wdev, const char *alg,
		struct wd_queue **q);
	int (*put_queue)(struct wd_queue *q);
	int (*is_q_updated)(struct wd_queue *q);
	void (*mask_notification)(struct wd_queue *q, int event_mask);
	int (*mmap)(struct wd_queue *q, struct vm_area_struct *vma);
	int (*open)(struct wd_queue *q);
	int (*close)(struct wd_queue *q);
	int (*reset)(struct wd_dev *wdev);
	int (*reset_queue)(struct wd_queue *q);
	long (*ioctl)(struct wd_queue *q, unsigned int cmd,
				unsigned long arg);
};

/**
 * struct wd_queue - WD queue on WD device
 * @mutex:  mutex while multiple threads
 * @wdev:  WD device it belongs to
 * @pid:   Process ID that it belongs to
 * @flags:  queue attributions indication
 * @mdev:  mediated devices it based on
 * @priv:   driver data
 * @vdi:   information of its VFIO device
 * @status:  status of WD management
 */
struct wd_queue {
	struct mutex mutex;
	struct wd_dev *wdev;
	int pid;
	__u32 flags;
	struct mdev_device *mdev;
	void *priv;
	struct vfio_device_info vdi;
	int status;
};

/**
 * struct wd_dev - Wrapdrive device description
 * @name:  device name
 * @status:  device status
 * @ref:  referrence count
 * @owner: module owner
 * @ops:  wd device operations
 * @dev:  its kernel device
 * @cls_dev:  its class device
 * @is_vf:  denotes wether it is virtual function
 * @iommu_type:  iommu type of hardware
 * @dev_id:   device ID
 * @priv: driver private data
 * @node_id: socket ID
 * @priority: priority while being selected, also can be set by users
 * @latency_level: latency while doing acceleration
 * @throughput_level: throughput while doing acceleration
 * @flags: device attributions
 * @api_ver: API version of WD
 * @mdev_fops: mediated device's parent operations
 */
struct wd_dev {
	char *name;
	int status;
	atomic_t ref;
	struct module *owner;
	const struct wd_dev_ops *ops;
	struct device *dev;
	struct device cls_dev;
	bool is_vf;
	u32 iommu_type;
	u32 dev_id;
	void *priv;
	int node_id;
	int priority;
	int latency_level;
	int throughput_level;
	int flags;
	const char *api_ver;
	struct mdev_parent_ops mdev_fops;
};

int wd_dev_register(struct wd_dev *wdev);
void wd_dev_unregister(struct wd_dev *wdev);
struct wd_queue *wd_queue(struct device *dev);
void wd_wake_up(struct wd_queue *q);

extern struct device_attribute dev_attr_pid;
#define WD_DEFAULT_MDEV_DEV_ATTRS \
	&dev_attr_pid.attr,

extern struct mdev_type_attribute mdev_type_attr_latency;
extern struct mdev_type_attribute mdev_type_attr_throughput;
extern struct mdev_type_attribute mdev_type_attr_flags;
#define WD_DEFAULT_MDEV_TYPE_ATTRS \
	&mdev_type_attr_latency.attr, \
	&mdev_type_attr_throughput.attr, \
	&mdev_type_attr_flags.attr,

#define _WD_EVENT_NOTIFY		(1 << 0)
#define _WD_EVENT_DISABLE		(1 << 1)
#endif

/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef _LINUX_UACCE_H
#define _LINUX_UACCE_H

#include <linux/cdev.h>
#include <uapi/misc/uacce/uacce.h>

#define UACCE_NAME		"uacce"
#define UACCE_MAX_REGION	3
#define UACCE_MAX_NAME_SIZE	64

struct uacce_queue;
struct uacce_device;

struct uacce_hw_err {
	struct list_head list;
	unsigned long long tick_stamp;
};

struct uacce_err_isolate {
	struct list_head hw_errs;
	u32 hw_err_isolate_hz;	/* user cfg freq which triggers isolation */
	atomic_t is_isolate;
};

struct uacce_dma_slice {
	void *kaddr;	/* kernel address for ss */
	dma_addr_t dma;	/* dma address, if created by dma api */
	u64 size;	/* Size of this dma slice */
	u32 total_num;	/* Total slices in this dma list */
};

/**
 * struct uacce_qfile_region - structure of queue file region
 * @type: type of the region
 */
struct uacce_qfile_region {
	enum uacce_qfrt type;
	unsigned long iova;	/* iova share between user and device space */
	unsigned long nr_pages;
	int prot;
	unsigned int flags;
	struct list_head qs;	/* qs sharing the same region, for ss */
	void *kaddr;		/* kernel address for dko */
	struct uacce_dma_slice *dma_list;
};

/**
 * struct uacce_ops - uacce device operations
 * @get_available_instances:  get available instances left of the device
 * @get_queue: get a queue from the device
 * @put_queue: free a queue to the device
 * @start_queue: make the queue start work after get_queue
 * @stop_queue: make the queue stop work before put_queue
 * @is_q_updated: check whether the task is finished
 * @mmap: mmap addresses of queue to user space
 * @ioctl: ioctl for user space users of the queue
 */
struct uacce_ops {
	int (*get_available_instances)(struct uacce_device *uacce);
	int (*get_queue)(struct uacce_device *uacce, unsigned long arg,
			 struct uacce_queue *q);
	void (*put_queue)(struct uacce_queue *q);
	int (*start_queue)(struct uacce_queue *q);
	void (*stop_queue)(struct uacce_queue *q);
	int (*is_q_updated)(struct uacce_queue *q);
	int (*mmap)(struct uacce_queue *q, struct vm_area_struct *vma,
		    struct uacce_qfile_region *qfr);
	long (*ioctl)(struct uacce_queue *q, unsigned int cmd,
		      unsigned long arg);
};

/**
 * struct uacce_interface - interface required for uacce_register()
 * @name: the uacce device name.  Will show up in sysfs
 * @flags: uacce device attributes
 * @ops: pointer to the struct uacce_ops
 */
struct uacce_interface {
	char name[UACCE_MAX_NAME_SIZE];
	unsigned int flags;
	const struct uacce_ops *ops;
};

enum uacce_q_state {
	UACCE_Q_ZOMBIE = 0,
	UACCE_Q_INIT,
	UACCE_Q_STARTED,
};

/**
 * struct uacce_queue
 * @uacce: pointer to uacce
 * @priv: private pointer
 * @wait: wait queue head
 * @list: index into uacce queues list
 * @qfrs: pointer of qfr regions
 * @state: queue state machine
 * @pasid: pasid associated to the mm
 * @handle: iommu_sva handle returned by iommu_sva_bind_device()
 */
struct uacce_queue {
	struct uacce_device *uacce;
	void *priv;
	wait_queue_head_t wait;
	struct list_head list;
	struct uacce_qfile_region *qfrs[UACCE_MAX_REGION];
	struct file *filep;
	enum uacce_q_state state;
	u32 pasid;
	struct iommu_sva *handle;
};

/**
 * struct uacce_device
 * @algs: supported algorithms
 * @api_ver: api version
 * @ops: pointer to the struct uacce_ops
 * @qf_pg_num: page numbers of the queue file regions
 * @parent: pointer to the parent device
 * @is_vf: whether virtual function
 * @flags: uacce attributes
 * @dev_id: id of the uacce device
 * @cdev: cdev of the uacce
 * @dev: dev of the uacce
 * @priv: private pointer of the uacce
 * @queues: list of queues
 * @queues_lock: lock for queues list
 * @inode: core vfs
 */
struct uacce_device {
	const char *algs;
	const char *api_ver;
	const struct uacce_ops *ops;
	unsigned long qf_pg_num[UACCE_MAX_REGION];
	struct device *parent;
	bool is_vf;
	u32 flags;
	u32 dev_id;
	struct cdev *cdev;
	struct device dev;
	void *priv;
	atomic_t ref;
	struct uacce_err_isolate isolate_data;
	struct uacce_err_isolate *isolate;
	struct list_head queues;
	struct mutex queues_lock;
	struct inode *inode;
};

#if IS_ENABLED(CONFIG_UACCE)

struct uacce_device *uacce_alloc(struct device *parent,
				 struct uacce_interface *interface);
int uacce_register(struct uacce_device *uacce);
void uacce_remove(struct uacce_device *uacce);
struct uacce_device *dev_to_uacce(struct device *dev);
int uacce_hw_err_isolate(struct uacce_device *uacce);
#else /* CONFIG_UACCE */

static inline
struct uacce_device *uacce_alloc(struct device *parent,
				 struct uacce_interface *interface)
{
	return ERR_PTR(-ENODEV);
}

static inline int uacce_register(struct uacce_device *uacce)
{
	return -EINVAL;
}

static inline void uacce_remove(struct uacce_device *uacce) {}

#endif /* CONFIG_UACCE */

#endif /* _LINUX_UACCE_H */

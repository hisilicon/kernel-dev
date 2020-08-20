/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_IOASID_H
#define __LINUX_IOASID_H

#include <linux/types.h>
#include <linux/errno.h>

#define INVALID_IOASID ((ioasid_t)-1)
typedef unsigned int ioasid_t;
typedef ioasid_t (*ioasid_alloc_fn_t)(ioasid_t min, ioasid_t max, void *data);
typedef void (*ioasid_free_fn_t)(ioasid_t ioasid, void *data);

/* IOASID set types */
enum ioasid_set_type {
	IOASID_SET_TYPE_NULL = 1, /* Set token is NULL */
	IOASID_SET_TYPE_MM,	  /* Set token is a mm_struct,
				   * i.e. associated with a process
				   */
	IOASID_SET_TYPE_NR,
};

/**
 * struct ioasid_set - Meta data about ioasid_set
 * @type:	Token types and other features
 * @token:	Unique to identify an IOASID set
 * @nh:		Notifier for IOASID events within the set
 * @xa:		XArray to store ioasid_set private IDs, can be used for
 *		guest-host IOASID mapping, or just a private IOASID namespace.
 * @quota:	Max number of IOASIDs can be allocated within the set
 * @nr_ioasids	Number of IOASIDs currently allocated in the set
 * @sid:	ID of the set
 * @ref:	Reference count of the users
 */
struct ioasid_set {
	void *token;
	struct atomic_notifier_head nh;
	struct xarray xa;
	int type;
	int quota;
	int nr_ioasids;
	int sid;
	refcount_t ref;
	struct rcu_head rcu;
};

/**
 * struct ioasid_allocator_ops - IOASID allocator helper functions and data
 *
 * @alloc:	helper function to allocate IOASID
 * @free:	helper function to free IOASID
 * @list:	for tracking ops that share helper functions but not data
 * @pdata:	data belong to the allocator, provided when calling alloc()
 */
struct ioasid_allocator_ops {
	ioasid_alloc_fn_t alloc;
	ioasid_free_fn_t free;
	struct list_head list;
	void *pdata;
};

/* Notification data when IOASID status changed */
enum ioasid_notify_val {
	IOASID_ALLOC = 1,
	IOASID_FREE,
	IOASID_BIND,
	IOASID_UNBIND,
	IOASID_SET_ALLOC,
	IOASID_SET_FREE,
};

#define IOASID_NOTIFY_ALL BIT(0)
#define IOASID_NOTIFY_SET BIT(1)
/**
 * enum ioasid_notifier_prios - IOASID event notification order
 *
 * When status of an IOASID changes, users might need to take actions to
 * reflect the new state. For example, when an IOASID is freed due to
 * exception, the hardware context in virtual CPU, DMA device, and IOMMU
 * shall be cleared and drained. Order is required to prevent life cycle
 * problems.
 */
enum ioasid_notifier_prios {
	IOASID_PRIO_LAST,
	IOASID_PRIO_DEVICE,
	IOASID_PRIO_IOMMU,
	IOASID_PRIO_CPU,
};

/**
 * struct ioasid_nb_args - Argument provided by IOASID core when notifier
 * is called.
 * @id:		The IOASID being notified
 * @spid:	The set private ID associated with the IOASID
 * @set:	The IOASID set of @id
 * @pdata:	The private data attached to the IOASID
 */
struct ioasid_nb_args {
	ioasid_t id;
	ioasid_t spid;
	struct ioasid_set *set;
	void *pdata;
};

#if IS_ENABLED(CONFIG_IOASID)
void ioasid_install_capacity(ioasid_t total);
struct ioasid_set *ioasid_alloc_set(void *token, ioasid_t quota, int type);
int ioasid_adjust_set(struct ioasid_set *set, int quota);
void ioasid_set_get_locked(struct ioasid_set *set);
void ioasid_set_put_locked(struct ioasid_set *set);
void ioasid_set_put(struct ioasid_set *set);

ioasid_t ioasid_alloc(struct ioasid_set *set, ioasid_t min, ioasid_t max,
		      void *private);
void ioasid_free(struct ioasid_set *set, ioasid_t ioasid);

bool ioasid_is_active(ioasid_t ioasid);

void *ioasid_find(struct ioasid_set *set, ioasid_t ioasid, bool (*getter)(void *));
int ioasid_attach_data(ioasid_t ioasid, void *data);
int ioasid_attach_spid(ioasid_t ioasid, ioasid_t spid);
ioasid_t ioasid_find_by_spid(struct ioasid_set *set, ioasid_t spid);

int ioasid_register_notifier(struct ioasid_set *set,
			struct notifier_block *nb);
void ioasid_unregister_notifier(struct ioasid_set *set,
				struct notifier_block *nb);

int ioasid_register_allocator(struct ioasid_allocator_ops *allocator);
void ioasid_unregister_allocator(struct ioasid_allocator_ops *allocator);

int ioasid_notify(ioasid_t ioasid, enum ioasid_notify_val cmd, unsigned int flags);
void ioasid_is_in_set(struct ioasid_set *set, ioasid_t ioasid);
int ioasid_get(struct ioasid_set *set, ioasid_t ioasid);
int ioasid_get_locked(struct ioasid_set *set, ioasid_t ioasid);
void ioasid_put(struct ioasid_set *set, ioasid_t ioasid);
void ioasid_put_locked(struct ioasid_set *set, ioasid_t ioasid);
int ioasid_set_for_each_ioasid(struct ioasid_set *sdata,
			       void (*fn)(ioasid_t id, void *data),
			       void *data);
int ioasid_register_notifier_mm(struct mm_struct *mm, struct notifier_block *nb);
void ioasid_unregister_notifier_mm(struct mm_struct *mm, struct notifier_block *nb);

#else /* !CONFIG_IOASID */
static inline ioasid_t ioasid_alloc(struct ioasid_set *set, ioasid_t min,
				    ioasid_t max, void *private)
{
	return INVALID_IOASID;
}

static inline void ioasid_free(struct ioasid_set *set, ioasid_t ioasid)
{
}

static inline bool ioasid_is_active(ioasid_t ioasid)
{
	return false;
}

static inline struct ioasid_set *ioasid_alloc_set(void *token, ioasid_t quota, int type)
{
	return ERR_PTR(-ENOTSUPP);
}

static inline void ioasid_set_put(struct ioasid_set *set)
{
}

static inline void *ioasid_find(struct ioasid_set *set, ioasid_t ioasid, bool (*getter)(void *))
{
	return NULL;
}

static inline int ioasid_register_notifier(struct notifier_block *nb)
{
	return -ENOTSUPP;
}

static inline void ioasid_unregister_notifier(struct notifier_block *nb)
{
}

static inline int ioasid_notify(ioasid_t ioasid, enum ioasid_notify_val cmd, unsigned int flags)
{
	return -ENOTSUPP;
}

static inline int ioasid_register_allocator(struct ioasid_allocator_ops *allocator)
{
	return -ENOTSUPP;
}

static inline void ioasid_unregister_allocator(struct ioasid_allocator_ops *allocator)
{
}

static inline int ioasid_attach_data(ioasid_t ioasid, void *data)
{
	return -ENOTSUPP;
}

staic inline int ioasid_attach_spid(ioasid_t ioasid, ioasid_t spid)
{
	return -ENOTSUPP;
}

static inline ioasid_t ioasid_find_by_spid(struct ioasid_set *set, ioasid_t spid)
{
	return -ENOTSUPP;
}

#endif /* CONFIG_IOASID */
#endif /* __LINUX_IOASID_H */

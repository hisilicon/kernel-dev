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
 * @xa:		XArray to store ioasid_set private IDs, can be used for
 *		guest-host IOASID mapping, or just a private IOASID namespace.
 * @quota:	Max number of IOASIDs can be allocated within the set
 * @nr_ioasids	Number of IOASIDs currently allocated in the set
 * @sid:	ID of the set
 * @ref:	Reference count of the users
 */
struct ioasid_set {
	void *token;
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
int ioasid_register_allocator(struct ioasid_allocator_ops *allocator);
void ioasid_unregister_allocator(struct ioasid_allocator_ops *allocator);
void ioasid_is_in_set(struct ioasid_set *set, ioasid_t ioasid);
int ioasid_get(struct ioasid_set *set, ioasid_t ioasid);
int ioasid_get_locked(struct ioasid_set *set, ioasid_t ioasid);
void ioasid_put(struct ioasid_set *set, ioasid_t ioasid);
void ioasid_put_locked(struct ioasid_set *set, ioasid_t ioasid);
int ioasid_set_for_each_ioasid(struct ioasid_set *sdata,
			       void (*fn)(ioasid_t id, void *data),
			       void *data);
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

#endif /* CONFIG_IOASID */
#endif /* __LINUX_IOASID_H */

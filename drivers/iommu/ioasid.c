// SPDX-License-Identifier: GPL-2.0
/*
 * I/O Address Space ID allocator. There is one global IOASID space, split into
 * subsets. Users create a subset with ioasid_alloc_set, then allocate/free IDs
 * with ioasid_alloc and ioasid_free.
 */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/xarray.h>
#include <linux/ioasid.h>

static DEFINE_XARRAY_ALLOC(ioasid_sets);
enum ioasid_state {
	IOASID_STATE_INACTIVE,
	IOASID_STATE_ACTIVE,
	IOASID_STATE_FREE_PENDING,
};

/**
 * struct ioasid_data - Meta data about ioasid
 *
 * @id:		Unique ID
 * @users	Number of active users
 * @state	Track state of the IOASID
 * @set		Meta data of the set this IOASID belongs to
 * @private	Private data associated with the IOASID
 * @rcu		For free after RCU grace period
 */
struct ioasid_data {
	ioasid_t id;
	struct ioasid_set *set;
	refcount_t users;
	enum ioasid_state state;
	void *private;
	struct rcu_head rcu;
};

/* Default to PCIe standard 20 bit PASID */
#define PCI_PASID_MAX 0x100000
static ioasid_t ioasid_capacity = PCI_PASID_MAX;
static ioasid_t ioasid_capacity_avail = PCI_PASID_MAX;

void ioasid_install_capacity(ioasid_t total)
{
	ioasid_capacity = ioasid_capacity_avail = total;
}
EXPORT_SYMBOL_GPL(ioasid_install_capacity);

/*
 * struct ioasid_allocator_data - Internal data structure to hold information
 * about an allocator. There are two types of allocators:
 *
 * - Default allocator always has its own XArray to track the IOASIDs allocated.
 * - Custom allocators may share allocation helpers with different private data.
 *   Custom allocators that share the same helper functions also share the same
 *   XArray.
 * Rules:
 * 1. Default allocator is always available, not dynamically registered. This is
 *    to prevent race conditions with early boot code that want to register
 *    custom allocators or allocate IOASIDs.
 * 2. Custom allocators take precedence over the default allocator.
 * 3. When all custom allocators sharing the same helper functions are
 *    unregistered (e.g. due to hotplug), all outstanding IOASIDs must be
 *    freed. Otherwise, outstanding IOASIDs will be lost and orphaned.
 * 4. When switching between custom allocators sharing the same helper
 *    functions, outstanding IOASIDs are preserved.
 * 5. When switching between custom allocator and default allocator, all IOASIDs
 *    must be freed to ensure unadulterated space for the new allocator.
 *
 * @ops:	allocator helper functions and its data
 * @list:	registered custom allocators
 * @slist:	allocators share the same ops but different data
 * @flags:	attributes of the allocator
 * @xa:		xarray holds the IOASID space
 * @rcu:	used for kfree_rcu when unregistering allocator
 */
struct ioasid_allocator_data {
	struct ioasid_allocator_ops *ops;
	struct list_head list;
	struct list_head slist;
#define IOASID_ALLOCATOR_CUSTOM BIT(0) /* Needs framework to track results */
	unsigned long flags;
	struct xarray xa;
	struct rcu_head rcu;
};

static DEFINE_SPINLOCK(ioasid_allocator_lock);
static LIST_HEAD(allocators_list);

static ioasid_t default_alloc(ioasid_t min, ioasid_t max, void *opaque);
static void default_free(ioasid_t ioasid, void *opaque);

static struct ioasid_allocator_ops default_ops = {
	.alloc = default_alloc,
	.free = default_free,
};

static struct ioasid_allocator_data default_allocator = {
	.ops = &default_ops,
	.flags = 0,
	.xa = XARRAY_INIT(ioasid_xa, XA_FLAGS_ALLOC),
};

static struct ioasid_allocator_data *active_allocator = &default_allocator;

static ioasid_t default_alloc(ioasid_t min, ioasid_t max, void *opaque)
{
	ioasid_t id;

	if (xa_alloc(&default_allocator.xa, &id, opaque, XA_LIMIT(min, max), GFP_ATOMIC)) {
		pr_err("Failed to alloc ioasid from %d to %d\n", min, max);
		return INVALID_IOASID;
	}

	return id;
}

static void default_free(ioasid_t ioasid, void *opaque)
{
	struct ioasid_data *ioasid_data;

	ioasid_data = xa_erase(&default_allocator.xa, ioasid);
	kfree_rcu(ioasid_data, rcu);
}

/* Allocate and initialize a new custom allocator with its helper functions */
static struct ioasid_allocator_data *ioasid_alloc_allocator(struct ioasid_allocator_ops *ops)
{
	struct ioasid_allocator_data *ia_data;

	ia_data = kzalloc(sizeof(*ia_data), GFP_ATOMIC);
	if (!ia_data)
		return NULL;

	xa_init_flags(&ia_data->xa, XA_FLAGS_ALLOC);
	INIT_LIST_HEAD(&ia_data->slist);
	ia_data->flags |= IOASID_ALLOCATOR_CUSTOM;
	ia_data->ops = ops;

	/* For tracking custom allocators that share the same ops */
	list_add_tail(&ops->list, &ia_data->slist);

	return ia_data;
}

static bool use_same_ops(struct ioasid_allocator_ops *a, struct ioasid_allocator_ops *b)
{
	return (a->free == b->free) && (a->alloc == b->alloc);
}

/**
 * ioasid_register_allocator - register a custom allocator
 * @ops: the custom allocator ops to be registered
 *
 * Custom allocators take precedence over the default xarray based allocator.
 * Private data associated with the IOASID allocated by the custom allocators
 * are managed by IOASID framework similar to data stored in xa by default
 * allocator.
 *
 * There can be multiple allocators registered but only one is active. In case
 * of runtime removal of a custom allocator, the next one is activated based
 * on the registration ordering.
 *
 * Multiple allocators can share the same alloc() function, in this case the
 * IOASID space is shared.
 */
int ioasid_register_allocator(struct ioasid_allocator_ops *ops)
{
	struct ioasid_allocator_data *ia_data;
	struct ioasid_allocator_data *pallocator;
	int ret = 0;

	spin_lock(&ioasid_allocator_lock);

	ia_data = ioasid_alloc_allocator(ops);
	if (!ia_data) {
		ret = -ENOMEM;
		goto out_unlock;
	}

	/*
	 * No particular preference, we activate the first one and keep
	 * the later registered allocators in a list in case the first one gets
	 * removed due to hotplug.
	 */
	if (list_empty(&allocators_list)) {
		WARN_ON(active_allocator != &default_allocator);
		/* Use this new allocator if default is not active */
		if (xa_empty(&active_allocator->xa)) {
			rcu_assign_pointer(active_allocator, ia_data);
			list_add_tail(&ia_data->list, &allocators_list);
			goto out_unlock;
		}
		pr_warn("Default allocator active with outstanding IOASID\n");
		ret = -EAGAIN;
		goto out_free;
	}

	/* Check if the allocator is already registered */
	list_for_each_entry(pallocator, &allocators_list, list) {
		if (pallocator->ops == ops) {
			pr_err("IOASID allocator already registered\n");
			ret = -EEXIST;
			goto out_free;
		} else if (use_same_ops(pallocator->ops, ops)) {
			/*
			 * If the new allocator shares the same ops,
			 * then they will share the same IOASID space.
			 * We should put them under the same xarray.
			 */
			list_add_tail(&ops->list, &pallocator->slist);
			goto out_free;
		}
	}
	list_add_tail(&ia_data->list, &allocators_list);

	spin_unlock(&ioasid_allocator_lock);
	return 0;
out_free:
	kfree(ia_data);
out_unlock:
	spin_unlock(&ioasid_allocator_lock);
	return ret;
}
EXPORT_SYMBOL_GPL(ioasid_register_allocator);

/**
 * ioasid_unregister_allocator - Remove a custom IOASID allocator ops
 * @ops: the custom allocator to be removed
 *
 * Remove an allocator from the list, activate the next allocator in
 * the order it was registered. Or revert to default allocator if all
 * custom allocators are unregistered without outstanding IOASIDs.
 */
void ioasid_unregister_allocator(struct ioasid_allocator_ops *ops)
{
	struct ioasid_allocator_data *pallocator;
	struct ioasid_allocator_ops *sops;

	spin_lock(&ioasid_allocator_lock);
	if (list_empty(&allocators_list)) {
		pr_warn("No custom IOASID allocators active!\n");
		goto exit_unlock;
	}

	list_for_each_entry(pallocator, &allocators_list, list) {
		if (!use_same_ops(pallocator->ops, ops))
			continue;

		if (list_is_singular(&pallocator->slist)) {
			/* No shared helper functions */
			list_del(&pallocator->list);
			/*
			 * All IOASIDs should have been freed before
			 * the last allocator that shares the same ops
			 * is unregistered.
			 */
			WARN_ON(!xa_empty(&pallocator->xa));
			if (list_empty(&allocators_list)) {
				pr_info("No custom IOASID allocators, switch to default.\n");
				rcu_assign_pointer(active_allocator, &default_allocator);
			} else if (pallocator == active_allocator) {
				rcu_assign_pointer(active_allocator,
						list_first_entry(&allocators_list,
								struct ioasid_allocator_data, list));
				pr_info("IOASID allocator changed");
			}
			kfree_rcu(pallocator, rcu);
			break;
		}
		/*
		 * Find the matching shared ops to delete,
		 * but keep outstanding IOASIDs
		 */
		list_for_each_entry(sops, &pallocator->slist, list) {
			if (sops == ops) {
				list_del(&ops->list);
				break;
			}
		}
		break;
	}

exit_unlock:
	spin_unlock(&ioasid_allocator_lock);
}
EXPORT_SYMBOL_GPL(ioasid_unregister_allocator);

/**
 * ioasid_attach_data - Set private data for an allocated ioasid
 * @ioasid: the ID to set data
 * @data:   the private data
 *
 * For IOASID that is already allocated, private data can be set
 * via this API. Future lookup can be done via ioasid_find.
 */
int ioasid_attach_data(ioasid_t ioasid, void *data)
{
	struct ioasid_data *ioasid_data;
	int ret = 0;

	spin_lock(&ioasid_allocator_lock);
	ioasid_data = xa_load(&active_allocator->xa, ioasid);
	if (ioasid_data)
		rcu_assign_pointer(ioasid_data->private, data);
	else
		ret = -ENOENT;
	spin_unlock(&ioasid_allocator_lock);

	/*
	 * Wait for readers to stop accessing the old private data, so the
	 * caller can free it.
	 */
	if (!ret)
		synchronize_rcu();

	return ret;
}
EXPORT_SYMBOL_GPL(ioasid_attach_data);

/**
 * ioasid_alloc - Allocate an IOASID
 * @set: the IOASID set
 * @min: the minimum ID (inclusive)
 * @max: the maximum ID (inclusive)
 * @private: data private to the caller
 *
 * Allocate an ID between @min and @max. The @private pointer is stored
 * internally and can be retrieved with ioasid_find().
 *
 * Return: the allocated ID on success, or %INVALID_IOASID on failure.
 */
ioasid_t ioasid_alloc(struct ioasid_set *set, ioasid_t min, ioasid_t max,
		      void *private)
{
	struct ioasid_data *data;
	void *adata;
	ioasid_t id = INVALID_IOASID;

	spin_lock(&ioasid_allocator_lock);
	/* Check if the IOASID set has been allocated and initialized */
	if (WARN_ON(xa_load(&ioasid_sets, set->sid) != set)) {
		pr_warn("Invalid set\n");
		goto done_unlock;
	}

	if (set->quota <= set->nr_ioasids) {
		pr_err("IOASID set %d out of quota %d\n", set->sid, set->quota);
		goto done_unlock;
	}

	data = kzalloc(sizeof(*data), GFP_ATOMIC);
	if (!data)
		goto done_unlock;

	data->set = set;
	data->private = private;

	/*
	 * Custom allocator needs allocator data to perform platform specific
	 * operations.
	 */
	adata = active_allocator->flags & IOASID_ALLOCATOR_CUSTOM ? active_allocator->ops->pdata : data;
	id = active_allocator->ops->alloc(min, max, adata);
	if (id == INVALID_IOASID) {
		pr_err("Failed ASID allocation %lu\n", active_allocator->flags);
		goto exit_free;
	}

	if ((active_allocator->flags & IOASID_ALLOCATOR_CUSTOM) &&
	     xa_alloc(&active_allocator->xa, &id, data, XA_LIMIT(id, id), GFP_ATOMIC)) {
		/* Custom allocator needs framework to store and track allocation results */
		pr_err("Failed to alloc ioasid from %d\n", id);
		active_allocator->ops->free(id, active_allocator->ops->pdata);
		goto exit_free;
	}
	data->id = id;
	data->state = IOASID_STATE_ACTIVE;
	refcount_set(&data->users, 1);

	/* Store IOASID in the per set data */
	if (xa_err(xa_store(&set->xa, id, data, GFP_ATOMIC))) {
		pr_err("Failed to ioasid %d in set %d\n", id, set->sid);
		goto exit_free;
	}
	set->nr_ioasids++;
	goto done_unlock;

exit_free:
	kfree(data);
done_unlock:
	spin_unlock(&ioasid_allocator_lock);
	return id;
}
EXPORT_SYMBOL_GPL(ioasid_alloc);

static void ioasid_do_free(struct ioasid_data *data)
{
	struct ioasid_data *ioasid_data;
	struct ioasid_set *sdata;

	active_allocator->ops->free(data->id, active_allocator->ops->pdata);
	/* Custom allocator needs additional steps to free the xa element */
	if (active_allocator->flags & IOASID_ALLOCATOR_CUSTOM) {
		ioasid_data = xa_erase(&active_allocator->xa, data->id);
		kfree_rcu(ioasid_data, rcu);
	}

	sdata = xa_load(&ioasid_sets, data->set->sid);
	if (!sdata) {
		pr_err("No set %d for IOASID %d\n", data->set->sid,
		       data->id);
		return;
	}
	xa_erase(&sdata->xa, data->id);
	sdata->nr_ioasids--;
}

static void ioasid_free_locked(struct ioasid_set *set, ioasid_t ioasid)
{
	struct ioasid_data *data;

	data = xa_load(&active_allocator->xa, ioasid);
	if (!data) {
		pr_err("Trying to free unknown IOASID %u\n", ioasid);
		return;
	}

	if (data->set != set) {
		pr_warn("Cannot free IOASID %u due to set ownership\n", ioasid);
		return;
	}
	data->state = IOASID_STATE_FREE_PENDING;

	if (!refcount_dec_and_test(&data->users))
		return;

	ioasid_do_free(data);
}

/**
 * ioasid_free - Drop reference on an IOASID. Free if refcount drops to 0,
 *               including free from its set and system-wide list.
 * @set:	The ioasid_set to check permission with. If not NULL, IOASID
 *		free will fail if the set does not match.
 * @ioasid:	The IOASID to remove
 */
void ioasid_free(struct ioasid_set *set, ioasid_t ioasid)
{
	spin_lock(&ioasid_allocator_lock);
	ioasid_free_locked(set, ioasid);
	spin_unlock(&ioasid_allocator_lock);
}
EXPORT_SYMBOL_GPL(ioasid_free);

/**
 * ioasid_alloc_set - Allocate a new IOASID set for a given token
 *
 * @token:	Unique token of the IOASID set, cannot be NULL
 * @quota:	Quota allowed in this set. Only for new set creation
 * @flags:	Special requirements
 *
 * IOASID can be limited system-wide resource that requires quota management.
 * If caller does not wish to enforce quota, use IOASID_SET_NO_QUOTA flag.
 *
 * Token will be stored in the ioasid_set returned. A reference will be taken
 * upon finding a matching set or newly created set.
 * IOASID allocation within the set and other per set operations will use
 * the retured ioasid_set *.
 *
 */
struct ioasid_set *ioasid_alloc_set(void *token, ioasid_t quota, int type)
{
	struct ioasid_set *sdata;
	unsigned long index;
	ioasid_t id;

	if (type >= IOASID_SET_TYPE_NR)
		return ERR_PTR(-EINVAL);

	/*
	 * Need to check space available if we share system-wide quota.
	 * TODO: we may need to support quota free sets in the future.
	 */
	spin_lock(&ioasid_allocator_lock);
	if (quota > ioasid_capacity_avail) {
		pr_warn("Out of IOASID capacity! ask %d, avail %d\n",
			quota, ioasid_capacity_avail);
		sdata = ERR_PTR(-ENOSPC);
		goto exit_unlock;
	}

	/*
	 * Token is only unique within its types but right now we have only
	 * mm type. If we have more token types, we have to match type as well.
	 */
	switch (type) {
	case IOASID_SET_TYPE_MM:
		/* Search existing set tokens, reject duplicates */
		xa_for_each(&ioasid_sets, index, sdata) {
			if (sdata->token == token &&
				sdata->type == IOASID_SET_TYPE_MM) {
				sdata = ERR_PTR(-EEXIST);
				goto exit_unlock;
			}
		}
		break;
	case IOASID_SET_TYPE_NULL:
		if (!token)
			break;
		fallthrough;
	default:
		pr_err("Invalid token and IOASID type\n");
		sdata = ERR_PTR(-EINVAL);
		goto exit_unlock;
	}

	/* REVISIT: may support set w/o quota, use system available */
	if (!quota) {
		sdata = ERR_PTR(-EINVAL);
		goto exit_unlock;
	}

	sdata = kzalloc(sizeof(*sdata), GFP_ATOMIC);
	if (!sdata) {
		sdata = ERR_PTR(-ENOMEM);
		goto exit_unlock;
	}

	if (xa_alloc(&ioasid_sets, &id, sdata,
			XA_LIMIT(0, ioasid_capacity_avail - quota),
			GFP_ATOMIC)) {
		kfree(sdata);
		sdata = ERR_PTR(-ENOSPC);
		goto exit_unlock;
	}

	sdata->token = token;
	sdata->type = type;
	sdata->quota = quota;
	sdata->sid = id;
	refcount_set(&sdata->ref, 1);

	/*
	 * Per set XA is used to store private IDs within the set, get ready
	 * for ioasid_set private ID and system-wide IOASID allocation
	 * results.
	 */
	xa_init_flags(&sdata->xa, XA_FLAGS_ALLOC);
	ioasid_capacity_avail -= quota;

exit_unlock:
	spin_unlock(&ioasid_allocator_lock);

	return sdata;
}
EXPORT_SYMBOL_GPL(ioasid_alloc_set);

void ioasid_set_get_locked(struct ioasid_set *set)
{
	if (WARN_ON(xa_load(&ioasid_sets, set->sid) != set)) {
		pr_warn("Invalid set data\n");
		return;
	}

	refcount_inc(&set->ref);
}
EXPORT_SYMBOL_GPL(ioasid_set_get_locked);

void ioasid_set_get(struct ioasid_set *set)
{
	spin_lock(&ioasid_allocator_lock);
	ioasid_set_get_locked(set);
	spin_unlock(&ioasid_allocator_lock);
}
EXPORT_SYMBOL_GPL(ioasid_set_get);

void ioasid_set_put_locked(struct ioasid_set *set)
{
	struct ioasid_data *entry;
	unsigned long index;

	if (WARN_ON(xa_load(&ioasid_sets, set->sid) != set)) {
		pr_warn("Invalid set data\n");
		return;
	}

	if (!refcount_dec_and_test(&set->ref)) {
		pr_debug("%s: IOASID set %d has %d users\n",
			__func__, set->sid, refcount_read(&set->ref));
		return;
	}

	/* The set is already empty, we just destroy the set. */
	if (xa_empty(&set->xa))
		goto done_destroy;

	/*
	 * Free all PASIDs from system-wide IOASID pool, all subscribers gets
	 * notified and do cleanup of their own.
	 * Note that some references of the IOASIDs within the set can still
	 * be held after the free call. This is OK in that the IOASIDs will be
	 * marked inactive, the only operations can be done is ioasid_put.
	 * No need to track IOASID set states since there is no reclaim phase.
	 */
	xa_for_each(&set->xa, index, entry) {
		ioasid_free_locked(set, index);
		/* Free from per set private pool */
		xa_erase(&set->xa, index);
	}

done_destroy:
	/* Return the quota back to system pool */
	ioasid_capacity_avail += set->quota;
	kfree_rcu(set, rcu);

	/*
	 * Token got released right away after the ioasid_set is freed.
	 * If a new set is created immediately with the newly released token,
	 * it will not allocate the same IOASIDs unless they are reclaimed.
	 */
	xa_erase(&ioasid_sets, set->sid);
}
EXPORT_SYMBOL_GPL(ioasid_set_put_locked);

/**
 * ioasid_set_put - Drop a reference to the IOASID set. Free all IOASIDs within
 *                  the set if there are no more users.
 *
 * @set:	The IOASID set ID to be freed
 *
 * If refcount drops to zero, all IOASIDs allocated within the set will be
 * freed.
 */
void ioasid_set_put(struct ioasid_set *set)
{
	spin_lock(&ioasid_allocator_lock);
	ioasid_set_put_locked(set);
	spin_unlock(&ioasid_allocator_lock);
}
EXPORT_SYMBOL_GPL(ioasid_set_put);

/**
 * ioasid_adjust_set - Adjust the quota of an IOASID set
 * @set:	IOASID set to be assigned
 * @quota:	Quota allowed in this set
 *
 * Return 0 on success. If the new quota is smaller than the number of
 * IOASIDs already allocated, -EINVAL will be returned. No change will be
 * made to the existing quota.
 */
int ioasid_adjust_set(struct ioasid_set *set, int quota)
{
	int ret = 0;

	spin_lock(&ioasid_allocator_lock);
	if (set->nr_ioasids > quota) {
		pr_err("New quota %d is smaller than outstanding IOASIDs %d\n",
			quota, set->nr_ioasids);
		ret = -EINVAL;
		goto done_unlock;
	}

	if (quota >= ioasid_capacity_avail) {
		ret = -ENOSPC;
		goto done_unlock;
	}

	/* Return the delta back to system pool */
	ioasid_capacity_avail += set->quota - quota;

	/*
	 * May have a policy to prevent giving all available IOASIDs
	 * to one set. But we don't enforce here, it should be in the
	 * upper layers.
	 */
	set->quota = quota;

done_unlock:
	spin_unlock(&ioasid_allocator_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(ioasid_adjust_set);

/**
 * ioasid_set_for_each_ioasid - Iterate over all the IOASIDs within the set
 *
 * Caller must hold a reference of the set and handles its own locking.
 */
int ioasid_set_for_each_ioasid(struct ioasid_set *set,
			       void (*fn)(ioasid_t id, void *data),
			       void *data)
{
	struct ioasid_data *entry;
	unsigned long index;
	int ret = 0;

	if (xa_empty(&set->xa)) {
		pr_warn("No IOASIDs in the set %d\n", set->sid);
		return -ENOENT;
	}

	xa_for_each(&set->xa, index, entry) {
		fn(index, data);
	}

	return ret;
}
EXPORT_SYMBOL_GPL(ioasid_set_for_each_ioasid);

/**
 * ioasid_find - Find IOASID data
 * @set: the IOASID set
 * @ioasid: the IOASID to find
 * @getter: function to call on the found object
 *
 * The optional getter function allows to take a reference to the found object
 * under the rcu lock. The function can also check if the object is still valid:
 * if @getter returns false, then the object is invalid and NULL is returned.
 *
 * If the IOASID exists, return the private pointer passed to ioasid_alloc.
 * Private data can be NULL if not set. Return an error if the IOASID is not
 * found, or if @set is not NULL and the IOASID does not belong to the set.
 */
void *ioasid_find(struct ioasid_set *set, ioasid_t ioasid,
		  bool (*getter)(void *))
{
	void *priv;
	struct ioasid_data *ioasid_data;
	struct ioasid_allocator_data *idata;

	rcu_read_lock();
	idata = rcu_dereference(active_allocator);
	ioasid_data = xa_load(&idata->xa, ioasid);
	if (!ioasid_data) {
		priv = ERR_PTR(-ENOENT);
		goto unlock;
	}
	if (set && ioasid_data->set != set) {
		/* data found but does not belong to the set */
		priv = ERR_PTR(-EACCES);
		goto unlock;
	}
	/* Now IOASID and its set is verified, we can return the private data */
	priv = rcu_dereference(ioasid_data->private);
	if (getter && !getter(priv))
		priv = NULL;
unlock:
	rcu_read_unlock();

	return priv;
}
EXPORT_SYMBOL_GPL(ioasid_find);

MODULE_AUTHOR("Jean-Philippe Brucker <jean-philippe.brucker@arm.com>");
MODULE_AUTHOR("Jacob Pan <jacob.jun.pan@linux.intel.com>");
MODULE_DESCRIPTION("IO Address Space ID (IOASID) allocator");
MODULE_LICENSE("GPL");

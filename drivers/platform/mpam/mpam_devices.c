// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2018 Arm Ltd.

#define pr_fmt(fmt) "mpam: " fmt

#include <linux/arm_mpam.h>
#include <linux/cacheinfo.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/errno.h>
#include <linux/gfp.h>
#include <linux/list.h>
#include <linux/lockdep.h>
#include <linux/mutex.h>
#include <linux/preempt.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/workqueue.h>

#include <asm/mpam.h>

#include "mpam_internal.h"

/*
 * During discovery this lock protects writers to class, components and devices.
 * Once all devices are successfully probed, the system_supports_mpam() static
 * key is enabled, and these lists become read only.
 */
static DEFINE_MUTEX(mpam_devices_lock);
/* Devices are MSCs */
static LIST_HEAD(mpam_all_devices);

/* Classes are the set of MSCs that make up components of the same type. */
LIST_HEAD(mpam_classes);

/* Hold this when registering/unregistering cpuhp callbacks */
static DEFINE_MUTEX(mpam_cpuhp_lock);
static int mpam_cpuhp_state;

struct mpam_sysprops mpam_sysprops;

/*
 * mpam is enabled once all devices have been probed from CPU online callbacks,
 * scheduled via this work_struct.
 */
static struct work_struct mpam_enable_work;

static inline u32 mpam_read_reg(struct mpam_device *dev, u16 reg)
{
	WARN_ON_ONCE(reg > SZ_MPAM_DEVICE);
	assert_spin_locked(&dev->lock);

	/*
	 * If we touch a device that isn't accessible from this CPU we may get
	 * an external-abort.
	 */
	WARN_ON_ONCE(preemptible());
	WARN_ON_ONCE(!cpumask_test_cpu(smp_processor_id(), &dev->fw_affinity));

	return readl_relaxed(dev->mapped_hwpage + reg);
}

static struct mpam_device * __init
mpam_device_alloc(struct mpam_component *comp)
{
	struct mpam_device *dev;

	lockdep_assert_held(&mpam_devices_lock);

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return ERR_PTR(-ENOMEM);

	spin_lock_init(&dev->lock);
	INIT_LIST_HEAD(&dev->comp_list);
	INIT_LIST_HEAD(&dev->glbl_list);

	dev->comp = comp;
	list_add(&dev->comp_list, &comp->devices);
	list_add(&dev->glbl_list, &mpam_all_devices);

	return dev;
}

static void mpam_devices_destroy(struct mpam_component *comp)
{
	struct mpam_device *dev, *tmp;

	lockdep_assert_held(&mpam_devices_lock);

	list_for_each_entry_safe(dev, tmp, &comp->devices, comp_list) {
		list_del(&dev->comp_list);
		list_del(&dev->glbl_list);
		kfree(dev);
	}
}

static struct mpam_component * __init mpam_component_alloc(int id)
{
	struct mpam_component *comp;

	comp = kzalloc(sizeof(*comp), GFP_KERNEL);
	if (!comp)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&comp->devices);
	//INIT_LIST_HEAD(&comp->resctrl_domain.list);
	INIT_LIST_HEAD(&comp->class_list);

	comp->comp_id = id;

	return comp;
}

struct mpam_component *mpam_component_get(struct mpam_class *class, int id,
					  bool alloc)
{
	struct mpam_component *comp;

	list_for_each_entry(comp, &class->components, class_list) {
		if (comp->comp_id == id)
			return comp;
	}

	if (!alloc)
		return ERR_PTR(-ENOENT);

	comp = mpam_component_alloc(id);
	if (IS_ERR(comp))
		return comp;

	list_add(&comp->class_list, &class->components);

	return comp;
}

/* free all components and devices of this class */
static void mpam_class_destroy(struct mpam_class *class)
{
	struct mpam_component *comp, *tmp;

	lockdep_assert_held(&mpam_devices_lock);

	list_for_each_entry_safe(comp, tmp, &class->components, class_list) {
		mpam_devices_destroy(comp);
		list_del(&comp->class_list);
		kfree(comp);
	}
}

static struct mpam_class * __init mpam_class_alloc(u8 level_idx, enum mpam_class_types type)
{
	struct mpam_class *class;

	lockdep_assert_held(&mpam_devices_lock);

	class = kzalloc(sizeof(*class), GFP_KERNEL);
	if (!class)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&class->components);
	INIT_LIST_HEAD(&class->classes_list);

	class->level = level_idx;
	class->type = type;

	list_add(&class->classes_list, &mpam_classes);

	return class;
}

static struct mpam_class * __init mpam_class_get(u8 level_idx,
						 enum mpam_class_types type,
						 bool alloc)
{
	bool found = false;
	struct mpam_class *class;

	pr_debug("mpam_class_get(%d)\n", level_idx);
	lockdep_assert_held(&mpam_devices_lock);

	list_for_each_entry(class, &mpam_classes, classes_list) {
		if (class->type == type && class->level == level_idx) {
			found = true;
			break;
		}
	}

	if (found)
		return class;

	if (!alloc)
		return ERR_PTR(-ENOENT);

	return mpam_class_alloc(level_idx, type);
}

/*
 * Create a a device with this @hwpage_address, of class type:level_idx.
 * class/component structures may be allocated.
 * Returns the new device, or an ERR_PTR().
 */
struct mpam_device * __init
__mpam_device_create(u8 level_idx, enum mpam_class_types type,
		     int component_id, const struct cpumask *fw_affinity,
		     phys_addr_t hwpage_address)
{
	struct mpam_device *dev;
	struct mpam_class *class;
	struct mpam_component *comp;

	if (!fw_affinity)
		fw_affinity = cpu_possible_mask;

	mutex_lock(&mpam_devices_lock);
	do {
		class = mpam_class_get(level_idx, type, true);
		if (IS_ERR(class)) {
			dev = (void *)class;
			break;
		}

		comp = mpam_component_get(class, component_id, true);
		if (IS_ERR(comp)) {
			dev = (void *)comp;
			break;
		}

		/*
		 * For caches we learn the affinity from the cache-id as CPUs
		 * come online. For everything else, we have to be told.
		 */
		if (type != MPAM_CLASS_CACHE)
			cpumask_or(&comp->fw_affinity, &comp->fw_affinity, fw_affinity);

		dev = mpam_device_alloc(comp);
		if (IS_ERR(dev))
			break;

		dev->fw_affinity = *fw_affinity;
		dev->hwpage_address = hwpage_address;
		dev->mapped_hwpage = ioremap(hwpage_address, SZ_MPAM_DEVICE);
		if (!dev->mapped_hwpage)
			dev = ERR_PTR(-ENOMEM);
	} while (0);
	mutex_unlock(&mpam_devices_lock);

	return dev;
}

void __init mpam_device_set_error_irq(struct mpam_device *dev, u32 irq,
				      u32 flags)
{
	spin_lock(&dev->lock);
	dev->error_irq = irq;
	dev->error_irq_flags = flags & MPAM_IRQ_FLAGS_MASK;
	spin_unlock(&dev->lock);
}

void __init mpam_device_set_overflow_irq(struct mpam_device *dev, u32 irq,
					 u32 flags)
{
	spin_lock(&dev->lock);
	dev->overflow_irq = irq;
	dev->overflow_irq_flags = flags & MPAM_IRQ_FLAGS_MASK;
	spin_unlock(&dev->lock);
}

static void mpam_probe_update_sysprops(u16 max_partid, u8 max_pmg)
{
	lockdep_assert_held(&mpam_devices_lock);

	mpam_sysprops.max_partid = min(mpam_sysprops.max_partid, max_partid);
	mpam_sysprops.max_pmg = min(mpam_sysprops.max_pmg, max_pmg);
}

static int mpam_device_probe(struct mpam_device *dev)
{
	u32 hwfeatures;
	u16 max_partid, max_pmg;

	if (mpam_read_reg(dev, MPAMF_AIDR) != MPAM_ARCHITECTURE_V1) {
		pr_err_once("device at 0x%llx does not match MPAM architecture v1.0\n",
			    dev->hwpage_address);
		return -EIO;
	}

	hwfeatures = mpam_read_reg(dev, MPAMF_IDR);
	max_partid = hwfeatures & MPAMF_IDR_PARTID_MAX_MASK;
	max_pmg = (hwfeatures & MPAMF_IDR_PMG_MAX_MASK) >> MPAMF_IDR_PMG_MAX_SHIFT;

	mpam_probe_update_sysprops(max_partid, max_pmg);

	dev->probed = true;

	return 0;
}

/*
 * Enable mpam once all devices have been probed.
 * Scheduled by mpam_discovery_complete() once all devices have been created.
 * Also scheduled when new devices are probed when new CPUs come online.
 */
static void mpam_enable(struct work_struct *work)
{
	struct mpam_device *dev;
	bool all_devices_probed = true;

	/* Have we probed all the devices? */
	mutex_lock(&mpam_devices_lock);
	list_for_each_entry(dev, &mpam_all_devices, glbl_list) {
		spin_lock(&dev->lock);
		if (!dev->probed)
			all_devices_probed = false;
		spin_unlock(&dev->lock);

		if (!all_devices_probed)
			break;
	}
	mutex_unlock(&mpam_devices_lock);

	if (!all_devices_probed)
		return;

}


int __init mpam_discovery_start(void)
{
	if (!mpam_cpus_have_feature())
		return -EOPNOTSUPP;

	mpam_sysprops.max_partid = mpam_cpu_max_partids();
	mpam_sysprops.max_pmg = mpam_cpu_max_pmgs();

	INIT_WORK(&mpam_enable_work, mpam_enable);

	return 0;
}

/*
 * Firmware didn't give us an affinity, but a cache-id, if this cpu has that
 * cache-id, update the fw_affinity for this component.
 */
static void mpam_sync_cpu_cache_component_fw_affinity(struct mpam_class *class,
						      int cpu)
{
	int cpu_cache_id;
	struct mpam_component *comp;

	lockdep_assert_held(&mpam_devices_lock);

	if (class->type != MPAM_CLASS_CACHE)
		return;

	cpu_cache_id = get_cpu_cacheinfo_id(cpu, class->level);
	comp = mpam_component_get(class, cpu_cache_id, false);

	/* This cpu does not have a component of this class */
	if (!comp)
		return;

	cpumask_set_cpu(cpu, &comp->fw_affinity);
	cpumask_set_cpu(cpu, &class->fw_affinity);
}

static int __online_devices(struct mpam_component *comp, int cpu)
{
	int err = 0;
	struct mpam_device *dev;
	bool new_device_probed = false;

	list_for_each_entry(dev, &comp->devices, comp_list) {
		if (!cpumask_test_cpu(cpu, &dev->fw_affinity))
			continue;


		spin_lock(&dev->lock);
		if (!dev->probed) {
			err = mpam_device_probe(dev);
			if (!err)
				new_device_probed = true;
		}

		cpumask_set_cpu(cpu, &dev->online_affinity);
		spin_unlock(&dev->lock);

		if (err)
			return err;
	}

	if (new_device_probed)
		return 1;

	return 0;
}

static int mpam_cpu_online(unsigned int cpu)
{
	int err = 0;
	struct mpam_class *class;
	struct mpam_component *comp;
	bool new_device_probed = false;

	mutex_lock(&mpam_devices_lock);
	list_for_each_entry(class, &mpam_classes, classes_list) {
		mpam_sync_cpu_cache_component_fw_affinity(class, cpu);

		list_for_each_entry(comp, &class->components, class_list) {
			if (!cpumask_test_cpu(cpu, &comp->fw_affinity))
				continue;

			err = __online_devices(comp, cpu);
			if (err > 0)
				new_device_probed = true;
			if (err < 0)
				break; // mpam_broken
		}
	}

	if (new_device_probed && err >= 0)
		schedule_work(&mpam_enable_work);

	mutex_unlock(&mpam_devices_lock);

	if (err < 0)
		return err;

	return 0;
}

static int mpam_cpu_offline(unsigned int cpu)
{
	struct mpam_device *dev;

	mutex_lock(&mpam_devices_lock);
	list_for_each_entry(dev, &mpam_all_devices, glbl_list)
		cpumask_clear_cpu(cpu, &dev->online_affinity);

	mutex_unlock(&mpam_devices_lock);

	return 0;
}

void __init mpam_discovery_complete(void)
{
	mutex_lock(&mpam_cpuhp_lock);
	mpam_cpuhp_state = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN,
					     "mpam:online", mpam_cpu_online,
					     mpam_cpu_offline);
	if (mpam_cpuhp_state <= 0)
		pr_err("Failed to register 'dyn' cpuhp callbacks");
	mutex_unlock(&mpam_cpuhp_lock);
}

void mpam_discovery_failed(void)
{
	struct mpam_class *class, *tmp;

	mutex_lock(&mpam_devices_lock);
	list_for_each_entry_safe(class, tmp, &mpam_classes, classes_list) {
		mpam_class_destroy(class);
		list_del(&class->classes_list);
		kfree(class);
	}
	mutex_unlock(&mpam_devices_lock);
}


// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2018 Arm Ltd.

#ifndef MPAM_INTERNAL_H
#define MPAM_INTERNAL_H

#include <linux/cpumask.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/resctrl.h>
#include <linux/sizes.h>

/*
 * An mpam_device corresponds to an MSC, an interface to a component's cache
 * or bandwidth controls. It is associated with a set of CPUs, and a component.
 * For resctrl the component is expected to be a well-known cache (e.g. L2).
 * We may have multiple interfaces per component, each for a set of CPUs that
 * share the same component.
 */
struct mpam_device
{
	/* member of mpam_component:devices */
	struct list_head        comp_list;
	struct mpam_component   *comp;

	/* member of mpam_all_devices */
	struct list_head        glbl_list;

	/* The affinity learn't from firmware */
	struct cpumask          fw_affinity;

	spinlock_t              lock;
	phys_addr_t             hwpage_address;
	void __iomem *          mapped_hwpage;

	u32                     error_irq;
	u32                     error_irq_flags;
	u32                     overflow_irq;
	u32                     overflow_irq_flags;
};

/*
 * A set of devices that share the same component. e.g. the MSCs that
 * make up the L2 cache. This may be 1:1. Exposed to user-space as a domain by
 * resctrl when the component is a well-known cache.
 */
struct mpam_component
{
	u32			comp_id;

	/* mpam_devices in this domain */
	struct list_head        devices;

	struct cpumask          fw_affinity;

	/* member of mpam_class:components */
	struct list_head        class_list;
};

/*
 * All the components of the same type at a particular level,
 * e.g. all the L2 cache components. Exposed to user-space as a resource
 * by resctrl when the component is a well-known cache. We may have additional
 * classes such as system-caches, or internal components that are not exposed.
 */
struct mpam_class
{
	/*
	 * resctrl expects to see an empty domain list if all 'those' CPUs are
	 * offline. As we can't discover the cpu affinity of 'unknown' MSCs, we
	 * need a second list.
	 * mpam_components in this class.
	 */
	struct list_head        components;

	struct cpumask          fw_affinity;

	u8			level;
	enum mpam_class_types	type;

	/* member of mpam_classes */
	struct list_head        classes_list;
};

/* List of all classes */
extern struct list_head mpam_classes;

/* Size of the memory mapped registers: 4K of feature page then 2x 4K bitmap registers */
#define SZ_MPAM_DEVICE	(3 * SZ_4K)

#endif /* MPAM_INTERNAL_H */

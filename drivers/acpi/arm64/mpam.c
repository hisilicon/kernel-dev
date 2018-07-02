// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2018 Arm Ltd.

/* Parse the MPAM ACPI table feeding the discovered nodes into the driver */

#define pr_fmt(fmt) "ACPI MPAM: " fmt

#include <linux/acpi.h>
#include <linux/arm_mpam.h>
#include <linux/cpu.h>
#include <linux/cpumask.h>

#include <asm/mpam.h>

#undef pr_debug
#define pr_debug pr_err

#define mpam_irq_flags_to_acpi(x) ((x & MPAM_IRQ_MODE_LEVEL) ? 	\
				ACPI_LEVEL_SENSITIVE : ACPI_EDGE_SENSITIVE)

static int __init acpi_mpam_parse_common(struct acpi_mpam_header *h,
					 struct mpam_device *dev)
{
	int irq, trigger, ret = 0;
	u8 irq_flags;

	if (h->overflow_interrupt) {
		irq_flags = h->overflow_flags & MPAM_IRQ_FLAGS_MASK;
		trigger = mpam_irq_flags_to_acpi(irq_flags);

		irq = acpi_register_gsi(NULL, h->overflow_interrupt, trigger,
					ACPI_ACTIVE_HIGH);
		if (irq < 0) {
			pr_err_once("Failed to register overflow interrupt with ACPI\n");
			return ret;
		}

		mpam_device_set_overflow_irq(dev, irq, irq_flags);
	}

	if (h->error_interrupt) {
		irq_flags = h->error_flags & MPAM_IRQ_FLAGS_MASK;
		trigger = mpam_irq_flags_to_acpi(irq_flags);
	
		irq = acpi_register_gsi(NULL, h->error_interrupt, trigger,
					ACPI_ACTIVE_HIGH);
		if (irq < 0) {
			pr_err_once("Failed to register error interrupt with ACPI\n");
			return ret;
		}

		mpam_device_set_error_irq(dev, irq, irq_flags);
	}

	return ret;
}

static int __init acpi_mpam_parse_unknown(struct acpi_mpam_header *h)
{
	struct mpam_device *dev;
	struct acpi_mpam_node_memory *node = (struct acpi_mpam_node_memory *)h;

	dev = mpam_device_create(1, 1, NULL, node->header.base_address);
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	return acpi_mpam_parse_common(h, dev);
}

static int __init acpi_mpam_parse_memory(struct acpi_mpam_header *h)
{
	int nid;
	struct mpam_device *dev;
	struct acpi_mpam_node_memory *node = (struct acpi_mpam_node_memory *)h;

	nid = acpi_map_pxm_to_node(node->proximity_domain);
	dev = mpam_device_create_memory(nid, node->header.base_address);
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	return acpi_mpam_parse_common(h, dev);
}

static int __init acpi_mpam_parse_cache(struct acpi_mpam_header *h,
					struct acpi_table_header *pptt)
{
	struct cacheinfo *ci;
	struct mpam_device *dev;
	int cache_id, cache_level;
	struct acpi_pptt_cache *pptt_cache;
	struct acpi_pptt_processor *pptt_cpu_node;
	struct acpi_mpam_node_cache *node = (struct acpi_mpam_node_cache *)h;

	if (!pptt) {
		pr_err("No PPTT table found, MPAM cannot be configured\n");
		return -EINVAL;
	}

	pptt_cache = acpi_pptt_validate_cache_node(pptt, node->PPTT_ref);
	if (!pptt_cache) {
		pr_err("Broken PPTT reference in the MPAM table\n");
		return -EINVAL;
	}
	pr_debug("Got PPTT cache 0x%px\n", pptt_cache);

	/*
	 * ... but we actually need a cpu_node, as a pointer to the PPTT cache
	 * description isn't unique.
	 */
	pptt_cpu_node = acpi_pptt_find_cache_backwards(pptt, pptt_cache);


	/*
	 * We want the cache id and level ... quickest thing to do is to pull
	 * the information out of cacheinfo, which is what it is later matched
	 * against. The right thing to do is breed more PPTT helpers to
	 * generate the same values, which is harder.
	 *
	 * Taking this shortcut forces all CPUs to be online at this point,
	 * which is something we don't require in the mpam-driver proper.
	 */
	cpus_read_lock();
	ci = cacheinfo_shared_cpu_map_search(pptt_cpu_node);
	if (!ci) {
		pr_err_once("No CPU has cache with PPTT reference 0x%x",
			    node->PPTT_ref);
		pr_err_once("All CPUs must be online to probe mpam.\n");
		cpus_read_unlock();
		return -ENODEV;
	}

	/* caches must have an id */
	if (!(ci->attributes & CACHE_ID)) {
		cpus_read_unlock();
		return -EINVAL;
	}

	cache_id = ci->id;
	cache_level = ci->level;

	ci = NULL;
	cpus_read_unlock();

	/*
	 * We could walk the PPTT to generate the affinity of this device,
	 * but the PPTT_ref points at the cache, we'd also need a point in
	 * the cpu topology in case they aren't the same.
	 */
	dev = mpam_device_create_cache(cache_level, cache_id, NULL,
				       node->header.base_address);
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	return acpi_mpam_parse_common(h, dev);
}

static int __init acpi_mpam_parse_table(struct acpi_table_header *table,
				        struct acpi_table_header *pptt)
{
	char *table_offset = (char *)(table + 1);
	char *table_end = (char *)table + table->length;
	struct acpi_mpam_header *node_hdr;
	int ret = 0;

	ret = mpam_discovery_start();
	if (ret)
		return ret;

	node_hdr = (struct acpi_mpam_header *)table_offset;
	while (table_offset < table_end) {
		switch (node_hdr->type) {

		case ACPI_MPAM_TYPE_CACHE:
			ret = acpi_mpam_parse_cache(node_hdr, pptt);
			break;
		case ACPI_MPAM_TYPE_MEMORY:
			ret = acpi_mpam_parse_memory(node_hdr);
			break;
		default:
			pr_warn_once("Unknown node type %u offset %ld.",
				     node_hdr->type,
				     (table_offset-(char *)table));
			/* fall through */
		case ACPI_MPAM_TYPE_SMMU:
			/* not yet supported */
			/* fall through */
		case ACPI_MPAM_TYPE_UNKNOWN:
			ret = acpi_mpam_parse_unknown(node_hdr);
			break;
		}
		if (ret)
			break;

		table_offset += node_hdr->length;
		node_hdr = (struct acpi_mpam_header *)table_offset;
	}

	if (ret) {
		pr_err("discovery failed: %d\n", ret);
		mpam_discovery_failed();
	} else {
		mpam_discovery_complete();
	}

	return ret;
}

int __init acpi_mpam_parse(void)
{
	struct acpi_table_header *mpam, *pptt;
	acpi_status status;
	int ret;

	if (acpi_disabled || !mpam_cpus_have_feature())
		return 0;

	status = acpi_get_table(ACPI_SIG_MPAM, 0, &mpam);
	if (ACPI_FAILURE(status))
		return -ENOENT;

	/* PPTT is optional, there may be no mpam cache controls */
	acpi_get_table(ACPI_SIG_PPTT, 0, &pptt);
	if (ACPI_FAILURE(status))
		pptt = NULL;

	ret = acpi_mpam_parse_table(mpam, pptt);
	acpi_put_table(pptt);
	acpi_put_table(mpam);

	return ret;
}

/*
 * We want to run after cacheinfo_sysfs_init() has caused the cacheinfo
 * structures to be populated. That runs as a device_initcall.
 */
device_initcall_sync(acpi_mpam_parse);

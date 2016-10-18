/*
 * ARM Specific GTDT table Support
 *
 * Copyright (C) 2016, Linaro Ltd.
 * Author: Daniel Lezcano <daniel.lezcano@linaro.org>
 *         Fu Wei <fu.wei@linaro.org>
 *         Hanjun Guo <hanjun.guo@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/acpi.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <clocksource/arm_arch_timer.h>

#undef pr_fmt
#define pr_fmt(fmt) "ACPI GTDT: " fmt

struct acpi_gtdt_descriptor {
	struct acpi_table_gtdt *gtdt;
	void *platform_timer_start;
	void *gtdt_end;
};

static struct acpi_gtdt_descriptor acpi_gtdt_desc __initdata;

static inline void *next_platform_timer(void *platform_timer)
{
	struct acpi_gtdt_header *gh = platform_timer;

	platform_timer += gh->length;
	if (platform_timer < acpi_gtdt_desc.gtdt_end)
		return platform_timer;

	return NULL;
}

#define for_each_platform_timer(_g)				\
	for (_g = acpi_gtdt_desc.platform_timer_start; _g;	\
	     _g = next_platform_timer(_g))

static inline bool is_timer_block(void *platform_timer)
{
	struct acpi_gtdt_header *gh = platform_timer;

	return gh->type == ACPI_GTDT_TYPE_TIMER_BLOCK;
}

static inline bool is_watchdog(void *platform_timer)
{
	struct acpi_gtdt_header *gh = platform_timer;

	return gh->type == ACPI_GTDT_TYPE_WATCHDOG;
}

static int __init map_gt_gsi(u32 interrupt, u32 flags)
{
	int trigger, polarity;

	if (!interrupt)
		return 0;

	trigger = (flags & ACPI_GTDT_INTERRUPT_MODE) ? ACPI_EDGE_SENSITIVE
			: ACPI_LEVEL_SENSITIVE;

	polarity = (flags & ACPI_GTDT_INTERRUPT_POLARITY) ? ACPI_ACTIVE_LOW
			: ACPI_ACTIVE_HIGH;

	return acpi_register_gsi(NULL, interrupt, trigger, polarity);
}

/*
 * Map the PPIs of per-cpu arch_timer.
 * @type: the type of PPI
 * Returns 0 if error.
 */
int __init acpi_gtdt_map_ppi(int type)
{
	struct acpi_table_gtdt *gtdt = acpi_gtdt_desc.gtdt;

	switch (type) {
	case PHYS_SECURE_PPI:
		return map_gt_gsi(gtdt->secure_el1_interrupt,
				  gtdt->secure_el1_flags);
	case PHYS_NONSECURE_PPI:
		return map_gt_gsi(gtdt->non_secure_el1_interrupt,
				  gtdt->non_secure_el1_flags);
	case VIRT_PPI:
		return map_gt_gsi(gtdt->virtual_timer_interrupt,
				  gtdt->virtual_timer_flags);

	case HYP_PPI:
		return map_gt_gsi(gtdt->non_secure_el2_interrupt,
				  gtdt->non_secure_el2_flags);
	default:
		pr_err("Failed to map timer interrupt: invalid type.\n");
	}

	return 0;
}

/*
 * acpi_gtdt_c3stop - got c3stop info from GTDT
 *
 * Returns 1 if the timer is powered in deep idle state, 0 otherwise.
 */
bool __init acpi_gtdt_c3stop(void)
{
	struct acpi_table_gtdt *gtdt = acpi_gtdt_desc.gtdt;

	return !(gtdt->non_secure_el1_flags & ACPI_GTDT_ALWAYS_ON);
}

/*
 * Get some basic info from GTDT table, and init the global variables above
 * for all timers initialization of Generic Timer.
 * This function does some validation on GTDT table.
 */
int __init acpi_gtdt_init(struct acpi_table_header *table)
{
	void *start;
	struct acpi_table_gtdt *gtdt;

	gtdt = container_of(table, struct acpi_table_gtdt, header);

	acpi_gtdt_desc.gtdt = gtdt;
	acpi_gtdt_desc.gtdt_end = (void *)table + table->length;

	if (table->revision < 2) {
		pr_debug("Revision:%d doesn't support Platform Timers.\n",
			 table->revision);
		return 0;
	}

	if (!gtdt->platform_timer_count) {
		pr_debug("No Platform Timer.\n");
		return 0;
	}

	start = (void *)gtdt + gtdt->platform_timer_offset;
	if (start < (void *)table + sizeof(struct acpi_table_gtdt)) {
		pr_err(FW_BUG "Failed to retrieve timer info from firmware: invalid data.\n");
		return -EINVAL;
	}
	acpi_gtdt_desc.platform_timer_start = start;

	return gtdt->platform_timer_count;
}

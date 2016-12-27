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

/**
 * struct acpi_gtdt_descriptor - Store the key info of GTDT for all functions
 * @gtdt:	The pointer to the struct acpi_table_gtdt of GTDT table.
 * @gtdt_end:	The pointer to the end of GTDT table.
 * @platform_timer:	The pointer to the start of Platform Timer Structure
 *
 * The struct store the key info of GTDT table, it should be initialized by
 * acpi_gtdt_init.
 */
struct acpi_gtdt_descriptor {
	struct acpi_table_gtdt *gtdt;
	void *gtdt_end;
	void *platform_timer;
};

static struct acpi_gtdt_descriptor acpi_gtdt_desc __initdata;

static int __init map_gt_gsi(u32 interrupt, u32 flags)
{
	int trigger, polarity;

	trigger = (flags & ACPI_GTDT_INTERRUPT_MODE) ? ACPI_EDGE_SENSITIVE
			: ACPI_LEVEL_SENSITIVE;

	polarity = (flags & ACPI_GTDT_INTERRUPT_POLARITY) ? ACPI_ACTIVE_LOW
			: ACPI_ACTIVE_HIGH;

	return acpi_register_gsi(NULL, interrupt, trigger, polarity);
}

/**
 * acpi_gtdt_map_ppi() - Map the PPIs of per-cpu arch_timer.
 * @type:	the type of PPI.
 *
 * Note: Linux on arm64 isn't supported on the secure side.
 * So we only handle the non-secure timer PPIs,
 * ARCH_TIMER_PHYS_SECURE_PPI is treated as invalid type.
 *
 * Return: the mapped PPI value, 0 if error.
 */
int __init acpi_gtdt_map_ppi(int type)
{
	struct acpi_table_gtdt *gtdt = acpi_gtdt_desc.gtdt;

	switch (type) {
	case ARCH_TIMER_PHYS_NONSECURE_PPI:
		return map_gt_gsi(gtdt->non_secure_el1_interrupt,
				  gtdt->non_secure_el1_flags);
	case ARCH_TIMER_VIRT_PPI:
		return map_gt_gsi(gtdt->virtual_timer_interrupt,
				  gtdt->virtual_timer_flags);

	case ARCH_TIMER_HYP_PPI:
		return map_gt_gsi(gtdt->non_secure_el2_interrupt,
				  gtdt->non_secure_el2_flags);
	default:
		pr_err("Failed to map timer interrupt: invalid type.\n");
	}

	return 0;
}

/**
 * acpi_gtdt_c3stop() - Got c3stop info from GTDT according to the type of PPI.
 * @type:	the type of PPI.
 *
 * Return: 1 if the timer can be in deep idle state, 0 otherwise.
 */
bool __init acpi_gtdt_c3stop(int type)
{
	struct acpi_table_gtdt *gtdt = acpi_gtdt_desc.gtdt;

	switch (type) {
	case ARCH_TIMER_PHYS_NONSECURE_PPI:
		return !(gtdt->non_secure_el1_flags & ACPI_GTDT_ALWAYS_ON);

	case ARCH_TIMER_VIRT_PPI:
		return !(gtdt->virtual_timer_flags & ACPI_GTDT_ALWAYS_ON);

	case ARCH_TIMER_HYP_PPI:
		return !(gtdt->non_secure_el2_flags & ACPI_GTDT_ALWAYS_ON);

	default:
		pr_err("Failed to get c3stop info: invalid type.\n");
	}

	return 0;
}

/**
 * acpi_gtdt_init() - Get the info of GTDT table to prepare for further init.
 * @table:	The pointer to GTDT table.
 * @platform_timer_count:	The pointer of int variate for returning the
 *				number of platform timers. It can be NULL, if
 *				driver don't need this info.
 *
 * Return: 0 if success, -EINVAL if error.
 */
int __init acpi_gtdt_init(struct acpi_table_header *table,
			  int *platform_timer_count)
{
	int ret = 0;
	int timer_count = 0;
	void *platform_timer = NULL;
	struct acpi_table_gtdt *gtdt;

	gtdt = container_of(table, struct acpi_table_gtdt, header);
	acpi_gtdt_desc.gtdt = gtdt;
	acpi_gtdt_desc.gtdt_end = (void *)table + table->length;

	if (table->revision < 2)
		pr_debug("Revision:%d doesn't support Platform Timers.\n",
			 table->revision);
	else if (!gtdt->platform_timer_count)
		pr_debug("No Platform Timer.\n");
	else
		timer_count = gtdt->platform_timer_count;

	if (timer_count) {
		platform_timer = (void *)gtdt + gtdt->platform_timer_offset;
		if (platform_timer < (void *)table +
				     sizeof(struct acpi_table_gtdt)) {
			pr_err(FW_BUG "invalid timer data.\n");
			timer_count = 0;
			platform_timer = NULL;
			ret = -EINVAL;
		}
	}

	acpi_gtdt_desc.platform_timer = platform_timer;
	if (platform_timer_count)
		*platform_timer_count = timer_count;

	return ret;
}

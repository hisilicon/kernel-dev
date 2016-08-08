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
#include <linux/platform_device.h>

#include <clocksource/arm_arch_timer.h>

#undef pr_fmt
#define pr_fmt(fmt) "ACPI GTDT: " fmt

typedef struct {
	struct acpi_table_gtdt *gtdt;
	void *platform_timer_start;
	void *gtdt_end;
} acpi_gtdt_desc_t;

static acpi_gtdt_desc_t acpi_gtdt_desc __initdata;

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

static int __init gtdt_parse_gt_block(struct acpi_gtdt_timer_block *block,
				      struct gt_block_data *data)
{
	struct acpi_gtdt_timer_entry *frame;
	int i;

	if (!block || !data)
		return -EINVAL;

	if (!block->block_address || !block->timer_count)
		return -EINVAL;

	data->cntctlbase_phy = (phys_addr_t)block->block_address;
	data->timer_count = block->timer_count;

	frame = (void *)block + block->timer_offset;
	if (frame + block->timer_count != (void *)block + block->header.length)
		return -EINVAL;

	/*
	 * Get the GT timer Frame data for every GT Block Timer
	 */
	for (i = 0; i < block->timer_count; i++, frame++) {
		if (!frame->base_address || !frame->timer_interrupt)
			return -EINVAL;

		data->timer[i].irq = map_gt_gsi(frame->timer_interrupt,
						frame->timer_flags);
		if (data->timer[i].irq <= 0)
			return -EINVAL;

		if (frame->virtual_timer_interrupt) {
			data->timer[i].virtual_irq =
				map_gt_gsi(frame->virtual_timer_interrupt,
					   frame->virtual_timer_flags);
			if (data->timer[i].virtual_irq <= 0)
				return -EINVAL;
		}

		data->timer[i].frame_nr = frame->frame_number;
		data->timer[i].cntbase_phy = frame->base_address;
	}

	return 0;
}

/*
 * Get the GT block info for memory-mapped timer from GTDT table.
 */
int __init gtdt_arch_timer_mem_init(struct gt_block_data *data)
{
	void *platform_timer;
	int index = 0;
	int ret;

	for_each_platform_timer(platform_timer) {
		if (!is_timer_block(platform_timer))
			continue;
		ret = gtdt_parse_gt_block(platform_timer, data + index);
		if (ret)
			return ret;
		index++;
	}

	if (index)
		pr_info("found %d memory-mapped timer block(s).\n", index);

	return index;
}

/*
 * Initialize a SBSA generic Watchdog platform device info from GTDT
 */
static int __init gtdt_import_sbsa_gwdt(struct acpi_gtdt_watchdog *wd,
					int index)
{
	struct platform_device *pdev;
	int irq = map_gt_gsi(wd->timer_interrupt, wd->timer_flags);
	int no_irq = 1;

	/*
	 * According to SBSA specification the size of refresh and control
	 * frames of SBSA Generic Watchdog is SZ_4K(Offset 0x000 – 0xFFF).
	 */
	struct resource res[] = {
		DEFINE_RES_MEM(wd->control_frame_address, SZ_4K),
		DEFINE_RES_MEM(wd->refresh_frame_address, SZ_4K),
		DEFINE_RES_IRQ(irq),
	};

	pr_debug("found a Watchdog (0x%llx/0x%llx gsi:%u flags:0x%x).\n",
		 wd->refresh_frame_address, wd->control_frame_address,
		 wd->timer_interrupt, wd->timer_flags);

	if (!(wd->refresh_frame_address && wd->control_frame_address)) {
		pr_err(FW_BUG "failed to get the Watchdog base address.\n");
		return -EINVAL;
	}

	if (!wd->timer_interrupt)
		pr_warn(FW_BUG "failed to get the Watchdog interrupt.\n");
	else if (irq <= 0)
		pr_warn("failed to map the Watchdog interrupt.\n");
	else
		no_irq = 0;

	/*
	 * Add a platform device named "sbsa-gwdt" to match the platform driver.
	 * "sbsa-gwdt": SBSA(Server Base System Architecture) Generic Watchdog
	 * The platform driver (like drivers/watchdog/sbsa_gwdt.c)can get device
	 * info below by matching this name.
	 */
	pdev = platform_device_register_simple("sbsa-gwdt", index, res,
					       ARRAY_SIZE(res) - no_irq);
	if (IS_ERR(pdev)) {
		acpi_unregister_gsi(wd->timer_interrupt);
		return PTR_ERR(pdev);
	}

	return 0;
}

static int __init gtdt_sbsa_gwdt_init(void)
{
	struct acpi_table_header *table;
	void *platform_timer;
	int index = 0;
	int ret;

	if (acpi_disabled)
		return 0;

	if (ACPI_FAILURE(acpi_get_table(ACPI_SIG_GTDT, 0, &table)))
		return -EINVAL;

	ret = acpi_gtdt_init(table);
	if (ret <= 0)
		return ret;

	for_each_platform_timer(platform_timer) {
		if (!is_watchdog(platform_timer))
			continue;
		ret = gtdt_import_sbsa_gwdt(platform_timer, index);
		if (ret)
			break;
		index++;
	}

	if (index)
		pr_info("found %d SBSA generic Watchdog(s).\n", index);

	return ret;
}

device_initcall(gtdt_sbsa_gwdt_init);

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

static inline void *next_platform_timer(void *platform_timer)
{
	struct acpi_gtdt_header *gh = platform_timer;

	platform_timer += gh->length;
	if (platform_timer < acpi_gtdt_desc.gtdt_end)
		return platform_timer;

	return NULL;
}

#define for_each_platform_timer(_g)				\
	for (_g = acpi_gtdt_desc.platform_timer; _g;	\
	     _g = next_platform_timer(_g))

static inline bool is_timer_block(void *platform_timer)
{
	struct acpi_gtdt_header *gh = platform_timer;

	return gh->type == ACPI_GTDT_TYPE_TIMER_BLOCK;
}

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

static int __init gtdt_parse_timer_block(struct acpi_gtdt_timer_block *block,
					 struct arch_timer_mem *data)
{
	int i;
	struct acpi_gtdt_timer_entry *frame;

	if (!block->timer_count) {
		pr_err(FW_BUG "GT block present, but frame count is zero.");
		return -ENODEV;
	}

	if (block->timer_count > ARCH_TIMER_MEM_MAX_FRAMES) {
		pr_err(FW_BUG "GT block lists %d frames, ACPI spec only allows 8\n",
		       block->timer_count);
		return -EINVAL;
	}

	data->cntctlbase = (phys_addr_t)block->block_address;
	/*
	 * According to "Table * CNTCTLBase memory map" of
	 * <ARM Architecture Reference Manual> for ARMv8,
	 * The size of the CNTCTLBase frame is 4KB(Offset 0x000 – 0xFFC).
	 */
	data->size = SZ_4K;
	data->num_frames = block->timer_count;

	frame = (void *)block + block->timer_offset;
	if (frame + block->timer_count != (void *)block + block->header.length)
		return -EINVAL;

	/*
	 * Get the GT timer Frame data for every GT Block Timer
	 */
	for (i = 0; i < block->timer_count; i++, frame++) {
		if (!frame->base_address || !frame->timer_interrupt)
			return -EINVAL;

		data->frame[i].phys_irq = map_gt_gsi(frame->timer_interrupt,
						     frame->timer_flags);
		if (data->frame[i].phys_irq <= 0) {
			pr_warn("failed to map physical timer irq in frame %d.\n",
				i);
			return -EINVAL;
		}

		data->frame[i].virt_irq =
			map_gt_gsi(frame->virtual_timer_interrupt,
				   frame->virtual_timer_flags);
		if (data->frame[i].virt_irq <= 0) {
			pr_warn("failed to map virtual timer irq in frame %d.\n",
				i);
			acpi_unregister_gsi(frame->timer_interrupt);
			return -EINVAL;
		}

		data->frame[i].frame_nr = frame->frame_number;
		data->frame[i].cntbase = frame->base_address;
		/*
		 * According to "Table * CNTBaseN memory map" of
		 * <ARM Architecture Reference Manual> for ARMv8,
		 * The size of the CNTBaseN frame is 4KB(Offset 0x000 – 0xFFC).
		 */
		data->frame[i].size = SZ_4K;
	}

	return 0;
}

/**
 * acpi_arch_timer_mem_init() - Get the info of all GT blocks in GTDT table.
 * @data:	the pointer to the array of struct arch_timer_mem for returning
 *		the result of parsing. The element number of this array should
 *		be platform_timer_count(the total number of platform timers).
 * @count:	The pointer of int variate for returning the number of GT
 *		blocks we have parsed.
 *
 * Return: 0 if success, -EINVAL/-ENODEV if error.
 */
int __init acpi_arch_timer_mem_init(struct arch_timer_mem *data,
				    int *timer_count)
{
	int ret;
	void *platform_timer;

	*timer_count = 0;
	for_each_platform_timer(platform_timer) {
		if (is_timer_block(platform_timer)) {
			ret = gtdt_parse_timer_block(platform_timer, data);
			if (ret)
				return ret;
			data++;
			(*timer_count)++;
		}
	}

	if (*timer_count)
		pr_info("found %d memory-mapped timer block(s).\n",
			*timer_count);

	return 0;
}

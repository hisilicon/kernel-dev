/*
 * ARM ACPI PMU support
 *
 * Copyright (C) 2015 Red Hat Inc.
 * Author: Mark Salter <msalter@redhat.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 */

#include <asm/cpu.h>
#include <linux/acpi.h>
#include <linux/irq.h>

struct pmu_irq {
	int  gsi;
	int  trigger;
	bool used;
};

static struct pmu_irq pmu_irqs[NR_CPUS];

/*
 * Called from acpi_map_gic_cpu_interface()'s MADT parsing during boot.
 * This routine saves off the GSI's and their trigger state for use when we are
 * ready to build the PMU platform device.
 */
void __init arm_pmu_parse_acpi(int cpu, struct acpi_madt_generic_interrupt *gic)
{
	pmu_irqs[cpu].gsi = gic->performance_interrupt;
	if (gic->flags & ACPI_MADT_PERFORMANCE_IRQ_MODE)
		pmu_irqs[cpu].trigger = ACPI_EDGE_SENSITIVE;
	else
		pmu_irqs[cpu].trigger = ACPI_LEVEL_SENSITIVE;
}

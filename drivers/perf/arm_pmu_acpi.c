/*
 * ARM ACPI PMU support
 *
 * Copyright (C) 2015 Red Hat Inc.
 * Copyright (C) 2016 ARM Ltd.
 * Author: Mark Salter <msalter@redhat.com>
 *	   Jeremy Linton <jeremy.linton@arm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 *
 */

#define pr_fmt(fmt) "ACPI-PMU: " fmt

#include <asm/cpu.h>
#include <linux/acpi.h>
#include <linux/irq.h>
#include <linux/irqdesc.h>
#include <linux/list.h>
#include <linux/perf/arm_pmu.h>
#include <linux/platform_device.h>

struct pmu_irq {
	int  gsi;
	int  trigger;
	int  irq;
	bool used;
};

struct pmu_types {
	struct list_head list;
	int		 cpu_type;
	int		 cpu_count;
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

static void __init arm_pmu_acpi_handle_alloc_failure(struct list_head *pmus)
{
	int i;
	struct pmu_types *pmu, *safe_temp;

	list_for_each_entry_safe(pmu, safe_temp, pmus, list) {
		list_del(&pmu->list);
		kfree(pmu);
	}

	for_each_possible_cpu(i)
		if (pmu_irqs[i].irq > 0)
			acpi_unregister_gsi(pmu_irqs[i].gsi);
}

/*
 * Count number and type of CPU cores in the system. Returns the number
 * of "unused" MADT entries we could not associate with a PMU. This can
 * be the result of CPU's not being online,  or errors in the MADT.
 * Under normal circumstances this will be 0.
 */
static int __init arm_pmu_acpi_determine_cpu_types(struct list_head *pmus)
{
	int i;
	int unused_madt_entries = 0;

	for_each_possible_cpu(i) {
		u32 reg_midr = read_specific_cpuid(i);
		struct pmu_types *pmu;

		/*
		 * Ignore GSI registration failure for now, as
		 * some of the MADT entries may not be used.
		 */
		pmu_irqs[i].irq = acpi_register_gsi(NULL, pmu_irqs[i].gsi,
						    pmu_irqs[i].trigger,
						    ACPI_ACTIVE_HIGH);
		/* likely not online */
		if (reg_midr == 0) {
			unused_madt_entries++;
			continue;
		}

		list_for_each_entry(pmu, pmus, list) {
			if (pmu->cpu_type == reg_midr) {
				pmu->cpu_count++;
				break;
			}
		}

		/* we didn't find the CPU type, add an entry to identify it */
		if (&pmu->list == pmus) {
			pmu = kzalloc(sizeof(struct pmu_types), GFP_KERNEL);
			if (!pmu) {
				pr_err("Unable to allocate pmu_types\n");
				arm_pmu_acpi_handle_alloc_failure(pmus);
				break;
			}
			pmu->cpu_type = reg_midr;
			pmu->cpu_count++;
			list_add_tail(&pmu->list, pmus);
		}
	}

	return unused_madt_entries;
}

static int __init arm_pmu_acpi_register_device(int count, struct resource *res,
					       int cpu_id)
{
	struct platform_device *pdev;
	int err = -ENOMEM;

	pdev = platform_device_alloc(ARMV8_PMU_PDEV_NAME, cpu_id);
	if (pdev) {
		err = platform_device_add_resources(pdev, res, count);
		if (!err)
			err = platform_device_add(pdev);
		if (err) {
			pr_warn("Unable to register PMU device\n");
			platform_device_put(pdev);
		}
	} else {
	    pr_warn("Unable to allocate platform device\n");
	}

	return err;
}

static void __init arm_pmu_acpi_unregister_pmu_gsi(int cpu_id)
{
	int i;

	for_each_possible_cpu(i) {

		if (read_specific_cpuid(i) == cpu_id) {
			pmu_irqs[i].used = false;
			if (pmu_irqs[i].irq > 0)
				acpi_unregister_gsi(pmu_irqs[i].gsi);
			pmu_irqs[i].gsi = -ENODEV;
		}
	}
}

/*
 * Registers the group of PMU interfaces which correspond to the 'cpu_id'.
 * This group utilizes 'count' resources in the 'res'.
 */
static int __init arm_pmu_acpi_register_pmu(int count, struct resource *res,
					    int cpu_id)
{
	int err;

	err = arm_pmu_acpi_register_device(count, res, cpu_id);

	/* unmark and unregister GSIs for this PMU */
	if (err)
		arm_pmu_acpi_unregister_pmu_gsi(cpu_id);

	return err;
}

int arm_pmu_acpi_retrieve_irq(struct resource *res, int cpu)
{
	int irq = -ENODEV;

	if (pmu_irqs[cpu].used) {
		pr_info("CPU %d's interrupt is already used\n", cpu);
	} else {
		pmu_irqs[cpu].used = true;
		res->start = pmu_irqs[cpu].irq;
		res->end = pmu_irqs[cpu].irq;
		res->flags = IORESOURCE_IRQ;
		if (pmu_irqs[cpu].trigger == ACPI_EDGE_SENSITIVE)
			res->flags |= IORESOURCE_IRQ_HIGHEDGE;
		else
			res->flags |= IORESOURCE_IRQ_HIGHLEVEL;
	}
	return irq;
}

/*
 * For the given cpu/pmu type, walk all known GSIs, register them, and add
 * them to the resource structure. Return the number of GSI's contained
 * in the res structure, and the id of the last CPU/PMU we added.
 */
static int __init arm_pmu_acpi_gsi_res(struct pmu_types *pmus,
				       struct resource *res)
{
	int i, count;

	/* lets group all the PMU's from similar CPU's together */
	count = 0;
	for_each_possible_cpu(i) {
		u32 reg_midr = read_specific_cpuid(i);

		if (pmus->cpu_type == reg_midr) {
			if ((pmu_irqs[i].gsi == 0) && (reg_midr != 0))
				continue;

			/* likely not online */
			if (!reg_midr)
				continue;

			arm_pmu_acpi_retrieve_irq(&res[count], i);
			count++;
		}
	}
	return count;
}

static int __init pmu_acpi_register(struct pmu_types *pmu)
{
	int count;
	int err = -ENOMEM;
	struct resource	*res;

	res = kcalloc(pmu->cpu_count, sizeof(struct resource), GFP_KERNEL);

	/* for a given PMU type, collect all the GSIs. */
	if (res) {
		count = arm_pmu_acpi_gsi_res(pmu, res);
		/* register this set of interrupts with a new PMU device */
		err = arm_pmu_acpi_register_pmu(pmu->cpu_count, res,
						pmu->cpu_type);
		if (!err)
			pr_info("Register %d devices for %X\n", count,
				pmu->cpu_type);
		kfree(res);
	} else {
		pr_warn("PMU unable to allocate interrupt resource\n");
		arm_pmu_acpi_unregister_pmu_gsi(pmu->cpu_type);
	}
	return err;
}

static int __init pmu_acpi_init(void)
{
	struct pmu_types *pmu, *safe_temp;
	int unused_madt_entries;
	LIST_HEAD(pmus);

	if (acpi_disabled)
		return 0;

	unused_madt_entries = arm_pmu_acpi_determine_cpu_types(&pmus);

	list_for_each_entry_safe(pmu, safe_temp, &pmus, list) {
		pmu->cpu_count += unused_madt_entries;
		pmu_acpi_register(pmu);

		list_del(&pmu->list);
		kfree(pmu);
	}

	return 0;
}

arch_initcall(pmu_acpi_init);

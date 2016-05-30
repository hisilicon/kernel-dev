/*
 * Copyright (C) 2016 Broadcom
 *	Author: Jayachandran C <jchandra@broadcom.com>
 * Copyright (C) 2016 Semihalf
 * 	Author: Tomasz Nowicki <tn@semihalf.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation (the "GPL").
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 (GPLv2) for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 (GPLv2) along with this source code.
 */

#define pr_fmt(fmt) "ACPI: " fmt

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/pci-acpi.h>

/* Root pointer to the mapped MCFG table */
static struct acpi_table_mcfg *mcfg_table;
static int mcfg_entries;

int pci_mcfg_lookup(struct acpi_pci_root *root)
{
	struct acpi_mcfg_allocation *mptr, *entry = NULL;
	struct resource *bus_res = &root->secondary;
	int i;

	if (mcfg_table) {
		mptr = (struct acpi_mcfg_allocation *) &mcfg_table[1];
		for (i = 0; i < mcfg_entries && !entry; i++, mptr++)
			if (mptr->pci_segment == root->segment &&
			    mptr->start_bus_number == bus_res->start)
				entry = mptr;
	}

	/* not found, use _CBA if available, else error */
	if (!entry) {
		if (root->mcfg_addr)
			return root->mcfg_addr;
		pr_err("%04x:%pR MCFG lookup failed\n", root->segment, bus_res);
		return -ENOENT;
	} else if (root->mcfg_addr && entry->address != root->mcfg_addr) {
		pr_warn("%04x:%pR CBA %pa != MCFG %lx, using CBA\n",
			root->segment, bus_res, &root->mcfg_addr,
			(unsigned long)entry->address);
		return 0;
	}

	/* found matching entry, bus range check */
	if (entry->end_bus_number != bus_res->end) {
		resource_size_t bus_end = min_t(resource_size_t,
					entry->end_bus_number, bus_res->end);
		pr_warn("%04x:%pR bus end mismatch, using %02lx\n",
			root->segment, bus_res, (unsigned long)bus_end);
		bus_res->end = bus_end;
	}

	if (!root->mcfg_addr)
		root->mcfg_addr = entry->address;
	return 0;
}

static __init int pci_mcfg_parse(struct acpi_table_header *header)
{
	if (header->length < sizeof(struct acpi_table_mcfg))
		return -EINVAL;

	mcfg_entries = (header->length - sizeof(struct acpi_table_mcfg)) /
					sizeof(struct acpi_mcfg_allocation);
	if (mcfg_entries == 0) {
		pr_err("MCFG has no entries\n");
		return -EINVAL;
	}

	mcfg_table = (struct acpi_table_mcfg *)header;
	pr_info("MCFG table detected, %d entries\n", mcfg_entries);
	return 0;
}

/* Interface called by ACPI - parse and save MCFG table */
void __init pci_mmcfg_late_init(void)
{
	int err = acpi_table_parse(ACPI_SIG_MCFG, pci_mcfg_parse);
	if (err)
		pr_err("Failed to parse MCFG (%d)\n", err);
}

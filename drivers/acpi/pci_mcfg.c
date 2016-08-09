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
#include <linux/pci-ecam.h>

/* Structure to hold entries from the MCFG table */
struct mcfg_entry {
	struct list_head	list;
	phys_addr_t		addr;
	u16			segment;
	u8			bus_start;
	u8			bus_end;
};

/* List to save MCFG entries */
static LIST_HEAD(pci_mcfg_list);

phys_addr_t pci_mcfg_lookup(u16 seg, struct resource *bus_res)
{
	struct mcfg_entry *e;

	/*
	 * We expect exact match, unless MCFG entry end bus covers more than
	 * specified by caller.
	 */
	list_for_each_entry(e, &pci_mcfg_list, list) {
		if (e->segment == seg && e->bus_start == bus_res->start &&
		    e->bus_end >= bus_res->end)
			return e->addr;
	}

	return 0;
}

/*
 * Lookup the bus range for the domain in MCFG, and set up config space
 * mapping.
 */
struct pci_config_window *
pci_acpi_setup_ecam_mapping(struct acpi_pci_root *root, struct pci_ops *ops)
{
	struct resource *bus_res = &root->secondary;
	u16 seg = root->segment;
	struct pci_config_window *cfg;
	struct resource cfgres;
	struct pci_ecam_ops ecam_ops = {
			.bus_shift = 20,
			.pci_ops = *ops,
	};

	/* Use address from _CBA if present, otherwise lookup MCFG */
	if (!root->mcfg_addr)
		root->mcfg_addr = pci_mcfg_lookup(seg, bus_res);

	if (!root->mcfg_addr) {
		dev_err(&root->device->dev, "%04x:%pR ECAM region not found\n",
			seg, bus_res);
		return NULL;
	}

	cfgres.start = root->mcfg_addr + (bus_res->start << 20);
	cfgres.end = cfgres.start + (resource_size(bus_res) << 20) - 1;
	cfgres.flags = IORESOURCE_MEM;
	cfg = pci_ecam_create(&root->device->dev, &cfgres, bus_res, &ecam_ops);
	if (IS_ERR(cfg)) {
		dev_err(&root->device->dev, "%04x:%pR error %ld mapping ECAM\n",
			seg, bus_res, PTR_ERR(cfg));
		return NULL;
	}

	return cfg;
}

static __init int pci_mcfg_parse(struct acpi_table_header *header)
{
	struct acpi_table_mcfg *mcfg;
	struct acpi_mcfg_allocation *mptr;
	struct mcfg_entry *e, *arr;
	int i, n;

	if (header->length < sizeof(struct acpi_table_mcfg))
		return -EINVAL;

	n = (header->length - sizeof(struct acpi_table_mcfg)) /
					sizeof(struct acpi_mcfg_allocation);
	mcfg = (struct acpi_table_mcfg *)header;
	mptr = (struct acpi_mcfg_allocation *) &mcfg[1];

	arr = kcalloc(n, sizeof(*arr), GFP_KERNEL);
	if (!arr)
		return -ENOMEM;

	for (i = 0, e = arr; i < n; i++, mptr++, e++) {
		e->segment = mptr->pci_segment;
		e->addr =  mptr->address;
		e->bus_start = mptr->start_bus_number;
		e->bus_end = mptr->end_bus_number;
		list_add(&e->list, &pci_mcfg_list);
	}

	pr_info("MCFG table detected, %d entries\n", n);
	return 0;
}

/* Interface called by ACPI - parse and save MCFG table */
void __init pci_mmcfg_late_init(void)
{
	int err = acpi_table_parse(ACPI_SIG_MCFG, pci_mcfg_parse);
	if (err)
		pr_err("Failed to parse MCFG (%d)\n", err);
}

/*
 * Copyright (C) 2016 Semihalf
 *	Author: Tomasz Nowicki <tn@semihalf.com>
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

#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/pci-acpi.h>
#include <linux/pci-ecam.h>

#include "mcfg-quirks.h"

struct pci_cfg_fixup {
	char *oem_id;
	char *oem_table_id;
	u32 oem_revision;
	struct resource domain_range;
	struct resource bus_range;
	struct pci_ops *ops;
	struct pci_config_window *(*init)(struct acpi_pci_root *root,
					  struct pci_ops *ops);
};

#define MCFG_DOM_RANGE(start, end)	DEFINE_RES_NAMED((start),	\
						((end) - (start) + 1), NULL, 0)
#define MCFG_DOM_ANY			MCFG_DOM_RANGE(0x0, 0xffff)
#define MCFG_BUS_RANGE(start, end)	DEFINE_RES_NAMED((start),	\
						((end) - (start) + 1),	\
						NULL, IORESOURCE_BUS)
#define MCFG_BUS_ANY			MCFG_BUS_RANGE(0x0, 0xff)

static struct pci_cfg_fixup mcfg_qurks[] __initconst = {
/*	{ OEM_ID, OEM_TABLE_ID, REV, DOMAIN, BUS_RANGE, pci_ops, init_hook }, */
};

static bool pci_mcfg_fixup_match(struct pci_cfg_fixup *f,
				 struct acpi_table_header *mcfg_header)
{
	int olen = min_t(u8, strlen(f->oem_id), ACPI_OEM_ID_SIZE);
	int tlen = min_t(u8, strlen(f->oem_table_id), ACPI_OEM_TABLE_ID_SIZE);

	return (!strncmp(f->oem_id, mcfg_header->oem_id, olen) &&
		!strncmp(f->oem_table_id, mcfg_header->oem_table_id, tlen) &&
		f->oem_revision == mcfg_header->oem_revision);
}

struct pci_config_window *pci_mcfg_match_quirks(struct acpi_pci_root *root)
{
	struct resource dom_res = MCFG_DOM_RANGE(root->segment, root->segment);
	struct resource *bus_res = &root->secondary;
	struct pci_cfg_fixup *f = mcfg_qurks;
	struct acpi_table_header *mcfg_header;
	acpi_status status;
	int i;

	status = acpi_get_table(ACPI_SIG_MCFG, 0, &mcfg_header);
	if (ACPI_FAILURE(status))
		return NULL;

	/*
	 * First match against PCI topology <domain:bus> then use OEM ID, OEM
	 * table ID, and OEM revision from MCFG table standard header.
	 */
	for (i = 0; i < ARRAY_SIZE(mcfg_qurks); i++, f++) {
		if (resource_contains(&f->domain_range, &dom_res) &&
		    resource_contains(&f->bus_range, bus_res) &&
		    pci_mcfg_fixup_match(f, mcfg_header)) {
			pr_info("Handling %s %s r%d PCI MCFG quirks\n",
				f->oem_id, f->oem_table_id, f->oem_revision);
			return f->init ? f->init(root, f->ops) :
				pci_acpi_setup_ecam_mapping(root, f->ops);
		}
	}
	return pci_acpi_setup_ecam_mapping(root, &pci_generic_ecam_ops.pci_ops);
}

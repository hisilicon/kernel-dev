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
	char oem_id[ACPI_OEM_ID_SIZE + 1];
	char oem_table_id[ACPI_OEM_TABLE_ID_SIZE + 1];
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

static struct pci_cfg_fixup mcfg_quirks[] __initconst = {
/*	{ OEM_ID, OEM_TABLE_ID, REV, DOMAIN, BUS_RANGE, pci_ops, init_hook }, */
#ifdef CONFIG_PCI_HOST_THUNDER_PEM
	/* Pass2.0 */
	{ "CAVIUM", "THUNDERX", 1, MCFG_DOM_RANGE(4, 9), MCFG_BUS_ANY, NULL,
	  thunder_pem_cfg_init },
	{ "CAVIUM", "THUNDERX", 1, MCFG_DOM_RANGE(14, 19), MCFG_BUS_ANY, NULL,
	  thunder_pem_cfg_init },
#endif
#ifdef CONFIG_PCI_HISI_ACPI
	{ "HISI  ", "HIP05   ", 0, MCFG_DOM_RANGE(0, 3), MCFG_BUS_ANY,
	  NULL, hisi_pcie_acpi_hip05_init},
	{ "HISI  ", "HIP06   ", 0, MCFG_DOM_RANGE(0, 3), MCFG_BUS_ANY,
	  NULL, hisi_pcie_acpi_hip06_init},
	{ "HISI  ", "HIP07   ", 0, MCFG_DOM_RANGE(0, 15), MCFG_BUS_ANY,
	  NULL, hisi_pcie_acpi_hip07_init},
#endif
};

static bool pci_mcfg_fixup_match(struct pci_cfg_fixup *f,
				 struct acpi_table_header *mcfg_header)
{
	return (!memcmp(f->oem_id, mcfg_header->oem_id, ACPI_OEM_ID_SIZE) &&
		!memcmp(f->oem_table_id, mcfg_header->oem_table_id,
			ACPI_OEM_TABLE_ID_SIZE) &&
		f->oem_revision == mcfg_header->oem_revision);
}

struct pci_config_window *pci_mcfg_match_quirks(struct acpi_pci_root *root)
{
	struct resource dom_res = MCFG_DOM_RANGE(root->segment, root->segment);
	struct resource *bus_res = &root->secondary;
	struct pci_cfg_fixup *f = mcfg_quirks;
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
	for (i = 0; i < ARRAY_SIZE(mcfg_quirks); i++, f++) {
		if (resource_contains(&f->domain_range, &dom_res) &&
		    resource_contains(&f->bus_range, bus_res) &&
		    pci_mcfg_fixup_match(f, mcfg_header)) {
			dev_info(&root->device->dev, "Applying PCI MCFG quirks for %s %s rev: %d\n",
				 f->oem_id, f->oem_table_id, f->oem_revision);
			return f->init ? f->init(root, f->ops) :
				pci_acpi_setup_ecam_mapping(root, f->ops);
		}
	}
	return pci_acpi_setup_ecam_mapping(root, &pci_generic_ecam_ops.pci_ops);
}

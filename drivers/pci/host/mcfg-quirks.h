/*
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

#ifndef __MCFG_QUIRKS_H__
#define __MCFG_QUIRKS_H__

/* MCFG quirks initialize call list */
#ifdef CONFIG_PCI_HOST_THUNDER_PEM
struct pci_config_window *
thunder_pem_cfg_init(struct acpi_pci_root *root, struct pci_ops *ops);
#endif

#ifdef CONFIG_PCI_HISI_ACPI
struct pci_config_window *
hisi_pcie_acpi_hip05_init(struct acpi_pci_root *root, struct pci_ops *ops);

struct pci_config_window *
hisi_pcie_acpi_hip06_init(struct acpi_pci_root *root, struct pci_ops *ops);

struct pci_config_window *
hisi_pcie_acpi_hip07_init(struct acpi_pci_root *root, struct pci_ops *ops);
#endif

#endif /* __MCFG_QUIRKS_H__ */

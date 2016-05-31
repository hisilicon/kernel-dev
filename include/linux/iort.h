/*
 * Copyright (C) 2016, Semihalf
 *	Author: Tomasz Nowicki <tn@semihalf.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef __IORT_H__
#define __IORT_H__

#include <linux/acpi.h>

struct fwnode_handle;
int iort_register_domain_token(int trans_id, struct fwnode_handle *fw_node);
void iort_deregister_domain_token(int trans_id);
struct fwnode_handle *iort_its_find_domain_token(int trans_id);
#ifdef CONFIG_IORT_TABLE
u32 iort_pci_get_msi_rid(struct pci_dev *pdev, u32 req_id);
struct irq_domain *iort_pci_get_domain(struct pci_dev *pdev, u32 req_id);
#else
static inline u32 iort_pci_get_msi_rid(struct pci_dev *pdev, u32 req_id)
{ return req_id; }
static inline struct irq_domain *
iort_pci_get_domain(struct pci_dev *pdev, u32 req_id) { return NULL; }
#endif

#endif /* __IORT_H__ */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020, Mellanox Technologies, Ltd.  All rights reserved.
 *     Author: Max Gurtovoy <mgurtovoy@nvidia.com>
 */

#ifndef NPU2_VFIO_PCI_H
#define NPU2_VFIO_PCI_H

#include <linux/pci.h>
#include <linux/module.h>

#ifdef CONFIG_VFIO_PCI_DRIVER_COMPAT
#if defined(CONFIG_VFIO_PCI_NPU2) || defined(CONFIG_VFIO_PCI_NPU2_MODULE)
struct pci_driver *get_npu2_vfio_pci_driver(struct pci_dev *pdev);
#else
struct pci_driver *get_npu2_vfio_pci_driver(struct pci_dev *pdev)
{
	return NULL;
}
#endif
#endif

#endif /* NPU2_VFIO_PCI_H */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020, Mellanox Technologies, Ltd.  All rights reserved.
 *     Author: Max Gurtovoy <mgurtovoy@nvidia.com>
 */

#ifndef NVLINK2GPU_VFIO_PCI_H
#define NVLINK2GPU_VFIO_PCI_H

#include <linux/pci.h>
#include <linux/module.h>

#ifdef CONFIG_VFIO_PCI_DRIVER_COMPAT
#if defined(CONFIG_VFIO_PCI_NVLINK2GPU) || defined(CONFIG_VFIO_PCI_NVLINK2GPU_MODULE)
struct pci_driver *get_nvlink2gpu_vfio_pci_driver(struct pci_dev *pdev);
#else
struct pci_driver *get_nvlink2gpu_vfio_pci_driver(struct pci_dev *pdev)
{
	return NULL;
}
#endif
#endif

#endif /* NVLINK2GPU_VFIO_PCI_H */

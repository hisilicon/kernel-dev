# SPDX-License-Identifier: GPL-2.0-only

obj-$(CONFIG_VFIO_PCI_CORE) += vfio-pci-core.o
obj-$(CONFIG_VFIO_PCI) += vfio-pci.o
obj-$(CONFIG_MLX5_VFIO_PCI) += mlx5-vfio-pci.o
obj-$(CONFIG_HISI_ACC_VFIO_PCI) += hisi-acc-vfio-pci.o

vfio-pci-core-y := vfio_pci_core.o vfio_pci_intrs.o vfio_pci_rdwr.o vfio_pci_config.o
vfio-pci-core-$(CONFIG_S390) += vfio_pci_zdev.o

vfio-pci-y := vfio_pci.o
vfio-pci-$(CONFIG_VFIO_PCI_IGD) += vfio_pci_igd.o

mlx5-vfio-pci-y := mlx5_vfio_pci.o
hisi-acc-vfio-pci-y := hisi_acc_vfio_pci.o

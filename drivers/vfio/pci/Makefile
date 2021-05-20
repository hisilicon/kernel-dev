# SPDX-License-Identifier: GPL-2.0-only

obj-$(CONFIG_VFIO_PCI_CORE) += vfio-pci-core.o
obj-$(CONFIG_VFIO_PCI) += vfio-pci.o
obj-$(CONFIG_VFIO_PCI_NPU2) += npu2-vfio-pci.o
obj-$(CONFIG_VFIO_PCI_NVLINK2GPU) += nvlink2gpu-vfio-pci.o
obj-$(CONFIG_VFIO_PCI_IGD) += igd-vfio-pci.o

vfio-pci-core-y := vfio_pci_core.o vfio_pci_intrs.o vfio_pci_rdwr.o vfio_pci_config.o
vfio-pci-core-$(CONFIG_S390) += vfio_pci_zdev.o
vfio-pci-core-$(CONFIG_VFIO_PCI_HISI_MIGRATION) += hisilicon/acc_vf_migration.o


vfio-pci-y := vfio_pci.o

npu2-vfio-pci-y := npu2_vfio_pci.o

nvlink2gpu-vfio-pci-y := nvlink2gpu_vfio_pci.o

igd-vfio-pci-y := igd_vfio_pci.o

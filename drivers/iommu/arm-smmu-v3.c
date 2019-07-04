// SPDX-License-Identifier: GPL-2.0
/*
 * IOMMU API for ARM architected SMMUv3 implementations.
 *
 * Copyright (C) 2015 ARM Limited
 *
 * Author: Will Deacon <will.deacon@arm.com>
 *
 * This driver is powered by bad coffee and bombay mix.
 */

#include <linux/acpi.h>
#include <linux/acpi_iort.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/cpufeature.h>
#include <linux/crash_dump.h>
#include <linux/delay.h>
#include <linux/dma-iommu.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io-pgtable.h>
#include <linux/ioasid.h>
#include <linux/iommu.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/mmu_context.h>
#include <linux/msi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_iommu.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/pci-ats.h>
#include <linux/platform_device.h>

#include <linux/amba/bus.h>

#include "io-pgtable-arm.h"
#include "iommu-sva.h"

/* MMIO registers */
#define ARM_SMMU_IDR0			0x0
#define IDR0_ST_LVL			GENMASK(28, 27)
#define IDR0_ST_LVL_2LVL		1
#define IDR0_STALL_MODEL		GENMASK(25, 24)
#define IDR0_STALL_MODEL_STALL		0
#define IDR0_STALL_MODEL_FORCE		2
#define IDR0_TTENDIAN			GENMASK(22, 21)
#define IDR0_TTENDIAN_MIXED		0
#define IDR0_TTENDIAN_LE		2
#define IDR0_TTENDIAN_BE		3
#define IDR0_CD2L			(1 << 19)
#define IDR0_VMID16			(1 << 18)
#define IDR0_PRI			(1 << 16)
#define IDR0_SEV			(1 << 14)
#define IDR0_MSI			(1 << 13)
#define IDR0_ASID16			(1 << 12)
#define IDR0_ATS			(1 << 10)
#define IDR0_HYP			(1 << 9)
#define IDR0_HD				(1 << 7)
#define IDR0_HA				(1 << 6)
#define IDR0_BTM			(1 << 5)
#define IDR0_COHACC			(1 << 4)
#define IDR0_TTF			GENMASK(3, 2)
#define IDR0_TTF_AARCH64		2
#define IDR0_TTF_AARCH32_64		3
#define IDR0_S1P			(1 << 1)
#define IDR0_S2P			(1 << 0)

#define ARM_SMMU_IDR1			0x4
#define IDR1_TABLES_PRESET		(1 << 30)
#define IDR1_QUEUES_PRESET		(1 << 29)
#define IDR1_REL			(1 << 28)
#define IDR1_CMDQS			GENMASK(25, 21)
#define IDR1_EVTQS			GENMASK(20, 16)
#define IDR1_PRIQS			GENMASK(15, 11)
#define IDR1_SSIDSIZE			GENMASK(10, 6)
#define IDR1_SIDSIZE			GENMASK(5, 0)

#define ARM_SMMU_IDR5			0x14
#define IDR5_STALL_MAX			GENMASK(31, 16)
#define IDR5_GRAN64K			(1 << 6)
#define IDR5_GRAN16K			(1 << 5)
#define IDR5_GRAN4K			(1 << 4)
#define IDR5_OAS			GENMASK(2, 0)
#define IDR5_OAS_32_BIT			0
#define IDR5_OAS_36_BIT			1
#define IDR5_OAS_40_BIT			2
#define IDR5_OAS_42_BIT			3
#define IDR5_OAS_44_BIT			4
#define IDR5_OAS_48_BIT			5
#define IDR5_OAS_52_BIT			6
#define IDR5_VAX			GENMASK(11, 10)
#define IDR5_VAX_52_BIT			1

#define ARM_SMMU_CR0			0x20
#define CR0_ATSCHK			(1 << 4)
#define CR0_CMDQEN			(1 << 3)
#define CR0_EVTQEN			(1 << 2)
#define CR0_PRIQEN			(1 << 1)
#define CR0_SMMUEN			(1 << 0)

#define ARM_SMMU_CR0ACK			0x24

#define ARM_SMMU_CR1			0x28
#define CR1_TABLE_SH			GENMASK(11, 10)
#define CR1_TABLE_OC			GENMASK(9, 8)
#define CR1_TABLE_IC			GENMASK(7, 6)
#define CR1_QUEUE_SH			GENMASK(5, 4)
#define CR1_QUEUE_OC			GENMASK(3, 2)
#define CR1_QUEUE_IC			GENMASK(1, 0)
/* CR1 cacheability fields don't quite follow the usual TCR-style encoding */
#define CR1_CACHE_NC			0
#define CR1_CACHE_WB			1
#define CR1_CACHE_WT			2

#define ARM_SMMU_CR2			0x2c
#define CR2_PTM				(1 << 2)
#define CR2_RECINVSID			(1 << 1)
#define CR2_E2H				(1 << 0)

#define ARM_SMMU_GBPA			0x44
#define GBPA_UPDATE			(1 << 31)
#define GBPA_ABORT			(1 << 20)

#define ARM_SMMU_IRQ_CTRL		0x50
#define IRQ_CTRL_EVTQ_IRQEN		(1 << 2)
#define IRQ_CTRL_PRIQ_IRQEN		(1 << 1)
#define IRQ_CTRL_GERROR_IRQEN		(1 << 0)

#define ARM_SMMU_IRQ_CTRLACK		0x54

#define ARM_SMMU_GERROR			0x60
#define GERROR_SFM_ERR			(1 << 8)
#define GERROR_MSI_GERROR_ABT_ERR	(1 << 7)
#define GERROR_MSI_PRIQ_ABT_ERR		(1 << 6)
#define GERROR_MSI_EVTQ_ABT_ERR		(1 << 5)
#define GERROR_MSI_CMDQ_ABT_ERR		(1 << 4)
#define GERROR_PRIQ_ABT_ERR		(1 << 3)
#define GERROR_EVTQ_ABT_ERR		(1 << 2)
#define GERROR_CMDQ_ERR			(1 << 0)
#define GERROR_ERR_MASK			0xfd

#define ARM_SMMU_GERRORN		0x64

#define ARM_SMMU_GERROR_IRQ_CFG0	0x68
#define ARM_SMMU_GERROR_IRQ_CFG1	0x70
#define ARM_SMMU_GERROR_IRQ_CFG2	0x74

#define ARM_SMMU_STRTAB_BASE		0x80
#define STRTAB_BASE_RA			(1UL << 62)
#define STRTAB_BASE_ADDR_MASK		GENMASK_ULL(51, 6)

#define ARM_SMMU_STRTAB_BASE_CFG	0x88
#define STRTAB_BASE_CFG_FMT		GENMASK(17, 16)
#define STRTAB_BASE_CFG_FMT_LINEAR	0
#define STRTAB_BASE_CFG_FMT_2LVL	1
#define STRTAB_BASE_CFG_SPLIT		GENMASK(10, 6)
#define STRTAB_BASE_CFG_LOG2SIZE	GENMASK(5, 0)

#define ARM_SMMU_CMDQ_BASE		0x90
#define ARM_SMMU_CMDQ_PROD		0x98
#define ARM_SMMU_CMDQ_CONS		0x9c

#define ARM_SMMU_EVTQ_BASE		0xa0
#define ARM_SMMU_EVTQ_PROD		0x100a8
#define ARM_SMMU_EVTQ_CONS		0x100ac
#define ARM_SMMU_EVTQ_IRQ_CFG0		0xb0
#define ARM_SMMU_EVTQ_IRQ_CFG1		0xb8
#define ARM_SMMU_EVTQ_IRQ_CFG2		0xbc

#define ARM_SMMU_PRIQ_BASE		0xc0
#define ARM_SMMU_PRIQ_PROD		0x100c8
#define ARM_SMMU_PRIQ_CONS		0x100cc
#define ARM_SMMU_PRIQ_IRQ_CFG0		0xd0
#define ARM_SMMU_PRIQ_IRQ_CFG1		0xd8
#define ARM_SMMU_PRIQ_IRQ_CFG2		0xdc

/* Common MSI config fields */
#define MSI_CFG0_ADDR_MASK		GENMASK_ULL(51, 2)
#define MSI_CFG2_SH			GENMASK(5, 4)
#define MSI_CFG2_MEMATTR		GENMASK(3, 0)

/* Common memory attribute values */
#define ARM_SMMU_SH_NSH			0
#define ARM_SMMU_SH_OSH			2
#define ARM_SMMU_SH_ISH			3
#define ARM_SMMU_MEMATTR_DEVICE_nGnRE	0x1
#define ARM_SMMU_MEMATTR_OIWB		0xf

#define Q_IDX(llq, p)			((p) & ((1 << (llq)->max_n_shift) - 1))
#define Q_WRP(llq, p)			((p) & (1 << (llq)->max_n_shift))
#define Q_OVERFLOW_FLAG			(1U << 31)
#define Q_OVF(p)			((p) & Q_OVERFLOW_FLAG)
#define Q_ENT(q, p)			((q)->base +			\
					 Q_IDX(&((q)->llq), p) *	\
					 (q)->ent_dwords)

#define Q_BASE_RWA			(1UL << 62)
#define Q_BASE_ADDR_MASK		GENMASK_ULL(51, 5)
#define Q_BASE_LOG2SIZE			GENMASK(4, 0)

/* Ensure DMA allocations are naturally aligned */
#ifdef CONFIG_CMA_ALIGNMENT
#define Q_MAX_SZ_SHIFT			(PAGE_SHIFT + CONFIG_CMA_ALIGNMENT)
#else
#define Q_MAX_SZ_SHIFT			(PAGE_SHIFT + MAX_ORDER - 1)
#endif

/*
 * Stream table.
 *
 * Linear: Enough to cover 1 << IDR1.SIDSIZE entries
 * 2lvl: 128k L1 entries,
 *       256 lazy entries per table (each table covers a PCI bus)
 */
#define STRTAB_L1_SZ_SHIFT		20
#define STRTAB_SPLIT			8

#define STRTAB_L1_DESC_DWORDS		1
#define STRTAB_L1_DESC_SPAN		GENMASK_ULL(4, 0)
#define STRTAB_L1_DESC_L2PTR_MASK	GENMASK_ULL(51, 6)

#define STRTAB_STE_DWORDS		8
#define STRTAB_STE_0_V			(1UL << 0)
#define STRTAB_STE_0_CFG		GENMASK_ULL(3, 1)
#define STRTAB_STE_0_CFG_ABORT		0
#define STRTAB_STE_0_CFG_BYPASS		4
#define STRTAB_STE_0_CFG_S1_TRANS	5
#define STRTAB_STE_0_CFG_S2_TRANS	6

#define STRTAB_STE_0_S1FMT		GENMASK_ULL(5, 4)
#define STRTAB_STE_0_S1FMT_LINEAR	0
#define STRTAB_STE_0_S1FMT_64K_L2	2
#define STRTAB_STE_0_S1CTXPTR_MASK	GENMASK_ULL(51, 6)
#define STRTAB_STE_0_S1CDMAX		GENMASK_ULL(63, 59)

#define STRTAB_STE_1_S1DSS		GENMASK_ULL(1, 0)
#define STRTAB_STE_1_S1DSS_TERMINATE	0x0
#define STRTAB_STE_1_S1DSS_BYPASS	0x1
#define STRTAB_STE_1_S1DSS_SSID0	0x2

#define STRTAB_STE_1_S1C_CACHE_NC	0UL
#define STRTAB_STE_1_S1C_CACHE_WBRA	1UL
#define STRTAB_STE_1_S1C_CACHE_WT	2UL
#define STRTAB_STE_1_S1C_CACHE_WB	3UL
#define STRTAB_STE_1_S1CIR		GENMASK_ULL(3, 2)
#define STRTAB_STE_1_S1COR		GENMASK_ULL(5, 4)
#define STRTAB_STE_1_S1CSH		GENMASK_ULL(7, 6)

#define STRTAB_STE_1_PPAR		(1UL << 18)
#define STRTAB_STE_1_S1STALLD		(1UL << 27)

#define STRTAB_STE_1_EATS		GENMASK_ULL(29, 28)
#define STRTAB_STE_1_EATS_ABT		0UL
#define STRTAB_STE_1_EATS_TRANS		1UL
#define STRTAB_STE_1_EATS_S1CHK		2UL

#define STRTAB_STE_1_STRW		GENMASK_ULL(31, 30)
#define STRTAB_STE_1_STRW_NSEL1		0UL
#define STRTAB_STE_1_STRW_EL2		2UL

#define STRTAB_STE_1_SHCFG		GENMASK_ULL(45, 44)
#define STRTAB_STE_1_SHCFG_INCOMING	1UL

#define STRTAB_STE_2_S2VMID		GENMASK_ULL(15, 0)
#define STRTAB_STE_2_VTCR		GENMASK_ULL(50, 32)
#define STRTAB_STE_2_VTCR_S2T0SZ	GENMASK_ULL(5, 0)
#define STRTAB_STE_2_VTCR_S2SL0		GENMASK_ULL(7, 6)
#define STRTAB_STE_2_VTCR_S2IR0		GENMASK_ULL(9, 8)
#define STRTAB_STE_2_VTCR_S2OR0		GENMASK_ULL(11, 10)
#define STRTAB_STE_2_VTCR_S2SH0		GENMASK_ULL(13, 12)
#define STRTAB_STE_2_VTCR_S2TG		GENMASK_ULL(15, 14)
#define STRTAB_STE_2_VTCR_S2PS		GENMASK_ULL(18, 16)
#define STRTAB_STE_2_S2AA64		(1UL << 51)
#define STRTAB_STE_2_S2ENDI		(1UL << 52)
#define STRTAB_STE_2_S2PTW		(1UL << 54)
#define STRTAB_STE_2_S2R		(1UL << 58)

#define STRTAB_STE_3_S2TTB_MASK		GENMASK_ULL(51, 4)

/*
 * Context descriptors.
 *
 * Linear: when less than 1024 SSIDs are supported
 * 2lvl: at most 1024 L1 entries,
 *       1024 lazy entries per table.
 */
#define CTXDESC_SPLIT			10
#define CTXDESC_L2_ENTRIES		(1 << CTXDESC_SPLIT)

#define CTXDESC_L1_DESC_DWORDS		1
#define CTXDESC_L1_DESC_V		(1UL << 0)
#define CTXDESC_L1_DESC_L2PTR_MASK	GENMASK_ULL(51, 12)

#define CTXDESC_CD_DWORDS		8
#define CTXDESC_CD_0_TCR_T0SZ		GENMASK_ULL(5, 0)
#define CTXDESC_CD_0_TCR_TG0		GENMASK_ULL(7, 6)
#define CTXDESC_CD_0_TCR_IRGN0		GENMASK_ULL(9, 8)
#define CTXDESC_CD_0_TCR_ORGN0		GENMASK_ULL(11, 10)
#define CTXDESC_CD_0_TCR_SH0		GENMASK_ULL(13, 12)
#define CTXDESC_CD_0_TCR_EPD0		(1ULL << 14)
#define CTXDESC_CD_0_TCR_EPD1		(1ULL << 30)

#define CTXDESC_CD_0_ENDI		(1UL << 15)
#define CTXDESC_CD_0_V			(1UL << 31)

#define CTXDESC_CD_0_TCR_IPS		GENMASK_ULL(34, 32)
#define CTXDESC_CD_0_TCR_TBI0		(1ULL << 38)

#define CTXDESC_CD_0_TCR_HA		(1UL << 43)
#define CTXDESC_CD_0_TCR_HD		(1UL << 42)

#define CTXDESC_CD_0_AA64		(1UL << 41)
#define CTXDESC_CD_0_S			(1UL << 44)
#define CTXDESC_CD_0_R			(1UL << 45)
#define CTXDESC_CD_0_A			(1UL << 46)
#define CTXDESC_CD_0_ASET		(1UL << 47)
#define CTXDESC_CD_0_ASID		GENMASK_ULL(63, 48)

#define CTXDESC_CD_1_TTB0_MASK		GENMASK_ULL(51, 4)

/*
 * When the SMMU only supports linear context descriptor tables, pick a
 * reasonable size limit (64kB).
 */
#define CTXDESC_LINEAR_CDMAX		ilog2(SZ_64K / (CTXDESC_CD_DWORDS << 3))

/* Command queue */
#define CMDQ_ENT_SZ_SHIFT		4
#define CMDQ_ENT_DWORDS			((1 << CMDQ_ENT_SZ_SHIFT) >> 3)
#define CMDQ_MAX_SZ_SHIFT		(Q_MAX_SZ_SHIFT - CMDQ_ENT_SZ_SHIFT)

#define CMDQ_CONS_ERR			GENMASK(30, 24)
#define CMDQ_ERR_CERROR_NONE_IDX	0
#define CMDQ_ERR_CERROR_ILL_IDX		1
#define CMDQ_ERR_CERROR_ABT_IDX		2
#define CMDQ_ERR_CERROR_ATC_INV_IDX	3

#define CMDQ_PROD_OWNED_FLAG		Q_OVERFLOW_FLAG

/*
 * This is used to size the command queue and therefore must be at least
 * BITS_PER_LONG so that the valid_map works correctly (it relies on the
 * total number of queue entries being a multiple of BITS_PER_LONG).
 */
#define CMDQ_BATCH_ENTRIES		BITS_PER_LONG

#define CMDQ_0_OP			GENMASK_ULL(7, 0)
#define CMDQ_0_SSV			(1UL << 11)

#define CMDQ_PREFETCH_0_SID		GENMASK_ULL(63, 32)
#define CMDQ_PREFETCH_1_SIZE		GENMASK_ULL(4, 0)
#define CMDQ_PREFETCH_1_ADDR_MASK	GENMASK_ULL(63, 12)

#define CMDQ_CFGI_0_SSID		GENMASK_ULL(31, 12)
#define CMDQ_CFGI_0_SID			GENMASK_ULL(63, 32)
#define CMDQ_CFGI_1_LEAF		(1UL << 0)
#define CMDQ_CFGI_1_RANGE		GENMASK_ULL(4, 0)

#define CMDQ_TLBI_0_VMID		GENMASK_ULL(47, 32)
#define CMDQ_TLBI_0_ASID		GENMASK_ULL(63, 48)
#define CMDQ_TLBI_1_LEAF		(1UL << 0)
#define CMDQ_TLBI_1_VA_MASK		GENMASK_ULL(63, 12)
#define CMDQ_TLBI_1_IPA_MASK		GENMASK_ULL(51, 12)

#define CMDQ_ATC_0_SSID			GENMASK_ULL(31, 12)
#define CMDQ_ATC_0_SID			GENMASK_ULL(63, 32)
#define CMDQ_ATC_0_GLOBAL		(1UL << 9)
#define CMDQ_ATC_1_SIZE			GENMASK_ULL(5, 0)
#define CMDQ_ATC_1_ADDR_MASK		GENMASK_ULL(63, 12)

#define CMDQ_PRI_0_SSID			GENMASK_ULL(31, 12)
#define CMDQ_PRI_0_SID			GENMASK_ULL(63, 32)
#define CMDQ_PRI_1_GRPID		GENMASK_ULL(8, 0)
#define CMDQ_PRI_1_RESP			GENMASK_ULL(13, 12)
#define CMDQ_PRI_1_RESP_FAILURE		0UL
#define CMDQ_PRI_1_RESP_INVALID		1UL
#define CMDQ_PRI_1_RESP_SUCCESS		2UL

#define CMDQ_RESUME_0_SID		GENMASK_ULL(63, 32)
#define CMDQ_RESUME_0_RESP_TERM		0UL
#define CMDQ_RESUME_0_RESP_RETRY	1UL
#define CMDQ_RESUME_0_RESP_ABORT	2UL
#define CMDQ_RESUME_0_RESP		GENMASK_ULL(13, 12)
#define CMDQ_RESUME_1_STAG		GENMASK_ULL(15, 0)

#define CMDQ_SYNC_0_CS			GENMASK_ULL(13, 12)
#define CMDQ_SYNC_0_CS_NONE		0
#define CMDQ_SYNC_0_CS_IRQ		1
#define CMDQ_SYNC_0_CS_SEV		2
#define CMDQ_SYNC_0_MSH			GENMASK_ULL(23, 22)
#define CMDQ_SYNC_0_MSIATTR		GENMASK_ULL(27, 24)
#define CMDQ_SYNC_0_MSIDATA		GENMASK_ULL(63, 32)
#define CMDQ_SYNC_1_MSIADDR_MASK	GENMASK_ULL(51, 2)

/* Event queue */
#define EVTQ_ENT_SZ_SHIFT		5
#define EVTQ_ENT_DWORDS			((1 << EVTQ_ENT_SZ_SHIFT) >> 3)
#define EVTQ_MAX_SZ_SHIFT		(Q_MAX_SZ_SHIFT - EVTQ_ENT_SZ_SHIFT)

#define EVTQ_0_ID			GENMASK_ULL(7, 0)

#define EVT_ID_TRANSLATION_FAULT	0x10
#define EVT_ID_ADDR_SIZE_FAULT		0x11
#define EVT_ID_ACCESS_FAULT		0x12
#define EVT_ID_PERMISSION_FAULT		0x13

#define EVTQ_0_SSV			(1UL << 11)
#define EVTQ_0_SSID			GENMASK_ULL(31, 12)
#define EVTQ_0_SID			GENMASK_ULL(63, 32)
#define EVTQ_1_STAG			GENMASK_ULL(15, 0)
#define EVTQ_1_STALL			(1UL << 31)
#define EVTQ_1_PRIV			(1UL << 33)
#define EVTQ_1_EXEC			(1UL << 34)
#define EVTQ_1_READ			(1UL << 35)
#define EVTQ_1_S2			(1UL << 39)
#define EVTQ_1_CLASS			GENMASK_ULL(41, 40)
#define EVTQ_1_TT_READ			(1UL << 44)
#define EVTQ_2_ADDR			GENMASK_ULL(63, 0)
#define EVTQ_3_IPA			GENMASK_ULL(51, 12)

/* PRI queue */
#define PRIQ_ENT_SZ_SHIFT		4
#define PRIQ_ENT_DWORDS			((1 << PRIQ_ENT_SZ_SHIFT) >> 3)
#define PRIQ_MAX_SZ_SHIFT		(Q_MAX_SZ_SHIFT - PRIQ_ENT_SZ_SHIFT)

#define PRIQ_0_SID			GENMASK_ULL(31, 0)
#define PRIQ_0_SSID			GENMASK_ULL(51, 32)
#define PRIQ_0_PERM_PRIV		(1UL << 58)
#define PRIQ_0_PERM_EXEC		(1UL << 59)
#define PRIQ_0_PERM_READ		(1UL << 60)
#define PRIQ_0_PERM_WRITE		(1UL << 61)
#define PRIQ_0_PRG_LAST			(1UL << 62)
#define PRIQ_0_SSID_V			(1UL << 63)

#define PRIQ_1_PRG_IDX			GENMASK_ULL(8, 0)
#define PRIQ_1_ADDR_MASK		GENMASK_ULL(63, 12)

/* High-level queue structures */
#define ARM_SMMU_POLL_TIMEOUT_US	1000000 /* 1s! */
#define ARM_SMMU_POLL_SPIN_COUNT	10

#define MSI_IOVA_BASE			0x8000000
#define MSI_IOVA_LENGTH			0x100000

static bool disable_bypass = 1;
module_param_named(disable_bypass, disable_bypass, bool, S_IRUGO);
MODULE_PARM_DESC(disable_bypass,
	"Disable bypass streams such that incoming transactions from devices that are not attached to an iommu domain will report an abort back to the device and will not be allowed to pass through the SMMU.");

enum arm_smmu_msi_index {
	EVTQ_MSI_INDEX,
	GERROR_MSI_INDEX,
	PRIQ_MSI_INDEX,
	ARM_SMMU_MAX_MSIS,
};

static phys_addr_t arm_smmu_msi_cfg[ARM_SMMU_MAX_MSIS][3] = {
	[EVTQ_MSI_INDEX] = {
		ARM_SMMU_EVTQ_IRQ_CFG0,
		ARM_SMMU_EVTQ_IRQ_CFG1,
		ARM_SMMU_EVTQ_IRQ_CFG2,
	},
	[GERROR_MSI_INDEX] = {
		ARM_SMMU_GERROR_IRQ_CFG0,
		ARM_SMMU_GERROR_IRQ_CFG1,
		ARM_SMMU_GERROR_IRQ_CFG2,
	},
	[PRIQ_MSI_INDEX] = {
		ARM_SMMU_PRIQ_IRQ_CFG0,
		ARM_SMMU_PRIQ_IRQ_CFG1,
		ARM_SMMU_PRIQ_IRQ_CFG2,
	},
};

struct arm_smmu_cmdq_ent {
	/* Common fields */
	u8				opcode;
	bool				substream_valid;

	/* Command-specific fields */
	union {
		#define CMDQ_OP_PREFETCH_CFG	0x1
		struct {
			u32			sid;
			u8			size;
			u64			addr;
		} prefetch;

		#define CMDQ_OP_CFGI_STE	0x3
		#define CMDQ_OP_CFGI_ALL	0x4
		#define CMDQ_OP_CFGI_CD		0x5
		#define CMDQ_OP_CFGI_CD_ALL	0x6
		struct {
			u32			sid;
			u32			ssid;
			union {
				bool		leaf;
				u8		span;
			};
		} cfgi;

		#define CMDQ_OP_TLBI_NH_ASID	0x11
		#define CMDQ_OP_TLBI_NH_VA	0x12
		#define CMDQ_OP_TLBI_EL2_ALL	0x20
		#define CMDQ_OP_TLBI_EL2_ASID	0x21
		#define CMDQ_OP_TLBI_EL2_VA	0x22
		#define CMDQ_OP_TLBI_S12_VMALL	0x28
		#define CMDQ_OP_TLBI_S2_IPA	0x2a
		#define CMDQ_OP_TLBI_NSNH_ALL	0x30
		struct {
			u16			asid;
			u16			vmid;
			bool			leaf;
			u64			addr;
		} tlbi;

		#define CMDQ_OP_ATC_INV		0x40
		#define ATC_INV_SIZE_ALL	52
		struct {
			u32			sid;
			u32			ssid;
			u64			addr;
			u8			size;
			bool			global;
		} atc;

		#define CMDQ_OP_PRI_RESP	0x41
		struct {
			u32			sid;
			u32			ssid;
			u16			grpid;
			u8			resp;
		} pri;

		#define CMDQ_OP_RESUME		0x44
		struct {
			u32			sid;
			u16			stag;
			u8			resp;
		} resume;

		#define CMDQ_OP_CMD_SYNC	0x46
		struct {
			u64			msiaddr;
		} sync;
	};
};

struct arm_smmu_ll_queue {
	union {
		u64			val;
		struct {
			u32		prod;
			u32		cons;
		};
		struct {
			atomic_t	prod;
			atomic_t	cons;
		} atomic;
		u8			__pad[SMP_CACHE_BYTES];
	} ____cacheline_aligned_in_smp;
	u32				max_n_shift;
};

struct arm_smmu_queue {
	struct arm_smmu_ll_queue	llq;
	int				irq; /* Wired interrupt */

	__le64				*base;
	dma_addr_t			base_dma;
	u64				q_base;

	size_t				ent_dwords;

	u32 __iomem			*prod_reg;
	u32 __iomem			*cons_reg;

	/* Event and PRI */
	u64				batch;
	wait_queue_head_t		wq;
};

struct arm_smmu_queue_poll {
	ktime_t				timeout;
	unsigned int			delay;
	unsigned int			spin_cnt;
	bool				wfe;
};

struct arm_smmu_cmdq {
	struct arm_smmu_queue		q;
	atomic_long_t			*valid_map;
	atomic_t			owner_prod;
	atomic_t			lock;
};

struct arm_smmu_cmdq_batch {
	u64				cmds[CMDQ_BATCH_ENTRIES * CMDQ_ENT_DWORDS];
	int				num;
};

struct arm_smmu_evtq {
	struct arm_smmu_queue		q;
	struct iopf_queue		*iopf;
	u32				max_stalls;
};

struct arm_smmu_priq {
	struct arm_smmu_queue		q;
	struct iopf_queue		*iopf;
};

/* High-level stream table and context descriptor structures */
struct arm_smmu_strtab_l1_desc {
	u8				span;

	__le64				*l2ptr;
	dma_addr_t			l2ptr_dma;
};

struct arm_smmu_ctx_desc {
	u16				asid;
	u64				ttbr;
	u64				tcr;
	u64				mair;

	refcount_t			refs;
	struct mm_struct		*mm;
	struct arm_smmu_device		*smmu_domain;
};

struct arm_smmu_l1_ctx_desc {
	__le64				*l2ptr;
	dma_addr_t			l2ptr_dma;
};

struct arm_smmu_ctx_desc_cfg {
	__le64				*cdtab;
	dma_addr_t			cdtab_dma;
	struct arm_smmu_l1_ctx_desc	*l1_desc;
	unsigned int			num_l1_ents;
};

struct arm_smmu_s1_cfg {
	struct arm_smmu_ctx_desc_cfg	cdcfg;
	struct arm_smmu_ctx_desc	cd;
	u8				s1fmt;
	u8				s1cdmax;
};

struct arm_smmu_s2_cfg {
	u16				vmid;
	u64				vttbr;
	u64				vtcr;
};

struct arm_smmu_strtab_cfg {
	__le64				*strtab;
	dma_addr_t			strtab_dma;
	struct arm_smmu_strtab_l1_desc	*l1_desc;
	unsigned int			num_l1_ents;

	u64				strtab_base;
	u32				strtab_base_cfg;
};

/* An SMMUv3 instance */
struct arm_smmu_device {
	struct device			*dev;
	void __iomem			*base;

#define ARM_SMMU_FEAT_2_LVL_STRTAB	(1 << 0)
#define ARM_SMMU_FEAT_2_LVL_CDTAB	(1 << 1)
#define ARM_SMMU_FEAT_TT_LE		(1 << 2)
#define ARM_SMMU_FEAT_TT_BE		(1 << 3)
#define ARM_SMMU_FEAT_PRI		(1 << 4)
#define ARM_SMMU_FEAT_ATS		(1 << 5)
#define ARM_SMMU_FEAT_SEV		(1 << 6)
#define ARM_SMMU_FEAT_MSI		(1 << 7)
#define ARM_SMMU_FEAT_COHERENCY		(1 << 8)
#define ARM_SMMU_FEAT_TRANS_S1		(1 << 9)
#define ARM_SMMU_FEAT_TRANS_S2		(1 << 10)
#define ARM_SMMU_FEAT_STALLS		(1 << 11)
#define ARM_SMMU_FEAT_HYP		(1 << 12)
#define ARM_SMMU_FEAT_STALL_FORCE	(1 << 13)
#define ARM_SMMU_FEAT_VAX		(1 << 14)
#define ARM_SMMU_FEAT_E2H		(1 << 15)
#define ARM_SMMU_FEAT_BTM		(1 << 16)
#define ARM_SMMU_FEAT_SVA		(1 << 17)
#define ARM_SMMU_FEAT_HA		(1 << 18)
#define ARM_SMMU_FEAT_HD		(1 << 19)
	u32				features;

#define ARM_SMMU_OPT_SKIP_PREFETCH	(1 << 0)
#define ARM_SMMU_OPT_PAGE0_REGS_ONLY	(1 << 1)
	u32				options;

	struct arm_smmu_cmdq		cmdq;
	struct arm_smmu_evtq		evtq;
	struct arm_smmu_priq		priq;

	int				gerr_irq;
	int				combined_irq;

	unsigned long			ias; /* IPA */
	unsigned long			oas; /* PA */
	unsigned long			pgsize_bitmap;

#define ARM_SMMU_MAX_ASIDS		(1 << 16)
	unsigned int			asid_bits;

#define ARM_SMMU_MAX_VMIDS		(1 << 16)
	unsigned int			vmid_bits;
	DECLARE_BITMAP(vmid_map, ARM_SMMU_MAX_VMIDS);

	unsigned int			ssid_bits;
	unsigned int			sid_bits;

	struct arm_smmu_strtab_cfg	strtab_cfg;

	/* IOMMU core code handle */
	struct iommu_device		iommu;

	struct rb_root			streams;
	struct mutex			streams_mutex;
};

struct arm_smmu_stream {
	u32				id;
	struct arm_smmu_master		*master;
	struct rb_node			node;
};

/* SMMU private data for each master */
struct arm_smmu_master {
	struct arm_smmu_device		*smmu;
	struct device			*dev;
	struct arm_smmu_domain		*domain;
	struct list_head		domain_head;
	struct arm_smmu_stream		*streams;
	unsigned int			num_streams;
	bool				ats_enabled;
	bool				stall_enabled;
	bool				pri_supported;
	bool				prg_resp_needs_ssid;
	bool				auxd_enabled;
	unsigned int			ssid_bits;
};

/* SMMU private data for an IOMMU domain */
enum arm_smmu_domain_stage {
	ARM_SMMU_DOMAIN_S1 = 0,
	ARM_SMMU_DOMAIN_S2,
	ARM_SMMU_DOMAIN_NESTED,
	ARM_SMMU_DOMAIN_BYPASS,
};

struct arm_smmu_domain {
	struct arm_smmu_device		*smmu;
	struct mutex			init_mutex; /* Protects smmu pointer */

	struct io_pgtable_ops		*pgtbl_ops;
	bool				non_strict;
	bool				stall_enabled;
	atomic_t			nr_ats_masters;

	enum arm_smmu_domain_stage	stage;
	union {
		struct arm_smmu_s1_cfg	s1_cfg;
		struct arm_smmu_s2_cfg	s2_cfg;
	};

	struct iommu_domain		domain;

	/* Unused in aux domains */
	struct list_head		devices;
	spinlock_t			devices_lock;

	/* Auxiliary domain stuff */
	struct arm_smmu_domain		*parent;
	ioasid_t			ssid;
	unsigned long			aux_nr_devs;
};

struct arm_smmu_option_prop {
	u32 opt;
	const char *prop;
};

static DEFINE_XARRAY_ALLOC1(asid_xa);
static DEFINE_SPINLOCK(contexts_lock);
static DECLARE_IOASID_SET(private_ioasid);

static struct arm_smmu_option_prop arm_smmu_options[] = {
	{ ARM_SMMU_OPT_SKIP_PREFETCH, "hisilicon,broken-prefetch-cmd" },
	{ ARM_SMMU_OPT_PAGE0_REGS_ONLY, "cavium,cn9900-broken-page1-regspace"},
	{ 0, NULL},
};

static inline void __iomem *arm_smmu_page1_fixup(unsigned long offset,
						 struct arm_smmu_device *smmu)
{
	if ((offset > SZ_64K) &&
	    (smmu->options & ARM_SMMU_OPT_PAGE0_REGS_ONLY))
		offset -= SZ_64K;

	return smmu->base + offset;
}

static struct arm_smmu_domain *to_smmu_domain(struct iommu_domain *dom)
{
	return container_of(dom, struct arm_smmu_domain, domain);
}

static struct arm_smmu_master *dev_to_master(struct device *dev)
{
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);

	if (!fwspec)
		return NULL;
	return fwspec->iommu_priv;
}

static void parse_driver_options(struct arm_smmu_device *smmu)
{
	int i = 0;

	do {
		if (of_property_read_bool(smmu->dev->of_node,
						arm_smmu_options[i].prop)) {
			smmu->options |= arm_smmu_options[i].opt;
			dev_notice(smmu->dev, "option %s\n",
				arm_smmu_options[i].prop);
		}
	} while (arm_smmu_options[++i].opt);
}

/* Low-level queue manipulation functions */
static bool queue_has_space(struct arm_smmu_ll_queue *q, u32 n)
{
	u32 space, prod, cons;

	prod = Q_IDX(q, q->prod);
	cons = Q_IDX(q, q->cons);

	if (Q_WRP(q, q->prod) == Q_WRP(q, q->cons))
		space = (1 << q->max_n_shift) - (prod - cons);
	else
		space = cons - prod;

	return space >= n;
}

static bool queue_full(struct arm_smmu_ll_queue *q)
{
	return Q_IDX(q, q->prod) == Q_IDX(q, q->cons) &&
	       Q_WRP(q, q->prod) != Q_WRP(q, q->cons);
}

static bool queue_empty(struct arm_smmu_ll_queue *q)
{
	return Q_IDX(q, q->prod) == Q_IDX(q, q->cons) &&
	       Q_WRP(q, q->prod) == Q_WRP(q, q->cons);
}

static bool queue_consumed(struct arm_smmu_ll_queue *q, u32 prod)
{
	return ((Q_WRP(q, q->cons) == Q_WRP(q, prod)) &&
		(Q_IDX(q, q->cons) > Q_IDX(q, prod))) ||
	       ((Q_WRP(q, q->cons) != Q_WRP(q, prod)) &&
		(Q_IDX(q, q->cons) <= Q_IDX(q, prod)));
}

static void queue_sync_cons_out(struct arm_smmu_queue *q)
{
	/*
	 * Ensure that all CPU accesses (reads and writes) to the queue
	 * are complete before we update the cons pointer.
	 */
	mb();
	writel_relaxed(q->llq.cons, q->cons_reg);
}

static void queue_inc_cons(struct arm_smmu_ll_queue *q)
{
	u32 cons = (Q_WRP(q, q->cons) | Q_IDX(q, q->cons)) + 1;
	q->cons = Q_OVF(q->cons) | Q_WRP(q, cons) | Q_IDX(q, cons);
}

static int queue_sync_prod_in(struct arm_smmu_queue *q)
{
	int ret = 0;
	u32 prod = readl_relaxed(q->prod_reg);

	if (Q_OVF(prod) != Q_OVF(q->llq.prod))
		ret = -EOVERFLOW;

	q->llq.prod = prod;
	return ret;
}

static u32 queue_inc_prod_n(struct arm_smmu_ll_queue *q, int n)
{
	u32 prod = (Q_WRP(q, q->prod) | Q_IDX(q, q->prod)) + n;
	return Q_OVF(q->prod) | Q_WRP(q, prod) | Q_IDX(q, prod);
}

static void queue_poll_init(struct arm_smmu_device *smmu,
			    struct arm_smmu_queue_poll *qp)
{
	qp->delay = 1;
	qp->spin_cnt = 0;
	qp->wfe = !!(smmu->features & ARM_SMMU_FEAT_SEV);
	qp->timeout = ktime_add_us(ktime_get(), ARM_SMMU_POLL_TIMEOUT_US);
}

static int queue_poll(struct arm_smmu_queue_poll *qp)
{
	if (ktime_compare(ktime_get(), qp->timeout) > 0)
		return -ETIMEDOUT;

	if (qp->wfe) {
		wfe();
	} else if (++qp->spin_cnt < ARM_SMMU_POLL_SPIN_COUNT) {
		cpu_relax();
	} else {
		udelay(qp->delay);
		qp->delay *= 2;
		qp->spin_cnt = 0;
	}

	return 0;
}

static void queue_write(__le64 *dst, u64 *src, size_t n_dwords)
{
	int i;

	for (i = 0; i < n_dwords; ++i)
		*dst++ = cpu_to_le64(*src++);
}

static void queue_read(__le64 *dst, u64 *src, size_t n_dwords)
{
	int i;

	for (i = 0; i < n_dwords; ++i)
		*dst++ = le64_to_cpu(*src++);
}

static int queue_remove_raw(struct arm_smmu_queue *q, u64 *ent)
{
	if (queue_empty(&q->llq))
		return -EAGAIN;

	queue_read(ent, Q_ENT(q, q->llq.cons), q->ent_dwords);
	queue_inc_cons(&q->llq);
	queue_sync_cons_out(q);
	return 0;
}

/* High-level queue accessors */
static int arm_smmu_cmdq_build_cmd(u64 *cmd, struct arm_smmu_cmdq_ent *ent)
{
	memset(cmd, 0, 1 << CMDQ_ENT_SZ_SHIFT);
	cmd[0] |= FIELD_PREP(CMDQ_0_OP, ent->opcode);

	switch (ent->opcode) {
	case CMDQ_OP_TLBI_EL2_ALL:
	case CMDQ_OP_TLBI_NSNH_ALL:
		break;
	case CMDQ_OP_PREFETCH_CFG:
		cmd[0] |= FIELD_PREP(CMDQ_PREFETCH_0_SID, ent->prefetch.sid);
		cmd[1] |= FIELD_PREP(CMDQ_PREFETCH_1_SIZE, ent->prefetch.size);
		cmd[1] |= ent->prefetch.addr & CMDQ_PREFETCH_1_ADDR_MASK;
		break;
	case CMDQ_OP_CFGI_CD:
		cmd[0] |= FIELD_PREP(CMDQ_CFGI_0_SSID, ent->cfgi.ssid);
		/* Fallthrough */
	case CMDQ_OP_CFGI_STE:
		cmd[0] |= FIELD_PREP(CMDQ_CFGI_0_SID, ent->cfgi.sid);
		cmd[1] |= FIELD_PREP(CMDQ_CFGI_1_LEAF, ent->cfgi.leaf);
		break;
	case CMDQ_OP_CFGI_CD_ALL:
		cmd[0] |= FIELD_PREP(CMDQ_CFGI_0_SID, ent->cfgi.sid);
		break;
	case CMDQ_OP_CFGI_ALL:
		/* Cover the entire SID range */
		cmd[1] |= FIELD_PREP(CMDQ_CFGI_1_RANGE, 31);
		break;
	case CMDQ_OP_TLBI_NH_VA:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_VMID, ent->tlbi.vmid);
		/* Fallthrough */
	case CMDQ_OP_TLBI_EL2_VA:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_ASID, ent->tlbi.asid);
		cmd[1] |= FIELD_PREP(CMDQ_TLBI_1_LEAF, ent->tlbi.leaf);
		cmd[1] |= ent->tlbi.addr & CMDQ_TLBI_1_VA_MASK;
		break;
	case CMDQ_OP_TLBI_S2_IPA:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_VMID, ent->tlbi.vmid);
		cmd[1] |= FIELD_PREP(CMDQ_TLBI_1_LEAF, ent->tlbi.leaf);
		cmd[1] |= ent->tlbi.addr & CMDQ_TLBI_1_IPA_MASK;
		break;
	case CMDQ_OP_TLBI_NH_ASID:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_ASID, ent->tlbi.asid);
		/* Fallthrough */
	case CMDQ_OP_TLBI_S12_VMALL:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_VMID, ent->tlbi.vmid);
		break;
	case CMDQ_OP_TLBI_EL2_ASID:
		cmd[0] |= FIELD_PREP(CMDQ_TLBI_0_ASID, ent->tlbi.asid);
		break;
	case CMDQ_OP_ATC_INV:
		cmd[0] |= FIELD_PREP(CMDQ_0_SSV, ent->substream_valid);
		cmd[0] |= FIELD_PREP(CMDQ_ATC_0_GLOBAL, ent->atc.global);
		cmd[0] |= FIELD_PREP(CMDQ_ATC_0_SSID, ent->atc.ssid);
		cmd[0] |= FIELD_PREP(CMDQ_ATC_0_SID, ent->atc.sid);
		cmd[1] |= FIELD_PREP(CMDQ_ATC_1_SIZE, ent->atc.size);
		cmd[1] |= ent->atc.addr & CMDQ_ATC_1_ADDR_MASK;
		break;
	case CMDQ_OP_PRI_RESP:
		cmd[0] |= FIELD_PREP(CMDQ_0_SSV, ent->substream_valid);
		cmd[0] |= FIELD_PREP(CMDQ_PRI_0_SSID, ent->pri.ssid);
		cmd[0] |= FIELD_PREP(CMDQ_PRI_0_SID, ent->pri.sid);
		cmd[1] |= FIELD_PREP(CMDQ_PRI_1_GRPID, ent->pri.grpid);
		cmd[1] |= FIELD_PREP(CMDQ_PRI_1_RESP, ent->pri.resp);
		break;
	case CMDQ_OP_RESUME:
		cmd[0] |= FIELD_PREP(CMDQ_RESUME_0_SID, ent->resume.sid);
		cmd[0] |= FIELD_PREP(CMDQ_RESUME_0_RESP, ent->resume.resp);
		cmd[1] |= FIELD_PREP(CMDQ_RESUME_1_STAG, ent->resume.stag);
		break;
	case CMDQ_OP_CMD_SYNC:
		if (ent->sync.msiaddr) {
			cmd[0] |= FIELD_PREP(CMDQ_SYNC_0_CS, CMDQ_SYNC_0_CS_IRQ);
			cmd[1] |= ent->sync.msiaddr & CMDQ_SYNC_1_MSIADDR_MASK;
		} else {
			cmd[0] |= FIELD_PREP(CMDQ_SYNC_0_CS, CMDQ_SYNC_0_CS_SEV);
		}
		cmd[0] |= FIELD_PREP(CMDQ_SYNC_0_MSH, ARM_SMMU_SH_ISH);
		cmd[0] |= FIELD_PREP(CMDQ_SYNC_0_MSIATTR, ARM_SMMU_MEMATTR_OIWB);
		break;
	default:
		return -ENOENT;
	}

	return 0;
}

static void arm_smmu_cmdq_build_sync_cmd(u64 *cmd, struct arm_smmu_device *smmu,
					 u32 prod)
{
	struct arm_smmu_queue *q = &smmu->cmdq.q;
	struct arm_smmu_cmdq_ent ent = {
		.opcode = CMDQ_OP_CMD_SYNC,
	};

	/*
	 * Beware that Hi16xx adds an extra 32 bits of goodness to its MSI
	 * payload, so the write will zero the entire command on that platform.
	 */
	if (smmu->features & ARM_SMMU_FEAT_MSI &&
	    smmu->features & ARM_SMMU_FEAT_COHERENCY) {
		ent.sync.msiaddr = q->base_dma + Q_IDX(&q->llq, prod) *
				   q->ent_dwords * 8;
	}

	arm_smmu_cmdq_build_cmd(cmd, &ent);
}

static void arm_smmu_cmdq_skip_err(struct arm_smmu_device *smmu)
{
	static const char *cerror_str[] = {
		[CMDQ_ERR_CERROR_NONE_IDX]	= "No error",
		[CMDQ_ERR_CERROR_ILL_IDX]	= "Illegal command",
		[CMDQ_ERR_CERROR_ABT_IDX]	= "Abort on command fetch",
		[CMDQ_ERR_CERROR_ATC_INV_IDX]	= "ATC invalidate timeout",
	};

	int i;
	u64 cmd[CMDQ_ENT_DWORDS];
	struct arm_smmu_queue *q = &smmu->cmdq.q;
	u32 cons = readl_relaxed(q->cons_reg);
	u32 idx = FIELD_GET(CMDQ_CONS_ERR, cons);
	struct arm_smmu_cmdq_ent cmd_sync = {
		.opcode = CMDQ_OP_CMD_SYNC,
	};

	dev_err(smmu->dev, "CMDQ error (cons 0x%08x): %s\n", cons,
		idx < ARRAY_SIZE(cerror_str) ?  cerror_str[idx] : "Unknown");

	switch (idx) {
	case CMDQ_ERR_CERROR_ABT_IDX:
		dev_err(smmu->dev, "retrying command fetch\n");
	case CMDQ_ERR_CERROR_NONE_IDX:
		return;
	case CMDQ_ERR_CERROR_ATC_INV_IDX:
		/*
		 * ATC Invalidation Completion timeout. CONS is still pointing
		 * at the CMD_SYNC. Attempt to complete other pending commands
		 * by repeating the CMD_SYNC, though we might well end up back
		 * here since the ATC invalidation may still be pending.
		 */
		return;
	case CMDQ_ERR_CERROR_ILL_IDX:
		/* Fallthrough */
	default:
		break;
	}

	/*
	 * We may have concurrent producers, so we need to be careful
	 * not to touch any of the shadow cmdq state.
	 */
	queue_read(cmd, Q_ENT(q, cons), q->ent_dwords);
	dev_err(smmu->dev, "skipping command in error state:\n");
	for (i = 0; i < ARRAY_SIZE(cmd); ++i)
		dev_err(smmu->dev, "\t0x%016llx\n", (unsigned long long)cmd[i]);

	/* Convert the erroneous command into a CMD_SYNC */
	if (arm_smmu_cmdq_build_cmd(cmd, &cmd_sync)) {
		dev_err(smmu->dev, "failed to convert to CMD_SYNC\n");
		return;
	}

	queue_write(Q_ENT(q, cons), cmd, q->ent_dwords);
}

/*
 * Command queue locking.
 * This is a form of bastardised rwlock with the following major changes:
 *
 * - The only LOCK routines are exclusive_trylock() and shared_lock().
 *   Neither have barrier semantics, and instead provide only a control
 *   dependency.
 *
 * - The UNLOCK routines are supplemented with shared_tryunlock(), which
 *   fails if the caller appears to be the last lock holder (yes, this is
 *   racy). All successful UNLOCK routines have RELEASE semantics.
 */
static void arm_smmu_cmdq_shared_lock(struct arm_smmu_cmdq *cmdq)
{
	int val;

	/*
	 * We can try to avoid the cmpxchg() loop by simply incrementing the
	 * lock counter. When held in exclusive state, the lock counter is set
	 * to INT_MIN so these increments won't hurt as the value will remain
	 * negative.
	 */
	if (atomic_fetch_inc_relaxed(&cmdq->lock) >= 0)
		return;

	do {
		val = atomic_cond_read_relaxed(&cmdq->lock, VAL >= 0);
	} while (atomic_cmpxchg_relaxed(&cmdq->lock, val, val + 1) != val);
}

static void arm_smmu_cmdq_shared_unlock(struct arm_smmu_cmdq *cmdq)
{
	(void)atomic_dec_return_release(&cmdq->lock);
}

static bool arm_smmu_cmdq_shared_tryunlock(struct arm_smmu_cmdq *cmdq)
{
	if (atomic_read(&cmdq->lock) == 1)
		return false;

	arm_smmu_cmdq_shared_unlock(cmdq);
	return true;
}

#define arm_smmu_cmdq_exclusive_trylock_irqsave(cmdq, flags)		\
({									\
	bool __ret;							\
	local_irq_save(flags);						\
	__ret = !atomic_cmpxchg_relaxed(&cmdq->lock, 0, INT_MIN);	\
	if (!__ret)							\
		local_irq_restore(flags);				\
	__ret;								\
})

#define arm_smmu_cmdq_exclusive_unlock_irqrestore(cmdq, flags)		\
({									\
	atomic_set_release(&cmdq->lock, 0);				\
	local_irq_restore(flags);					\
})


/*
 * Command queue insertion.
 * This is made fiddly by our attempts to achieve some sort of scalability
 * since there is one queue shared amongst all of the CPUs in the system.  If
 * you like mixed-size concurrency, dependency ordering and relaxed atomics,
 * then you'll *love* this monstrosity.
 *
 * The basic idea is to split the queue up into ranges of commands that are
 * owned by a given CPU; the owner may not have written all of the commands
 * itself, but is responsible for advancing the hardware prod pointer when
 * the time comes. The algorithm is roughly:
 *
 * 	1. Allocate some space in the queue. At this point we also discover
 *	   whether the head of the queue is currently owned by another CPU,
 *	   or whether we are the owner.
 *
 *	2. Write our commands into our allocated slots in the queue.
 *
 *	3. Mark our slots as valid in arm_smmu_cmdq.valid_map.
 *
 *	4. If we are an owner:
 *		a. Wait for the previous owner to finish.
 *		b. Mark the queue head as unowned, which tells us the range
 *		   that we are responsible for publishing.
 *		c. Wait for all commands in our owned range to become valid.
 *		d. Advance the hardware prod pointer.
 *		e. Tell the next owner we've finished.
 *
 *	5. If we are inserting a CMD_SYNC (we may or may not have been an
 *	   owner), then we need to stick around until it has completed:
 *		a. If we have MSIs, the SMMU can write back into the CMD_SYNC
 *		   to clear the first 4 bytes.
 *		b. Otherwise, we spin waiting for the hardware cons pointer to
 *		   advance past our command.
 *
 * The devil is in the details, particularly the use of locking for handling
 * SYNC completion and freeing up space in the queue before we think that it is
 * full.
 */
static void __arm_smmu_cmdq_poll_set_valid_map(struct arm_smmu_cmdq *cmdq,
					       u32 sprod, u32 eprod, bool set)
{
	u32 swidx, sbidx, ewidx, ebidx;
	struct arm_smmu_ll_queue llq = {
		.max_n_shift	= cmdq->q.llq.max_n_shift,
		.prod		= sprod,
	};

	ewidx = BIT_WORD(Q_IDX(&llq, eprod));
	ebidx = Q_IDX(&llq, eprod) % BITS_PER_LONG;

	while (llq.prod != eprod) {
		unsigned long mask;
		atomic_long_t *ptr;
		u32 limit = BITS_PER_LONG;

		swidx = BIT_WORD(Q_IDX(&llq, llq.prod));
		sbidx = Q_IDX(&llq, llq.prod) % BITS_PER_LONG;

		ptr = &cmdq->valid_map[swidx];

		if ((swidx == ewidx) && (sbidx < ebidx))
			limit = ebidx;

		mask = GENMASK(limit - 1, sbidx);

		/*
		 * The valid bit is the inverse of the wrap bit. This means
		 * that a zero-initialised queue is invalid and, after marking
		 * all entries as valid, they become invalid again when we
		 * wrap.
		 */
		if (set) {
			atomic_long_xor(mask, ptr);
		} else { /* Poll */
			unsigned long valid;

			valid = (ULONG_MAX + !!Q_WRP(&llq, llq.prod)) & mask;
			atomic_long_cond_read_relaxed(ptr, (VAL & mask) == valid);
		}

		llq.prod = queue_inc_prod_n(&llq, limit - sbidx);
	}
}

/* Mark all entries in the range [sprod, eprod) as valid */
static void arm_smmu_cmdq_set_valid_map(struct arm_smmu_cmdq *cmdq,
					u32 sprod, u32 eprod)
{
	__arm_smmu_cmdq_poll_set_valid_map(cmdq, sprod, eprod, true);
}

/* Wait for all entries in the range [sprod, eprod) to become valid */
static void arm_smmu_cmdq_poll_valid_map(struct arm_smmu_cmdq *cmdq,
					 u32 sprod, u32 eprod)
{
	__arm_smmu_cmdq_poll_set_valid_map(cmdq, sprod, eprod, false);
}

/* Wait for the command queue to become non-full */
static int arm_smmu_cmdq_poll_until_not_full(struct arm_smmu_device *smmu,
					     struct arm_smmu_ll_queue *llq)
{
	unsigned long flags;
	struct arm_smmu_queue_poll qp;
	struct arm_smmu_cmdq *cmdq = &smmu->cmdq;
	int ret = 0;

	/*
	 * Try to update our copy of cons by grabbing exclusive cmdq access. If
	 * that fails, spin until somebody else updates it for us.
	 */
	if (arm_smmu_cmdq_exclusive_trylock_irqsave(cmdq, flags)) {
		WRITE_ONCE(cmdq->q.llq.cons, readl_relaxed(cmdq->q.cons_reg));
		arm_smmu_cmdq_exclusive_unlock_irqrestore(cmdq, flags);
		llq->val = READ_ONCE(cmdq->q.llq.val);
		return 0;
	}

	queue_poll_init(smmu, &qp);
	do {
		llq->val = READ_ONCE(smmu->cmdq.q.llq.val);
		if (!queue_full(llq))
			break;

		ret = queue_poll(&qp);
	} while (!ret);

	return ret;
}

/*
 * Wait until the SMMU signals a CMD_SYNC completion MSI.
 * Must be called with the cmdq lock held in some capacity.
 */
static int __arm_smmu_cmdq_poll_until_msi(struct arm_smmu_device *smmu,
					  struct arm_smmu_ll_queue *llq)
{
	int ret = 0;
	struct arm_smmu_queue_poll qp;
	struct arm_smmu_cmdq *cmdq = &smmu->cmdq;
	u32 *cmd = (u32 *)(Q_ENT(&cmdq->q, llq->prod));

	queue_poll_init(smmu, &qp);

	/*
	 * The MSI won't generate an event, since it's being written back
	 * into the command queue.
	 */
	qp.wfe = false;
	smp_cond_load_relaxed(cmd, !VAL || (ret = queue_poll(&qp)));
	llq->cons = ret ? llq->prod : queue_inc_prod_n(llq, 1);
	return ret;
}

/*
 * Wait until the SMMU cons index passes llq->prod.
 * Must be called with the cmdq lock held in some capacity.
 */
static int __arm_smmu_cmdq_poll_until_consumed(struct arm_smmu_device *smmu,
					       struct arm_smmu_ll_queue *llq)
{
	struct arm_smmu_queue_poll qp;
	struct arm_smmu_cmdq *cmdq = &smmu->cmdq;
	u32 prod = llq->prod;
	int ret = 0;

	queue_poll_init(smmu, &qp);
	llq->val = READ_ONCE(smmu->cmdq.q.llq.val);
	do {
		if (queue_consumed(llq, prod))
			break;

		ret = queue_poll(&qp);

		/*
		 * This needs to be a readl() so that our subsequent call
		 * to arm_smmu_cmdq_shared_tryunlock() can fail accurately.
		 *
		 * Specifically, we need to ensure that we observe all
		 * shared_lock()s by other CMD_SYNCs that share our owner,
		 * so that a failing call to tryunlock() means that we're
		 * the last one out and therefore we can safely advance
		 * cmdq->q.llq.cons. Roughly speaking:
		 *
		 * CPU 0		CPU1			CPU2 (us)
		 *
		 * if (sync)
		 * 	shared_lock();
		 *
		 * dma_wmb();
		 * set_valid_map();
		 *
		 * 			if (owner) {
		 *				poll_valid_map();
		 *				<control dependency>
		 *				writel(prod_reg);
		 *
		 *						readl(cons_reg);
		 *						tryunlock();
		 *
		 * Requires us to see CPU 0's shared_lock() acquisition.
		 */
		llq->cons = readl(cmdq->q.cons_reg);
	} while (!ret);

	return ret;
}

static int arm_smmu_cmdq_poll_until_sync(struct arm_smmu_device *smmu,
					 struct arm_smmu_ll_queue *llq)
{
	if (smmu->features & ARM_SMMU_FEAT_MSI &&
	    smmu->features & ARM_SMMU_FEAT_COHERENCY)
		return __arm_smmu_cmdq_poll_until_msi(smmu, llq);

	return __arm_smmu_cmdq_poll_until_consumed(smmu, llq);
}

static void arm_smmu_cmdq_write_entries(struct arm_smmu_cmdq *cmdq, u64 *cmds,
					u32 prod, int n)
{
	int i;
	struct arm_smmu_ll_queue llq = {
		.max_n_shift	= cmdq->q.llq.max_n_shift,
		.prod		= prod,
	};

	for (i = 0; i < n; ++i) {
		u64 *cmd = &cmds[i * CMDQ_ENT_DWORDS];

		prod = queue_inc_prod_n(&llq, i);
		queue_write(Q_ENT(&cmdq->q, prod), cmd, CMDQ_ENT_DWORDS);
	}
}

/*
 * This is the actual insertion function, and provides the following
 * ordering guarantees to callers:
 *
 * - There is a dma_wmb() before publishing any commands to the queue.
 *   This can be relied upon to order prior writes to data structures
 *   in memory (such as a CD or an STE) before the command.
 *
 * - On completion of a CMD_SYNC, there is a control dependency.
 *   This can be relied upon to order subsequent writes to memory (e.g.
 *   freeing an IOVA) after completion of the CMD_SYNC.
 *
 * - Command insertion is totally ordered, so if two CPUs each race to
 *   insert their own list of commands then all of the commands from one
 *   CPU will appear before any of the commands from the other CPU.
 */
static int arm_smmu_cmdq_issue_cmdlist(struct arm_smmu_device *smmu,
				       u64 *cmds, int n, bool sync)
{
	u64 cmd_sync[CMDQ_ENT_DWORDS];
	u32 prod;
	unsigned long flags;
	bool owner;
	struct arm_smmu_cmdq *cmdq = &smmu->cmdq;
	struct arm_smmu_ll_queue llq = {
		.max_n_shift = cmdq->q.llq.max_n_shift,
	}, head = llq;
	int ret = 0;

	/* 1. Allocate some space in the queue */
	local_irq_save(flags);
	llq.val = READ_ONCE(cmdq->q.llq.val);
	do {
		u64 old;

		while (!queue_has_space(&llq, n + sync)) {
			local_irq_restore(flags);
			if (arm_smmu_cmdq_poll_until_not_full(smmu, &llq))
				dev_err_ratelimited(smmu->dev, "CMDQ timeout\n");
			local_irq_save(flags);
		}

		head.cons = llq.cons;
		head.prod = queue_inc_prod_n(&llq, n + sync) |
					     CMDQ_PROD_OWNED_FLAG;

		old = cmpxchg_relaxed(&cmdq->q.llq.val, llq.val, head.val);
		if (old == llq.val)
			break;

		llq.val = old;
	} while (1);
	owner = !(llq.prod & CMDQ_PROD_OWNED_FLAG);
	head.prod &= ~CMDQ_PROD_OWNED_FLAG;
	llq.prod &= ~CMDQ_PROD_OWNED_FLAG;

	/*
	 * 2. Write our commands into the queue
	 * Dependency ordering from the cmpxchg() loop above.
	 */
	arm_smmu_cmdq_write_entries(cmdq, cmds, llq.prod, n);
	if (sync) {
		prod = queue_inc_prod_n(&llq, n);
		arm_smmu_cmdq_build_sync_cmd(cmd_sync, smmu, prod);
		queue_write(Q_ENT(&cmdq->q, prod), cmd_sync, CMDQ_ENT_DWORDS);

		/*
		 * In order to determine completion of our CMD_SYNC, we must
		 * ensure that the queue can't wrap twice without us noticing.
		 * We achieve that by taking the cmdq lock as shared before
		 * marking our slot as valid.
		 */
		arm_smmu_cmdq_shared_lock(cmdq);
	}

	/* 3. Mark our slots as valid, ensuring commands are visible first */
	dma_wmb();
	arm_smmu_cmdq_set_valid_map(cmdq, llq.prod, head.prod);

	/* 4. If we are the owner, take control of the SMMU hardware */
	if (owner) {
		/* a. Wait for previous owner to finish */
		atomic_cond_read_relaxed(&cmdq->owner_prod, VAL == llq.prod);

		/* b. Stop gathering work by clearing the owned flag */
		prod = atomic_fetch_andnot_relaxed(CMDQ_PROD_OWNED_FLAG,
						   &cmdq->q.llq.atomic.prod);
		prod &= ~CMDQ_PROD_OWNED_FLAG;

		/*
		 * c. Wait for any gathered work to be written to the queue.
		 * Note that we read our own entries so that we have the control
		 * dependency required by (d).
		 */
		arm_smmu_cmdq_poll_valid_map(cmdq, llq.prod, prod);

		/*
		 * d. Advance the hardware prod pointer
		 * Control dependency ordering from the entries becoming valid.
		 */
		writel_relaxed(prod, cmdq->q.prod_reg);

		/*
		 * e. Tell the next owner we're done
		 * Make sure we've updated the hardware first, so that we don't
		 * race to update prod and potentially move it backwards.
		 */
		atomic_set_release(&cmdq->owner_prod, prod);
	}

	/* 5. If we are inserting a CMD_SYNC, we must wait for it to complete */
	if (sync) {
		llq.prod = queue_inc_prod_n(&llq, n);
		ret = arm_smmu_cmdq_poll_until_sync(smmu, &llq);
		if (ret) {
			dev_err_ratelimited(smmu->dev,
					    "CMD_SYNC timeout at 0x%08x [hwprod 0x%08x, hwcons 0x%08x]\n",
					    llq.prod,
					    readl_relaxed(cmdq->q.prod_reg),
					    readl_relaxed(cmdq->q.cons_reg));
		}

		/*
		 * Try to unlock the cmq lock. This will fail if we're the last
		 * reader, in which case we can safely update cmdq->q.llq.cons
		 */
		if (!arm_smmu_cmdq_shared_tryunlock(cmdq)) {
			WRITE_ONCE(cmdq->q.llq.cons, llq.cons);
			arm_smmu_cmdq_shared_unlock(cmdq);
		}
	}

	local_irq_restore(flags);
	return ret;
}

static int arm_smmu_cmdq_issue_cmd(struct arm_smmu_device *smmu,
				   struct arm_smmu_cmdq_ent *ent)
{
	u64 cmd[CMDQ_ENT_DWORDS];

	if (arm_smmu_cmdq_build_cmd(cmd, ent)) {
		dev_warn(smmu->dev, "ignoring unknown CMDQ opcode 0x%x\n",
			 ent->opcode);
		return -EINVAL;
	}

	return arm_smmu_cmdq_issue_cmdlist(smmu, cmd, 1, false);
}

static int arm_smmu_cmdq_issue_sync(struct arm_smmu_device *smmu)
{
	return arm_smmu_cmdq_issue_cmdlist(smmu, NULL, 0, true);
}

static void arm_smmu_cmdq_batch_add(struct arm_smmu_device *smmu,
				    struct arm_smmu_cmdq_batch *cmds,
				    struct arm_smmu_cmdq_ent *cmd)
{
	if (cmds->num == CMDQ_BATCH_ENTRIES) {
		arm_smmu_cmdq_issue_cmdlist(smmu, cmds->cmds, cmds->num, false);
		cmds->num = 0;
	}
	arm_smmu_cmdq_build_cmd(&cmds->cmds[cmds->num * CMDQ_ENT_DWORDS], cmd);
	cmds->num++;
}

static int arm_smmu_cmdq_batch_submit(struct arm_smmu_device *smmu,
				      struct arm_smmu_cmdq_batch *cmds)
{
	return arm_smmu_cmdq_issue_cmdlist(smmu, cmds->cmds, cmds->num, true);
}

static int arm_smmu_page_response(struct device *dev,
				  struct iommu_fault_event *unused,
				  struct iommu_page_response *resp)
{
	struct arm_smmu_cmdq_ent cmd = {0};
	struct arm_smmu_master *master = dev_iommu_fwspec_get(dev)->iommu_priv;
	bool pasid_valid = resp->flags & IOMMU_PAGE_RESP_PASID_VALID;
	int sid = master->streams[0].id;

	if (master->stall_enabled) {
		cmd.opcode		= CMDQ_OP_RESUME;
		cmd.resume.sid		= sid;
		cmd.resume.stag		= resp->grpid;
		switch (resp->code) {
		case IOMMU_PAGE_RESP_INVALID:
		case IOMMU_PAGE_RESP_FAILURE:
			cmd.resume.resp = CMDQ_RESUME_0_RESP_ABORT;
			break;
		case IOMMU_PAGE_RESP_SUCCESS:
			cmd.resume.resp = CMDQ_RESUME_0_RESP_RETRY;
			break;
		default:
			return -EINVAL;
		}
	} else if (master->pri_supported) {
		cmd.opcode		= CMDQ_OP_PRI_RESP;
		cmd.substream_valid	= pasid_valid &&
					  master->prg_resp_needs_ssid;
		cmd.pri.sid		= sid;
		cmd.pri.ssid		= resp->pasid;
		cmd.pri.grpid		= resp->grpid;
		switch (resp->code) {
		case IOMMU_PAGE_RESP_FAILURE:
			cmd.pri.resp = CMDQ_PRI_1_RESP_FAILURE;
			break;
		case IOMMU_PAGE_RESP_INVALID:
			cmd.pri.resp = CMDQ_PRI_1_RESP_INVALID;
			break;
		case IOMMU_PAGE_RESP_SUCCESS:
			cmd.pri.resp = CMDQ_PRI_1_RESP_SUCCESS;
			break;
		default:
			return -EINVAL;
		}
	} else {
		return -ENODEV;
	}

	arm_smmu_cmdq_issue_cmd(master->smmu, &cmd);
	/*
	 * Don't send a SYNC, it doesn't do anything for RESUME or PRI_RESP.
	 * RESUME consumption guarantees that the stalled transaction will be
	 * terminated... at some point in the future. PRI_RESP is fire and
	 * forget.
	 */

	return 0;
}

/* Context descriptor manipulation functions */
static void arm_smmu_tlb_inv_asid(struct arm_smmu_device *smmu, u16 asid)
{
	struct arm_smmu_cmdq_ent cmd = {
		.opcode	= smmu->features & ARM_SMMU_FEAT_E2H ?
			CMDQ_OP_TLBI_EL2_ASID : CMDQ_OP_TLBI_NH_ASID,
		.tlbi.asid = asid,
	};

	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	arm_smmu_cmdq_issue_sync(smmu);
}

static void arm_smmu_sync_cd(struct arm_smmu_domain *smmu_domain,
			     int ssid, bool leaf)
{
	size_t i;
	unsigned long flags;
	struct arm_smmu_master *master;
	struct arm_smmu_cmdq_batch cmds = {};
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_cmdq_ent cmd = {
		.opcode	= CMDQ_OP_CFGI_CD,
		.cfgi	= {
			.ssid	= ssid,
			.leaf	= leaf,
		},
	};

	spin_lock_irqsave(&smmu_domain->devices_lock, flags);
	list_for_each_entry(master, &smmu_domain->devices, domain_head) {
		for (i = 0; i < master->num_streams; i++) {
			cmd.cfgi.sid = master->streams[i].id;
			arm_smmu_cmdq_batch_add(smmu, &cmds, &cmd);
		}
	}
	spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);

	arm_smmu_cmdq_batch_submit(smmu, &cmds);
}

static int arm_smmu_alloc_cd_leaf_table(struct arm_smmu_device *smmu,
					struct arm_smmu_l1_ctx_desc *l1_desc)
{
	size_t size = CTXDESC_L2_ENTRIES * (CTXDESC_CD_DWORDS << 3);

	l1_desc->l2ptr = dmam_alloc_coherent(smmu->dev, size,
					     &l1_desc->l2ptr_dma, GFP_ATOMIC);
	if (!l1_desc->l2ptr) {
		dev_warn(smmu->dev,
			 "failed to allocate context descriptor table\n");
		return -ENOMEM;
	}
	return 0;
}

static void arm_smmu_write_cd_l1_desc(__le64 *dst,
				      struct arm_smmu_l1_ctx_desc *l1_desc)
{
	u64 val = (l1_desc->l2ptr_dma & CTXDESC_L1_DESC_L2PTR_MASK) |
		  CTXDESC_L1_DESC_V;

	/* See comment in arm_smmu_write_ctx_desc() */
	WRITE_ONCE(*dst, cpu_to_le64(val));
}

static __le64 *arm_smmu_get_cd_ptr(struct arm_smmu_domain *smmu_domain,
				   u32 ssid)
{
	__le64 *l1ptr;
	unsigned int idx;
	struct arm_smmu_l1_ctx_desc *l1_desc;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_ctx_desc_cfg *cdcfg = &smmu_domain->s1_cfg.cdcfg;

	if (smmu_domain->s1_cfg.s1fmt == STRTAB_STE_0_S1FMT_LINEAR)
		return cdcfg->cdtab + ssid * CTXDESC_CD_DWORDS;

	idx = ssid >> CTXDESC_SPLIT;
	l1_desc = &cdcfg->l1_desc[idx];
	if (!l1_desc->l2ptr) {
		if (arm_smmu_alloc_cd_leaf_table(smmu, l1_desc))
			return NULL;

		l1ptr = cdcfg->cdtab + idx * CTXDESC_L1_DESC_DWORDS;
		arm_smmu_write_cd_l1_desc(l1ptr, l1_desc);
		/* An invalid L1CD can be cached */
		arm_smmu_sync_cd(smmu_domain, ssid, false);
	}
	idx = ssid & (CTXDESC_L2_ENTRIES - 1);
	return l1_desc->l2ptr + idx * CTXDESC_CD_DWORDS;
}

static int __arm_smmu_write_ctx_desc(struct arm_smmu_domain *smmu_domain,
				     int ssid, struct arm_smmu_ctx_desc *cd)
{
	/*
	 * This function handles the following cases:
	 *
	 * (1) Install primary CD, for normal DMA traffic (SSID = 0).
	 * (2) Install a secondary CD, for SID+SSID traffic.
	 * (3) Update ASID of a CD. Atomically write the first 64 bits of the
	 *     CD, then invalidate the old entry and mappings.
	 * (4) Remove a secondary CD.
	 */
	u64 val;
	bool cd_live;
	__le64 *cdptr;
	struct arm_smmu_device *smmu = smmu_domain->smmu;

	if (WARN_ON(ssid >= (1 << smmu_domain->s1_cfg.s1cdmax)))
		return -E2BIG;

	cdptr = arm_smmu_get_cd_ptr(smmu_domain, ssid);
	if (!cdptr)
		return -ENOMEM;

	val = le64_to_cpu(cdptr[0]);
	cd_live = !!(val & CTXDESC_CD_0_V);

	if (!cd) { /* (4) */
		val = 0;
	} else if (cd_live) { /* (3) */
		val &= ~CTXDESC_CD_0_ASID;
		val |= FIELD_PREP(CTXDESC_CD_0_ASID, cd->asid);
		/*
		 * Until CD+TLB invalidation, both ASIDs may be used for tagging
		 * this substream's traffic
		 */
	} else { /* (1) and (2) */
		u64 tcr = cd->tcr;

		cdptr[1] = cpu_to_le64(cd->ttbr & CTXDESC_CD_1_TTB0_MASK);
		cdptr[2] = 0;
		cdptr[3] = cpu_to_le64(cd->mair);

		if (!(smmu->features & ARM_SMMU_FEAT_HD))
			tcr &= ~CTXDESC_CD_0_TCR_HD;
		if (!(smmu->features & ARM_SMMU_FEAT_HA))
			tcr &= ~CTXDESC_CD_0_TCR_HA;

		/*
		 * STE is live, and the SMMU might read dwords of this CD in any
		 * order. Ensure that it observes valid values before reading
		 * V=1.
		 */
		arm_smmu_sync_cd(smmu_domain, ssid, true);

		val = tcr |
#ifdef __BIG_ENDIAN
			CTXDESC_CD_0_ENDI |
#endif
			CTXDESC_CD_0_R | CTXDESC_CD_0_A |
			(cd->mm ? 0 : CTXDESC_CD_0_ASET) |
			CTXDESC_CD_0_AA64 |
			FIELD_PREP(CTXDESC_CD_0_ASID, cd->asid) |
			CTXDESC_CD_0_V;

		if (smmu_domain->stall_enabled)
			val |= CTXDESC_CD_0_S;
	}

	/*
	 * The SMMU accesses 64-bit values atomically. See IHI0070Ca 3.21.3
	 * "Configuration structures and configuration invalidation completion"
	 *
	 *   The size of single-copy atomic reads made by the SMMU is
	 *   IMPLEMENTATION DEFINED but must be at least 64 bits. Any single
	 *   field within an aligned 64-bit span of a structure can be altered
	 *   without first making the structure invalid.
	 */
	WRITE_ONCE(cdptr[0], cpu_to_le64(val));
	arm_smmu_sync_cd(smmu_domain, ssid, true);
	return 0;
}

static int arm_smmu_write_ctx_desc(struct arm_smmu_domain *smmu_domain,
				   int ssid, struct arm_smmu_ctx_desc *cd)
{
	int ret;

	spin_lock(&contexts_lock);
	ret = __arm_smmu_write_ctx_desc(smmu_domain, ssid, cd);
	spin_unlock(&contexts_lock);
	return ret;
}

static int arm_smmu_alloc_cd_tables(struct arm_smmu_domain *smmu_domain)
{
	int ret;
	size_t l1size;
	size_t max_contexts;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;
	struct arm_smmu_ctx_desc_cfg *cdcfg = &cfg->cdcfg;

	max_contexts = 1 << cfg->s1cdmax;

	if (!(smmu->features & ARM_SMMU_FEAT_2_LVL_CDTAB) ||
	    max_contexts <= CTXDESC_L2_ENTRIES) {
		cfg->s1fmt = STRTAB_STE_0_S1FMT_LINEAR;
		cdcfg->num_l1_ents = max_contexts;

		l1size = max_contexts * (CTXDESC_CD_DWORDS << 3);
	} else {
		cfg->s1fmt = STRTAB_STE_0_S1FMT_64K_L2;
		cdcfg->num_l1_ents = DIV_ROUND_UP(max_contexts,
						  CTXDESC_L2_ENTRIES);

		cdcfg->l1_desc = devm_kcalloc(smmu->dev, cdcfg->num_l1_ents,
					      sizeof(*cdcfg->l1_desc),
					      GFP_KERNEL);
		if (!cdcfg->l1_desc)
			return -ENOMEM;

		l1size = cdcfg->num_l1_ents * (CTXDESC_L1_DESC_DWORDS << 3);
	}

	cdcfg->cdtab = dmam_alloc_coherent(smmu->dev, l1size, &cdcfg->cdtab_dma,
					   GFP_KERNEL);
	if (!cdcfg->cdtab) {
		dev_warn(smmu->dev, "failed to allocate context descriptor\n");
		ret = -ENOMEM;
		goto err_free_l1;
	}

	return 0;

err_free_l1:
	if (cdcfg->l1_desc) {
		devm_kfree(smmu->dev, cdcfg->l1_desc);
		cdcfg->l1_desc = NULL;
	}
	return ret;
}

static void arm_smmu_free_cd_tables(struct arm_smmu_domain *smmu_domain)
{
	int i;
	size_t size, l1size;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_ctx_desc_cfg *cdcfg = &smmu_domain->s1_cfg.cdcfg;

	if (!cdcfg->cdtab)
		return;

	if (cdcfg->l1_desc) {
		size = CTXDESC_L2_ENTRIES * (CTXDESC_CD_DWORDS << 3);

		for (i = 0; i < cdcfg->num_l1_ents; i++) {
			if (!cdcfg->l1_desc[i].l2ptr)
				continue;

			dmam_free_coherent(smmu->dev, size,
					   cdcfg->l1_desc[i].l2ptr,
					   cdcfg->l1_desc[i].l2ptr_dma);
		}
		devm_kfree(smmu->dev, cdcfg->l1_desc);
		cdcfg->l1_desc = NULL;

		l1size = cdcfg->num_l1_ents * (CTXDESC_L1_DESC_DWORDS << 3);
	} else {
		l1size = cdcfg->num_l1_ents * (CTXDESC_CD_DWORDS << 3);
	}

	dmam_free_coherent(smmu->dev, l1size, cdcfg->cdtab, cdcfg->cdtab_dma);
	cdcfg->cdtab_dma = 0;
	cdcfg->cdtab = NULL;
}

static void arm_smmu_init_cd(struct arm_smmu_ctx_desc *cd)
{
	refcount_set(&cd->refs, 1);
}

static bool arm_smmu_free_asid(struct arm_smmu_ctx_desc *cd)
{
	bool free;
	struct arm_smmu_ctx_desc *old_cd;

	if (!cd->asid)
		return false;

	xa_lock(&asid_xa);
	free = refcount_dec_and_test(&cd->refs);
	if (free) {
		old_cd = __xa_erase(&asid_xa, cd->asid);
		WARN_ON(old_cd != cd);
	}
	xa_unlock(&asid_xa);
	return free;
}

/*
 * Try to reserve this ASID in the SMMU. If it is in use, try to steal it from
 * the private entry. Careful here, we may be modifying the context tables of
 * another SMMU!
 */
static struct arm_smmu_ctx_desc *arm_smmu_share_asid(u16 asid)
{
	int ret;
	u32 new_asid;
	struct arm_smmu_ctx_desc *cd;
	struct arm_smmu_device *smmu;
	struct arm_smmu_domain *smmu_domain;

	cd = xa_load(&asid_xa, asid);
	if (!cd)
		return NULL;

	if (cd->mm) {
		/*
		 * It's pretty common to find a stale CD when doing unbind-bind,
		 * given that the release happens after a RCU grace period.
		 * arm_smmu_free_asid() hasn't gone through yet, so reuse it.
		 */
		refcount_inc(&cd->refs);
		return cd;
	}

	smmu_domain = container_of(cd, struct arm_smmu_domain, s1_cfg.cd);
	smmu = smmu_domain->smmu;

	/*
	 * Race with unmap: TLB invalidations will start targeting the new ASID,
	 * which isn't assigned yet. We'll do an invalidate-all on the old ASID
	 * later, so it doesn't matter.
	 */
	ret = __xa_alloc(&asid_xa, &new_asid, cd,
			 XA_LIMIT(1, 1 << smmu->asid_bits), GFP_ATOMIC);
	if (ret)
		return ERR_PTR(-ENOSPC);
	cd->asid = new_asid;

	/*
	 * Update ASID and invalidate CD in all associated masters. There will
	 * be some overlap between use of both ASIDs, until we invalidate the
	 * TLB.
	 */
	arm_smmu_write_ctx_desc(smmu_domain, 0, cd);

	/* Invalidate TLB entries previously associated with that context */
	arm_smmu_tlb_inv_asid(smmu, asid);

	return NULL;
}

static struct arm_smmu_ctx_desc *arm_smmu_alloc_shared_cd(struct mm_struct *mm)
{
	u16 asid;
	int ret = 0;
	u64 tcr, par, reg;
	struct arm_smmu_ctx_desc *cd;
	struct arm_smmu_ctx_desc *old_cd = NULL;

	asid = mm_context_get(mm);
	if (!asid)
		return ERR_PTR(-ESRCH);

	cd = kzalloc(sizeof(*cd), GFP_KERNEL);
	if (!cd) {
		ret = -ENOMEM;
		goto err_put_context;
	}

	arm_smmu_init_cd(cd);

	xa_lock(&asid_xa);
	old_cd = arm_smmu_share_asid(asid);
	if (!old_cd) {
		old_cd = __xa_store(&asid_xa, asid, cd, GFP_ATOMIC);
		/*
		 * Keep error, ignore valid pointers. If there was an old entry
		 * it has been moved already by arm_smmu_share_asid().
		 */
		old_cd = ERR_PTR(xa_err(old_cd));
		cd->asid = asid;
	}
	xa_unlock(&asid_xa);

	if (IS_ERR(old_cd)) {
		ret = PTR_ERR(old_cd);
		goto err_free_cd;
	} else if (old_cd) {
		if (WARN_ON(old_cd->mm != mm)) {
			ret = -EINVAL;
			goto err_free_cd;
		}
		kfree(cd);
		mm_context_put(mm);
		return old_cd;
	}

	/* HA and HD will be filtered out later if not supported by the SMMU */
	tcr = FIELD_PREP(CTXDESC_CD_0_TCR_T0SZ, 64ULL - VA_BITS) |
	      FIELD_PREP(CTXDESC_CD_0_TCR_IRGN0, ARM_LPAE_TCR_RGN_WBWA) |
	      FIELD_PREP(CTXDESC_CD_0_TCR_ORGN0, ARM_LPAE_TCR_RGN_WBWA) |
	      FIELD_PREP(CTXDESC_CD_0_TCR_SH0, ARM_LPAE_TCR_SH_IS) |
	      CTXDESC_CD_0_TCR_HA | CTXDESC_CD_0_TCR_HD |
	      CTXDESC_CD_0_TCR_EPD1 | CTXDESC_CD_0_AA64;

	switch (PAGE_SIZE) {
		case SZ_4K:
			tcr |= FIELD_PREP(CTXDESC_CD_0_TCR_TG0,
					  ARM_LPAE_TCR_TG0_4K);
			break;
		case SZ_16K:
			tcr |= FIELD_PREP(CTXDESC_CD_0_TCR_TG0,
					  ARM_LPAE_TCR_TG0_16K);
			break;
		case SZ_64K:
			tcr |= FIELD_PREP(CTXDESC_CD_0_TCR_TG0,
					  ARM_LPAE_TCR_TG0_64K);
			break;
		default:
			WARN_ON(1);
			ret = -EINVAL;
			goto err_free_asid;
	}

	reg = read_sanitised_ftr_reg(SYS_ID_AA64MMFR0_EL1);
	par = cpuid_feature_extract_unsigned_field(reg, ID_AA64MMFR0_PARANGE_SHIFT);
	tcr |= FIELD_PREP(CTXDESC_CD_0_TCR_IPS, par);

	cd->ttbr = virt_to_phys(mm->pgd);
	cd->tcr = tcr;
	/*
	 * MAIR value is pretty much constant and global, so we can just get it
	 * from the current CPU register
	 */
	cd->mair = read_sysreg(mair_el1);

	cd->mm = mm;

	return cd;

err_free_asid:
	arm_smmu_free_asid(cd);
err_free_cd:
	kfree(cd);
err_put_context:
	mm_context_put(mm);
	return ERR_PTR(ret);
}

static void arm_smmu_free_shared_cd(struct arm_smmu_ctx_desc *cd)
{
	if (arm_smmu_free_asid(cd)) {
		/* Unpin ASID */
		mm_context_put(cd->mm);
		kfree(cd);
	}
}

/* Stream table manipulation functions */
static void
arm_smmu_write_strtab_l1_desc(__le64 *dst, struct arm_smmu_strtab_l1_desc *desc)
{
	u64 val = 0;

	val |= FIELD_PREP(STRTAB_L1_DESC_SPAN, desc->span);
	val |= desc->l2ptr_dma & STRTAB_L1_DESC_L2PTR_MASK;

	/* See comment in arm_smmu_write_ctx_desc() */
	WRITE_ONCE(*dst, cpu_to_le64(val));
}

static void arm_smmu_sync_ste_for_sid(struct arm_smmu_device *smmu, u32 sid)
{
	struct arm_smmu_cmdq_ent cmd = {
		.opcode	= CMDQ_OP_CFGI_STE,
		.cfgi	= {
			.sid	= sid,
			.leaf	= true,
		},
	};

	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	arm_smmu_cmdq_issue_sync(smmu);
}

static void arm_smmu_write_strtab_ent(struct arm_smmu_master *master, u32 sid,
				      __le64 *dst)
{
	/*
	 * This is hideously complicated, but we only really care about
	 * three cases at the moment:
	 *
	 * 1. Invalid (all zero) -> bypass/fault (init)
	 * 2. Bypass/fault -> translation/bypass (attach)
	 * 3. Translation/bypass -> bypass/fault (detach)
	 *
	 * Given that we can't update the STE atomically and the SMMU
	 * doesn't read the thing in a defined order, that leaves us
	 * with the following maintenance requirements:
	 *
	 * 1. Update Config, return (init time STEs aren't live)
	 * 2. Write everything apart from dword 0, sync, write dword 0, sync
	 * 3. Update Config, sync
	 */
	u64 val = le64_to_cpu(dst[0]);
	bool ste_live = false;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_s1_cfg *s1_cfg = NULL;
	struct arm_smmu_s2_cfg *s2_cfg = NULL;
	struct arm_smmu_domain *smmu_domain = NULL;
	struct arm_smmu_cmdq_ent prefetch_cmd = {
		.opcode		= CMDQ_OP_PREFETCH_CFG,
		.prefetch	= {
			.sid	= sid,
		},
	};

	if (master) {
		smmu_domain = master->domain;
		smmu = master->smmu;
	}

	if (smmu_domain) {
		switch (smmu_domain->stage) {
		case ARM_SMMU_DOMAIN_S1:
			s1_cfg = &smmu_domain->s1_cfg;
			break;
		case ARM_SMMU_DOMAIN_S2:
		case ARM_SMMU_DOMAIN_NESTED:
			s2_cfg = &smmu_domain->s2_cfg;
			break;
		default:
			break;
		}
	}

	if (val & STRTAB_STE_0_V) {
		switch (FIELD_GET(STRTAB_STE_0_CFG, val)) {
		case STRTAB_STE_0_CFG_BYPASS:
			break;
		case STRTAB_STE_0_CFG_S1_TRANS:
		case STRTAB_STE_0_CFG_S2_TRANS:
			ste_live = true;
			break;
		case STRTAB_STE_0_CFG_ABORT:
			BUG_ON(!disable_bypass);
			break;
		default:
			BUG(); /* STE corruption */
		}
	}

	/* Nuke the existing STE_0 value, as we're going to rewrite it */
	val = STRTAB_STE_0_V;

	/* Bypass/fault */
	if (!smmu_domain || !(s1_cfg || s2_cfg)) {
		if (!smmu_domain && disable_bypass)
			val |= FIELD_PREP(STRTAB_STE_0_CFG, STRTAB_STE_0_CFG_ABORT);
		else
			val |= FIELD_PREP(STRTAB_STE_0_CFG, STRTAB_STE_0_CFG_BYPASS);

		dst[0] = cpu_to_le64(val);
		dst[1] = cpu_to_le64(FIELD_PREP(STRTAB_STE_1_SHCFG,
						STRTAB_STE_1_SHCFG_INCOMING));
		dst[2] = 0; /* Nuke the VMID */
		/*
		 * The SMMU can perform negative caching, so we must sync
		 * the STE regardless of whether the old value was live.
		 */
		if (smmu)
			arm_smmu_sync_ste_for_sid(smmu, sid);
		return;
	}

	if (s1_cfg) {
		int strw = smmu->features & ARM_SMMU_FEAT_E2H ?
			STRTAB_STE_1_STRW_EL2 : STRTAB_STE_1_STRW_NSEL1;

		BUG_ON(ste_live);
		dst[1] = cpu_to_le64(
			 FIELD_PREP(STRTAB_STE_1_S1DSS, STRTAB_STE_1_S1DSS_SSID0) |
			 FIELD_PREP(STRTAB_STE_1_S1CIR, STRTAB_STE_1_S1C_CACHE_WBRA) |
			 FIELD_PREP(STRTAB_STE_1_S1COR, STRTAB_STE_1_S1C_CACHE_WBRA) |
			 FIELD_PREP(STRTAB_STE_1_S1CSH, ARM_SMMU_SH_ISH) |
			 FIELD_PREP(STRTAB_STE_1_STRW, strw));

		if (master->prg_resp_needs_ssid)
			dst[1] |= STRTAB_STE_1_PPAR;

		if (smmu->features & ARM_SMMU_FEAT_STALLS &&
		    !master->stall_enabled)
			dst[1] |= cpu_to_le64(STRTAB_STE_1_S1STALLD);

		val |= (s1_cfg->cdcfg.cdtab_dma & STRTAB_STE_0_S1CTXPTR_MASK) |
			FIELD_PREP(STRTAB_STE_0_CFG, STRTAB_STE_0_CFG_S1_TRANS) |
			FIELD_PREP(STRTAB_STE_0_S1CDMAX, s1_cfg->s1cdmax) |
			FIELD_PREP(STRTAB_STE_0_S1FMT, s1_cfg->s1fmt);
	}

	if (s2_cfg) {
		BUG_ON(ste_live);
		dst[2] = cpu_to_le64(
			 FIELD_PREP(STRTAB_STE_2_S2VMID, s2_cfg->vmid) |
			 FIELD_PREP(STRTAB_STE_2_VTCR, s2_cfg->vtcr) |
#ifdef __BIG_ENDIAN
			 STRTAB_STE_2_S2ENDI |
#endif
			 STRTAB_STE_2_S2PTW | STRTAB_STE_2_S2AA64 |
			 STRTAB_STE_2_S2R);

		dst[3] = cpu_to_le64(s2_cfg->vttbr & STRTAB_STE_3_S2TTB_MASK);

		val |= FIELD_PREP(STRTAB_STE_0_CFG, STRTAB_STE_0_CFG_S2_TRANS);
	}

	if (master->ats_enabled)
		dst[1] |= cpu_to_le64(FIELD_PREP(STRTAB_STE_1_EATS,
						 STRTAB_STE_1_EATS_TRANS));

	arm_smmu_sync_ste_for_sid(smmu, sid);
	/* See comment in arm_smmu_write_ctx_desc() */
	WRITE_ONCE(dst[0], cpu_to_le64(val));
	arm_smmu_sync_ste_for_sid(smmu, sid);

	/* It's likely that we'll want to use the new STE soon */
	if (!(smmu->options & ARM_SMMU_OPT_SKIP_PREFETCH))
		arm_smmu_cmdq_issue_cmd(smmu, &prefetch_cmd);
}

static void arm_smmu_init_bypass_stes(u64 *strtab, unsigned int nent)
{
	unsigned int i;

	for (i = 0; i < nent; ++i) {
		arm_smmu_write_strtab_ent(NULL, -1, strtab);
		strtab += STRTAB_STE_DWORDS;
	}
}

static int arm_smmu_init_l2_strtab(struct arm_smmu_device *smmu, u32 sid)
{
	size_t size;
	void *strtab;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;
	struct arm_smmu_strtab_l1_desc *desc = &cfg->l1_desc[sid >> STRTAB_SPLIT];

	if (desc->l2ptr)
		return 0;

	size = 1 << (STRTAB_SPLIT + ilog2(STRTAB_STE_DWORDS) + 3);
	strtab = &cfg->strtab[(sid >> STRTAB_SPLIT) * STRTAB_L1_DESC_DWORDS];

	desc->span = STRTAB_SPLIT + 1;
	desc->l2ptr = dmam_alloc_coherent(smmu->dev, size, &desc->l2ptr_dma,
					  GFP_KERNEL);
	if (!desc->l2ptr) {
		dev_err(smmu->dev,
			"failed to allocate l2 stream table for SID %u\n",
			sid);
		return -ENOMEM;
	}

	arm_smmu_init_bypass_stes(desc->l2ptr, 1 << STRTAB_SPLIT);
	arm_smmu_write_strtab_l1_desc(strtab, desc);
	return 0;
}

static struct arm_smmu_master *
arm_smmu_find_master(struct arm_smmu_device *smmu, u32 sid)
{
	struct rb_node *node;
	struct arm_smmu_stream *stream;
	struct arm_smmu_master *master = NULL;

	mutex_lock(&smmu->streams_mutex);
	node = smmu->streams.rb_node;
	while (node) {
		stream = rb_entry(node, struct arm_smmu_stream, node);
		if (stream->id < sid) {
			node = node->rb_right;
		} else if (stream->id > sid) {
			node = node->rb_left;
		} else {
			master = stream->master;
			break;
		}
	}
	mutex_unlock(&smmu->streams_mutex);

	return master;
}

/* IRQ and event handlers */
static int arm_smmu_handle_evt(struct arm_smmu_device *smmu, u64 *evt)
{
	int ret;
	u32 perm = 0;
	struct arm_smmu_master *master;
	bool ssid_valid = evt[0] & EVTQ_0_SSV;
	u8 type = FIELD_GET(EVTQ_0_ID, evt[0]);
	u32 sid = FIELD_GET(EVTQ_0_SID, evt[0]);
	struct iommu_fault_event fault_evt = { };
	struct iommu_fault *flt = &fault_evt.fault;

	/* Stage-2 is always pinned at the moment */
	if (evt[1] & EVTQ_1_S2)
		return -EFAULT;

	master = arm_smmu_find_master(smmu, sid);
	if (!master)
		return -EINVAL;

	if (evt[1] & EVTQ_1_READ)
		perm |= IOMMU_FAULT_PERM_READ;
	else
		perm |= IOMMU_FAULT_PERM_WRITE;

	if (evt[1] & EVTQ_1_EXEC)
		perm |= IOMMU_FAULT_PERM_EXEC;

	if (evt[1] & EVTQ_1_PRIV)
		perm |= IOMMU_FAULT_PERM_PRIV;

	if (evt[1] & EVTQ_1_STALL) {
		flt->type = IOMMU_FAULT_PAGE_REQ;
		flt->prm = (struct iommu_fault_page_request) {
			.flags = IOMMU_FAULT_PAGE_REQUEST_LAST_PAGE,
			.pasid = FIELD_GET(EVTQ_0_SSID, evt[0]),
			.grpid = FIELD_GET(EVTQ_1_STAG, evt[1]),
			.perm = perm,
			.addr = FIELD_GET(EVTQ_2_ADDR, evt[2]),
		};

		if (ssid_valid)
			flt->prm.flags |= IOMMU_FAULT_PAGE_REQUEST_PASID_VALID;
	} else {
		flt->type = IOMMU_FAULT_DMA_UNRECOV;
		flt->event = (struct iommu_fault_unrecoverable) {
			.flags = IOMMU_FAULT_UNRECOV_ADDR_VALID |
				 IOMMU_FAULT_UNRECOV_FETCH_ADDR_VALID,
			.pasid = FIELD_GET(EVTQ_0_SSID, evt[0]),
			.perm = perm,
			.addr = FIELD_GET(EVTQ_2_ADDR, evt[2]),
			.fetch_addr = FIELD_GET(EVTQ_3_IPA, evt[3]),
		};

		if (ssid_valid)
			flt->event.flags |= IOMMU_FAULT_UNRECOV_PASID_VALID;

		switch (type) {
		case EVT_ID_TRANSLATION_FAULT:
		case EVT_ID_ADDR_SIZE_FAULT:
		case EVT_ID_ACCESS_FAULT:
			flt->event.reason = IOMMU_FAULT_REASON_PTE_FETCH;
			break;
		case EVT_ID_PERMISSION_FAULT:
			flt->event.reason = IOMMU_FAULT_REASON_PERMISSION;
			break;
		default:
			/* TODO: report other unrecoverable faults. */
			return -EFAULT;
		}
	}

	ret = iommu_report_device_fault(master->dev, &fault_evt);
	if (ret && flt->type == IOMMU_FAULT_PAGE_REQ) {
		/* Nobody cared, abort the access */
		struct iommu_page_response resp = {
			.pasid		= flt->prm.pasid,
			.grpid		= flt->prm.grpid,
			.code		= IOMMU_PAGE_RESP_FAILURE,
		};
		arm_smmu_page_response(master->dev, NULL, &resp);
	}

	return ret;
}

static irqreturn_t arm_smmu_evtq_thread(int irq, void *dev)
{
	int i, ret;
	int num_handled = 0;
	struct arm_smmu_device *smmu = dev;
	struct arm_smmu_queue *q = &smmu->evtq.q;
	struct arm_smmu_ll_queue *llq = &q->llq;
	size_t queue_size = 1 << llq->max_n_shift;
	static DEFINE_RATELIMIT_STATE(rs, DEFAULT_RATELIMIT_INTERVAL,
				      DEFAULT_RATELIMIT_BURST);
	u64 evt[EVTQ_ENT_DWORDS];

	spin_lock(&q->wq.lock);
	do {
		while (!queue_remove_raw(q, evt)) {
			u8 id = FIELD_GET(EVTQ_0_ID, evt[0]);

			spin_unlock(&q->wq.lock);
			ret = arm_smmu_handle_evt(smmu, evt);
			spin_lock(&q->wq.lock);

			if (++num_handled == queue_size) {
				q->batch++;
				wake_up_all_locked(&q->wq);
				num_handled = 0;
			}

			if (ret && __ratelimit(&rs)) {
				dev_info(smmu->dev, "event 0x%02x received:\n", id);
				for (i = 0; i < ARRAY_SIZE(evt); ++i)
					dev_info(smmu->dev, "\t0x%016llx\n",
						 (unsigned long long)evt[i]);
			}
		}

		/*
		 * Not much we can do on overflow, so scream and pretend we're
		 * trying harder.
		 */
		if (queue_sync_prod_in(q) == -EOVERFLOW)
			dev_err(smmu->dev, "EVTQ overflow detected -- events lost\n");
	} while (!queue_empty(llq));

	/* Sync our overflow flag, as we believe we're up to speed */
	llq->cons = Q_OVF(llq->prod) | Q_WRP(llq, llq->cons) |
		    Q_IDX(llq, llq->cons);
	queue_sync_cons_out(q);

	wake_up_all_locked(&q->wq);
	spin_unlock(&q->wq.lock);

	return IRQ_HANDLED;
}

static void arm_smmu_handle_ppr(struct arm_smmu_device *smmu, u64 *evt)
{
	u32 sid = FIELD_PREP(PRIQ_0_SID, evt[0]);

	bool pasid_valid, last;
	struct arm_smmu_master *master;
	struct iommu_fault_event fault_evt = {
		.fault.type = IOMMU_FAULT_PAGE_REQ,
		.fault.prm = {
			.pasid		= FIELD_GET(PRIQ_0_SSID, evt[0]),
			.grpid		= FIELD_GET(PRIQ_1_PRG_IDX, evt[1]),
			.addr		= evt[1] & PRIQ_1_ADDR_MASK,
		},
	};
	struct iommu_fault_page_request *pr = &fault_evt.fault.prm;

	pasid_valid = evt[0] & PRIQ_0_SSID_V;
	last = evt[0] & PRIQ_0_PRG_LAST;

	/* Discard Stop PASID marker, it isn't used */
	if (!(evt[0] & (PRIQ_0_PERM_READ | PRIQ_0_PERM_WRITE)) && last)
		return;

	if (last)
		pr->flags |= IOMMU_FAULT_PAGE_REQUEST_LAST_PAGE;
	if (pasid_valid)
		pr->flags |= IOMMU_FAULT_PAGE_REQUEST_PASID_VALID;
	if (evt[0] & PRIQ_0_PERM_READ)
		pr->perm |= IOMMU_FAULT_PERM_READ;
	if (evt[0] & PRIQ_0_PERM_WRITE)
		pr->perm |= IOMMU_FAULT_PERM_WRITE;
	if (evt[0] & PRIQ_0_PERM_EXEC)
		pr->perm |= IOMMU_FAULT_PERM_EXEC;
	if (evt[0] & PRIQ_0_PERM_PRIV)
		pr->perm |= IOMMU_FAULT_PERM_PRIV;

	master = arm_smmu_find_master(smmu, sid);
	if (WARN_ON(!master))
		return;

	if (iommu_report_device_fault(master->dev, &fault_evt)) {
		/*
		 * No handler registered, so subsequent faults won't produce
		 * better results. Try to disable PRI.
		 */
		struct iommu_page_response resp = {
			.flags		= pasid_valid ?
				          IOMMU_PAGE_RESP_PASID_VALID : 0,
			.pasid		= pr->pasid,
			.grpid		= pr->grpid,
			.code		= IOMMU_PAGE_RESP_FAILURE,
		};

		dev_warn(master->dev,
			 "PPR 0x%x:0x%llx 0x%x: nobody cared, disabling PRI\n",
			 pasid_valid ? pr->pasid : 0, pr->addr, pr->perm);
		if (last)
			arm_smmu_page_response(master->dev, NULL, &resp);
	}
}

static irqreturn_t arm_smmu_priq_thread(int irq, void *dev)
{
	int num_handled = 0;
	bool overflow = false;
	struct arm_smmu_device *smmu = dev;
	struct arm_smmu_queue *q = &smmu->priq.q;
	struct arm_smmu_ll_queue *llq = &q->llq;
	size_t queue_size = 1 << llq->max_n_shift;
	u64 evt[PRIQ_ENT_DWORDS];

	spin_lock(&q->wq.lock);
	do {
		while (!queue_remove_raw(q, evt)) {
			spin_unlock(&q->wq.lock);
			arm_smmu_handle_ppr(smmu, evt);
			spin_lock(&q->wq.lock);
			if (++num_handled == queue_size) {
				q->batch++;
				wake_up_all_locked(&q->wq);
				num_handled = 0;
			}
		}

		if (queue_sync_prod_in(q) == -EOVERFLOW) {
			dev_err(smmu->dev, "PRIQ overflow detected -- requests lost\n");
			overflow = true;
		}
	} while (!queue_empty(llq));

	/* Sync our overflow flag, as we believe we're up to speed */
	llq->cons = Q_OVF(llq->prod) | Q_WRP(llq, llq->cons) |
		      Q_IDX(llq, llq->cons);
	queue_sync_cons_out(q);

	wake_up_all_locked(&q->wq);
	spin_unlock(&q->wq.lock);

	/*
	 * On overflow, the SMMU might have discarded the last PPR in a group.
	 * There is no way to know more about it, so we have to discard all
	 * partial faults already queued.
	 */
	if (overflow)
		iopf_queue_discard_partial(smmu->priq.iopf);

	return IRQ_HANDLED;
}

/*
 * arm_smmu_flush_evtq - wait until all events currently in the queue have been
 *                       consumed.
 *
 * Wait until the evtq thread finished a batch, or until the queue is empty.
 * Note that we don't handle overflows on q->batch. If it occurs, just wait for
 * the queue to be empty.
 */
static int arm_smmu_flush_evtq(void *cookie, struct device *dev, int pasid)
{
	int ret;
	u64 batch;
	struct arm_smmu_device *smmu = cookie;
	struct arm_smmu_queue *q = &smmu->evtq.q;

	spin_lock(&q->wq.lock);
	if (queue_sync_prod_in(q) == -EOVERFLOW)
		dev_err(smmu->dev, "evtq overflow detected -- requests lost\n");

	batch = q->batch;
	ret = wait_event_interruptible_locked(q->wq, queue_empty(&q->llq) ||
					      q->batch >= batch + 2);
	spin_unlock(&q->wq.lock);

	return ret;
}

static int arm_smmu_flush_priq(void *cookie, struct device *dev, int pasid)
{
	int ret;
	u64 batch;
	bool overflow = false;
	struct arm_smmu_device *smmu = cookie;
	struct arm_smmu_queue *q = &smmu->priq.q;

	spin_lock(&q->wq.lock);
	if (queue_sync_prod_in(q) == -EOVERFLOW) {
		dev_err(smmu->dev, "priq overflow detected -- requests lost\n");
		overflow = true;
	}

	batch = q->batch;
	ret = wait_event_interruptible_locked(q->wq, queue_empty(&q->llq) ||
					      q->batch >= batch + 2);
	spin_unlock(&q->wq.lock);

	if (overflow)
		iopf_queue_discard_partial(smmu->priq.iopf);
	return ret;
}

static int arm_smmu_device_disable(struct arm_smmu_device *smmu);

static irqreturn_t arm_smmu_gerror_handler(int irq, void *dev)
{
	u32 gerror, gerrorn, active;
	struct arm_smmu_device *smmu = dev;

	gerror = readl_relaxed(smmu->base + ARM_SMMU_GERROR);
	gerrorn = readl_relaxed(smmu->base + ARM_SMMU_GERRORN);

	active = gerror ^ gerrorn;
	if (!(active & GERROR_ERR_MASK))
		return IRQ_NONE; /* No errors pending */

	dev_warn(smmu->dev,
		 "unexpected global error reported (0x%08x), this could be serious\n",
		 active);

	if (active & GERROR_SFM_ERR) {
		dev_err(smmu->dev, "device has entered Service Failure Mode!\n");
		arm_smmu_device_disable(smmu);
	}

	if (active & GERROR_MSI_GERROR_ABT_ERR)
		dev_warn(smmu->dev, "GERROR MSI write aborted\n");

	if (active & GERROR_MSI_PRIQ_ABT_ERR)
		dev_warn(smmu->dev, "PRIQ MSI write aborted\n");

	if (active & GERROR_MSI_EVTQ_ABT_ERR)
		dev_warn(smmu->dev, "EVTQ MSI write aborted\n");

	if (active & GERROR_MSI_CMDQ_ABT_ERR)
		dev_warn(smmu->dev, "CMDQ MSI write aborted\n");

	if (active & GERROR_PRIQ_ABT_ERR)
		dev_err(smmu->dev, "PRIQ write aborted -- events may have been lost\n");

	if (active & GERROR_EVTQ_ABT_ERR)
		dev_err(smmu->dev, "EVTQ write aborted -- events may have been lost\n");

	if (active & GERROR_CMDQ_ERR)
		arm_smmu_cmdq_skip_err(smmu);

	writel(gerror, smmu->base + ARM_SMMU_GERRORN);
	return IRQ_HANDLED;
}

static irqreturn_t arm_smmu_combined_irq_thread(int irq, void *dev)
{
	struct arm_smmu_device *smmu = dev;

	arm_smmu_evtq_thread(irq, dev);
	if (smmu->features & ARM_SMMU_FEAT_PRI)
		arm_smmu_priq_thread(irq, dev);

	return IRQ_HANDLED;
}

static irqreturn_t arm_smmu_combined_irq_handler(int irq, void *dev)
{
	arm_smmu_gerror_handler(irq, dev);
	return IRQ_WAKE_THREAD;
}

static void
arm_smmu_atc_inv_to_cmd(int ssid, unsigned long iova, size_t size,
			struct arm_smmu_cmdq_ent *cmd)
{
	size_t log2_span;
	size_t span_mask;
	/* ATC invalidates are always on 4096-bytes pages */
	size_t inval_grain_shift = 12;
	unsigned long page_start, page_end;

	/*
	 * ATS and PASID:
	 *
	 * If substream_valid is clear, the PCIe TLP is sent without a PASID
	 * prefix. In that case all ATC entries within the address range are
	 * invalidated, including those that were requested with a PASID! There
	 * is no way to invalidate only entries without PASID.
	 *
	 * When using STRTAB_STE_1_S1DSS_SSID0 (reserving CD 0 for non-PASID
	 * traffic), translation requests without PASID create ATC entries
	 * without PASID, which must be invalidated with substream_valid clear.
	 * This has the unpleasant side-effect of invalidating all PASID-tagged
	 * ATC entries within the address range.
	 */
	*cmd = (struct arm_smmu_cmdq_ent) {
		.opcode			= CMDQ_OP_ATC_INV,
		.substream_valid	= !!ssid,
		.atc.ssid		= ssid,
	};

	if (!size) {
		cmd->atc.size = ATC_INV_SIZE_ALL;
		return;
	}

	page_start	= iova >> inval_grain_shift;
	page_end	= (iova + size - 1) >> inval_grain_shift;

	/*
	 * In an ATS Invalidate Request, the address must be aligned on the
	 * range size, which must be a power of two number of page sizes. We
	 * thus have to choose between grossly over-invalidating the region, or
	 * splitting the invalidation into multiple commands. For simplicity
	 * we'll go with the first solution, but should refine it in the future
	 * if multiple commands are shown to be more efficient.
	 *
	 * Find the smallest power of two that covers the range. The most
	 * significant differing bit between the start and end addresses,
	 * fls(start ^ end), indicates the required span. For example:
	 *
	 * We want to invalidate pages [8; 11]. This is already the ideal range:
	 *		x = 0b1000 ^ 0b1011 = 0b11
	 *		span = 1 << fls(x) = 4
	 *
	 * To invalidate pages [7; 10], we need to invalidate [0; 15]:
	 *		x = 0b0111 ^ 0b1010 = 0b1101
	 *		span = 1 << fls(x) = 16
	 */
	log2_span	= fls_long(page_start ^ page_end);
	span_mask	= (1ULL << log2_span) - 1;

	page_start	&= ~span_mask;

	cmd->atc.addr	= page_start << inval_grain_shift;
	cmd->atc.size	= log2_span;
}

static int arm_smmu_atc_inv_master(struct arm_smmu_master *master, int ssid)
{
	int i;
	struct arm_smmu_cmdq_ent cmd;

	arm_smmu_atc_inv_to_cmd(ssid, 0, 0, &cmd);

	for (i = 0; i < master->num_streams; i++) {
		cmd.atc.sid = master->streams[i].id;
		arm_smmu_cmdq_issue_cmd(master->smmu, &cmd);
	}

	return arm_smmu_cmdq_issue_sync(master->smmu);
}

static int arm_smmu_atc_inv_domain(struct arm_smmu_domain *smmu_domain,
				   int ssid, unsigned long iova, size_t size)
{
	int i;
	unsigned long flags;
	struct arm_smmu_cmdq_ent cmd;
	struct arm_smmu_master *master;
	struct arm_smmu_cmdq_batch cmds = {};

	if (!(smmu_domain->smmu->features & ARM_SMMU_FEAT_ATS))
		return 0;

	/*
	 * Ensure that we've completed prior invalidation of the main TLBs
	 * before we read 'nr_ats_masters' in case of a concurrent call to
	 * arm_smmu_enable_ats():
	 *
	 *	// unmap()			// arm_smmu_enable_ats()
	 *	TLBI+SYNC			atomic_inc(&nr_ats_masters);
	 *	smp_mb();			[...]
	 *	atomic_read(&nr_ats_masters);	pci_enable_ats() // writel()
	 *
	 * Ensures that we always see the incremented 'nr_ats_masters' count if
	 * ATS was enabled at the PCI device before completion of the TLBI.
	 */
	smp_mb();
	if (!atomic_read(&smmu_domain->nr_ats_masters))
		return 0;

	arm_smmu_atc_inv_to_cmd(ssid, iova, size, &cmd);

	spin_lock_irqsave(&smmu_domain->devices_lock, flags);
	list_for_each_entry(master, &smmu_domain->devices, domain_head) {
		if (!master->ats_enabled)
			continue;

		for (i = 0; i < master->num_streams; i++) {
			cmd.atc.sid = master->streams[i].id;
			arm_smmu_cmdq_batch_add(smmu_domain->smmu, &cmds, &cmd);
		}
	}
	spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);

	return arm_smmu_cmdq_batch_submit(smmu_domain->smmu, &cmds);
}

/* IO_PGTABLE API */
static void arm_smmu_tlb_inv_context(void *cookie)
{
	struct arm_smmu_domain *smmu_domain = cookie;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_cmdq_ent cmd;

	/*
	 * NOTE: when io-pgtable is in non-strict mode, we may get here with
	 * PTEs previously cleared by unmaps on the current CPU not yet visible
	 * to the SMMU. We are relying on the dma_wmb() implicit during cmd
	 * insertion to guarantee those are observed before the TLBI. Do be
	 * careful, 007.
	 */
	if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		arm_smmu_tlb_inv_asid(smmu, smmu_domain->s1_cfg.cd.asid);
	} else {
		cmd.opcode	= CMDQ_OP_TLBI_S12_VMALL;
		cmd.tlbi.vmid	= smmu_domain->s2_cfg.vmid;
		arm_smmu_cmdq_issue_cmd(smmu, &cmd);
		arm_smmu_cmdq_issue_sync(smmu);
	}
	if (smmu_domain->parent)
		arm_smmu_atc_inv_domain(smmu_domain->parent, smmu_domain->ssid,
					0, 0);
	else
		arm_smmu_atc_inv_domain(smmu_domain, 0, 0, 0);

}

static void arm_smmu_tlb_inv_range(unsigned long iova, size_t size,
				   size_t granule, bool leaf,
				   struct arm_smmu_domain *smmu_domain)
{
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	unsigned long start = iova, end = iova + size;
	struct arm_smmu_cmdq_batch cmds = {};
	struct arm_smmu_cmdq_ent cmd = {
		.tlbi = {
			.leaf	= leaf,
		},
	};

	if (!size)
		return;

	if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		cmd.opcode	= smmu->features & ARM_SMMU_FEAT_E2H ?
				  CMDQ_OP_TLBI_EL2_VA : CMDQ_OP_TLBI_NH_VA;
		cmd.tlbi.asid	= smmu_domain->s1_cfg.cd.asid;
	} else {
		cmd.opcode	= CMDQ_OP_TLBI_S2_IPA;
		cmd.tlbi.vmid	= smmu_domain->s2_cfg.vmid;
	}

	while (iova < end) {
		cmd.tlbi.addr = iova;
		arm_smmu_cmdq_batch_add(smmu, &cmds, &cmd);
		iova += granule;
	}
	arm_smmu_cmdq_batch_submit(smmu, &cmds);

	/*
	 * Unfortunately, this can't be leaf-only since we may have
	 * zapped an entire table.
	 */
	if (smmu_domain->parent)
		arm_smmu_atc_inv_domain(smmu_domain->parent, smmu_domain->ssid,
					start, size);
	else
		arm_smmu_atc_inv_domain(smmu_domain, 0, start, size);
}

static void arm_smmu_tlb_inv_page_nosync(struct iommu_iotlb_gather *gather,
					 unsigned long iova, size_t granule,
					 void *cookie)
{
	struct arm_smmu_domain *smmu_domain = cookie;
	struct iommu_domain *domain = &smmu_domain->domain;

	iommu_iotlb_gather_add_page(domain, gather, iova, granule);
}

static void arm_smmu_tlb_inv_walk(unsigned long iova, size_t size,
				  size_t granule, void *cookie)
{
	arm_smmu_tlb_inv_range(iova, size, granule, false, cookie);
}

static void arm_smmu_tlb_inv_leaf(unsigned long iova, size_t size,
				  size_t granule, void *cookie)
{
	arm_smmu_tlb_inv_range(iova, size, granule, true, cookie);
}

static const struct iommu_flush_ops arm_smmu_flush_ops = {
	.tlb_flush_all	= arm_smmu_tlb_inv_context,
	.tlb_flush_walk = arm_smmu_tlb_inv_walk,
	.tlb_flush_leaf = arm_smmu_tlb_inv_leaf,
	.tlb_add_page	= arm_smmu_tlb_inv_page_nosync,
};

/* IOMMU API */
static bool arm_smmu_capable(enum iommu_cap cap)
{
	switch (cap) {
	case IOMMU_CAP_CACHE_COHERENCY:
		return true;
	case IOMMU_CAP_NOEXEC:
		return true;
	default:
		return false;
	}
}

static struct iommu_domain *arm_smmu_domain_alloc(unsigned type)
{
	struct arm_smmu_domain *smmu_domain;

	if (type != IOMMU_DOMAIN_UNMANAGED &&
	    type != IOMMU_DOMAIN_DMA &&
	    type != IOMMU_DOMAIN_IDENTITY)
		return NULL;

	/*
	 * Allocate the domain and initialise some of its data structures.
	 * We can't really do anything meaningful until we've added a
	 * master.
	 */
	smmu_domain = kzalloc(sizeof(*smmu_domain), GFP_KERNEL);
	if (!smmu_domain)
		return NULL;

	if (type == IOMMU_DOMAIN_DMA &&
	    iommu_get_dma_cookie(&smmu_domain->domain)) {
		kfree(smmu_domain);
		return NULL;
	}

	mutex_init(&smmu_domain->init_mutex);
	INIT_LIST_HEAD(&smmu_domain->devices);
	spin_lock_init(&smmu_domain->devices_lock);

	return &smmu_domain->domain;
}

static int arm_smmu_bitmap_alloc(unsigned long *map, int span)
{
	int idx, size = 1 << span;

	do {
		idx = find_first_zero_bit(map, size);
		if (idx == size)
			return -ENOSPC;
	} while (test_and_set_bit(idx, map));

	return idx;
}

static void arm_smmu_bitmap_free(unsigned long *map, int idx)
{
	clear_bit(idx, map);
}

static void arm_smmu_domain_free(struct iommu_domain *domain)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct arm_smmu_device *smmu = smmu_domain->smmu;

	iommu_put_dma_cookie(domain);
	free_io_pgtable_ops(smmu_domain->pgtbl_ops);

	/* Free the CD and ASID, if we allocated them */
	if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;

		if (cfg->cdcfg.cdtab)
			arm_smmu_free_cd_tables(smmu_domain);
		arm_smmu_free_asid(&cfg->cd);
		if (smmu_domain->ssid)
			ioasid_free(smmu_domain->ssid);
	} else {
		struct arm_smmu_s2_cfg *cfg = &smmu_domain->s2_cfg;
		if (cfg->vmid)
			arm_smmu_bitmap_free(smmu->vmid_map, cfg->vmid);
	}

	kfree(smmu_domain);
}

static int arm_smmu_domain_finalise_cd(struct arm_smmu_domain *smmu_domain,
				       struct arm_smmu_master *master,
				       struct io_pgtable_cfg *pgtbl_cfg)
{
	int ret;
	u32 asid;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;
	typeof(&pgtbl_cfg->arm_lpae_s1_cfg.tcr) tcr = &pgtbl_cfg->arm_lpae_s1_cfg.tcr;

	arm_smmu_init_cd(&cfg->cd);

	ret = xa_alloc(&asid_xa, &asid, &cfg->cd,
		       XA_LIMIT(1, (1 << smmu->asid_bits) - 1), GFP_KERNEL);
	if (ret)
		return ret;

	cfg->cd.asid	= (u16)asid;
	cfg->cd.ttbr	= pgtbl_cfg->arm_lpae_s1_cfg.ttbr;
	cfg->cd.tcr	= FIELD_PREP(CTXDESC_CD_0_TCR_T0SZ, tcr->tsz) |
			  FIELD_PREP(CTXDESC_CD_0_TCR_TG0, tcr->tg) |
			  FIELD_PREP(CTXDESC_CD_0_TCR_IRGN0, tcr->irgn) |
			  FIELD_PREP(CTXDESC_CD_0_TCR_ORGN0, tcr->orgn) |
			  FIELD_PREP(CTXDESC_CD_0_TCR_SH0, tcr->sh) |
			  FIELD_PREP(CTXDESC_CD_0_TCR_IPS, tcr->ips) |
			  CTXDESC_CD_0_TCR_EPD1 | CTXDESC_CD_0_AA64;
	cfg->cd.mair	= pgtbl_cfg->arm_lpae_s1_cfg.mair;
	return 0;
}

static int arm_smmu_domain_finalise_s1(struct arm_smmu_domain *smmu_domain,
				       struct arm_smmu_master *master,
				       struct io_pgtable_cfg *pgtbl_cfg)
{
	int ret;
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;

	ret = arm_smmu_domain_finalise_cd(smmu_domain, master, pgtbl_cfg);
	if (ret)
		return ret;

	cfg->s1cdmax = master->ssid_bits;

	if (master->stall_enabled)
		smmu_domain->stall_enabled = true;

	ret = arm_smmu_alloc_cd_tables(smmu_domain);
	if (ret)
		goto out_free_asid;

	/*
	 * Note that this will end up calling arm_smmu_sync_cd() before
	 * the master has been added to the devices list for this domain.
	 * This isn't an issue because the STE hasn't been installed yet.
	 */
	ret = arm_smmu_write_ctx_desc(smmu_domain, 0, &cfg->cd);
	if (ret)
		goto out_free_cd_tables;

	return 0;

out_free_cd_tables:
	arm_smmu_free_cd_tables(smmu_domain);
out_free_asid:
	arm_smmu_free_asid(&cfg->cd);
	return ret;
}

static int arm_smmu_domain_finalise_s2(struct arm_smmu_domain *smmu_domain,
				       struct arm_smmu_master *master,
				       struct io_pgtable_cfg *pgtbl_cfg)
{
	int vmid;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_s2_cfg *cfg = &smmu_domain->s2_cfg;
	typeof(&pgtbl_cfg->arm_lpae_s2_cfg.vtcr) vtcr;

	vmid = arm_smmu_bitmap_alloc(smmu->vmid_map, smmu->vmid_bits);
	if (vmid < 0)
		return vmid;

	vtcr = &pgtbl_cfg->arm_lpae_s2_cfg.vtcr;
	cfg->vmid	= (u16)vmid;
	cfg->vttbr	= pgtbl_cfg->arm_lpae_s2_cfg.vttbr;
	cfg->vtcr	= FIELD_PREP(STRTAB_STE_2_VTCR_S2T0SZ, vtcr->tsz) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2SL0, vtcr->sl) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2IR0, vtcr->irgn) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2OR0, vtcr->orgn) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2SH0, vtcr->sh) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2TG, vtcr->tg) |
			  FIELD_PREP(STRTAB_STE_2_VTCR_S2PS, vtcr->ps);
	return 0;
}

static int arm_smmu_domain_finalise(struct iommu_domain *domain,
				    struct arm_smmu_master *master)
{
	int ret;
	unsigned long ias, oas;
	enum io_pgtable_fmt fmt;
	struct io_pgtable_cfg pgtbl_cfg;
	struct io_pgtable_ops *pgtbl_ops;
	int (*finalise_stage_fn)(struct arm_smmu_domain *,
				 struct arm_smmu_master *,
				 struct io_pgtable_cfg *);
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct arm_smmu_device *smmu = smmu_domain->smmu;

	if (domain->type == IOMMU_DOMAIN_IDENTITY) {
		smmu_domain->stage = ARM_SMMU_DOMAIN_BYPASS;
		return 0;
	}

	/* Restrict the stage to what we can actually support */
	if (!(smmu->features & ARM_SMMU_FEAT_TRANS_S1))
		smmu_domain->stage = ARM_SMMU_DOMAIN_S2;
	if (!(smmu->features & ARM_SMMU_FEAT_TRANS_S2))
		smmu_domain->stage = ARM_SMMU_DOMAIN_S1;

	switch (smmu_domain->stage) {
	case ARM_SMMU_DOMAIN_S1:
		ias = (smmu->features & ARM_SMMU_FEAT_VAX) ? 52 : 48;
		ias = min_t(unsigned long, ias, VA_BITS);
		oas = smmu->ias;
		fmt = ARM_64_LPAE_S1;
		if (smmu_domain->parent)
			finalise_stage_fn = arm_smmu_domain_finalise_cd;
		else
			finalise_stage_fn = arm_smmu_domain_finalise_s1;
		break;
	case ARM_SMMU_DOMAIN_NESTED:
	case ARM_SMMU_DOMAIN_S2:
		ias = smmu->ias;
		oas = smmu->oas;
		fmt = ARM_64_LPAE_S2;
		finalise_stage_fn = arm_smmu_domain_finalise_s2;
		break;
	default:
		return -EINVAL;
	}

	pgtbl_cfg = (struct io_pgtable_cfg) {
		.pgsize_bitmap	= smmu->pgsize_bitmap,
		.ias		= ias,
		.oas		= oas,
		.coherent_walk	= smmu->features & ARM_SMMU_FEAT_COHERENCY,
		.tlb		= &arm_smmu_flush_ops,
		.iommu_dev	= smmu->dev,
	};

	if (smmu_domain->non_strict)
		pgtbl_cfg.quirks |= IO_PGTABLE_QUIRK_NON_STRICT;

	pgtbl_ops = alloc_io_pgtable_ops(fmt, &pgtbl_cfg, smmu_domain);
	if (!pgtbl_ops)
		return -ENOMEM;

	domain->pgsize_bitmap = pgtbl_cfg.pgsize_bitmap;
	domain->geometry.aperture_end = (1UL << pgtbl_cfg.ias) - 1;
	domain->geometry.force_aperture = true;

	ret = finalise_stage_fn(smmu_domain, master, &pgtbl_cfg);
	if (ret < 0) {
		free_io_pgtable_ops(pgtbl_ops);
		return ret;
	}

	smmu_domain->pgtbl_ops = pgtbl_ops;
	return 0;
}

static __le64 *arm_smmu_get_step_for_sid(struct arm_smmu_device *smmu, u32 sid)
{
	__le64 *step;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB) {
		struct arm_smmu_strtab_l1_desc *l1_desc;
		int idx;

		/* Two-level walk */
		idx = (sid >> STRTAB_SPLIT) * STRTAB_L1_DESC_DWORDS;
		l1_desc = &cfg->l1_desc[idx];
		idx = (sid & ((1 << STRTAB_SPLIT) - 1)) * STRTAB_STE_DWORDS;
		step = &l1_desc->l2ptr[idx];
	} else {
		/* Simple linear lookup */
		step = &cfg->strtab[sid * STRTAB_STE_DWORDS];
	}

	return step;
}

static void arm_smmu_install_ste_for_dev(struct arm_smmu_master *master)
{
	int i, j;
	struct arm_smmu_device *smmu = master->smmu;

	for (i = 0; i < master->num_streams; ++i) {
		u32 sid = master->streams[i].id;
		__le64 *step = arm_smmu_get_step_for_sid(smmu, sid);

		/* Bridged PCI devices may end up with duplicated IDs */
		for (j = 0; j < i; j++)
			if (master->streams[j].id == sid)
				break;
		if (j < i)
			continue;

		arm_smmu_write_strtab_ent(master, sid, step);
	}
}

static bool arm_smmu_ats_supported(struct arm_smmu_master *master)
{
	struct device *dev = master->dev;
	struct arm_smmu_device *smmu = master->smmu;

	return (smmu->features & ARM_SMMU_FEAT_ATS) && dev_is_pci(dev) &&
		pci_ats_supported(to_pci_dev(dev));
}

static void arm_smmu_enable_ats(struct arm_smmu_master *master)
{
	size_t stu;
	struct pci_dev *pdev;
	struct arm_smmu_device *smmu = master->smmu;
	struct arm_smmu_domain *smmu_domain = master->domain;

	/* Don't enable ATS at the endpoint if it's not enabled in the STE */
	if (!master->ats_enabled)
		return;

	/* Smallest Translation Unit: log2 of the smallest supported granule */
	stu = __ffs(smmu->pgsize_bitmap);
	pdev = to_pci_dev(master->dev);

	atomic_inc(&smmu_domain->nr_ats_masters);
	arm_smmu_atc_inv_domain(smmu_domain, 0, 0, 0);
	if (pci_enable_ats(pdev, stu))
		dev_err(master->dev, "Failed to enable ATS (STU %zu)\n", stu);
}

static void arm_smmu_disable_ats(struct arm_smmu_master *master)
{
	struct arm_smmu_domain *smmu_domain = master->domain;

	if (!master->ats_enabled)
		return;

	pci_disable_ats(to_pci_dev(master->dev));
	/*
	 * Ensure ATS is disabled at the endpoint before we issue the
	 * ATC invalidation via the SMMU.
	 */
	wmb();
	arm_smmu_atc_inv_master(master, 0);
	atomic_dec(&smmu_domain->nr_ats_masters);
}

static int arm_smmu_enable_pasid(struct arm_smmu_master *master)
{
	int ret;
	int features;
	int num_pasids;
	struct pci_dev *pdev;

	if (!dev_is_pci(master->dev))
		return -ENODEV;

	pdev = to_pci_dev(master->dev);

	features = pci_pasid_features(pdev);
	if (features < 0)
		return features;

	num_pasids = pci_max_pasids(pdev);
	if (num_pasids <= 0)
		return num_pasids;

	ret = pci_enable_pasid(pdev, features);
	if (ret) {
		dev_err(&pdev->dev, "Failed to enable PASID\n");
		return ret;
	}

	master->ssid_bits = min_t(u8, ilog2(num_pasids),
				  master->smmu->ssid_bits);
	return 0;
}

static void arm_smmu_disable_pasid(struct arm_smmu_master *master)
{
	struct pci_dev *pdev;

	if (!dev_is_pci(master->dev))
		return;

	pdev = to_pci_dev(master->dev);

	if (!pdev->pasid_enabled)
		return;

	master->ssid_bits = 0;
	pci_disable_pasid(pdev);
}

static int arm_smmu_init_pri(struct arm_smmu_master *master)
{
	int pos;
	struct pci_dev *pdev;

	if (!dev_is_pci(master->dev))
		return -EINVAL;

	if (!(master->smmu->features & ARM_SMMU_FEAT_PRI))
		return 0;

	pdev = to_pci_dev(master->dev);
	pos = pci_find_ext_capability(pdev, PCI_EXT_CAP_ID_PRI);
	if (!pos)
		return 0;

	/* If the device supports PASID and PRI, set STE.PPAR */
	if (master->ssid_bits)
		master->prg_resp_needs_ssid = pci_prg_resp_pasid_required(pdev);

	master->pri_supported = true;
	return 0;
}

static int arm_smmu_enable_pri(struct arm_smmu_master *master)
{
	int ret;
	struct pci_dev *pdev;
	/*
	 * TODO: find a good inflight PPR number. We should divide the PRI queue
	 * by the number of PRI-capable devices, but it's impossible to know
	 * about future (probed late or hotplugged) devices. So we're at risk of
	 * dropping PPRs (and leaking pending requests in the FQ).
	 */
	size_t max_inflight_pprs = 16;

	if (!master->pri_supported || !master->ats_enabled)
		return -ENOSYS;

	pdev = to_pci_dev(master->dev);

	ret = pci_reset_pri(pdev);
	if (ret)
		return ret;

	ret = pci_enable_pri(pdev, max_inflight_pprs);
	if (ret) {
		dev_err(master->dev, "cannot enable PRI: %d\n", ret);
		return ret;
	}

	return 0;
}

static void arm_smmu_disable_pri(struct arm_smmu_master *master)
{
	struct pci_dev *pdev;

	if (!dev_is_pci(master->dev))
		return;

	pdev = to_pci_dev(master->dev);

	if (!pdev->pri_enabled)
		return;

	pci_disable_pri(pdev);
}

static void arm_smmu_detach_dev(struct arm_smmu_master *master)
{
	unsigned long flags;
	struct arm_smmu_domain *smmu_domain = master->domain;

	if (!smmu_domain)
		return;

	arm_smmu_disable_ats(master);

	spin_lock_irqsave(&smmu_domain->devices_lock, flags);
	list_del(&master->domain_head);
	spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);

	master->domain = NULL;
	master->ats_enabled = false;
	arm_smmu_install_ste_for_dev(master);
}

static int arm_smmu_attach_dev(struct iommu_domain *domain, struct device *dev)
{
	int ret = 0;
	unsigned long flags;
	struct arm_smmu_device *smmu;
	struct arm_smmu_master *master = dev_to_master(dev);
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (!master)
		return -ENOENT;

	smmu = master->smmu;

	if (iommu_sva_enabled(dev)) {
		/* Did the previous driver forget to release SVA handles? */
		dev_err(dev, "cannot attach - SVA enabled\n");
		return -EBUSY;
	}

	arm_smmu_detach_dev(master);

	mutex_lock(&smmu_domain->init_mutex);

	if (!smmu_domain->smmu) {
		smmu_domain->smmu = smmu;
		ret = arm_smmu_domain_finalise(domain, master);
		if (ret) {
			smmu_domain->smmu = NULL;
			goto out_unlock;
		}
	} else if (smmu_domain->smmu != smmu) {
		dev_err(dev,
			"cannot attach to SMMU %s (upstream of %s)\n",
			dev_name(smmu_domain->smmu->dev),
			dev_name(smmu->dev));
		ret = -ENXIO;
		goto out_unlock;
	} else if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1 &&
		   master->ssid_bits != smmu_domain->s1_cfg.s1cdmax) {
		dev_err(dev,
			"cannot attach to incompatible domain (%u SSID bits != %u)\n",
			smmu_domain->s1_cfg.s1cdmax, master->ssid_bits);
		ret = -EINVAL;
		goto out_unlock;
	} else if (smmu_domain->stall_enabled && !master->stall_enabled) {
		dev_err(dev, "cannot attach to stall-enabled domain\n");
		ret = -EINVAL;
		goto out_unlock;
	} else if (smmu_domain->parent) {
		dev_err(dev, "cannot attach auxiliary domain\n");
		ret = -EINVAL;
		goto out_unlock;
	}

	master->domain = smmu_domain;

	if (smmu_domain->stage != ARM_SMMU_DOMAIN_BYPASS)
		master->ats_enabled = arm_smmu_ats_supported(master);

	arm_smmu_install_ste_for_dev(master);

	spin_lock_irqsave(&smmu_domain->devices_lock, flags);
	list_add(&master->domain_head, &smmu_domain->devices);
	spin_unlock_irqrestore(&smmu_domain->devices_lock, flags);

	arm_smmu_enable_ats(master);

out_unlock:
	mutex_unlock(&smmu_domain->init_mutex);
	return ret;
}

static int arm_smmu_map(struct iommu_domain *domain, unsigned long iova,
			phys_addr_t paddr, size_t size, int prot, gfp_t gfp)
{
	struct io_pgtable_ops *ops = to_smmu_domain(domain)->pgtbl_ops;

	if (!ops)
		return -ENODEV;

	return ops->map(ops, iova, paddr, size, prot);
}

static size_t arm_smmu_unmap(struct iommu_domain *domain, unsigned long iova,
			     size_t size, struct iommu_iotlb_gather *gather)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct io_pgtable_ops *ops = smmu_domain->pgtbl_ops;

	if (!ops)
		return 0;

	return ops->unmap(ops, iova, size, gather);
}

static void arm_smmu_flush_iotlb_all(struct iommu_domain *domain)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (smmu_domain->smmu)
		arm_smmu_tlb_inv_context(smmu_domain);
}

static void arm_smmu_iotlb_sync(struct iommu_domain *domain,
				struct iommu_iotlb_gather *gather)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	arm_smmu_tlb_inv_range(gather->start, gather->end - gather->start,
			       gather->pgsize, true, smmu_domain);
}

static phys_addr_t
arm_smmu_iova_to_phys(struct iommu_domain *domain, dma_addr_t iova)
{
	struct io_pgtable_ops *ops = to_smmu_domain(domain)->pgtbl_ops;

	if (domain->type == IOMMU_DOMAIN_IDENTITY)
		return iova;

	if (!ops)
		return 0;

	return ops->iova_to_phys(ops, iova);
}

static void arm_smmu_mm_invalidate(struct device *dev, int pasid, void *entry,
				   unsigned long iova, size_t size)
{
	int i;
	struct arm_smmu_cmdq_ent cmd;
	struct arm_smmu_cmdq_batch cmds = {};
	struct arm_smmu_master *master = dev_to_master(dev);

	if (!master->ats_enabled)
		return;

	arm_smmu_atc_inv_to_cmd(pasid, iova, size, &cmd);

	for (i = 0; i < master->num_streams; i++) {
		cmd.atc.sid = master->streams[i].id;
		arm_smmu_cmdq_batch_add(master->smmu, &cmds, &cmd);
	}

	arm_smmu_cmdq_batch_submit(master->smmu, &cmds);
}

static int arm_smmu_mm_attach(struct device *dev, int pasid, void *entry,
			      bool attach_domain)
{
	struct arm_smmu_ctx_desc *cd = entry;
	struct iommu_domain *domain = iommu_get_domain_for_dev(dev);
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	/*
	 * If another device in the domain has already been attached, the
	 * context descriptor is already valid.
	 */
	if (!attach_domain)
		return 0;

	return arm_smmu_write_ctx_desc(smmu_domain, pasid, cd);
}

static void arm_smmu_mm_detach(struct device *dev, int pasid, void *entry,
			       bool detach_domain)
{
	struct arm_smmu_ctx_desc *cd = entry;
	struct arm_smmu_master *master = dev_to_master(dev);
	struct iommu_domain *domain = iommu_get_domain_for_dev(dev);
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (detach_domain) {
		arm_smmu_write_ctx_desc(smmu_domain, pasid, NULL);

		/*
		 * The ASID allocator won't broadcast the final TLB
		 * invalidations for this ASID, so we need to do it manually.
		 * For private contexts, freeing io-pgtable ops performs the
		 * invalidation.
		 */
		arm_smmu_tlb_inv_asid(smmu_domain->smmu, cd->asid);
		arm_smmu_atc_inv_domain(smmu_domain, pasid, 0, 0);

	} else if (master->ats_enabled) {
		arm_smmu_atc_inv_master(master, pasid);
	}
}

static void *arm_smmu_mm_alloc(struct mm_struct *mm)
{
	return arm_smmu_alloc_shared_cd(mm);
}

static void arm_smmu_mm_free(void *entry)
{
	arm_smmu_free_shared_cd(entry);
}

static struct io_mm_ops arm_smmu_mm_ops = {
	.alloc		= arm_smmu_mm_alloc,
	.invalidate	= arm_smmu_mm_invalidate,
	.attach		= arm_smmu_mm_attach,
	.detach		= arm_smmu_mm_detach,
	.release	= arm_smmu_mm_free,
};

static struct iommu_sva *
arm_smmu_sva_bind(struct device *dev, struct mm_struct *mm, void *drvdata)
{
	struct iommu_domain *domain = iommu_get_domain_for_dev(dev);
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (smmu_domain->stage != ARM_SMMU_DOMAIN_S1)
		return ERR_PTR(-EINVAL);

	return iommu_sva_bind_generic(dev, mm, &arm_smmu_mm_ops, drvdata);
}

static struct platform_driver arm_smmu_driver;

static
struct arm_smmu_device *arm_smmu_get_by_fwnode(struct fwnode_handle *fwnode)
{
	struct device *dev = driver_find_device_by_fwnode(&arm_smmu_driver.driver,
							  fwnode);
	put_device(dev);
	return dev ? dev_get_drvdata(dev) : NULL;
}

static bool arm_smmu_sid_in_range(struct arm_smmu_device *smmu, u32 sid)
{
	unsigned long limit = smmu->strtab_cfg.num_l1_ents;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB)
		limit *= 1UL << STRTAB_SPLIT;

	return sid < limit;
}

static int arm_smmu_insert_master(struct arm_smmu_device *smmu,
				  struct arm_smmu_master *master)
{
	int i;
	int ret = 0;
	struct arm_smmu_stream *new_stream, *cur_stream;
	struct rb_node **new_node, *parent_node = NULL;
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(master->dev);

	master->streams = kcalloc(fwspec->num_ids,
				  sizeof(struct arm_smmu_stream), GFP_KERNEL);
	if (!master->streams)
		return -ENOMEM;
	master->num_streams = fwspec->num_ids;

	mutex_lock(&smmu->streams_mutex);
	for (i = 0; i < fwspec->num_ids && !ret; i++) {
		u32 sid = fwspec->ids[i];

		new_stream = &master->streams[i];
		new_stream->id = sid;
		new_stream->master = master;

		/* Check the SIDs are in range of the SMMU and our stream table */
		if (!arm_smmu_sid_in_range(smmu, sid)) {
			ret = -ERANGE;
			break;
		}

		/* Ensure l2 strtab is initialised */
		if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB) {
			ret = arm_smmu_init_l2_strtab(smmu, sid);
			if (ret)
				break;
		}

		/* Insert into SID tree */
		new_node = &(smmu->streams.rb_node);
		while (*new_node) {
			cur_stream = rb_entry(*new_node, struct arm_smmu_stream,
					      node);
			parent_node = *new_node;
			if (cur_stream->id > new_stream->id) {
				new_node = &((*new_node)->rb_left);
			} else if (cur_stream->id < new_stream->id) {
				new_node = &((*new_node)->rb_right);
			} else {
				dev_warn(master->dev,
					 "stream %u already in tree\n",
					 cur_stream->id);
				ret = -EINVAL;
				break;
			}
		}

		if (!ret) {
			rb_link_node(&new_stream->node, parent_node, new_node);
			rb_insert_color(&new_stream->node, &smmu->streams);
		}
	}

	if (ret) {
		for (; i > 0; i--)
			rb_erase(&master->streams[i].node, &smmu->streams);
		kfree(master->streams);
	}
	mutex_unlock(&smmu->streams_mutex);

	return ret;
}

static void arm_smmu_remove_master(struct arm_smmu_device *smmu,
				   struct arm_smmu_master *master)
{
	int i;
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(master->dev);

	if (!master->streams)
		return;

	mutex_lock(&smmu->streams_mutex);
	for (i = 0; i < fwspec->num_ids; i++)
		rb_erase(&master->streams[i].node, &smmu->streams);
	mutex_unlock(&smmu->streams_mutex);

	kfree(master->streams);
}

static struct iommu_ops arm_smmu_ops;

static int arm_smmu_add_device(struct device *dev)
{
	int ret;
	struct arm_smmu_device *smmu;
	struct arm_smmu_master *master;
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);
	struct iommu_group *group;

	if (!fwspec || fwspec->ops != &arm_smmu_ops)
		return -ENODEV;

	if (WARN_ON_ONCE(fwspec->iommu_priv))
		return -EBUSY;

	smmu = arm_smmu_get_by_fwnode(fwspec->iommu_fwnode);
	if (!smmu)
		return -ENODEV;

	master = kzalloc(sizeof(*master), GFP_KERNEL);
	if (!master)
		return -ENOMEM;

	master->dev = dev;
	master->smmu = smmu;
	fwspec->iommu_priv = master;

	ret = arm_smmu_insert_master(smmu, master);
	if (ret)
		goto err_free_master;

	master->ssid_bits = min(smmu->ssid_bits, fwspec->num_pasid_bits);

	/*
	 * Note that PASID must be enabled before, and disabled after ATS:
	 * PCI Express Base 4.0r1.0 - 10.5.1.3 ATS Control Register
	 *
	 *   Behavior is undefined if this bit is Set and the value of the PASID
	 *   Enable, Execute Requested Enable, or Privileged Mode Requested bits
	 *   are changed.
	 */
	arm_smmu_enable_pasid(master);

	if (!(smmu->features & ARM_SMMU_FEAT_2_LVL_CDTAB))
		master->ssid_bits = min_t(u8, master->ssid_bits,
					  CTXDESC_LINEAR_CDMAX);

	if ((smmu->features & ARM_SMMU_FEAT_STALLS && fwspec->can_stall) ||
	    smmu->features & ARM_SMMU_FEAT_STALL_FORCE)
		master->stall_enabled = true;

	arm_smmu_init_pri(master);

	ret = iommu_device_link(&smmu->iommu, dev);
	if (ret)
		goto err_disable_pasid;

	group = iommu_group_get_for_dev(dev);
	if (IS_ERR(group)) {
		ret = PTR_ERR(group);
		goto err_unlink;
	}

	iommu_group_put(group);
	return 0;

err_unlink:
	iommu_device_unlink(&smmu->iommu, dev);
err_disable_pasid:
	arm_smmu_disable_pasid(master);
	arm_smmu_remove_master(smmu, master);
err_free_master:
	kfree(master);
	fwspec->iommu_priv = NULL;
	return ret;
}

static void arm_smmu_remove_device(struct device *dev)
{
	struct iommu_fwspec *fwspec = dev_iommu_fwspec_get(dev);
	struct arm_smmu_master *master;
	struct arm_smmu_device *smmu;

	if (!fwspec || fwspec->ops != &arm_smmu_ops)
		return;

	master = fwspec->iommu_priv;
	smmu = master->smmu;
	iopf_queue_remove_device(smmu->evtq.iopf, dev);
	iopf_queue_remove_device(smmu->priq.iopf, dev);
	iommu_sva_disable(dev);
	arm_smmu_detach_dev(master);
	iommu_group_remove_device(dev);
	iommu_device_unlink(&smmu->iommu, dev);
	arm_smmu_disable_pasid(master);
	arm_smmu_remove_master(smmu, master);
	kfree(master);
	iommu_fwspec_free(dev);
}

static struct iommu_group *arm_smmu_device_group(struct device *dev)
{
	struct iommu_group *group;

	/*
	 * We don't support devices sharing stream IDs other than PCI RID
	 * aliases, since the necessary ID-to-device lookup becomes rather
	 * impractical given a potential sparse 32-bit stream ID space.
	 */
	if (dev_is_pci(dev))
		group = pci_device_group(dev);
	else
		group = generic_device_group(dev);

	return group;
}

static int arm_smmu_domain_get_attr(struct iommu_domain *domain,
				    enum iommu_attr attr, void *data)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	switch (domain->type) {
	case IOMMU_DOMAIN_UNMANAGED:
		switch (attr) {
		case DOMAIN_ATTR_NESTING:
			*(int *)data = (smmu_domain->stage == ARM_SMMU_DOMAIN_NESTED);
			return 0;
		default:
			return -ENODEV;
		}
		break;
	case IOMMU_DOMAIN_DMA:
		switch (attr) {
		case DOMAIN_ATTR_DMA_USE_FLUSH_QUEUE:
			*(int *)data = smmu_domain->non_strict;
			return 0;
		default:
			return -ENODEV;
		}
		break;
	default:
		return -EINVAL;
	}
}

static int arm_smmu_domain_set_attr(struct iommu_domain *domain,
				    enum iommu_attr attr, void *data)
{
	int ret = 0;
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	mutex_lock(&smmu_domain->init_mutex);

	switch (domain->type) {
	case IOMMU_DOMAIN_UNMANAGED:
		switch (attr) {
		case DOMAIN_ATTR_NESTING:
			if (smmu_domain->smmu) {
				ret = -EPERM;
				goto out_unlock;
			}

			if (*(int *)data)
				smmu_domain->stage = ARM_SMMU_DOMAIN_NESTED;
			else
				smmu_domain->stage = ARM_SMMU_DOMAIN_S1;
			break;
		default:
			ret = -ENODEV;
		}
		break;
	case IOMMU_DOMAIN_DMA:
		switch(attr) {
		case DOMAIN_ATTR_DMA_USE_FLUSH_QUEUE:
			smmu_domain->non_strict = *(int *)data;
			break;
		default:
			ret = -ENODEV;
		}
		break;
	default:
		ret = -EINVAL;
	}

out_unlock:
	mutex_unlock(&smmu_domain->init_mutex);
	return ret;
}

static int arm_smmu_of_xlate(struct device *dev, struct of_phandle_args *args)
{
	return iommu_fwspec_add_ids(dev, args->args, 1);
}

static void arm_smmu_get_resv_regions(struct device *dev,
				      struct list_head *head)
{
	struct iommu_resv_region *region;
	int prot = IOMMU_WRITE | IOMMU_NOEXEC | IOMMU_MMIO;

	region = iommu_alloc_resv_region(MSI_IOVA_BASE, MSI_IOVA_LENGTH,
					 prot, IOMMU_RESV_SW_MSI);
	if (!region)
		return;

	list_add_tail(&region->list, head);

	iommu_dma_get_resv_regions(dev, head);
}

static bool arm_smmu_iopf_supported(struct arm_smmu_master *master)
{
	return master->stall_enabled || master->pri_supported;
}

static bool arm_smmu_dev_has_feature(struct device *dev,
				     enum iommu_dev_features feat)
{
	struct arm_smmu_master *master = dev_to_master(dev);

	if (!master)
		return false;

	switch (feat) {
	case IOMMU_DEV_FEAT_SVA:
		if (!(master->smmu->features & ARM_SMMU_FEAT_SVA))
			return false;

		/* SSID and IOPF support are mandatory for the moment */
		return master->ssid_bits && arm_smmu_iopf_supported(master);
	case IOMMU_DEV_FEAT_AUX:
		return master->ssid_bits != 0;
	default:
		return false;
	}
}

static bool arm_smmu_dev_feature_enabled(struct device *dev,
					 enum iommu_dev_features feat)
{
	struct arm_smmu_master *master = dev_to_master(dev);

	if (!master)
		return false;

	switch (feat) {
	case IOMMU_DEV_FEAT_SVA:
		return iommu_sva_enabled(dev);
	case IOMMU_DEV_FEAT_AUX:
		return master->auxd_enabled;
	default:
		return false;
	}
}

static int arm_smmu_dev_enable_sva(struct device *dev)
{
	int ret;
	struct arm_smmu_master *master = dev_to_master(dev);
	struct iommu_sva_param param = {
		.min_pasid = 1,
		.max_pasid = 0xfffffU,
	};

	param.max_pasid = min(param.max_pasid, (1U << master->ssid_bits) - 1);

	ret = iommu_sva_enable(dev, &param);
	if (ret)
		return ret;

	if (master->stall_enabled) {
		ret = iopf_queue_add_device(master->smmu->evtq.iopf, dev);
		if (ret)
			goto err_disable_sva;
	} else if (master->pri_supported) {
		ret = iopf_queue_add_device(master->smmu->priq.iopf, dev);
		if (ret)
			goto err_disable_sva;

		if (arm_smmu_enable_pri(master)) {
			iopf_queue_remove_device(master->smmu->priq.iopf, dev);
			goto err_disable_sva;
		}
	}

	return 0;

err_disable_sva:
	iommu_sva_disable(dev);
	return ret;
}

static int arm_smmu_dev_enable_feature(struct device *dev,
				       enum iommu_dev_features feat)
{
	struct arm_smmu_master *master = dev_to_master(dev);

	if (!arm_smmu_dev_has_feature(dev, feat))
		return -ENODEV;

	if (arm_smmu_dev_feature_enabled(dev, feat))
		return -EBUSY;

	switch (feat) {
	case IOMMU_DEV_FEAT_SVA:
		return arm_smmu_dev_enable_sva(dev);
	case IOMMU_DEV_FEAT_AUX:
		master->auxd_enabled = true;
		return 0;
	default:
		return -EINVAL;
	}
}

static int arm_smmu_dev_disable_feature(struct device *dev,
					enum iommu_dev_features feat)
{
	struct arm_smmu_master *master = dev_to_master(dev);

	if (!arm_smmu_dev_feature_enabled(dev, feat))
		return -EINVAL;

	switch (feat) {
	case IOMMU_DEV_FEAT_SVA:
		arm_smmu_disable_pri(master);
		iopf_queue_remove_device(master->smmu->evtq.iopf, dev);
		iopf_queue_remove_device(master->smmu->priq.iopf, dev);
		return iommu_sva_disable(dev);
	case IOMMU_DEV_FEAT_AUX:
		/* TODO: check if aux domains are still attached? */
		master->auxd_enabled = false;
		return 0;
	default:
		return -EINVAL;
	}
}

static int arm_smmu_aux_attach_dev(struct iommu_domain *domain, struct device *dev)
{
	int ret;
	struct iommu_domain *parent_domain;
	struct arm_smmu_domain *parent_smmu_domain;
	struct arm_smmu_master *master = dev_to_master(dev);
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (!arm_smmu_dev_feature_enabled(dev, IOMMU_DEV_FEAT_AUX))
		return -EINVAL;

	parent_domain = iommu_get_domain_for_dev(dev);
	if (!parent_domain)
		return -EINVAL;
	parent_smmu_domain = to_smmu_domain(parent_domain);

	mutex_lock(&smmu_domain->init_mutex);
	if (smmu_domain->stage != ARM_SMMU_DOMAIN_S1 ||
	    parent_smmu_domain->stage != ARM_SMMU_DOMAIN_S1) {
		ret = -EINVAL;
		goto out_unlock;
	} else if (smmu_domain->s1_cfg.cdcfg.cdtab) {
		/* Already attached as a normal domain */
		dev_err(dev, "cannot attach domain in auxiliary mode\n");
		ret = -EINVAL;
		goto out_unlock;
	} else if (!smmu_domain->smmu) {
		ioasid_t ssid = ioasid_alloc(&private_ioasid, 1,
					     (1UL << master->ssid_bits) - 1,
					     NULL);
		if (ssid == INVALID_IOASID) {
			ret = -EINVAL;
			goto out_unlock;
		}
		smmu_domain->smmu = master->smmu;
		smmu_domain->parent = parent_smmu_domain;
		smmu_domain->ssid = ssid;

		ret = arm_smmu_domain_finalise(domain, master);
		if (ret) {
			smmu_domain->smmu = NULL;
			smmu_domain->ssid = 0;
			smmu_domain->parent = NULL;
			ioasid_free(ssid);
			goto out_unlock;
		}
	} else if (smmu_domain->parent != parent_smmu_domain) {
		/* Additional restriction: an aux domain has a single parent */
		dev_err(dev, "cannot attach aux domain with different parent\n");
		ret = -EINVAL;
		goto out_unlock;
	}

	if (!smmu_domain->aux_nr_devs++)
		arm_smmu_write_ctx_desc(parent_smmu_domain, smmu_domain->ssid,
					&smmu_domain->s1_cfg.cd);
	/*
	 * Note that all other devices attached to the parent domain can now
	 * access this context as well.
	 */

out_unlock:
	mutex_unlock(&smmu_domain->init_mutex);
	return ret;
}

static void arm_smmu_aux_detach_dev(struct iommu_domain *domain, struct device *dev)
{
	struct iommu_domain *parent_domain;
	struct arm_smmu_domain *parent_smmu_domain;
	struct arm_smmu_master *master = dev_to_master(dev);
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (!arm_smmu_dev_feature_enabled(dev, IOMMU_DEV_FEAT_AUX))
		return;

	parent_domain = iommu_get_domain_for_dev(dev);
	if (!parent_domain)
		return;
	parent_smmu_domain = to_smmu_domain(parent_domain);

	mutex_lock(&smmu_domain->init_mutex);
	if (!smmu_domain->aux_nr_devs)
		goto out_unlock;

	if (!--smmu_domain->aux_nr_devs) {
		arm_smmu_write_ctx_desc(parent_smmu_domain, smmu_domain->ssid,
					NULL);
		/*
		 * TLB doesn't need invalidation since accesses from the device
		 * can't use this domain's ASID once the CD is clear.
		 *
		 * Sadly that doesn't apply to ATCs, which are PASID tagged.
		 * Invalidate all other devices as well, because even though
		 * they weren't 'officially' attached to the auxiliary domain,
		 * they could have formed ATC entries.
		 */
		arm_smmu_atc_inv_domain(parent_smmu_domain, smmu_domain->ssid,
					0, 0);
	} else {
		/* Invalidate only this device's ATC */
		arm_smmu_atc_inv_master(master, smmu_domain->ssid);
	}
out_unlock:
	mutex_unlock(&smmu_domain->init_mutex);
}

static int arm_smmu_aux_get_pasid(struct iommu_domain *domain, struct device *dev)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	return smmu_domain->ssid ?: -EINVAL;
}

static struct iommu_ops arm_smmu_ops = {
	.capable		= arm_smmu_capable,
	.domain_alloc		= arm_smmu_domain_alloc,
	.domain_free		= arm_smmu_domain_free,
	.attach_dev		= arm_smmu_attach_dev,
	.map			= arm_smmu_map,
	.unmap			= arm_smmu_unmap,
	.flush_iotlb_all	= arm_smmu_flush_iotlb_all,
	.iotlb_sync		= arm_smmu_iotlb_sync,
	.iova_to_phys		= arm_smmu_iova_to_phys,
	.add_device		= arm_smmu_add_device,
	.remove_device		= arm_smmu_remove_device,
	.device_group		= arm_smmu_device_group,
	.domain_get_attr	= arm_smmu_domain_get_attr,
	.domain_set_attr	= arm_smmu_domain_set_attr,
	.of_xlate		= arm_smmu_of_xlate,
	.get_resv_regions	= arm_smmu_get_resv_regions,
	.put_resv_regions	= generic_iommu_put_resv_regions,
	.dev_has_feat		= arm_smmu_dev_has_feature,
	.dev_feat_enabled	= arm_smmu_dev_feature_enabled,
	.dev_enable_feat	= arm_smmu_dev_enable_feature,
	.dev_disable_feat	= arm_smmu_dev_disable_feature,
	.sva_bind		= arm_smmu_sva_bind,
	.sva_unbind		= iommu_sva_unbind_generic,
	.sva_get_pasid		= iommu_sva_get_pasid_generic,
	.page_response		= arm_smmu_page_response,
	.aux_attach_dev		= arm_smmu_aux_attach_dev,
	.aux_detach_dev		= arm_smmu_aux_detach_dev,
	.aux_get_pasid		= arm_smmu_aux_get_pasid,
	.pgsize_bitmap		= -1UL, /* Restricted during device attach */
};

/* Probing and initialisation functions */
static int arm_smmu_init_one_queue(struct arm_smmu_device *smmu,
				   struct arm_smmu_queue *q,
				   unsigned long prod_off,
				   unsigned long cons_off,
				   size_t dwords, const char *name)
{
	size_t qsz;

	do {
		qsz = ((1 << q->llq.max_n_shift) * dwords) << 3;
		q->base = dmam_alloc_coherent(smmu->dev, qsz, &q->base_dma,
					      GFP_KERNEL);
		if (q->base || qsz < PAGE_SIZE)
			break;

		q->llq.max_n_shift--;
	} while (1);

	if (!q->base) {
		dev_err(smmu->dev,
			"failed to allocate queue (0x%zx bytes) for %s\n",
			qsz, name);
		return -ENOMEM;
	}

	if (!WARN_ON(q->base_dma & (qsz - 1))) {
		dev_info(smmu->dev, "allocated %u entries for %s\n",
			 1 << q->llq.max_n_shift, name);
	}

	q->prod_reg	= arm_smmu_page1_fixup(prod_off, smmu);
	q->cons_reg	= arm_smmu_page1_fixup(cons_off, smmu);
	q->ent_dwords	= dwords;

	q->q_base  = Q_BASE_RWA;
	q->q_base |= q->base_dma & Q_BASE_ADDR_MASK;
	q->q_base |= FIELD_PREP(Q_BASE_LOG2SIZE, q->llq.max_n_shift);

	q->llq.prod = q->llq.cons = 0;

	init_waitqueue_head(&q->wq);
	q->batch = 0;

	return 0;
}

static void arm_smmu_cmdq_free_bitmap(void *data)
{
	unsigned long *bitmap = data;
	bitmap_free(bitmap);
}

static int arm_smmu_cmdq_init(struct arm_smmu_device *smmu)
{
	int ret = 0;
	struct arm_smmu_cmdq *cmdq = &smmu->cmdq;
	unsigned int nents = 1 << cmdq->q.llq.max_n_shift;
	atomic_long_t *bitmap;

	atomic_set(&cmdq->owner_prod, 0);
	atomic_set(&cmdq->lock, 0);

	bitmap = (atomic_long_t *)bitmap_zalloc(nents, GFP_KERNEL);
	if (!bitmap) {
		dev_err(smmu->dev, "failed to allocate cmdq bitmap\n");
		ret = -ENOMEM;
	} else {
		cmdq->valid_map = bitmap;
		devm_add_action(smmu->dev, arm_smmu_cmdq_free_bitmap, bitmap);
	}

	return ret;
}

static int arm_smmu_init_queues(struct arm_smmu_device *smmu)
{
	int ret;

	/* cmdq */
	ret = arm_smmu_init_one_queue(smmu, &smmu->cmdq.q, ARM_SMMU_CMDQ_PROD,
				      ARM_SMMU_CMDQ_CONS, CMDQ_ENT_DWORDS,
				      "cmdq");
	if (ret)
		return ret;

	ret = arm_smmu_cmdq_init(smmu);
	if (ret)
		return ret;

	/* evtq */
	ret = arm_smmu_init_one_queue(smmu, &smmu->evtq.q, ARM_SMMU_EVTQ_PROD,
				      ARM_SMMU_EVTQ_CONS, EVTQ_ENT_DWORDS,
				      "evtq");
	if (ret)
		return ret;

	if (smmu->features & ARM_SMMU_FEAT_STALLS) {
		smmu->evtq.iopf = iopf_queue_alloc(dev_name(smmu->dev),
						   arm_smmu_flush_evtq, smmu);
		if (!smmu->evtq.iopf)
			return -ENOMEM;
	}

	/* priq */
	if (!(smmu->features & ARM_SMMU_FEAT_PRI))
		return 0;

	smmu->priq.iopf = iopf_queue_alloc(dev_name(smmu->dev),
					   arm_smmu_flush_priq, smmu);
	if (!smmu->priq.iopf)
		return -ENOMEM;

	return arm_smmu_init_one_queue(smmu, &smmu->priq.q, ARM_SMMU_PRIQ_PROD,
				       ARM_SMMU_PRIQ_CONS, PRIQ_ENT_DWORDS,
				       "priq");
}

static int arm_smmu_init_l1_strtab(struct arm_smmu_device *smmu)
{
	unsigned int i;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;
	size_t size = sizeof(*cfg->l1_desc) * cfg->num_l1_ents;
	void *strtab = smmu->strtab_cfg.strtab;

	cfg->l1_desc = devm_kzalloc(smmu->dev, size, GFP_KERNEL);
	if (!cfg->l1_desc) {
		dev_err(smmu->dev, "failed to allocate l1 stream table desc\n");
		return -ENOMEM;
	}

	for (i = 0; i < cfg->num_l1_ents; ++i) {
		arm_smmu_write_strtab_l1_desc(strtab, &cfg->l1_desc[i]);
		strtab += STRTAB_L1_DESC_DWORDS << 3;
	}

	return 0;
}

static int arm_smmu_init_strtab_2lvl(struct arm_smmu_device *smmu)
{
	void *strtab;
	u64 reg;
	u32 size, l1size;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	/* Calculate the L1 size, capped to the SIDSIZE. */
	size = STRTAB_L1_SZ_SHIFT - (ilog2(STRTAB_L1_DESC_DWORDS) + 3);
	size = min(size, smmu->sid_bits - STRTAB_SPLIT);
	cfg->num_l1_ents = 1 << size;

	size += STRTAB_SPLIT;
	if (size < smmu->sid_bits)
		dev_warn(smmu->dev,
			 "2-level strtab only covers %u/%u bits of SID\n",
			 size, smmu->sid_bits);

	l1size = cfg->num_l1_ents * (STRTAB_L1_DESC_DWORDS << 3);
	strtab = dmam_alloc_coherent(smmu->dev, l1size, &cfg->strtab_dma,
				     GFP_KERNEL);
	if (!strtab) {
		dev_err(smmu->dev,
			"failed to allocate l1 stream table (%u bytes)\n",
			size);
		return -ENOMEM;
	}
	cfg->strtab = strtab;

	/* Configure strtab_base_cfg for 2 levels */
	reg  = FIELD_PREP(STRTAB_BASE_CFG_FMT, STRTAB_BASE_CFG_FMT_2LVL);
	reg |= FIELD_PREP(STRTAB_BASE_CFG_LOG2SIZE, size);
	reg |= FIELD_PREP(STRTAB_BASE_CFG_SPLIT, STRTAB_SPLIT);
	cfg->strtab_base_cfg = reg;

	return arm_smmu_init_l1_strtab(smmu);
}

static int arm_smmu_init_strtab_linear(struct arm_smmu_device *smmu)
{
	void *strtab;
	u64 reg;
	u32 size;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	size = (1 << smmu->sid_bits) * (STRTAB_STE_DWORDS << 3);
	strtab = dmam_alloc_coherent(smmu->dev, size, &cfg->strtab_dma,
				     GFP_KERNEL);
	if (!strtab) {
		dev_err(smmu->dev,
			"failed to allocate linear stream table (%u bytes)\n",
			size);
		return -ENOMEM;
	}
	cfg->strtab = strtab;
	cfg->num_l1_ents = 1 << smmu->sid_bits;

	/* Configure strtab_base_cfg for a linear table covering all SIDs */
	reg  = FIELD_PREP(STRTAB_BASE_CFG_FMT, STRTAB_BASE_CFG_FMT_LINEAR);
	reg |= FIELD_PREP(STRTAB_BASE_CFG_LOG2SIZE, smmu->sid_bits);
	cfg->strtab_base_cfg = reg;

	arm_smmu_init_bypass_stes(strtab, cfg->num_l1_ents);
	return 0;
}

static int arm_smmu_init_strtab(struct arm_smmu_device *smmu)
{
	u64 reg;
	int ret;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB)
		ret = arm_smmu_init_strtab_2lvl(smmu);
	else
		ret = arm_smmu_init_strtab_linear(smmu);

	if (ret)
		return ret;

	/* Set the strtab base address */
	reg  = smmu->strtab_cfg.strtab_dma & STRTAB_BASE_ADDR_MASK;
	reg |= STRTAB_BASE_RA;
	smmu->strtab_cfg.strtab_base = reg;

	/* Allocate the first VMID for stage-2 bypass STEs */
	set_bit(0, smmu->vmid_map);
	return 0;
}

static int arm_smmu_init_structures(struct arm_smmu_device *smmu)
{
	int ret;

	mutex_init(&smmu->streams_mutex);
	smmu->streams = RB_ROOT;

	ret = arm_smmu_init_queues(smmu);
	if (ret)
		return ret;

	return arm_smmu_init_strtab(smmu);
}

static int arm_smmu_write_reg_sync(struct arm_smmu_device *smmu, u32 val,
				   unsigned int reg_off, unsigned int ack_off)
{
	u32 reg;

	writel_relaxed(val, smmu->base + reg_off);
	return readl_relaxed_poll_timeout(smmu->base + ack_off, reg, reg == val,
					  1, ARM_SMMU_POLL_TIMEOUT_US);
}

/* GBPA is "special" */
static int arm_smmu_update_gbpa(struct arm_smmu_device *smmu, u32 set, u32 clr)
{
	int ret;
	u32 reg, __iomem *gbpa = smmu->base + ARM_SMMU_GBPA;

	ret = readl_relaxed_poll_timeout(gbpa, reg, !(reg & GBPA_UPDATE),
					 1, ARM_SMMU_POLL_TIMEOUT_US);
	if (ret)
		return ret;

	reg &= ~clr;
	reg |= set;
	writel_relaxed(reg | GBPA_UPDATE, gbpa);
	ret = readl_relaxed_poll_timeout(gbpa, reg, !(reg & GBPA_UPDATE),
					 1, ARM_SMMU_POLL_TIMEOUT_US);

	if (ret)
		dev_err(smmu->dev, "GBPA not responding to update\n");
	return ret;
}

static void arm_smmu_free_msis(void *data)
{
	struct device *dev = data;
	platform_msi_domain_free_irqs(dev);
}

static void arm_smmu_write_msi_msg(struct msi_desc *desc, struct msi_msg *msg)
{
	phys_addr_t doorbell;
	struct device *dev = msi_desc_to_dev(desc);
	struct arm_smmu_device *smmu = dev_get_drvdata(dev);
	phys_addr_t *cfg = arm_smmu_msi_cfg[desc->platform.msi_index];

	doorbell = (((u64)msg->address_hi) << 32) | msg->address_lo;
	doorbell &= MSI_CFG0_ADDR_MASK;

	writeq_relaxed(doorbell, smmu->base + cfg[0]);
	writel_relaxed(msg->data, smmu->base + cfg[1]);
	writel_relaxed(ARM_SMMU_MEMATTR_DEVICE_nGnRE, smmu->base + cfg[2]);
}

static void arm_smmu_setup_msis(struct arm_smmu_device *smmu)
{
	struct msi_desc *desc;
	int ret, nvec = ARM_SMMU_MAX_MSIS;
	struct device *dev = smmu->dev;

	/* Clear the MSI address regs */
	writeq_relaxed(0, smmu->base + ARM_SMMU_GERROR_IRQ_CFG0);
	writeq_relaxed(0, smmu->base + ARM_SMMU_EVTQ_IRQ_CFG0);

	if (smmu->features & ARM_SMMU_FEAT_PRI)
		writeq_relaxed(0, smmu->base + ARM_SMMU_PRIQ_IRQ_CFG0);
	else
		nvec--;

	if (!(smmu->features & ARM_SMMU_FEAT_MSI))
		return;

	if (!dev->msi_domain) {
		dev_info(smmu->dev, "msi_domain absent - falling back to wired irqs\n");
		return;
	}

	/* Allocate MSIs for evtq, gerror and priq. Ignore cmdq */
	ret = platform_msi_domain_alloc_irqs(dev, nvec, arm_smmu_write_msi_msg);
	if (ret) {
		dev_warn(dev, "failed to allocate MSIs - falling back to wired irqs\n");
		return;
	}

	for_each_msi_entry(desc, dev) {
		switch (desc->platform.msi_index) {
		case EVTQ_MSI_INDEX:
			smmu->evtq.q.irq = desc->irq;
			break;
		case GERROR_MSI_INDEX:
			smmu->gerr_irq = desc->irq;
			break;
		case PRIQ_MSI_INDEX:
			smmu->priq.q.irq = desc->irq;
			break;
		default:	/* Unknown */
			continue;
		}
	}

	/* Add callback to free MSIs on teardown */
	devm_add_action(dev, arm_smmu_free_msis, dev);
}

static void arm_smmu_setup_unique_irqs(struct arm_smmu_device *smmu)
{
	int irq, ret;

	arm_smmu_setup_msis(smmu);

	/* Request interrupt lines */
	irq = smmu->evtq.q.irq;
	if (irq) {
		ret = devm_request_threaded_irq(smmu->dev, irq, NULL,
						arm_smmu_evtq_thread,
						IRQF_ONESHOT,
						"arm-smmu-v3-evtq", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable evtq irq\n");
	} else {
		dev_warn(smmu->dev, "no evtq irq - events will not be reported!\n");
	}

	irq = smmu->gerr_irq;
	if (irq) {
		ret = devm_request_irq(smmu->dev, irq, arm_smmu_gerror_handler,
				       0, "arm-smmu-v3-gerror", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable gerror irq\n");
	} else {
		dev_warn(smmu->dev, "no gerr irq - errors will not be reported!\n");
	}

	if (smmu->features & ARM_SMMU_FEAT_PRI) {
		irq = smmu->priq.q.irq;
		if (irq) {
			ret = devm_request_threaded_irq(smmu->dev, irq, NULL,
							arm_smmu_priq_thread,
							IRQF_ONESHOT,
							"arm-smmu-v3-priq",
							smmu);
			if (ret < 0)
				dev_warn(smmu->dev,
					 "failed to enable priq irq\n");
		} else {
			dev_warn(smmu->dev, "no priq irq - PRI will be broken\n");
		}
	}
}

static int arm_smmu_setup_irqs(struct arm_smmu_device *smmu)
{
	int ret, irq;
	u32 irqen_flags = IRQ_CTRL_EVTQ_IRQEN | IRQ_CTRL_GERROR_IRQEN;

	/* Disable IRQs first */
	ret = arm_smmu_write_reg_sync(smmu, 0, ARM_SMMU_IRQ_CTRL,
				      ARM_SMMU_IRQ_CTRLACK);
	if (ret) {
		dev_err(smmu->dev, "failed to disable irqs\n");
		return ret;
	}

	irq = smmu->combined_irq;
	if (irq) {
		/*
		 * Cavium ThunderX2 implementation doesn't support unique irq
		 * lines. Use a single irq line for all the SMMUv3 interrupts.
		 */
		ret = devm_request_threaded_irq(smmu->dev, irq,
					arm_smmu_combined_irq_handler,
					arm_smmu_combined_irq_thread,
					IRQF_ONESHOT,
					"arm-smmu-v3-combined-irq", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable combined irq\n");
	} else
		arm_smmu_setup_unique_irqs(smmu);

	if (smmu->features & ARM_SMMU_FEAT_PRI)
		irqen_flags |= IRQ_CTRL_PRIQ_IRQEN;

	/* Enable interrupt generation on the SMMU */
	ret = arm_smmu_write_reg_sync(smmu, irqen_flags,
				      ARM_SMMU_IRQ_CTRL, ARM_SMMU_IRQ_CTRLACK);
	if (ret)
		dev_warn(smmu->dev, "failed to enable irqs\n");

	return 0;
}

static int arm_smmu_device_disable(struct arm_smmu_device *smmu)
{
	int ret;

	ret = arm_smmu_write_reg_sync(smmu, 0, ARM_SMMU_CR0, ARM_SMMU_CR0ACK);
	if (ret)
		dev_err(smmu->dev, "failed to clear cr0\n");

	return ret;
}

static int arm_smmu_device_reset(struct arm_smmu_device *smmu, bool bypass)
{
	int ret;
	u32 reg, enables;
	struct arm_smmu_cmdq_ent cmd;

	/* Clear CR0 and sync (disables SMMU and queue processing) */
	reg = readl_relaxed(smmu->base + ARM_SMMU_CR0);
	if (reg & CR0_SMMUEN) {
		dev_warn(smmu->dev, "SMMU currently enabled! Resetting...\n");
		WARN_ON(is_kdump_kernel() && !disable_bypass);
		arm_smmu_update_gbpa(smmu, GBPA_ABORT, 0);
	}

	ret = arm_smmu_device_disable(smmu);
	if (ret)
		return ret;

	/* CR1 (table and queue memory attributes) */
	reg = FIELD_PREP(CR1_TABLE_SH, ARM_SMMU_SH_ISH) |
	      FIELD_PREP(CR1_TABLE_OC, CR1_CACHE_WB) |
	      FIELD_PREP(CR1_TABLE_IC, CR1_CACHE_WB) |
	      FIELD_PREP(CR1_QUEUE_SH, ARM_SMMU_SH_ISH) |
	      FIELD_PREP(CR1_QUEUE_OC, CR1_CACHE_WB) |
	      FIELD_PREP(CR1_QUEUE_IC, CR1_CACHE_WB);
	writel_relaxed(reg, smmu->base + ARM_SMMU_CR1);

	/* CR2 (random crap) */
	reg = CR2_RECINVSID;

	if (smmu->features & ARM_SMMU_FEAT_E2H)
		reg |= CR2_E2H;

	if (!(smmu->features & ARM_SMMU_FEAT_BTM))
		reg |= CR2_PTM;

	writel_relaxed(reg, smmu->base + ARM_SMMU_CR2);

	/* Stream table */
	writeq_relaxed(smmu->strtab_cfg.strtab_base,
		       smmu->base + ARM_SMMU_STRTAB_BASE);
	writel_relaxed(smmu->strtab_cfg.strtab_base_cfg,
		       smmu->base + ARM_SMMU_STRTAB_BASE_CFG);

	/* Command queue */
	writeq_relaxed(smmu->cmdq.q.q_base, smmu->base + ARM_SMMU_CMDQ_BASE);
	writel_relaxed(smmu->cmdq.q.llq.prod, smmu->base + ARM_SMMU_CMDQ_PROD);
	writel_relaxed(smmu->cmdq.q.llq.cons, smmu->base + ARM_SMMU_CMDQ_CONS);

	enables = CR0_CMDQEN;
	ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
				      ARM_SMMU_CR0ACK);
	if (ret) {
		dev_err(smmu->dev, "failed to enable command queue\n");
		return ret;
	}

	/* Invalidate any cached configuration */
	cmd.opcode = CMDQ_OP_CFGI_ALL;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	arm_smmu_cmdq_issue_sync(smmu);

	/* Invalidate any stale TLB entries */
	if (smmu->features & ARM_SMMU_FEAT_HYP) {
		cmd.opcode = CMDQ_OP_TLBI_EL2_ALL;
		arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	}

	cmd.opcode = CMDQ_OP_TLBI_NSNH_ALL;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	arm_smmu_cmdq_issue_sync(smmu);

	/* Event queue */
	writeq_relaxed(smmu->evtq.q.q_base, smmu->base + ARM_SMMU_EVTQ_BASE);
	writel_relaxed(smmu->evtq.q.llq.prod,
		       arm_smmu_page1_fixup(ARM_SMMU_EVTQ_PROD, smmu));
	writel_relaxed(smmu->evtq.q.llq.cons,
		       arm_smmu_page1_fixup(ARM_SMMU_EVTQ_CONS, smmu));

	enables |= CR0_EVTQEN;
	ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
				      ARM_SMMU_CR0ACK);
	if (ret) {
		dev_err(smmu->dev, "failed to enable event queue\n");
		return ret;
	}

	/* PRI queue */
	if (smmu->features & ARM_SMMU_FEAT_PRI) {
		writeq_relaxed(smmu->priq.q.q_base,
			       smmu->base + ARM_SMMU_PRIQ_BASE);
		writel_relaxed(smmu->priq.q.llq.prod,
			       arm_smmu_page1_fixup(ARM_SMMU_PRIQ_PROD, smmu));
		writel_relaxed(smmu->priq.q.llq.cons,
			       arm_smmu_page1_fixup(ARM_SMMU_PRIQ_CONS, smmu));

		enables |= CR0_PRIQEN;
		ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
					      ARM_SMMU_CR0ACK);
		if (ret) {
			dev_err(smmu->dev, "failed to enable PRI queue\n");
			return ret;
		}
	}

	if (smmu->features & ARM_SMMU_FEAT_ATS) {
		enables |= CR0_ATSCHK;
		ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
					      ARM_SMMU_CR0ACK);
		if (ret) {
			dev_err(smmu->dev, "failed to enable ATS check\n");
			return ret;
		}
	}

	ret = arm_smmu_setup_irqs(smmu);
	if (ret) {
		dev_err(smmu->dev, "failed to setup irqs\n");
		return ret;
	}

	if (is_kdump_kernel())
		enables &= ~(CR0_EVTQEN | CR0_PRIQEN);

	/* Enable the SMMU interface, or ensure bypass */
	if (!bypass || disable_bypass) {
		enables |= CR0_SMMUEN;
	} else {
		ret = arm_smmu_update_gbpa(smmu, 0, GBPA_ABORT);
		if (ret)
			return ret;
	}
	ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
				      ARM_SMMU_CR0ACK);
	if (ret) {
		dev_err(smmu->dev, "failed to enable SMMU interface\n");
		return ret;
	}

	return 0;
}

static bool arm_smmu_supports_sva(struct arm_smmu_device *smmu)
{
	unsigned long reg, fld;
	unsigned long oas;
	unsigned long asid_bits;

	u32 feat_mask = ARM_SMMU_FEAT_BTM | ARM_SMMU_FEAT_COHERENCY;

	if ((smmu->features & feat_mask) != feat_mask)
		return false;

	if (!(smmu->pgsize_bitmap & PAGE_SIZE))
		return false;

	/*
	 * Get the smallest PA size of all CPUs (sanitized by cpufeature). We're
	 * not even pretending to support AArch32 here.
	 */
	reg = read_sanitised_ftr_reg(SYS_ID_AA64MMFR0_EL1);
	fld = cpuid_feature_extract_unsigned_field(reg, ID_AA64MMFR0_PARANGE_SHIFT);
	switch (fld) {
	case 0x0:
		oas = 32;
		break;
	case 0x1:
		oas = 36;
		break;
	case 0x2:
		oas = 40;
		break;
	case 0x3:
		oas = 42;
		break;
	case 0x4:
		oas = 44;
		break;
	case 0x5:
		oas = 48;
		break;
	case 0x6:
		oas = 52;
		break;
	default:
		return false;
	}

	/* abort if MMU outputs addresses greater than what we support. */
	if (smmu->oas < oas)
		return false;

	/* We can support bigger ASIDs than the CPU, but not smaller */
	fld = cpuid_feature_extract_unsigned_field(reg, ID_AA64MMFR0_ASID_SHIFT);
	asid_bits = fld ? 16 : 8;
	if (smmu->asid_bits < asid_bits)
		return false;

	/*
	 * See max_pinned_asids in arch/arm64/mm/context.c. The following is
	 * generally the maximum number of bindable processes.
	 */
	if (IS_ENABLED(CONFIG_UNMAP_KERNEL_AT_EL0))
		asid_bits--;
	dev_dbg(smmu->dev, "%d shared contexts\n", (1 << asid_bits) -
		num_possible_cpus() - 2);

	return true;
}

static int arm_smmu_device_hw_probe(struct arm_smmu_device *smmu)
{
	u32 reg;
	bool coherent = smmu->features & ARM_SMMU_FEAT_COHERENCY;
	bool vhe = cpus_have_cap(ARM64_HAS_VIRT_HOST_EXTN);

	/* IDR0 */
	reg = readl_relaxed(smmu->base + ARM_SMMU_IDR0);

	/* 2-level structures */
	if (FIELD_GET(IDR0_ST_LVL, reg) == IDR0_ST_LVL_2LVL)
		smmu->features |= ARM_SMMU_FEAT_2_LVL_STRTAB;

	if (reg & IDR0_CD2L)
		smmu->features |= ARM_SMMU_FEAT_2_LVL_CDTAB;

	/*
	 * Translation table endianness.
	 * We currently require the same endianness as the CPU, but this
	 * could be changed later by adding a new IO_PGTABLE_QUIRK.
	 */
	switch (FIELD_GET(IDR0_TTENDIAN, reg)) {
	case IDR0_TTENDIAN_MIXED:
		smmu->features |= ARM_SMMU_FEAT_TT_LE | ARM_SMMU_FEAT_TT_BE;
		break;
#ifdef __BIG_ENDIAN
	case IDR0_TTENDIAN_BE:
		smmu->features |= ARM_SMMU_FEAT_TT_BE;
		break;
#else
	case IDR0_TTENDIAN_LE:
		smmu->features |= ARM_SMMU_FEAT_TT_LE;
		break;
#endif
	default:
		dev_err(smmu->dev, "unknown/unsupported TT endianness!\n");
		return -ENXIO;
	}

	/* Boolean feature flags */
	if (IS_ENABLED(CONFIG_PCI_PRI) && reg & IDR0_PRI)
		smmu->features |= ARM_SMMU_FEAT_PRI;

	if (IS_ENABLED(CONFIG_PCI_ATS) && reg & IDR0_ATS)
		smmu->features |= ARM_SMMU_FEAT_ATS;

	if (reg & IDR0_SEV)
		smmu->features |= ARM_SMMU_FEAT_SEV;

	if (reg & IDR0_MSI)
		smmu->features |= ARM_SMMU_FEAT_MSI;

	if (reg & IDR0_HYP) {
		smmu->features |= ARM_SMMU_FEAT_HYP;
		if (vhe)
			smmu->features |= ARM_SMMU_FEAT_E2H;
	}

	if (reg & (IDR0_HA | IDR0_HD)) {
		smmu->features |= ARM_SMMU_FEAT_HA;
		if (reg & IDR0_HD)
			smmu->features |= ARM_SMMU_FEAT_HD;
	}

	/*
	 * If the CPU is using VHE, but the SMMU doesn't support it, the SMMU
	 * will create TLB entries for NH-EL1 world and will miss the
	 * broadcasted TLB invalidations that target EL2-E2H world. Don't enable
	 * BTM in that case.
	 */
	if (reg & IDR0_BTM && (!vhe || reg & IDR0_HYP))
		smmu->features |= ARM_SMMU_FEAT_BTM;

	/*
	 * The coherency feature as set by FW is used in preference to the ID
	 * register, but warn on mismatch.
	 */
	if (!!(reg & IDR0_COHACC) != coherent)
		dev_warn(smmu->dev, "IDR0.COHACC overridden by FW configuration (%s)\n",
			 coherent ? "true" : "false");

	switch (FIELD_GET(IDR0_STALL_MODEL, reg)) {
	case IDR0_STALL_MODEL_FORCE:
		smmu->features |= ARM_SMMU_FEAT_STALL_FORCE;
		/* Fallthrough */
	case IDR0_STALL_MODEL_STALL:
		smmu->features |= ARM_SMMU_FEAT_STALLS;
	}

	if (reg & IDR0_S1P)
		smmu->features |= ARM_SMMU_FEAT_TRANS_S1;

	if (reg & IDR0_S2P)
		smmu->features |= ARM_SMMU_FEAT_TRANS_S2;

	if (!(reg & (IDR0_S1P | IDR0_S2P))) {
		dev_err(smmu->dev, "no translation support!\n");
		return -ENXIO;
	}

	/* We only support the AArch64 table format at present */
	switch (FIELD_GET(IDR0_TTF, reg)) {
	case IDR0_TTF_AARCH32_64:
		smmu->ias = 40;
		/* Fallthrough */
	case IDR0_TTF_AARCH64:
		break;
	default:
		dev_err(smmu->dev, "AArch64 table format not supported!\n");
		return -ENXIO;
	}

	/* ASID/VMID sizes */
	smmu->asid_bits = reg & IDR0_ASID16 ? 16 : 8;
	smmu->vmid_bits = reg & IDR0_VMID16 ? 16 : 8;

	/* IDR1 */
	reg = readl_relaxed(smmu->base + ARM_SMMU_IDR1);
	if (reg & (IDR1_TABLES_PRESET | IDR1_QUEUES_PRESET | IDR1_REL)) {
		dev_err(smmu->dev, "embedded implementation not supported\n");
		return -ENXIO;
	}

	/* Queue sizes, capped to ensure natural alignment */
	smmu->cmdq.q.llq.max_n_shift = min_t(u32, CMDQ_MAX_SZ_SHIFT,
					     FIELD_GET(IDR1_CMDQS, reg));
	if (smmu->cmdq.q.llq.max_n_shift <= ilog2(CMDQ_BATCH_ENTRIES)) {
		/*
		 * We don't support splitting up batches, so one batch of
		 * commands plus an extra sync needs to fit inside the command
		 * queue. There's also no way we can handle the weird alignment
		 * restrictions on the base pointer for a unit-length queue.
		 */
		dev_err(smmu->dev, "command queue size <= %d entries not supported\n",
			CMDQ_BATCH_ENTRIES);
		return -ENXIO;
	}

	smmu->evtq.q.llq.max_n_shift = min_t(u32, EVTQ_MAX_SZ_SHIFT,
					     FIELD_GET(IDR1_EVTQS, reg));
	smmu->priq.q.llq.max_n_shift = min_t(u32, PRIQ_MAX_SZ_SHIFT,
					     FIELD_GET(IDR1_PRIQS, reg));

	/* SID/SSID sizes */
	smmu->ssid_bits = FIELD_GET(IDR1_SSIDSIZE, reg);
	smmu->sid_bits = FIELD_GET(IDR1_SIDSIZE, reg);

	/*
	 * If the SMMU supports fewer bits than would fill a single L2 stream
	 * table, use a linear table instead.
	 */
	if (smmu->sid_bits <= STRTAB_SPLIT)
		smmu->features &= ~ARM_SMMU_FEAT_2_LVL_STRTAB;

	/* IDR5 */
	reg = readl_relaxed(smmu->base + ARM_SMMU_IDR5);

	/* Maximum number of outstanding stalls */
	smmu->evtq.max_stalls = FIELD_GET(IDR5_STALL_MAX, reg);

	/* Page sizes */
	if (reg & IDR5_GRAN64K)
		smmu->pgsize_bitmap |= SZ_64K | SZ_512M;
	if (reg & IDR5_GRAN16K)
		smmu->pgsize_bitmap |= SZ_16K | SZ_32M;
	if (reg & IDR5_GRAN4K)
		smmu->pgsize_bitmap |= SZ_4K | SZ_2M | SZ_1G;

	/* Input address size */
	if (FIELD_GET(IDR5_VAX, reg) == IDR5_VAX_52_BIT)
		smmu->features |= ARM_SMMU_FEAT_VAX;

	/* Output address size */
	switch (FIELD_GET(IDR5_OAS, reg)) {
	case IDR5_OAS_32_BIT:
		smmu->oas = 32;
		break;
	case IDR5_OAS_36_BIT:
		smmu->oas = 36;
		break;
	case IDR5_OAS_40_BIT:
		smmu->oas = 40;
		break;
	case IDR5_OAS_42_BIT:
		smmu->oas = 42;
		break;
	case IDR5_OAS_44_BIT:
		smmu->oas = 44;
		break;
	case IDR5_OAS_52_BIT:
		smmu->oas = 52;
		smmu->pgsize_bitmap |= 1ULL << 42; /* 4TB */
		break;
	default:
		dev_info(smmu->dev,
			"unknown output address size. Truncating to 48-bit\n");
		/* Fallthrough */
	case IDR5_OAS_48_BIT:
		smmu->oas = 48;
	}

	if (arm_smmu_ops.pgsize_bitmap == -1UL)
		arm_smmu_ops.pgsize_bitmap = smmu->pgsize_bitmap;
	else
		arm_smmu_ops.pgsize_bitmap |= smmu->pgsize_bitmap;

	/* Set the DMA mask for our table walker */
	if (dma_set_mask_and_coherent(smmu->dev, DMA_BIT_MASK(smmu->oas)))
		dev_warn(smmu->dev,
			 "failed to set DMA mask for table walker\n");

	smmu->ias = max(smmu->ias, smmu->oas);

	if (arm_smmu_supports_sva(smmu))
		smmu->features |= ARM_SMMU_FEAT_SVA;

	dev_info(smmu->dev, "ias %lu-bit, oas %lu-bit (features 0x%08x)\n",
		 smmu->ias, smmu->oas, smmu->features);
	return 0;
}

#ifdef CONFIG_ACPI
static void acpi_smmu_get_options(u32 model, struct arm_smmu_device *smmu)
{
	switch (model) {
	case ACPI_IORT_SMMU_V3_CAVIUM_CN99XX:
		smmu->options |= ARM_SMMU_OPT_PAGE0_REGS_ONLY;
		break;
	case ACPI_IORT_SMMU_V3_HISILICON_HI161X:
		smmu->options |= ARM_SMMU_OPT_SKIP_PREFETCH;
		break;
	}

	dev_notice(smmu->dev, "option mask 0x%x\n", smmu->options);
}

static int arm_smmu_device_acpi_probe(struct platform_device *pdev,
				      struct arm_smmu_device *smmu)
{
	struct acpi_iort_smmu_v3 *iort_smmu;
	struct device *dev = smmu->dev;
	struct acpi_iort_node *node;

	node = *(struct acpi_iort_node **)dev_get_platdata(dev);

	/* Retrieve SMMUv3 specific data */
	iort_smmu = (struct acpi_iort_smmu_v3 *)node->node_data;

	acpi_smmu_get_options(iort_smmu->model, smmu);

	if (iort_smmu->flags & ACPI_IORT_SMMU_V3_COHACC_OVERRIDE)
		smmu->features |= ARM_SMMU_FEAT_COHERENCY;

	return 0;
}
#else
static inline int arm_smmu_device_acpi_probe(struct platform_device *pdev,
					     struct arm_smmu_device *smmu)
{
	return -ENODEV;
}
#endif

static int arm_smmu_device_dt_probe(struct platform_device *pdev,
				    struct arm_smmu_device *smmu)
{
	struct device *dev = &pdev->dev;
	u32 cells;
	int ret = -EINVAL;

	if (of_property_read_u32(dev->of_node, "#iommu-cells", &cells))
		dev_err(dev, "missing #iommu-cells property\n");
	else if (cells != 1)
		dev_err(dev, "invalid #iommu-cells value (%d)\n", cells);
	else
		ret = 0;

	parse_driver_options(smmu);

	if (of_dma_is_coherent(dev->of_node))
		smmu->features |= ARM_SMMU_FEAT_COHERENCY;

	return ret;
}

static unsigned long arm_smmu_resource_size(struct arm_smmu_device *smmu)
{
	if (smmu->options & ARM_SMMU_OPT_PAGE0_REGS_ONLY)
		return SZ_64K;
	else
		return SZ_128K;
}

static int arm_smmu_set_bus_ops(struct iommu_ops *ops)
{
	int err;

#ifdef CONFIG_PCI
	if (pci_bus_type.iommu_ops != ops) {
		err = bus_set_iommu(&pci_bus_type, ops);
		if (err)
			return err;
	}
#endif
#ifdef CONFIG_ARM_AMBA
	if (amba_bustype.iommu_ops != ops) {
		err = bus_set_iommu(&amba_bustype, ops);
		if (err)
			goto err_reset_pci_ops;
	}
#endif
	if (platform_bus_type.iommu_ops != ops) {
		err = bus_set_iommu(&platform_bus_type, ops);
		if (err)
			goto err_reset_amba_ops;
	}

	return 0;

err_reset_amba_ops:
#ifdef CONFIG_ARM_AMBA
	bus_set_iommu(&amba_bustype, NULL);
#endif
err_reset_pci_ops: __maybe_unused;
#ifdef CONFIG_PCI
	bus_set_iommu(&pci_bus_type, NULL);
#endif
	return err;
}

static int arm_smmu_device_probe(struct platform_device *pdev)
{
	int irq, ret;
	struct resource *res;
	resource_size_t ioaddr;
	struct arm_smmu_device *smmu;
	struct device *dev = &pdev->dev;
	bool bypass;

	smmu = devm_kzalloc(dev, sizeof(*smmu), GFP_KERNEL);
	if (!smmu) {
		dev_err(dev, "failed to allocate arm_smmu_device\n");
		return -ENOMEM;
	}
	smmu->dev = dev;

	if (dev->of_node) {
		ret = arm_smmu_device_dt_probe(pdev, smmu);
	} else {
		ret = arm_smmu_device_acpi_probe(pdev, smmu);
		if (ret == -ENODEV)
			return ret;
	}

	/* Set bypass mode according to firmware probing result */
	bypass = !!ret;

	/* Base address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (resource_size(res) < arm_smmu_resource_size(smmu)) {
		dev_err(dev, "MMIO region too small (%pr)\n", res);
		return -EINVAL;
	}
	ioaddr = res->start;

	smmu->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(smmu->base))
		return PTR_ERR(smmu->base);

	/* Interrupt lines */

	irq = platform_get_irq_byname_optional(pdev, "combined");
	if (irq > 0)
		smmu->combined_irq = irq;
	else {
		irq = platform_get_irq_byname_optional(pdev, "eventq");
		if (irq > 0)
			smmu->evtq.q.irq = irq;

		irq = platform_get_irq_byname_optional(pdev, "priq");
		if (irq > 0)
			smmu->priq.q.irq = irq;

		irq = platform_get_irq_byname_optional(pdev, "gerror");
		if (irq > 0)
			smmu->gerr_irq = irq;
	}
	/* Probe the h/w */
	ret = arm_smmu_device_hw_probe(smmu);
	if (ret)
		return ret;

	/* Initialise in-memory data structures */
	ret = arm_smmu_init_structures(smmu);
	if (ret)
		return ret;

	/* Record our private device structure */
	platform_set_drvdata(pdev, smmu);

	/* Reset the device */
	ret = arm_smmu_device_reset(smmu, bypass);
	if (ret)
		return ret;

	/* And we're up. Go go go! */
	ret = iommu_device_sysfs_add(&smmu->iommu, dev, NULL,
				     "smmu3.%pa", &ioaddr);
	if (ret)
		return ret;

	iommu_device_set_ops(&smmu->iommu, &arm_smmu_ops);
	iommu_device_set_fwnode(&smmu->iommu, dev->fwnode);

	ret = iommu_device_register(&smmu->iommu);
	if (ret) {
		dev_err(dev, "Failed to register iommu\n");
		return ret;
	}

	return arm_smmu_set_bus_ops(&arm_smmu_ops);
}

static int arm_smmu_device_remove(struct platform_device *pdev)
{
	struct arm_smmu_device *smmu = platform_get_drvdata(pdev);

	arm_smmu_set_bus_ops(NULL);
	iommu_device_unregister(&smmu->iommu);
	iommu_device_sysfs_remove(&smmu->iommu);
	arm_smmu_device_disable(smmu);
	iopf_queue_free(smmu->evtq.iopf);
	iopf_queue_free(smmu->priq.iopf);

	return 0;
}

static void arm_smmu_device_shutdown(struct platform_device *pdev)
{
	arm_smmu_device_remove(pdev);
}

static const struct of_device_id arm_smmu_of_match[] = {
	{ .compatible = "arm,smmu-v3", },
	{ },
};
MODULE_DEVICE_TABLE(of, arm_smmu_of_match);

static struct platform_driver arm_smmu_driver = {
	.driver	= {
		.name			= "arm-smmu-v3",
		.of_match_table		= arm_smmu_of_match,
		.suppress_bind_attrs	= true,
	},
	.probe	= arm_smmu_device_probe,
	.remove	= arm_smmu_device_remove,
	.shutdown = arm_smmu_device_shutdown,
};
module_platform_driver(arm_smmu_driver);

MODULE_DESCRIPTION("IOMMU API for ARM architected SMMUv3 implementations");
MODULE_AUTHOR("Will Deacon <will@kernel.org>");
MODULE_ALIAS("platform:arm-smmu-v3");
MODULE_LICENSE("GPL v2");

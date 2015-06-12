/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (C) 2014 Hisilicon Limited
 *
 * Author: Zhen Lei <thunder.leizhen@huawei.com>
 *
 * Hisilicon smmu-v1 implementation
 *
 */

#define pr_fmt(fmt) "hisi-smmu: " fmt

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/iommu.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include <linux/amba/bus.h>

#include <asm/pgalloc.h>

/* Maximum number of stream IDs assigned to a single device */
#define MAX_MASTER_STREAMIDS	1

/* Maximum number of context banks per SMMU */
#define SMMU_MAX_CBS		256

/* SMMU global address space */
#define SMMU_GR0(smmu)		((smmu)->base)

#ifdef CONFIG_MM_OUTTER_SHAREABLE
#define SH_DOMAIN               2       /* outer shareable */
#else
#define SH_DOMAIN               3       /* inner shareable */
#endif

/* Page table bits */
#define SMMU_PTE_XN		(((pteval_t)3) << 53)
#define SMMU_PTE_CONT		(((pteval_t)1) << 52)
#define SMMU_PTE_AF		(((pteval_t)1) << 10)
#define SMMU_PTE_SH		(((pteval_t)SH_DOMAIN) << 8)
#define SMMU_PTE_PAGE		(((pteval_t)3) << 0)

#if PAGE_SIZE == SZ_4K
#define SMMU_PTE_CONT_ENTRIES	16
#elif PAGE_SIZE == SZ_64K
#define SMMU_PTE_CONT_ENTRIES	32
#else
#define SMMU_PTE_CONT_ENTRIES	1
#endif

#define SMMU_PTE_CONT_SIZE	(PAGE_SIZE * SMMU_PTE_CONT_ENTRIES)
#define SMMU_PTE_CONT_MASK	(~(SMMU_PTE_CONT_SIZE - 1))

/* Stage-1 PTE */
#define SMMU_PTE_AP_UNPRIV	(((pteval_t)1) << 6)
#define SMMU_PTE_AP_RDONLY	(((pteval_t)2) << 6)
#define SMMU_PTE_ATTRINDX_SHIFT	2
#define SMMU_PTE_nG		(((pteval_t)1) << 11)

/* Stage-2 PTE */
#define SMMU_PTE_HAP_FAULT	(((pteval_t)0) << 6)
#define SMMU_PTE_HAP_READ	(((pteval_t)1) << 6)
#define SMMU_PTE_HAP_WRITE	(((pteval_t)2) << 6)
#define SMMU_PTE_MEMATTR_OIWB	(((pteval_t)0xf) << 2)
#define SMMU_PTE_MEMATTR_NC	(((pteval_t)0x5) << 2)
#define SMMU_PTE_MEMATTR_DEV	(((pteval_t)0x1) << 2)

#define SMMU_OS_VMID		0
#define SMMU_CB_NUMIRPT		8
#define SMMU_S1CBT_SIZE		0x10000
#define SMMU_S2CBT_SIZE		0x2000
#define SMMU_S1CBT_SHIFT	16
#define SMMU_S2CBT_SHIFT	12


#define SMMU_CTRL_CR0			0x0
#define SMMU_CTRL_ACR			0x8
#define SMMU_CFG_S2CTBAR		0xc
#define SMMU_IDR0			0x10
#define SMMU_IDR1			0x14
#define SMMU_IDR2			0x18
#define SMMU_HIS_GFAR_LOW		0x20
#define SMMU_HIS_GFAR_HIGH		0x24
#define SMMU_RINT_GFSR			0x28
#define SMMU_RINT_GFSYNR		0x2c
#define SMMU_CFG_GFIM			0x30
#define SMMU_CFG_CBF			0x34
#define SMMU_TLBIALL			0x40
#define SMMU_TLBIVMID			0x44
#define SMMU_TLBISID			0x48
#define SMMU_TLBIVA_LOW			0x4c
#define SMMU_TLBIVA_HIGH		0x50
#define SMMU_TLBGSYNC			0x54
#define SMMU_TLBGSTATUS			0x58
#define SMMU_CXTIALL			0x60
#define SMMU_CXTIVMID			0x64
#define SMMU_CXTISID			0x68
#define SMMU_CXTGSYNC			0x6c
#define SMMU_CXTGSTATUS			0x70
#define SMMU_RINT_CB_FSR(n)		(0x100 + ((n) << 2))
#define SMMU_RINT_CB_FSYNR(n)		(0x120 + ((n) << 2))
#define SMMU_HIS_CB_FAR_LOW(n)		(0x140 + ((n) << 3))
#define SMMU_HIS_CB_FAR_HIGH(n)		(0x144 + ((n) << 3))
#define SMMU_CTRL_CB_RESUME(n)		(0x180 + ((n) << 2))
#define SMMU_RINT_CB_FSYNR_MSTID	0x1a0

#define SMMU_CB_S2CR(n)			(0x0  + ((n) << 5))
#define SMMU_CB_CBAR(n)			(0x4  + ((n) << 5))
#define SMMU_CB_S1CTBAR(n)		(0x18 + ((n) << 5))

#define SMMU_S1_MAIR0			0x0
#define SMMU_S1_MAIR1			0x4
#define SMMU_S1_TTBR0_L			0x8
#define SMMU_S1_TTBR0_H			0xc
#define SMMU_S1_TTBR1_L			0x10
#define SMMU_S1_TTBR1_H			0x14
#define SMMU_S1_TTBCR			0x18
#define SMMU_S1_SCTLR			0x1c

#define CFG_CBF_S1_ORGN_WA		(1 << 12)
#define CFG_CBF_S1_IRGN_WA		(1 << 10)
#define CFG_CBF_S1_SHCFG		(SH_DOMAIN << 8)
#define CFG_CBF_S2_ORGN_WA		(1 << 4)
#define CFG_CBF_S2_IRGN_WA		(1 << 2)
#define CFG_CBF_S2_SHCFG		(SH_DOMAIN << 0)

/* Configuration registers */
#define sCR0_CLIENTPD			(1 << 0)
#define sCR0_GFRE			(1 << 1)
#define sCR0_GFIE			(1 << 2)
#define sCR0_GCFGFRE			(1 << 4)
#define sCR0_GCFGFIE			(1 << 5)

#if (PAGE_SIZE == SZ_4K)
#define sACR_WC_EN			(7 << 0)
#elif (PAGE_SIZE == SZ_64K)
#define sACR_WC_EN			(3 << 5)
#else
#define sACR_WC_EN			0
#endif

#define ID0_S1TS			(1 << 30)
#define ID0_S2TS			(1 << 29)
#define ID0_NTS				(1 << 28)
#define ID0_PTFS_SHIFT			24
#define ID0_PTFS_MASK			0x2
#define ID0_PTFS_V8_ONLY		0x2
#define ID0_CTTW			(1 << 14)

#define ID2_OAS_SHIFT			8
#define ID2_OAS_MASK			0xff
#define ID2_IAS_SHIFT			0
#define ID2_IAS_MASK			0xff

#define S2CR_TYPE_SHIFT			16
#define S2CR_TYPE_MASK			0x3
#define S2CR_TYPE_TRANS			(0 << S2CR_TYPE_SHIFT)
#define S2CR_TYPE_BYPASS		(1 << S2CR_TYPE_SHIFT)
#define S2CR_TYPE_FAULT			(2 << S2CR_TYPE_SHIFT)
#define S2CR_SHCFG_NS			(3 << 8)
#define S2CR_MTCFG			(1 << 11)
#define S2CR_MEMATTR_OIWB		(0xf << 12)
#define S2CR_MTSH_WEAKEST		(S2CR_SHCFG_NS | \
				S2CR_MTCFG | S2CR_MEMATTR_OIWB)

/* Context bank attribute registers */
#define CBAR_VMID_SHIFT			0
#define CBAR_VMID_MASK			0xff
#define CBAR_S1_BPSHCFG_SHIFT		8
#define CBAR_S1_BPSHCFG_MASK		3
#define CBAR_S1_BPSHCFG_NSH		3
#define CBAR_S1_MEMATTR_SHIFT		12
#define CBAR_S1_MEMATTR_MASK		0xf
#define CBAR_S1_MEMATTR_WB		0xf
#define CBAR_TYPE_SHIFT			16
#define CBAR_TYPE_MASK			0x3
#define CBAR_TYPE_S2_TRANS		(0 << CBAR_TYPE_SHIFT)
#define CBAR_TYPE_S1_TRANS_S2_BYPASS	(1 << CBAR_TYPE_SHIFT)
#define CBAR_TYPE_S1_TRANS_S2_FAULT	(2 << CBAR_TYPE_SHIFT)
#define CBAR_TYPE_S1_TRANS_S2_TRANS	(3 << CBAR_TYPE_SHIFT)
#define CBAR_IRPTNDX_SHIFT		24
#define CBAR_IRPTNDX_MASK		0xff

#define SMMU_CB_BASE(smmu)		((smmu)->s1cbt)
#define SMMU_CB(smmu, n)		((n) << 5)

#define sTLBGSTATUS_GSACTIVE		(1 << 0)
#define TLB_LOOP_TIMEOUT		1000000	/* 1s! */

#define SCTLR_WACFG_WA		(2 << 26)
#define SCTLR_RACFG_RA		(2 << 24)
#ifdef CONFIG_P660_2P
#define SCTLR_SHCFG		(1 << 22)
#else
#define SCTLR_SHCFG		(2 << 22)
#endif
#define SCTLR_MTCFG		(1 << 20)
#define SCTLR_MEMATTR_WB	(0xf << 16)
#define SCTLR_MEMATTR_NC	(0x5 << 16)
#define SCTLR_MEMATTR_NGNRE	(0x1 << 16)
#define SCTLR_CACHE_WBRAWA	(SCTLR_WACFG_WA | SCTLR_RACFG_RA | \
			SCTLR_SHCFG | SCTLR_MTCFG | SCTLR_MEMATTR_WB)
#define SCTLR_CACHE_NC		(SCTLR_SHCFG | \
			SCTLR_MTCFG | SCTLR_MEMATTR_NC)
#define SCTLR_CACHE_NGNRE	(SCTLR_SHCFG | \
			SCTLR_MTCFG | SCTLR_MEMATTR_NGNRE)

#define SCTLR_CFCFG			(1 << 7)
#define SCTLR_CFIE			(1 << 6)
#define SCTLR_CFRE			(1 << 5)
#define SCTLR_E				(1 << 4)
#define SCTLR_AFED			(1 << 3)
#define SCTLR_M				(1 << 0)
#define SCTLR_EAE_SBOP			(SCTLR_AFED)

#define RESUME_RETRY			(0 << 0)
#define RESUME_TERMINATE		(1 << 0)

#define TTBCR_TG0_4K			(0 << 14)
#define TTBCR_TG0_64K			(3 << 14)

#define TTBCR_SH0_SHIFT			12
#define TTBCR_SH0_MASK			0x3
#define TTBCR_SH_NS			0
#define TTBCR_SH_OS			2
#define TTBCR_SH_IS			3
#define TTBCR_ORGN0_SHIFT		10
#define TTBCR_IRGN0_SHIFT		8
#define TTBCR_RGN_MASK			0x3
#define TTBCR_RGN_NC			0
#define TTBCR_RGN_WBWA			1
#define TTBCR_RGN_WT			2
#define TTBCR_RGN_WB			3
#define TTBCR_T1SZ_SHIFT		16
#define TTBCR_T0SZ_SHIFT		0
#define TTBCR_SZ_MASK			0xf

#define MAIR_ATTR_SHIFT(n)		((n) << 3)
#define MAIR_ATTR_MASK			0xff
#define MAIR_ATTR_DEVICE		0x04
#define MAIR_ATTR_NC			0x44
#define MAIR_ATTR_WBRWA			0xff
#define MAIR_ATTR_IDX_NC		0
#define MAIR_ATTR_IDX_CACHE		1
#define MAIR_ATTR_IDX_DEV		2

#define FSR_MULTI		(1 << 31)
#define FSR_EF			(1 << 4)
#define FSR_PF			(1 << 3)
#define FSR_AFF			(1 << 2)
#define FSR_TF			(1 << 1)
#define FSR_IGN			(FSR_AFF)
#define FSR_FAULT		(FSR_MULTI | FSR_EF | FSR_PF | FSR_TF | FSR_IGN)

#define FSYNR0_ASID(n)			(0xff & ((n) >> 24))
#define FSYNR0_VMID(n)			(0xff & ((n) >> 16))
#define FSYNR0_WNR			(1 << 4)
#define FSYNR0_SS			(1 << 2)
#define FSYNR0_CF			(1 << 0)

#define SMMU_FEAT_COHERENT_WALK		(1 << 0)
#define SMMU_FEAT_STREAM_MATCH		(1 << 1)
#define SMMU_FEAT_TRANS_S1		(1 << 2)
#define SMMU_FEAT_TRANS_S2		(1 << 3)
#define SMMU_FEAT_TRANS_NESTED		(1 << 4)

struct hisi_smmu_master {
	struct device_node		*of_node;
	struct iommu_domain		*domain;

	/*
	 * The following is specific to the master's position in the
	 * SMMU chain.
	 */
	struct rb_node			node;
	int				num_streamids;
	u16				streamids[MAX_MASTER_STREAMIDS];
};

struct hisi_smmu_device {
	struct device			*dev;

	void __iomem			*s1cbt;
	void __iomem			*s2cbt;
	void __iomem			*base;
	unsigned long			size;
	unsigned long			pagesize;

	u8				cb_mtcfg[SMMU_MAX_CBS];
	u32				features;
	int				version;

	u32				num_context_banks;
	DECLARE_BITMAP(context_map, SMMU_MAX_CBS);

	u32				num_mapping_groups;

	unsigned long			input_size;
	unsigned long			s1_output_size;
	unsigned long			s2_output_size;

	u32				num_global_irqs;
	u32				num_context_irqs;
	unsigned int			*irqs;

	struct list_head		list;
	struct rb_root			masters;
};

struct hisi_smmu_cfg {
	struct hisi_smmu_device		*smmu;
	u8				cbndx;
	u32				cbar;
	pgd_t				*pgd;
};

#define SMMU_CB_SID(cfg)	(((u16)SMMU_OS_VMID << 8) | ((cfg)->cbndx))

enum arm_smmu_domain_stage {
	ARM_SMMU_DOMAIN_S1 = 0,
	ARM_SMMU_DOMAIN_S2,
	ARM_SMMU_DOMAIN_NESTED,
};

struct hisi_smmu_domain {
	/*
	 * A domain can span across multiple, chained SMMUs and requires
	 * all devices within the domain to follow the same translation
	 * path.
	 */
	struct hisi_smmu_device		*smmu;
	struct hisi_smmu_cfg		root_cfg;
	enum arm_smmu_domain_stage	stage;
	spinlock_t			lock;
	int				num_of_masters;
	struct mutex			init_mutex; /* Protects smmu pointer */
	struct iommu_domain		domain;
};

static DEFINE_SPINLOCK(hisi_smmu_devices_lock);
static LIST_HEAD(hisi_smmu_devices);
static u32 hisi_bypass_vmid = 0xff;

static struct hisi_smmu_domain *to_smmu_domain(struct iommu_domain *dom)
{
	return container_of(dom, struct hisi_smmu_domain, domain);
}

static struct hisi_smmu_master *find_smmu_master(struct hisi_smmu_device *smmu,
						struct device_node *dev_node)
{
	struct rb_node *node = smmu->masters.rb_node;

	while (node) {
		struct hisi_smmu_master *master;
		master = container_of(node, struct hisi_smmu_master, node);

		if (dev_node < master->of_node)
			node = node->rb_left;
		else if (dev_node > master->of_node)
			node = node->rb_right;
		else
			return master;
	}

	return NULL;
}

static int insert_smmu_master(struct hisi_smmu_device *smmu,
			      struct hisi_smmu_master *master)
{
	struct rb_node **new, *parent;

	new = &smmu->masters.rb_node;
	parent = NULL;
	while (*new) {
		struct hisi_smmu_master *this;
		this = container_of(*new, struct hisi_smmu_master, node);

		parent = *new;
		if (master->of_node < this->of_node)
			new = &((*new)->rb_left);
		else if (master->of_node > this->of_node)
			new = &((*new)->rb_right);
		else
			return -EEXIST;
	}

	rb_link_node(&master->node, parent, new);
	rb_insert_color(&master->node, &smmu->masters);
	return 0;
}

static int register_smmu_master(struct hisi_smmu_device *smmu,
				struct device *dev,
				struct of_phandle_args *masterspec)
{
	int i;
	struct hisi_smmu_master *master;

	master = find_smmu_master(smmu, masterspec->np);
	if (master) {
		dev_err(dev,
			"rejecting multiple registrations for master device %s\n",
			masterspec->np->name);
		return -EBUSY;
	}

	if (masterspec->args_count > MAX_MASTER_STREAMIDS) {
		dev_err(dev,
			"reached maximum number (%d) of stream IDs for master device %s\n",
			MAX_MASTER_STREAMIDS, masterspec->np->name);
		return -ENOSPC;
	}

	master = devm_kzalloc(dev, sizeof(*master), GFP_KERNEL);
	if (!master)
		return -ENOMEM;

	master->of_node		= masterspec->np;
	master->num_streamids	= masterspec->args_count;

	for (i = 0; i < master->num_streamids; ++i)
		master->streamids[i] = masterspec->args[i];

	return insert_smmu_master(smmu, master);
}

static int __hisi_smmu_alloc_bitmap(struct hisi_smmu_device *smmu, int idx)
{
	if (test_and_set_bit(idx, smmu->context_map))
		return -ENOSPC;

	return idx;
}

static void __hisi_smmu_free_bitmap(struct hisi_smmu_device *smmu, int idx)
{
	clear_bit(idx, smmu->context_map);
}

/* Wait for any pending TLB invalidations to complete */
static void hisi_smmu_tlb_sync(struct hisi_smmu_device *smmu)
{
	int count = 0;
	void __iomem *gr0_base = SMMU_GR0(smmu);

	writel_relaxed(0, gr0_base + SMMU_TLBGSYNC);
	while (readl_relaxed(gr0_base + SMMU_TLBGSTATUS)
	       & sTLBGSTATUS_GSACTIVE) {
		cpu_relax();
		if (++count == TLB_LOOP_TIMEOUT) {
			dev_err_ratelimited(smmu->dev,
			"TLB sync timed out -- SMMU may be deadlocked\n");
			return;
		}
		udelay(1);
	}
}

static void hisi_smmu_tlb_inv_context(struct hisi_smmu_cfg *cfg)
{
	struct hisi_smmu_device *smmu = cfg->smmu;

	writel_relaxed(SMMU_CB_SID(cfg), SMMU_GR0(smmu) + SMMU_CXTISID);

	hisi_smmu_tlb_sync(smmu);
}

static irqreturn_t hisi_smmu_context_fault(int irq, void *dev)
{
	int i, flags, ret = IRQ_NONE, num_unhandled = 0;
	u32 fsr, far, fsynr, resume;
	unsigned long iova;
	struct iommu_domain *domain = NULL;
	struct hisi_smmu_device *smmu = dev;
	void __iomem *gr0_base = SMMU_GR0(smmu);

	for (i = 0; i < SMMU_CB_NUMIRPT; i++) {
		struct hisi_smmu_master *master, *n;

		fsynr = readl_relaxed(gr0_base + SMMU_RINT_CB_FSYNR(i));
		if (!(fsynr & FSYNR0_CF))
			continue;

		if (FSYNR0_VMID(fsynr) != SMMU_OS_VMID)
			continue;

		rbtree_postorder_for_each_entry_safe(master, n, &smmu->masters, node)
			if (master->streamids[0] == FSYNR0_ASID(fsynr)) {
				domain = master->domain;
				break;
			}

		fsr = readl_relaxed(gr0_base + SMMU_RINT_CB_FSR(i));
		if (fsr & FSR_IGN)
			dev_err_ratelimited(smmu->dev,
					    "Unexpected context fault (fsr 0x%u)\n",
					    fsr);

		flags = fsynr & FSYNR0_WNR ? IOMMU_FAULT_WRITE : IOMMU_FAULT_READ;

		far = readl_relaxed(gr0_base + SMMU_HIS_CB_FAR_LOW(i));
		iova = far;
#ifdef CONFIG_64BIT
		far = readl_relaxed(gr0_base + SMMU_HIS_CB_FAR_HIGH(i));
		iova |= ((unsigned long)far << 32);
#endif

		if (domain && !report_iommu_fault(domain, smmu->dev, iova, flags)) {
			ret = IRQ_HANDLED;
			resume = RESUME_RETRY;
		} else {
			dev_err_ratelimited(smmu->dev,
			    "Unhandled context fault: iova=0x%08lx, fsynr=0x%x, cb=%d\n",
			    iova, fsynr, FSYNR0_ASID(fsynr));
			num_unhandled++;
			resume = RESUME_TERMINATE;
		}

		/* Clear the faulting FSR */
		writel(fsr, gr0_base + SMMU_RINT_CB_FSR(i));

		/* Retry or terminate any stalled transactions */
		if (fsynr & FSYNR0_SS)
			writel_relaxed(resume, gr0_base + SMMU_CTRL_CB_RESUME(i));
	}

	/*
	 * If any fault unhandled, treat IRQ_NONE, although some maybe handled.
	 */
	if (num_unhandled)
		ret = IRQ_NONE;

	return ret;
}

static irqreturn_t hisi_smmu_global_fault(int irq, void *dev)
{
	u32 gfsr, gfsynr0;
	struct hisi_smmu_device *smmu = dev;
	void __iomem *gr0_base = SMMU_GR0(smmu);

	gfsr = readl_relaxed(gr0_base + SMMU_RINT_GFSR);
	if (!gfsr)
		return IRQ_NONE;

	gfsynr0 = readl_relaxed(gr0_base + SMMU_RINT_GFSYNR);

	dev_err_ratelimited(smmu->dev,
		"Unexpected global fault, this could be serious\n");
	dev_err_ratelimited(smmu->dev,
		"\tGFSR 0x%08x, GFSYNR0 0x%08x\n", gfsr, gfsynr0);

	writel(gfsr, gr0_base + SMMU_RINT_GFSR);
	return IRQ_HANDLED;
}

static void hisi_smmu_flush_pgtable(struct hisi_smmu_device *smmu, void *addr,
				   size_t size)
{
	unsigned long offset = (unsigned long)addr & ~PAGE_MASK;


	/* Ensure new page tables are visible to the hardware walker */
	if (smmu->features & SMMU_FEAT_COHERENT_WALK) {
		dsb(ishst);
	} else {
		/*
		 * If the SMMU can't walk tables in the CPU caches, treat them
		 * like non-coherent DMA since we need to flush the new entries
		 * all the way out to memory. There's no possibility of
		 * recursion here as the SMMU table walker will not be wired
		 * through another SMMU.
		 */
		dma_map_page(smmu->dev, virt_to_page(addr), offset, size,
				DMA_TO_DEVICE);
	}
}

static void hisi_smmu_init_context_bank(struct hisi_smmu_domain *smmu_domain)
{
	u32 reg;
	struct hisi_smmu_cfg *root_cfg = &smmu_domain->root_cfg;
	struct hisi_smmu_device *smmu = root_cfg->smmu;
	void __iomem *cb_base, *gr0_base;

	gr0_base = SMMU_GR0(smmu);
	cb_base = SMMU_CB_BASE(smmu) + SMMU_CB(smmu, root_cfg->cbndx);

	/* TTBR0 */
	hisi_smmu_flush_pgtable(smmu, root_cfg->pgd,
			       PTRS_PER_PGD * sizeof(pgd_t));
	reg = __pa(root_cfg->pgd);
	writel_relaxed(reg, cb_base + SMMU_S1_TTBR0_L);
	reg = (phys_addr_t)__pa(root_cfg->pgd) >> 32;
	writel_relaxed(reg, cb_base + SMMU_S1_TTBR0_H);

	/*
	 * TTBCR
	 * We use long descriptor, with inner-shareable WBWA tables in TTBR0.
	 */
	if (PAGE_SIZE == SZ_4K)
		reg = TTBCR_TG0_4K;
	else
		reg = TTBCR_TG0_64K;

	reg |= (64 - smmu->s1_output_size) << TTBCR_T0SZ_SHIFT;

	reg |= (TTBCR_SH_IS << TTBCR_SH0_SHIFT) |
	       (TTBCR_RGN_WBWA << TTBCR_ORGN0_SHIFT) |
	       (TTBCR_RGN_WBWA << TTBCR_IRGN0_SHIFT);
	writel_relaxed(reg, cb_base + SMMU_S1_TTBCR);

	reg = (MAIR_ATTR_NC << MAIR_ATTR_SHIFT(MAIR_ATTR_IDX_NC)) |
	      (MAIR_ATTR_WBRWA << MAIR_ATTR_SHIFT(MAIR_ATTR_IDX_CACHE)) |
	      (MAIR_ATTR_DEVICE << MAIR_ATTR_SHIFT(MAIR_ATTR_IDX_DEV));
	writel_relaxed(reg, cb_base + SMMU_S1_MAIR0);

	/* SCTLR */
	reg = SCTLR_CFCFG | SCTLR_CFIE | SCTLR_CFRE | SCTLR_M | SCTLR_EAE_SBOP;
#ifdef __BIG_ENDIAN
	reg |= SCTLR_E;
#endif
	writel_relaxed(reg, cb_base + SMMU_S1_SCTLR);
}

static int hisi_smmu_init_domain_context(struct iommu_domain *domain,
					struct device *dev)
{
	int ret;
	struct hisi_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct hisi_smmu_cfg *root_cfg = &smmu_domain->root_cfg;
	struct hisi_smmu_device *smmu;
	struct hisi_smmu_master *master;

	smmu = dev->archdata.iommu;

	master = find_smmu_master(smmu, dev->of_node);
	if (!master) {
		dev_err(dev, "unable to find root SMMU for device\n");
		return -ENODEV;
	}

	if (smmu->features & SMMU_FEAT_TRANS_NESTED) {
		/*
		 * We will likely want to change this if/when KVM gets
		 * involved.
		 */
		root_cfg->cbar = CBAR_TYPE_S1_TRANS_S2_BYPASS;
	} else {
		root_cfg->cbar = CBAR_TYPE_S1_TRANS_S2_BYPASS;
	}

	ret = __hisi_smmu_alloc_bitmap(smmu, master->streamids[0]);
	if (IS_ERR_VALUE(ret))
		return ret;

	root_cfg->cbndx = ret;

	root_cfg->smmu = smmu;
	hisi_smmu_init_context_bank(smmu_domain);
	return ret;
}

static void hisi_smmu_destroy_domain_context(struct iommu_domain *domain)
{
	struct hisi_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct hisi_smmu_cfg *root_cfg = &smmu_domain->root_cfg;
	struct hisi_smmu_device *smmu = root_cfg->smmu;
	void __iomem *cb_base;

	if (!smmu)
		return;

	/* Disable the context bank and nuke the TLB before freeing it. */
	cb_base = SMMU_CB_BASE(smmu) + SMMU_CB(smmu, root_cfg->cbndx);
	writel_relaxed(0, cb_base + SMMU_S1_SCTLR);
	hisi_smmu_tlb_inv_context(root_cfg);

	__hisi_smmu_free_bitmap(smmu, root_cfg->cbndx);
}

static struct iommu_domain *hisi_smmu_domain_alloc(unsigned type)
{
	struct hisi_smmu_domain *smmu_domain;
	pgd_t *pgd;

	if (type != IOMMU_DOMAIN_UNMANAGED)
		return NULL;
	/*
	 * Allocate the domain and initialise some of its data structures.
	 * We can't really do anything meaningful until we've added a
	 * master.
	 */
	smmu_domain = kzalloc(sizeof(*smmu_domain), GFP_KERNEL);
	if (!smmu_domain)
		return NULL;

	pgd = kzalloc(PTRS_PER_PGD * sizeof(pgd_t), GFP_KERNEL);
	if (!pgd)
		goto out_free_domain;
	smmu_domain->root_cfg.pgd = pgd;
	smmu_domain->stage = ARM_SMMU_DOMAIN_S1;

	mutex_init(&smmu_domain->init_mutex);
	spin_lock_init(&smmu_domain->lock);
	return &smmu_domain->domain;

out_free_domain:
	kfree(smmu_domain);
	return NULL;
}

static void hisi_smmu_free_ptes(pmd_t *pmd)
{
	pgtable_t table = pmd_pgtable(*pmd);
	pgtable_page_dtor(table);
	__free_page(table);
}

static void hisi_smmu_free_pmds(pud_t *pud)
{
	int i;
	pmd_t *pmd, *pmd_base = pmd_offset(pud, 0);

	pmd = pmd_base;
	for (i = 0; i < PTRS_PER_PMD; ++i) {
		if (pmd_none(*pmd))
			continue;

		hisi_smmu_free_ptes(pmd);
		pmd++;
	}

	pmd_free(NULL, pmd_base);
}

static void hisi_smmu_free_puds(pgd_t *pgd)
{
	int i;
	pud_t *pud, *pud_base = pud_offset(pgd, 0);

	pud = pud_base;
	for (i = 0; i < PTRS_PER_PUD; ++i) {
		if (pud_none(*pud))
			continue;

		hisi_smmu_free_pmds(pud);
		pud++;
	}

	pud_free(NULL, pud_base);
}

static void hisi_smmu_free_pgtables(struct hisi_smmu_domain *smmu_domain)
{
	int i;
	struct hisi_smmu_cfg *root_cfg = &smmu_domain->root_cfg;
	pgd_t *pgd, *pgd_base = root_cfg->pgd;

	/*
	 * Recursively free the page tables for this domain. We don't
	 * care about speculative TLB filling, because the TLB will be
	 * nuked next time this context bank is re-allocated and no devices
	 * currently map to these tables.
	 */
	pgd = pgd_base;
	for (i = 0; i < PTRS_PER_PGD; ++i) {
		if (pgd_none(*pgd))
			continue;
		hisi_smmu_free_puds(pgd);
		pgd++;
	}

	kfree(pgd_base);
}

static void hisi_smmu_domain_free(struct iommu_domain *domain)
{
	struct hisi_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (smmu_domain->num_of_masters)
		dev_err(smmu_domain->smmu->dev, "destroy domain with active dev!\n");

	/*
	 * Free the domain resources. We assume that all devices have
	 * already been detached.
	 */
	hisi_smmu_destroy_domain_context(domain);
	hisi_smmu_free_pgtables(smmu_domain);
	kfree(smmu_domain);
}

static int hisi_smmu_domain_add_master(struct hisi_smmu_domain *smmu_domain,
				      struct hisi_smmu_master *master)
{
	unsigned long flags;

	spin_lock_irqsave(&smmu_domain->lock, flags);
	smmu_domain->num_of_masters++;
	spin_unlock_irqrestore(&smmu_domain->lock, flags);

	return 0;
}

static void hisi_smmu_domain_remove_master(struct hisi_smmu_domain *smmu_domain,
					  struct hisi_smmu_master *master)
{
	unsigned long flags;

	spin_lock_irqsave(&smmu_domain->lock, flags);
	smmu_domain->num_of_masters--;
	spin_unlock_irqrestore(&smmu_domain->lock, flags);

	master->domain = NULL;
}

static int hisi_smmu_attach_dev(struct iommu_domain *domain, struct device *dev)
{
	int ret = -EINVAL;
	struct hisi_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct hisi_smmu_device *device_smmu = dev->archdata.iommu;
	struct hisi_smmu_master *master;
	unsigned long flags;

	if (!device_smmu) {
		dev_err(dev, "cannot attach to SMMU, is it on the same bus?\n");
		return -ENXIO;
	}

	/*
	 * Sanity check the domain. We don't currently support domains
	 * that cross between different SMMU chains.
	 */
	spin_lock_irqsave(&smmu_domain->lock, flags);
	if (!smmu_domain->smmu) {
		/* Now that we have a master, we can finalise the domain */
		ret = hisi_smmu_init_domain_context(domain, dev);
		if (IS_ERR_VALUE(ret))
			goto err_unlock;

		smmu_domain->smmu = device_smmu;
	} else if (smmu_domain->smmu != device_smmu) {
		dev_err(dev,
			"cannot attach to SMMU %s whilst already attached to domain on SMMU %s\n",
			dev_name(smmu_domain->smmu->dev),
			dev_name(device_smmu->dev));
		goto err_unlock;
	}
	spin_unlock_irqrestore(&smmu_domain->lock, flags);

	master = find_smmu_master(smmu_domain->smmu, dev->of_node);
	if (!master)
		return -ENODEV;

	if (SMMU_CB_SID(&smmu_domain->root_cfg) != master->streamids[0]) {
		dev_err(dev,
			"cannot attach %s to SMMU %s, sid diff from attached\n",
			dev_name(dev),
			dev_name(smmu_domain->smmu->dev));
		return -ENODEV;
	}

	master->domain = domain;

	return hisi_smmu_domain_add_master(smmu_domain, master);

err_unlock:
	spin_unlock_irqrestore(&smmu_domain->lock, flags);
	return ret;
}

static void hisi_smmu_detach_dev(struct iommu_domain *domain, struct device *dev)
{
	struct hisi_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct hisi_smmu_master *master;

	master = find_smmu_master(smmu_domain->smmu, dev->of_node);
	if (master)
		hisi_smmu_domain_remove_master(smmu_domain, master);
}

static bool hisi_smmu_pte_is_contiguous_range(unsigned long addr,
					     unsigned long end)
{
	return !(addr & ~SMMU_PTE_CONT_MASK) &&
		(addr + SMMU_PTE_CONT_SIZE <= end);
}

static int hisi_smmu_alloc_init_pte(struct hisi_smmu_device *smmu, pmd_t *pmd,
				   unsigned long addr, unsigned long end,
				   unsigned long pfn, int flags, int stage)
{
	pte_t *pte, *start;
	pteval_t pteval = SMMU_PTE_PAGE | SMMU_PTE_AF | SMMU_PTE_XN;

	if (pmd_none(*pmd)) {
		/* Allocate a new set of tables */
		pgtable_t table = alloc_page(GFP_ATOMIC|__GFP_ZERO);
		if (!table)
			return -ENOMEM;

		hisi_smmu_flush_pgtable(smmu, page_address(table), PAGE_SIZE);
		if (!pgtable_page_ctor(table)) {
			__free_page(table);
			return -ENOMEM;
		}
		pmd_populate(NULL, pmd, table);
		hisi_smmu_flush_pgtable(smmu, pmd, sizeof(*pmd));
	}

	if (stage == 1) {
		pteval |= SMMU_PTE_AP_UNPRIV | SMMU_PTE_nG;
		if (!(flags & IOMMU_WRITE) && (flags & IOMMU_READ))
			pteval |= SMMU_PTE_AP_RDONLY;

		if (flags & IOMMU_CACHE)
			pteval |= (MAIR_ATTR_IDX_CACHE <<
				   SMMU_PTE_ATTRINDX_SHIFT);
	} else {
		pteval |= SMMU_PTE_HAP_FAULT;
		if (flags & IOMMU_READ)
			pteval |= SMMU_PTE_HAP_READ;
		if (flags & IOMMU_WRITE)
			pteval |= SMMU_PTE_HAP_WRITE;
		if (flags & IOMMU_CACHE)
			pteval |= SMMU_PTE_MEMATTR_OIWB;
		else
			pteval |= SMMU_PTE_MEMATTR_NC;
	}

	/* If no access, create a faulting entry to avoid TLB fills */
	/*if (flags & IOMMU_EXEC)
		pteval &= ~SMMU_PTE_XN;
	else */if (!(flags & (IOMMU_READ | IOMMU_WRITE)))
		pteval &= ~SMMU_PTE_PAGE;

	pteval |= SMMU_PTE_SH;
	start = pmd_page_vaddr(*pmd) + pte_index(addr);
	pte = start;

	/*
	 * Install the page table entries. This is fairly complicated
	 * since we attempt to make use of the contiguous hint in the
	 * ptes where possible. The contiguous hint indicates a series
	 * of SMMU_PTE_CONT_ENTRIES ptes mapping a physically
	 * contiguous region with the following constraints:
	 *
	 *   - The region start is aligned to SMMU_PTE_CONT_SIZE
	 *   - Each pte in the region has the contiguous hint bit set
	 *
	 * This complicates unmapping (also handled by this code, when
	 * neither IOMMU_READ or IOMMU_WRITE are set) because it is
	 * possible, yet highly unlikely, that a client may unmap only
	 * part of a contiguous range. This requires clearing of the
	 * contiguous hint bits in the range before installing the new
	 * faulting entries.
	 *
	 * Note that re-mapping an address range without first unmapping
	 * it is not supported, so TLB invalidation is not required here
	 * and is instead performed at unmap and domain-init time.
	 */
	do {
		int i = 1;
		pteval &= ~SMMU_PTE_CONT;

		if (hisi_smmu_pte_is_contiguous_range(addr, end)) {
			i = SMMU_PTE_CONT_ENTRIES;
			pteval |= SMMU_PTE_CONT;
		} else if (pte_val(*pte) &
			   (SMMU_PTE_CONT | SMMU_PTE_PAGE)) {
			int j;
			pte_t *cont_start;
			unsigned long idx = pte_index(addr);

			idx &= ~(SMMU_PTE_CONT_ENTRIES - 1);
			cont_start = pmd_page_vaddr(*pmd) + idx;
			for (j = 0; j < SMMU_PTE_CONT_ENTRIES; ++j)
				pte_val(*(cont_start + j)) &= ~SMMU_PTE_CONT;

			hisi_smmu_flush_pgtable(smmu, cont_start,
					       sizeof(*pte) *
					       SMMU_PTE_CONT_ENTRIES);
		}

		do {
			*pte = pfn_pte(pfn, __pgprot(pteval));
		} while (pte++, pfn++, addr += PAGE_SIZE, --i);
	} while (addr != end);

	hisi_smmu_flush_pgtable(smmu, start, sizeof(*pte) * (pte - start));
	return 0;
}

static int hisi_smmu_alloc_init_pmd(struct hisi_smmu_device *smmu, pud_t *pud,
				   unsigned long addr, unsigned long end,
				   phys_addr_t phys, int flags, int stage)
{
	int ret;
	pmd_t *pmd;
	unsigned long next, pfn = __phys_to_pfn(phys);

#ifndef __PAGETABLE_PMD_FOLDED
	if (pud_none(*pud)) {
		pmd = (pmd_t *)get_zeroed_page(GFP_ATOMIC);
		if (!pmd)
			return -ENOMEM;

		hisi_smmu_flush_pgtable(smmu, pmd, PAGE_SIZE);
		pud_populate(NULL, pud, pmd);
		hisi_smmu_flush_pgtable(smmu, pud, sizeof(*pud));

		pmd += pmd_index(addr);
	} else
#endif
		pmd = pmd_offset(pud, addr);

	do {
		next = pmd_addr_end(addr, end);
		ret = hisi_smmu_alloc_init_pte(smmu, pmd, addr, end, pfn,
					      flags, stage);
		phys += next - addr;
	} while (pmd++, addr = next, addr < end);

	return ret;
}

static int hisi_smmu_alloc_init_pud(struct hisi_smmu_device *smmu, pgd_t *pgd,
				   unsigned long addr, unsigned long end,
				   phys_addr_t phys, int flags, int stage)
{
	int ret = 0;
	pud_t *pud;
	unsigned long next;

#ifndef __PAGETABLE_PUD_FOLDED
	if (pgd_none(*pgd)) {
		pud = (pud_t *)get_zeroed_page(GFP_ATOMIC);
		if (!pud)
			return -ENOMEM;

		hisi_smmu_flush_pgtable(smmu, pud, PAGE_SIZE);
		pgd_populate(NULL, pgd, pud);
		hisi_smmu_flush_pgtable(smmu, pgd, sizeof(*pgd));

		pud += pud_index(addr);
	} else
#endif
		pud = pud_offset(pgd, addr);

	do {
		next = pud_addr_end(addr, end);
		ret = hisi_smmu_alloc_init_pmd(smmu, pud, addr, next, phys,
					      flags, stage);
		phys += next - addr;
	} while (pud++, addr = next, addr < end);

	return ret;
}

static int hisi_smmu_handle_mapping(struct hisi_smmu_domain *smmu_domain,
				   unsigned long iova, phys_addr_t paddr,
				   size_t size, int flags)
{
	int ret, stage;
	unsigned long end;
	unsigned long output_size;
	phys_addr_t input_mask, output_mask;
	struct hisi_smmu_cfg *root_cfg = &smmu_domain->root_cfg;
	pgd_t *pgd = root_cfg->pgd;
	struct hisi_smmu_device *smmu = root_cfg->smmu;
	unsigned long irqflags;

	switch (root_cfg->cbar) {
	case CBAR_TYPE_S2_TRANS:
		stage = 2;
		output_size = smmu->s2_output_size;
		break;
	case CBAR_TYPE_S1_TRANS_S2_BYPASS:
		stage = 1;
		output_size = min(smmu->s1_output_size, smmu->s2_output_size);
		break;
	default:
		stage = 1;
		output_size = smmu->s1_output_size;
		break;
	}

	if (!pgd)
		return -EINVAL;

	if (size & ~PAGE_MASK)
		return -EINVAL;

	input_mask = (1ULL << smmu->input_size) - 1;
	if ((phys_addr_t)iova & ~input_mask)
		return -ERANGE;

	output_mask = (1ULL << output_size) - 1;
	if (paddr & ~output_mask)
		return -ERANGE;

	spin_lock_irqsave(&smmu_domain->lock, irqflags);
	pgd += pgd_index(iova);
	end = iova + size;
	do {
		unsigned long next = pgd_addr_end(iova, end);

		ret = hisi_smmu_alloc_init_pud(smmu, pgd, iova, next, paddr,
					      flags, stage);
		if (ret)
			goto out_unlock;

		paddr += next - iova;
		iova = next;
	} while (pgd++, iova != end);

out_unlock:
	spin_unlock_irqrestore(&smmu_domain->lock, irqflags);

	return ret;
}

static int hisi_smmu_map(struct iommu_domain *domain, unsigned long iova,
			phys_addr_t paddr, size_t size, int flags)
{
	struct hisi_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (!smmu_domain)
		return -ENODEV;

	return hisi_smmu_handle_mapping(smmu_domain, iova, paddr, size, flags);
}

static size_t hisi_smmu_unmap(struct iommu_domain *domain, unsigned long iova,
			     size_t size)
{
	int ret;
	struct hisi_smmu_domain *smmu_domain = to_smmu_domain(domain);

	ret = hisi_smmu_handle_mapping(smmu_domain, iova, 0, size, 0);
	hisi_smmu_tlb_inv_context(&smmu_domain->root_cfg);
	return ret ? ret : size;
}

static phys_addr_t hisi_smmu_iova_to_phys(struct iommu_domain *domain,
					 dma_addr_t iova)
{
	pgd_t *pgdp, pgd;
	pud_t pud;
	pmd_t pmd;
	pte_t pte;
	struct hisi_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct hisi_smmu_cfg *root_cfg = &smmu_domain->root_cfg;

	pgdp = root_cfg->pgd;
	if (!pgdp)
		return 0;

	pgd = *(pgdp + pgd_index(iova));
	if (pgd_none(pgd))
		return 0;

	pud = *pud_offset(&pgd, iova);
	if (pud_none(pud))
		return 0;

	pmd = *pmd_offset(&pud, iova);
	if (pmd_none(pmd))
		return 0;

	pte = *(pmd_page_vaddr(pmd) + pte_index(iova));
	if (pte_none(pte))
		return 0;

	return __pfn_to_phys(pte_pfn(pte)) | (iova & ~PAGE_MASK);
}

static bool hisi_smmu_capable(enum iommu_cap cap)
{
	switch (cap) {
	case IOMMU_CAP_CACHE_COHERENCY:
		/*
		 * Return true here as the SMMU can always send out coherent
		 * requests.
		 */
		return true;
	case IOMMU_CAP_INTR_REMAP:
		return true; /* MSIs are just memory writes */
	case IOMMU_CAP_NOEXEC:
		return true;
	default:
		return false;
	}
}

static int hisi_smmu_add_device(struct device *dev)
{
	struct hisi_smmu_device *smmu;
	struct hisi_smmu_master *master = NULL;
	struct iommu_group *group;
	int ret;

	if (dev->archdata.iommu) {
		dev_warn(dev, "IOMMU driver already assigned to device\n");
		return -EINVAL;
	}

	spin_lock(&hisi_smmu_devices_lock);
	list_for_each_entry(smmu, &hisi_smmu_devices, list) {
		master = find_smmu_master(smmu, dev->of_node);
		if (master)
			break;
	}
	spin_unlock(&hisi_smmu_devices_lock);

	if (!master)
		return -ENODEV;

	group = iommu_group_alloc();
	if (IS_ERR(group)) {
		dev_err(dev, "Failed to allocate IOMMU group\n");
		return PTR_ERR(group);
	}

	ret = iommu_group_add_device(group, dev);
	iommu_group_put(group);
	dev->archdata.iommu = smmu;

	return ret;
}

static void hisi_smmu_remove_device(struct device *dev)
{
	dev->archdata.iommu = NULL;
	iommu_group_remove_device(dev);
}

static int hisi_smmu_domain_get_attr(struct iommu_domain *domain,
				    enum iommu_attr attr, void *data)
{
	struct hisi_smmu_domain *smmu_domain = to_smmu_domain(domain);

	switch (attr) {
	case DOMAIN_ATTR_NESTING:
		*(int *)data = (smmu_domain->stage == ARM_SMMU_DOMAIN_NESTED);
		return 0;
	default:
		return -ENODEV;
	}
}

static int hisi_smmu_domain_set_attr(struct iommu_domain *domain,
				    enum iommu_attr attr, void *data)
{
	int ret = 0;
	struct hisi_smmu_domain *smmu_domain = to_smmu_domain(domain);

	mutex_lock(&smmu_domain->init_mutex);

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

out_unlock:
	mutex_unlock(&smmu_domain->init_mutex);
	return ret;
}

static struct iommu_ops hisi_smmu_ops = {
	.capable		= hisi_smmu_capable,
	.domain_alloc		= hisi_smmu_domain_alloc,
	.domain_free		= hisi_smmu_domain_free,
	.attach_dev		= hisi_smmu_attach_dev,
	.detach_dev		= hisi_smmu_detach_dev,
	.map			= hisi_smmu_map,
	.unmap			= hisi_smmu_unmap,
	.map_sg			= default_iommu_map_sg,
	.iova_to_phys		= hisi_smmu_iova_to_phys,
	.add_device		= hisi_smmu_add_device,
	.remove_device		= hisi_smmu_remove_device,
	.domain_get_attr	= hisi_smmu_domain_get_attr,
	.domain_set_attr	= hisi_smmu_domain_set_attr,
	.pgsize_bitmap		= (SECTION_SIZE |
				SMMU_PTE_CONT_SIZE |
				PAGE_SIZE),
};

static int hisi_smmu_device_reset(struct hisi_smmu_device *smmu)
{
	void __iomem *gr0_base = SMMU_GR0(smmu);
	void __iomem *cb_base;
	struct page	 *cbt_page;
	int i = 0;
	u32 reg;

	/* Clear Global FSR */
	reg = readl_relaxed(gr0_base + SMMU_RINT_GFSR);
	writel(reg, gr0_base + SMMU_RINT_GFSR);

	/* unmask all global interrupt */
	writel_relaxed(0, gr0_base + SMMU_CFG_GFIM);

	reg  = CFG_CBF_S1_ORGN_WA | CFG_CBF_S1_IRGN_WA | CFG_CBF_S1_SHCFG;
	reg |= CFG_CBF_S2_ORGN_WA | CFG_CBF_S2_IRGN_WA | CFG_CBF_S2_SHCFG;
	writel_relaxed(reg, gr0_base + SMMU_CFG_CBF);

	/* stage 2 context bank table */
	reg = readl_relaxed(gr0_base + SMMU_CFG_S2CTBAR);
	if (!reg) {
		cbt_page = alloc_pages(GFP_DMA32, get_order(SMMU_S2CBT_SIZE));
		if (!cbt_page) {
			pr_err("Failed to allocate SnCB table\n");
			return -ENOMEM;
		}

		reg = (u32)(page_to_phys(cbt_page) >> SMMU_S2CBT_SHIFT);
		writel_relaxed(reg, gr0_base + SMMU_CFG_S2CTBAR);
		smmu->s2cbt = page_address(cbt_page);

		for (i = 0; i < SMMU_MAX_CBS; i++) {
			writel_relaxed(0, smmu->s2cbt + SMMU_CB_S1CTBAR(i));
			writel_relaxed(S2CR_TYPE_BYPASS, smmu->s2cbt + SMMU_CB_S2CR(i));
		}

		/* Invalidate all TLB, just in case */
		writel_relaxed(0, gr0_base + SMMU_TLBIALL);
		hisi_smmu_tlb_sync(smmu);
	} else {
		smmu->s2cbt = ioremap_cache(
			(phys_addr_t)reg << SMMU_S2CBT_SHIFT, SMMU_S2CBT_SIZE);
	}

	/* stage 1 context bank table */
	cbt_page = alloc_pages(GFP_DMA32, get_order(SMMU_S1CBT_SIZE));
	if (!cbt_page) {
		pr_err("Failed to allocate SnCB table\n");
		return -ENOMEM;
	}

	reg = (u32)(page_to_phys(cbt_page) >> SMMU_S1CBT_SHIFT);
	writel_relaxed(reg, smmu->s2cbt + SMMU_CB_S1CTBAR(SMMU_OS_VMID));
	smmu->s1cbt = page_address(cbt_page);

	/* Make sure all context banks are disabled */
	for (i = 0; i < smmu->num_context_banks; i++) {
		cb_base = SMMU_CB_BASE(smmu) + SMMU_CB(smmu, i);

		switch (smmu->cb_mtcfg[i]) {
		case 1:
			reg = SCTLR_CACHE_WBRAWA;
			break;
		case 2:
			reg = SCTLR_CACHE_NC;
			break;
		case 3:
			reg = SCTLR_CACHE_NGNRE;
			break;
		default:
			reg = 0;
			break;
		}

		writel_relaxed(reg, cb_base + SMMU_S1_SCTLR);
	}

	/* Clear CB_FSR  */
	for (i = 0; i < SMMU_CB_NUMIRPT; i++) {
		writel_relaxed(FSR_FAULT, gr0_base + SMMU_RINT_CB_FSR(i));
	}

	/*
	 * Use the weakest attribute, so no impact stage 1 output attribute.
	 */
	reg = CBAR_TYPE_S1_TRANS_S2_BYPASS |
		(CBAR_S1_BPSHCFG_NSH << CBAR_S1_BPSHCFG_SHIFT) |
		(CBAR_S1_MEMATTR_WB << CBAR_S1_MEMATTR_SHIFT);
	writel_relaxed(reg, smmu->s2cbt + SMMU_CB_CBAR(SMMU_OS_VMID));

	/* Mark S2CR as translation */
	reg = S2CR_TYPE_TRANS | S2CR_MTSH_WEAKEST;
	writel_relaxed(reg, smmu->s2cbt + SMMU_CB_S2CR(SMMU_OS_VMID));

	/* Bypass need use another S2CR */
	reg = S2CR_TYPE_BYPASS;
	writel_relaxed(reg, smmu->s2cbt + SMMU_CB_S2CR(hisi_bypass_vmid));

	/* Invalidate the TLB, just in case */
	writel_relaxed(SMMU_OS_VMID, gr0_base + SMMU_TLBIVMID);
	hisi_smmu_tlb_sync(smmu);

	writel_relaxed(sACR_WC_EN, gr0_base + SMMU_CTRL_ACR);

	/* Enable fault reporting */
	reg  = (sCR0_GFRE | sCR0_GFIE | sCR0_GCFGFRE | sCR0_GCFGFIE);
	reg &= ~sCR0_CLIENTPD;

	writel_relaxed(reg, gr0_base + SMMU_CTRL_CR0);
	dsb(sy);

	return 0;
}

static int hisi_smmu_id_size_to_bits(unsigned long size)
{
	int i;

	for (i = 7; i <= 0; i--)
		if ((size >> i) & 0x1)
			break;

	return 32 + 4 * (i + 1);
}

static int hisi_smmu_device_cfg_probe(struct hisi_smmu_device *smmu)
{
	unsigned long size;
	void __iomem *gr0_base = SMMU_GR0(smmu);
	u32 id;

	dev_notice(smmu->dev, "probing hardware configuration...\n");

	smmu->version = 1;

	/* ID0 */
	id = readl_relaxed(gr0_base + SMMU_IDR0);
#ifndef CONFIG_64BIT
	if (((id >> ID0_PTFS_SHIFT) & ID0_PTFS_MASK) == ID0_PTFS_V8_ONLY) {
		dev_err(smmu->dev, "\tno v7 descriptor support!\n");
		return -ENODEV;
	}
#endif

	if (id & ID0_NTS) {
		smmu->features |= SMMU_FEAT_TRANS_NESTED;
		smmu->features |= SMMU_FEAT_TRANS_S1;
		smmu->features |= SMMU_FEAT_TRANS_S2;
		dev_notice(smmu->dev, "\tnested translation\n");
	} else if (id & ID0_S1TS) {
		smmu->features |= SMMU_FEAT_TRANS_S1;
		dev_notice(smmu->dev, "\tstage 1 translation\n");
	}

	if (!(smmu->features & SMMU_FEAT_TRANS_S1)) {
		dev_err(smmu->dev, "\tstage 1 translation not support!\n");
		return -ENODEV;
	}

	if (id & ID0_CTTW) {
		smmu->features |= SMMU_FEAT_COHERENT_WALK;
		dev_notice(smmu->dev, "\tcoherent table walk\n");
	}

	smmu->num_context_banks = SMMU_MAX_CBS;

	/* ID2 */
	id = readl_relaxed(gr0_base + SMMU_IDR2);
	size = hisi_smmu_id_size_to_bits((id >> ID2_IAS_SHIFT) & ID2_IAS_MASK);

	smmu->input_size = min((unsigned long)VA_BITS, size);

	/* The stage-2 output mask is also applied for bypass */
	size = hisi_smmu_id_size_to_bits((id >> ID2_OAS_SHIFT) & ID2_OAS_MASK);
	smmu->s2_output_size = min((unsigned long)PHYS_MASK_SHIFT, size);

	/*
	 * What the page table walker can address actually depends on which
	 * descriptor format is in use, but since a) we don't know that yet,
	 * and b) it can vary per context bank, this will have to do...
	 */
	if (dma_set_mask_and_coherent(smmu->dev, DMA_BIT_MASK(size)))
		dev_warn(smmu->dev,
			 "failed to set DMA mask for table walker\n");

	/*
	 * Stage-1 output limited by stage-2 input size due to pgd
	 * allocation (PTRS_PER_PGD).
	 */
#ifdef CONFIG_64BIT
	smmu->s1_output_size = min(39UL, size);
#else
	smmu->s1_output_size = min(32UL, size);
#endif

	dev_notice(smmu->dev,
		   "\t%lu-bit VA, %lu-bit IPA, %lu-bit PA\n",
		   smmu->input_size, smmu->s1_output_size, smmu->s2_output_size);

	return 0;
}

static int hisi_smmu_device_dt_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct hisi_smmu_device *smmu;
	struct device *dev = &pdev->dev;
	struct rb_node *node;
	struct of_phandle_args masterspec;
	int num_irqs, i, err;
	const __be32 *prop;
	int len;

	smmu = devm_kzalloc(dev, sizeof(*smmu), GFP_KERNEL);
	if (!smmu) {
		dev_err(dev, "failed to allocate hisi_smmu_device\n");
		return -ENOMEM;
	}
	smmu->dev = dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	smmu->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(smmu->base))
		return PTR_ERR(smmu->base);
	smmu->size = resource_size(res);

	if (of_property_read_u32(dev->of_node, "#global-interrupts",
				 &smmu->num_global_irqs)) {
		dev_err(dev, "missing #global-interrupts property\n");
		return -ENODEV;
	}

	num_irqs = 0;
	while ((res = platform_get_resource(pdev, IORESOURCE_IRQ, num_irqs))) {
		num_irqs++;
		if (num_irqs > smmu->num_global_irqs)
			smmu->num_context_irqs++;
	}

	if (smmu->num_context_irqs != 1) {
		dev_err(dev, "found %d context interrupts but expected exact one\n",
			smmu->num_context_irqs);
		return -ENODEV;
	}

	smmu->irqs = devm_kzalloc(dev, sizeof(*smmu->irqs) * num_irqs,
				  GFP_KERNEL);
	if (!smmu->irqs) {
		dev_err(dev, "failed to allocate %d irqs\n", num_irqs);
		return -ENOMEM;
	}

	for (i = 0; i < num_irqs; ++i) {
		int irq = platform_get_irq(pdev, i);
		if (irq < 0) {
			dev_err(dev, "failed to get irq index %d\n", i);
			return -ENODEV;
		}
		smmu->irqs[i] = irq;
	}

	i = 0;
	smmu->masters = RB_ROOT;
	while (!of_parse_phandle_with_fixed_args(dev->of_node, "smmu-masters",
					   1, i,
					   &masterspec)) {
		err = register_smmu_master(smmu, dev, &masterspec);
		if (err) {
			dev_err(dev, "failed to add master %s\n",
				masterspec.np->name);
			goto out_put_masters;
		}

		i++;
	}
	dev_notice(dev, "registered %d master devices\n", i);

	/*
	 * some devices may not support bring cache attributes, but want
	 * specified cache attributes. Here list three common cases:
	 * 1, cahceable, WBRAWA
	 * 2, non-cacheable
	 * 3, device, nGnRE
	 */
	prop = (__be32 *)of_get_property(dev->of_node, "smmu-cb-memtype", &len);
	for (i = 0; prop && (i < (len / 4) - 1); i += 2) {
		int ret, cbidx;

		cbidx = of_read_number(&prop[i], 1);

		if (cbidx >= SMMU_MAX_CBS) {
			dev_err(dev, "invalid StreamID %d\n", cbidx);
			goto out_put_masters;
		}

		ret = __hisi_smmu_alloc_bitmap(smmu, cbidx);
		if (IS_ERR_VALUE(ret)) {
			dev_err(dev, "conflict StreamID %d\n", cbidx);
			goto out_put_masters;
		}

		smmu->cb_mtcfg[cbidx] = (u8)of_read_number(&prop[i + 1], 1);

		if (!smmu->cb_mtcfg[cbidx])
			smmu->cb_mtcfg[cbidx] = 0xff;
	}

	of_property_read_u32(dev->of_node, "smmu-bypass-vmid", &hisi_bypass_vmid);

	err = hisi_smmu_device_cfg_probe(smmu);
	if (err)
		goto out_put_masters;

	for (i = 0; i < smmu->num_global_irqs; ++i) {
		err = request_irq(smmu->irqs[i],
				  hisi_smmu_global_fault,
				  IRQF_SHARED,
				  "hisi-smmu global fault",
				  smmu);
		if (err) {
			dev_err(dev, "failed to request global IRQ %d (%u)\n",
				i, smmu->irqs[i]);
			goto out_free_irqs;
		}
	}

	/* exact one context fault interrupt */
	err = request_irq(smmu->irqs[i], hisi_smmu_context_fault, IRQF_SHARED,
			  "hisi-smmu-context-fault", smmu);
	if (err) {
		dev_err(dev, "failed to request context IRQ (%u)\n", smmu->irqs[i]);
		goto out_free_irqs;
	}

	INIT_LIST_HEAD(&smmu->list);
	spin_lock(&hisi_smmu_devices_lock);
	list_add(&smmu->list, &hisi_smmu_devices);
	spin_unlock(&hisi_smmu_devices_lock);

	err = hisi_smmu_device_reset(smmu);
	if (err)
		goto out_free_irqs;

	return 0;

out_free_irqs:
	while (i--)
		free_irq(smmu->irqs[i], smmu);

out_put_masters:
	for (node = rb_first(&smmu->masters); node; node = rb_next(node)) {
		struct hisi_smmu_master *master;
		master = container_of(node, struct hisi_smmu_master, node);
		of_node_put(master->of_node);
	}

	return err;
}

static int hisi_smmu_device_remove(struct platform_device *pdev)
{
	int i;
	struct device *dev = &pdev->dev;
	struct hisi_smmu_device *curr, *smmu = NULL;
	struct rb_node *node;

	spin_lock(&hisi_smmu_devices_lock);
	list_for_each_entry(curr, &hisi_smmu_devices, list) {
		if (curr->dev == dev) {
			smmu = curr;
			list_del(&smmu->list);
			break;
		}
	}
	spin_unlock(&hisi_smmu_devices_lock);

	if (!smmu)
		return -ENODEV;

	for (node = rb_first(&smmu->masters); node; node = rb_next(node)) {
		struct hisi_smmu_master *master;
		master = container_of(node, struct hisi_smmu_master, node);
		of_node_put(master->of_node);
	}

	for (i = 0; i < smmu->num_context_banks; i++) {
		if (smmu->cb_mtcfg[i])
			__hisi_smmu_free_bitmap(smmu, i);
	}

	if (!bitmap_empty(smmu->context_map, SMMU_MAX_CBS))
		dev_err(dev, "removing device with active domains!\n");

	/* global and context irq are registered as the same manner */
	for (i = 0; i < (smmu->num_global_irqs + smmu->num_context_irqs); ++i)
		free_irq(smmu->irqs[i], smmu);

	/* Here, we only free s1cbt. The s2cbt may be shared with hypervisor */
	free_pages((unsigned long)smmu->s1cbt, get_order(SMMU_S1CBT_SIZE));

	/* Turn the thing off */
	writel_relaxed(sCR0_CLIENTPD, SMMU_GR0(smmu) + SMMU_CTRL_CR0);
	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id hisi_smmu_of_match[] = {
	{ .compatible = "hisilicon,smmu-v1", },
	{ },
};
MODULE_DEVICE_TABLE(of, hisi_smmu_of_match);
#endif

static struct platform_driver hisi_smmu_driver = {
	.driver	= {
		.owner		= THIS_MODULE,
		.name		= "hisi-smmu",
		.of_match_table	= of_match_ptr(hisi_smmu_of_match),
	},
	.probe	= hisi_smmu_device_dt_probe,
	.remove	= hisi_smmu_device_remove,
};

static int __init hisi_smmu_init(void)
{
	int ret;

	ret = platform_driver_register(&hisi_smmu_driver);
	if (ret)
		return ret;

	if (!iommu_present(&platform_bus_type))
		bus_set_iommu(&platform_bus_type, &hisi_smmu_ops);

#ifdef CONFIG_ARM_AMBA
	if (!iommu_present(&amba_bustype))
		bus_set_iommu(&amba_bustype, &hisi_smmu_ops);
#endif

	return 0;
}

static void __exit hisi_smmu_exit(void)
{
	return platform_driver_unregister(&hisi_smmu_driver);
}

subsys_initcall(hisi_smmu_init);
module_exit(hisi_smmu_exit);

MODULE_DESCRIPTION("IOMMU API for hisilicon architected SMMU implementations");
MODULE_AUTHOR("Zhen Lei <thunder.leizhen@huawei.com>");
MODULE_LICENSE("GPL v2");

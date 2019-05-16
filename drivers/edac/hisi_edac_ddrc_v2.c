// SPDX-License-Identifier: GPL-2.0+
/* Copyright (c) 2016-2017 Hisilicon Limited. */
/*
 * EDAC Driver for Hisilicon DDR3/4 DRAM Controller.
 */
#include <linux/delay.h>
#include <linux/edac.h>
#include <linux/interrupt.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>

#include "edac_mc.h"

#define DDRC_LOG_ONLY_ERR(regval)	(SBRAM_SERR_MASK & regval ||  \
	(DDR_ECC_SERR_MASK & regval))

#define RASC_LOG_ONLY_ERR(regval)	((DRAM_MERR_MASK & regval) ||  \
	(ERAM_MERR_MASK & regval) || (DRAM_SERR_MASK & regval) ||  \
	(ERAM_SERR_MASK & regval) || (SP_DEV_ERR_MASK & regval) ||  \
	(PA_CORR_ERR_MASK & regval) ||  (DE_SUCS_MASK & regval) ||  \
	(HA_CORR_ERR_MASK & regval))

#define DDRC_LOG_OS_ERR(regval)		((SBRAM_MERR_MASK & regval) || \
	(DDR_ECC_MERR_MASK & regval))

#define RASC_LOG_OS_ERR(regval)		((PA_UNCORR_ERR_MASK & regval) || \
	(DE_FAIL_ERR_MASK & regval) || (HA_UNCORR_ERR_MASK & regval))

#define EDAC_MOD_STR			"edac_ddrc"
#define DDRC_DES			"DDRC"
#define SBRAM_DES			"SideBand RAM"
#define DRAM_DES			"Device Cnt SRAM"
#define ERAM_DES			"Error ADDR SRAM"
#define CA_PARITY_DES			"CA Parity"
#define PA_DES				"Patrol Process "
#define HA_CORR_DES			"HA Interface"

/*
 * trace_frqlimit - print frequency control interface
 * @interval: print interval
 */
#define trace_frqlimit(interval, X, args...) \
	do { \
		static unsigned long last; \
		if (time_after_eq(jiffies, last+(interval))) { \
			last = jiffies; \
			edac_printk(KERN_CRIT, EDAC_MOD_STR, X, ##args); \
		} \
	} while (0)

#define DDRC_RINT_ADDR			0x8284
#define DDRC_INTMSK			0x8280
#define DDRC_INTSTS_ADDR		0X8288
#define SBRAM_MERR_MASK			BIT(24)
#define SBRAM_SERR_MASK			BIT(20)
#define DIMM_PARITY_MASK		BIT(8)
#define DDR_ECC_MERR_MASK		BIT(4)
#define DDR_ECC_SERR_MASK		BIT(0)

#define RASC_INTMSK			0x5200
#define RASC_RINT_ADDR			0x5204
#define RASC_INTSTS_ADDR		0x5208
#define RASC_HIS_UNCORR_ADDR_L		0x5310
#define RASC_HIS_UNCORR_ADDR_H		0x5314
#define DRAM_MERR_MASK			BIT(23)
#define DRAM_SERR_MASK			BIT(22)
#define ERAM_MERR_MASK			BIT(21)
#define ERAM_SERR_MASK			BIT(20)
#define SP_DEV_ERR_MASK			BIT(12)
#define PA_UNCORR_ERR_MASK		BIT(9)
#define PA_CORR_ERR_MASK		BIT(8)
#define DE_FAIL_ERR_MASK		BIT(5)
#define DE_SUCS_MASK			BIT(4)
#define HA_UNCORR_ERR_MASK		BIT(1)
#define HA_CORR_ERR_MASK		BIT(0)

#define RASC_HIS_CORR_ADDR_L		0x5350
#define RASC_HIS_CORR_ADDR_H		0x5354
#define ERR_COUNT			5
#define DDRC_IRQ_NUM			2

struct hisi_edac_ddrc {
	struct device	*dev;
	unsigned int irq[DDRC_IRQ_NUM];
	void __iomem	*ddrc_base;
	struct mem_ctl_info	*mci;
	unsigned int channel_id;
};

/* used for edac module privte data */
struct edac_priv_data {
	void __iomem	*ddrc_base;
	unsigned int channel_id;
	unsigned int dimm_parity_mask;
};

static DEFINE_SPINLOCK(ddrc_maint_lock);

static const struct of_device_id ddrc_edac_of_match[] = {
	{ .compatible = "hisilicon,hip07-edac-ddrc" },
	{},
};

static void hisi_edac_mc_ce_handle(struct mem_ctl_info *mci, void __iomem *addr,
			unsigned int ddrc_insts, unsigned int rasc_insts)
{
	unsigned long page_frame_number, offset_in_page;
	unsigned long long paddr;
	u32 err_addr_low, err_addr_high;

	err_addr_low = readl(addr + RASC_HIS_CORR_ADDR_L);
	err_addr_high = readl(addr + RASC_HIS_CORR_ADDR_H);

	paddr = (((unsigned long)err_addr_high & 0x3f) << 32) | err_addr_low;
	offset_in_page = paddr & (PAGE_SIZE - 1);
	page_frame_number = paddr >> PAGE_SHIFT;

	if (DDR_ECC_SERR_MASK & ddrc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s single_bit ECC error\n",
				mci->mc_idx, DDRC_DES);

	if (SBRAM_SERR_MASK & ddrc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s single_bit ECC error\n",
				mci->mc_idx, SBRAM_DES);
	/* RASC Error */
	if (DRAM_MERR_MASK & rasc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s multi_bit ECC error\n",
				mci->mc_idx, DRAM_DES);

	if (DRAM_SERR_MASK & rasc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s single_bit ECC error\n",
				mci->mc_idx, DRAM_DES);
	if (ERAM_MERR_MASK & rasc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s multi_bit ECC error\n",
				mci->mc_idx, ERAM_DES);
	if (ERAM_SERR_MASK & rasc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s single_bit ECC error\n",
				mci->mc_idx, ERAM_DES);
	if (PA_CORR_ERR_MASK & rasc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s correctable error\n",
				mci->mc_idx, PA_DES);
	if (HA_CORR_ERR_MASK & rasc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s correctable error\n",
				mci->mc_idx, HA_CORR_DES);

	edac_mc_handle_error(HW_EVENT_ERR_CORRECTED,
		mci, 1, page_frame_number,
		offset_in_page, 0, -1, -1, -1,
		mci->ctl_name, "");
}

static void hisi_edac_mc_ue_handle(struct mem_ctl_info *mci,
	void __iomem *addr,
	unsigned int ddrc_insts,
	unsigned int rasc_insts)
{
	unsigned long page_frame_number, offset_in_page;
	unsigned long long paddr;
	u32 err_addr_low, err_addr_high;

	err_addr_low = readl(addr + RASC_HIS_UNCORR_ADDR_L);
	err_addr_high = readl(addr + RASC_HIS_UNCORR_ADDR_H);

	paddr = (((unsigned long)err_addr_high & 0x3f) << 32) | err_addr_low;

	offset_in_page = (unsigned long)(paddr & (PAGE_SIZE - 1));
	page_frame_number = (unsigned long)(paddr >> PAGE_SHIFT);

	if (DDR_ECC_MERR_MASK & ddrc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s multi_bit ECC error\n",
				mci->mc_idx, DDRC_DES);

	if (SBRAM_MERR_MASK & ddrc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s multi_bit ECC error\n",
				mci->mc_idx, SBRAM_DES);

	if (DIMM_PARITY_MASK & ddrc_insts)
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s error\n",
				mci->mc_idx, CA_PARITY_DES);

	/* RASC Error */
	if (PA_UNCORR_ERR_MASK & rasc_insts) {
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s Uncorrectable error\n",
				mci->mc_idx, PA_DES);
	}
	if (HA_UNCORR_ERR_MASK & rasc_insts) {
		trace_frqlimit(ERR_COUNT * HZ, "MC%d %s Uncorrectable error\n",
				mci->mc_idx, HA_CORR_DES);
	}

	edac_mc_handle_error(HW_EVENT_ERR_UNCORRECTED,
		mci, 1, page_frame_number,
		offset_in_page, 0, -1, -1, -1,
		mci->ctl_name, "");

	panic("EDAC trigger system reboot: %s\n", __func__);
}

irqreturn_t hisi_edac_mc_isr(int irq, void *dev_id)
{
	struct mem_ctl_info *mci = dev_id;
	struct edac_priv_data *priv = mci->pvt_info;
	void __iomem *res_addr;
	unsigned int ddrc_insts, rasc_insts;
	unsigned long flag = 0;

	/* Read the state of interrupt res */
	res_addr = priv->ddrc_base;

	spin_lock_irqsave(&ddrc_maint_lock, flag);

	/* read interruption */
	ddrc_insts = readl(res_addr + DDRC_INTSTS_ADDR);
	rasc_insts = readl(res_addr + RASC_INTSTS_ADDR);

	/* error log only */
	if (DDRC_LOG_ONLY_ERR(ddrc_insts) || RASC_LOG_ONLY_ERR(rasc_insts)) {
		trace_frqlimit(50*HZ,
			"channel_id:0x%x,ddrc_insts:0x%x,rasc_insts:0x%x\n",
			priv->channel_id, ddrc_insts, rasc_insts);

		hisi_edac_mc_ce_handle(mci, res_addr, ddrc_insts, rasc_insts);
	}

	/* error log and report os */
	if (DDRC_LOG_OS_ERR(ddrc_insts) || RASC_LOG_OS_ERR(rasc_insts)) {
		trace_frqlimit(50*HZ,
			"channel_id:0x%x,ddrc_insts:0x%x,rasc_insts:0x%x\n",
			priv->channel_id, ddrc_insts, rasc_insts);
		hisi_edac_mc_ue_handle(mci, res_addr, ddrc_insts, rasc_insts);
	}

	/* clear interruption */
	writel(rasc_insts, res_addr + RASC_RINT_ADDR);
	writel(ddrc_insts, res_addr + DDRC_RINT_ADDR);

	spin_unlock_irqrestore(&ddrc_maint_lock, flag);

	return IRQ_HANDLED;
}

static int hisi_edac_mc_add(struct hisi_edac_ddrc *edac)
{
	struct device_node *np, *edac_np;
	const char *ca_parity_str;
	struct mem_ctl_info *mci;
	struct edac_mc_layer layers[1];
	struct edac_priv_data *priv;
	unsigned int mc_index;
	int res, ret;

	if (!edac) {
		dev_err(edac->dev, "%s edac was NULL\n", __func__);
		return -ENOENT;
	}

	mc_index = edac->channel_id;

	layers[0].type = EDAC_MC_LAYER_CHANNEL;
	layers[0].size = 1;
	layers[0].is_virt_csrow = false;
	mci = edac_mc_alloc(mc_index, ARRAY_SIZE(layers), layers,
				sizeof(struct edac_priv_data));
	if (!mci)
		return -ENOMEM;

	/* priv is saved in mci as module's private data */
	priv = mci->pvt_info;

	priv->channel_id = edac->channel_id;
	priv->ddrc_base = edac->ddrc_base;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,lemon-1p");
	if (np) {
		edac_np = edac->dev->of_node;
		ret = of_property_read_string(edac_np, "ca-parity",
					      &ca_parity_str);
		if (!ret && !strcmp(ca_parity_str, "enable"))
			priv->dimm_parity_mask = 0;
		else
			priv->dimm_parity_mask = DIMM_PARITY_MASK;
	} else {
		priv->dimm_parity_mask = 0;
	}

	mci->pdev = edac->dev;
	mci->ctl_name = dev_name(edac->dev);
	mci->dev_name = dev_name(edac->dev);

	mci->mtype_cap = MEM_FLAG_RDDR | MEM_FLAG_RDDR2 | MEM_FLAG_RDDR3 |
			 MEM_FLAG_DDR | MEM_FLAG_DDR2 | MEM_FLAG_DDR3;
	mci->edac_ctl_cap = EDAC_FLAG_SECDED;
	mci->edac_cap = EDAC_FLAG_SECDED;
	mci->mod_name = EDAC_MOD_STR;
	mci->ctl_page_to_phys = NULL;
	mci->scrub_cap = SCRUB_FLAG_HW_SRC;
	mci->scrub_mode = SCRUB_UNKNOWN;

	/* add mci to edac frame */
	res = edac_mc_add_mc(mci);

	if (res < 0) {
		dev_err(edac->dev, "add mci fail,res = %d\n", res);
		edac_mc_free(mci);
		return -ENOENT;
	}

	edac->mci = mci;

	return 0;
}

static int hisi_ddrc_probe(struct platform_device *pdev)
{
	struct hisi_edac_ddrc *edac;
	struct edac_priv_data *priv;
	struct resource *edac_res;
	unsigned int count;
	static int channel_id;
	int rc;
	int i;

	edac = devm_kzalloc(&pdev->dev, sizeof(*edac), GFP_KERNEL);
	if (!edac)
		return -ENOMEM;

	edac->dev = &pdev->dev;
	platform_set_drvdata(pdev, edac);

	edac->channel_id = channel_id;

	/* use a static variable to record channel number */
	channel_id = channel_id + 1;
	edac_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!edac_res) {
		dev_err(&pdev->dev, "can not find this hi1610-edac resource\n");
		return -ENOENT;
	}

	edac->ddrc_base = devm_ioremap_resource(&pdev->dev, edac_res);
	if (IS_ERR(edac->ddrc_base)) {
		dev_err(&pdev->dev, "no resource address\n");
		return -ENOENT;
	}

	if (hisi_edac_mc_add(edac)) {
		dev_err(&pdev->dev, "hisi_edac_mc_add fail\n");
		return -ENOENT;
	}

	/* clear wrong interrupt and open mask */
	writel(0xffffff, edac->ddrc_base + RASC_RINT_ADDR);
	writel(0xffffffff, edac->ddrc_base + DDRC_RINT_ADDR);
	writel(0x3000, edac->ddrc_base + RASC_INTMSK);

	priv = (struct edac_priv_data *)edac->mci->pvt_info;

	writel(priv->dimm_parity_mask, edac->ddrc_base + DDRC_INTMSK);

	for (count = 0; count < DDRC_IRQ_NUM; count++) {
		edac->irq[count] = platform_get_irq(pdev, count);
		if (edac->irq[count] < 0) {
			dev_err(&pdev->dev, "No IRQ resource\n");
			rc = -EINVAL;
			goto out_err;
		}
		rc = devm_request_irq(&pdev->dev, edac->irq[count],
				      hisi_edac_mc_isr, 0, dev_name(&pdev->dev),
				      edac->mci);
		if (rc) {
			dev_err(&pdev->dev, "irq count:%d req failed\n", count);
			goto out_err;
		}
	}

	return 0;

out_err:
	for (i = 0; i < count; i++)
		devm_free_irq(&pdev->dev, edac->irq[count], edac->mci);

	if (edac->mci) {
		edac->mci = edac_mc_del_mc(edac->mci->pdev);
		if (edac->mci)
			edac_mc_free(edac->mci);
		edac->mci = NULL;
	}

	return rc;
}

static int hisi_ddrc_remove(struct platform_device *pdev)
{
	struct hisi_edac_ddrc *edac = platform_get_drvdata(pdev);

	if (edac->mci) {
		edac->mci = edac_mc_del_mc(edac->mci->pdev);
		if (edac->mci)
			edac_mc_free(edac->mci);
		edac->mci = NULL;
	}

	return 0;
}

MODULE_DEVICE_TABLE(of, ddrc_edac_of_match);

static struct platform_driver ddrc_edac_driver = {
	.probe = hisi_ddrc_probe,
	.remove = hisi_ddrc_remove,
	.driver = {
		.name = "hip07-edac-ddrc",
		.owner = THIS_MODULE,
		.of_match_table = ddrc_edac_of_match,
	},
};

module_platform_driver(ddrc_edac_driver);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("HISILICON DDR Controller v2 hw EDAC driver");
MODULE_AUTHOR("Peter Chen <luck.chen@huawei.com>");
MODULE_AUTHOR("Fengying Wang <fy.wang@huawei.com>");

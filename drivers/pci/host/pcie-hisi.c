/*
 * PCIe host controller driver for Hisilicon Hip05 SoCs
 *
 * Copyright (C) 2014 Hisilicon Co., Ltd. http://www.hisilicon.com
 *
 * Author: Zhou Wang <wangzhou1@hisilicon.com>
 *         Dacai Zhu <zhudacai@hisilicon.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of_pci.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/signal.h>
#include <linux/types.h>

#include "pcie-designware.h"

#define PCIE_SYS_CTRL20_REG                             (0x20)

#define PCIE_SUBCTRL_RESET_REQ_REG                      (0xA00)
#define PCIE0_2_SUBCTRL_RESET_REQ_REG(port_id) \
	(PCIE_SUBCTRL_RESET_REQ_REG + (port_id << 3))
#define PCIE3_SUBCTRL_RESET_REQ_REG                     (0xA68)

#define PCIE_SUBCTRL_DRESET_REQ_REG                     (0xA04)
#define PCIE0_2_SUBCTRL_DRESET_REQ_REG(port_id) \
	(PCIE_SUBCTRL_DRESET_REQ_REG + (port_id << 3))
#define PCIE3_SUBCTRL_DRESET_REQ_REG                    (0xA6C)

#define PCIE_SUBCTRL_CLKREQ_REG                         (0x2800)

#define PCIE_SUBCTRL_RESET_ST_REG                       (0x5A00)
#define PCIE0_2_SUBCTRL_RESET_ST_REG(port_id) \
	(PCIE_SUBCTRL_RESET_ST_REG + (port_id << 2))
#define PCIE3_SUBCTRL_RESET_ST_REG                      (0x5A34)

#define PCIE_SUBCTRL_SC_PCIE0_CLK_DIS_REG               (0x304)
#define PCIE_SUBCTRL_SC_PCIE0_2_CLK_DIS_REG(port_id) \
	(PCIE_SUBCTRL_SC_PCIE0_CLK_DIS_REG + port_id * 0x8)
#define PCIE_SUBCTRL_SC_PCIE3_CLK_DIS_REG               (0x324)

#define PCIE_SUBCTRL_SC_PCIE0_CLK_ST_REG                (0x5300)
#define PCIE_SUBCTRL_SC_PCIE0_2_CLK_ST_REG(port_id) \
	(PCIE_SUBCTRL_SC_PCIE0_CLK_ST_REG + port_id * 0x4)
#define PCIE_SUBCTRL_SC_PCIE3_CLK_ST_REG                (0x5310)

#define PCIE_SUBCTRL_SC_PCIE0_CLK_EN_REG                (0x300)
#define PCIE_SUBCTRL_SC_PCIE0_2_CLK_EN_REG(port_id) \
	(PCIE_SUBCTRL_SC_PCIE0_CLK_EN_REG + port_id * 0x8)
#define PCIE_SUBCTRL_SC_PCIE3_CLK_EN_REG                (0x320)

#define PCIE_PCS_LOCAL_RESET_ST                         (0x5A60)

#define PCIE_SUBCTRL_SYS_STATE4_REG                     (0x6818)

#define PCIE_CTRL_7_REG                                 (0x114)

#define PCIE_SLV_DBI_ENABLE                             BIT(0)

#define PCIE_SLV_DBI_MODE                               (0x0)
#define PCIE_SLV_SYSCTRL_MODE                           (0x1)
#define PCIE_SLV_CONTENT_MODE                           (0x2)

#define PCIE_LTSSM_LINKUP_STATE                         (0x11)
#define PCIE_LTSSM_STATE_MASK                           (0x3F)
#define PCIE_LTSSM_ENABLE_SHIFT                         BIT(11)
#define PCIE_PCS_LOCAL_RESET_REQ                        (0xAC0)
#define PCIE_PCS_RESET_REQ_REG                          (0xA60)
#define PCIE_PCS_RESET_REG_ST                           (0x5A30)
#define PCIE_PCS_LOCAL_DRESET_REQ                       (0xAC4)
#define PCIE_PCS_LOCAL_DRESET_ST                        (0x5A60)
#define PCIE_PCS_DRESET_REQ_REG                         (0xA64)
#define PCIE_M_PCS_IN15_CFG                             (0x74)
#define PCIE_M_PCS_IN13_CFG                             (0x34)
#define PCIE_PCS_SERDES_STATUS                          (0x8108)
#define PCIE_PCS_RXDETECTED                             (0xE4)
#define PCIE_MSI_CONTEXT_VALUE                          (0x1011000)
#define PCIE_MSI_TRANS_ENABLE                           (0x1ff0)

#define PCIE_ASSERT_RESET_ON                            (1)
#define PCIE_DEASSERT_RESET_ON                          (0)
#define PCIE_CLOCK_ON                                   (1)
#define PCIE_CLOCK_OFF                                  (0)

#define to_hisi_pcie(x)	container_of(x, struct hisi_pcie, pp)

struct hisi_pcie {
	void __iomem *subctrl_base;
	void __iomem *reg_base;
	void __iomem *phy_base;
	struct msi_controller *msi;
	u32 port_id;
	struct pcie_port pp;
};

static inline void hisi_pcie_subctrl_writel(struct hisi_pcie *pcie,
					    u32 val, u32 reg)
{
	writel(val, pcie->subctrl_base + reg);
}

static inline u32 hisi_pcie_subctrl_readl(struct hisi_pcie *pcie, u32 reg)
{
	return readl(pcie->subctrl_base + reg);
}

static inline void hisi_pcie_apb_writel(struct hisi_pcie *pcie,
					u32 val, u32 reg)
{
	writel(val, pcie->reg_base + reg);
}

static inline u32 hisi_pcie_apb_readl(struct hisi_pcie *pcie, u32 reg)
{
	return readl(pcie->reg_base + reg);
}

static inline void hisi_pcie_pcs_writel(struct hisi_pcie *pcie,
					u32 val, u32 reg)
{
	writel(val, pcie->phy_base + reg);
}

static inline u32 hisi_pcie_pcs_readl(struct hisi_pcie *pcie, u32 reg)
{
	return readl(pcie->phy_base + reg);
}

/* Change reg_base to indicate base of PCIe host configure registers,
 * RC cofigure space or vmid/asid context table
 */
static void hisi_pcie_apb_slave_mode(struct pcie_port *pp, u32 mode)
{
	u32 val;
	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);
	u32 bit_mask;
	u32 bit_shift;
	u32 port_id = hisi_pcie->port_id;
	u32 reg = PCIE_SUBCTRL_CLKREQ_REG + 0x100 * port_id;

	/* port0&3 use diff bit as port1&2 */
	if ((port_id > 0) && (port_id < 3)) {
		bit_mask = 0xc;
		bit_shift = 0x2;
	} else {
		bit_mask = 0x6;
		bit_shift = 0x1;
	}
	do {
		val = hisi_pcie_subctrl_readl(hisi_pcie, reg);
		val = (val & (~bit_mask)) | (mode << bit_shift);
		hisi_pcie_subctrl_writel(hisi_pcie, val, reg);
		val = hisi_pcie_subctrl_readl(hisi_pcie, reg);
		val = (val & bit_mask) >> bit_shift;
	} while (val != mode);
}

static int hisi_pcie_link_up(struct pcie_port *pp)
{
	u32 val;

	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);

	val = hisi_pcie_subctrl_readl(hisi_pcie, PCIE_SUBCTRL_SYS_STATE4_REG +
				      0x100 * hisi_pcie->port_id);

	return ((val & PCIE_LTSSM_STATE_MASK) == PCIE_LTSSM_LINKUP_STATE);
}

static void hisi_pcie_enable_ltssm(struct pcie_port *pp, bool on)
{
	u32 val;

	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);

	hisi_pcie_apb_slave_mode(pp, PCIE_SLV_SYSCTRL_MODE);

	val = hisi_pcie_apb_readl(hisi_pcie, PCIE_CTRL_7_REG);
	if (on)
		val |= (PCIE_LTSSM_ENABLE_SHIFT);
	else
		val &= ~(PCIE_LTSSM_ENABLE_SHIFT);
	hisi_pcie_apb_writel(hisi_pcie, val, PCIE_CTRL_7_REG);
	hisi_pcie_apb_slave_mode(pp, PCIE_SLV_DBI_MODE);
}

static void hisi_pcie_core_reset_ctrl(struct pcie_port *pp, bool reset_on)
{
	u32 reg_reset_ctrl;
	u32 reg_dereset_ctrl;
	u32 reg_reset_status;

	u32 reset_status;
	u32 reset_status_checked;

	unsigned long timeout;

	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);
	u32 port_id = hisi_pcie->port_id;

	if (port_id == 3) {
		reg_reset_ctrl = PCIE3_SUBCTRL_RESET_REQ_REG;
		reg_dereset_ctrl = PCIE3_SUBCTRL_DRESET_REQ_REG;
		reg_reset_status = PCIE3_SUBCTRL_RESET_ST_REG;
	} else {
		reg_reset_ctrl = PCIE0_2_SUBCTRL_RESET_REQ_REG(port_id);
		reg_dereset_ctrl = PCIE0_2_SUBCTRL_DRESET_REQ_REG(port_id);
		reg_reset_status = PCIE0_2_SUBCTRL_RESET_ST_REG(port_id);
	}

	if (reset_on) {
		/* if core is link up, stop the ltssm state machine first */
		if (hisi_pcie_link_up(pp))
			hisi_pcie_enable_ltssm(pp, 0);

		/* reset port */
		hisi_pcie_subctrl_writel(hisi_pcie, 0x1, reg_reset_ctrl);
	} else {
		/* dreset port */
		hisi_pcie_subctrl_writel(hisi_pcie, 0x1, reg_dereset_ctrl);
	}

	timeout = jiffies + HZ*1;

	do {
		reset_status = hisi_pcie_subctrl_readl(hisi_pcie,
							reg_reset_status);
		if (reset_on)
			reset_status_checked = ((reset_status & 0x01) != 1);
		else
			reset_status_checked = ((reset_status & 0x01) != 0);

	} while ((reset_status_checked) && (time_before(jiffies, timeout)));

	/* get a timeout error */
	if (reset_status_checked)
		dev_err(pp->dev, "error:pcie core reset or dereset failed!\n");
}

static void hisi_pcie_clock_ctrl(struct pcie_port *pp, bool clock_on)
{
	u32 reg_clock_disable;
	u32 reg_clock_enable;
	u32 reg_clock_status;

	u32 clock_status;
	u32 clock_status_checked;

	unsigned long timeout;

	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);
	u32 port_id = hisi_pcie->port_id;

	if (port_id == 3) {
		reg_clock_disable = PCIE_SUBCTRL_SC_PCIE3_CLK_DIS_REG;
		reg_clock_enable = PCIE_SUBCTRL_SC_PCIE3_CLK_EN_REG;
		reg_clock_status = PCIE_SUBCTRL_SC_PCIE3_CLK_ST_REG;
	} else {
		reg_clock_disable =
				PCIE_SUBCTRL_SC_PCIE0_2_CLK_DIS_REG(port_id);
		reg_clock_enable = PCIE_SUBCTRL_SC_PCIE0_2_CLK_EN_REG(port_id);
		reg_clock_status = PCIE_SUBCTRL_SC_PCIE0_2_CLK_ST_REG(port_id);
	}

	if (clock_on) {
		/* switch on pcie core clock */
		hisi_pcie_subctrl_writel(hisi_pcie, 0x3, reg_clock_enable);
	} else {
		/* switch off pcie core clock */
		hisi_pcie_subctrl_writel(hisi_pcie, 0x3, reg_clock_disable);
	}

	timeout = jiffies + HZ*1;

	do {
		clock_status = hisi_pcie_subctrl_readl(hisi_pcie,
						reg_clock_status);
		if (clock_on)
			clock_status_checked = ((clock_status & 0x03) != 0x3);
		else
			clock_status_checked = ((clock_status & 0x03) != 0);

	} while ((clock_status_checked) && (time_before(jiffies, timeout)));

	/* get a timeout error */
	if (clock_status_checked)
		dev_err(pp->dev, "error:clock operation failed!\n");
}

/* will implement in BIOS */
static void hisi_pcie_assert_core_reset(struct pcie_port *pp)
{
	hisi_pcie_core_reset_ctrl(pp, PCIE_ASSERT_RESET_ON);
	hisi_pcie_clock_ctrl(pp, PCIE_CLOCK_OFF);
}

/* will implement in BIOS */
static void hisi_pcie_deassert_core_reset(struct pcie_port *pp)
{
	hisi_pcie_core_reset_ctrl(pp, PCIE_DEASSERT_RESET_ON);
	hisi_pcie_clock_ctrl(pp, PCIE_CLOCK_ON);
}

/* will implement in BIOS */
static void hisi_pcie_deassert_pcs_reset(struct pcie_port *pp)
{
	u32 val0;

	u32 hilink_reset_status;
	u32 pcs_local_status;

	u32 hilink_status_checked;
	u32 pcs_local_status_checked;

	unsigned long timeout;

	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);
	u32 port_id = hisi_pcie->port_id;

	val0 = 1 << port_id;
	hisi_pcie_subctrl_writel(hisi_pcie, val0, PCIE_PCS_LOCAL_DRESET_REQ);

	val0 = 0xff << (port_id * 8);
	hisi_pcie_subctrl_writel(hisi_pcie, val0, PCIE_PCS_DRESET_REQ_REG);

	timeout = jiffies + HZ*1;

	/*read reset status,make sure pcs is deassert */
	do {
		pcs_local_status = hisi_pcie_subctrl_readl(hisi_pcie,
						 PCIE_PCS_LOCAL_RESET_ST);
		pcs_local_status_checked = (pcs_local_status & (1 << port_id));
	} while ((pcs_local_status_checked) && (time_before(jiffies, timeout)));

	/* get a timeout error */
	if (pcs_local_status_checked)
		dev_err(pp->dev, "pcs deassert reset failed!\n");

	timeout = jiffies + HZ*1;

	do {
		hilink_reset_status = hisi_pcie_subctrl_readl(hisi_pcie,
						 PCIE_PCS_RESET_REG_ST);
		hilink_status_checked = (hilink_reset_status &
						(0xff << (port_id * 8)));
	} while ((hilink_status_checked) && (time_before(jiffies, timeout)));

	if (hilink_status_checked)
		dev_err(pp->dev, "pcs deassert reset  failed!\n");
}

/* will implement in BIOS */
static void hisi_pcie_assert_pcs_reset(struct pcie_port *pp)
{
	u32 reg;
	u32 hilink_reset_status;
	u32 pcs_local_reset_status;

	u32 hilink_status_checked;
	u32 pcs_local_status_checked;

	unsigned long timeout;

	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);
	u32 port_id = hisi_pcie->port_id;

	reg = 1 << port_id;
	hisi_pcie_subctrl_writel(hisi_pcie, reg, PCIE_PCS_LOCAL_RESET_REQ);

	reg = 0xff << (port_id * 8);
	hisi_pcie_subctrl_writel(hisi_pcie, reg, PCIE_PCS_RESET_REQ_REG);

	timeout = jiffies + HZ*1;

	/*read reset status,make sure pcs is reset */
	do {
		pcs_local_reset_status = hisi_pcie_subctrl_readl(hisi_pcie,
						 PCIE_PCS_LOCAL_RESET_ST);
		pcs_local_status_checked =
		  ((pcs_local_reset_status & (1 << port_id)) != (1 << port_id));

	} while ((pcs_local_status_checked) && (time_before(jiffies, timeout)));

	if (pcs_local_status_checked)
		dev_err(pp->dev, "pcs local reset status read failed\n");

	timeout = jiffies + HZ*1;

	do {
		hilink_reset_status = hisi_pcie_subctrl_readl(hisi_pcie,
						 PCIE_PCS_RESET_REG_ST);
		hilink_status_checked =
			((hilink_reset_status & (0xff << (port_id << 3))) !=
					(0xff << (port_id << 3)));
	} while ((hilink_status_checked) && (time_before(jiffies, timeout)));

	if (hilink_status_checked)
		dev_err(pp->dev, "error:pcs assert reset failed\n");
}

/* will implement in BIOS */
static void hisi_pcie_init_pcs(struct pcie_port *pp)
{
	u32 lane_num = 8;
	u32 i;
	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);

	if (hisi_pcie->port_id <= 2) {
		u32 *addr = ioremap_nocache(0xb200c088, 0x100);
		u32 *addr1 = ioremap_nocache(0xb210c088, 0x100);
		u32 *addr2 = ioremap_nocache(0xb218c088, 0x100);
		u32 *addr3 = ioremap_nocache(0xb208c088, 0x100);
		*addr = 0x212;
		*addr1 = 0x212;
		*addr2 = 0x212;
		*addr3 = 0x212;
		iounmap(addr);
		iounmap(addr1);
		iounmap(addr2);
		iounmap(addr3);

		hisi_pcie_pcs_writel(hisi_pcie, 0x2026044, 0x8020);
		hisi_pcie_pcs_writel(hisi_pcie, 0x2126044, 0x8060);
		hisi_pcie_pcs_writel(hisi_pcie, 0x2126044, 0x80c4);
		hisi_pcie_pcs_writel(hisi_pcie, 0x2026044, 0x80e4);
		hisi_pcie_pcs_writel(hisi_pcie, 0x4018, 0x80a0);
		hisi_pcie_pcs_writel(hisi_pcie, 0x804018, 0x80a4);
		hisi_pcie_pcs_writel(hisi_pcie, 0x11201100, 0x80c0);
		hisi_pcie_pcs_writel(hisi_pcie, 0x3, 0x15c);
		hisi_pcie_pcs_writel(hisi_pcie, 0x0, 0x158);
	} else {
		for (i = 0; i < lane_num; i++)
			hisi_pcie_pcs_writel(hisi_pcie, 0x46e000,
					     PCIE_M_PCS_IN15_CFG + (i << 2));
		for (i = 0; i < lane_num; i++)
			hisi_pcie_pcs_writel(hisi_pcie, 0x1001,
					     PCIE_M_PCS_IN13_CFG + (i << 2));

		hisi_pcie_pcs_writel(hisi_pcie, 0xffff, PCIE_PCS_RXDETECTED);
	}
}

static void hisi_pcie_config_context(struct pcie_port *pp)
{
	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);
	int i = 0;

	hisi_pcie_apb_slave_mode(pp, PCIE_SLV_CONTENT_MODE);

	for (i = 0; i < 0x400; i++)
		hisi_pcie_apb_writel(hisi_pcie, 0x0, i * 4);

	for (i = 0x400; i < 0x800; i++)
		hisi_pcie_apb_writel(hisi_pcie, 0x0, i * 4);

	hisi_pcie_apb_slave_mode(pp, PCIE_SLV_SYSCTRL_MODE);

	/* FIX ME! */
	hisi_pcie_apb_writel(hisi_pcie, 0xb7010040, 0x1b4);
	hisi_pcie_apb_writel(hisi_pcie, 0x0, 0x1c4);
	hisi_pcie_apb_writel(hisi_pcie, PCIE_MSI_CONTEXT_VALUE, 0x10);
	hisi_pcie_apb_writel(hisi_pcie, PCIE_MSI_TRANS_ENABLE, 0x1c8);

	hisi_pcie_apb_slave_mode(pp, PCIE_SLV_DBI_MODE);
}

static void hisi_pcie_mask_link_up_int(struct pcie_port *pp)
{
	u32 val;
	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);

	hisi_pcie_apb_slave_mode(pp, PCIE_SLV_SYSCTRL_MODE);
	val = hisi_pcie_apb_readl(hisi_pcie, 0x1d0);
	val |= 1 << 12;
	hisi_pcie_apb_writel(hisi_pcie, val, 0x1d0);
	hisi_pcie_apb_slave_mode(pp, PCIE_SLV_DBI_MODE);
}

void pcie_equalization(struct pcie_port *pp)
{
	u32 val = 0;
	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);

	if (hisi_pcie->port_id <= 2) {
		hisi_pcie_apb_writel(hisi_pcie, 0x1400, 0x890);
		hisi_pcie_apb_writel(hisi_pcie, 0xfd7, 0x894);

		val = hisi_pcie_apb_readl(hisi_pcie, 0x80);
		val |= 0x80;
		hisi_pcie_apb_writel(hisi_pcie, val, 0x80);

		hisi_pcie_apb_writel(hisi_pcie, 0x0, 0x89c);
		hisi_pcie_apb_writel(hisi_pcie, 0xfc00, 0x898);
		hisi_pcie_apb_writel(hisi_pcie, 0x1, 0x89c);
		hisi_pcie_apb_writel(hisi_pcie, 0xdb00, 0x898);
		hisi_pcie_apb_writel(hisi_pcie, 0x2, 0x89c);
		hisi_pcie_apb_writel(hisi_pcie, 0xccc0, 0x898);
		hisi_pcie_apb_writel(hisi_pcie, 0x3, 0x89c);
		hisi_pcie_apb_writel(hisi_pcie, 0x8dc0, 0x898);
		hisi_pcie_apb_writel(hisi_pcie, 0x4, 0x89c);
		hisi_pcie_apb_writel(hisi_pcie, 0xfc0, 0x898);
		hisi_pcie_apb_writel(hisi_pcie, 0x5, 0x89c);
		hisi_pcie_apb_writel(hisi_pcie, 0xe46, 0x898);
		hisi_pcie_apb_writel(hisi_pcie, 0x6, 0x89c);
		hisi_pcie_apb_writel(hisi_pcie, 0x7, 0x89c);
		hisi_pcie_apb_writel(hisi_pcie, 0xcb46, 0x898);
		hisi_pcie_apb_writel(hisi_pcie, 0x8, 0x89c);
		hisi_pcie_apb_writel(hisi_pcie, 0x8c07, 0x898);
		hisi_pcie_apb_writel(hisi_pcie, 0x9, 0x89c);
		hisi_pcie_apb_writel(hisi_pcie, 0xd0b, 0x898);
		hisi_pcie_apb_writel(hisi_pcie, 0x103ff21, 0x8a8);

		hisi_pcie_apb_writel(hisi_pcie, 0x44444444, 0x184);
		hisi_pcie_apb_writel(hisi_pcie, 0x44444444, 0x188);
		hisi_pcie_apb_writel(hisi_pcie, 0x44444444, 0x18c);
		hisi_pcie_apb_writel(hisi_pcie, 0x44444444, 0x190);
	} else {
		hisi_pcie_apb_writel(hisi_pcie, 0x10e01, 0x890);
	}
}

/* will implement in BIOS */
static int hisi_pcie_establish_link(struct pcie_port *pp)
{
	u32 val;
	int count = 0;
	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);

	if (dw_pcie_link_up(pp)) {
		dev_err(pp->dev, "Link already up\n");
		return 0;
	}

	/* assert reset signals */
	hisi_pcie_assert_core_reset(pp);
	hisi_pcie_assert_pcs_reset(pp);

	/* de-assert phy reset */
	hisi_pcie_deassert_pcs_reset(pp);

	/* de-assert core reset */
	hisi_pcie_deassert_core_reset(pp);

	/* initialize phy */
	hisi_pcie_init_pcs(pp);

	/* setup root complex */
	dw_pcie_setup_rc(pp);

	/* disable link up interrupt */
	hisi_pcie_mask_link_up_int(pp);

	pcie_equalization(pp);

	/* assert LTSSM enable */
	hisi_pcie_enable_ltssm(pp, 1);

	/* check if the link is up or not */
	while (!dw_pcie_link_up(pp)) {
		mdelay(100);
		count++;
		if (count == 10) {
			while ((hisi_pcie_pcs_readl(hisi_pcie,
				PCIE_PCS_SERDES_STATUS) & 0x3) == 0) {
				val = hisi_pcie_pcs_readl(hisi_pcie,
				      PCIE_PCS_SERDES_STATUS);
				dev_info(pp->dev, "PLL Locked: 0x%x\n", val);
			}

			dev_err(pp->dev, "PCIe Link Fail\n");
			return -EINVAL;
		}
	}

	/*add a 1s delay between linkup and enumeration,make sure
	 the EP device'sconfiguration registers are prepared well */
	mdelay(999);
	dev_info(pp->dev, "Link up\n");

	return 0;
}

static void hisi_pcie_host_init(struct pcie_port *pp)
{
	hisi_pcie_establish_link(pp);
	hisi_pcie_config_context(pp);
}

static struct pcie_host_ops hisi_pcie_host_ops = {
	.link_up = hisi_pcie_link_up,
	.host_init = hisi_pcie_host_init,
};

static int __init hisi_add_pcie_port(struct pcie_port *pp,
				     struct platform_device *pdev)
{
	int ret;
	struct resource busn;

	struct hisi_pcie *hisi_pcie = to_hisi_pcie(pp);

	if (of_property_read_u32(pdev->dev.of_node, "port-id",
				 &hisi_pcie->port_id)) {
		dev_err(&pdev->dev, "failed to property port-id\n");
		return -EINVAL;
	}

	if (of_pci_parse_bus_range(pdev->dev.of_node, &busn)) {
		dev_err(&pdev->dev, "fail to read bus-ranges\n");
		return -ENOMEM;
	}

	/* set root pcie tree base bus */
	pp->root_bus_nr = busn.start;
	pp->ops = &hisi_pcie_host_ops;

	ret = dw_pcie_host_init(pp);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize host\n");
		return ret;
	}
	return 0;
}

static
void hisi_pcie_msi_enable(struct device_node *np, struct hisi_pcie *hisi_pcie)
{
	struct device_node *msi_node;
	struct irq_domain *irq_domain;

	msi_node = of_parse_phandle(np,  "msi-parent", 0);
	if (!msi_node) {
		pr_err("failed to find msi-parent\n");
		return;
	}

	irq_domain = irq_find_host(msi_node);
	if (!irq_domain) {
		pr_err("failed to find irq domain\n");
		return;
	}

	hisi_pcie->pp.domain = irq_domain;
}

static int hisi_pcie_probe(struct platform_device *pdev)
{
	struct hisi_pcie *hisi_pcie;
	struct pcie_port *pp;
	struct device_node *np = pdev->dev.of_node;
	struct resource *reg;
	struct resource *subctrl;
	struct resource *phy;
	int ret;

	hisi_pcie = devm_kzalloc(&pdev->dev, sizeof(*hisi_pcie), GFP_KERNEL);
	if (!hisi_pcie)
		return -ENOMEM;

	pp = &hisi_pcie->pp;
	pp->dev = &pdev->dev;

	subctrl = platform_get_resource_byname(pdev, IORESOURCE_MEM, "subctrl");
	hisi_pcie->subctrl_base = devm_ioremap_nocache(&pdev->dev,
					subctrl->start, resource_size(subctrl));
	if (IS_ERR(hisi_pcie->subctrl_base))
		return PTR_ERR(hisi_pcie->subctrl_base);

	reg = platform_get_resource_byname(pdev, IORESOURCE_MEM, "rc_dbi");
	hisi_pcie->reg_base = devm_ioremap_resource(&pdev->dev, reg);
	if (IS_ERR(hisi_pcie->reg_base))
		return PTR_ERR(hisi_pcie->reg_base);

	/* dbi slave use the common IO port with pcie's sys-state reg */
	hisi_pcie->pp.dbi_base = hisi_pcie->reg_base;

	phy = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pcs");
	hisi_pcie->phy_base = devm_ioremap_resource(&pdev->dev, phy);
	if (IS_ERR(hisi_pcie->phy_base))
		return PTR_ERR(hisi_pcie->phy_base);

	hisi_pcie_msi_enable(np, hisi_pcie);

	ret = hisi_add_pcie_port(pp, pdev);
	if (ret < 0)
		return ret;

	platform_set_drvdata(pdev, hisi_pcie);

	return ret;
}

static const struct of_device_id hisi_pcie_of_match[] = {
	{.compatible = "hisilicon,hip05-pcie",},
	{},
};

MODULE_DEVICE_TABLE(of, hisi_pcie_of_match);

static struct platform_driver hisi_pcie_driver = {
	.probe  = hisi_pcie_probe,
	.driver = {
		   .name = "hisi-pcie",
		   .owner = THIS_MODULE,
		   .of_match_table = hisi_pcie_of_match,
	},
};

module_platform_driver(hisi_pcie_driver);

MODULE_AUTHOR("Zhou Wang <wangzhou1@huawei.com>");
MODULE_AUTHOR("Dacai Zhu <zhudacai@huawei.com>");
MODULE_LICENSE("GPL v2");

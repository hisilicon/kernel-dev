/* SPDX-License-Identifier: GPL-2.0+ */
/*  Copyright (c) 2017-2018 Hisilicon Limited. */

/*
 * Driver for Hisilicon djtag r/w via System Controller.
 */

#ifndef __HISI_DJTAG_H
#define __HISI_DJTAG_H

int djtag_readl(struct device_node *node, u32 offset, u32 mod_sel,
					int chain_id, u32 *val);
int djtag_writel(struct device_node *node, u32 offset, u32 mod_sel,
					u32 mod_mask, u32 val);
#endif

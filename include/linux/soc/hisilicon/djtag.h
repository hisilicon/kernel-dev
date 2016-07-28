/*
 * Driver for Hisilicon djtag r/w via System Controller.
 *
 * Copyright (C) 2016-2017 Hisilicon Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __HISI_DJTAG_H
#define __HISI_DJTAG_H

int hisi_djtag_readl(struct device_node *node, u32 offset, u32 mod_sel,
						int chain_id, u32 *val);
int hisi_djtag_writel(struct device_node *node, u32 offset, u32 mod_sel,
						u32 mod_mask, u32 val);
#endif /* __HISI_DJTAG_H */

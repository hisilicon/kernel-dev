/*
 * PCIe host controller common functions for HiSilicon SoCs
 *
 * Copyright (C) 2015 HiSilicon Co., Ltd. http://www.hisilicon.com
 *
 * Author: Zhou Wang <wangzhou1@hisilicon.com>
 *         Dacai Zhu <zhudacai@hisilicon.com>
 *         Gabriele Paoloni <gabriele.paoloni@huawei.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/pci.h>
#include "pcie-hisi.h"

/* HipXX PCIe host only supports 32-bit config access */
int hisi_pcie_common_cfg_read(void __iomem *reg_base, int where, int size,
			      u32 *val)
{
	u32 reg;
	u32 reg_val;
	void *walker = &reg_val;

	walker += (where & 0x3);
	reg = where & ~0x3;
	reg_val = readl(reg_base + reg);

	if (size == 1)
		*val = *(u8 __force *) walker;
	else if (size == 2)
		*val = *(u16 __force *) walker;
	else if (size == 4)
		*val = reg_val;
	else
		return PCIBIOS_BAD_REGISTER_NUMBER;

	return PCIBIOS_SUCCESSFUL;
}

/* HipXX PCIe host only supports 32-bit config access */
int hisi_pcie_common_cfg_write(void __iomem *reg_base, int where, int  size,
				u32 val)
{
	u32 reg_val;
	u32 reg;
	void *walker = &reg_val;

	walker += (where & 0x3);
	reg = where & ~0x3;
	if (size == 4)
		writel(val, reg_base + reg);
	else if (size == 2) {
		reg_val = readl(reg_base + reg);
		*(u16 __force *) walker = val;
		writel(reg_val, reg_base + reg);
	} else if (size == 1) {
		reg_val = readl(reg_base + reg);
		*(u8 __force *) walker = val;
		writel(reg_val, reg_base + reg);
	} else
		return PCIBIOS_BAD_REGISTER_NUMBER;

	return PCIBIOS_SUCCESSFUL;
}

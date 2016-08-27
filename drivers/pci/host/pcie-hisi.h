/*
 * PCIe host controller driver for HiSilicon SoCs
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
#ifndef PCIE_HISI_H_
#define PCIE_HISI_H_

#define PCIE_LTSSM_LINKUP_STATE				0x11
#define PCIE_LTSSM_STATE_MASK				0x3F

int hisi_pcie_common_cfg_read(void __iomem *reg_base, int where, int size,
			      u32 *val);
int hisi_pcie_common_cfg_write(void __iomem *reg_base, int where, int size,
			       u32 val);

#endif /* PCIE_HISI_H_ */

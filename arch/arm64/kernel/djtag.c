/*
  djtag.c
  Hisilicon djtag interface
  Copyright(c) 2014 - 2019 Huawei Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.
 */

#include <linux/kernel.h>
#include <asm/io.h>
#include "djtag.h"

/*#define DEBUG */

/* djtag read interface
 *
 */

int hisi_djtag_readreg(u32 module_id, u32 cfg_en, u32 offset,
				u64 phys_base_addr, u32 *pvalue)
{
	u32 i = 0;
	u32 start;

	if (phys_base_addr == 0)
		return -1;

#ifdef DEBUG
	pr_info("\n module_id=%x cfg_en=%x offset=%x baseaddr=%llx\n",
				module_id, cfg_en, offset, phys_base_addr);
#endif

	/* make sure djtag has finished */
	do {
		read_reg(phys_base_addr + t1n1_djtag_mstr_start_en_reg, &start);
	} while ((start & DJTAG_START) == 1);

	/* configure the module_id */
	write_reg(phys_base_addr + t1n1_djtag_debug_module_sel_reg, module_id);

	/* configure operate djtag mode : read */
	write_reg(phys_base_addr + t1n1_djtag_mstr_wr_reg, DJTAG_MSTR_READ);

	/* configure the offset of the register */
	write_reg(phys_base_addr + t1n1_djtag_mstr_addr_reg, offset);

	/* configure the chain uint cfg en */
	write_reg(phys_base_addr + t1n1_djtag_chain_unit_cfg_en_reg, cfg_en);

	/* start the djtag */
	write_reg(phys_base_addr + t1n1_djtag_mstr_start_en_reg, DJTAG_START);

	/* make sure the djtag has done */
	do {
		read_reg(phys_base_addr + t1n1_djtag_mstr_start_en_reg, &start);
	} while ((start & DJTAG_START) == 1);

	while (cfg_en != 1) {
		cfg_en = (cfg_en >> 0x1);
		i++;
	}

	read_reg((phys_base_addr + t1n1_djtag_rd_data0_reg + i * 4), pvalue);

	return 0;
}


int hisi_djtag_writereg(u32 module_id, u32 cfg_en, u32 offset, u32 value,
							u64 phys_base_addr)
{
	u32 start;

	if (phys_base_addr == 0)
		return -1;
#ifdef DEBUG
	pr_info("\n module_id=%x cfg_en=%x offset=%x baseaddr=%llx\n",
				module_id, cfg_en, offset, phys_base_addr);
#endif

	/* make sure djtag has finished */
	do {
		read_reg(phys_base_addr + t1n1_djtag_mstr_start_en_reg, &start);
#ifdef DEBUG
		pr_info("djtag wait for ready..\n");
#endif
	} while ((start & DJTAG_START) == 1);

	/* configure the module_id */
	write_reg(phys_base_addr + t1n1_djtag_debug_module_sel_reg, module_id);

	/* configure operate djtag mode : write */
	write_reg(phys_base_addr + t1n1_djtag_mstr_wr_reg, DJTAG_MSTR_WRITE);

	/* configure the offset of the register */
	write_reg(phys_base_addr + t1n1_djtag_mstr_addr_reg, offset);

	/* configure the chain uint cfg en */
	write_reg(phys_base_addr + t1n1_djtag_chain_unit_cfg_en_reg, cfg_en);

	/* configure the written value */
	write_reg(phys_base_addr + t1n1_djtag_mstr_data_reg, value);

	/* start the djtag */
	write_reg(phys_base_addr + t1n1_djtag_mstr_start_en_reg, DJTAG_START);

	/* make sure the djtag has done */
	do {
		read_reg(phys_base_addr + t1n1_djtag_mstr_start_en_reg, &start);
	} while ((start & DJTAG_START) == 1);

	return 0;
}

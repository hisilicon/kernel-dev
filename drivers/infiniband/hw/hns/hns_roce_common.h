/*
 * Copyright (c) 2016 Hisilicon Limited.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _HNS_ROCE_COMMON_H
#define _HNS_ROCE_COMMON_H

#define roce_write(dev, reg, val)	writel((val), (dev)->reg_base + (reg))
#define roce_read(dev, reg)		readl((dev)->reg_base + (reg))
#define roce_raw_write(value, addr) \
	__raw_writel((__force u32)cpu_to_le32(value), (addr))

#define roce_get_field(origin, mask, shift) \
	(((origin) & (mask)) >> (shift))

#define roce_get_bit(origin, shift) \
	roce_get_field((origin), (1ul << (shift)), (shift))

#define roce_set_field(origin, mask, shift, val) \
	do { \
		(origin) &= (~(mask)); \
		(origin) |= (((u32)(val) << (shift)) & (mask)); \
	} while (0)

#define roce_set_bit(origin, shift, val) \
	roce_set_field((origin), (1ul << (shift)), (shift), (val))

#define ROCEE_CAEP_AEQC_AEQE_SHIFT_CAEP_AEQC_STATE_S 0
#define ROCEE_CAEP_AEQC_AEQE_SHIFT_CAEP_AEQC_STATE_M   \
	(((1UL << 2) - 1) << ROCEE_CAEP_AEQC_AEQE_SHIFT_CAEP_AEQC_STATE_S)

#define ROCEE_CAEP_AEQC_AEQE_SHIFT_CAEP_AEQC_AEQE_SHIFT_S 8
#define ROCEE_CAEP_AEQC_AEQE_SHIFT_CAEP_AEQC_AEQE_SHIFT_M   \
	(((1UL << 4) - 1) << ROCEE_CAEP_AEQC_AEQE_SHIFT_CAEP_AEQC_AEQE_SHIFT_S)

#define ROCEE_CAEP_AEQC_AEQE_SHIFT_CAEP_AEQ_ALM_OVF_INT_ST_S 17

#define ROCEE_CAEP_AEQE_CUR_IDX_CAEP_AEQ_BT_H_S 0
#define ROCEE_CAEP_AEQE_CUR_IDX_CAEP_AEQ_BT_H_M   \
	(((1UL << 5) - 1) << ROCEE_CAEP_AEQE_CUR_IDX_CAEP_AEQ_BT_H_S)

#define ROCEE_CAEP_AEQE_CUR_IDX_CAEP_AEQE_CUR_IDX_S 16
#define ROCEE_CAEP_AEQE_CUR_IDX_CAEP_AEQE_CUR_IDX_M   \
	(((1UL << 16) - 1) << ROCEE_CAEP_AEQE_CUR_IDX_CAEP_AEQE_CUR_IDX_S)

#define ROCEE_CAEP_AEQE_CONS_IDX_CAEP_AEQE_CONS_IDX_S 0
#define ROCEE_CAEP_AEQE_CONS_IDX_CAEP_AEQE_CONS_IDX_M   \
	(((1UL << 16) - 1) << ROCEE_CAEP_AEQE_CONS_IDX_CAEP_AEQE_CONS_IDX_S)

#define ROCEE_CAEP_CEQC_SHIFT_CAEP_CEQ_ALM_OVF_INT_ST_S 16
#define ROCEE_CAEP_CE_IRQ_MASK_CAEP_CEQ_ALM_OVF_MASK_S 1
#define ROCEE_CAEP_CEQ_ALM_OVF_CAEP_CEQ_ALM_OVF_S 0

#define ROCEE_CAEP_AE_MASK_CAEP_AEQ_ALM_OVF_MASK_S 0
#define ROCEE_CAEP_AE_MASK_CAEP_AE_IRQ_MASK_S 1

#define ROCEE_CAEP_AE_ST_CAEP_AEQ_ALM_OVF_S 0

/*************ROCEE_REG DEFINITION****************/
#define ROCEE_VENDOR_ID_REG			0x0
#define ROCEE_VENDOR_PART_ID_REG		0x4

#define ROCEE_HW_VERSION_REG			0x8

#define ROCEE_SYS_IMAGE_GUID_L_REG		0xC
#define ROCEE_SYS_IMAGE_GUID_H_REG		0x10

#define ROCEE_CAEP_AEQE_CONS_IDX_REG		0x3AC
#define ROCEE_CAEP_CEQC_CONS_IDX_0_REG		0x3BC

#define ROCEE_ECC_UCERR_ALM1_REG		0xB38
#define ROCEE_ECC_UCERR_ALM2_REG		0xB3C
#define ROCEE_ECC_CERR_ALM1_REG			0xB44
#define ROCEE_ECC_CERR_ALM2_REG			0xB48

#define ROCEE_ACK_DELAY_REG			0x14

#define ROCEE_CAEP_CE_INTERVAL_CFG_REG		0x190
#define ROCEE_CAEP_CE_BURST_NUM_CFG_REG		0x194

#define ROCEE_MB1_REG				0x210

#define ROCEE_CAEP_AEQC_AEQE_SHIFT_REG		0x3A0
#define ROCEE_CAEP_CEQC_SHIFT_0_REG		0x3B0
#define ROCEE_CAEP_CE_IRQ_MASK_0_REG		0x3C0
#define ROCEE_CAEP_CEQ_ALM_OVF_0_REG		0x3C4
#define ROCEE_CAEP_AE_MASK_REG			0x6C8
#define ROCEE_CAEP_AE_ST_REG			0x6CC

#define ROCEE_ECC_UCERR_ALM0_REG		0xB34
#define ROCEE_ECC_CERR_ALM0_REG			0xB40

#endif /* _HNS_ROCE_COMMON_H */

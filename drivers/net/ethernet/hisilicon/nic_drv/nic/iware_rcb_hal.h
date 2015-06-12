/************************************************************************

  Hisilicon NIC driver
  Copyright(c) 2014 - 2019 Huawei Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:TBD

************************************************************************/

#ifndef _RCB_HAL_H
#define _RCB_HAL_H
/*
#include "iware_module.h"
#include "iware_typedef.h" */

#include "iware_rcb_main.h"

/*----------------------------------------------*/
/*NIC MAX tx buffer num for hardware */
#define NIC_MAX_TX_BUF_NUM 3

#define RCB_COMMON_ENDIAN 1

#define RCB_NOF_ALLOC_RX_BUFF_ONCE 16

#define RCB_ERROR_PRINT_CYCLE 1000

#define ETH_STATIC_REG	 1
#define ETH_DUMP_REG	   5

#define RCB_RESET_WAIT_TIMES 30
#define RCB_RESET_TRY_TIMES 10

#define BD_SIZE_512_TYPE 0
#define BD_SIZE_1024_TYPE 1
#define BD_SIZE_2048_TYPE 2
#define BD_SIZE_4096_TYPE 3

#define ETH_DUMP_REG_NUM 500
#define ETH_STATIC_REG_NUM 12

/************************************************************/
/*					PhosphorV660 RCB_COMMON			*/
/*************************************************************/
#define RCB_COM_CFG_ENDIAN_REG				(0x0)
#define RCB_COM_CFG_RING_CORE_NUM_REG			(0x4)
#define RCB_COM_CFG_RING_IT_NUM_REG			(0x8)
#define RCB_COM_CFG_SYS_FSH_REG				(0xC)
#define RCB_COM_CFG_INIT_FLAG_REG				(0x10)
#define RCB_COM_CFG_DSA_FABRIC_MODE_REG		(0x14)
#define RCB_COM_CFG_PPE0_QID_NUM_REG			(0x18)
#define RCB_COM_CFG_PPE1_QID_NUM_REG			(0x1C)
#define RCB_COM_CFG_PPE2_QID_NUM_REG			(0x20)
#define RCB_COM_CFG_PPE3_QID_NUM_REG			(0x24)
#define RCB_COM_CFG_PPE4_QID_NUM_REG			(0x28)
#define RCB_COM_CFG_PPE5_QID_NUM_REG			(0x2C)
#define RCB_COM_CFG_PKT_REG					(0x30)
#define RCB_COM_CFG_RINVLD_REG				(0x34)
#define RCB_COM_CFG_FNA_REG					(0x38)
#define RCB_COM_CFG_FA_REG					(0x3C)
#define RCB_COM_CFG_PKT_TC_BP_REG				(0x40)
#define RCB_COM_CFG_PPE_TNL_CLKEN_REG			(0x44)
#define RCB_COM_CPU_CHK_TASK_STS_REG			(0x100)
#define RCB_COM_INTMSK_TX_PKT_REG				(0x3A0)
#define RCB_COM_RINT_TX_PKT_REG				(0x3A8)
#define RCB_COM_INTSTS_TX_BDNUM_REG			(0x3B0)
#define RCB_COM_RINT_TX_BDNUM_REG				(0x3BC)
#define RCB_COM_INT_CPU_TX_BD_ERR_EN_REG		(0x3C0)
#define RCB_COM_INTSTS_CPU_TX_BD_ERR_REG		(0x3C4)
#define RCB_COM_RINT_CPU_TX_BD_ERR_REG		(0x3C8)
#define RCB_COM_INTMSK_CPU_TX_ERR_REG			(0x3CC)
#define RCB_COM_INTSTS_CPU_TX_ERR_REG			(0x3D0)
#define RCB_COM_RINT_CPU_TX_ERR_REG			(0x3D4)
#define RCB_COM_INTMASK_ECC_ERR_REG			(0x400)
#define RCB_COM_INTSTS_ECC_ERR_REG			(0x408)
#define RCB_COM_EBD_MA_ERR_REG				(0x40C)
#define RCB_COM_EBD_SRAM_ERR_REG				(0x410)
#define RCB_COM_RXRING_ERR_REG				(0x41C)
#define RCB_COM_TXRING_ERR_REG				(0x420)
#define RCB_COM_TX_FBD_ERR_REG				(0x424)
#define RCB_SRAM_ECC_CHK_EN_REG				(0x428)
#define RCB_SRAM_ECC_CHK0_REG					(0x42C)
#define RCB_SRAM_ECC_CHK1_REG					(0x430)
#define RCB_SRAM_ECC_CHK2_REG					(0x434)
#define RCB_SRAM_ECC_CHK3_REG					(0x438)
#define RCB_SRAM_ECC_CHK4_REG					(0x43c)
#define RCB_SRAM_ECC_CHK5_REG					(0x440)
#define RCB_ECC_ERR_ADDR0_REG					(0x450)
#define RCB_ECC_ERR_ADDR3_REG					(0x45C)
#define RCB_ECC_ERR_ADDR4_REG					(0x460)
#define RCB_ECC_ERR_ADDR5_REG					(0x464)
#define RCB_COM_SF_CFG_INTMASK_RING			(0x480)
#define RCB_COM_SF_CFG_RING_STS				(0x484)
#define RCB_COM_SF_CFG_RING					(0x488)
#define RCB_COM_SF_CFG_INTMASK_BD				(0x48C)
#define RCB_COM_SF_CFG_BD_RINT_STS			(0x470)
#define RCB_COM_RCB_RD_BD_BUSY				(0x490)
#define RCB_COM_RCB_FBD_CRT_EN				(0x494)
#define RCB_COM_AXI_WR_ERR_INTMASK			(0x498)
#define RCB_COM_AXI_ERR_STS					(0x49C)
#define RCB_COM_CHK_TX_FBD_NUM_REG			(0x4a0)

#define RCB_TX_RD_FBD_OTS_FULL_REG			(0x600)
#define RCB_TX_FIFO_FULL_REG					(0x604)
#define RCB_TX_ACK_OTS_FULL_REG				(0x608)
#define RCB_RX_RD_EBD_OTS_FULL_REG			(0x60c)
#define RCB_RX_ACK_OTS_FULL_REG				(0x610)
#define RCB_RX_RING_FULL_REG					(0x614)
#define RCB_RX_RING_PIPELINE_REG				(0x618)
#define RCB_TX_RING_PIPELINE_REG				(0x61c)
#define RCB_CTRL_CPU_WR_FBD_WORD0_REG			(0x3000)
#define RCB_CTRL_CPU_WR_FBD_WORD1_REG			(0x3004)
#define RCB_CTRL_CPU_WR_FBD_WORD2_REG			(0x3008)
#define RCB_CTRL_CPU_WR_FBD_WORD3_REG			(0x300C)
#define RCB_CTRL_CPU_WR_FBD_WORD4_REG			(0x3010)
#define RCB_CTRL_CPU_WR_FBD_WORD5_REG			(0x3014)
#define RCB_CTRL_CPU_WR_FBD_WORD6_REG			(0x3018)
#define RCB_CTRL_CPU_WR_FBD_WORD7_REG			(0x301C)
#define RCB_CTRL_CPU_WR_EBD_WORD0_REG			(0x7000)
#define RCB_CTRL_CPU_WR_EBD_WORD1_REG			(0x7004)
#define RCB_CTRL_CPU_WR_EBD_WORD2_REG			(0x7008)
#define RCB_CTRL_CPU_WR_EBD_WORD3_REG			(0x700C)
#define RCB_CTRL_CPU_WR_EBD_WORD4_REG			(0x7010)
#define RCB_CTRL_CPU_WR_EBD_WORD5_REG			(0x7014)
#define RCB_CTRL_CPU_WR_EBD_WORD6_REG			(0x7018)
#define RCB_CTRL_CPU_WR_EBD_WORD7_REG			(0x701C)
#define RCB_CTRL_CPU_RD_FBD_WORD0_REG			(0x8000)
#define RCB_CTRL_CPU_RD_FBD_WORD1_REG			(0x8004)
#define RCB_CTRL_CPU_RD_FBD_WORD2_REG			(0x8008)
#define RCB_CTRL_CPU_RD_FBD_WORD3_REG			(0x800C)
#define RCB_CTRL_CPU_RD_FBD_WORD4_REG			(0x8010)
#define RCB_CTRL_CPU_RD_FBD_WORD5_REG			(0x8014)
#define RCB_CTRL_CPU_RD_FBD_WORD6_REG			(0x8018)
#define RCB_CTRL_CPU_RD_FBD_WORD7_REG			(0x801C)
#define RCB_CFG_BD_NUM_REG					(0x9000)
#define RCB_CFG_PKTLINE_REG					(0x9050)
#define RCB_CFG_OVERTIME_REG					(0x9300)
#define RCB_CFG_PKTLINE_INT_NUM_REG			(0x9304)
#define RCB_CFG_OVERTIME_INT_NUM_REG			(0x9308)
#define RCB_INTMASK_PKTLINE_REG				(0x9400)
#define RCB_RINT_PKTLINE_REG					(0x9404)
#define RCB_INTMASK_Pbd_num_REG				(0x9408)
#define RCB_RINT_bd_num_REG					(0x940C)

#define RCB_RING_RX_RING_BASEADDR_L_REG		(0x00000)
#define RCB_RING_RX_RING_BASEADDR_H_REG		(0x00004)
#define RCB_RING_RX_RING_BD_NUM_REG			(0x00008)
#define RCB_RING_RX_RING_BD_LEN_REG			(0x0000C)
#define RCB_RING_RX_RING_PKTLINE_REG			(0x00010)

#define RCB_RING_RX_RING_TAIL_REG				(0x00018)
#define RCB_RING_RX_RING_HEAD_REG				(0x0001C)
#define RCB_RING_RX_RING_FBDNUM_REG			(0x00020)
#define RCB_RING_RX_RING_OFFSET_REG			(0x00024)

#define RCB_RING_RX_RING_PKTNUM_RECORD_REG	(0x0002C)
#define RCB_RING_TX_RING_BASEADDR_L_REG		(0x00040)
#define RCB_RING_TX_RING_BASEADDR_H_REG		(0x00044)
#define RCB_RING_TX_RING_BD_NUM_REG			(0x00048)
#define RCB_RING_TX_RING_BD_LEN_REG			(0x0004C)
#define RCB_RING_TX_RING_PKTLINE_REG			(0x00050)
#define RCB_RING_TX_VM_REG				(0x00054)
#define RCB_RING_TX_RING_TAIL_REG				(0x00058)
#define RCB_RING_TX_RING_HEAD_REG				(0x0005C)
#define RCB_RING_TX_RING_FBDNUM_REG			(0x00060)
#define RCB_RING_TX_RING_OFFSET_REG			(0x00064)

#define RCB_RING_TX_RING_PKTNUM_RECORD_REG	(0x0006C)
#define RCB_RING_PREFETCH_EN_REG				(0x0007C)
#define RCB_RING_OVERTIME_REG				(0x00080)
#define RCB_RING_RING_IDLE_REG				(0x00084)
#define RCB_RING_RING_CLR_REG				(0x00088)
#define RCB_RING_ASID_REG					(0x0008C)
#define RCB_RING_RX_VM_REG				(0x00090)
#define RCB_RING_T0_BE_RST				(0x00094)
#define RCB_RING_COULD_BE_RST				(0x00098)
#define RCB_RING_WRR_WEIGHT_REG				(0x0009c)
#define RCB_RING_INTMSK_RXWL_REG				(0x000A0)
#define RCB_RING_INTSTS_RX_RING_REG			(0x000A4)
#define RCB_RING_INTMSK_TXWL_REG				(0x000AC)
#define RCB_RING_INTSTS_TX_RING_REG			(0x000B0)
#define RCB_RING_RINT_TX_RING_REG				(0x000B4)
#define RCB_RING_INTMSK_RX_OVERTIME_REG		(0x000B8)
#define RCB_RING_INTSTS_RX_OVERTIME_REG		(0x000BC)
#define RCB_RING_RINT_RX_OVERTIME_REG			(0x000C0)
#define RCB_RING_INTMSK_TX_OVERTIME_REG		(0x000C4)
#define RCB_RING_INTSTS_TX_OVERTIME_REG		(0x000C8)
#define RCB_RING_RINT_TX_OVERTIME_REG			(0x000CC)
#define RCB_RING_INTMSK_CFG_ERR_REG			(0x00200)
#define RCB_RING_RINT_CFG_ERR0_REG			(0x00208)
#define RCB_RING_CHK_RX_EBD_NUM_REG			(0x00210)
#define RCB_RING_CHK_RX_FBD_NUM_REG			(0x00220)

/**RCB rx buffer dscription */
struct rcb_rx_des {
	u64 addr;

	union {
		struct {

			u32 IP_offset:8;
			u32 Buf_Num:2;
			u32 PORT_ID:3;
			u32 DMAC:2;
			u32 VLAN:2;
			u32 L3_ID:4;
			u32 L4_ID:4;
			u32 FE:1;
			u32 FRAG:1;
			u32 VLD:1;
			u32 L2E:1;
			u32 L3E:1;
			u32 L4E:1;
			u32 DROP:1;
		} bits;
		u32 u_32;
	} word2;

	union {
		struct {
			u32 pkt_len:16;
			u32 size:16;
		} bits;
		u32 u_32;
	} word3;

	union {
		struct {
			u32 Reserved:8;
			u32 VLAN_ID:12;
			u32 CFI:1;
			u32 Priority:3;
			u32 asid:8;

		} bits;
		u32 u_32;
	} word4;

	u32 ulaRes[3];
};

struct rcb_tx_des {
	u64 addr;
	union {
		struct {
			u32 asid:8;
			u32 Buf_num:2;
			u32 PORT_ID:3;
			u32 Reserved:3;
			u32 Send_size:16;
		} bits;
		u32 u_32;
	} word2;

	union {
		struct {
			u32 Reserved:8;
			u32 RA:1;
			u32 RI:1;
			u32 L4CS:1;
			u32 L3CS:1;
			u32 FE:1;
			u32 VLD:1;
			u32 IP_offset:8;
			u32 Reserved1:10;
		} bits;
		u32 u_32;
	} word3;

	u32 ulaRes[4];
};

#define RCB_RX_DESC(R, i)		\
	(&(((struct rcb_rx_des *)((R)->desc))[i]))

#define RCB_TX_DESC(R, i)		\
	(&(((struct rcb_tx_des *)((R)->desc))[i]))

static inline void rcb_write_reg(struct rcb_device *rcb_dev, u32 reg, u32 value)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(rcb_dev->vbase);

	writel(value, reg_addr + reg);
}

#define RCB_WRITE_REG(a, reg, value) rcb_write_reg((a), (reg), (value))

static inline u32 rcb_read_reg(struct rcb_device *rcb_dev, u32 reg)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(rcb_dev->vbase);
	u32 value;

	value = readl(reg_addr + reg);
	return value;
}

#define RCB_READ_REG(a, reg) rcb_read_reg((a), (reg))

static inline void rcb_com_write_reg(struct rcb_common_dev *rcb_common,
				     u32 reg, u32 value)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(rcb_common->vbase);

	writel(value, reg_addr + reg);
}

#define RCB_COM_WRITE_REG(a, reg, value) rcb_com_write_reg((a), \
(reg), (value))

static inline u32 rcb_com_read_reg(struct rcb_common_dev *rcb_common, u32 reg)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(rcb_common->vbase);
	u32 value;

	value = readl(reg_addr + reg);
	return value;
}

#define RCB_COM_READ_REG(a, reg) rcb_com_read_reg((a), (reg))

extern int rcb_recv_hw_ex(struct nic_ring_pair * ring, int budget,
	int(* rx_process)(struct napi_struct *, struct sk_buff *));

#endif				/*_RCB_HAL_H */

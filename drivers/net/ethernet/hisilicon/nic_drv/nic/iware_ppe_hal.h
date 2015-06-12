/*******************************************************************************

  Hisilicon network interface controller driver
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

*******************************************************************************/

#ifndef _PPE_HAL_H
#define _PPE_HAL_H

/* #include "hrd_module.h"
#include "hrd_typedef.h" */
#include "iware_ppe_main.h"

#define ETH_STATIC_REG	 1
#define ETH_DUMP_REG	   5

#define PPE_NUM_PER_CHIP 8

/******************************************************************************/
/*					  PhosphorV660 PPE_COMMON register					  */
/******************************************************************************/
#define PPE_COM_CFG_QID_MODE_REG				  (0x0)
#define PPE_COM_CFG_MEM_TIMING_REG				(0x4)
#define PPE_COM_CFG_CMM_TO_BE_RST_REG			 (0x8)
#define PPE_COM_CURR_CMM_CAN_RST_REG			  (0xC)
#define PPE_COM_INTEN_REG						 (0x110)
#define PPE_COM_RINT_REG				(0x114)
#define PPE_COM_INTSTS_REG						(0x118)
#define PPE_COM_HIS_RX_PKT_QID_DROP_CNT_REG	   (0x300)
#define PPE_COM_HIS_RX_PKT_QID_OK_CNT_REG		 (0x600)
#define PPE_COM_HIS_TX_PKT_QID_ERR_CNT_REG		(0x900)
#define PPE_COM_HIS_TX_PKT_QID_OK_CNT_REG		 (0xC00)
#define PPE_COM_HIS_MST_RD_LAT_REG				(0x1000)
#define PPE_COM_HIS_MST_WR_LAT_REG				(0x1020)
#define PPE_COM_HIS_MST_RD_STS_1_REG			  (0x1040)
#define PPE_COM_HIS_MST_RD_STS_2_REG			  (0x1060)
#define PPE_COM_HIS_MST_WR_STS_1_REG			  (0x1080)
#define PPE_COM_HIS_MST_WR_STS_2_REG			  (0x10A0)
#define PPE_COM_HIS_MST_WR_STS_3_REG			  (0x10C0)
#define PPE_COM_HIS_MST_RD_STS_3_REG			  (0x10E0)
#define PPE_COM_CFG_MST_STS_CLR_REG			   (0x1100)
#define PPE_COM_COMMON_CNT_CLR_CE_REG			 (0x1120)

/******************************************************************************/
/*					  PhosphorV660 PPE_TNL_0_5 register					 */
/******************************************************************************/
#define PPE_CFG_TX_FIFO_THRSLD_REG			(0x0)
#define PPE_CFG_RX_FIFO_THRSLD_REG			(0x4)
#define PPE_CFG_RX_FIFO_PAUSE_THRSLD_REG	  (0x8)
#define PPE_CFG_RX_FIFO_SW_BP_THRSLD_REG	  (0xC)
#define PPE_CFG_PAUSE_IDLE_CNT_REG			(0x10)
#define PPE_CFG_BUS_CTRL_REG				  (0x40)
#define PPE_CFG_TNL_TO_BE_RST_REG			 (0x48)
#define PPE_CURR_TNL_CAN_RST_REG			  (0x4C)
#define PPE_CFG_XGE_MODE_REG				  (0x80)
#define PPE_CFG_MAX_FRAME_LEN_REG			 (0x84)
#define PPE_CFG_RX_PKT_MODE_REG			   (0x88)
#define PPE_CFG_RX_VLAN_TAG_REG			   (0x8C)
#define PPE_CFG_TAG_GEN_REG				   (0x90)
#define PPE_CFG_PARSE_TAG_REG				 (0x94)
#define PPE_CFG_PRO_CHECK_EN_REG			  (0x98)
#define PPE_INTEN_REG						 (0x100)
#define PPE_RINT_REG						  (0x104)
#define PPE_INTSTS_REG						(0x108)
#define PPE_CFG_RX_PKT_INT_REG				(0x140)
#define PPE_CFG_HEAT_DECT_TIME0_REG		   (0x144)
#define PPE_CFG_HEAT_DECT_TIME1_REG		   (0x148)
#define PPE_HIS_RX_SW_PKT_CNT_REG			 (0x200)
#define PPE_HIS_RX_WR_BD_OK_PKT_CNT_REG	   (0x204)
#define PPE_HIS_RX_PKT_NO_BUF_CNT_REG		 (0x208)
#define PPE_HIS_TX_BD_CNT_REG				 (0x20C)
#define PPE_HIS_TX_PKT_CNT_REG				(0x210)
#define PPE_HIS_TX_PKT_OK_CNT_REG			 (0x214)
#define PPE_HIS_TX_PKT_EPT_CNT_REG			(0x218)
#define PPE_HIS_TX_PKT_CS_FAIL_CNT_REG		(0x21C)
#define PPE_HIS_RX_APP_BUF_FAIL_CNT_REG	   (0x220)
#define PPE_HIS_RX_APP_BUF_WAIT_CNT_REG	   (0x224)
#define PPE_HIS_RX_PKT_DROP_FUL_CNT_REG	   (0x228)
#define PPE_HIS_RX_PKT_DROP_PRT_CNT_REG	   (0x22C)
#define PPE_TNL_0_5_CNT_CLR_CE_REG			(0x300)
#define PPE_CFG_AXI_DBG_REG				   (0x304)
#define PPE_HIS_PRO_ERR_REG				   (0x308)
#define PPE_HIS_TNL_FIFO_ERR_REG			  (0x30C)
#define PPE_CURR_CFF_DATA_NUM_REG			 (0x310)
#define PPE_CURR_RX_ST_REG					(0x314)
#define PPE_CURR_TX_ST_REG					(0x318)
#define PPE_CURR_RX_FIFO0_REG				 (0x31C)
#define PPE_CURR_RX_FIFO1_REG				 (0x320)
#define PPE_CURR_TX_FIFO0_REG				 (0x324)
#define PPE_CURR_TX_FIFO1_REG				 (0x328)
#define PPE_ECO0_REG						  (0x32C)
#define PPE_ECO1_REG						  (0x330)
#define PPE_ECO2_REG						  (0x334)
#define PPE_CFG_L3_TYPE_VALUE_0_REG		   (0x500)
#define PPE_CFG_L3_TYPE_MASK_0_REG			(0x504)
#define PPE_CFG_L3_TYPE_OFFSET_0_REG		  (0x508)
#define PPE_CFG_L3_TYPE_VALUE_1_REG		   (0x510)
#define PPE_CFG_L3_TYPE_MASK_1_REG			(0x514)
#define PPE_CFG_L3_TYPE_OFFSET_1_REG		  (0x518)
#define PPE_CFG_L3_TYPE_VALUE_2_REG		   (0x520)
#define PPE_CFG_L3_TYPE_MASK_2_REG			(0x524)
#define PPE_CFG_L3_TYPE_OFFSET_2_REG		  (0x528)
#define PPE_CFG_L3_TYPE_VALUE_3_REG		   (0x530)
#define PPE_CFG_L3_TYPE_MASK_3_REG			(0x534)
#define PPE_CFG_L3_TYPE_OFFSET_3_REG		  (0x538)
#define PPE_CFG_L3_TYPE_VALUE_4_REG		   (0x540)
#define PPE_CFG_L3_TYPE_MASK_4_REG			(0x544)
#define PPE_CFG_L3_TYPE_OFFSET_4_REG		  (0x548)
#define PPE_CFG_L3_TYPE_VALUE_5_REG		   (0x550)
#define PPE_CFG_L3_TYPE_MASK_5_REG			(0x554)
#define PPE_CFG_L3_TYPE_OFFSET_5_REG		  (0x558)
#define PPE_CFG_L3_TYPE_VALUE_6_REG		   (0x560)
#define PPE_CFG_L3_TYPE_MASK_6_REG			(0x564)
#define PPE_CFG_L3_TYPE_OFFSET_6_REG		  (0x568)
#define PPE_CFG_L3_TYPE_VALUE_7_REG		   (0x570)
#define PPE_CFG_L3_TYPE_MASK_7_REG			(0x574)
#define PPE_CFG_L3_TYPE_OFFSET_7_REG		  (0x578)
#define PPE_CFG_L3_TYPE_VALUE_8_REG		   (0x580)
#define PPE_CFG_L3_TYPE_MASK_8_REG			(0x584)
#define PPE_CFG_L3_TYPE_OFFSET_8_REG		  (0x588)
#define PPE_CFG_L3_TYPE_VALUE_9_REG		   (0x590)
#define PPE_CFG_L3_TYPE_MASK_9_REG			(0x594)
#define PPE_CFG_L3_TYPE_OFFSET_9_REG		  (0x598)
#define PPE_CFG_L3_TYPE_VALUE_10_REG		  (0x5A0)
#define PPE_CFG_L3_TYPE_MASK_10_REG		   (0x5A4)
#define PPE_CFG_L3_TYPE_OFFSET_10_REG		 (0x5A8)
#define PPE_CFG_L3_TYPE_VALUE_11_REG		  (0x5B0)
#define PPE_CFG_L3_TYPE_MASK_11_REG		   (0x5B4)
#define PPE_CFG_L3_TYPE_OFFSET_11_REG		 (0x5B8)
#define PPE_CFG_L3_TYPE_VALUE_12_REG		  (0x5C0)
#define PPE_CFG_L3_TYPE_MASK_12_REG		   (0x5C4)
#define PPE_CFG_L3_TYPE_OFFSET_12_REG		 (0x5C8)
#define PPE_CFG_L3_TYPE_VALUE_13_REG		  (0x5D0)
#define PPE_CFG_L3_TYPE_MASK_13_REG		   (0x5D4)
#define PPE_CFG_L3_TYPE_OFFSET_13_REG		 (0x5D8)
#define PPE_CFG_L3_TYPE_VALUE_14_REG		  (0x5E0)
#define PPE_CFG_L3_TYPE_MASK_14_REG		   (0x5E4)
#define PPE_CFG_L3_TYPE_OFFSET_14_REG		 (0x5E8)
#define PPE_CFG_L4_TYPE_VALUE_0_REG		   (0x600)
#define PPE_CFG_L4_TYPE_MASK_0_REG			(0x604)
#define PPE_CFG_L4_SUB_ID_0_REG			   (0x608)
#define PPE_CFG_L4_TYPE_VALUE_1_REG		   (0x610)
#define PPE_CFG_L4_TYPE_MASK_1_REG			(0x614)
#define PPE_CFG_L4_SUB_ID_1_REG			   (0x618)
#define PPE_CFG_L4_TYPE_VALUE_2_REG		   (0x620)
#define PPE_CFG_L4_TYPE_MASK_2_REG			(0x624)
#define PPE_CFG_L4_SUB_ID_2_REG			   (0x628)
#define PPE_CFG_L4_TYPE_VALUE_3_REG		   (0x630)
#define PPE_CFG_L4_TYPE_MASK_3_REG			(0x634)
#define PPE_CFG_L4_SUB_ID_3_REG			   (0x638)
#define PPE_CFG_L4_TYPE_VALUE_4_REG		   (0x640)
#define PPE_CFG_L4_TYPE_MASK_4_REG			(0x644)
#define PPE_CFG_L4_SUB_ID_4_REG			   (0x648)
#define PPE_CFG_L4_TYPE_VALUE_5_REG		   (0x650)
#define PPE_CFG_L4_TYPE_MASK_5_REG			(0x654)
#define PPE_CFG_L4_SUB_ID_5_REG			   (0x658)
#define PPE_CFG_L4_TYPE_VALUE_6_REG		   (0x660)
#define PPE_CFG_L4_TYPE_MASK_6_REG			(0x664)
#define PPE_CFG_L4_SUB_ID_6_REG			   (0x668)
#define PPE_CFG_L4_TYPE_VALUE_7_REG		   (0x670)
#define PPE_CFG_L4_TYPE_MASK_7_REG			(0x674)
#define PPE_CFG_L4_SUB_ID_7_REG			   (0x678)
#define PPE_CFG_L4_TYPE_VALUE_8_REG		   (0x680)
#define PPE_CFG_L4_TYPE_MASK_8_REG			(0x684)
#define PPE_CFG_L4_SUB_ID_8_REG			   (0x688)
#define PPE_CFG_L4_TYPE_VALUE_9_REG		   (0x690)
#define PPE_CFG_L4_TYPE_MASK_9_REG			(0x694)
#define PPE_CFG_L4_SUB_ID_9_REG			   (0x698)
#define PPE_CFG_L4_TYPE_VALUE_10_REG		  (0x6A0)
#define PPE_CFG_L4_TYPE_MASK_10_REG		   (0x6A4)
#define PPE_CFG_L4_SUB_ID_10_REG			  (0x6A8)
#define PPE_CFG_L4_TYPE_VALUE_11_REG		  (0x6B0)
#define PPE_CFG_L4_TYPE_MASK_11_REG		   (0x6B4)
#define PPE_CFG_L4_SUB_ID_11_REG			  (0x6B8)
#define PPE_CFG_L4_TYPE_VALUE_12_REG		  (0x6C0)
#define PPE_CFG_L4_TYPE_MASK_12_REG		   (0x6C4)
#define PPE_CFG_L4_SUB_ID_12_REG			  (0x6C8)
#define PPE_CFG_L4_TYPE_VALUE_13_REG		  (0x6D0)
#define PPE_CFG_L4_TYPE_MASK_13_REG		   (0x6D4)
#define PPE_CFG_L4_SUB_ID_13_REG			  (0x6D8)
#define PPE_CFG_L4_TYPE_VALUE_14_REG		  (0x6E0)
#define PPE_CFG_L4_TYPE_MASK_14_REG		   (0x6E4)
#define PPE_CFG_L4_SUB_ID_14_REG			  (0x6E8)
#define PPE_CFG_L5_TYPE_VALUE_0_REG		   (0x700)
#define PPE_CFG_L5_TYPE_MASK_0_REG			(0x704)
#define PPE_CFG_L5_SUB_ID_0_REG			   (0x708)
#define PPE_CFG_L5_TYPE_VALUE_1_REG		   (0x710)
#define PPE_CFG_L5_TYPE_MASK_1_REG			(0x714)
#define PPE_CFG_L5_SUB_ID_1_REG			   (0x718)
#define PPE_CFG_L5_TYPE_VALUE_2_REG		   (0x720)
#define PPE_CFG_L5_TYPE_MASK_2_REG			(0x724)
#define PPE_CFG_L5_SUB_ID_2_REG			   (0x728)
#define PPE_CFG_L5_TYPE_VALUE_3_REG		   (0x730)
#define PPE_CFG_L5_TYPE_MASK_3_REG			(0x734)
#define PPE_CFG_L5_SUB_ID_3_REG			   (0x738)
#define PPE_CFG_L5_TYPE_VALUE_4_REG		   (0x740)
#define PPE_CFG_L5_TYPE_MASK_4_REG			(0x744)
#define PPE_CFG_L5_SUB_ID_4_REG			   (0x748)
#define PPE_CFG_L5_TYPE_VALUE_5_REG		   (0x750)
#define PPE_CFG_L5_TYPE_MASK_5_REG			(0x754)
#define PPE_CFG_L5_SUB_ID_5_REG			   (0x758)
#define PPE_CFG_L5_TYPE_VALUE_6_REG		   (0x760)
#define PPE_CFG_L5_TYPE_MASK_6_REG			(0x764)
#define PPE_CFG_L5_SUB_ID_6_REG			   (0x768)
#define PPE_CFG_L5_TYPE_VALUE_7_REG		   (0x770)
#define PPE_CFG_L5_TYPE_MASK_7_REG			(0x774)
#define PPE_CFG_L5_SUB_ID_7_REG			   (0x778)
#define PPE_CFG_L5_TYPE_VALUE_8_REG		   (0x780)
#define PPE_CFG_L5_TYPE_MASK_8_REG			(0x784)
#define PPE_CFG_L5_SUB_ID_8_REG			   (0x788)
#define PPE_CFG_L5_TYPE_VALUE_9_REG		   (0x790)
#define PPE_CFG_L5_TYPE_MASK_9_REG			(0x794)
#define PPE_CFG_L5_SUB_ID_9_REG			   (0x798)
#define PPE_CFG_L5_TYPE_VALUE_10_REG		  (0x7A0)
#define PPE_CFG_L5_TYPE_MASK_10_REG		   (0x7A4)
#define PPE_CFG_L5_SUB_ID_10_REG			  (0x7A8)
#define PPE_CFG_L5_TYPE_VALUE_11_REG		  (0x7B0)
#define PPE_CFG_L5_TYPE_MASK_11_REG		   (0x7B4)
#define PPE_CFG_L5_SUB_ID_11_REG			  (0x7B8)
#define PPE_CFG_L5_TYPE_VALUE_12_REG		  (0x7C0)
#define PPE_CFG_L5_TYPE_MASK_12_REG		   (0x7C4)
#define PPE_CFG_L5_SUB_ID_12_REG			  (0x7C8)
#define PPE_CFG_L5_TYPE_VALUE_13_REG		  (0x7D0)
#define PPE_CFG_L5_TYPE_MASK_13_REG		   (0x7D4)
#define PPE_CFG_L5_SUB_ID_13_REG			  (0x7D8)
#define PPE_CFG_L5_TYPE_VALUE_14_REG		  (0x7E0)
#define PPE_CFG_L5_TYPE_MASK_14_REG		   (0x7E4)
#define PPE_CFG_L5_SUB_ID_14_REG			  (0x7E8)

enum ppe_qid_mode {
	PPE_QID_MODE0 = 0,	/* fixed queue id mode */
	PPE_QID_MODE1,		/* switch:128VM non switch:6Port/4VM/4TC */
	PPE_QID_MODE2,		/* switch:32VM/4TC non switch:6Port/16VM */
	PPE_QID_MODE3,		/* switch:4TC/8TAG non switch:2Port/64VM */
	PPE_QID_MODE4,		/* switch:8VM/16TAG non switch:2Port/16VM/4TC */
	PPE_QID_MODE5,		/* non switch:6Port/16TAG */
	PPE_QID_MODE6,		/* non switch:6Port/2VM/8TC */
	PPE_QID_MODE7,		/* non switch:2Port/8VM/8TC */
};

enum ppe_port_mode {
	PPE_MODE_GE = 0,
	PPE_MODE_XGE,
};

enum ppe_vlan_tag_sel {
	PPE_VLAN_INNER = 0,
	PPE_VLAN_OUTER
};

enum ppe_tag_gen_mode {
	PPE_PARSE_MODE = 0,
	PPE_FIX_MODE,
};

union ppe_cfg_qid_mode {
	/* Define the struct bits */
	struct {
		unsigned int def_qid:8;
		unsigned int cf_qid_mode:3;
		unsigned int reserved_0:21;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

union ppe_cnt_clr_ce {
    // Define the struct bits
    struct
    {
        unsigned int    cnt_clr_ce            : 1   ; // [0]
        unsigned int    snap_en               : 1   ; // [1]
        unsigned int    reserved_17           : 30  ; // [31..2]
    } bits;

    // Define an unsigned member
    unsigned int    u32;

};

union ppe_tnl_tx_fifo {
	struct {
		unsigned int cf_tx_p_empty_th:10;
		unsigned int cf_tx_p_full_th:10;
		unsigned int cf_tx_stop_rd_th:10;
		unsigned int reserved:2;
	} bits;
	unsigned int u32;
};

union ppe_tnl_rx_fifo {
	struct {
		unsigned int cf_rx_p_empty_th:10;
		unsigned int cf_rx_p_full_th:10;
		unsigned int cf_rx_stop_rd_th:10;
		unsigned int reserved:2;
	} bits;
	unsigned int u32;
};

union ppe_cfg_rx_pkt_mode {
	/* Define the struct bits */
	struct {
		unsigned int eth_offset:7;
		unsigned int cf_parse_mode:2;
		unsigned int cf_l2_skip2:5;
		unsigned int cf_l2_skip1:6;
		unsigned int reserved_15:12;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

union ppe_cfg_rx_vlan_tag {
	/* Define the struct bits */
	struct {
		unsigned int rx_vlan_tag:16;
		unsigned int vlan_id_pri_choose:1;
		unsigned int reserved_16:15;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

union ppe_cfg_tag_gen {
	/* Define the struct bits */
	struct {
		unsigned int def_tag:16;
		unsigned int tag_mode:2;
		unsigned int reserved_17:14;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

union ppe_cfg_parse_tag {
	/* Define the struct bits */
	struct {
		unsigned int tag_msk3_flow:1;
		unsigned int tag_msk3_ip_prot:1;
		unsigned int tag_msk3_ip_dst:1;
		unsigned int tag_msk3_ip_src:1;
		unsigned int tag_msk2_spi:1;
		unsigned int tag_msk2_flow:1;
		unsigned int tag_msk2_ip_prot:1;
		unsigned int tag_msk2_ip_src:1;
		unsigned int tag_msk2_ip_dst:1;
		unsigned int tag_msk1_flow:1;
		unsigned int tag_msk1_ip_prot:1;
		unsigned int tag_msk1_ip_dst:1;
		unsigned int tag_msk1_ip_src:1;
		unsigned int tag_msk1_port_dst:1;
		unsigned int tag_msk1_port_src:1;
		unsigned int reserved_18:17;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

union ppe_tnl_int_en {
	/* Define the struct bits */
	struct {
		unsigned int rx_sram_par_int_msk:1;
		unsigned int tx_sram_par_int_msk:1;
		unsigned int tx_drop_int_msk:1;
		unsigned int rx_fifo_overflow_int_msk:1;
		unsigned int rx_drop_int_msk:1;
		unsigned int rx_no_buf_int_msk:1;
		unsigned int rx_pkt_int_msk:1;
		unsigned int ddr_rw_int_msk:1;
		unsigned int tx_bd_addr_fifo_int_msk:1;
		unsigned int tx_st_heat_int_msk:1;
		unsigned int rx_st_heat_int_msk:1;
		unsigned int pa_st_heat_int_msk:1;
		unsigned int reserved_20:20;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

struct ppe_pkt_mode {
	u32 cf_skip1_len;
	u32 cf_skip2_len;
	u32 cf_parse_mode;
	u32 hig_dsa_mode;
	u32 eth_offset;
};

struct ppe_tag_parse_msk {
	u32 tag_msk1_port_src;
	u32 tag_msk1_port_dst;
	u32 tag_msk1_ip_src;
	u32 tag_msk1_ip_dst;
	u32 tag_msk1_ip_prot;
	u32 tag_msk1_flow;
	u32 tag_msk2_ip_dst;
	u32 tag_msk2_ip_src;
	u32 tag_msk2_ip_prot;
	u32 tag_msk2_flow;
	u32 tag_msk2_spi;
	u32 tag_msk3_ip_src;
	u32 tag_msk3_ip_dst;
	u32 tag_msk3_ip_prot;
	u32 tag_msk3_flow;
};

struct ppe_tnl_int_msk {
	u32 rx_sram_par_int;
	u32 tx_sram_par_int;
	u32 tx_drop_int;
	u32 rx_fifo_overflow_int;
	u32 rx_drop_int;
	u32 rx_no_buf_int;
	u32 rx_pkt_int;
	u32 ddr_rw_int;
	u32 tx_bd_addr_fifo_int;
	u32 tx_st_heat_int;
	u32 rx_st_heat_int;
	u32 pa_st_heat_int;
	u32 Reserved_20;
};

static inline void ppe_write_reg(struct ppe_device *ppe_device, u32 reg, u32 value)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(ppe_device->vbase);

	writel(value, reg_addr + reg);
}

#define PPE_WRITE_REG(a, reg, value) ppe_write_reg((a), (reg), (value))

static inline u32 ppe_read_reg(struct ppe_device *ppe_device, u32 reg)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(ppe_device->vbase);
	u32 value;

	value = readl(reg_addr + reg);
	return value;
}

#define PPE_READ_REG(a, reg) ppe_read_reg((a), (reg))

static inline void ppe_com_write_reg(struct ppe_common_dev *ppe_common, u32 reg,
				     u32 value)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(ppe_common->vbase);

	writel(value, reg_addr + reg);
}

#define PPE_COM_WRITE_REG(a, reg, value) ppe_com_write_reg((a), (reg), (value))

static inline u32 ppe_com_read_reg(struct ppe_common_dev *ppe_common, u32 reg)
{
	u8 __iomem *reg_addr = ACCESS_ONCE(ppe_common->vbase);
	u32 value;

	value = readl(reg_addr + reg);
	return value;
}

#define PPE_COM_READ_REG(a, reg) ppe_com_read_reg((a), (reg))

#endif				/* _PPE_HAL_H */

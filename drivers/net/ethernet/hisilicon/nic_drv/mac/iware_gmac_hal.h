/************************************************************************

  Hisilicon MAC driver
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

#ifndef _IWARE_GMAC_HAL_H
#define _IWARE_GMAC_HAL_H
/*
#include "iware_module.h"
#include "iware_typedef.h" */
#include "iware_log.h"
#include "iware_mac_main.h"

#define ETH_STATIC_REG	 1
#define ETH_DUMP_REG	   5

#define GE_BASE_ADDR	0xc7040000
#define ADDR_PER_PORT	0x4000
#define GE_REG_NUM 		222


#define gmac_reg_write(addr, data) writel((data), (__iomem u32 *)(addr))
#define gmac_reg_read(addr) readl((__iomem u32 *)(addr))

#define GMAC_DUPLEX_TYPE_REG				(0x0008UL)
#define GMAC_FD_FC_TYPE_REG					(0x000CUL)
#define GMAC_FC_TX_TIMER_REG				(0x001CUL)
#define GMAC_FD_FC_ADDR_LOW_REG				(0x0020UL)
#define GMAC_FD_FC_ADDR_HIGH_REG			(0x0024UL)
#define GMAC_IPG_TX_TIMER_REG				(0x0030UL)
#define GMAC_PAUSE_THR_REG					(0x0038UL)
#define GMAC_MAX_FRM_SIZE_REG				(0x003CUL)
#define GMAC_PORT_MODE_REG					(0x0040UL)
#define GMAC_PORT_EN_REG					(0x0044UL)
#define GMAC_PAUSE_EN_REG					(0x0048UL)
#define GMAC_SHORT_RUNTS_THR_REG			(0x0050UL)
#define GMAC_AN_NEG_STATE_REG				(0x0058UL)
#define GMAC_TX_LOCAL_PAGE_REG				(0x005CUL)
#define GMAC_TRANSMIT_CONTROL_REG			(0x0060UL)
#define GMAC_REC_FILT_CONTROL_REG			(0x0064UL)
#define GMAC_PTP_CONFIG_REG					(0x0074UL)
#define GMAC_RX_OCTETS_TOTAL_OK_REG			(0x0080UL)
#define GMAC_RX_OCTETS_BAD_REG				(0x0084UL)
#define GMAC_RX_UC_PKTS_REG					(0x0088UL)
#define GMAC_RX_MC_PKTS_REG					(0x008CUL)
#define GMAC_RX_BC_PKTS_REG					(0x0090UL)
#define GMAC_RX_PKTS_64OCTETS_REG			(0x0094UL)
#define GMAC_RX_PKTS_65TO127OCTETS_REG		(0x0098UL)
#define GMAC_RX_PKTS_128TO255OCTETS_REG		(0x009CUL)
#define GMAC_RX_PKTS_255TO511OCTETS_REG		(0x00A0UL)
#define GMAC_RX_PKTS_512TO1023OCTETS_REG	(0x00A4UL)
#define GMAC_RX_PKTS_1024TO1518OCTETS_REG	(0x00A8UL)
#define GMAC_RX_PKTS_1519TOMAXOCTETS_REG	(0x00ACUL)
#define GMAC_RX_FCS_ERRORS_REG				(0x00B0UL)
#define GMAC_RX_TAGGED_REG					(0x00B4UL)
#define GMAC_RX_DATA_ERR_REG				(0x00B8UL)
#define GMAC_RX_ALIGN_ERRORS_REG			(0x00BCUL)
#define GMAC_RX_LONG_ERRORS_REG				(0x00C0UL)
#define GMAC_RX_JABBER_ERRORS_REG			(0x00C4UL)
#define GMAC_RX_PAUSE_MACCTRL_FRAM_REG		(0x00C8UL)
#define GMAC_RX_UNKNOWN_MACCTRL_FRAM_REG		(0x00CCUL)
#define GMAC_RX_VERY_LONG_ERR_CNT_REG		(0x00D0UL)
#define GMAC_RX_RUNT_ERR_CNT_REG			(0x00D4UL)
#define GMAC_RX_SHORT_ERR_CNT_REG			(0x00D8UL)
#define GMAC_RX_FILT_PKT_CNT_REG			(0x00E8UL)
#define GMAC_RX_OCTETS_TOTAL_FILT_REG		(0x00ECUL)
#define GMAC_OCTETS_TRANSMITTED_OK_REG		(0x0100UL)
#define GMAC_OCTETS_TRANSMITTED_BAD_REG		(0x0104UL)
#define GMAC_TX_UC_PKTS_REG					(0x0108UL)
#define GMAC_TX_MC_PKTS_REG					(0x010CUL)
#define GMAC_TX_BC_PKTS_REG					(0x0110UL)
#define GMAC_TX_PKTS_64OCTETS_REG			(0x0114UL)
#define GMAC_TX_PKTS_65TO127OCTETS_REG		(0x0118UL)
#define GMAC_TX_PKTS_128TO255OCTETS_REG		(0x011CUL)
#define GMAC_TX_PKTS_255TO511OCTETS_REG		(0x0120UL)
#define GMAC_TX_PKTS_512TO1023OCTETS_REG	(0x0124UL)
#define GMAC_TX_PKTS_1024TO1518OCTETS_REG	(0x0128UL)
#define GMAC_TX_PKTS_1519TOMAXOCTETS_REG	(0x012CUL)
#define GMAC_TX_EXCESSIVE_LENGTH_DROP_REG	(0x014CUL)
#define GMAC_TX_UNDERRUN_REG				(0x0150UL)
#define GMAC_TX_TAGGED_REG					(0x0154UL)
#define GMAC_TX_CRC_ERROR_REG				(0x0158UL)
#define GMAC_TX_PAUSE_FRAMES_REG			(0x015CUL)
#define GMAC_LED_MOD_REG					(0x016CUL)
#define SRE_GAMC_RX_MAX_FRAME					(0x0170UL)
#define GMAC_CF_ASYM_CRT_HIGH_REG			(0x0174UL)
#define GMAC_CF_ASYM_CRT_LOW_REG			(0x0178UL)
#define GMAC_TIMSTP_INGRESS_REG				(0x017CUL)
#define GMAC_LINE_LOOP_BACK_REG				(0x01A8UL)
#define GMAC_CF_CRC_STRIP_REG				(0x01B0UL)
#define GMAC_MODE_CHANGE_EN_REG				(0x01B4UL)
#define GMAC_SIXTEEN_BIT_CNTR_REG			(0x01CCUL)
#define GMAC_LD_LINK_COUNTER_REG			(0x01D0UL)
#define GMAC_LOOP_REG					(0x01DCUL)
#define GMAC_RECV_CONTROL_REG				(0x01E0UL)
#define GMAC_VLAN_CODE_REG					(0x01E8UL)
#define GMAC_RX_OVERRUN_CNT_REG				(0x01ECUL)
#define GMAC_RX_LENGTHFIELD_ERR_CNT_REG		(0x01F4UL)
#define GMAC_RX_FAIL_COMMA_CNT_REG			(0x01F8UL)
#define GMAC_STATION_ADDR_LOW_0_REG			(0x0200UL)
#define GMAC_STATION_ADDR_HIGH_0_REG		(0x0204UL)
#define GMAC_STATION_ADDR_LOW_1_REG			(0x0208UL)
#define GMAC_STATION_ADDR_HIGH_1_REG		(0x020CUL)
#define GMAC_STATION_ADDR_LOW_2_REG			(0x0210UL)
#define GMAC_STATION_ADDR_HIGH_2_REG		(0x0214UL)
#define GMAC_STATION_ADDR_LOW_3_REG			(0x0218UL)
#define GMAC_STATION_ADDR_HIGH_3_REG		(0x021CUL)
#define GMAC_STATION_ADDR_LOW_4_REG			(0x0220UL)
#define GMAC_STATION_ADDR_HIGH_4_REG		(0x0224UL)
#define GMAC_STATION_ADDR_LOW_5_REG			(0x0228UL)
#define GMAC_STATION_ADDR_HIGH_5_REG		(0x022CUL)
#define GMAC_STATION_ADDR_LOW_MSK_0_REG		(0x0230UL)
#define GMAC_STATION_ADDR_HIGH_MSK_0_REG	(0x0234UL)
#define GMAC_STATION_ADDR_LOW_MSK_1_REG		(0x0238UL)
#define GMAC_STATION_ADDR_HIGH_MSK_1_REG	(0x023CUL)
#define GMAC_MAC_SKIP_LEN_REG				(0x0240UL)
#define GMAC_DEBUG_RD_DATA_REG				(0x0244UL)
#define GMAC_DEBUG_PKT_PTR_REG				(0x0344UL)
#define GMAC_DEBUG_EN_REG					(0x0348UL)
#define GMAC_INTR_STATE_REG					(0x034CUL)
#define GMAC_INTR_CLR_REG					(0x0350UL)
#define GMAC_INTR_MSK_REG					(0x0354UL)
#define GMAC_SEQ_ID_REG					(0x0358UL)
#define GMAC_STP_63_TO_32_REG				(0x035CUL)
#define GMAC_STP_31_TO_0_REG				(0x0360UL)
#define GMAC_DBG_GRP0_VLD_WORDS_REG			(0x0364UL)
#define GMAC_DBG_GRP1_VLD_WORDS_REG			(0x0368UL)
#define GMAC_DMAC_EN_REG					(0x0370UL)
#define GMAC_DMAC_WITH_MSK_EN_REG			(0x0374UL)
#define GMAC_TX_LOOP_PKT_PRI_REG			(0x0378UL)

union gmac_duplex_type {
	/* Define the struct bits  */
	struct {
		u32 duplex_type:1;	/*[0]  */
		u32 reserved_0:31;	/*[31..1]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_fc_tx_timer  addr :0x1c*/
union gmac_fc_tx_timer {
	/* Define the struct bits  */
	struct {
		u32 fc_tx_timer:16;	/*[15..0]  */
		u32 reserved_0:16;	/*[31..16]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_max_frm_size  addr :0x3c*/
union gmac_max_frm_size {
	/* Define the struct bits  */
	struct {
		u32 max_frm_size:16;	/*[15..0]  */
		u32 reserved_0:16;	/*[31..15]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_port_mode  addr :0x40*/
union gmac_port_mode {
	/* Define the struct bits  */
	struct {
		u32 port_mode:4;	/*[3..0]  */
		u32 rgmii_1000m_delay:1;	/*[4]  */
		u32 mii_tx_edge_sel:1;	/*[5]  */
		u32 fifo_err_auto_rst:1;	/*[6]  */
		u32 dbg_clk_los_msk:1;	/*[7]  */
		u32 reserved_0:24;	/*[31..8]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_port_en  addr :0x44*/
union gmac_port_en {
	/* Define the struct bits  */
	struct {
		u32 Reserved_1:1;	/*[0]  */
		u32 rx_en:1;	/*[1]  */
		u32 tx_en:1;	/*[2]  */
		u32 reserved_0:29;	/*[31..3]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_pause_en  addr :0x48*/
union gmac_pause_en {
	/* Define the struct bits  */
	struct {
		u32 rx_fdfc:1;	/*[0]  */
		u32 tx_fdfc:1;	/*[1]  */
		u32 tx_hdfc:1;	/*[2]  */
		u32 reserved_0:29;	/*[31..3]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_short_runts_thr  addr :0x50*/
union gmac_short_runts_thr {
	/* Define the struct bits  */
	struct {
		u32 short_runts_thr:5;	/*[4..0]  */
		u32 reserved_0:27;	/*[31..5]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_an_neg_state  addr :0x58*/
union gmac_an_neg_state {
	/* Define the struct bits  */
	struct {
		u32 Reserved_4:5;	/*[4..0]  */
		u32 fd:1;	/*[5]  */
		u32 hd:1;	/*[6]  */
		u32 ps:2;	/*[7..8]  */
		u32 Reserved_3:1;	/*[9]  */
		u32 speed:2;	/*[11..10]  */
		u32 rf1_duplex:1;	/*[12]  */
		u32 rf2:1;	/*[13]  */
		u32 Reserved_2:1;	/*[14]  */
		u32 np_link_ok:1;	/*[15]  */
		u32 Reserved_1:4;	/*[19..16]  */
		u32 rx_sync_ok:1;	/*[20] */
		u32 an_done:1;	/*[21] */
		u32 reserved_0:10;	/*[31..22]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_transmit  addr :0x60*/
union gmac_transmit {
	/* Define the struct bits  */
	struct {
		u32 Reserved_1:5;	/*[4..0]  */
		u32 an_enable:1;	/*[5] */
		u32 crc_add:1;	/*[6]  */
		u32 pad_enable:1;	/*[7]  */
		u32 reserved_0:24;	/*[31..8]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_rec_filt_control  addr :0x64*/
union gmac_rec_filt_control {
	/* Define the struct bits  */
	struct {
		u32 uc_match_en:1;	/*[0]  */
		u32 mc_match_en:1;	/*[1]  */
		u32 bc_drop_en:1;	/*[2]  */
		u32 reserved_1:1;	/*[3]  */
		u32 pause_frm_pass:1;	/*[4]  */
		u32 crc_err_pass:1;	/*[5]  */
		u32 reserved_0:26;	/*[31..6]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_led_mod  addr :0x16c*/
union gmac_led_mod {
	/* Define the struct bits  */
	struct {
		u32 led_mod:1;	/*[0]  */
		u32 reserved_0:31;	/*[31..1]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_line_loopback  addr :0x1A8*/
union gmac_line_loopback {
	/* Define the struct bits  */
	struct {
		u32 line_loop_back:1;	/*[0]  */
		u32 reserved_0:31;	/*[31..1]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_cf_crc_strip  addr :0x1B0*/
union gmac_cf_crc_strip {
	/* Define the struct bits  */
	struct {
		u32 cf_crc_strip:1;	/*[0]  */
		u32 Reserved_0:31;	/*[31..1]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_mode_change_en  addr :0x1B4*/
union gmac_mode_change_en {
	/* Define the struct bits  */
	struct {
		u32 mode_change_en:1;	/*[0]  */
		u32 reserved_0:31;	/*[31..1]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_sixteen_bit_cntr  addr :0x1CC*/
union gmac_sixteen_bit_cntr {
	/* Define the struct bits  */
	struct {
		u32 sixteen_bit_cntr:16;	/*[15..0]  */
		u32 reserved_0:16;	/*[31..16] */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_ld_link_counter  addr :0x1D0*/
union gmac_ld_link_counter {
	/* Define the struct bits  */
	struct {
		u32 reserved_1:1;	/*[0]  */
		u32 ld_link_counter:1;	/*[1]  */
		u32 reserved_0:30;	/*[31..2]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_loop_reg  addr :0x1DC*/
union gmac_loop_reg {
	/* Define the struct bits  */
	struct {
		u32 reserved_1:1;	/*[0]  */
		u32 cf_ext_drive_lp:1;	/*[1]  */
		u32 cf2mi_lp_en:1;	/*[2]  */
		u32 reserved_0:29;	/*[31..3]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union union gmac_recv_control  addr :0x1e0*/
union gmac_recv_control {
	/* Define the struct bits  */
	struct {
		u32 reserved_1:3;	/*[2..0]  */
		u32 strip_pad_en:1;	/*[3]  */
		u32 runt_pkt_en:1;	/*[4]  */
		u32 reserved_0:27;	/*[31..5]  */
	} bits;

	/* Define an unsigned member  */
	u32 u32;

};

/* Define the union gmac_tx_loop_pkt_pri  addr :0x0378*/
union gmac_tx_loop_pkt_pri
{
	/* Define the struct bits  */
	struct
	{
		u32	loop_pkt_hig_pri: 1;
		u32	loop_pkt_en: 1;
		u32	reserved: 3;
	} bits;

	/* Define an unsigned member  */
	u32	u32;
};


/***********************************************************
****************************************************************
			  MAC regs union define END
***************************************************************
***************************************************************/
enum port_mode {
	GMAC_10M_MII = 0,
	GMAC_100M_MII,
	GMAC_1000M_GMII,
	GMAC_10M_RGMII,
	GMAC_100M_RGMII,
	GMAC_1000M_RGMII,
	GMAC_10M_SGMII,
	GMAC_100M_SGMII,
	GMAC_1000M_SGMII,
	GMAC_10000M_SGMII	/* 10GE */
};

enum gmac_duplex_mdoe {
	GMAC_HALF_DUPLEX_MODE = 0,
	GMAC_FULL_DUPLEX_MODE
};

struct gmac_port_mode_cfg {
	enum port_mode port_mode;
	u32 max_frm_size;
	u32 short_runts_thr;
	u32 pad_enable;
	u32 crc_add;
	u32 an_enable;	/*auto-nego enable  */
	u32 runt_pkt_en;
	u32 strip_pad_en;
};

#endif				/* __IWARE_GMAC_HAL_H__ */


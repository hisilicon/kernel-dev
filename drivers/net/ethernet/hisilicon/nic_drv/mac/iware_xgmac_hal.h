/*********************************************************************

  Hisilicon mac driver
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

**********************************************************************/


#ifndef _XGMAC_HAL_H
#define _XGMAC_HAL_H

#define ETH_STATIC_REG	 1 /* TBD why nic/xgamc/gmac define the same maloc */
#define ETH_DUMP_REG	   5

#define XGMAC_DUMP_NUM	   124
#define XGMAC_MIB_NUM		68

#define XGMAC_PORT_NUM  8
#define XGMAC_ADDR_PER_PORT 0x4000

union xgmac_port_mode {
	struct {
		u32 tx_mode:3;
		u32 tx_40g_ngsf_mode:1;
		u32 rx_mode:3;
		u32 rx_40g_ngsf_mode:1;
		u32 reserved_9:24;
	} bits;
	u32 u32;
};

union xgmac_mac_enable {
	struct {
		unsigned int tx_enable	 : 1;
		unsigned int rx_enable	 : 1;
		unsigned int Reserved_0	 : 30;
	} bits;

	unsigned int u32;

};

union xgmac_mac_control {
	/* define the struct bits */
	struct {
		unsigned int tx_fcs_en: 1;
		unsigned int tx_pad_en: 1;
		unsigned int reserved_24: 1;
		unsigned int tx_preamble_trans_en: 1;
		unsigned int tx_under_min_err_en: 1;
		unsigned int tx_over_size_truncate_en: 1;
		unsigned int reserved_23: 2;
		unsigned int tx_1588_en: 1;
		unsigned int tx_1731_en: 1;
		unsigned int tx_pfc_en: 1;
		unsigned int reserved_22: 5;
		unsigned int rx_fcs_en: 1;
		unsigned int rx_fcs_strip_en: 1;
		unsigned int reserved_21: 1;
		unsigned int rx_preamble_trans_en: 1;
		unsigned int rx_under_min_err_en: 1;
		unsigned int rx_over_size_truncate_en: 1;
		unsigned int reserved_20: 2;
		unsigned int rx_1588_en: 1;
		unsigned int rx_1731_en: 1;
		unsigned int rx_pfc_en: 1;
		unsigned int reserved_19: 5;
	} bits;

	/* define an unsigned member */
	unsigned int u32;
};

union xgmac_pma_fec_ctrl {
	struct {
		u32	tx_pma_fec_en: 1;
		u32	rx_pma_fec_en: 1;
		u32	err_ind_enable: 1;
		u32	err_ind_over_all_sh: 1;
		u32	reserved_93: 28;
	} bits;
	u32	u32;
};

union xgmac_mac_pause_ctrl {
	/* Define the struct bits */
	struct {
		unsigned int tx_pause_en: 1;
		unsigned int rx_pause_en: 1;
		unsigned int rsp_mode: 1;
		unsigned int tx_force_xon_xoff: 1;
		unsigned int reserved_33: 28;
	} bits;

	/* Define an unsigned member */
	unsigned int	u32;

};




#define XGMAC_INT_STATUS_REG				(0x0)
#define XGMAC_INT_ENABLE_REG				(0x4)
#define XGMAC_INT_SET_REG					(0x8)
#define XGMAC_IERR_U_INFO_REG				(0xC)
#define XGMAC_OVF_INFO_REG					(0x10)
#define XGMAC_OVF_CNT_REG					(0x14)
#define XGMAC_PORT_MODE_REG					(0x40)
#define XGMAC_CLK_ENABLE_REG				(0x44)
#define XGMAC_RESET_REG					(0x48)
#define XGMAC_LINK_CONTROL_REG				(0x50)
#define XGMAC_LINK_STATUS_REG				(0x54)
#define XGMAC_SPARE_REG						(0xC4)
#define XGMAC_SPARE_CNT_REG					(0xC8)
#define XGMAC_MAC_ENABLE_REG				(0x100)
#define XGMAC_MAC_CONTROL_REG				(0x104)
#define XGMAC_MAC_IPG_REG					(0x120)
#define XGMAC_MAC_MSG_CRC_EN_REG			(0x124)
#define XGMAC_MAC_MSG_IMG_REG				(0x128)
#define XGMAC_MAC_MSG_FC_CFG_REG			(0x12C)
#define XGMAC_MAC_MSG_TC_CFG_REG			(0x130)
#define XGMAC_MAC_PAD_SIZE_REG				(0x134)
#define XGMAC_MAC_MIN_PKT_SIZE_REG			(0x138)
#define XGMAC_MAC_MAX_PKT_SIZE_REG			(0x13C)
#define XGMAC_MAC_PAUSE_CTRL_REG			(0x160)
#define XGMAC_MAC_PAUSE_TIME_REG			(0x164)
#define XGMAC_MAC_PAUSE_GAP_REG				(0x168)
#define XGMAC_MAC_PAUSE_LOCAL_MAC_H_REG		(0x16C)
#define XGMAC_MAC_PAUSE_LOCAL_MAC_L_REG		(0x170)
#define XGMAC_MAC_PAUSE_PEER_MAC_H_REG		(0x174)
#define XGMAC_MAC_PAUSE_PEER_MAC_L_REG		(0x178)
#define XGMAC_MAC_PFC_PRI_EN_REG			(0x17C)
#define XGMAC_MAC_1588_CTRL_REG				(0x180)
#define XGMAC_MAC_1588_TX_PORT_DLY_REG		(0x184)
#define XGMAC_MAC_1588_RX_PORT_DLY_REG		(0x188)
#define XGMAC_MAC_1588_ASYM_DLY_REG			(0x18C)
#define XGMAC_MAC_1588_ADJUST_CFG_REG		(0x190)
#define XGMAC_MAC_Y1731_ETH_TYPE_REG		(0x194)
#define XGMAC_MAC_MIB_CONTROL_REG			(0x198)
#define XGMAC_MAC_WAN_RATE_ADJUST_REG		(0x19C)
#define XGMAC_MAC_TX_ERR_MARK_REG			(0x1A0)
#define XGMAC_MAC_TX_LF_RF_CONTROL_REG		(0x1A4)
#define XGMAC_MAC_RX_LF_RF_STATUS_REG		(0x1A8)
#define XGMAC_MAC_TX_RUNT_PKT_CNT_REG		(0x1C0)
#define XGMAC_MAC_RX_RUNT_PKT_CNT_REG		(0x1C4)
#define XGMAC_MAC_RX_PREAM_ERR_PKT_CNT_REG	(0x1C8)
#define XGMAC_MAC_TX_LF_RF_TERM_PKT_CNT_REG	(0x1CC)
#define XGMAC_MAC_TX_SN_MISMATCH_PKT_CNT_REG	(0x1D0)
#define XGMAC_MAC_RX_ERR_MSG_CNT_REG		(0x1D4)
#define XGMAC_MAC_RX_ERR_EFD_CNT_REG		(0x1D8)
#define XGMAC_MAC_ERR_INFO_REG				(0x1DC)
#define XGMAC_MAC_DBG_INFO_REG				(0x1E0)
#define XGMAC_PCS_BASEX_LANE_SWAP_REG		(0x320)
#define XGMAC_PCS_BASEX_DESKEW_THD_REG		(0x324)
#define XGMAC_PCS_BASEX_SCRAMBLE_CONTROL_REG	(0x328)
#define XGMAC_PCS_BASER_SYNC_THD_REG		(0x330)
#define XGMAC_PCS_BASEX_DEC_ERR_CNT__0_REG	(0x340)
#define XGMAC_PCS_BASEX_DEC_ERR_CNT__1_REG	(0x344)
#define XGMAC_PCS_BASEX_DEC_ERR_CNT__2_REG	(0x348)
#define XGMAC_PCS_BASEX_DEC_ERR_CNT__3_REG	(0x34C)
#define XGMAC_PCS_STATUS1_0_REG				(0x404)
#define XGMAC_PCS_STATUS1_1_REG				(0x408)
#define XGMAC_PCS_STATUS1_2_REG				(0x40C)
#define XGMAC_PCS_STATUS1_3_REG				(0x410)
#define XGMAC_PCS_BASEX_STATUS_0_REG		(0x408)
#define XGMAC_PCS_BASEX_STATUS_1_REG		(0x40C)
#define XGMAC_PCS_BASEX_STATUS_2_REG		(0x410)
#define XGMAC_PCS_BASEX_STATUS_3_REG		(0x414)
#define XGMAC_PCS_BASEX_TEST_CONTROL_0_REG	(0x40C)
#define XGMAC_PCS_BASEX_TEST_CONTROL_1_REG	(0x410)
#define XGMAC_PCS_BASEX_TEST_CONTROL_2_REG	(0x414)
#define XGMAC_PCS_BASEX_TEST_CONTROL_3_REG	(0x418)
#define XGMAC_PCS_BASER_STATUS1_0_REG		(0x410)
#define XGMAC_PCS_BASER_STATUS1_1_REG		(0x414)
#define XGMAC_PCS_BASER_STATUS1_2_REG		(0x418)
#define XGMAC_PCS_BASER_STATUS1_3_REG		(0x41C)
#define XGMAC_PCS_BASER_STATUS2_0_REG		(0x414)
#define XGMAC_PCS_BASER_STATUS2_1_REG		(0x418)
#define XGMAC_PCS_BASER_STATUS2_2_REG		(0x41C)
#define XGMAC_PCS_BASER_STATUS2_3_REG		(0x420)
#define XGMAC_PCS_BASER_SEEDA_0_0_REG		(0x420)
#define XGMAC_PCS_BASER_SEEDA_0_1_REG		(0x424)
#define XGMAC_PCS_BASER_SEEDA_0_2_REG		(0x428)
#define XGMAC_PCS_BASER_SEEDA_0_3_REG		(0x42C)
#define XGMAC_PCS_BASER_SEEDA_1_0_REG		(0x424)
#define XGMAC_PCS_BASER_SEEDA_1_1_REG		(0x428)
#define XGMAC_PCS_BASER_SEEDA_1_2_REG		(0x42C)
#define XGMAC_PCS_BASER_SEEDA_1_3_REG		(0x430)
#define XGMAC_PCS_BASER_SEEDB_0_0_REG		(0x428)
#define XGMAC_PCS_BASER_SEEDB_0_1_REG		(0x42C)
#define XGMAC_PCS_BASER_SEEDB_0_2_REG		(0x430)
#define XGMAC_PCS_BASER_SEEDB_0_3_REG		(0x434)
#define XGMAC_PCS_BASER_SEEDB_1_0_REG		(0x42C)
#define XGMAC_PCS_BASER_SEEDB_1_1_REG		(0x430)
#define XGMAC_PCS_BASER_SEEDB_1_2_REG		(0x434)
#define XGMAC_PCS_BASER_SEEDB_1_3_REG		(0x438)
#define XGMAC_PCS_BASER_TEST_CONTROL_0_REG	(0x430)
#define XGMAC_PCS_BASER_TEST_CONTROL_1_REG	(0x434)
#define XGMAC_PCS_BASER_TEST_CONTROL_2_REG	(0x438)
#define XGMAC_PCS_BASER_TEST_CONTROL_3_REG	(0x43C)
#define XGMAC_PCS_BASER_TEST_ERR_CNT_0_REG	(0x434)
#define XGMAC_PCS_BASER_TEST_ERR_CNT_1_REG	(0x438)
#define XGMAC_PCS_BASER_TEST_ERR_CNT_2_REG	(0x43C)
#define XGMAC_PCS_BASER_TEST_ERR_CNT_3_REG	(0x440)
#define XGMAC_PCS_DBG_INFO_0_REG			(0x4C0)
#define XGMAC_PCS_DBG_INFO_1_REG			(0x4C4)
#define XGMAC_PCS_DBG_INFO_2_REG			(0x4C8)
#define XGMAC_PCS_DBG_INFO_3_REG			(0x4CC)
#define XGMAC_PCS_DBG_INFO1_0_REG			(0x4C4)
#define XGMAC_PCS_DBG_INFO1_1_REG			(0x4C8)
#define XGMAC_PCS_DBG_INFO1_2_REG			(0x4CC)
#define XGMAC_PCS_DBG_INFO1_3_REG			(0x4D0)
#define XGMAC_PCS_DBG_INFO2_0_REG			(0x4C8)
#define XGMAC_PCS_DBG_INFO2_1_REG			(0x4CC)
#define XGMAC_PCS_DBG_INFO2_2_REG			(0x4D0)
#define XGMAC_PCS_DBG_INFO2_3_REG			(0x4D4)
#define XGMAC_PCS_DBG_INFO3_0_REG			(0x4CC)
#define XGMAC_PCS_DBG_INFO3_1_REG			(0x4D0)
#define XGMAC_PCS_DBG_INFO3_2_REG			(0x4D4)
#define XGMAC_PCS_DBG_INFO3_3_REG			(0x4D8)
#define XGMAC_PMA_ENABLE_REG				(0x700)
#define XGMAC_PMA_CONTROL_REG				(0x704)
#define XGMAC_PMA_SIGNAL_STATUS_REG			(0x708)
#define XGMAC_PMA_DBG_INFO_REG				(0x70C)
#define XGMAC_PMA_FEC_ABILITY_REG			(0x740)
#define XGMAC_PMA_FEC_CONTROL_REG			(0x744)
#define XGMAC_PMA_FEC_CORR_BLOCK_CNT__REG	(0x750)
#define XGMAC_PMA_FEC_UNCORR_BLOCK_CNT__REG	(0x760)
#define XGMAC_MIB_REG						(0xC00)

#define XGMAC_TX_PKTS_FRAGMENT					(0x0000)
#define XGMAC_TX_PKTS_UNDERSIZE 				(0x0008)
#define XGMAC_TX_PKTS_UNDERMIN    				(0x0010)
#define XGMAC_TX_PKTS_64OCTETS  				(0x0018)
#define XGMAC_TX_PKTS_65TO127OCTETS 				(0x0020)
#define XGMAC_TX_PKTS_128TO255OCTETS 				(0x0028)
#define XGMAC_TX_PKTS_256TO511OCTETS 				(0x0030)
#define XGMAC_TX_PKTS_512TO1023OCTETS 				(0x0038)
#define XGMAC_TX_PKTS_1024TO1518OCTETS 				(0x0040)
#define XGMAC_TX_PKTS_1519TOMAXOCTETS 				(0x0048)
#define XGMAC_TX_PKTS_1519TOMAXOCTETSOK				(0x0050)
#define XGMAC_TX_PKTS_OVERSIZE   				(0x0058)
#define XGMAC_TX_PKTS_JABBER    				(0x0060)
#define XGMAC_TX_GOODPKTS 					(0x0068)
#define XGMAC_TX_GOODOCTETS	  				(0x0070)
#define XGMAC_TX_TOTAL_PKTS					(0x0078)
#define XGMAC_TX_TOTALOCTETS 					(0x0080)
#define XGMAC_TX_UNICASTPKTS 					(0x0088)
#define XGMAC_TX_MULTICASTPKTS	 				(0x0090)
#define XGMAC_TX_BROADCASTPKTS 					(0x0098)
#define XGMAC_TX_PRI0PAUSEPKTS 					(0x00a0)
#define XGMAC_TX_PRI1PAUSEPKTS 					(0x00a8)
#define XGMAC_TX_PRI2PAUSEPKTS 					(0x00b0)
#define XGMAC_TX_PRI3PAUSEPKTS 					(0x00b8)
#define XGMAC_TX_PRI4PAUSEPKTS 					(0x00c0)
#define XGMAC_TX_PRI5PAUSEPKTS 					(0x00c8)
#define XGMAC_TX_PRI6PAUSEPKTS 					(0x00d0)
#define XGMAC_TX_PRI7PAUSEPKTS 					(0x00d8)
#define XGMAC_TX_MACCTRLPKTS  					(0x00e0)
#define XGMAC_TX_1731PKTS  					(0x00e8)
#define XGMAC_TX_1588PKTS  					(0x00f0)
#define XGMAC_RX_FROMAPPGOODPKTS 				(0x00f8)
#define XGMAC_RX_FROMAPPBADPKTS 				(0x0100)
#define XGMAC_TX_ERRALLPKTS  					(0x0108)

#define XGMAC_RX_PKTS_FRAGMENT  				(0x0110)
#define XGMAC_RX_PKTSUNDERSIZE 					(0x0118)
#define XGMAC_RX_PKTS_UNDERMIN  				(0x0120)
#define XGMAC_RX_PKTS_64OCTETS 					(0x0128)
#define XGMAC_RX_PKTS_65TO127OCTETS				(0x0130)
#define XGMAC_RX_PKTS_128TO255OCTETS				(0x0138)
#define XGMAC_RX_PKTS_256TO511OCTETS				(0x0140)
#define XGMAC_RX_PKTS_512TO1023OCTETS				(0x0148)
#define XGMAC_RX_PKTS_1024TO1518OCTETS				(0x0150)
#define XGMAC_RX_PKTS_1519TOMAXOCTETS  				(0x0158)
#define XGMAC_RX_PKTS_1519TOMAXOCTETSOK  			(0x0160)
#define XGMAC_RX_PKTS_OVERSIZE 					(0x0168)
#define XGMAC_RX_PKTS_JABBER  	 				(0x0170)
#define XGMAC_RX_GOODPKTS  					(0x0178)
#define XGMAC_RX_GOODOCTETS					(0x0180)
#define XGMAC_RX_TOTAL_PKTS 					(0x0188)
#define XGMAC_RX_TOTALOCTETS					(0x0190)
#define XGMAC_RX_UNICASTPKTS 					(0x0198)
#define XGMAC_RX_MULTICASTPKTS 					(0x01a0)
#define XGMAC_RX_BROADCASTPKTS  				(0x01a8)
#define XGMAC_RX_PRI0PAUSEPKTS 					(0x01b0)
#define XGMAC_RX_PRI1PAUSEPKTS 					(0x01b8)
#define XGMAC_RX_PRI2PAUSEPKTS 					(0x01c0)
#define XGMAC_RX_PRI3PAUSEPKTS 					(0x01c8)
#define XGMAC_RX_PRI4PAUSEPKTS 					(0x01d0)
#define XGMAC_RX_PRI5PAUSEPKTS 					(0x01d8)
#define XGMAC_RX_PRI6PAUSEPKTS 					(0x01e0)
#define XGMAC_RX_PRI7PAUSEPKTS 					(0x01e8)
#define XGMAC_RX_MACCTRLPKTS  					(0x01f0)
#define XGMAC_TX_SENDAPPGOODPKTS				(0x01f8)
#define XGMAC_TX_SENDAPPBADPKTS					(0x0200)
#define XGMAC_RX_1731PKTS  					(0x0208)
#define XGMAC_RX_SYMBOLERRPKTS 					(0x0210)
#define XGMAC_RX_FCSERRPKTS 					(0x0218)

#endif


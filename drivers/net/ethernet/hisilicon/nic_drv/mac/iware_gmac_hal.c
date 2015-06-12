/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe484ZPOIoFs3QaW/IUusxZ6rbapjpbLJCnxVejpQxly2hBFCfzdkEWAk5lEZGp83Tw2mHl
poF6AwgseUmKZ078hFcqDznHU28EVb6qxU1S1XkIUPVfdunh3+iLLYjfW+oWvw==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
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
#include "iware_log.h"
#include "osal_api.h"
/* #include "iware_typedef.h"
#include "iware_module.h" */
#include "iware_dsaf_main.h"
#include "iware_error.h"
#include "iware_mac_hal.h"
#include "iware_gmac_hal.h"
/*#include "SRE_memmap.ph"*/
#include "hrd_crg_api.h"

#define MAC_PORT_NUM 8
static const struct mac_stats_string g_gmac_rx_stats_string[] =
{
	{"RX paket bytes: %llx\n", MAC_STATS_FIELD_OFF(rx_good_bytes)},
	{"RX error pakets bytes: %llx\n", MAC_STATS_FIELD_OFF(rx_bad_bytes)},
	{"RX unicast packets: %llx\n", MAC_STATS_FIELD_OFF(rx_uc_pkts)},
	{"RX multicast packets: %llx\n", MAC_STATS_FIELD_OFF(rx_mc_pkts)},
	{"RX broadcast packets: %llx\n", MAC_STATS_FIELD_OFF(rx_bc_pkts)},
	{"RX frames size 64 bytes: %llx\n", MAC_STATS_FIELD_OFF(rx_64bytes)},
	{"RX frames size 65-127 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(rx_65to127)},
	{"RX frames size 128-255 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(rx_128to255)},
	{"RX frames size 256-511 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(rx_256to511)},
	{"RX frames size 511-1023 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(rx_512to1023)},
	{"RX frames size 1024-1518 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(rx_1024to1518)},
	{"RX frames size bigger than 1518 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(rx_1519tomax)},
	{"RX packet crc error: %llx\n", MAC_STATS_FIELD_OFF(rx_fcs_err)},
	{"RX vlan packets: %llx\n", MAC_STATS_FIELD_OFF(rx_vlan_pkts)},
	{"RX packet data error: %llx\n", MAC_STATS_FIELD_OFF(rx_data_err)},
	{"RX frame align error: %llx\n", MAC_STATS_FIELD_OFF(rx_align_err)},
	{"RX long frames: %llx\n", MAC_STATS_FIELD_OFF(rx_oversize)},
	{"RX long frames error: %llx\n", MAC_STATS_FIELD_OFF(rx_jabber_err)},
	{"RX flow control frames: %llx\n", MAC_STATS_FIELD_OFF(rx_pfc_tc0)},
	{"RX unknown mac control frames: %llx\n",
	MAC_STATS_FIELD_OFF(rx_unknown_ctrl)},
	{"RX very long frames error: %llx\n", MAC_STATS_FIELD_OFF(rx_long_err)},
	{"RX short frames: %llx\n", MAC_STATS_FIELD_OFF(rx_minto64)},
	{"RX super short frames: %llx\n", MAC_STATS_FIELD_OFF(rx_under_min)},
	{"RX filter total bytes: %llx\n", MAC_STATS_FIELD_OFF(rx_filter_bytes)},
	{"RX filter packets: %llx\n", MAC_STATS_FIELD_OFF(rx_filter_pkts)},
	{"RX fifo overrun: %llx\n", MAC_STATS_FIELD_OFF(rx_fifo_overrun_err)},
	{"RX length err: %llx\n", MAC_STATS_FIELD_OFF(rx_len_err)},
	{"RX fail comma err: %llx\n", MAC_STATS_FIELD_OFF(rx_comma_err)}
};
static const struct mac_stats_string g_gmac_tx_stats_string[]=
{
	{"TX packet bytes: %llx\n", MAC_STATS_FIELD_OFF(tx_good_bytes)},
	{"TX bad packets bytes: %llx\n", MAC_STATS_FIELD_OFF(tx_bad_bytes)},
	{"TX unicast packets: %llx\n", MAC_STATS_FIELD_OFF(tx_uc_pkts)},
	{"TX multicast packets: %llx\n", MAC_STATS_FIELD_OFF(tx_mc_pkts)},
	{"TX broadcast packets: %llx\n", MAC_STATS_FIELD_OFF(tx_bc_pkts)},
	{"TX frames size 64 bytes: %llx\n", MAC_STATS_FIELD_OFF(tx_64bytes)},
	{"TX frames size 65-127 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(tx_65to127)},
	{"TX frames size 128-255 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(tx_128to255)},
	{"TX frames size 256-511 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(tx_256to511)},
	{"TX frames size 511-1023 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(tx_512to1023)},
	{"TX frames size 1024-1518 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(tx_1024to1518)},
	{"TX frames size bigger than 1518 bytes: %llx\n",
		MAC_STATS_FIELD_OFF(tx_1519tomax)},
	{"TX excessive length frame failed drop: %llx\n",
		MAC_STATS_FIELD_OFF(tx_jabber_err)},
	{"TX failed counter: %llx\n", MAC_STATS_FIELD_OFF(tx_underrun_err)},
	{"TX vlan packets counter: %llx\n", MAC_STATS_FIELD_OFF(tx_vlan)},
	{"TX packet crc error: %llx\n", MAC_STATS_FIELD_OFF(tx_crc_err)},
	{"TX flow control frames: %llx\n", MAC_STATS_FIELD_OFF(tx_pfc_tc0)}
};

static int gmac_enable(void *mac_drv, enum mac_commom_mode mode)
{
	u32 read_bak = 0;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	u8 mac_id = drv->mac_id;
	union gmac_port_en unporten;

	log_dbg(drv->dev, "addr %#llx dsaf%d mac%d\n",
		(u64)drv->vaddr, drv->chip_id, mac_id);

	/*enable GE rX/tX */
	if ((MAC_COMM_MODE_TX == mode) || (MAC_COMM_MODE_RX_AND_TX == mode)) {
		unporten.u32 =
		    gmac_reg_read((u64) (drv->vaddr) + GMAC_PORT_EN_REG);
		unporten.bits.tx_en = 1;
		/*osal_printf("unporten.u32=0x%08x\n",unporten.u32); */
		gmac_reg_write((u64) (drv->vaddr) + GMAC_PORT_EN_REG,
			       unporten.u32);

		read_bak = gmac_reg_read((u64) (drv->vaddr) + GMAC_PORT_EN_REG);
		log_dbg(drv->dev,
			"gmac_enable addr %#llx val 0x%x dsaf%d mac%d\n",
			(u64) (drv->vaddr) + GMAC_PORT_EN_REG, read_bak,
			drv->chip_id, mac_id);
	}

	if ((MAC_COMM_MODE_RX == mode) || (MAC_COMM_MODE_RX_AND_TX == mode)) {
		unporten.u32 =
		    gmac_reg_read((u64) (drv->vaddr) + GMAC_PORT_EN_REG);
		unporten.bits.rx_en = 1;
		gmac_reg_write((u64) (drv->vaddr) + GMAC_PORT_EN_REG,
			       unporten.u32);

		read_bak = gmac_reg_read((u64) (drv->vaddr) + GMAC_PORT_EN_REG);
		log_dbg(drv->dev,
			"gmac_enable addr %#llx val 0x%x dsaf%d mac%d\n",
			(u64) (drv->vaddr) + GMAC_PORT_EN_REG, read_bak,
			drv->chip_id, mac_id);
	}

	return 0;
}

static int gmac_disable(void *mac_drv, enum mac_commom_mode mode)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	union gmac_port_en unporten;

	/*disable GE rX/tX */
	if ((MAC_COMM_MODE_TX == mode) || (MAC_COMM_MODE_RX_AND_TX == mode)) {
		unporten.u32 =
		    gmac_reg_read((u64) (drv->vaddr) + GMAC_PORT_EN_REG);
		unporten.bits.tx_en = 0;
		gmac_reg_write((u64) (drv->vaddr) + GMAC_PORT_EN_REG,
			       unporten.u32);
	}

	if ((MAC_COMM_MODE_RX == mode) || (MAC_COMM_MODE_RX_AND_TX == mode)) {
		unporten.u32 =
		    gmac_reg_read((u64) (drv->vaddr) + GMAC_PORT_EN_REG);
		unporten.bits.rx_en = 0;
		gmac_reg_write((u64) (drv->vaddr) + GMAC_PORT_EN_REG,
			       unporten.u32);
	}

	return 0;
}

/**
*gmac_get_en - get port enable
*@mac_drv:mac device
*@rx:rx enable
*@tx:tx enable
*/
static int gmac_get_en(void *mac_drv, u32 *rx, u32 *tx)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	union gmac_port_en unporten;

	/*get GE rX/tX */
	unporten.u32 = gmac_reg_read((u64) (drv->vaddr) + GMAC_PORT_EN_REG);
	*tx = unporten.bits.tx_en;
	*rx = unporten.bits.rx_en;

	return 0;
}

int gmac_get_an_link(void *mac_drv, u32 *an_link)
{

	union gmac_an_neg_state an_state;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	an_state.u32 = gmac_reg_read((u64)(drv->vaddr) + GMAC_AN_NEG_STATE_REG);

	if ( 1 != an_state.bits.an_done)
	{
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	*an_link = an_state.bits.np_link_ok;

	return 0;
}

static int gmac_free(void *mac_drv)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	u32 mac_id = drv->chip_id * MAC_PORT_NUM + drv->mac_id;

	(void)HRD_Dsaf_GeSrstByPort(mac_id, 0);
	log_dbg(drv->dev, "gmac_free() free ge port %d.\n", mac_id);
	osal_kfree(mac_drv);

	return 0;
}

static int gmac_set_tx_auto_pause_frames(void *mac_drv, u16 newval)
{
	union gmac_fc_tx_timer tx_timer;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	tx_timer.u32 =
	    gmac_reg_read((u64)(drv->vaddr) + GMAC_FC_TX_TIMER_REG);
	tx_timer.bits.fc_tx_timer = newval;
	gmac_reg_write((u64)(drv->vaddr) + GMAC_FC_TX_TIMER_REG,
		       tx_timer.u32);

	return 0;
}

static int gmac_get_tx_auto_pause_frames(void *mac_drv, u16 *newval)
{
	union gmac_fc_tx_timer tx_timer;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	tx_timer.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_FC_TX_TIMER_REG);
	*newval = tx_timer.bits.fc_tx_timer;

	return 0;
}

static int gmac_set_rx_auto_pause_frames(void *mac_drv, u32 newval)
{
	union gmac_pause_en pause_en;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	pause_en.u32 = gmac_reg_read((u64) (drv->vaddr) + GMAC_PAUSE_EN_REG);
	pause_en.bits.rx_fdfc = (newval & 1);
	gmac_reg_write((u64) (drv->vaddr) + GMAC_PAUSE_EN_REG, pause_en.u32);
	return 0;
}

static int gmac_config_max_frame_length(void *mac_drv, u16 newval)
{
	union gmac_max_frm_size unmaxfrmsize;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	log_dbg(drv->dev, "addr %#llx dsaf%d mac%d\n",
		(u64) drv->vaddr, drv->chip_id, drv->mac_id);

	unmaxfrmsize.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_MAX_FRM_SIZE_REG);
	unmaxfrmsize.bits.max_frm_size = newval;
	gmac_reg_write((u64) (drv->vaddr) + GMAC_MAX_FRM_SIZE_REG,
		       unmaxfrmsize.u32);

	unmaxfrmsize.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + SRE_GAMC_RX_MAX_FRAME);
	unmaxfrmsize.bits.max_frm_size = newval;
	gmac_reg_write((u64) (drv->vaddr) + SRE_GAMC_RX_MAX_FRAME,
		       unmaxfrmsize.u32);

	return 0;
}


static int gmac_config_an_mode(void *mac_drv, u8 newval)
{
	union gmac_transmit untransctl;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	untransctl.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_TRANSMIT_CONTROL_REG);
	untransctl.bits.an_enable = (newval & 1);
	gmac_reg_write((u64) (drv->vaddr) + GMAC_TRANSMIT_CONTROL_REG,
		       untransctl.u32);

	return 0;
}

static int gmac_tx_loop_pkt_dis(void *mac_drv)
{
	union gmac_tx_loop_pkt_pri tx_loop_pkt_pri;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	tx_loop_pkt_pri.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_TX_LOOP_PKT_PRI_REG);

	tx_loop_pkt_pri.bits.loop_pkt_en = 1;
	tx_loop_pkt_pri.bits.loop_pkt_hig_pri = 0;

	gmac_reg_write((u64) (drv->vaddr) + GMAC_TX_LOOP_PKT_PRI_REG,
		       tx_loop_pkt_pri.u32);

	return 0;
}

static int gmac_set_duplex_type(void *mac_drv, u8 newval)
{
	union gmac_duplex_type duplex_type;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	duplex_type.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_DUPLEX_TYPE_REG);
	duplex_type.bits.duplex_type = (newval & 1);
	gmac_reg_write((u64) (drv->vaddr) + GMAC_DUPLEX_TYPE_REG,
		       duplex_type.u32);

	return 0;
}

static int gmac_get_duplex_type(void *mac_drv,
				    enum gmac_duplex_mdoe *duplex_mode)
{
	union gmac_duplex_type duplex_type;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	duplex_type.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_DUPLEX_TYPE_REG);
	*duplex_mode = (enum gmac_duplex_mdoe)duplex_type.bits.duplex_type;

	return 0;
}

static int gmac_get_portmode(void *mac_drv, enum port_mode *port_mode)
{
	union gmac_port_mode port_mode_reg;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port_mode_reg.u32
		= gmac_reg_read((u64)(drv->vaddr) + GMAC_PORT_MODE_REG);
	*port_mode = (enum port_mode)port_mode_reg.bits.port_mode;
	return 0;
}

static int gmac_port_mode_get(void *mac_drv, struct gmac_port_mode_cfg *port_mode)
{
	union gmac_max_frm_size max_frm_size;
	union gmac_short_runts_thr short_frm_thr;
	union gmac_transmit tx_ctrl;
	union gmac_recv_control recv_ctrl;
	union gmac_port_mode port_mode_reg;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port_mode_reg.u32 = gmac_reg_read((u64) (drv->vaddr) + GMAC_PORT_MODE_REG);
	port_mode->port_mode = (enum port_mode)port_mode_reg.bits.port_mode;

	max_frm_size.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_MAX_FRM_SIZE_REG);
	short_frm_thr.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_SHORT_RUNTS_THR_REG);
	tx_ctrl.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_TRANSMIT_CONTROL_REG);
	recv_ctrl.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_RECV_CONTROL_REG);

	port_mode->max_frm_size = max_frm_size.bits.max_frm_size;
	port_mode->short_runts_thr = short_frm_thr.bits.short_runts_thr;

	port_mode->pad_enable = tx_ctrl.bits.pad_enable;
	port_mode->crc_add = tx_ctrl.bits.crc_add;
	port_mode->an_enable = tx_ctrl.bits.an_enable;

	port_mode->runt_pkt_en = recv_ctrl.bits.runt_pkt_en;
	port_mode->strip_pad_en = recv_ctrl.bits.strip_pad_en;

	return 0;
}

static int gmac_pause_frm_cfg(void *mac_drv,
				 u32 rx_pause_en, u32 tx_pause_en)
{
	union gmac_pause_en pause_en;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	pause_en.u32 = gmac_reg_read((u64) (drv->vaddr) + GMAC_PAUSE_EN_REG);
	pause_en.bits.rx_fdfc = (rx_pause_en & 1);
	pause_en.bits.tx_fdfc = (tx_pause_en & 1);
	gmac_reg_write((u64) (drv->vaddr) + GMAC_PAUSE_EN_REG, pause_en.u32);

	return 0;
}

static int gmac_get_pausefrm_cfg(void *mac_drv,
				     u32 *rx_pause_en, u32 *tx_pause_en)
{
	union gmac_pause_en pause_en;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	pause_en.u32 = gmac_reg_read((u64) (drv->vaddr) + GMAC_PAUSE_EN_REG);

	*rx_pause_en = pause_en.bits.rx_fdfc;
	*tx_pause_en = pause_en.bits.tx_fdfc;
	return 0;
}

static int gmac_adjust_link(void *mac_drv, enum mac_speed speed,
			    u32 full_duplex)
{
	union gmac_duplex_type duplex_type;
	union gmac_port_mode port_mode;
	union gmac_mode_change_en mode_change_en;
	union gmac_transmit tx_ctrl;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	duplex_type.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_DUPLEX_TYPE_REG);
	duplex_type.bits.duplex_type = full_duplex;
	gmac_reg_write((u64) (drv->vaddr) + GMAC_DUPLEX_TYPE_REG,
		       duplex_type.u32);

	port_mode.u32 = gmac_reg_read((u64) (drv->vaddr) + GMAC_PORT_MODE_REG);
	switch (speed) {
	case MAC_SPEED_10:
		port_mode.bits.port_mode = 0x6;	/*10M SGMII */
		break;
	case MAC_SPEED_100:
		port_mode.bits.port_mode = 0x7;	/*100M SGMII */
		break;
	case MAC_SPEED_1000:
		port_mode.bits.port_mode = 0x8;	/*1000M SGMII */
		break;
	default:
		log_err(drv->dev, "gmac_adjust_link fail, speed 0x%x dsaf%d mac%d\n",
			speed, drv->chip_id, drv->mac_id);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}
	gmac_reg_write((u64) (drv->vaddr) + GMAC_PORT_MODE_REG, port_mode.u32);

	tx_ctrl.u32 = gmac_reg_read((u64)(drv->vaddr) + GMAC_TRANSMIT_CONTROL_REG);
	tx_ctrl.bits.pad_enable = 1;
	tx_ctrl.bits.crc_add = 1;
	gmac_reg_write((u64)(drv->vaddr) + GMAC_TRANSMIT_CONTROL_REG,
		tx_ctrl.u32);

	mode_change_en.u32 =
	    gmac_reg_read((u64) (drv->vaddr) + GMAC_MODE_CHANGE_EN_REG);
	mode_change_en.bits.mode_change_en = 1;
	gmac_reg_write((u64) (drv->vaddr) + GMAC_MODE_CHANGE_EN_REG,
		       mode_change_en.u32);

	return 0;
}

static int gmac_init(void *mac_drv)
{
	u32 port;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port = drv->chip_id * MAC_PORT_NUM + drv->mac_id;

	(void)HRD_Dsaf_GeSrstByPort(port, 0);
	osal_mdelay(10);
	(void)HRD_Dsaf_GeSrstByPort(port, 1);
	log_dbg(drv->dev, "gmac_init() init ge port %d.\n", port);

	(void)gmac_disable(mac_drv, MAC_COMM_MODE_RX_AND_TX);
	(void)gmac_tx_loop_pkt_dis(mac_drv);

	return 0;
}

static void gmac_update_stats(struct mac_device *mac_dev)
{
	struct mac_priv *priv = NULL;
	struct mac_driver *drv = NULL;
	struct mac_hw_stats *hw_stats = NULL;

	priv = mac_dev_priv(mac_dev);
	drv = (struct mac_driver *)(priv->mac);
	hw_stats = &mac_dev->hw_stats;

	/* RX */
	hw_stats->rx_good_bytes
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_OCTETS_TOTAL_OK_REG);
	hw_stats->rx_bad_bytes
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_OCTETS_BAD_REG);
	hw_stats->rx_uc_pkts += gmac_reg_read((u64)drv->vaddr + GMAC_RX_UC_PKTS_REG);
	hw_stats->rx_mc_pkts += gmac_reg_read((u64)drv->vaddr + GMAC_RX_MC_PKTS_REG);
	hw_stats->rx_bc_pkts += gmac_reg_read((u64)drv->vaddr + GMAC_RX_BC_PKTS_REG);
	hw_stats->rx_64bytes
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_64OCTETS_REG);
	hw_stats->rx_65to127
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_65TO127OCTETS_REG);
	hw_stats->rx_128to255
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_128TO255OCTETS_REG);
	hw_stats->rx_256to511
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_255TO511OCTETS_REG);
	hw_stats->rx_512to1023
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_512TO1023OCTETS_REG);
	hw_stats->rx_1024to1518
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_1024TO1518OCTETS_REG);
	hw_stats->rx_1519tomax
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_1519TOMAXOCTETS_REG);
	hw_stats->rx_fcs_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_FCS_ERRORS_REG);
	hw_stats->rx_vlan_pkts
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_TAGGED_REG);
	hw_stats->rx_data_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_DATA_ERR_REG);
	hw_stats->rx_align_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_ALIGN_ERRORS_REG);
	hw_stats->rx_oversize
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_LONG_ERRORS_REG);
	hw_stats->rx_jabber_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_JABBER_ERRORS_REG);
	hw_stats->rx_pfc_tc0
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_PAUSE_MACCTRL_FRAM_REG);
	hw_stats->rx_unknown_ctrl
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_UNKNOWN_MACCTRL_FRAM_REG);
	hw_stats->rx_long_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_VERY_LONG_ERR_CNT_REG);
	hw_stats->rx_minto64
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_RUNT_ERR_CNT_REG);
	hw_stats->rx_under_min
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_SHORT_ERR_CNT_REG);
	hw_stats->rx_filter_pkts
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_FILT_PKT_CNT_REG);
	hw_stats->rx_filter_bytes
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_OCTETS_TOTAL_FILT_REG);
	hw_stats->rx_fifo_overrun_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_OVERRUN_CNT_REG);
	hw_stats->rx_len_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_LENGTHFIELD_ERR_CNT_REG);
	hw_stats->rx_comma_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_RX_FAIL_COMMA_CNT_REG);

	/* TX */
	hw_stats->tx_good_bytes
		+= gmac_reg_read((u64)drv->vaddr + GMAC_OCTETS_TRANSMITTED_OK_REG);
	hw_stats->tx_bad_bytes
		+= gmac_reg_read((u64)drv->vaddr + GMAC_OCTETS_TRANSMITTED_BAD_REG);
	hw_stats->tx_uc_pkts += gmac_reg_read((u64)drv->vaddr + GMAC_TX_UC_PKTS_REG);
	hw_stats->tx_mc_pkts += gmac_reg_read((u64)drv->vaddr + GMAC_TX_MC_PKTS_REG);
	hw_stats->tx_bc_pkts += gmac_reg_read((u64)drv->vaddr + GMAC_TX_BC_PKTS_REG);
	hw_stats->tx_64bytes
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_64OCTETS_REG);
	hw_stats->tx_65to127
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_65TO127OCTETS_REG);
	hw_stats->tx_128to255
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_128TO255OCTETS_REG);
	hw_stats->tx_256to511
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_255TO511OCTETS_REG);
	hw_stats->tx_512to1023
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_512TO1023OCTETS_REG);
	hw_stats->tx_1024to1518
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_1024TO1518OCTETS_REG);
	hw_stats->tx_1519tomax
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_1519TOMAXOCTETS_REG);
	hw_stats->tx_jabber_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_EXCESSIVE_LENGTH_DROP_REG);
	hw_stats->tx_underrun_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_UNDERRUN_REG);
	hw_stats->tx_vlan += gmac_reg_read((u64)drv->vaddr + GMAC_TX_TAGGED_REG);
	hw_stats->tx_crc_err
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_CRC_ERROR_REG);
	hw_stats->tx_pfc_tc0
		+= gmac_reg_read((u64)drv->vaddr + GMAC_TX_PAUSE_FRAMES_REG);
}

static int gmac_get_statistics(struct mac_device *mac_dev,
	struct mac_statistics *statistics, u8 reset_count)
{
	u64 stats_val;
	u32 stats_num;
	u32 i;

	struct mac_hw_stats *hw_stats = NULL;

	hw_stats = &mac_dev->hw_stats;

	gmac_update_stats(mac_dev);

	osal_printf("\n*************************");
	osal_printf("***RX COUNTERS****************************\n");

	stats_num = ARRAY_SIZE(g_gmac_rx_stats_string);
	for (i = 0; i < stats_num; i++) {
		stats_val = MAC_STATS_READ(hw_stats,
			g_gmac_rx_stats_string[i].offset);
		osal_printf(g_gmac_rx_stats_string[i].desc, stats_val);
	}

	osal_printf("\n***********************");
	osal_printf("*****TX COUNTERS*******************\n");
	stats_num = ARRAY_SIZE(g_gmac_tx_stats_string);
	for (i = 0; i < stats_num; i++) {
		stats_val = MAC_STATS_READ(hw_stats,
			g_gmac_tx_stats_string[i].offset);
		osal_printf(g_gmac_tx_stats_string[i].desc, stats_val);
	}

	return 0;
}

static int gmac_reset(void *mac_drv, u32 wait)
{
	u8 port;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port = drv->chip_id * MAC_PORT_NUM + drv->mac_id;
	osal_mdelay(10);

	(void)HRD_Dsaf_GeSrstByPort(port, 0);

	osal_mdelay(10);

	(void)HRD_Dsaf_GeSrstByPort(port, 1);

	osal_mdelay(10);

	return 0;
}

static int gmac_set_mac_addr(void *mac_drv, char *mac_addr)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	/* TBD magic num */
	if (drv->mac_id >= 6) {
		u32 high_val = mac_addr[1] | (mac_addr[0] << 8);

		u32 low_val = mac_addr[5] | (mac_addr[4] << 8)
			| (mac_addr[3] <<	16) | (mac_addr[2] << 24);
		gmac_reg_write((u64) (drv->vaddr) +
			       GMAC_STATION_ADDR_LOW_2_REG, low_val);
		gmac_reg_write((u64) (drv->vaddr) +
			       GMAC_STATION_ADDR_HIGH_2_REG, high_val);
	}

	return 0;

}
static int gmac_config_mac_in_loopback(void *mac_drv, u8 enable)
{

	union gmac_loop_reg unloopback;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	unloopback.u32 =  gmac_reg_read((u64)(drv->vaddr) + GMAC_LOOP_REG);
	unloopback.bits.cf2mi_lp_en = (enable & 1);
	gmac_reg_write((u64)(drv->vaddr) + GMAC_LOOP_REG, unloopback.u32);
	return 0;
}

static int gmac_config_loopback(void *mac_drv,
				enum mac_loop_mode loop_mode, u8 enable)
{
	union gmac_line_loopback unlineloopback;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	switch (loop_mode) {
	case MAC_LOOP_NONE:
		(void)gmac_config_mac_in_loopback(mac_drv, 0);
		break;
	case MAC_INTERNALLOOP_MAC:
		(void)gmac_config_mac_in_loopback(mac_drv, 1);
		break;

	case MAC_EXTERNALLOOP_MAC:
		unlineloopback.u32 =
		    gmac_reg_read((u64)(drv->vaddr) + GMAC_LINE_LOOP_BACK_REG);
		unlineloopback.bits.line_loop_back = (enable & 1);
		gmac_reg_write((u64)(drv->vaddr) + GMAC_LINE_LOOP_BACK_REG,
			unlineloopback.u32);
		break;

	default:
		log_err(drv->dev, "loop_mode error\n");
	}

	return 0;
}

static int gmac_config_pad_and_crc(void *mac_drv, u8 newval)
{
	union gmac_transmit unTxCtrl;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	unTxCtrl.u32 =
	    gmac_reg_read((u64)(drv->vaddr) + GMAC_TRANSMIT_CONTROL_REG);
	unTxCtrl.bits.crc_add = (newval & 1);
	unTxCtrl.bits.pad_enable = (newval & 1);
	gmac_reg_write((u64)(drv->vaddr) + GMAC_TRANSMIT_CONTROL_REG,
		unTxCtrl.u32);

	return 0;
}

static int gmac_get_id(void *mac_drv, u8 *mac_id)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	*mac_id = drv->mac_id + drv->chip_id * MAC_PORT_NUM;

	return 0;
}

static u16 gmac_get_max_frame_length(void *mac_drv)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	return gmac_reg_read((u64)(drv->vaddr) + GMAC_MAX_FRM_SIZE_REG);
}

static int gmac_dump_regs(void *mac_drv)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	u32 port;
	u32 i;

	port = drv->chip_id * MAC_PORT_NUM + drv->mac_id;
	osal_printf("********************gmac   ***********************");
	osal_printf("**********\n");

	for (i = 0; i < GE_REG_NUM; i += 4) { /* TBD magic num */
		osal_printf("%#18x :     %#08x    %#08x    %#08x    %#08x\n ",
		     GE_BASE_ADDR + ADDR_PER_PORT * port + i * 4,
		     gmac_reg_read((u64)(drv->vaddr) + i * 4),
		     gmac_reg_read((u64)(drv->vaddr) + (i + 1) * 4),
		     gmac_reg_read((u64)(drv->vaddr) + (i + 2) * 4),
		     gmac_reg_read((u64)(drv->vaddr) + (i + 3) * 4));
	}

	osal_printf("\n");

	return 0;
}

static int gmac_get_info(void *mac_drv, struct mac_info *mac_info)
{
	enum gmac_duplex_mdoe duplex;
	enum port_mode speed;
	u32 rx_pause = 0;
	u32 tx_pause = 0;
	u32 rx = 0;
	u32 tx = 0;
	u16 fc_tx_timer = 0;
	struct gmac_port_mode_cfg strPortMode = { GMAC_10M_MII, 0 };

	(void)gmac_port_mode_get(mac_drv, &strPortMode);
	mac_info->pad_and_crc_en = strPortMode.crc_add
	    && strPortMode.pad_enable;
	mac_info->auto_neg = strPortMode.an_enable;

	(void)gmac_get_tx_auto_pause_frames(mac_drv, &fc_tx_timer);
	mac_info->tx_pause_time = fc_tx_timer;

	(void)gmac_get_en(mac_drv, &rx, &tx);
	mac_info->port_en = rx && tx;

	(void)gmac_get_duplex_type(mac_drv, &duplex);
	mac_info->duplex = duplex;

	(void)gmac_get_portmode(mac_drv, &speed);
	switch (speed) {
	case GMAC_10M_SGMII:
		mac_info->speed = MAC_SPEED_10;
		break;
	case GMAC_100M_SGMII:
		mac_info->speed = MAC_SPEED_100;
		break;
	case GMAC_1000M_SGMII:
		mac_info->speed = MAC_SPEED_1000;
		break;
	default:
		mac_info->speed = 0;
		break;
	}

	(void)gmac_get_pausefrm_cfg(mac_drv, &rx_pause, &tx_pause);
	mac_info->rx_pause_en = rx_pause;
	mac_info->tx_pause_en = tx_pause;

	return 0;
}

static int gmac_autoneg_stat(void *mac_drv, u32 *enable)
{
	union gmac_transmit trans_ctrl;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	trans_ctrl.u32 =
	    gmac_reg_read((u64)(drv->vaddr) + GMAC_TRANSMIT_CONTROL_REG);
	*enable = trans_ctrl.bits.an_enable;
	return 0;
}

static int gmac_get_link_status(void *mac_drv, u32 *link_stat)
{
	union gmac_an_neg_state an_state;
	u32 neg_enable = 0;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	an_state.u32 = gmac_reg_read((u64)(drv->vaddr) + GMAC_AN_NEG_STATE_REG);

	(void)gmac_autoneg_stat(mac_drv, &neg_enable);
	if(neg_enable)
		*link_stat = an_state.bits.an_done && an_state.bits.rx_sync_ok;
	else
		*link_stat = an_state.bits.rx_sync_ok;

    return 0;
}

static void gmac_get_regs(void *mac_drv, struct ethtool_regs *cmd, void *data)
{
	u32 *buf = data;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	buf[0] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_OCTETS_TOTAL_OK_REG);
	buf[1] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_OCTETS_BAD_REG);
	buf[2] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_UC_PKTS_REG);
	buf[3] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_MC_PKTS_REG);
	buf[4] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_BC_PKTS_REG);
	buf[5] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_64OCTETS_REG);
	buf[6] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_65TO127OCTETS_REG);
	buf[7] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_128TO255OCTETS_REG);
	buf[8] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_255TO511OCTETS_REG);
	buf[9] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_512TO1023OCTETS_REG);
	buf[10] =
	    gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_1024TO1518OCTETS_REG);
	buf[11] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_PKTS_1519TOMAXOCTETS_REG);
	buf[12] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_FCS_ERRORS_REG);
	buf[13] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_TAGGED_REG);
	buf[14] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_DATA_ERR_REG);
	buf[15] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_ALIGN_ERRORS_REG);
	buf[16] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_LONG_ERRORS_REG);
	buf[17] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_JABBER_ERRORS_REG);
	buf[18] =
	    gmac_reg_read((u64)drv->vaddr + GMAC_RX_PAUSE_MACCTRL_FRAM_REG);
	buf[19] =
	    gmac_reg_read((u64)drv->vaddr + GMAC_RX_UNKNOWN_MACCTRL_FRAM_REG);
	buf[20] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_VERY_LONG_ERR_CNT_REG);
	buf[21] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_RUNT_ERR_CNT_REG);
	buf[22] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_SHORT_ERR_CNT_REG);
	buf[23] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_FILT_PKT_CNT_REG);
	buf[24] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_OCTETS_TOTAL_FILT_REG);

	buf[25] = gmac_reg_read((u64)drv->vaddr + GMAC_OCTETS_TRANSMITTED_OK_REG);
	buf[26] = gmac_reg_read((u64)drv->vaddr + GMAC_OCTETS_TRANSMITTED_BAD_REG);
	buf[27] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_UC_PKTS_REG);
	buf[28] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_MC_PKTS_REG);
	buf[29] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_BC_PKTS_REG);
	buf[30] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_64OCTETS_REG);
	buf[31] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_65TO127OCTETS_REG);
	buf[32] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_128TO255OCTETS_REG);
	buf[33] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_255TO511OCTETS_REG);
	buf[34] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_512TO1023OCTETS_REG);
	buf[35] =
	    gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_1024TO1518OCTETS_REG);
	buf[36] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_PKTS_1519TOMAXOCTETS_REG);
	buf[37] =
	    gmac_reg_read((u64)drv->vaddr + GMAC_TX_EXCESSIVE_LENGTH_DROP_REG);
	buf[38] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_UNDERRUN_REG);
	buf[39] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_TAGGED_REG);
	buf[40] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_CRC_ERROR_REG);
	buf[41] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_PAUSE_FRAMES_REG);

	buf[42] = gmac_reg_read((u64)drv->vaddr + GMAC_DUPLEX_TYPE_REG);
	buf[43] = gmac_reg_read((u64)drv->vaddr + GMAC_FD_FC_TYPE_REG);
	buf[44] = gmac_reg_read((u64)drv->vaddr + GMAC_FC_TX_TIMER_REG);
	buf[45] = gmac_reg_read((u64)drv->vaddr + GMAC_PAUSE_THR_REG);
	buf[46] = gmac_reg_read((u64)drv->vaddr + GMAC_MAX_FRM_SIZE_REG);
	buf[47] = gmac_reg_read((u64)drv->vaddr + GMAC_PORT_MODE_REG);
	buf[48] = gmac_reg_read((u64)drv->vaddr + GMAC_PORT_EN_REG);
	buf[49] = gmac_reg_read((u64)drv->vaddr + GMAC_PAUSE_EN_REG);
	buf[50] = gmac_reg_read((u64)drv->vaddr + GMAC_SHORT_RUNTS_THR_REG);
	buf[51] = gmac_reg_read((u64)drv->vaddr + GMAC_AN_NEG_STATE_REG);
	buf[52] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_LOCAL_PAGE_REG);
	buf[53] = gmac_reg_read((u64)drv->vaddr + GMAC_TRANSMIT_CONTROL_REG);
	buf[54] = gmac_reg_read((u64)drv->vaddr + GMAC_REC_FILT_CONTROL_REG);
	buf[55] = gmac_reg_read((u64)drv->vaddr + GMAC_LINE_LOOP_BACK_REG);
	buf[56] = gmac_reg_read((u64)drv->vaddr + GMAC_CF_CRC_STRIP_REG);
	buf[57] = gmac_reg_read((u64)drv->vaddr + GMAC_MODE_CHANGE_EN_REG);
	buf[58] = gmac_reg_read((u64)drv->vaddr + GMAC_SIXTEEN_BIT_CNTR_REG);
	buf[59] = gmac_reg_read((u64)drv->vaddr + GMAC_LD_LINK_COUNTER_REG);
	buf[60] = gmac_reg_read((u64)drv->vaddr + GMAC_LOOP_REG);
	buf[61] = gmac_reg_read((u64)drv->vaddr + GMAC_RECV_CONTROL_REG);
	buf[62] = gmac_reg_read((u64)drv->vaddr + GMAC_VLAN_CODE_REG);
	buf[63] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_OVERRUN_CNT_REG);
	buf[64] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_FAIL_COMMA_CNT_REG);
}

static void gmac_get_ethtool_stats(struct mac_device *mac_dev,
	struct ethtool_stats *cmd, u64 *data)
{
	u64 *buf = data;
	struct mac_priv *priv = NULL;
	struct mac_driver *drv = NULL;
	struct mac_hw_stats *hw_stats = NULL;

	priv = mac_dev_priv(mac_dev);
	drv = (struct mac_driver *)(priv->mac);
	hw_stats = &mac_dev->hw_stats;

	gmac_update_stats(mac_dev);

	buf[0] = hw_stats->rx_good_bytes;
	buf[1] = hw_stats->rx_bad_bytes;
	buf[2] = hw_stats->rx_uc_pkts;
	buf[3] = hw_stats->rx_mc_pkts;
	buf[4] = hw_stats->rx_bc_pkts;
	buf[5] = hw_stats->rx_64bytes;
	buf[6] = hw_stats->rx_65to127;
	buf[7] = hw_stats->rx_128to255;
	buf[8] = hw_stats->rx_256to511;
	buf[9] = hw_stats->rx_512to1023;
	buf[10] = hw_stats->rx_1024to1518;
	buf[11] = hw_stats->rx_1519tomax;
	buf[12] = hw_stats->rx_fcs_err;
	buf[13] = hw_stats->rx_vlan_pkts;
	buf[14] = hw_stats->rx_data_err;
	buf[15] = hw_stats->rx_align_err;
	buf[16] = hw_stats->rx_long_err;
	buf[17] = hw_stats->rx_jabber_err;
	buf[18] = hw_stats->rx_pfc_tc0;
	buf[19] = hw_stats->rx_unknown_ctrl;
	buf[20] = hw_stats->rx_oversize;
	buf[21] = hw_stats->rx_minto64;
	buf[22] = hw_stats->rx_under_min;
	buf[23] = hw_stats->rx_filter_pkts;
	buf[24] = hw_stats->rx_filter_bytes;

	buf[25] = hw_stats->tx_good_bytes;
	buf[26] = hw_stats->tx_bad_bytes;
	buf[27] = hw_stats->tx_uc_pkts;
	buf[28] = hw_stats->tx_mc_pkts;
	buf[29] = hw_stats->tx_bc_pkts;
	buf[30] = hw_stats->tx_64bytes;
	buf[31] = hw_stats->tx_65to127;
	buf[32] = hw_stats->tx_128to255;
	buf[33] = hw_stats->tx_256to511;
	buf[34] = hw_stats->tx_512to1023;
	buf[35] = hw_stats->tx_1024to1518;
	buf[36] = hw_stats->tx_1519tomax;
	buf[37] = hw_stats->tx_jabber_err;
	buf[38] = hw_stats->tx_underrun_err;
	buf[39] = hw_stats->tx_vlan;
	buf[40] = hw_stats->tx_crc_err;
	buf[41] = hw_stats->tx_pfc_tc0;

	buf[42] = gmac_reg_read((u64)drv->vaddr + GMAC_DUPLEX_TYPE_REG);
	buf[43] = gmac_reg_read((u64)drv->vaddr + GMAC_FD_FC_TYPE_REG);
	buf[44] = gmac_reg_read((u64)drv->vaddr + GMAC_FC_TX_TIMER_REG);
	buf[45] = gmac_reg_read((u64)drv->vaddr + GMAC_PAUSE_THR_REG);
	buf[46] = gmac_reg_read((u64)drv->vaddr + GMAC_MAX_FRM_SIZE_REG);
	buf[47] = gmac_reg_read((u64)drv->vaddr + GMAC_PORT_MODE_REG);
	buf[48] = gmac_reg_read((u64)drv->vaddr + GMAC_PORT_EN_REG);
	buf[49] = gmac_reg_read((u64)drv->vaddr + GMAC_PAUSE_EN_REG);
	buf[50] = gmac_reg_read((u64)drv->vaddr + GMAC_SHORT_RUNTS_THR_REG);
	buf[51] = gmac_reg_read((u64)drv->vaddr + GMAC_AN_NEG_STATE_REG);
	buf[52] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_LOCAL_PAGE_REG);
	buf[53] = gmac_reg_read((u64)drv->vaddr + GMAC_TRANSMIT_CONTROL_REG);
	buf[54] = gmac_reg_read((u64)drv->vaddr + GMAC_REC_FILT_CONTROL_REG);
	buf[55] = gmac_reg_read((u64)drv->vaddr + GMAC_LINE_LOOP_BACK_REG);
	buf[56] = gmac_reg_read((u64)drv->vaddr + GMAC_CF_CRC_STRIP_REG);
	buf[57] = gmac_reg_read((u64)drv->vaddr + GMAC_MODE_CHANGE_EN_REG);
	buf[58] = gmac_reg_read((u64)drv->vaddr + GMAC_SIXTEEN_BIT_CNTR_REG);
	buf[59] = gmac_reg_read((u64)drv->vaddr + GMAC_LD_LINK_COUNTER_REG);
	buf[60] = gmac_reg_read((u64)drv->vaddr + GMAC_LOOP_REG);
	buf[61] = gmac_reg_read((u64)drv->vaddr + GMAC_RECV_CONTROL_REG);
	buf[62] = gmac_reg_read((u64)drv->vaddr + GMAC_VLAN_CODE_REG);
	buf[63] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_OVERRUN_CNT_REG);
	buf[64] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_FAIL_COMMA_CNT_REG);
}

static void gmac_get_strings(void *mac_drv, u32 stringset, u8 *data)
{
	char *buff = (char *)data;

	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_OCTETS_TOTAL_OK");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_OCTETS_BAD");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_UC_PKTS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_MC_PKTS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_BC_PKTS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_PKTS_64OCTETS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_PKTS_65TO127");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_PKTS_128TO255");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_PKTS_255TO511");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_PKTS_512TO1023");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_PKTS_1024TO1518");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_PKTS_1519TOMAX");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_FCS_ERRORS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_TAGGED");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_DATA_ERR");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_ALIGN_ERRORS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_LONG_ERRORS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_JABBER_ERRORS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_PAUSE_MACCONTROL");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_UNKNOWN_MACCONTROL");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_VERY_LONG_ERR");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_RUNT_ERR");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_SHORT_ERR");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_FILT_PKT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_OCTETS_TOTAL_FILT");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "GMAC_OCTETS_TRANSMITTED_OK");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_OCTETS_TRANSMITTED_BAD");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_UC_PKTS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_MC_PKTS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_BC_PKTS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_PKTS_64OCTETS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_PKTS_65TO127");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_PKTS_128TO255");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_PKTS_255TO511");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_PKTS_512TO1023");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_PKTS_1024TO1518");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_PKTS_1519TOMAX");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_EXCESSIVE_LENGTH_DROP");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_UNDERRUN");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_TAGGED");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_CRC_ERROR");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_PAUSE_FRAMES");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "GMAC_DUPLEX_TYPE");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_FD_FC_TYPE");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_FC_TX_TIMER");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_PAUSE_THR");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_MAX_FRM_SIZE");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_PORT_MODE");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_PORT_EN");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_PAUSE_EN");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_SHORT_RUNTS_THR");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "GMAC_AN_NEG_STATE");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TX_LOCAL_PAGE");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_TRANSMIT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_REC_FILT_CONTROL");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_LINE_LOOP_BACK");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_CF_CRC_STRIP");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_MODE_CHANGE_EN");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_SIXTEEN_BIT_CNTR");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_LD_LINK_COUNTER");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_LOOP_REG");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RECV_COMTROL");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_VLAN_CODE");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_OVERRUN_CNT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "GMAC_RX_FAIL_COMMA");
}

static int gmac_get_sset_count(void *mac_drv, u32 stringset)
{
	u32 num = 0;

	if (ETH_STATIC_REG == stringset)
		num = 64;
	else if (ETH_DUMP_REG == stringset)
		num = 64;

	return num;
}
static void gmac_get_total_pkts(void *mac_drv, u32 *tx_pkt, u32 *rx_pkt)
{
	u32 rx_pkt_num[3] = {0};
	u32 tx_pkt_num[3] = {0};
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	tx_pkt_num[0] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_UC_PKTS_REG);
	tx_pkt_num[1] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_MC_PKTS_REG);
	tx_pkt_num[2] = gmac_reg_read((u64)drv->vaddr + GMAC_TX_BC_PKTS_REG);
	rx_pkt_num[0] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_UC_PKTS_REG);
	rx_pkt_num[1] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_MC_PKTS_REG);
	rx_pkt_num[2] = gmac_reg_read((u64)drv->vaddr + GMAC_RX_BC_PKTS_REG);
	*tx_pkt = tx_pkt_num[0] + tx_pkt_num[1] + tx_pkt_num[2];
	*rx_pkt = rx_pkt_num[0] + rx_pkt_num[1] + rx_pkt_num[2];
}


void *gmac_config(struct mac_params *mac_param)
{
	struct mac_driver *mac_drv = NULL;

	mac_drv =
	    (struct mac_driver *)osal_kmalloc(sizeof(*mac_drv), GFP_KERNEL);
	if (NULL == mac_drv)
		return NULL;

	memset(mac_drv, 0, sizeof(struct mac_driver));

	mac_drv->mac_init = gmac_init;
	mac_drv->mac_enable = gmac_enable;
	mac_drv->mac_disable = gmac_disable;
	mac_drv->mac_free = gmac_free;
	mac_drv->mac_adjust_link = gmac_adjust_link;
	mac_drv->mac_set_tx_auto_pause_frames = gmac_set_tx_auto_pause_frames;
	mac_drv->mac_config_max_frame_length = gmac_config_max_frame_length;
	mac_drv->mac_get_statistics = gmac_get_statistics;
	mac_drv->mac_pausefrm_cfg = gmac_pause_frm_cfg;

	mac_drv->chip_id = mac_param->chip_id;
	mac_drv->mac_id = mac_param->mac_id;
	mac_drv->mac_mode = mac_param->mac_mode;
	mac_drv->vaddr = mac_param->vaddr;
	mac_drv->dev = mac_param->dev;

	mac_drv->mac_reset = gmac_reset;
	mac_drv->mac_set_mac_addr = gmac_set_mac_addr;
	mac_drv->mac_set_an_mode = gmac_config_an_mode;
	mac_drv->mac_config_loopback = gmac_config_loopback;
	mac_drv->mac_config_pad_and_crc = gmac_config_pad_and_crc;
	mac_drv->mac_config_half_duplex = gmac_set_duplex_type;
	mac_drv->mac_set_rx_ignore_pause_frames = gmac_set_rx_auto_pause_frames;
	mac_drv->mac_get_id = gmac_get_id;
	mac_drv->mac_get_max_frame_length = gmac_get_max_frame_length;
	mac_drv->mac_dump_regs = gmac_dump_regs;
	mac_drv->get_info = gmac_get_info;
	mac_drv->autoneg_stat = gmac_autoneg_stat;
	mac_drv->get_pause_enable = gmac_get_pausefrm_cfg;
	mac_drv->get_link_status = gmac_get_link_status;
	mac_drv->get_regs = gmac_get_regs;
	mac_drv->get_ethtool_stats = gmac_get_ethtool_stats;
	mac_drv->get_sset_count = gmac_get_sset_count;
	mac_drv->get_strings = gmac_get_strings;
	mac_drv->mac_ioctl = NULL;
	mac_drv->update_stats = gmac_update_stats;
	mac_drv->get_total_txrx_pkts = gmac_get_total_pkts;

	return (void *)mac_drv;
}

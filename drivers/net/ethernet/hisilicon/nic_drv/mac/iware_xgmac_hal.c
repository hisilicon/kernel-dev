/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe486w+QA9m4/tbgoc6ne0rMAf/sgZi/QFoXGK+hCct3TlbuP4YzNzWGd9hxFzlH3ekj0DE
QuPPs8QvI4kzpvBwdOWPZ5IcTsT33G398c1oFMyGLry6/YLLTD58G5dCt4CWHQ==*/
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
/*
#include "iware_typedef.h"
#include "iware_module.h"
#include "iwareCommon.h"
#include "iwareOs.h"*/
#include "iware_dsaf_main.h"
#include "hrd_crg_api.h"
#include "iware_mac_hal.h"
#include "iware_xgmac_hal.h"

static u64 g_xgmac_base_addr[16];

static const struct mac_stats_string g_xgmac_stats_string[XGMAC_MIB_NUM] =
{
	{"Tx total bad (TxFiltersize<size< 64):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_fragment_err)},
	{"Tx total good (TxFiltersize<size< 64):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_undersize)},
	{"Tx total good and bad TxFiltersize <size< 64): 0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_under_min_pkts)},
	{"Tx total good and bad (size = 64):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_64bytes)},
	{"Tx total good and bad (65 <= size < 127):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_65to127)},
	{"Tx total good and bad (128 <= size < 256):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_128to255)},
	{"Tx total good and bad (256 <= size < 512):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_256to511)},
	{"Tx total good and bad (512 <= size < 1024):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_512to1023)},
	{"Tx total good and bad (1024 <= size < 1518):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_1024to1518)},
	{"Tx total good and bad (1519 <= size <= Maxsize): 0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_1519tomax)},
	{"Tx total good (1519 <= size <= Maxsize):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_1519tomax_good)},
	{"Tx total good  (Maxsize < size):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_oversize)},
	{"Tx total bad (Maxsize < size):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_jabber_err)},
	{"Tx good pkt cnt: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_good_pkts)},
	{"Tx good byte cnt: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_good_bytes)},
	{"Tx total good and bad: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_total_pkts)},
	{"Tx total good and bad byte: 0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_total_bytes)},
	{"Tx total unicast: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_uc_pkts)},
	{"Tx total multicast: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_mc_pkts)},
	{"Tx total broadcast: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_bc_pkts)},
	{"Tx total pause frame (PFC pri 0 pause frame):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_pfc_tc0)},
	{"Tx total PFC pri 1: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_pfc_tc1)},
	{"Tx total PFC pri 2: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_pfc_tc2)},
	{"Tx total PFC pri 3: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_pfc_tc3)},
	{"Tx total PFC pri 4: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_pfc_tc4)},
	{"Tx total PFC pri 5: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_pfc_tc5)},
	{"Tx total PFC pri 6: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_pfc_tc6)},
	{"Tx total PFC pri 7: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_pfc_tc7)},
	{"Tx total mac control: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_ctrl)},
	{"Tx total 1731 packet: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_1731_pkts)},
	{"Tx total 1588 packet: 0x%llx\n", MAC_STATS_FIELD_OFF(tx_1588_pkts)},
	{"Tx total good receive from dsaf fbric: 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_good_from_sw)},
	{"Tx total bad receive from dsaf fbric:0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_bad_from_sw)},
	{"Tx total bad(64<=size<=Maxsize):0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_bad_pkts)},

	{"Rx total not well formed(RxFiltersize <size< 64): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_fragment_err)},
	{"Rx total good well formed(RxFiltersize <size< 64): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_undersize)},
	{"Rx total all(RxFiltersize < size < minFramesize): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_under_min)},
	{"Rx total the good and bad(size = 64): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_64bytes)},
	{"Rx total good and bad(65 <= size < 128):0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_65to127)},
	{"Rx total good and bad(128 <= size < 256): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_128to255)},
	{"Rx total good and bad(256 <= size < 512): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_256to511)},
	{"Rx total good and bad(512 <= size < 1024): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_512to1023)},
	{"Rx total good and bad(1024 <= size <= 1518): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_1024to1518)},
	{"Rx total good and bad(1519 <= size <= Maxsize): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_1519tomax)},
	{"Rx total good(1519 <= size <= Maxsize): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_1519tomax_good)},
	{"Rx total good(Maxsize < size):0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_oversize)},
	{"Rx total number of bad(Maxsize < size):0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_jabber_err)},
	{"Rx good pkt cnt: 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_good_pkts)},
	{"Rx good byte cnt: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_good_bytes)},
	{"Rx total good and bad packets: 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_total_pkts)},
	{"Rx total good and bad byte: 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_total_bytes)},
	{"Rx total unicast packets: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_uc_pkts)},
	{"Rx total multicast packets: 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_mc_pkts)},
	{"Rx total broadcast packets: 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_bc_pkts)},
	{"Rx total pause frame (PFC pri 0 pause frame): 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_pfc_tc0)},
	{"Rx total PFC pri 1: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_pfc_tc1)},
	{"Rx total PFC pri 2: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_pfc_tc2)},
	{"Rx total PFC pri 3: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_pfc_tc3)},
	{"Rx total PFC pri 4: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_pfc_tc4)},
	{"Rx total PFC pri 5: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_pfc_tc5)},
	{"Rx total PFC pri 6: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_pfc_tc6)},
	{"Rx total PFC pri 7: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_pfc_tc7)},
	{"Rx total mac control: 0x%llx\n",
		MAC_STATS_FIELD_OFF(rx_unknown_ctrl)},
	{"Rx total good send to dsaf fbric: 0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_good_to_sw)},
	{"Tx total bad send to dsaf fbric: 0x%llx\n",
		MAC_STATS_FIELD_OFF(tx_bad_to_sw)},
	{"Rx total 1731 packet: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_1731_pkts)},
	{"Rx total symbol error: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_symbol_err)},
	{"Rx total fcs error: 0x%llx\n", MAC_STATS_FIELD_OFF(rx_fcs_err)},
};

static inline u32 xgmac_read(u32 port, u32 offset)
{
	u8 __iomem *reg_addr = (u8 __iomem *)g_xgmac_base_addr[port];
	u32 value;

	value = readl(reg_addr + offset);
	return value;
}

static inline u64 xgmac_mib_read(u32 port, u32 offset)
{
	u8 __iomem *reg_addr = ((u8 __iomem *)(g_xgmac_base_addr[port] + 0xC00));

	return readq(reg_addr + offset);
}

static inline void xgmac_write(u32 port, u32 offset, u32 value)
{
	u8 __iomem *reg_addr = (u8 __iomem *)g_xgmac_base_addr[port];

	writel(value, reg_addr + offset);
}

/***************************************************************
*********************** code for XGE *****************************
***************************************************************/

/**
 *xgmac_tx_enable - xgmac port tx enable
 *@port: mac port number
 *@value: value of enable
 *return status
 */
static int xgmac_tx_enable(u32 port, u32 value)
{
	union xgmac_mac_enable value_tmp;

	value_tmp.u32 = xgmac_read(port, XGMAC_MAC_ENABLE_REG);

	value_tmp.bits.tx_enable = value;

	xgmac_write(port, XGMAC_MAC_ENABLE_REG, value_tmp.u32);

	value_tmp.u32 = xgmac_read(port, XGMAC_MAC_ENABLE_REG);

	pr_debug("port=%d, value=%d base=%#llx value_tmp=%d\n",
			port, value, g_xgmac_base_addr[port], value_tmp.u32);

	return 0;
}

/**
 *xgmac_rx_enable - xgmac port rx enable
 *@port: mac port number
 *@value: value of enable
 *return status
 */
int xgmac_rx_enable(u32 port, u32 value)
{
	union xgmac_mac_enable value_tmp;

	value_tmp.u32 = xgmac_read(port, XGMAC_MAC_ENABLE_REG);
	value_tmp.bits.rx_enable = value;

	xgmac_write(port, XGMAC_MAC_ENABLE_REG, value_tmp.u32);

	value_tmp.u32 = xgmac_read(port, XGMAC_MAC_ENABLE_REG);

	pr_debug("port=%d, rx2 value=%d base=%#llx value_tmp=%d!\n",
			port, value, g_xgmac_base_addr[port], value_tmp.u32);

	return 0;
}

/**
 *xgmac_enable - enable xgmac port
 *@port: mac port number
 *@mode: mode of mac port
 *return status
 */
static int xgmac_enable(void *mac_drv, enum mac_commom_mode mode)
{
	u32 port = 0xff;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;

	/*enable XGE rX/tX */
	if (MAC_COMM_MODE_TX == mode)
		(void)xgmac_tx_enable(port, 1);
	else if (MAC_COMM_MODE_RX == mode)
		(void)xgmac_rx_enable(port, 1);
	else if (MAC_COMM_MODE_RX_AND_TX == mode) {
		(void)xgmac_tx_enable(port, 1);
		(void)xgmac_rx_enable(port, 1);
	} else {
		log_err(drv->dev, "xgmac_enable fail, mode = %d\n", mode);
		return -EINVAL;
	}

	return 0;
}

/**
 *xgmac_disable - disable xgmac port
 *@port: mac port number
 *@mode: mode of mac port
 *return status
 */
static int xgmac_disable(void *mac_drv, enum mac_commom_mode mode)
{
	u32 port = 0xff;
	struct mac_driver *drv;

	if (NULL == mac_drv) {
		pr_err("mac_dev is null ! \r\n");
		return -EINVAL;
	}
	drv = (struct mac_driver *)mac_drv;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;

	if (MAC_COMM_MODE_TX == mode)
		(void)xgmac_tx_enable(port, 0);

	else if (MAC_COMM_MODE_RX == mode)
		(void)xgmac_rx_enable(port, 0);
	else if (MAC_COMM_MODE_RX_AND_TX == mode) {
		(void)xgmac_tx_enable(port, 0);
		(void)xgmac_rx_enable(port, 0);
	} else {
		log_err(drv->dev,
				"xgmac_disable fail, mode = %d\n", mode);
		return -EINVAL;
	}

	return 0;
}

/**
 *xgmac_pad_enable - xgmac PAD enable
 *@port: mac port number
 *@value: value of enable
 *return status
 */
int xgmac_pad_enable(u32 port, u32 value)
{
	union xgmac_mac_control value_tmp;

	value_tmp.u32 = xgmac_read(port, XGMAC_MAC_CONTROL_REG);

	value_tmp.bits.tx_pad_en = value;

	xgmac_write(port, XGMAC_MAC_CONTROL_REG, value_tmp.u32);

	return 0;
}

/**
 *xgmac_pma_fec_enable - xgmac PMA FEC enable
 *@port: mac port number
 *@tx_value: tx value
 *@rx_value: rx value
 *return status
 */
int xgmac_pma_fec_enable(u32 port, u32 tx_value, u32 rx_value)
{
	union xgmac_pma_fec_ctrl value;
	value.u32 = xgmac_read(port, XGMAC_PMA_FEC_CONTROL_REG);
	value.bits.tx_pma_fec_en = tx_value;
	value.bits.rx_pma_fec_en = rx_value;
	xgmac_write(port, XGMAC_PMA_FEC_CONTROL_REG, value.u32);

	return 0;
}

/**
 *xgmac_baseaddr_init - initialize XGE base address
 *@mac_drv: mac driver
 *return status
 */
static int xgmac_baseaddr_init(void *mac_drv)
{
	u32 port = 0;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;
	g_xgmac_base_addr[port] = (u64)drv->vaddr;

	return 0;
}

/**
 *xgmac_init - initialize XGE
 *@mac_drv: mac driver
 *return status
 */
static int xgmac_init(void *mac_drv)
{
	u32 port = 0;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;

	(void)xgmac_baseaddr_init(mac_drv);
	(void)HRD_Dsaf_XgeSrstByPort(port, 0);
	osal_mdelay(100);
	(void)HRD_Dsaf_XgeSrstByPort(port, 1);

	osal_mdelay(100);
	(void)xgmac_pma_fec_enable(port, 0x0, 0x0);
	/*xgmac_pma_fec_enable(port, 0x1, 0x1);*/

	(void)xgmac_disable(mac_drv, MAC_COMM_MODE_RX_AND_TX);

	return 0;
}
int xgmac_reset(void *mac_drv, u32 wait)
{
	return 0;
}

/**
 *xgmac_set_an_mode - set xgmac auto negotiate enable
 *@mac_drv: mac driver
 *@enable:xgmac enable
 *return status
 */
int xgmac_set_an_mode(void *mac_drv, u8 enable)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	union xgmac_port_mode port_mode;
	u32 port = 0;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;
	port_mode.u32 = xgmac_read(port, XGMAC_PORT_MODE_REG);

	port_mode.bits.tx_mode = (enable & 1);
	port_mode.bits.rx_mode = (enable & 1);

	xgmac_write(port, XGMAC_PORT_MODE_REG, port_mode.u32);

	return 0;
}

/**
 *xgmac_config_loopback - set xgmac loopback mode
 *@mac_drv: mac driver
 *@loop_mode:loopmode
 *@enable:xgmac enable
 *return status
 */
int xgmac_config_loopback(void *mac_drv,
			  enum mac_loop_mode loop_mode, u8 enable)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;


	switch (loop_mode) {
	case MAC_LOOP_NONE:
		return 0;
	default:
		log_err(drv->dev,
			"xgmac_config_loopback faild,mode%d mac%d dsaf%d\n",
			loop_mode,drv->mac_id, drv->chip_id);
		return -EINVAL;

	}

}

/**
 *xgmac_config_pad_and_crc - set xgmac pad and crc enable the same time
 *@mac_drv: mac driver
 *@newval:enable of pad and crc
 *return status
 */
int xgmac_config_pad_and_crc(void * mac_drv, u8 newval)
{

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	union xgmac_mac_control value_tmp;

	u32 port = 0;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;
	value_tmp.u32 = xgmac_read(port, XGMAC_MAC_CONTROL_REG);

	value_tmp.bits.tx_pad_en = (newval & 1);
	value_tmp.bits.tx_fcs_en = (newval & 1);
	value_tmp.bits.rx_fcs_en = (newval & 1);

	xgmac_write(port, XGMAC_MAC_CONTROL_REG, value_tmp.u32);

	return 0;
}

/**
 *xgmac_pausefrm_cfg - set pause param about xgmac
 *@mac_drv: mac driver
 *@newval:enable of pad and crc
 *return status
 */
int xgmac_pausefrm_cfg(void *mac_drv, u32 rx_pause_en, u32 tx_pause_en)
{
	union xgmac_mac_pause_ctrl pause_ctrl;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	u32 port = 0;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;
	pause_ctrl.u32 = xgmac_read(port, XGMAC_MAC_PAUSE_CTRL_REG);

	pause_ctrl.bits.tx_pause_en = (tx_pause_en & 1);
	pause_ctrl.bits.rx_pause_en = (rx_pause_en & 1);

	xgmac_write(port, XGMAC_MAC_PAUSE_CTRL_REG, pause_ctrl.u32);

	return 0;
}

/**
 *xgmac_set_rx_ignore_pause_frames - set rx pause param about xgmac
 *@mac_drv: mac driver
 *@enable:enable rx pause param
 *return status
 */
int xgmac_set_rx_ignore_pause_frames(void *mac_drv, u32 enable)
{
	union xgmac_mac_pause_ctrl pause_ctrl;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	u32 port = 0;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;
	pause_ctrl.u32 = xgmac_read(port, XGMAC_MAC_PAUSE_CTRL_REG);
	pause_ctrl.bits.rx_pause_en = (enable & 1);

	xgmac_write(port, XGMAC_MAC_PAUSE_CTRL_REG, pause_ctrl.u32);

	return 0;
}

/**
 *xgmac_set_tx_auto_pause_frames - set tx pause param about xgmac
 *@mac_drv: mac driver
 *@enable:enable tx pause param
 *return status
 */
int xgmac_set_tx_auto_pause_frames(void *mac_drv, u16 enable)
{
	union xgmac_mac_pause_ctrl pause_ctrl;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	u32 port = 0;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;

	pause_ctrl.u32 = xgmac_read(port, XGMAC_MAC_PAUSE_CTRL_REG);
	pause_ctrl.bits.tx_pause_en = (enable & 1);
	xgmac_write(port, XGMAC_MAC_PAUSE_CTRL_REG, pause_ctrl.u32);

	/*if enable is not zero ,set tx pause time */
	if (enable)
		xgmac_write(port, XGMAC_MAC_PAUSE_TIME_REG, enable);

	return 0;
}

/**
 *xgmac_get_id - get xgmac port id
 *@mac_drv: mac driver
 *@newval:xgmac max frame length
 *return status
 */
int xgmac_get_id(void *mac_drv, u8 *mac_id)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	*mac_id = drv->mac_id + drv->chip_id * XGMAC_PORT_NUM;

	return 0;
}

/**
 * xgmac_get_max_frame_length - get xgmac max frame length
 * @mac_drv: mac driver
 * @newval:xgmac max frame length
 * Return max frame length
 */
u16 xgmac_get_max_frame_length(void *mac_drv)
{
	u16 xgamc_max_frame_length = 0;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	u32 port = 0;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;

	xgamc_max_frame_length = xgmac_read(port, XGMAC_MAC_MAX_PKT_SIZE_REG);
	return xgamc_max_frame_length;
}

/**
 *xgmac_config_max_frame_length - set xgmac max frame length
 *@mac_drv: mac driver
 *@newval:xgmac max frame length
 *return status
 */
int xgmac_config_max_frame_length(void *mac_drv, u16 newval)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	u32 port = 0;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;

	xgmac_write(port, XGMAC_MAC_MAX_PKT_SIZE_REG, newval);
	return 0;
}

/**
 *xgmac_dump_regs - dump xgmac regs
 *@mac_drv: mac driver
 *return status
 */
int xgmac_dump_regs(void *mac_drv)
{
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	u32 port;
	u32 i;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;

	osal_printf("**************** xgmac ENetMIB  *******************\n");
	for (i = 0; i < XGMAC_MIB_NUM; i += 2)
		osal_printf("%#18x :  %#18llx   %#18llx\n ",
			0xc7020000 + 0x4000 * port + i * 2,
			xgmac_mib_read(port, i * 8),
			xgmac_mib_read(port, (i + 1) * 8));

	osal_printf("**************** xgmac HiMAC_Reg  ****************\n");
	for (i = 0; i < XGMAC_DUMP_NUM; i += 4)
		osal_printf("%#18x :      %#08x      %#08x      %#08x      %#08x\n ",
			0xc7020000 + 0x4000 * port + i * 4, /* TBD */
			xgmac_read(port, i * 4),
			xgmac_read(port, (i + 1) * 4),
			xgmac_read(port, (i + 2) * 4),
			xgmac_read(port, (i + 3) * 4));

	osal_printf("\n");

	return 0;
}

static void xgmac_update_stats(struct mac_device *mac_dev)
{
	u32 port = mac_dev->chip_id * XGMAC_PORT_NUM + mac_dev->mac_id;
	struct mac_hw_stats *hw_stats = &mac_dev->hw_stats;

	/* TX */
	hw_stats->tx_fragment_err
		= xgmac_mib_read(port, XGMAC_TX_PKTS_FRAGMENT);
	hw_stats->tx_undersize = xgmac_mib_read(port, XGMAC_TX_PKTS_UNDERSIZE);
	hw_stats->tx_under_min_pkts
		= xgmac_mib_read(port, XGMAC_TX_PKTS_UNDERMIN);
	hw_stats->tx_64bytes = xgmac_mib_read(port, XGMAC_TX_PKTS_64OCTETS);
	hw_stats->tx_65to127
		= xgmac_mib_read(port, XGMAC_TX_PKTS_65TO127OCTETS);
	hw_stats->tx_128to255
		= xgmac_mib_read(port, XGMAC_TX_PKTS_128TO255OCTETS);
	hw_stats->tx_256to511
		= xgmac_mib_read(port, XGMAC_TX_PKTS_256TO511OCTETS);
	hw_stats->tx_512to1023
		= xgmac_mib_read(port, XGMAC_TX_PKTS_512TO1023OCTETS);
	hw_stats->tx_1024to1518
		= xgmac_mib_read(port, XGMAC_TX_PKTS_1024TO1518OCTETS);
	hw_stats->tx_1519tomax
		= xgmac_mib_read(port, XGMAC_TX_PKTS_1519TOMAXOCTETS);
	hw_stats->tx_1519tomax_good
		= xgmac_mib_read(port, XGMAC_TX_PKTS_1519TOMAXOCTETSOK);
	hw_stats->tx_oversize = xgmac_mib_read(port, XGMAC_TX_PKTS_OVERSIZE);
	hw_stats->tx_jabber_err = xgmac_mib_read(port, XGMAC_TX_PKTS_JABBER);
	hw_stats->tx_good_pkts = xgmac_mib_read(port, XGMAC_TX_GOODPKTS);
	hw_stats->tx_good_bytes = xgmac_mib_read(port, XGMAC_TX_GOODOCTETS);
	hw_stats->tx_total_pkts = xgmac_mib_read(port, XGMAC_TX_TOTAL_PKTS);
	hw_stats->tx_total_bytes = xgmac_mib_read(port, XGMAC_TX_TOTALOCTETS);
	hw_stats->tx_uc_pkts = xgmac_mib_read(port, XGMAC_TX_UNICASTPKTS);
	hw_stats->tx_mc_pkts = xgmac_mib_read(port, XGMAC_TX_MULTICASTPKTS);
	hw_stats->tx_bc_pkts = xgmac_mib_read(port, XGMAC_TX_BROADCASTPKTS);
	hw_stats->tx_pfc_tc0 = xgmac_mib_read(port, XGMAC_TX_PRI0PAUSEPKTS);
	hw_stats->tx_pfc_tc1 = xgmac_mib_read(port, XGMAC_TX_PRI1PAUSEPKTS);
	hw_stats->tx_pfc_tc2 = xgmac_mib_read(port, XGMAC_TX_PRI2PAUSEPKTS);
	hw_stats->tx_pfc_tc3 = xgmac_mib_read(port, XGMAC_TX_PRI3PAUSEPKTS);
	hw_stats->tx_pfc_tc4 = xgmac_mib_read(port, XGMAC_TX_PRI4PAUSEPKTS);
	hw_stats->tx_pfc_tc5 = xgmac_mib_read(port, XGMAC_TX_PRI5PAUSEPKTS);
	hw_stats->tx_pfc_tc6 = xgmac_mib_read(port, XGMAC_TX_PRI6PAUSEPKTS);
	hw_stats->tx_pfc_tc7 = xgmac_mib_read(port, XGMAC_TX_PRI7PAUSEPKTS);
	hw_stats->tx_ctrl = xgmac_mib_read(port, XGMAC_TX_MACCTRLPKTS);
	hw_stats->tx_1731_pkts = xgmac_mib_read(port, XGMAC_TX_1731PKTS);
	hw_stats->tx_1588_pkts = xgmac_mib_read(port, XGMAC_TX_1588PKTS);
	hw_stats->rx_good_from_sw
		= xgmac_mib_read(port, XGMAC_RX_FROMAPPGOODPKTS);
	hw_stats->rx_bad_from_sw
		= xgmac_mib_read(port, XGMAC_RX_FROMAPPBADPKTS);
	hw_stats->tx_bad_pkts = xgmac_mib_read(port, XGMAC_TX_ERRALLPKTS);

	/* RX */
	hw_stats->rx_fragment_err
		= xgmac_mib_read(port, XGMAC_RX_PKTS_FRAGMENT);
	hw_stats->rx_undersize = xgmac_mib_read(port, XGMAC_RX_PKTSUNDERSIZE);
	hw_stats->rx_under_min = xgmac_mib_read(port, XGMAC_RX_PKTS_UNDERMIN);
	hw_stats->rx_64bytes = xgmac_mib_read(port, XGMAC_RX_PKTS_64OCTETS);
	hw_stats->rx_65to127
		= xgmac_mib_read(port, XGMAC_RX_PKTS_65TO127OCTETS);
	hw_stats->rx_128to255
		= xgmac_mib_read(port, XGMAC_RX_PKTS_128TO255OCTETS);
	hw_stats->rx_256to511
		= xgmac_mib_read(port, XGMAC_RX_PKTS_256TO511OCTETS);
	hw_stats->rx_512to1023
		= xgmac_mib_read(port, XGMAC_RX_PKTS_512TO1023OCTETS);
	hw_stats->rx_1024to1518
		= xgmac_mib_read(port, XGMAC_RX_PKTS_1024TO1518OCTETS);
	hw_stats->rx_1519tomax
		= xgmac_mib_read(port, XGMAC_RX_PKTS_1519TOMAXOCTETS);
	hw_stats->rx_1519tomax_good
		= xgmac_mib_read(port, XGMAC_RX_PKTS_1519TOMAXOCTETSOK);
	hw_stats->rx_oversize = xgmac_mib_read(port, XGMAC_RX_PKTS_OVERSIZE);
	hw_stats->rx_jabber_err = xgmac_mib_read(port, XGMAC_RX_PKTS_JABBER);
	hw_stats->rx_good_pkts = xgmac_mib_read(port, XGMAC_RX_GOODPKTS);
	hw_stats->rx_good_bytes = xgmac_mib_read(port, XGMAC_RX_GOODOCTETS);
	hw_stats->rx_total_pkts = xgmac_mib_read(port, XGMAC_RX_TOTAL_PKTS);
	hw_stats->rx_total_bytes = xgmac_mib_read(port, XGMAC_RX_TOTALOCTETS);
	hw_stats->rx_uc_pkts = xgmac_mib_read(port, XGMAC_RX_UNICASTPKTS);
	hw_stats->rx_mc_pkts = xgmac_mib_read(port, XGMAC_RX_MULTICASTPKTS);
	hw_stats->rx_bc_pkts = xgmac_mib_read(port, XGMAC_RX_BROADCASTPKTS);
	hw_stats->rx_pfc_tc0 = xgmac_mib_read(port, XGMAC_RX_PRI0PAUSEPKTS);
	hw_stats->rx_pfc_tc1 = xgmac_mib_read(port, XGMAC_RX_PRI1PAUSEPKTS);
	hw_stats->rx_pfc_tc2 = xgmac_mib_read(port, XGMAC_RX_PRI2PAUSEPKTS);
	hw_stats->rx_pfc_tc3 = xgmac_mib_read(port, XGMAC_RX_PRI3PAUSEPKTS);
	hw_stats->rx_pfc_tc4 = xgmac_mib_read(port, XGMAC_RX_PRI4PAUSEPKTS);
	hw_stats->rx_pfc_tc5 = xgmac_mib_read(port, XGMAC_RX_PRI5PAUSEPKTS);
	hw_stats->rx_pfc_tc6 = xgmac_mib_read(port, XGMAC_RX_PRI6PAUSEPKTS);
	hw_stats->rx_pfc_tc7 = xgmac_mib_read(port, XGMAC_RX_PRI7PAUSEPKTS);

	hw_stats->rx_unknown_ctrl = xgmac_mib_read(port, XGMAC_RX_MACCTRLPKTS);
	hw_stats->tx_good_to_sw
		= xgmac_mib_read(port, XGMAC_TX_SENDAPPGOODPKTS);
	hw_stats->tx_bad_to_sw = xgmac_mib_read(port, XGMAC_TX_SENDAPPBADPKTS);
	hw_stats->rx_1731_pkts = xgmac_mib_read(port, XGMAC_RX_1731PKTS);
	hw_stats->rx_symbol_err = xgmac_mib_read(port, XGMAC_RX_SYMBOLERRPKTS);
	hw_stats->rx_fcs_err = xgmac_mib_read(port, XGMAC_RX_FCSERRPKTS);
}

void xgmac_clean_stats(struct mac_device *mac_dev)
{
	u32 port = mac_dev->chip_id * XGMAC_PORT_NUM + mac_dev->mac_id;

	xgmac_write(port, XGMAC_MAC_MIB_CONTROL_REG, 1);
}

/**
 *xgmac_get_statistics - get xgmac statistics
 *@mac_drv: mac driver
 *@statistics:xgmac statistics data
 *return status
 */
static int xgmac_get_statistics(struct mac_device *mac_dev,
	struct mac_statistics *statistics, u8 reset_count)
{
	u32 i;
	u64 stats_val;
	struct mac_hw_stats *hw_stats = &mac_dev->hw_stats;

	xgmac_update_stats(mac_dev);

	for (i = 0; i < XGMAC_MIB_NUM; i++) {
		stats_val = MAC_STATS_READ(hw_stats,
			g_xgmac_stats_string[i].offset);
		osal_printf(g_xgmac_stats_string[i].desc, stats_val);
	}

	return 0;
}

/**
 *xgmac_free - free xgmac driver
 *@mac_drv: mac driver
 *return status
 */
static int xgmac_free(void *mac_drv)
{
	osal_kfree(mac_drv);

	return 0;
}

/**
 *xgmac_get_info - get xgmac information
 *@mac_drv: mac driver
 *@mac_info:mac information
 *return status
 */
int xgmac_get_info(void *mac_drv, struct mac_info *mac_info)
{
	union xgmac_mac_control value_tmp;
	u32 pause_time;
	union xgmac_mac_pause_ctrl pause_ctrl;
	union xgmac_port_mode port_mode;

	u32 port = 0;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;

	value_tmp.u32 = xgmac_read(port, XGMAC_MAC_CONTROL_REG);
	mac_info->pad_and_crc_en = value_tmp.bits.tx_pad_en
		&& value_tmp.bits.tx_pad_en;
	mac_info->auto_neg = 0;

	pause_time = xgmac_read(port, XGMAC_MAC_PAUSE_TIME_REG);
	mac_info->tx_pause_time = pause_time;

	port_mode.u32 = xgmac_read(port, XGMAC_PORT_MODE_REG);
	mac_info->port_en = port_mode.bits.tx_mode && port_mode.bits.rx_mode;

	mac_info->duplex = 1;
	mac_info->speed = MAC_SPEED_10000;

	pause_ctrl.u32 = xgmac_read(port, XGMAC_MAC_PAUSE_CTRL_REG);
	mac_info->rx_pause_en = pause_ctrl.bits.tx_pause_en;
	mac_info->tx_pause_en = pause_ctrl.bits.rx_pause_en;

	return 0;
}

/**
 *xgmac_get_pausefrm_cfg - get xgmac pause param
 *@mac_drv: mac driver
 *@uwrxpauseen:xgmac rx pause enable
 *@uwtxpauseen:xgmac tx pause enable
 *return status
 */
int xgmac_get_pausefrm_cfg(void *mac_drv, u32 *rx_pause_en, u32 *tx_pause_en)
{
	union xgmac_mac_pause_ctrl pause_ctrl;
	u32 port = 0;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;

	pause_ctrl.u32 = xgmac_read(port, XGMAC_MAC_PAUSE_CTRL_REG);
	*rx_pause_en = pause_ctrl.bits.rx_pause_en;
	*tx_pause_en = pause_ctrl.bits.tx_pause_en;
	return 0;
}

/**
 *xgmac_get_link_status - get xgmac link status
 *@mac_drv: mac driver
 *@link_stat:xgmac link stat
 *return status
 */
int xgmac_get_link_status(void *mac_drv, u32 *link_stat)
{
	u32 port = 0;

	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;
	*link_stat = xgmac_read(port, XGMAC_LINK_STATUS_REG);
	return 0;
}

/**
 *xgmac_get_regs - dump xgmac regs
 *@mac_drv: mac driver
 *@cmd:ethtool cmd
 *@data:data for value of regs
 *return status
 */
void xgmac_get_regs(void * mac_drv, struct ethtool_regs *cmd, void *data)
{
	u32 i = 0;
	u32 port = 0;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;
	u32 *p = data;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;
	for (i = 0; i < XGMAC_DUMP_NUM; i++)
		p[i] = xgmac_read(port, i * 4);
}

/**
 *xgmac_get_ethtool_stats - get xgmac statistic
 *@mac_drv: mac driver
 *@cmd:ethtool cmd
 *@data:data for value of stats regs
 *return status
 */
void xgmac_get_ethtool_stats(struct mac_device *mac_dev,
	struct ethtool_stats *cmd, u64 *data)
{
	u64 *p = data;
	struct mac_hw_stats *hw_stats = &mac_dev->hw_stats;

	xgmac_update_stats(mac_dev);

	p[0] = hw_stats->tx_fragment_err;
	p[1] = hw_stats->tx_undersize;
	p[2] = hw_stats->tx_under_min_pkts;
	p[3] = hw_stats->tx_64bytes;
	p[4] = hw_stats->tx_65to127;
	p[5] = hw_stats->tx_128to255;
	p[6] = hw_stats->tx_256to511;
	p[7] = hw_stats->tx_512to1023;
	p[8] = hw_stats->tx_1024to1518;
	p[9] = hw_stats->tx_1519tomax;

	p[10] = hw_stats->tx_1519tomax_good;
	p[11] = hw_stats->tx_oversize;
	p[12] = hw_stats->tx_jabber_err;
	p[13] = hw_stats->tx_good_pkts;
	p[14] = hw_stats->tx_good_bytes;
	p[15] = hw_stats->tx_total_pkts;
	p[16] = hw_stats->tx_total_bytes;
	p[17] = hw_stats->tx_uc_pkts;
	p[18] = hw_stats->tx_mc_pkts;
	p[19] = hw_stats->tx_bc_pkts;

	p[20] = hw_stats->tx_pfc_tc0;
	p[21] = hw_stats->tx_pfc_tc1;
	p[22] = hw_stats->tx_pfc_tc2;
	p[23] = hw_stats->tx_pfc_tc3;
	p[24] = hw_stats->tx_pfc_tc4;
	p[25] = hw_stats->tx_pfc_tc5;
	p[26] = hw_stats->tx_pfc_tc6;
	p[27] = hw_stats->tx_pfc_tc7;
	p[28] = hw_stats->tx_ctrl;
	p[29] = hw_stats->tx_1731_pkts;

	p[30] = hw_stats->tx_1588_pkts;
	p[31] = hw_stats->rx_good_from_sw;
	p[32] = hw_stats->rx_bad_from_sw;
	p[33] = hw_stats->tx_bad_pkts;

	p[34] = hw_stats->rx_fragment_err;
	p[35] = hw_stats->rx_undersize;
	p[36] = hw_stats->rx_under_min;
	p[37] = hw_stats->rx_64bytes;
	p[38] = hw_stats->rx_65to127;
	p[39] = hw_stats->rx_128to255;

	p[40] = hw_stats->rx_256to511;
	p[41] = hw_stats->rx_512to1023;
	p[42] = hw_stats->rx_1024to1518;
	p[43] = hw_stats->rx_1519tomax;
	p[44] = hw_stats->rx_1519tomax_good;
	p[45] = hw_stats->rx_oversize;
	p[46] = hw_stats->rx_jabber_err;
	p[47] = hw_stats->rx_good_pkts;
	p[48] = hw_stats->rx_good_bytes;
	p[49] = hw_stats->rx_total_pkts;

	p[50] = hw_stats->rx_total_bytes;
	p[51] = hw_stats->rx_uc_pkts;
	p[52] = hw_stats->rx_mc_pkts;
	p[53] = hw_stats->rx_bc_pkts;
	p[54] = hw_stats->rx_pfc_tc0;
	p[55] = hw_stats->rx_pfc_tc1;
	p[56] = hw_stats->rx_pfc_tc2;
	p[57] = hw_stats->rx_pfc_tc3;
	p[58] = hw_stats->rx_pfc_tc4;
	p[59] = hw_stats->rx_pfc_tc5;

	p[60] = hw_stats->rx_pfc_tc6;
	p[61] = hw_stats->rx_pfc_tc7;
	p[62] = hw_stats->rx_unknown_ctrl;
	p[63] = hw_stats->tx_good_to_sw;
	p[64] = hw_stats->tx_bad_to_sw;
	p[65] = hw_stats->rx_1731_pkts;
	p[66] = hw_stats->rx_symbol_err;
	p[67] = hw_stats->rx_fcs_err;
}

/**
 *xgmac_get_strings - get xgmac strings name
 *@mac_drv: mac driver
 *@stringset: type of values in data,0-selftest,1-ststistics,5-dump regs
 *@data:data for value of string name
 *return status
 */
void xgmac_get_strings(void *mac_drv, u32 stringset, u8 *data)
{
	char *buff = (char *)data;

	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_bad_pkts_minto64");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_good_pkts_minto64");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_total_pkts_minto64");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pkts_64");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pkts_65TO127");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pkts_128TO256");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pkts_256TO512");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pkts_512TO1024");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pkts_1024TO1519");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pkts_1519TOmax");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_good_pkts_1519TOmax");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_good_pkts_untralmax");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_bad_pkts_untralmax");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_good_pkts_ALL");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_good_byte_ALL");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_total_pkt");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_total_byt");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_UC_pkt");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_MC_pkt");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_BC_pkt");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pause_frame_num");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pfc_per_1pause_framer");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pfc_per_2pause_framer");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pfc_per_3pause_framer");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pfc_per_4pause_framer");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pfc_per_5pause_framer");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pfc_per_6pause_framer");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_pfc_per_7pause_framer");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_MAC_CTROL_FRAME");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_1731_PKTS");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_1588_PKTS");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_GOOD_PKT_FROM_DSAF");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_BAD_PKT_FROM_DSAF");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_BAD_PKT_64TOMAX");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_NOT_WELL_PKT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_GOOD_WELL_PKT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_TOTAL_PKT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PKT_64");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PKT_65TO128");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PKT_128TO256");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PKT_256TO512");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PKT_512TO1024");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PKT_1024TO1518");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PKT_1519TOMAX");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_GOOD_PKT_1519TOMAX");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_GOOD_PKT_UNTRAMAX");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_BAD_PKT_UNTRAMAX");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_GOOD_PKT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_GOOD_BYT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PKT");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_BYT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_UC_PKT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_MC_PKT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_BC_PKT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PAUSE_FRAME_NUM");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PFC_PER_1PAUSE_FRAME");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PFC_PER_2PAUSE_FRAME");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PFC_PER_3PAUSE_FRAME");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PFC_PER_4PAUSE_FRAME");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PFC_PER_5PAUSE_FRAME");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PFC_PER_6PAUSE_FRAME");
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_PFC_PER_7PAUSE_FRAME");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_MAC_CONTROL");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_GOOD_PKT_TODSAF");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_TX_BAD_PKT_TODSAF");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_1731_PKT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_SYMBOL_ERR_PKT");
	buff = buff + ETH_GSTRING_LEN;
	snprintf(buff, ETH_GSTRING_LEN, "XGMAC_RX_FCS_PKT");

}

/**
 *xgmac_get_sset_count - get xgmac strings name count
 *@mac_drv: mac driver
 *@stringset: type of values in data,0-selftest,1-ststistics,5-dump regs
 *@data:data for value of string name
 *return xgmac strings name count
 */
int xgmac_get_sset_count(void *mac_drv, u32 stringset)
{
	u32 num = 0;

	if (ETH_STATIC_REG == stringset)
		num = XGMAC_MIB_NUM;
	else if (ETH_DUMP_REG == stringset)
		num = XGMAC_DUMP_NUM;

	return num;
}

/**
 *xgmac_get_total_pkts - get xgmac total packets
 *@mac_drv: mac driver
 *@tx_pkt: tx pkts number
 *@rx_pkt: rx pkts number
 */
void xgmac_get_total_pkts(void *mac_drv, u32 *tx_pkt, u32 *rx_pkt)
{
	u32 port = 0;
	struct mac_driver *drv = (struct mac_driver *)mac_drv;

	port = drv->chip_id * XGMAC_PORT_NUM + drv->mac_id;
	*tx_pkt = xgmac_mib_read(port, XGMAC_TX_TOTAL_PKTS);
	*rx_pkt = xgmac_mib_read(port, XGMAC_RX_TOTAL_PKTS);
}


void * xgmac_config(struct mac_params *mac_param)
{
	struct mac_driver *mac_drv = NULL;

	mac_drv =
		(struct mac_driver *)osal_kmalloc(sizeof(struct mac_driver), GFP_KERNEL);
	if (NULL == mac_drv) {
		log_err(mac_param->dev,	"xgmac_config kmalloc faild!\n");
		return NULL;
	}
	memset(mac_drv, 0, sizeof(struct mac_driver));

	mac_drv->mac_init = xgmac_init;
	mac_drv->mac_enable = xgmac_enable;
	mac_drv->mac_disable = xgmac_disable;

	mac_drv->chip_id = mac_param->chip_id;
	mac_drv->mac_id = mac_param->mac_id;
	mac_drv->mac_mode = mac_param->mac_mode;
	mac_drv->vaddr = mac_param->vaddr;
	mac_drv->dev = mac_param->dev;

	mac_drv->mac_reset = xgmac_reset;
	mac_drv->mac_set_mac_addr = NULL;
	mac_drv->mac_set_an_mode = NULL;
	mac_drv->mac_config_loopback = xgmac_config_loopback;
	mac_drv->mac_config_pad_and_crc = xgmac_config_pad_and_crc;
	mac_drv->mac_config_half_duplex = NULL;
	mac_drv->mac_set_rx_ignore_pause_frames =
		xgmac_set_rx_ignore_pause_frames;
	mac_drv->mac_get_id = xgmac_get_id;
	mac_drv->mac_get_max_frame_length = xgmac_get_max_frame_length;
	mac_drv->mac_dump_regs = xgmac_dump_regs;
	mac_drv->mac_ioctl = NULL;
	mac_drv->mac_free = xgmac_free;
	mac_drv->mac_adjust_link = NULL;
	mac_drv->mac_set_tx_auto_pause_frames = xgmac_set_tx_auto_pause_frames;
	mac_drv->mac_config_max_frame_length = xgmac_config_max_frame_length;
	mac_drv->mac_get_statistics = xgmac_get_statistics;
	mac_drv->mac_pausefrm_cfg = xgmac_pausefrm_cfg;
	mac_drv->autoneg_stat = NULL;
	mac_drv->get_info = xgmac_get_info;
	mac_drv->get_pause_enable = xgmac_get_pausefrm_cfg;
	mac_drv->get_link_status = xgmac_get_link_status;
	mac_drv->get_regs = xgmac_get_regs;
	mac_drv->get_ethtool_stats = xgmac_get_ethtool_stats;
	mac_drv->get_sset_count = xgmac_get_sset_count;
	mac_drv->get_strings = xgmac_get_strings;
	mac_drv->get_total_txrx_pkts = xgmac_get_total_pkts;
	mac_drv->update_stats = xgmac_update_stats;
	mac_drv->clean_stats = xgmac_clean_stats;
	return (void *) mac_drv;
}


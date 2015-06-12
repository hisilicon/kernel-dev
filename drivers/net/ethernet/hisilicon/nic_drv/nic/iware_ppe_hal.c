/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd														 */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfZkl+DIbpA1/rySlMnffoKuThoUczxgB0XRhNTxvkpnoJC4NEBEX8MpWOzT4mmBaqJFx
MyClWXxO1DUdes2L4OUPBX/e7OnhSxfh6CT7lDtp+B8mvuuCWg+rWNBQFngMug==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
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
/*
#include "hrd_module.h"
#include "hrd_typedef.h" */
#include "iware_error.h"
#include "iware_log.h"
#include "iware_ppe_hal.h"

#include <linux/netdevice.h>

void ppe_checksum_hw(struct ppe_device *ppe_dev, u32 value);

/**
 * ppe_set_qid_mode - set ppe qid mode
 * @ppe_common: ppe common device
 * @qid_type: qid mode
 *
 * Return 0 on success, negative on failure
 */
static int ppe_set_qid_mode(struct ppe_common_dev *ppe_common,
			    enum ppe_qid_mode qid_mdoe)
{
	union ppe_cfg_qid_mode qid_mode_reg;

	qid_mode_reg.u32 =
	    ppe_com_read_reg(ppe_common, PPE_COM_CFG_QID_MODE_REG);

	qid_mode_reg.bits.cf_qid_mode = qid_mdoe;

	ppe_com_write_reg(ppe_common, PPE_COM_CFG_QID_MODE_REG,
			  qid_mode_reg.u32);

	return 0;
}

/**
 * ppe_set_qid - set ppe qid
 * @ppe_common: ppe common device
 * @qid: queue id
 *
 * Return 0 on success, negative on failure
 */
static int ppe_set_qid(struct ppe_common_dev *ppe_common, u32 qid)
{
	union ppe_cfg_qid_mode qid_mode;

	qid_mode.u32 = ppe_com_read_reg(ppe_common, PPE_COM_CFG_QID_MODE_REG);

	if (0 == qid_mode.bits.cf_qid_mode) {
		qid_mode.bits.def_qid = qid;
		ppe_com_write_reg(ppe_common, PPE_COM_CFG_QID_MODE_REG,
				  qid_mode.u32);
	} else
		return -EINVAL;

	return 0;
}

/**
 * ppe_set_port_mode - set port mode
 * @ppe_device: ppe device
 * @mode: port mode
 */
static void ppe_set_port_mode(struct ppe_device *ppe_dev, enum ppe_port_mode mode)
{
	ppe_write_reg(ppe_dev, PPE_CFG_XGE_MODE_REG, mode);
}

/**
 * ppe_show_cnt_by_que - show statistics by queue
 * @ppe_common: ppe common device
 * @ring_id: queue id
 *
 * Return 0 on success, negative on failure
 */
int ppe_show_cnt_by_que(struct ppe_common_dev *ppe_common, u32 queue_id)
{
	u32 cnt[4];

	cnt[0] = ppe_com_read_reg(ppe_common,
				  PPE_COM_HIS_TX_PKT_QID_OK_CNT_REG +
				  queue_id * 0x4);

	cnt[1] = ppe_com_read_reg(ppe_common,
				  PPE_COM_HIS_TX_PKT_QID_ERR_CNT_REG +
				  queue_id * 0x4);

	cnt[2] = ppe_com_read_reg(ppe_common,
				  PPE_COM_HIS_RX_PKT_QID_OK_CNT_REG +
				  queue_id * 0x4);

	cnt[3] = ppe_com_read_reg(ppe_common,
				  PPE_COM_HIS_RX_PKT_QID_DROP_CNT_REG +
				  queue_id * 0x4);

	osal_printf("Ring %d ppe tx count	 :%d \r\n", queue_id, cnt[0]);
	osal_printf("Ring %d ppe txdrop count :%d \r\n", queue_id, cnt[1]);
	osal_printf("Ring %d ppe rx count	 :%d \r\n", queue_id, cnt[2]);
	osal_printf("Ring %d ppe rxdrop count :%d \r\n", queue_id, cnt[3]);

	return 0;
}

/**
 * ppe_clr_cnt_by_que - clear statistics by queue
 * @ppe_common: ppe common device
 * @ring_id: queue id
 *
 * Return 0 on success, negative on failure
 */
void ppe_clr_cnt_by_que(struct ppe_common_dev *ppe_common, u32 queue_id)
{
	ppe_com_write_reg(ppe_common,
			  (PPE_COM_HIS_TX_PKT_QID_OK_CNT_REG + queue_id * 0x4),
			  1);

	ppe_com_write_reg(ppe_common,
			  (PPE_COM_HIS_TX_PKT_QID_ERR_CNT_REG + queue_id * 0x4),
			  1);

	ppe_com_write_reg(ppe_common,
			  (PPE_COM_HIS_RX_PKT_QID_OK_CNT_REG + queue_id * 0x4),
			  1);

	ppe_com_write_reg(ppe_common,
			  (PPE_COM_HIS_RX_PKT_QID_DROP_CNT_REG +
			   queue_id * 0x4), 1);
}

/**
 * ppe_show_stat_by_port_hw - show statistics by port
 * @ppe_device: ppe device
 */
static void ppe_show_stat_by_port_hw(struct ppe_device *ppe_dev)
{
	u32 cnt[13];
	u32 port = ppe_dev->index + (PPE_NUM_PER_CHIP * ppe_dev->chip_id);
	struct ppe_hw_stats *hw_stats = &ppe_dev->hw_stats;

	cnt[0] = hw_stats->rx_pkts_from_sw;
	cnt[1] = hw_stats->rx_pkts;
	cnt[2] = hw_stats->rx_drop_no_bd;
	cnt[3] = hw_stats->tx_bd_form_rcb;
	cnt[4] = hw_stats->tx_pkts_from_rcb;
	cnt[5] = hw_stats->tx_pkts;
	cnt[6] = hw_stats->tx_err_fifo_empty;
	cnt[7] = hw_stats->tx_err_checksum;
	cnt[8] = hw_stats->rx_alloc_buf_fail;
	cnt[9] = hw_stats->rx_alloc_buf_wait;
	cnt[10] = hw_stats->rx_drop_no_buf;
	cnt[11] = hw_stats->rx_err_fifo_full;

	osal_printf("Port %d ppe(rx) get from xge count	:%d \r\n", port,
		    cnt[0]);
	osal_printf("Port %d ppe(rx) sent to rcb count	 :%d \r\n", port,
		    cnt[1]);
	osal_printf("Port %d ppe(rx) drop count			:%d \r\n", port,
		    cnt[2]);
	osal_printf("Port %d ppe(rx) request buf fail count:%d \r\n", port,
		    cnt[8]);
	osal_printf("Port %d ppe(rx) request buf wait count:%d \r\n", port,
		    cnt[9]);
	osal_printf("Port %d ppe(rx) full drop count	   :%d \r\n", port,
		    cnt[10]);
	osal_printf("Port %d ppe(rx) full cut count		:%d \r\n", port,
		    cnt[11]);
	osal_printf("Port %d ppe(tx) get from rcb bd count :%d \r\n", port,
		    cnt[3]);
	osal_printf("Port %d ppe(tx) get from rcb pktount  :%d \r\n", port,
		    cnt[4]);
	osal_printf("Port %d ppe(tx) sent to xge count	 :%d \r\n", port,
		    cnt[5]);
	osal_printf("Port %d ppe(tx) fifo empty cut count  :%d \r\n", port,
		    cnt[6]);
	osal_printf("Port %d ppe(tx) checksum fail count   :%d \r\n", port,
		    cnt[7]);
}

/**
 * ppe_clr_cnt_by_port - clear statistics by port
 * @ppe_device: ppe device
 */
void ppe_clr_cnt_by_port(struct ppe_device *ppe_dev)
{
	ppe_write_reg(ppe_dev, PPE_HIS_RX_SW_PKT_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_RX_WR_BD_OK_PKT_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_RX_PKT_NO_BUF_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_TX_BD_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_TX_PKT_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_TX_PKT_OK_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_TX_PKT_EPT_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_TX_PKT_CS_FAIL_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_RX_APP_BUF_FAIL_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_RX_APP_BUF_WAIT_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_RX_PKT_DROP_FUL_CNT_REG, 1);
	ppe_write_reg(ppe_dev, PPE_HIS_RX_PKT_DROP_PRT_CNT_REG, 1);
}

/**
 * ppe_init_hw - init ppe
 * @ppe_device: ppe device
 *
 * Return 0 on success, negative on failure
 */
static int ppe_init_hw(struct ppe_device *ppe_dev)
{
	int ret;
	u32 port = ppe_dev->index + (PPE_NUM_PER_CHIP * ppe_dev->chip_id);
	union ppe_cnt_clr_ce clrce;

	log_dbg(&ppe_dev->netdev->dev, "func begin ppe_index%d\n", port);

	if (ppe_dev->comm_index) {
		ret = HRD_Dsaf_PpeSrstByPort(5 + ppe_dev->comm_index + port, 0);
		osal_mdelay(10);
		log_dbg(&ppe_dev->netdev->dev,
			"(5 + ppe_dev->comm_index + port) = %d\n",
			(5 + ppe_dev->comm_index + port));
		ret = HRD_Dsaf_PpeSrstByPort(5 + ppe_dev->comm_index + port, 1);
	} else {
		ret = HRD_Dsaf_PpeSrstByPort(port, 0);
		osal_mdelay(10);
		ret = HRD_Dsaf_PpeSrstByPort(port, 1);
	}


	if (ret) {
		log_err(&ppe_dev->netdev->dev,
			"HRD_Dsaf_PpeSrstByPort failed! ret=%d\n", ret);
		return ret;
	}

	if (PPE_COMMON_MODE_SINGLE == ppe_dev->ppe_mode)
		ppe_set_port_mode(ppe_dev, PPE_MODE_GE);
	else
		ppe_set_port_mode(ppe_dev, PPE_MODE_XGE);

	log_dbg(&ppe_dev->netdev->dev,
		">>. ppe_dev->ppe_mode = %d ppe_index%d\n",
		ppe_dev->ppe_mode, port);

	ppe_checksum_hw(ppe_dev, 0xffffffff);

	clrce.u32 = ppe_read_reg(ppe_dev, PPE_TNL_0_5_CNT_CLR_CE_REG);
	clrce.bits.cnt_clr_ce = 1;
	ppe_write_reg(ppe_dev, PPE_TNL_0_5_CNT_CLR_CE_REG, clrce.u32);

	return 0;
}

/**
 * ppe_uninit_hw - uninit ppe
 * @ppe_device: ppe device
 */
static void ppe_uninit_hw(struct ppe_device *ppe_dev)
{
	int ret;
	u32 port = ppe_dev->index;

	ret = HRD_Dsaf_PpeSrstByPort(port, 0);
	if (ret)
		log_err(&ppe_dev->netdev->dev,
			"HRD_Dsaf_PpeSrstByPort failed! ret=%d\n", ret);
}

void ppe_update_stats(struct ppe_device *ppe_dev)
{
	struct ppe_hw_stats *hw_stats = &ppe_dev->hw_stats;

	hw_stats->rx_pkts_from_sw
		+= ppe_read_reg(ppe_dev, PPE_HIS_RX_SW_PKT_CNT_REG);
	hw_stats->rx_pkts
		+= ppe_read_reg(ppe_dev, PPE_HIS_RX_WR_BD_OK_PKT_CNT_REG);
	hw_stats->rx_drop_no_bd
		+= ppe_read_reg(ppe_dev, PPE_HIS_RX_PKT_NO_BUF_CNT_REG);
	hw_stats->rx_alloc_buf_fail
		+= ppe_read_reg(ppe_dev, PPE_HIS_RX_APP_BUF_FAIL_CNT_REG);
	hw_stats->rx_alloc_buf_wait
		+= ppe_read_reg(ppe_dev, PPE_HIS_RX_APP_BUF_WAIT_CNT_REG);
	hw_stats->rx_drop_no_buf
		+= ppe_read_reg(ppe_dev, PPE_HIS_RX_PKT_DROP_FUL_CNT_REG);
	hw_stats->rx_err_fifo_full
		+= ppe_read_reg(ppe_dev, PPE_HIS_RX_PKT_DROP_PRT_CNT_REG);

	hw_stats->tx_bd_form_rcb
		+= ppe_read_reg(ppe_dev, PPE_HIS_TX_BD_CNT_REG);
	hw_stats->tx_pkts_from_rcb
		+= ppe_read_reg(ppe_dev, PPE_HIS_TX_PKT_CNT_REG);
	hw_stats->tx_pkts
		+= ppe_read_reg(ppe_dev, PPE_HIS_TX_PKT_OK_CNT_REG);
	hw_stats->tx_err_fifo_empty
		+= ppe_read_reg(ppe_dev, PPE_HIS_TX_PKT_EPT_CNT_REG);
	hw_stats->tx_err_checksum
		+= ppe_read_reg(ppe_dev, PPE_HIS_TX_PKT_CS_FAIL_CNT_REG);
}

/**
 * ppe_get_ethtool_status - get ppe hardware statistics
 * @ppe_device: ppe device
 * @cmd: cmd
 * @data: statistics data
 */
void ppe_get_ethtool_status(struct ppe_device *ppe_dev,
			    struct ethtool_stats *cmd, u64 *data)
{
	u64 *regs_buff = data;
	struct ppe_hw_stats *hw_stats = &ppe_dev->hw_stats;

	ppe_update_stats(ppe_dev);

	regs_buff[0] = hw_stats->rx_pkts_from_sw;
	regs_buff[1] = hw_stats->rx_pkts;
	regs_buff[2] = hw_stats->rx_drop_no_bd;
	regs_buff[3] = hw_stats->tx_bd_form_rcb;
	regs_buff[4] = hw_stats->tx_pkts_from_rcb;
	regs_buff[5] = hw_stats->tx_pkts;
	regs_buff[6] = hw_stats->tx_err_fifo_empty;
	regs_buff[7] = hw_stats->tx_err_checksum;
	regs_buff[8] = hw_stats->rx_alloc_buf_fail;
	regs_buff[9] = hw_stats->rx_alloc_buf_wait;
	regs_buff[10] = hw_stats->rx_drop_no_buf;
	regs_buff[11] = hw_stats->rx_err_fifo_full;
}

/**
 * ppe_get_regs - get ppe register
 * @ppe_device: ppe device
 * @cmd: cmd
 * @data: statistics data
 */
void ppe_get_regs(struct ppe_device *ppe_dev,
		   struct ethtool_regs *cmd, void *data)
{
	u32 *regs_buff = data;
	u32 i;

	/* PPE_TNL_0_5  0x0 - 0x334  total 205 */
	for (i = 0; i < 205; i++)
		regs_buff[i] = ppe_read_reg(ppe_dev, i * 4);

	for (i = 205; i < (205 + 3); i++)
		regs_buff[i] = 0x5a5a5a5a;
}

/**
 * ppe_get_sset_count - get ppe srting set count
 * @ppe_device: ppe device
 * @stringset: string set type
 *
 * Return string set count
 */
int ppe_get_sset_count(struct ppe_device *ppe_dev, u32 stringset)
{
	if (ETH_DUMP_REG == stringset)
		return 208;
	else if (ETH_STATIC_REG == stringset)
		return 12;

	return 0;
}

/**
 * ppe_get_strings - get ppe srting
 * @ppe_device: ppe device
 * @stringset: string set type
 * @data: output string
 */
void ppe_get_strings(struct ppe_device *ppe_dev, u32 stringset, u8 *data)
{
	char *buff = (char *)data;
	int index = ppe_dev->index;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_RX_SW_PKT", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_RX_WR_BD_OK", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_RX_PKT_NO_BUF", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_TX_BD", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_TX_PKT", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_TX_PKT_OK", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_TX_PKT_EPT", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_TX_PKT_CS_FAIL", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_RX_APP_BUF_FAIL", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_RX_APP_BUF_WAIT", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_RRX_PKT_DROP_FUL", index);
	buff = buff + ETH_GSTRING_LEN;

	snprintf(buff, ETH_GSTRING_LEN, "PPE%d_RX_PKT_DROP_PRT", index);
}

/**
 * ppe_checksum_hw - set ppe checksum caculate
 * @ppe_device: ppe device
 * @value: value
 */
void ppe_checksum_hw(struct ppe_device *ppe_dev, u32 value)
{
	value = ppe_read_reg(ppe_dev, PPE_CFG_PRO_CHECK_EN_REG);
	value |= 0xfffffff;
	ppe_write_reg(ppe_dev, PPE_CFG_PRO_CHECK_EN_REG, value);
}

/**
 * ppe_dump_regs - dump ppe register
 * @ppe_device: ppe device
 */
void ppe_dump_regs(struct ppe_device *ppe_dev)
{
	u32 i;

	osal_printf("********************   ppe   *******");
	osal_printf("**************************\n");

	/* TBD */
	for (i = 0; i < 204; i += 4) {
		osal_printf("%#18x :     %#08x     %#08x     %#08x     %#08x\n",
			    0xc5000000 + 0x10000 * ppe_dev->index + i * 4,
			    ppe_read_reg(ppe_dev, i * 4),
			    ppe_read_reg(ppe_dev, (i + 1) * 4),
			    ppe_read_reg(ppe_dev, (i + 2) * 4),
			    ppe_read_reg(ppe_dev, (i + 3) * 4));
	}
	osal_printf("\n");
}

/**
 * ppe_set_ops - set ppe ops
 * @ops: ppe ops
 */
void ppe_set_ops(struct ppe_ops *ops)
{
	ops->init = ppe_init_hw;
	ops->uninit = ppe_uninit_hw;
	ops->show_stat_by_port = ppe_show_stat_by_port_hw;
	ops->checksum_hw = ppe_checksum_hw;
	ops->get_ethtool_stats = ppe_get_ethtool_status;
	ops->get_regs = ppe_get_regs;
	ops->get_sset_count = ppe_get_sset_count;
	ops->get_strings = ppe_get_strings;
	ops->dump_regs = ppe_dump_regs;
}

/*****************************************************************************
****************************PPE COMMON****************************************
*****************************************************************************/

/**
 * ppe_common_init_hw - init ppe common device
 * @ppe_common: ppe common device
 *
 * Return 0 on success, negative on failure
 */
static int ppe_common_init_hw(struct ppe_common_dev *ppe_common)
{
	int ret;
	enum ppe_qid_mode qid_mode;

	log_dbg(ppe_common->dev, "func begin\n");

	if (PPE_COMMON_MODE_MULTI == ppe_common->ppe_mode) {
		ret = HRD_Dsaf_PpeComSrst(ppe_common->comm_index, 0);
		osal_mdelay(100);
		ret = HRD_Dsaf_PpeComSrst(ppe_common->comm_index, 1);
		osal_mdelay(100);
		if (ret) {
			log_err(ppe_common->dev,
				"HRD_Dsaf_PpeComSrst failed! ret=%d\n", ret);
			return ret;
		}

		switch (ppe_common->dsaf_mode) {
		case DSAF_MODE_ENABLE_FIX:
		case DSAF_MODE_DISABLE_FIX:
			qid_mode = PPE_QID_MODE0;
			(void)ppe_set_qid(ppe_common, 0);
			break;
		case DSAF_MODE_ENABLE_0VM:
		case DSAF_MODE_DISABLE_2PORT_64VM:
			qid_mode = PPE_QID_MODE3;
			break;
		case DSAF_MODE_ENABLE_8VM:
		case DSAF_MODE_DISABLE_2PORT_16VM:
			qid_mode = PPE_QID_MODE4;
			break;
		case DSAF_MODE_ENABLE_16VM:
		case DSAF_MODE_DISABLE_6PORT_0VM:
			qid_mode = PPE_QID_MODE5;
			break;
		case DSAF_MODE_ENABLE_32VM:
		case DSAF_MODE_DISABLE_6PORT_16VM:
			qid_mode = PPE_QID_MODE2;
			break;
		case DSAF_MODE_ENABLE_128VM:
		case DSAF_MODE_DISABLE_6PORT_4VM:
			qid_mode = PPE_QID_MODE1;
			break;
		case DSAF_MODE_DISABLE_2PORT_8VM:
			qid_mode = PPE_QID_MODE7;
			break;
		case DSAF_MODE_DISABLE_6PORT_2VM:
			qid_mode = PPE_QID_MODE6;
			break;
		default:
			log_err(ppe_common->dev,
				"get ppe queue mode failed! dsaf_mode=%d\n",
				ppe_common->dsaf_mode);
			return HRD_COMMON_ERR_INPUT_INVALID;
		}
		(void)ppe_set_qid_mode(ppe_common, qid_mode);
	} else {
		ret = HRD_Dsaf_PpeComSrst(ppe_common->comm_index, 0);
		if (ret) {
			log_err(ppe_common->dev,
				"HRD_Dsaf_PpeComSrst failed! ret=%d\n", ret);
			return ret;
		}
		osal_mdelay(10);
		ret = HRD_Dsaf_PpeComSrst(ppe_common->comm_index, 1);
		if (ret) {
			log_err(ppe_common->dev,
				"HRD_Dsaf_PpeComSrst failed! ret=%d\n", ret);
			return ret;
		}
	}

	return 0;
}

/**
 * ppe_common_uninit_hw - uninit ppe common device
 * @ppe_common: ppe common device
 */
static void ppe_common_uninit_hw(struct ppe_common_dev *ppe_common)
{
	/* int ret; */

	log_dbg(ppe_common->dev, "func begin\n");
#if 0				/* TBD */
	ret = HRD_Dsaf_PpeComSrst(0);
	if (ret)
		log_err(ppe_common->dev,
			"HRD_Dsaf_PpeComSrst failed! ret=%d\n", ret);
#endif
}

/**
 * ppe_set_common_ops - set ppe common ops
 * @ops: ppe common ops
 */
void ppe_set_common_ops(struct ppe_common_ops *ops)
{
	ops->init = ppe_common_init_hw;
	ops->uninit = ppe_common_uninit_hw;
}

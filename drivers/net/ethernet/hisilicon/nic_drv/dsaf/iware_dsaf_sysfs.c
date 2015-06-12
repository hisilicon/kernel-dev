/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe48/i6HdQpKryBUZCpUEzBiy1IK5tlwcO5HwmUiXpw1cjNIM3mk0jzd2Qlxy8mRJB5xyi7
MJ1cNVmr8FFBcj0a6krUGRok6yEuOUA4UQIpouuzd8ayEQNLBHxM7RgCqUfZ/w==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/*******************************************************************************

  Hisilicon DSAF SYSFS - dsaf dfx
  Copyright(c) 2014 - 2019 Huawei Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2,  as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful,  but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not,  write to the Free Software Foundation,  Inc.,
  51 Franklin St - Fifth Floor,  Boston,  MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:TBD

*******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/slab.h>

#include "iware_error.h"

#include "iware_dsaf_main.h"
#include "iware_dsaf_sysfs.h"
#include "iware_dsaf_drv_hw.h"
#include "iware_dsaf_reg_define.h"
#include "iware_dsaf_reg_offset.h"

/**
 * dsaf_print_2hex_format - print fun
 * @print_num: print num
 * @data: print data
 */
void dsaf_print_2hex_format(u32 print_num, u32 data)
{
	u32  i;

	for (i = 0; i < print_num; i++) {
		if (data & 0x1)
			osal_pr("1|");
		else
			osal_pr("0|");
		data = data >> 1;
	}
	osal_pr("\r\n");
}

/**
 * dsaf_print_char_format - print fun
 * @print_num: print num
 * @mod_val: print data
 */
void dsaf_print_char_format(u32 print_num, u32 mod_val)
{
	u32  i;
	char c = '0';
	u32 mod = 0;

	for (i = 0; i < print_num; i++) {
		mod = i / mod_val;
		osal_pr("%c|", c + (i - mod_val * mod));
	}
	osal_pr("\r\n");
}

/**
 * dsaf_pfc_en_show - show	pfc_en
 * @dsaf_id: dsa fabric id
 */
void dsaf_pfc_en_show(u32 dsaf_id)
{
	u32 pfc_en[DSAF_COMM_CHN] = {0};
	u32 *ppfc_en = pfc_en;
	u32 i;

	for (i = 0; i < DSAF_COMM_CHN; i++)
		pfc_en[i] = dsaf_read(dsaf_id, DSAF_PFC_EN_0_REG + 0x0004 * (u64)i);

	osal_pr("dsaf pfc enable state : \r\n");

	osal_pr("	 tc : ");
	dsaf_print_char_format(8, 10);

	for (i = 0; i < DSAF_COMM_CHN; i++) {
		osal_pr("channel%d : ", i);
		dsaf_print_2hex_format(8, *ppfc_en);
		ppfc_en++;
	}
}

/**
 * dsaf_pfc_en_show - show	sram_init_over
 * @dsaf_id: dsa fabric id

 */
void dsaf_sram_init_over_show(u32 dsaf_id)
{
	u32 sram_init_over;

	sram_init_over = dsaf_read(dsaf_id, DSAF_SRAM_INIT_OVER_0_REG);

	osal_pr("dsaf sram init over state : \r\n");

	osal_pr("channel : ");
	dsaf_print_char_format(8, 10);

	osal_pr("		 ");
	dsaf_print_2hex_format(8, sram_init_over);
}

/**
 * dsaf_comm_state_show - show	comm
 * @dsaf_id: dsa fabric id
 */
void dsaf_comm_state_show(u32 dsaf_id)
{
	u32 i;
	u32 fsm_timeout;
	u32 abnormal_timeout;
	union dsaf_cfg_reg dsaf_cfg;
	union dsaf_fabric_reg_cnt_clr_ce reg_cnt_clr_ce;

	u32 stp_port_type[DSAF_XGE_CHN] = {0};
	union dsaf_stp_port_type_reg stp_port_type_reg;

	u32 ppe_qid_cfg[DSAF_PPE_TX_NUM] = {0};
	u32 mix_def_qid[DSAF_PPE_TX_NUM] = {0};
	union dsaf_ppe_qid_cfg ppe_qid_cfg_reg;
	union dsaf_mix_def_qid mix_def_qid_reg;

	u32 sw_port_type[DSAF_SW_PORT_NUM] = {0};
	u32 port_def_vlan[DSAF_SW_PORT_NUM] = {0};
	union dsaf_sw_port_type_reg sw_port_type_reg;
	union dsaf_port_def_vlan port_def_vlan_reg;

	dsaf_cfg.u32 = dsaf_read(dsaf_id, DSAF_CFG_0_REG);
	reg_cnt_clr_ce.u32 = dsaf_read(dsaf_id, DSAF_DSA_REG_CNT_CLR_CE_REG);
	osal_pr("dsaf comm state: \r\n");
	osal_pr(
		"dsaf_en|tc_mode|crc_en|mix_mode|stp_mode|snap_en|cnt_clr\r\n");
	osal_pr(
		"    %d   |   %d   |   %d  |   %d    |   %d    |   %d   |   %d   \r\n",
		dsaf_cfg.bits.dsaf_en, dsaf_cfg.bits.dsaf_tc_mode,
		dsaf_cfg.bits.dsaf_crc_en, dsaf_cfg.bits.dsaf_mix_mode,
		dsaf_cfg.bits.dsaf_stp_mode, reg_cnt_clr_ce.bits.snap_en,
		reg_cnt_clr_ce.bits.cnt_clr_ce);
	osal_pr("local_addr_en|\r\n");
	osal_pr("       %d      |\r\n", dsaf_cfg.bits.dsaf_local_addr_en);

	fsm_timeout = dsaf_read(dsaf_id, DSAF_FSM_TIMEOUT_0_REG);
	abnormal_timeout  = dsaf_read(dsaf_id, DSAF_ABNORMAL_TIMEOUT_0_REG);
	osal_pr("-----------------------------------\r\n");
	osal_pr("dsaf fsm_timeout state: 0x%8x\r\n", fsm_timeout);
	osal_pr("dsaf abnormal_timeout state: 0x%8x \r\n",
		abnormal_timeout);

	for (i = 0; i < DSAF_XGE_CHN; i++) {
		stp_port_type_reg.u32 = dsaf_read(dsaf_id,
			DSAF_STP_PORT_TYPE_0_REG + 0x0004 * (u64)i);
		stp_port_type[i] = stp_port_type_reg.bits.dsaf_stp_port_type;
	}

	for (i = 0; i < DSAF_PPE_TX_NUM; i++) {
		ppe_qid_cfg_reg.u32 = dsaf_read(dsaf_id,
			DSAF_PPE_QID_CFG_0_REG + 0x0004 * (u64)i);
		mix_def_qid_reg.u32 = dsaf_read(dsaf_id,
			DSAF_MIX_DEF_QID_0_REG  + 0x0004 * (u64)i);

		if (i > 0)
			ppe_qid_cfg[i] += ppe_qid_cfg_reg.bits.dsaf_ppe_qid_cfg;
		else
			ppe_qid_cfg[i] = ppe_qid_cfg_reg.bits.dsaf_ppe_qid_cfg;

		mix_def_qid[i] = mix_def_qid_reg.bits.dsaf_mix_def_qid;
	}

	osal_pr("-----------------------------------\r\n");
	osal_pr("stp work mode|rstp work mode\r\n");
	osal_pr("0: discard   | discarding   \r\n");
	osal_pr("1: blocking  | learning      \r\n");
	osal_pr("2: listening | forwarding   \r\n");
	osal_pr("3: learning  |  /       \r\n");
	osal_pr("4: forwarding|  /        \r\n");
	osal_pr(
		"stp work state: xge/ge0|xge/ge1|xge/ge2|xge/ge3|xge/ge4|xge/ge5|\r\n");
	osal_pr(
		"                     %d  |   %d   |   %d   |   %d   |   %d   |   %d   |\r\n",
		stp_port_type[0], stp_port_type[1], stp_port_type[2],
		stp_port_type[3], stp_port_type[4], stp_port_type[5]);
	osal_pr("-----------------------------------\r\n");
	osal_pr(
		"                     ppe0  |  ppe1  |  ppe2  |  ppe3  |  ppe4  |  ppe5  |\r\n");
	osal_pr(
		"ppe qid cfg       :%3d-%3d |%3d-%3d |%3d-%3d |%3d-%3d |%3d-%3d |%3d-%3d |\r\n",
		0, ppe_qid_cfg[0] - 1, ppe_qid_cfg[0], ppe_qid_cfg[1] - 1,
		ppe_qid_cfg[1], ppe_qid_cfg[2] - 1, ppe_qid_cfg[2],
		ppe_qid_cfg[3] - 1, ppe_qid_cfg[3], ppe_qid_cfg[4] - 1,
		ppe_qid_cfg[4], ppe_qid_cfg[5] - 1);
	osal_pr(
	   "mix def qid      :  %3d   |  %3d   |  %3d   |  %3d   |  %3d   |  %3d   |\r\n",
	   mix_def_qid[0], mix_def_qid[1], mix_def_qid[2],
	   mix_def_qid[3], mix_def_qid[4], mix_def_qid[5]);
	osal_pr("-----------------------------------\r\n");

	for (i = 0; i < DSAF_SW_PORT_NUM; i++) {
		sw_port_type_reg.u32 = dsaf_read(dsaf_id,
			DSAF_SW_PORT_TYPE_0_REG + 0x0004 * (u64)i);
		port_def_vlan_reg.u32 = dsaf_read(dsaf_id,
			DSAF_PORT_DEF_VLAN_0_REG + 0x0004 * (u64)i);
		sw_port_type[i] = sw_port_type_reg.bits.dsaf_sw_port_type;
		port_def_vlan[i] = port_def_vlan_reg.bits.dsaf_port_def_vlan;
	}
	osal_pr("sw port type: 0-no support vlan; 1-access; 2-truck;\r\n");
	osal_pr(
	   "xge/ge0|xge/ge1|xge/ge2|xge/ge3|xge/ge4|xge/ge5|   ppe | rocee |\r\n");
	osal_pr(
	   "   %d   |   %d   |   %d   |   %d   |   %d   |   %d   |   %d   |   %d   |\r\n",
	   sw_port_type[0], sw_port_type[1], sw_port_type[2],
	   sw_port_type[3], sw_port_type[4], sw_port_type[5],
	   sw_port_type[6], sw_port_type[7]);
	osal_pr("port def vlan:\r\n");
	osal_pr(
	   "0x%3x  | 0x%3x | 0x%3x | 0x%3x | 0x%3x | 0x%3x | 0x%3x | 0x%3x |\r\n",
	   port_def_vlan[0], port_def_vlan[1], port_def_vlan[2],
	   port_def_vlan[3], port_def_vlan[4], port_def_vlan[5],
	   port_def_vlan[6], port_def_vlan[7]);
}


/**
 * dsaf_ecc_err_state_show - show ecc err
 * @dsaf_id: dsa fabric id
 */
void dsaf_ecc_err_state_show(u32 dsaf_id)
{
	u32 i;
	u32 reg_val;

	/*xid ecc state var*/
	union dsaf_inode_ecc_invert_en o_inode_sram_ecc_invert_en;
	union dsaf_inode_ecc_err_addr o_inode_ecc_err_addr;
	u32 xid_ecc_err_invert_en;
	u32 xid_ecc_2bit_addr;

	/*sbm ecc state var*/
	union dsaf_sbm_cfg_reg o_sbm_cfg_reg;

	/*voq ecc state var*/
	union dsaf_voq_ecc_invert_en o_voq_sram_ecc_invert_en;
	union dsaf_voq_ecc_err_addr o_voq_ecc_err_addr;

	/*tbl ecc state var*/
	union dsaf_tbl_dfx_ctrl o_tbl_ctrl;
	union dsaf_tbl_dfx_stat o_tbl_dfx_stat;

	/*ecc int var*/
	union dsaf_xge_int_src o_xge_int_src;
	union dsaf_xge_int_msk o_xge_int_msk;
	union dsaf_xge_int_sts o_xge_int_sts;
	union dsaf_ppe_int_src o_ppe_int_src;
	union dsaf_ppe_int_msk o_ppe_int_msk;
	union dsaf_ppe_int_sts o_ppe_int_sts;
	union dsaf_rocee_int_src rocee_int_src;
	union dsaf_rocee_int_msk rocee_int_msk;
	union dsaf_rocee_int_sts rocee_int_sts;
	union dsaf_tbl_int_src tbl_int_src;
	union dsaf_tbl_int_msk1 tbl_int_msk;
	union dsaf_tbl_int_sts o_tbl_int_sts;

	/****ecc 1bit err cnt ****/
	osal_pr("ecc cnt sel note: \r\n");
	osal_pr("0 - 5: xge inode sram 1bit ecc err cnt sel\r\n");
	osal_pr("6 -23: sbm data sram 1bit ecc err cnt sel\r\n");
	osal_pr("      : |  xge 0-5  |  ppe 0-5  |  roc 0-5  |\r\n");
	osal_pr("      : |6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|\r\n");
	osal_pr("24-41: sbm link sram 1bit ecc err cnt sel\r\n");
	osal_pr("      : |  xge 0-5  |  ppe 0-5  |  roc 0-5  |\r\n");
	osal_pr("      : |4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|\r\n");
	osal_pr("42-59: voq sram 1bit ecc err cnt sel\r\n");
	osal_pr("      : |  xge 0-5  |  ppe 0-5   |  roc 0-5  |\r\n");
	osal_pr("      : |2|3|4|5|6|7|8|9|10|1|2|3|4|5|6|7|8|9|\r\n");
	osal_pr("    60: node table tcam sram 1bit ecc err cnt sel\r\n");
	osal_pr("    61: node table line sram 1bit ecc err cnt sel\r\n");

	reg_val = dsaf_read(dsaf_id, DSAF_DSA_SRAM_1BIT_ECC_SEL_REG);
	osal_pr("DSAF_DSA_SRAM_1BIT_ECC_SEL:%d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id, DSAF_DSA_SRAM_1BIT_ECC_CNT_REG);
	osal_pr("DSA_SRAM_1BIT_ECC_CNT:%d\r\n", reg_val);

	osal_pr("==========\r\n");

	/****ecc err invert show****/
	osal_pr("dsaf ecc error invert bit : \r\n");
	osal_pr("         ");
	dsaf_print_char_format(32, 10);

	osal_pr("%3d_%3d : ", (0 * 32), ((0 + 1) * 32 - 1));
	reg_val = dsaf_read(dsaf_id, DSAF_ECC_ERR_INVERT_0_REG);
	dsaf_print_2hex_format(32, reg_val);

	osal_pr("==========\r\n");

	/****xid ecc state show****/
	osal_pr("dsaf xid ecc state : \r\n");
	osal_pr("chn|ecc en|2bit err addr|\r\n");

	for (i = 0; i < DSAF_XGE_NUM; i++) {
		o_inode_sram_ecc_invert_en.u32 = dsaf_read(dsaf_id,
			DSAF_INODE_ECC_INVERT_EN_0_REG + 0x80 * (u64)i);
		o_inode_ecc_err_addr.u32 = dsaf_read(dsaf_id,
			DSAF_INODE_ECC_ERR_ADDR_0_REG + 0x80 * (u64)i);

		xid_ecc_err_invert_en =
		o_inode_sram_ecc_invert_en.bits.inode_sram_ecc_invert_en;
		xid_ecc_2bit_addr =
			o_inode_ecc_err_addr.bits.inode_ecc_err_addr;

		osal_pr(" %2d|   %d  |  0x%08x |\r\n", i,
			xid_ecc_err_invert_en, xid_ecc_2bit_addr);
	}
	osal_pr("==========\r\n");

	/****sbm ecc state show****/
	osal_pr("dsaf sbm ecc state : \r\n");
	osal_pr("chn|ecc en|\r\n");

	for (i = 0; i < DSAF_SBM_NUM; i++) {
		o_sbm_cfg_reg.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_CFG_REG_0_REG + 0x80 * (u64)i);
		osal_pr(" %2d|   %d  |\r\n", i,
			o_sbm_cfg_reg.bits.sbm_cfg_ecc_err_invert_en);
	}
	osal_pr("==========\r\n");

	/****voq ecc state show****/
	osal_pr("dsaf voq ecc state : \r\n");
	osal_pr("chn|ecc en|2bit err addr|\r\n");

	for (i = 0; i < DSAF_VOQ_NUM; i++) {
		o_voq_sram_ecc_invert_en.u32 = dsaf_read(dsaf_id,
			DSAF_VOQ_ECC_INVERT_EN_0_REG + 0x80 * (u64)i);
		o_voq_ecc_err_addr.u32 = dsaf_read(dsaf_id,
			DSAF_VOQ_ECC_ERR_ADDR_0_REG + 0x80 * (u64)i);
		osal_pr(" %2d|   %d  |  0x%08x |\r\n", i,
			o_voq_sram_ecc_invert_en.bits.voq_sram_ecc_invert_en,
			o_voq_ecc_err_addr.bits.voq_ecc_err_addr);
	}
	osal_pr("==========\r\n");

	/****tbl ecc state show****/
	osal_pr("dsaf tbl ecc state : \r\n");
	osal_pr("ecc en|t 2bit err addr|l 2bit err addr|\r\n");

	o_tbl_ctrl.u32 = dsaf_read(dsaf_id, DSAF_TBL_DFX_CTRL_0_REG);
	o_tbl_dfx_stat.u32 = dsaf_read(dsaf_id, DSAF_TBL_DFX_STAT_0_REG);
	osal_pr("    %d  |  0x%08x   |  0x%08x   |\r\n",
		o_tbl_ctrl.bits.ram_err_inject_en,
		o_tbl_dfx_stat.bits.tram_ecc_err_addr,
		o_tbl_dfx_stat.bits.lram_ecc_err_addr);
	osal_pr("==========\r\n");

	/****ecc int show****/
	osal_pr("dsaf ecc int show : \r\n");
	osal_pr("chn xge : xid|sbm s|sbm l|voq|\r\n");

	for (i = 0; i < DSAF_XGE_CHN; i++) {
		o_xge_int_src.u32 = dsaf_read(dsaf_id,
			DSAF_XGE_INT_SRC_0_REG + 0x4 * (u64)i);
		o_xge_int_msk.u32 = dsaf_read(dsaf_id,
			DSAF_XGE_INT_MSK_0_REG + 0x4 * (u64)i);
		o_xge_int_sts.u32 = dsaf_read(dsaf_id,
			DSAF_XGE_INT_STS_0_REG + 0x4 * (u64)i);
		osal_pr(" %d    src :  %d |  %d  |  %d  | %d |\r\n",
			i, o_xge_int_src.bits.xid_xge_ecc_err_int_src,
			o_xge_int_src.bits.sbm_xge_sram_ecc_2bit_int_src,
			o_xge_int_src.bits.sbm_xge_lnk_ecc_2bit_int_src,
			o_xge_int_src.bits.voq_xge_ecc_err_int_src);
		osal_pr("    msk :  %d |    %d  |  %d  | %d |\r\n",
			o_xge_int_msk.bits.xid_xge_ecc_err_int_msk,
			o_xge_int_msk.bits.sbm_xge_sram_ecc_2bit_int_msk,
			o_xge_int_msk.bits.sbm_xge_lnk_ecc_2bit_int_msk,
			o_xge_int_msk.bits.voq_xge_ecc_err_int_msk);
		osal_pr("    sts :  %d |    %d  |  %d  | %d |\r\n",
			o_xge_int_sts.bits.xid_xge_ecc_err_int_sts,
			o_xge_int_sts.bits.sbm_xge_sram_ecc_2bit_int_sts,
			o_xge_int_sts.bits.sbm_xge_lnk_ecc_2bit_int_sts,
			o_xge_int_sts.bits.voq_xge_ecc_err_int_sts);
		osal_pr("--------------------------------------\r\n");
	}
	osal_pr("==========\r\n");

	osal_pr("chn ppe : xid|sbm s|sbm l|voq|\r\n");

	for (i = 0; i < DSAF_PPE_CHN; i++) {
		o_ppe_int_src.u32 = dsaf_read(dsaf_id,
			DSAF_PPE_INT_SRC_0_REG + 0x4*(u64)i);
		o_ppe_int_msk.u32 = dsaf_read(dsaf_id,
			DSAF_PPE_INT_MSK_0_REG + 0x4*(u64)i);
		o_ppe_int_sts.u32 = dsaf_read(dsaf_id,
			DSAF_PPE_INT_STS_0_REG + 0x4*(u64)i);
		osal_pr(" %d    src :  / |  %d  |  %d  | %d |\r\n",
			i, o_ppe_int_src.bits.sbm_ppe_sram_ecc_2bit_int_src,
			o_ppe_int_src.bits.sbm_ppe_lnk_ecc_2bit_int_src,
			o_ppe_int_src.bits.voq_ppe_ecc_err_int_src);
		osal_pr("    msk :  / |  %d  |  %d  | %d |\r\n",
			o_ppe_int_msk.bits.sbm_ppe_sram_ecc_2bit_int_msk,
			o_ppe_int_msk.bits.sbm_ppe_lnk_ecc_2bit_int_msk,
			o_ppe_int_msk.bits.voq_ppe_ecc_err_int_msk);
		osal_pr("    sts :  / |  %d  |  %d  | %d |\r\n",
			o_ppe_int_sts.bits.sbm_ppe_sram_ecc_2bit_int_sts,
			o_ppe_int_sts.bits.sbm_ppe_lnk_ecc_2bit_int_sts,
			o_ppe_int_sts.bits.voq_ppe_ecc_err_int_sts);
		osal_pr("--------------------------------------\r\n");
	}
	osal_pr("==========\r\n");

	osal_pr("chn roc : xid|sbm s|sbm l|voq|\r\n");

	for (i = 0; i < DSAF_ROCEE_CHN; i++) {
		rocee_int_src.u32 = dsaf_read(dsaf_id,
			DSAF_ROCEE_INT_SRC_0_REG + 0x4*(u64)i);
		rocee_int_msk.u32 = dsaf_read(dsaf_id,
			DSAF_ROCEE_INT_MSK_0_REG + 0x4*(u64)i);
		rocee_int_sts.u32 = dsaf_read(dsaf_id,
			DSAF_ROCEE_INT_STS_0_REG + 0x4*(u64)i);
		osal_pr(" %d    src :  / |  %d  |  %d  | %d |\r\n",
			i,
			rocee_int_src.bits.sbm_rocee_sram_ecc_2bit_int_src,
			rocee_int_src.bits.sbm_rocee_lnk_ecc_2bit_int_src,
			rocee_int_src.bits.voq_rocee_ecc_err_int_src);
		osal_pr("    msk :  / |  %d  |  %d  | %d |\r\n",
			rocee_int_msk.bits.sbm_rocee_sram_ecc_2bit_int_msk,
			rocee_int_msk.bits.sbm_rocee_lnk_ecc_2bit_int_msk,
			rocee_int_msk.bits.voq_rocee_ecc_err_int_msk);
		osal_pr("    sts :  / |  %d  |  %d  | %d |\r\n",
			rocee_int_sts.bits.sbm_rocee_sram_ecc_2bit_int_sts,
			rocee_int_sts.bits.sbm_rocee_lnk_ecc_2bit_int_sts,
			rocee_int_sts.bits.voq_rocee_ecc_err_int_sts);
		osal_pr("--------------------------------------\r\n");
	}
	osal_pr("==========\r\n");

	osal_pr("tbl : t 1bit err|l 1bit err|t 2bit err|l 2bit err|\r\n");

	tbl_int_src.u32 = dsaf_read(dsaf_id, DSAF_TBL_INT_SRC_0_REG);
	tbl_int_msk.u32 = dsaf_read(dsaf_id, DSAF_TBL_INT_MSK_0_REG);
	o_tbl_int_sts.u32 = dsaf_read(dsaf_id, DSAF_TBL_INT_STS_0_REG);
	osal_pr(
		"src :     %d     |     %d    |     %d    |     %d    |\r\n",
		tbl_int_src.bits.tram_ecc_err1_src,
		tbl_int_src.bits.lram_ecc_err1_src,
		tbl_int_src.bits.tram_ecc_err2_src,
		tbl_int_src.bits.lram_ecc_err2_src);
	osal_pr(
		"msk :     %d     |     %d    |     %d    |     %d    |\r\n",
		tbl_int_msk.bits.tram_ecc_err1_msk,
		tbl_int_msk.bits.lram_ecc_err1_msk,
		tbl_int_msk.bits.tram_ecc_err2_msk,
		tbl_int_msk.bits.lram_ecc_err2_msk);
	osal_pr(
		"sts :     %d     |     %d    |     %d    |     %d    |\r\n",
		o_tbl_int_sts.bits.tram_ecc_err1_sts,
		o_tbl_int_sts.bits.lram_ecc_err1_sts,
		o_tbl_int_sts.bits.tram_ecc_err2_sts,
		o_tbl_int_sts.bits.lram_ecc_err2_sts);
}

/**
 * dsaf_comm_state_show - show
 * @dsaf_id: dsa fabric id
 * @vm_num:
 * @prt_num:
 */
void dsaf_vm_def_vlan_show(u32 dsaf_id, u32 vm_num, u32 prt_num)
{
	u32 i;
	struct dsaf_vm_def_vlan vm_def_vlan[DSAF_PPE_QID_NUM];
	union dsaf_vm_def_vlan_reg vm_def_vlan_reg;

	for (i = 0; i < DSAF_PPE_QID_NUM; i++) {
		vm_def_vlan_reg.u32 = dsaf_read(dsaf_id,
			DSAF_VM_DEF_VLAN_0_REG + 0x0004 * (u64)i);
		vm_def_vlan[i].vm_def_vlan_id =
			vm_def_vlan_reg.bits.dsaf_vm_def_vlan_id;
		vm_def_vlan[i].vm_def_vlan_cfi =
			vm_def_vlan_reg.bits.dsaf_vm_def_vlan_cfi;
		vm_def_vlan[i].vm_def_vlan_pri =
			vm_def_vlan_reg.bits.dsaf_vm_def_vlan_pri;
	}

	osal_pr("vm def vlan:\r\n");
	osal_pr("vm num|vlan_pri|vlan_cfi|vlan_id|\r\n");
	for (i = vm_num; i < vm_num + prt_num; i++) {
		osal_pr("   %3d |    %d   |    %d   | 0x%3x |\r\n", i,
			vm_def_vlan[i].vm_def_vlan_pri,
		vm_def_vlan[i].vm_def_vlan_cfi, vm_def_vlan[i].vm_def_vlan_id);
	}
}

void dsaf_update_stats(struct dsaf_device *dsaf_dev, u32 inode_num)
{
	struct dsaf_inode_hw_stats *hw_stats
		= &dsaf_dev->inode_hw_stats[inode_num];

	hw_stats->pad_drop += dsaf_read(dsaf_dev->chip_id,
		DSAF_INODE_PAD_DISCARD_NUM_0_REG + 0x80 * (u64)inode_num);
	hw_stats->sbm_drop += dsaf_read(dsaf_dev->chip_id,
		DSAF_INODE_SBM_DROP_NUM_0_REG + 0x80 * (u64)inode_num);
	hw_stats->crc_false += dsaf_read(dsaf_dev->chip_id,
		DSAF_INODE_CRC_FALSE_NUM_0_REG + 0x80 * (u64)inode_num);
	hw_stats->bp_drop += dsaf_read(dsaf_dev->chip_id,
		DSAF_INODE_BP_DISCARD_NUM_0_REG + 0x80 * (u64)inode_num);
	hw_stats->rslt_drop += dsaf_read(dsaf_dev->chip_id,
		DSAF_INODE_RSLT_DISCARD_NUM_0_REG + 0x80 * (u64)inode_num);
	hw_stats->local_addr_false += dsaf_read(dsaf_dev->chip_id,
		DSAF_INODE_LOCAL_ADDR_FALSE_NUM_0_REG + 0x80 * (u64)inode_num);
	hw_stats->vlan_drop += dsaf_read(dsaf_dev->chip_id,
		DSAF_INODE_SW_VLAN_TAG_DISC_0_REG + 0x80 * (u64)inode_num);
	hw_stats->stp_drop += dsaf_read(dsaf_dev->chip_id,
		DSAF_INODE_IN_DATA_STP_DISC_0_REG + 0x80 * (u64)inode_num);
}
EXPORT_SYMBOL(dsaf_update_stats);
/**
 * dsaf_inode_stat_show - show inode
 * @dsaf_id: dsa fabric id
 * @pinode_pri_tc_cfg: pointer
 */
void dsaf_inode_stat_show(struct dsaf_device *dsaf_dev, u32 inode_num)
{
	u32 dsaf_id = dsaf_dev->chip_id;
	struct dsaf_inode_hw_stats *hw_stats
		= &dsaf_dev->inode_hw_stats[inode_num];
	u32 inode_in_vc0_pkt_num;
	u32 inode_in_vc1_pkt_num;
	u32 inode_final_in_man_num;
	u32 inode_final_in_pkt_num;
	u32 inode_sbm_pid_num;
	u32 inode_final_in_pause_num;
	u32 inode_sbm_rels_num;
	u32 inode_voq_voer_num;

	dsaf_update_stats(dsaf_dev, inode_num);

	inode_final_in_man_num = dsaf_read(dsaf_id,
		DSAF_INODE_FINAL_IN_MAN_NUM_0_REG + 0x80 * (u64)inode_num);
	inode_final_in_pkt_num = dsaf_read(dsaf_id,
		DSAF_INODE_FINAL_IN_PKT_NUM_0_REG + 0x80 * (u64)inode_num);
	inode_sbm_pid_num = dsaf_read(dsaf_id,
		DSAF_INODE_SBM_PID_NUM_0_REG + 0x80 * (u64)inode_num);
	inode_final_in_pause_num = dsaf_read(dsaf_id,
		DSAF_INODE_FINAL_IN_PAUSE_NUM_0_REG + 0x80 * (u64)inode_num);
	inode_sbm_rels_num = dsaf_read(dsaf_id,
		DSAF_INODE_SBM_RELS_NUM_0_REG + 0x80 * (u64)inode_num);
	inode_voq_voer_num = dsaf_read(dsaf_id,
		DSAF_INODE_VOQ_OVER_NUM_0_REG + 0x80 * (u64)inode_num);
	inode_in_vc0_pkt_num = dsaf_read(dsaf_id,
		DSAF_INODE_VC0_IN_PKT_NUM_0_REG + 0x4 * (u64)inode_num);
	inode_in_vc1_pkt_num = dsaf_read(dsaf_id,
		DSAF_INODE_VC1_IN_PKT_NUM_0_REG + 0x4 * (u64)inode_num);

	osal_pr(
		"\n***************inode %d statistics*******************************\n",
		inode_num);
	osal_pr("inode_in_vc0_pkt_num: %u\r\n",   inode_in_vc0_pkt_num);
	osal_pr("inode_in_vc1_pkt_num: %u\r\n",   inode_in_vc1_pkt_num);
	osal_pr("inode_pad_discard_num: %llu\r\n", hw_stats->pad_drop);
	osal_pr("inode_final_in_man_num: %u\r\n", inode_final_in_man_num);
	osal_pr("inode_final_in_pkt_num: %u\r\n", inode_final_in_pkt_num);
	osal_pr("inode_sbm_pid_num: %u\r\n", inode_sbm_pid_num);
	osal_pr("inode_final_in_pause_num: %u\r\n",
		inode_final_in_pause_num);
	osal_pr("inode_sbm_rels_num: %u\r\n", inode_sbm_rels_num);
	osal_pr("inode_sbm_drop_num: %llu\r\n", hw_stats->sbm_drop);
	osal_pr("inode_crc_false_num: %llu\r\n", hw_stats->crc_false);
	osal_pr("inode_bp_discard_num: %llu\r\n", hw_stats->bp_drop);
	osal_pr("inode_rslt_discard_num: %llu\r\n", hw_stats->rslt_drop);
	osal_pr("inode_local_addr_false_num: %llu\r\n",
		hw_stats->local_addr_false);
	osal_pr("inode_voq_voer_num: %u\r\n", inode_voq_voer_num);
	osal_pr("inode_sw_vlan_tag_drop_num: %llu\r\n", hw_stats->vlan_drop);
	osal_pr("inode_in_data_stp_drop_num: %llu\r\n", hw_stats->stp_drop);
	osal_pr(
		"\n****************inode statistics end******************************\n");
}

/**
 * dsaf_inode_state_show - show inode
 * @dsaf_id: dsa fabric id
 */
void dsaf_inode_state_show(u32 dsaf_id)
{
	u32 i;

	union dsaf_inode_cut_through_cfg inode_cut_through_cfg;
	union dsaf_inode_in_port_num inode_in_port_num;
	union dsaf_inode_bp_status1 inode_bp_status;
	union dsaf_inode_pri_tc_cfg inode_pri_tc;
	char mode_name[][4] = {"xge", "ppe", "roc"};

	inode_cut_through_cfg.u32
		= dsaf_read(dsaf_id, DSAF_INODE_CUT_THROUGH_CFG_0_REG);
	osal_pr("inode cut through en : %d\r\n",
		inode_cut_through_cfg.bits.dxi_cut_through_en);
	osal_pr("=======================\r\n");

	osal_pr("chn num|inport|                    tc");
	osal_pr("                            |bp status|\r\n");
	osal_pr("     |  num |pri0|pri1|pri2|pri3|pri4|pri5|");
	osal_pr("pri6|pri7|data|man|t->i|i->t|\r\n");
	for (i = 0; i < DSAF_INODE_NUM; i++) {
		inode_in_port_num.u32 = dsaf_read(dsaf_id,
			DSAF_INODE_IN_PORT_NUM_0_REG + 0x80 * (u64)i);
		inode_bp_status.u32 = dsaf_read(dsaf_id,
			DSAF_INODE_BP_STATUS_0_REG + 0x80 * (u64)i);
		inode_pri_tc.u32 = dsaf_read(dsaf_id,
			DSAF_INODE_PRI_TC_CFG_0_REG + 0x80 * (u64)i);

		osal_pr("%s    %d |  %d   |  %d |  %d |  %d |  %d |  %d |",
			mode_name[i/DSAF_XGE_NUM], i%DSAF_XGE_NUM,
			inode_in_port_num.bits.inode_in_port_num,
			inode_pri_tc.bits.inode_pri0_tc,
			inode_pri_tc.bits.inode_pri1_tc,
			inode_pri_tc.bits.inode_pri2_tc,
			inode_pri_tc.bits.inode_pri3_tc,
			inode_pri_tc.bits.inode_pri4_tc);
		osal_pr("  %d |  %d |  %d |  %d | %d |  %d |  %d |\r\n",
			inode_pri_tc.bits.inode_pri5_tc,
			inode_pri_tc.bits.inode_pri6_tc,
			inode_pri_tc.bits.inode_pri7_tc,
			inode_pri_tc.bits.inode_no_vlan_tag_data_tc,
			inode_pri_tc.bits.inode_no_vlan_tag_man_tc,
			inode_bp_status.bits.tbl_inode_bp_status,
			inode_bp_status.bits.inode_tbl_bp_status);
	}
}

/**
 * dsaf_sbm_stat_show - get inner sbm
 * @dsaf_id: dsa fabric id
 * @sbm_num: value
 */
void dsaf_sbm_stat_show(u32 dsaf_id, u32 sbm_num)
{
	u32 reg_val;

	union dsaf_sbm_free_cnt_0 sbm_free_cnt_0;
	union dsaf_sbm_free_cnt_1 sbm_free_cnt_1;
	union dsaf_sbm_bp_cnt_0	sbm_bp_cnt_0;
	union dsaf_sbm_bp_cnt_1	sbm_bp_cnt_1;
	union dsaf_sbm_bp_cnt_2	sbm_bp_cnt_2;

	sbm_free_cnt_0.u32 = dsaf_read(dsaf_id,
		DSAF_SBM_FREE_CNT_0_0_REG + 0x80 * (u64)sbm_num);
	sbm_free_cnt_1.u32 = dsaf_read(dsaf_id,
		DSAF_SBM_FREE_CNT_1_0_REG + 0x80 * (u64)sbm_num);
	sbm_bp_cnt_0.u32 = dsaf_read(dsaf_id,
		DSAF_SBM_BP_CNT_0_0_REG + 0x80 * (u64)sbm_num);
	sbm_bp_cnt_1.u32 = dsaf_read(dsaf_id,
		DSAF_SBM_BP_CNT_0_1_REG + 0x80 * (u64)sbm_num);
	sbm_bp_cnt_2.u32 = dsaf_read(dsaf_id,
		DSAF_SBM_BP_CNT_0_2_REG + 0x80 * (u64)sbm_num);

	osal_pr("\n*******************sbm %d statistics", sbm_num);
	osal_pr("***************************\n");
	osal_pr("sbm common unused buffer num: %d\r\n",
		sbm_free_cnt_0.bits.sbm_mib_com_free_buf_cnt);
	osal_pr("sbm vc0 free buffer num: %d\r\n",
		sbm_free_cnt_0.bits.sbm_mib_vc0_free_buf_cnt);
	osal_pr("sbm vc1 free buffer num: %d\r\n",
		sbm_free_cnt_0.bits.sbm_mib_vc1_free_buf_cnt);
	osal_pr("sbm tc0 free buffer num: %d\r\n",
		sbm_free_cnt_1.bits.sbm_mib_tc0_free_buf_cnt);
	osal_pr("sbm tc4 free buffer num: %d\r\n",
		sbm_free_cnt_1.bits.sbm_mib_tc4_free_buf_cnt);
	osal_pr("sbm tc0 used buffer num: %d\r\n",
		sbm_bp_cnt_0.bits.sbm_mib_tc0_used_buf_cnt);
	osal_pr("sbm tc1 used buffer num: %d\r\n",
		sbm_bp_cnt_0.bits.sbm_mib_tc1_used_buf_cnt);
	osal_pr("sbm tc2 used buffer num: %d\r\n",
		sbm_bp_cnt_0.bits.sbm_mib_tc2_used_buf_cnt);
	osal_pr("sbm tc3 used buffer num: %d\r\n",
		sbm_bp_cnt_1.bits.sbm_mib_tc3_used_buf_cnt);
	osal_pr("sbm tc4 used buffer num: %d\r\n",
		sbm_bp_cnt_1.bits.sbm_mib_tc4_used_buf_cnt);
	osal_pr("sbm tc5 used buffer num: %d\r\n",
		sbm_bp_cnt_1.bits.sbm_mib_tc5_used_buf_cnt);
	osal_pr("sbm tc6 used buffer num: %d\r\n",
		sbm_bp_cnt_2.bits.sbm_mib_tc6_used_buf_cnt);
	osal_pr("sbm tc7 used buffer num: %d\r\n",
		sbm_bp_cnt_2.bits.sbm_mib_tc7_used_buf_cnt);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_INPORT_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm receive total packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_DROP_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm drop	 total packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_INF_OUTPORT_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm send	 total packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_INPORT_TC0_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm receive tc0 packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_INPORT_TC1_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm receive tc1 packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_INPORT_TC2_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm receive tc2 packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_INPORT_TC3_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm receive tc3 packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_INPORT_TC4_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm receive tc4 packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_INPORT_TC5_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm receive tc5 packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_INPORT_TC6_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm receive tc6 packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_INPORT_TC7_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm receive tc7 packet num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_REQ_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm request buffer total num: %d\r\n", reg_val);

	reg_val = dsaf_read(dsaf_id,
		DSAF_SBM_LNK_RELS_CNT_0_REG + 0x80 * (u64)sbm_num);
	osal_pr("sbm release buffer total num: %d\r\n", reg_val);

	osal_pr("\n********************sbm statistics end");
	osal_pr("**************************\n");
}

/**
 * dsaf_sbm_state_show - show  sbm
 * @dsaf_id: dsa fabric id
 * @sbm_num: value
 * @psbm_stat: pointer
 */
void dsaf_sbm_state_show(u32 dsaf_id)
{
	u32 i;
	union dsaf_sbm_inf_fifo_thrd dsaf_sbm_inf_fifo_thrd;
	union dsaf_sbm_cfg_reg sbm_cfg_reg;
	union dsaf_sbm_bp_cfg_0_reg sbm_bp_cfg_0_reg;
	union dsaf_sbm_bp_cfg_1_reg sbm_bp_cfg_1_reg;
	union dsaf_sbm_bp_cfg_2_reg sbm_bp_cfg_2_reg;
	union dsaf_sbm_bp_cfg_3_reg sbm_bp_cfg_3_reg;
	union dsaf_sbm_bp_cfg_4_reg sbm_bp_cfg_4_reg;

	union dsaf_sbm_free_cnt_0  sbm_free_cnt_0;
	union dsaf_sbm_free_cnt_1 sbm_free_cnt_1;
	union dsaf_sbm_bp_cnt_0 sbm_bp_cnt_0;
	union dsaf_sbm_bp_cnt_1 sbm_bp_cnt_1;
	union dsaf_sbm_bp_cnt_2 sbm_bp_cnt_2;
	union dsaf_sbm_bp_cnt_3 sbm_bp_cnt_3;

	union dsaf_sbm_iner_st sbm_iner_st;
	union dsaf_sbm_mib_req_failed sbm_req_failed_tc;

	osal_pr("====sbm cfg state====\n");

	dsaf_sbm_inf_fifo_thrd.u32 = dsaf_read(dsaf_id,
		DSAF_DSA_SBM_INF_FIFO_THRD_REG);
	osal_pr("dsaf_sbm_inf_fifo_aful_thrd:0x%x\n",
		dsaf_sbm_inf_fifo_thrd.bits.dsaf_sbm_inf_fifo_aful_thrd);
	osal_pr("dsaf_sbm_inf_fifo_sread_thrd:0x%x\n",
		dsaf_sbm_inf_fifo_thrd.bits.dsaf_sbm_inf_fifo_sread_thrd);

	osal_pr("sbm| enable cfg |  max buf num cfg     |");
	osal_pr("           bp buf num cfg       |\n");
	osal_pr("   |          |                        |");
	osal_pr("    pfc   | no pfc   |  pause   |\n");
	osal_pr("num|mib|en|shcut|com |vc0 |vc1 |tc0 |tc4 |");
	osal_pr("reset|set |reset|set |reset|set |\n");
	for (i = 0; i < DSAF_SBM_NUM; i++) {
		sbm_cfg_reg.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_CFG_REG_0_REG + 0x80 * (u64)i);
		sbm_bp_cfg_0_reg.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_0_XGE_REG_0_REG + 0x80 * (u64)i);
		sbm_bp_cfg_1_reg.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_1_REG_0_REG + 0x80 * (u64)i);
		sbm_bp_cfg_2_reg.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_2_XGE_REG_0_REG + 0x80 * (u64)i);
		sbm_bp_cfg_3_reg.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_3_REG_0_REG + 0x80 * (u64)i);
		sbm_bp_cfg_4_reg.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_4_REG_0_REG + 0x80 * (u64)i);

		osal_pr("%2d | %d | %d|  %d  |%4d|%4d|%4d|%4d|",
			i, sbm_cfg_reg.bits.sbm_cfg_mib_en,
			sbm_cfg_reg.bits.sbm_cfg_en,
			sbm_cfg_reg.bits.sbm_cfg_shcut_en,
			sbm_bp_cfg_0_reg.bits.sbm_cfg_com_max_buf_num,
			sbm_bp_cfg_0_reg.bits.sbm_cfg_vc0_max_buf_num,
			sbm_bp_cfg_0_reg.bits.sbm_cfg_vc1_max_buf_num,
			sbm_bp_cfg_1_reg.bits.sbm_cfg_tc0_max_buf_num);
		osal_pr("%4d|%4d |%4d|%4d |%4d|%4d |%4d|\n",
			sbm_bp_cfg_1_reg.bits.sbm_cfg_tc4_max_buf_num,
			sbm_bp_cfg_2_reg.bits.sbm_cfg_reset_buf_num,
			sbm_bp_cfg_2_reg.bits.sbm_cfg_set_buf_num,
			sbm_bp_cfg_3_reg.bits.sbm_cfg_reset_buf_num_no_pfc,
			sbm_bp_cfg_3_reg.bits.sbm_cfg_set_buf_num_no_pfc,
			sbm_bp_cfg_4_reg.bits.sbm_cfg_reset_buf_num_pause,
			sbm_bp_cfg_4_reg.bits.sbm_cfg_set_buf_num_pause);
	}

	osal_pr("====sbm buf state show====\n");
	osal_pr("sbm|    mib free buf num    |        ");
	osal_pr("mib used buf num             |\n");
	osal_pr("num|com |vc0 |vc1 |tc0 |tc4 |tc0 |tc1 ");
	osal_pr("|tc2 |tc3 |tc4 |tc5 |tc6 |tc7 |\n");
	for (i = 0; i < DSAF_SBM_NUM; i++) {
		sbm_free_cnt_0.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_FREE_CNT_0_0_REG + 0x80 * (u64)i);
		sbm_free_cnt_1.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_FREE_CNT_1_0_REG + 0x80 * (u64)i);
		sbm_bp_cnt_0.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CNT_0_0_REG + 0x80 * (u64)i);
		sbm_bp_cnt_1.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CNT_1_0_REG + 0x80 * (u64)i);
		sbm_bp_cnt_2.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CNT_2_0_REG + 0x80 * (u64)i);

		osal_pr("%2d |%4d|%4d|%4d|%4d|%4d|%4d|%4d",
			i, sbm_free_cnt_0.bits.sbm_mib_com_free_buf_cnt,
			sbm_free_cnt_0.bits.sbm_mib_vc0_free_buf_cnt,
			sbm_free_cnt_0.bits.sbm_mib_vc1_free_buf_cnt,
			sbm_free_cnt_1.bits.sbm_mib_tc0_free_buf_cnt,
			sbm_free_cnt_1.bits.sbm_mib_tc4_free_buf_cnt,
			sbm_bp_cnt_0.bits.sbm_mib_tc0_used_buf_cnt,
			sbm_bp_cnt_0.bits.sbm_mib_tc1_used_buf_cnt);
		osal_pr("|%4d|%4d|%4d|%4d|%4d|%4d|\n",
			sbm_bp_cnt_0.bits.sbm_mib_tc2_used_buf_cnt,
			sbm_bp_cnt_1.bits.sbm_mib_tc3_used_buf_cnt,
			sbm_bp_cnt_1.bits.sbm_mib_tc4_used_buf_cnt,
			sbm_bp_cnt_1.bits.sbm_mib_tc5_used_buf_cnt,
			sbm_bp_cnt_2.bits.sbm_mib_tc6_used_buf_cnt,
			sbm_bp_cnt_2.bits.sbm_mib_tc7_used_buf_cnt);
	}

	osal_pr("====sbm bp state show====\n");
	osal_pr("   |    xge bp   |    dxi bp     |\n");
	osal_pr("sbm|vc0 tc |vc1 tc |vc0 tc |vc1 tc |\n");
	osal_pr("num|0|1|2|3|0|1|2|3|0|1|2|3|0|1|2|3|\n");

	for (i = 0; i < DSAF_SBM_NUM; i++) {
		sbm_bp_cnt_3.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CNT_3_0_REG + 0x80 * (u64)i);

		osal_pr("%2d |", i);
		dsaf_print_2hex_format(16, sbm_bp_cnt_3.u32);
	}

	osal_pr("====sbm inner state show====\n");
	osal_pr("sbm| mib buf status|lnk request|req failed|\n");
	osal_pr("num|release|request|   status  |  tc num  |\n");
	for (i = 0; i < DSAF_SBM_NUM; i++) {
		sbm_iner_st.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_INER_ST_0_REG + 0x80 * (u64)i);
		sbm_req_failed_tc.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_MIB_REQ_FAILED_TC_0_REG  + 0x80 * (u64)i);
		osal_pr("%2d |  %2d |  %2d   |     %2d    |     %d    |\n",
			i, sbm_iner_st.bits.sbm_mib_rels_crt_st,
			sbm_iner_st.bits.sbm_mib_req_crt_st,
			sbm_iner_st.bits.sbm_lnk_crt_st,
			sbm_req_failed_tc.bits.sbm_mib_req_failed_tc);
	}
}

/**
 * dsaf_xod_ets_bw_ost_cfg - show xod
 * @dsaf_id: dsa fabric id
 */
void dsaf_xod_ets_cfg_show(u32 dsaf_id)
{
	u32 i;
	union dsaf_xod_ets_tsa_tc0_tc3_cfg o_xod_ets_tsa_tc0_tc3;
	union dsaf_xod_ets_tsa_tc4_tc7_cfg o_xod_ets_tsa_tc4_tc7;
	union dsaf_xod_ets_bw_tc0_tc3_cfg o_xod_ets_bw_tc0_tc3;
	union dsaf_xod_ets_bw_tc4_tc7_cfg o_xod_ets_bw_tc4_tc7;
	union dsaf_xod_ets_bw_offset_cfg o_xod_ets_bw_ost;
	union dsaf_xod_ets_token_cfg o_xod_ets_token_cfg;

	osal_pr("====xod ets cfg show====\r\n");
	osal_pr("xod|            tsa              |     |\r\n");
	osal_pr("num|tc0|tx1|tc2|tc3|tc4|tc5|tc6|tc7|token|\r\n");
	for (i = 0; i < DSAF_XOD_NUM; i++) {
		o_xod_ets_tsa_tc0_tc3.u32 = dsaf_read(dsaf_id,
			DSAF_XOD_ETS_TSA_TC0_TC3_CFG_0_REG + 0x90 * (u64)i);
		o_xod_ets_tsa_tc4_tc7.u32 = dsaf_read(dsaf_id,
			DSAF_XOD_ETS_TSA_TC4_TC7_CFG_0_REG + 0x90 * (u64)i);
		o_xod_ets_token_cfg.u32 = dsaf_read(dsaf_id,
			DSAF_XOD_ETS_TOKEN_CFG_0_REG);
		osal_pr(" %d |%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%4d |\r\n", i,
			o_xod_ets_tsa_tc0_tc3.bits.xod_tsa_tc0,
			o_xod_ets_tsa_tc0_tc3.bits.xod_tsa_tc1,
			o_xod_ets_tsa_tc0_tc3.bits.xod_tsa_tc2,
			o_xod_ets_tsa_tc0_tc3.bits.xod_tsa_tc3,
			o_xod_ets_tsa_tc4_tc7.bits.xod_tsa_tc4,
			o_xod_ets_tsa_tc4_tc7.bits.xod_tsa_tc5,
			o_xod_ets_tsa_tc4_tc7.bits.xod_tsa_tc6,
			o_xod_ets_tsa_tc4_tc7.bits.xod_tsa_tc7,
			o_xod_ets_token_cfg.bits.xod_ets_token_thr);
	}

	osal_pr("-------------------------------------------");
	osal_pr("-------------------------\r\n");
	osal_pr("xod|             bw              |");
	osal_pr("            bw off             |\r\n");
	osal_pr("num|tc0|tx1|tc2|tc3|tc4|tc5|tc6|tc7|tc0|");
	osal_pr("tx1|tc2|tc3|tc4|tc5|tc6|tc7|\r\n");
	for (i = 0; i < DSAF_XOD_NUM; i++) {
		o_xod_ets_bw_tc0_tc3.u32 = dsaf_read(dsaf_id,
			DSAF_XOD_ETS_BW_TC0_TC3_CFG_0_REG + 0x90 * (u64)i);
		o_xod_ets_bw_tc4_tc7.u32 = dsaf_read(dsaf_id,
			DSAF_XOD_ETS_BW_TC4_TC7_CFG_0_REG + 0x90 * (u64)i);
		o_xod_ets_bw_ost.u32 = dsaf_read(dsaf_id,
			DSAF_XOD_ETS_BW_OFFSET_CFG_0_REG + 0x90 * (u64)i);
		osal_pr(" %d |%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d",
			i, o_xod_ets_bw_tc0_tc3.bits.xod_ets_bw_tc0,
			o_xod_ets_bw_tc0_tc3.bits.xod_ets_bw_tc1,
			o_xod_ets_bw_tc0_tc3.bits.xod_ets_bw_tc2,
			o_xod_ets_bw_tc0_tc3.bits.xod_ets_bw_tc3,
			o_xod_ets_bw_tc4_tc7.bits.xod_ets_bw_tc4,
			o_xod_ets_bw_tc4_tc7.bits.xod_ets_bw_tc5,
			o_xod_ets_bw_tc4_tc7.bits.xod_ets_bw_tc6,
			o_xod_ets_bw_tc4_tc7.bits.xod_ets_bw_tc7,
			o_xod_ets_bw_ost.bits.xod_ets_bw_ost_tc0);
		osal_pr("|%3d|%3d|%3d|%3d|%3d|%3d|%3d|\r\n",
			o_xod_ets_bw_ost.bits.xod_ets_bw_ost_tc1,
			o_xod_ets_bw_ost.bits.xod_ets_bw_ost_tc2,
			o_xod_ets_bw_ost.bits.xod_ets_bw_ost_tc3,
			o_xod_ets_bw_ost.bits.xod_ets_bw_ost_tc4,
			o_xod_ets_bw_ost.bits.xod_ets_bw_ost_tc5,
			o_xod_ets_bw_ost.bits.xod_ets_bw_ost_tc6,
			o_xod_ets_bw_ost.bits.xod_ets_bw_ost_tc7);
	}
}

/**
 * dsaf_xod_pause_para_cfg_show - show xod
 * @dsaf_id: dsa fabric id
 */
void dsaf_xod_pause_para_cfg_show(u32 dsaf_id)
{
	u32 i;
	u32 o_xod_cf_pause_sa_l;
	union dsaf_xod_pfs_cfg_0 o_xod_cf_pause_sa_h;
	union dsaf_xod_pfs_cfg_2 o_xod_cf_pause_para;
	u32 pfc_unit_cnt;

	pfc_unit_cnt = dsaf_read(dsaf_id, DSAF_PFC_UNIT_CNT_0_REG);

	osal_pr("====xod pause para cfg show====\r\n");
	osal_pr("pfc unit cnt : %3d\r\n", pfc_unit_cnt);
	osal_pr("xge num|pause sa h|pause sa l|pause thr|tx timer|\r\n");

	for (i = 0; i < DSAF_XGE_NUM; i++) {
		o_xod_cf_pause_sa_h.u32 = dsaf_read(dsaf_id,
			DSAF_XOD_PFS_CFG_0_0_REG + 0x0090 * (u64)i);
		o_xod_cf_pause_sa_l = dsaf_read(dsaf_id,
			DSAF_XOD_PFS_CFG_1_0_REG + 0x0090 * (u64)i);
		o_xod_cf_pause_para.u32 = dsaf_read(dsaf_id,
			DSAF_XOD_PFS_CFG_2_0_REG + 0x0090 * (u64)i);

		osal_pr(
			"   %d   |  0x%4X  |0x%8X|  0x%4X | 0x%4X |\r\n",
			i, o_xod_cf_pause_sa_h.bits.xod_cf_pause_sa_h,
			o_xod_cf_pause_sa_l,
			o_xod_cf_pause_para.bits.xod_cf_pause_thr,
			o_xod_cf_pause_para.bits.xod_cf_tx_timer);
	}
}

/**
 * dsaf_xod_grant_signle_show - show xod
 * @dsaf_id: dsa fabric id
 */
void dsaf_xod_grant_signle_show(u32 dsaf_id)
{
	u32 i;

	u32 grant_l;
	u32 grant_h;

	osal_pr("====xod grant single show====\r\n");
	osal_pr("xod| xge0 input tc | xge1 input tc |");
	osal_pr(" xge2 input tc | xge3 input tc |\r\n");
	osal_pr("chn|");
	dsaf_print_char_format(32, 8);

	for (i = 0; i < DSAF_XOD_BIG_NUM; i++) {
		grant_l = dsaf_read(dsaf_id, DSAF_XOD_GNT_L_0_REG + 0x90 * (u64)i);
		osal_pr(" %2d|", i);
		dsaf_print_2hex_format(32, grant_l);
	}

	osal_pr("------------------------------\r\n");
	osal_pr("xod| xge4 input tc | xge5 input tc |");
	osal_pr(" ppe input tc  | roce input tc |\r\n");
	osal_pr("chn|");
	dsaf_print_char_format(32, 8);
	for (i = 0; i < DSAF_XOD_BIG_NUM; i++) {
		grant_h = dsaf_read(dsaf_id, DSAF_XOD_GNT_H_0_REG + 0x90 * (u64)i);
		osal_pr(" %2d|", i);
		dsaf_print_2hex_format(32, grant_h);
	}
}

/**
 * dsaf_xod_connect_state_show -  show
 * @dsaf_id: dsa fabric id
 */
void dsaf_xod_connect_state_show(u32 dsaf_id)
{
	u32 i;
	u32 connect_state;

	osal_pr("====xod connect state show====\r\n");
	osal_pr("   |1:connect      |0:chn0     |0:tc0-tc3|0:bs|\r\n");
	osal_pr("xod|    xge    |p|c|xge->accchn|   tc  |v|idle|\r\n");
	osal_pr("chn|0|1|2|3|4|5|6|7|0|1|2|3|4|5|0|1|2|3|4|    |\r\n");

	for (i = 0; i < DSAF_XOD_BIG_NUM; i++) {
		connect_state = dsaf_read(dsaf_id,
			DSAF_XOD_CONNECT_STATE_0_REG + 0x90 * (u64)i);
		osal_pr(" %2d|", i);
		dsaf_print_2hex_format(20, connect_state);
	}
}

/**
 * dsaf_xod_fifo_state_show -  show
 * @dsaf_id: dsa fabric id

 */
void dsaf_xod_fifo_state_show(u32 dsaf_id)
{
	u32 i;
	union dsaf_xod_fifo_status o_xod_ppe_fifo_state;

	osal_pr("====xod fifo state show====\n");
	osal_pr("       |     ppe fifo     |     roc fifo     |\n");
	osal_pr("xod chn|empty|full|al full|empty|full|al full|\n");

	for (i = 0; i < DSAF_XOD_BIG_NUM; i++) {
		o_xod_ppe_fifo_state.u32 = dsaf_read(dsaf_id,
			DSAF_XOD_CONNECT_STATE_0_REG + 0x90 * (u64)i);
		osal_pr(
			"  %2d   |  %d  | %d  |   %d   |  %d  | %d  |   %d   |\n",
			i, o_xod_ppe_fifo_state.bits.xod_ppe_fifo_empty,
			o_xod_ppe_fifo_state.bits.xod_ppe_fifo_full,
			o_xod_ppe_fifo_state.bits.xod_ppe_fifo_pfull,
			o_xod_ppe_fifo_state.bits.xod_rocee_fifo_empty,
			o_xod_ppe_fifo_state.bits.xod_rocee_fifo_full,
			o_xod_ppe_fifo_state.bits.xod_rocee_fifo_pfull);
	}
}

/**
 * dsaf_xod_stat_show -  show
 * @dsaf_id: dsa fabric id
 */
void dsaf_xod_stat_show(u32 dsaf_id, u32 xod_num)
{
	u32 reg;

	osal_pr("*******************xod %d statistics\n", xod_num);
	osal_pr("***************************\n");

	reg = dsaf_read(dsaf_id, DSAF_XOD_RCVPKT_CNT_0_REG + 0x90 * (u64)xod_num);
	osal_pr("send total packet num: %d\r\n", reg);
	reg = dsaf_read(dsaf_id, DSAF_XOD_RCVTC0_CNT_0_REG + 0x90 * (u64)xod_num);
	osal_pr("send tc0 packet num: %d\r\n", reg);
	reg = dsaf_read(dsaf_id, DSAF_XOD_RCVTC1_CNT_0_REG + 0x90 * (u64)xod_num);
	osal_pr("send tc1 packet num: %d\r\n", reg);
	reg = dsaf_read(dsaf_id, DSAF_XOD_RCVTC2_CNT_0_REG + 0x90 * (u64)xod_num);
	osal_pr("send tc2 packet num: %d\r\n", reg);
	reg = dsaf_read(dsaf_id, DSAF_XOD_RCVTC3_CNT_0_REG + 0x90 * (u64)xod_num);
	osal_pr("send tc3 packet num: %d\r\n", reg);
	reg = dsaf_read(dsaf_id, DSAF_XOD_RCVVC0_CNT_0_REG + 0x90 * (u64)xod_num);
	osal_pr("send vc0 packet num: %d\r\n", reg);
	reg = dsaf_read(dsaf_id, DSAF_XOD_RCVVC1_CNT_0_REG + 0x90 * (u64)xod_num);
	osal_pr("send vc1 packet num: %d\r\n", reg);

	if (xod_num <= 5) {
		osal_pr("*******************xge%d statistics", xod_num);
		osal_pr("***************************\n");

		reg = dsaf_read(dsaf_id,
			DSAF_XOD_XGE_RCVIN0_CNT_0_REG + 0x90 * (u64)xod_num);
		osal_pr("(receive xge0 input/send pri0 pfc) packet num: %d\n",
			reg);
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_XGE_RCVIN1_CNT_0_REG + 0x90 * (u64)xod_num);
		osal_pr("(receive xge1 input/send pri1 pfc) packet num: %d\n",
			reg);
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_XGE_RCVIN2_CNT_0_REG + 0x90 * (u64)xod_num);
		osal_pr("(receive xge2 input/send pri2 pfc) packet num: %d\n",
			reg);
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_XGE_RCVIN3_CNT_0_REG + 0x90 * (u64)xod_num);
		osal_pr("(receive xge3 input/send pri3 pfc) packet num: %d\n",
			reg);
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_XGE_RCVIN4_CNT_0_REG + 0x90 * (u64)xod_num);
		osal_pr("(receive xge4 input/send pri4 pfc) packet num: %d\n",
			reg);
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_XGE_RCVIN5_CNT_0_REG + 0x90 * (u64)xod_num);
		osal_pr("(receive xge5 input/send pri5 pfc) packet num: %d\n",
			reg);
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_XGE_RCVIN6_CNT_0_REG + 0x90 * (u64)xod_num);
		osal_pr("(receive ppe%d input/send pri6 pfc) packet num: %d\n",
			xod_num, reg);
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_XGE_RCVIN7_CNT_0_REG + 0x90 * (u64)xod_num);
		osal_pr("(receive roce%d input/send pri7 pfc) packet num: %d\n",
			xod_num, reg);
	}

	if (xod_num >= 6 && xod_num <= 11) {
		osal_pr("*******************ppe%d statistics", (xod_num - 6));
		osal_pr("***************************\n");
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_PPE_RCVIN0_CNT_0_REG + 0x90 * (u64)(xod_num - 6));
		osal_pr("receive xge%d input packet num: %d\n",
			(xod_num - 6), reg);
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_PPE_RCVIN1_CNT_0_REG + 0x90 * (u64)(xod_num - 6));
		osal_pr("receive ppe%d input packet num: %d\n",
			(xod_num - 6), reg);
	}

	if (xod_num >= 12 && xod_num <= 17) {
		osal_pr("\n*******************rocee%d statistics",
			(xod_num - 12));
		osal_pr("***************************\n");
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_ROCEE_RCVIN0_CNT_0_REG + 0x90*(u64)(xod_num - 12));
		osal_pr("receive xge%d input packet num: %d\n",
			(xod_num - 12), reg);
		reg = dsaf_read(dsaf_id,
			DSAF_XOD_ROCEE_RCVIN1_CNT_0_REG + 0x90*(u64)(xod_num - 12));
		osal_pr("receive rocee%d input packet num: %d\n",
			(xod_num - 12), reg);
	}
}

/**
 * dsaf_voq_xod_req_show -	voq
 * @dsaf_id: dsa fabric id

 */
void dsaf_voq_xod_req_show(u32 dsaf_id)
{
	u32 i;
	u32 voq_xge_xod_req0;
	u32 voq_xge_xod_req1;
	u32 voq_ppe_xod_req;
	u32 voq_roc_xod_req;

	osal_pr("xge-voq receive xod request signal show:\r\n");
	osal_pr("xge|  xge0 xod tc  |  xge1 xod tc  |");
	osal_pr("  xge2 xod tc  |  xge3 xod tc  |\r\n");
	osal_pr("chn|");
	dsaf_print_char_format(32, 8);
	for (i = 0; i < DSAF_XGE_VOQ_NUM; i++) {
		voq_xge_xod_req0 = dsaf_read(dsaf_id,
			DSAF_VOQ_XGE_XOD_REQ_0_0_REG + 0x40 * (u64)i);
		osal_pr(" %d |", i);
		dsaf_print_2hex_format(32, voq_xge_xod_req0);
	}

	osal_pr("xge|  xge4 xod tc  |  xge5 xod tc  |");
	osal_pr("  xge6 xod tc  |  xge7 xod tc  |\r\n");
	osal_pr("chn|");
	dsaf_print_char_format(32, 8);
	for (i = 0; i < DSAF_XGE_VOQ_NUM; i++) {
		voq_xge_xod_req1 = dsaf_read(dsaf_id,
			DSAF_VOQ_XGE_XOD_REQ_1_0_REG + 0x40 * (u64)i);
		osal_pr(" %d |", i);
		dsaf_print_2hex_format(32, voq_xge_xod_req1);
	}
	osal_pr("----------------------------------------\r\n");

	osal_pr("ppe-voq receive xod request signal show:\r\n");
	osal_pr("ppe|  xge xod tc   |  ppe xod tc   |\r\n");
	osal_pr("chn|");
	dsaf_print_char_format(16, 8);
	for (i = 0; i < DSAF_PPE_VOQ_NUM; i++) {
		voq_ppe_xod_req = dsaf_read(dsaf_id,
			DSAF_VOQ_PPE_XOD_REQ_0_REG + 0x40 * (u64)i);
		osal_pr(" %d |", i);
		dsaf_print_2hex_format(16, voq_ppe_xod_req);
	}
	osal_pr("----------------------------------------\r\n");

	osal_pr("roc-voq receive xod request signal show:\r\n");
	osal_pr("roc|  xge xod tc   |  roc xod tc   |\r\n");
	osal_pr("chn|");
	dsaf_print_char_format(16, 8);
	for (i = 0; i < DSAF_ROC_VOQ_NUM; i++) {
		voq_roc_xod_req = dsaf_read(dsaf_id,
			DSAF_VOQ_ROCEE_XOD_REQ_0_REG + 0x40 * (u64)i);
		osal_pr(" %d |", i);
		dsaf_print_2hex_format(16, voq_roc_xod_req);
	}
}

/**
 * dsaf_voq_bp_thrd_cfg_show -	voq
 * @dsaf_id: dsa fabric id
 */
void dsaf_voq_bp_thrd_cfg_show(u32 dsaf_id)
{
	u32 i;
	union dsaf_voq_bp_all_thrd1 voq_bp_all_thrd;

	osal_pr("voq bp thrd show:\r\n");
	osal_pr("voq num|all down| all up |\r\n");

	for (i = 0; i < DSAF_VOQ_NUM; i++) {
		voq_bp_all_thrd.u32 = dsaf_read(dsaf_id,
			DSAF_VOQ_BP_ALL_THRD_0_REG + 0x40 * (u64)i);
		osal_pr("   %2d   |  %4d  |  %4d  |\r\n",
			i, voq_bp_all_thrd.bits.voq_bp_all_downthrd,
			voq_bp_all_thrd.bits.voq_bp_all_upthrd);
	}
}

/**
 * dsaf_voq_state_show -  voq
 * @dsaf_id: dsa fabric id
 */
void dsaf_voq_state_show(u32 dsaf_id)
{
	u32 i;
	union dsaf_voq_bp_status voq_bp_status;
	union dsaf_voq_spup_idle voq_spup_idle;

	osal_pr("voq bp status/voq sbm speedup idle status show:\r\n");
	osal_pr("voq num|dxi rdy|spup0 idle|spup1 idle|\r\n");
	for (i = 0; i < DSAF_VOQ_NUM; i++) {
		voq_bp_status.u32 = dsaf_read(dsaf_id,
			DSAF_VOQ_BP_STATUS_0_REG + 0x40 * (u64)i);
		voq_spup_idle.u32 = dsaf_read(dsaf_id,
			DSAF_VOQ_SPUP_IDLE_0_REG + 0x40 * (u64)i);
		osal_pr("   %2d  |   %d   |    %d     |    %d     |\r\n", i,
			voq_bp_status.bits.voq_dxi_rdy,
			voq_spup_idle.bits.voq_spup0_idle,
			voq_spup_idle.bits.voq_spup1_idle);
	}
}

/**
 * dsaf_voq_state_show -  voq
 * @dsaf_id: dsa fabric id
 */
void dsaf_voq_stat_show(u32 dsaf_id, u32 voq_num)
{
	u32 voq_in_pkt_num;
	u32 voq_out_pkt_num;
	union dsaf_voq_sram_pkt_num voq_sram_pkt_num;

	voq_in_pkt_num = dsaf_read(dsaf_id,
		DSAF_VOQ_IN_PKT_NUM_0_REG + 0x40 * (u64)voq_num);
	voq_out_pkt_num = dsaf_read(dsaf_id,
		DSAF_VOQ_OUT_PKT_NUM_0_REG + 0x40 * (u64)voq_num);
	voq_sram_pkt_num.u32 = dsaf_read(dsaf_id,
		DSAF_VOQ_SRAM_PKT_NUM_0_REG + 0x40 * (u64)voq_num);

	osal_pr("*******************voq %d statistics", voq_num);
	osal_pr("***************************\r\n");
	osal_pr("voq input packet total num: %d\r\n", voq_in_pkt_num);
	osal_pr("voq output packet total num: %d\r\n", voq_out_pkt_num);
	osal_pr("voq sram current packet total num: %d\r\n",
		voq_sram_pkt_num.bits.voq_sram_pkt_num);
	osal_pr("********************voq statistics end");
	osal_pr("*************************\r\n");
}

/**
 * dsaf_tbl_old_scan_val_show -  tbl
 * @dsaf_id: dsa fabric id
 */
void dsaf_tbl_old_scan_val_show(u32 dsaf_id)
{
	u32 old_scan_val;

	old_scan_val = dsaf_read(dsaf_id, DSAF_TBL_OLD_SCAN_VAL_0_REG);
	osal_pr("old scan time interval:%d, unit:512*1.25ns\r\n",
		old_scan_val);
}

/**
 * dsaf_tbl_stat_show - tbl
 * @dsaf_id: dsa fabric id
 * @tbl_num: value
 */
void dsaf_tbl_stat_show(u32 dsaf_id, u32 tbl_num)
{
	u32 tbl_lkup_num_i;
	u32 tbl_lkup_num_o;

	tbl_lkup_num_i = dsaf_read(dsaf_id,
		DSAF_TBL_LKUP_NUM_I_0_REG + 0x04 * (u64)tbl_num);
	tbl_lkup_num_o = dsaf_read(dsaf_id,
		DSAF_TBL_LKUP_NUM_O_0_REG + 0x04 * (u64)tbl_num);

	osal_pr("\n*******************tbl %d statistics", tbl_num);
	osal_pr("***************************\n");
	osal_pr("tbl receive look up request num: %d\r\n",
		tbl_lkup_num_i);
	osal_pr("tbl return look up result num: %d\r\n",
		tbl_lkup_num_o);
	osal_pr("\n********************tbl statistics end");
	osal_pr("**************************\n");
}

/**
 * dsaf_tbl_cfg_show - tbl
 * @dsaf_id: dsa fabric id
 */
void dsaf_tbl_cfg_show(u32 dsaf_id)
{
	union dsaf_tbl_ctrl1 tbl_ctrl;
	union dsaf_tbl_dfx_ctrl tbl_dfx_ctrl;

	osal_pr("tbl config state show:\r\n");

	tbl_ctrl.u32 = dsaf_read(dsaf_id, DSAF_TBL_CTRL_0_REG);
	tbl_dfx_ctrl.u32 = dsaf_read(dsaf_id, DSAF_TBL_DFX_CTRL_0_REG);

	osal_pr("tcam_t_sel:%d\r\n", tbl_ctrl.bits.tcam_t_sel);
	osal_pr("tbl_old_mask_en:%d\r\n", tbl_ctrl.bits.tbl_old_mask_en);
	osal_pr("tbl_old_scan_en:%d\r\n", tbl_ctrl.bits.tbl_old_scan_en);
	osal_pr("tbl_ucast_bcast_en:%d\r\n", tbl_ctrl.bits.tbl_ucast_bcast_en);
	osal_pr("tbl_vswitch_en:%d\r\n", tbl_ctrl.bits.tbl_vswitch_en);
	osal_pr("tbl_xge0_lkup_en:%d\r\n", tbl_ctrl.bits.xge0_lkup_en);
	osal_pr("tbl_xge1_lkup_en:%d\r\n", tbl_ctrl.bits.xge1_lkup_en);
	osal_pr("tbl_xge2_lkup_en:%d\r\n", tbl_ctrl.bits.xge2_lkup_en);
	osal_pr("tbl_xge3_lkup_en:%d\r\n", tbl_ctrl.bits.xge3_lkup_en);
	osal_pr("tbl_xge4_lkup_en:%d\r\n", tbl_ctrl.bits.xge4_lkup_en);
	osal_pr("tbl_xge5_lkup_en:%d\r\n", tbl_ctrl.bits.xge5_lkup_en);
	osal_pr("tbl_ppe_lkup_en:%d\r\n", tbl_ctrl.bits.ppe_lkup_en);
	osal_pr("tbl_rocee_lkup_en:%d\r\n", tbl_ctrl.bits.rocee_lkup_en);
	osal_pr("----------------------\r\n");
	osal_pr("bc_lkup_num_en:%d\r\n", tbl_dfx_ctrl.bits.bc_lkup_num_en);
	osal_pr("mc_lkup_num_en:%d\r\n", tbl_dfx_ctrl.bits.mc_lkup_num_en);
	osal_pr("uc_lkup_num_en:%d\r\n", tbl_dfx_ctrl.bits.uc_lkup_num_en);
	osal_pr("line_lkup_num_en:%d\r\n", tbl_dfx_ctrl.bits.line_lkup_num_en);
}

/**
 * dsaf_asyn_fifo_wl_show - fifo
 * @dsaf_id: dsa fabric id
 */
void dsaf_asyn_fifo_wl_show(u32 dsaf_id)
{
	u32 i;
	union dsaf_inode_fifo_wl1 inode_fifo_wl;
	union dsaf_onode_fifo_wl1 onode_fifo_wl;

	osal_pr("inode/onode asyn fifo water level show\r\n");
	osal_pr("        |      inode    |       onode    |\r\n");
	osal_pr("fifo num|afull on|alempt|alfull on|alempt|\r\n");
	for (i = 0; i < DSAF_FIFO_NUM; i++) {
		inode_fifo_wl.u32 = dsaf_read(dsaf_id,
			DSAF_INODE_FIFO_WL_0_REG + 0x4 * (u64)i);
		onode_fifo_wl.u32 = dsaf_read(dsaf_id,
			DSAF_ONODE_FIFO_WL_0_REG + 0x4 * (u64)i);
		osal_pr("   %d    |   %2d   |  %2d  |   %2d    |  %2d  |\r\n",
			i, inode_fifo_wl.bits.inode_fifo_afull_on_th,
			inode_fifo_wl.bits.inode_fifo_alempt_th,
			onode_fifo_wl.bits.onode_fifo_afull_on_th,
			onode_fifo_wl.bits.onode_fifo_alempt_th);
	}
}

/**
 * dsaf_ge_xge_cfg_show - fifo
 * @dsaf_id: dsa fabric id
 */
void dsaf_ge_xge_cfg_show(u32 dsaf_id)
{
	u32 i;
	union dsaf_xge_ge_work_mode xge_ge_work_mode;
	union dsaf_xge_ctrl_sig_cfg xge_ctrl_sig_cfg;
	union dsaf_xge_sds_vsemi_choose xge_sds_choose;

	osal_pr("ge/xge cfg show\n");
	osal_pr("xgenum|workmode|loopback|fc_xge_tx_pause|regs_xge_cnt_car|\n");
	for (i = 0; i < DSAF_XGE_NUM; i++) {
		xge_ctrl_sig_cfg.u32 = dsaf_read(dsaf_id,
			DSAF_XGE_CTRL_SIG_CFG_0_REG + 0x4 * (u64)i);
		xge_ge_work_mode.u32 = dsaf_read(dsaf_id,
			DSAF_XGE_GE_WORK_MODE_0_REG + 0x4 * (u64)i);
		osal_pr("  %d   |   %d    |   %d    |",
			i, xge_ge_work_mode.bits.xge_ge_work_mode,
			xge_ge_work_mode.bits.xge_ge_loopback);
		osal_pr("       %d       |       %d        |\n",
			xge_ctrl_sig_cfg.bits.fc_xge_tx_pause,
			xge_ctrl_sig_cfg.bits.regs_xge_cnt_car);
	}

	xge_sds_choose.u32
		= dsaf_read(dsaf_id, DSAF_XGE_SDS_VSEMI_CHOOSE_0_REG);
	osal_pr("------------------------------\n");
	osal_pr("port0 serdes datapath select: %d ",
		xge_sds_choose.bits.xge_sds_vsemi_choose);
	osal_pr("(0:vsemi channel3, 1:vsemi channel1)\n");
}

/**
 * dsaf_netport_state_show - fifo
 * @dsaf_id: dsa fabric id
 */
void dsaf_netport_state_show(u32 dsaf_id)
{
	u32 i;
	union dsaf_xge_app_rx_link_up xge_app_rx_link_up;
	union dsaf_netport_ctrl_sig netport_ctrl_sig;

	osal_pr("netport state show\r\n");
	osal_pr("xge num|rx link up|gmac err int|gmac led");
	osal_pr("|fc in|fc out|sync ok|\r\n");
	for (i = 0; i < DSAF_XGE_NUM; i++) {
		xge_app_rx_link_up.u32 = dsaf_read(dsaf_id,
			DSAF_XGE_APP_RX_LINK_UP_0_REG + 0x4 * (u64)i);
		netport_ctrl_sig.u32 = dsaf_read(dsaf_id,
			DSAF_NETPORT_CTRL_SIG_0_REG + 0x4 * (u64)i);
		osal_pr("   %d   |    %d     |     %d      |   %d    ",
			i, xge_app_rx_link_up.bits.xge_app_rx_link_up,
			netport_ctrl_sig.bits.ge_fifo_err_int,
			netport_ctrl_sig.bits.gmac_led_activity);
		osal_pr("|  %d  |  %d   |   %d   |\r\n",
			netport_ctrl_sig.bits.xge_app_rx_pause,
			netport_ctrl_sig.bits.xge_app_fc_vector,
			netport_ctrl_sig.bits.gmac_sync_ok);
	}
}

/**
 * dsaf_ge_xge_int_show - INT
 * @dsaf_id: dsa fabric id
 */
static void dsaf_ge_xge_int_show(u32 dsaf_id)
{
	union dsaf_xge_int_src int_src;
	union dsaf_xge_int_msk int_msk;
	union dsaf_xge_int_sts int_sts;
	u32 i;

	osal_pr("dsaf xge/ge int show: \r\n");
	osal_pr("0: dsaf_xid_xge_ecc_err_int\r\n");
	osal_pr("1: dsaf_xid_xge_fsm_timeout_int\r\n");
	osal_pr("8: dsaf_sbm_xge_lnk_fsm_timout_int\r\n");
	osal_pr("9: dsaf_sbm_xge_lnk_ecc_2bit_int\r\n");
	osal_pr("10:dsaf_sbm_xge_mib_req_failed_int\r\n");
	osal_pr("11:dsaf_sbm_xge_mib_req_fsm_timout\r\n");
	osal_pr("12:dsaf_sbm_xge_mib_rels_fsm_timout\r\n");
	osal_pr("13:dsaf_sbm_xge_sram_ecc_2bit_int\r\n");
	osal_pr("14:dsaf_sbm_xge_mib_buf_sum_err_int\r\n");
	osal_pr("15:dsaf_sbm_xge_mib_req_extra_int\r\n");
	osal_pr("16:dsaf_sbm_xge_mib_rels_extra_int\r\n");
	osal_pr("20:dsaf_voq_xge_start_to_over_0_int\r\n");
	osal_pr("21:dsaf_voq_xge_start_to_over_1_int\r\n");
	osal_pr("22:dsaf_voq_xge_ecc_err_int\r\n");

	osal_pr("chn      ");
	dsaf_print_char_format(32, 10);

	for (i = 0; i < DSAF_XGE_CHN; i++) {
		int_src.u32
			= dsaf_read(dsaf_id, DSAF_XGE_INT_SRC_0_REG + 0x4*(u64)i);
		int_msk.u32
			= dsaf_read(dsaf_id, DSAF_XGE_INT_MSK_0_REG + 0x4*(u64)i);
		int_sts.u32
			= dsaf_read(dsaf_id,DSAF_XGE_INT_STS_0_REG + 0x4*(u64)i);
		osal_pr(" %d src : ", i);
		dsaf_print_2hex_format(32, int_src.u32);
		osal_pr("   msk : ");
		dsaf_print_2hex_format(32, int_msk.u32);
		osal_pr("   sts : ");
		dsaf_print_2hex_format(32, int_sts.u32);
		osal_pr("---------------------------------------------------");
		osal_pr("----------------------\r\n");
	}
}

/**
 * dsaf_ppe_int_show - INT
 * @dsaf_id: dsa fabric id
 */
static void dsaf_ppe_int_show(u32 dsaf_id)
{
	union dsaf_ppe_int_src int_src;
	union dsaf_ppe_int_msk int_msk;
	union dsaf_ppe_int_sts int_sts;
	u32 i;

	osal_pr("dsaf ppe int show: \r\n");
	osal_pr("0: dsaf_xid_ppe_fsm_timeout_int\r\n");
	osal_pr("8 :dsaf_sbm_ppe_lnk_fsm_timout_int\r\n");
	osal_pr("9 :dsaf_sbm_ppe_lnk_ecc_2bit_int\r\n");
	osal_pr("10:dsaf_sbm_ppe_mib_req_failed_int\r\n");
	osal_pr("11:dsaf_sbm_ppe_mib_req_fsm_timout_int\r\n");
	osal_pr("12:dsaf_sbm_ppe_mib_rels_fsm_timout_int\r\n");
	osal_pr("13:dsaf_sbm_ppe_sram_ecc_2bit_int\r\n");
	osal_pr("14:dsaf_sbm_ppe_mib_buf_sum_err_int\r\n");
	osal_pr("15:dsaf_sbm_ppe_mib_req_extra_int\r\n");
	osal_pr("16:dsaf_sbm_ppe_mib_rels_extra_int\r\n");
	osal_pr("20:dsaf_voq_ppe_start_to_over_0_int\r\n");
	osal_pr("21:dsaf_voq_ppe_ecc_err_int\r\n");
	osal_pr("24:dsaf_xod_ppe_fifo_rd_empty_int\r\n");
	osal_pr("25:dsaf_xod_ppe_fifo_wr_full_int\r\n");

	osal_pr("chn	  ");
	dsaf_print_char_format(32, 10);

	for (i = 0; i < DSAF_PPE_CHN; i++) {
		int_src.u32
			= dsaf_read(dsaf_id, DSAF_PPE_INT_SRC_0_REG + 0x4*(u64)i);
		int_msk.u32
			= dsaf_read(dsaf_id, DSAF_PPE_INT_MSK_0_REG + 0x4*(u64)i);
		int_sts.u32
			= dsaf_read(dsaf_id, DSAF_PPE_INT_STS_0_REG + 0x4*(u64)i);
		osal_pr(" %d src : ", i);
		dsaf_print_2hex_format(32, int_src.u32);
		osal_pr("   msk : ");
		dsaf_print_2hex_format(32, int_msk.u32);
		osal_pr("   sts : ");
		dsaf_print_2hex_format(32, int_sts.u32);
		osal_pr("-------------------------------------------------");
		osal_pr("------------------------\r\n");
	}
}

/**
 * dsaf_rocee_int_show - INT
 * @dsaf_id: dsa fabric id
 */
static void dsaf_rocee_int_show(u32 dsaf_id)
{
	union dsaf_rocee_int_src int_src;
	union dsaf_rocee_int_msk int_msk;
	union dsaf_rocee_int_sts int_sts;
	u32 i;

	osal_pr("dsaf rocee int show: \r\n");
	osal_pr("0: dsaf_xid_rocee_fsm_timeout_int\r\n");
	osal_pr("8:dsaf_sbm_rocee_lnk_fsm_timout_int\r\n");
	osal_pr("9:dsaf_sbm_rocee_lnk_ecc_2bit_int\r\n");
	osal_pr("10:dsaf_sbm_rocee_mib_req_failed_int\r\n");
	osal_pr("11:dsaf_sbm_rocee_mib_req_fsm_timout_int\r\n");
	osal_pr("12:dsaf_sbm_rocee_mib_rels_fsm_timout_int\r\n");
	osal_pr("13:dsaf_sbm_rocee_sram_ecc_2bit_int\r\n");
	osal_pr("14:dsaf_sbm_rocee_mib_buf_sum_err_int\r\n");
	osal_pr("15:dsaf_sbm_rocee_mib_req_extra_int\r\n");
	osal_pr("16:dsaf_sbm_rocee_mib_rels_extra_int\r\n");
	osal_pr("20:dsaf_voq_rocee_start_to_over_0_int\r\n");
	osal_pr("21:dsaf_voq_rocee_ecc_err_int\r\n");

	osal_pr("chn      ");
	dsaf_print_char_format(32, 10);

	for (i = 0; i < DSAF_ROCEE_CHN; i++) {
		int_src.u32
			= dsaf_read(dsaf_id, DSAF_ROCEE_INT_SRC_0_REG + 0x4*(u64)i);
		int_msk.u32
			= dsaf_read(dsaf_id, DSAF_ROCEE_INT_MSK_0_REG + 0x4*(u64)i);
		int_sts.u32
			= dsaf_read(dsaf_id, DSAF_ROCEE_INT_STS_0_REG + 0x4*(u64)i);
		osal_pr(" %d src : ", i);
		dsaf_print_2hex_format(32, int_src.u32);
		osal_pr("   msk : ");
		dsaf_print_2hex_format(32, int_msk.u32);
		osal_pr("   sts : ");
		dsaf_print_2hex_format(32, int_sts.u32);
		osal_pr("----------------------------------------------");
		osal_pr("---------------------------\r\n");
	}
}

/**
 * dsaf_tbl_int_show - INT
 * @dsaf_id: dsa fabric id
 */
static void dsaf_tbl_int_show(u32 dsaf_id)
{
	union dsaf_tbl_int_src tbl_int_src;
	union dsaf_tbl_int_msk1 tbl_int_msk;
	union dsaf_tbl_int_sts o_tbl_int_sts;

	tbl_int_src.u32 = dsaf_read(dsaf_id, DSAF_TBL_INT_SRC_0_REG);
	tbl_int_msk.u32 = dsaf_read(dsaf_id, DSAF_TBL_INT_MSK_0_REG);
	o_tbl_int_sts.u32 = dsaf_read(dsaf_id, DSAF_TBL_INT_STS_0_REG);

	osal_pr("dsaf tbl int show: \r\n");

	osal_pr("0:tbl_da0_mis\r\n");
	osal_pr("1:tbl_da1_mis\r\n");
	osal_pr("2:tbl_da2_mis\r\n");
	osal_pr("3:tbl_da3_mis\r\n");
	osal_pr("4:tbl_da4_mis\r\n");
	osal_pr("5:tbl_da5_mis\r\n");
	osal_pr("6:tbl_da6_mis\r\n");
	osal_pr("7:tbl_da7_mis\r\n");
	osal_pr("8:tbl_sa_mis\r\n");
	osal_pr("9:tbl_old_sech_end\r\n");
	osal_pr("10:lram_ecc_err1\r\n");
	osal_pr("11:lram_ecc_err2\r\n");
	osal_pr("12:tram_ecc_err1\r\n");
	osal_pr("13:tram_ecc_err2\r\n");
	osal_pr("14:tbl_ucast_bcast_xge0\r\n");
	osal_pr("15:tbl_ucast_bcast_xge1\r\n");
	osal_pr("16:tbl_ucast_bcast_xge2\r\n");
	osal_pr("17:tbl_ucast_bcast_xge3\n");
	osal_pr("18:tbl_ucast_bcast_xge4\r\n");
	osal_pr("19:tbl_ucast_bcast_xge5\r\n");
	osal_pr("20:tbl_ucast_bcast_ppe\r\n");
	osal_pr("21:tbl_ucast_bcast_rocee\r\n");

	osal_pr("      ");
	dsaf_print_char_format(32, 10);

	osal_pr("src : ");
	dsaf_print_2hex_format(32, tbl_int_src.u32);
	osal_pr("msk : ");
	dsaf_print_2hex_format(32, tbl_int_msk.u32);
	osal_pr("sts : ");
	dsaf_print_2hex_format(32, o_tbl_int_sts.u32);
}

/**
 * dsaf_int_xge_stat_show - INT
 * void
 */
void dsaf_int_xge_stat_show(void)
{
	u32 i;
	struct dsaf_int_xge_src *p;

	osal_pr("+++++++++++++++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf int xge stat\r\n");
	osal_pr("+++++++++++++++++++++++++++++++++++++++++++\r\n");

	for (i = 0; i < DSAF_XGE_CHN; i++) {
		p = &g_dsaf_int_xge_stat[i];
		dsaf_non_zero_printf(p->xid_xge_ecc_err_int_src,
			"xge[%d] xid_xge_ecc_err_int_src              : %d\r\n",
			i, p->xid_xge_ecc_err_int_src);
		dsaf_non_zero_printf(p->xid_xge_fsm_timout_int_src,
			"xge[%d] xid_xge_fsm_timout_int_src           : %d\r\n",
			i, p->xid_xge_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_xge_lnk_fsm_timout_int_src,
			"xge[%d] sbm_xge_lnk_fsm_timout_int_src       : %d\r\n",
			i, p->sbm_xge_lnk_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_xge_lnk_ecc_2bit_int_src,
			"xge[%d] sbm_xge_lnk_ecc_2bit_int_src         : %d\r\n",
			i, p->sbm_xge_lnk_ecc_2bit_int_src);
		dsaf_non_zero_printf(p->sbm_xge_mib_req_failed_int_src,
			"xge[%d] sbm_xge_mib_req_failed_int_src       : %d\r\n",
			i, p->sbm_xge_mib_req_failed_int_src);
		dsaf_non_zero_printf(p->sbm_xge_mib_req_fsm_timout_int_src,
			"xge[%d] sbm_xge_mib_req_fsm_timout_int_src   : %d\r\n",
			i, p->sbm_xge_mib_req_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_xge_mib_rels_fsm_timout_int_src,
			"xge[%d] sbm_xge_mib_rels_fsm_timout_int_src  : %d\r\n",
			i, p->sbm_xge_mib_rels_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_xge_sram_ecc_2bit_int_src,
			"xge[%d] sbm_xge_sram_ecc_2bit_int_src        : %d\r\n",
			i, p->sbm_xge_sram_ecc_2bit_int_src);
		dsaf_non_zero_printf(p->sbm_xge_mib_buf_sum_err_int_src,
			"xge[%d] sbm_xge_mib_buf_sum_err_int_src      : %d\r\n",
			i, p->sbm_xge_mib_buf_sum_err_int_src);
		dsaf_non_zero_printf(p->sbm_xge_mib_req_extra_int_src,
			"xge[%d] sbm_xge_mib_req_extra_int_src        : %d\r\n",
			i, p->sbm_xge_mib_req_extra_int_src);
		dsaf_non_zero_printf(p->sbm_xge_mib_rels_extra_int_src,
			"xge[%d] sbm_xge_mib_rels_extra_int_src       : %d\r\n",
			i, p->sbm_xge_mib_rels_extra_int_src);
		dsaf_non_zero_printf(p->voq_xge_start_to_over_0_int_src,
			"xge[%d] voq_xge_start_to_over_0_int_src      : %d\r\n",
			i, p->voq_xge_start_to_over_0_int_src);
		dsaf_non_zero_printf(p->voq_xge_start_to_over_1_int_src,
			"xge[%d] voq_xge_start_to_over_1_int_src      : %d\r\n",
			i, p->voq_xge_start_to_over_1_int_src);
		dsaf_non_zero_printf(p->voq_xge_ecc_err_int_src,
			"xge[%d] voq_xge_ecc_err_int_src              : %d\r\n",
			i, p->voq_xge_ecc_err_int_src);
	}
}

/**
 * dsaf_int_ppe_stat_show - INT
 * void
 */
void dsaf_int_ppe_stat_show(void)
{
	u32 i;
	struct dsaf_int_ppe_src *p;

	osal_pr("+++++++++++++++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf int ppe stat\r\n");
	osal_pr("+++++++++++++++++++++++++++++++++++++++++++\r\n");

	for (i = 0; i < DSAF_XGE_CHN; i++) {
		p = &g_dsaf_int_ppe_stat[i];
		dsaf_non_zero_printf(p->xid_ppe_fsm_timout_int_src,
			"ppe[%d] xid_ppe_fsm_timout_int_src       : %d\r\n",
			i, p->xid_ppe_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_ppe_lnk_fsm_timout_int_src,
			"ppe[%d] sbm_ppe_lnk_fsm_timout_int_src   : %d\r\n",
			i, p->sbm_ppe_lnk_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_ppe_lnk_ecc_2bit_int_src,
			"ppe[%d] sbm_ppe_lnk_ecc_2bit_int_src         : %d\r\n",
			i, p->sbm_ppe_lnk_ecc_2bit_int_src);
		dsaf_non_zero_printf(p->sbm_ppe_mib_req_failed_int_src,
			"ppe[%d] sbm_ppe_mib_req_failed_int_src   : %d\r\n",
			i, p->sbm_ppe_mib_req_failed_int_src);
		dsaf_non_zero_printf(p->sbm_ppe_mib_req_fsm_timout_int_src,
			"ppe[%d] sbm_ppe_mib_req_fsm_timout_int_src  : %d\r\n",
			i, p->sbm_ppe_mib_req_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_ppe_mib_rels_fsm_timout_int_src,
			"ppe[%d] sbm_ppe_mib_rels_fsm_timout_int_src : %d\r\n",
			i, p->sbm_ppe_mib_rels_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_ppe_sram_ecc_2bit_int_src,
			"ppe[%d] sbm_ppe_sram_ecc_2bit_int_src    : %d\r\n",
			i, p->sbm_ppe_sram_ecc_2bit_int_src);
		dsaf_non_zero_printf(p->sbm_ppe_mib_buf_sum_err_int_src,
			"ppe[%d] sbm_ppe_mib_buf_sum_err_int_src      : %d\r\n",
			i, p->sbm_ppe_mib_buf_sum_err_int_src);
		dsaf_non_zero_printf(p->sbm_ppe_mib_req_extra_int_src,
			"ppe[%d] sbm_ppe_mib_req_extra_int_src    : %d\r\n",
			i, p->sbm_ppe_mib_req_extra_int_src);
		dsaf_non_zero_printf(p->sbm_ppe_mib_rels_extra_int_src,
			"ppe[%d] sbm_ppe_mib_rels_extra_int_src   : %d\r\n",
			i, p->sbm_ppe_mib_rels_extra_int_src);
		dsaf_non_zero_printf(p->voq_ppe_start_to_over_0_int_src,
			"ppe[%d] voq_ppe_start_to_over_0_int_src      : %d\r\n",
			i, p->voq_ppe_start_to_over_0_int_src);
		dsaf_non_zero_printf(p->voq_ppe_ecc_err_int_src,
			"ppe[%d] voq_ppe_ecc_err_int_src              : %d\r\n",
			i, p->voq_ppe_ecc_err_int_src);
		dsaf_non_zero_printf(p->xod_ppe_fifo_rd_empty_int_src,
			"ppe[%d] xod_ppe_fifo_rd_empty_int_src    : %d\r\n",
			i, p->xod_ppe_fifo_rd_empty_int_src);
		dsaf_non_zero_printf(p->xod_ppe_fifo_wr_full_int_src,
			"ppe[%d] xod_ppe_fifo_wr_full_int_src         : %d\r\n",
			i, p->xod_ppe_fifo_wr_full_int_src);
	}
}

/**
 * dsaf_int_rocee_stat_show - INT
 * void
 */
void dsaf_int_rocee_stat_show(void)
{
	u32 i;
	struct dsaf_int_rocee_src *p;

	osal_pr("+++++++++++++++++++++++++++++++++++++++++++\n");
	osal_pr("dsaf int rocee stat\r\n");
	osal_pr("+++++++++++++++++++++++++++++++++++++++++++\n");

	for (i = 0; i < DSAF_XGE_CHN; i++) {
		p = &g_dsaf_int_rocee_stat[i];
		dsaf_non_zero_printf(p->xid_rocee_fsm_timout_int_src,
			"roc[%d] xid_rocee_fsm_timout_int_src         : %d\n",
			i, p->xid_rocee_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_rocee_lnk_fsm_timout_int_src,
			"roc[%d] sbm_rocee_lnk_fsm_timout_int_src     : %d\n",
			i, p->sbm_rocee_lnk_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_rocee_lnk_ecc_2bit_int_src,
			"roc[%d] sbm_rocee_lnk_ecc_2bit_int_src       : %d\n",
			i, p->sbm_rocee_lnk_ecc_2bit_int_src);
		dsaf_non_zero_printf(p->sbm_rocee_mib_req_failed_int_src,
			"roc[%d] sbm_rocee_mib_req_failed_int_src     : %d\n",
			i, p->sbm_rocee_mib_req_failed_int_src);
		dsaf_non_zero_printf(p->sbm_rocee_mib_req_fsm_timout_int_src,
			"roc[%d] sbm_rocee_mib_req_fsm_timout_int_src  : %d\n",
			i, p->sbm_rocee_mib_req_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_rocee_mib_rels_fsm_timout_int_src,
			"roc[%d] sbm_rocee_mib_rels_fsm_timout_int_src : %d\n",
			i, p->sbm_rocee_mib_rels_fsm_timout_int_src);
		dsaf_non_zero_printf(p->sbm_rocee_sram_ecc_2bit_int_src,
			"roc[%d] sbm_rocee_sram_ecc_2bit_int_src      : %d\n",
			i, p->sbm_rocee_sram_ecc_2bit_int_src);
		dsaf_non_zero_printf(p->sbm_rocee_mib_buf_sum_err_int_src,
			"roc[%d] sbm_rocee_mib_buf_sum_err_int_src    : %d\n",
			i, p->sbm_rocee_mib_buf_sum_err_int_src);
		dsaf_non_zero_printf(p->sbm_rocee_mib_req_extra_int_src,
			"roc[%d] sbm_rocee_mib_req_extra_int_src      : %d\n",
			i, p->sbm_rocee_mib_req_extra_int_src);
		dsaf_non_zero_printf(p->sbm_rocee_mib_rels_extra_int_src,
			"roc[%d] sbm_rocee_mib_rels_extra_int_src     : %d\n",
			i, p->sbm_rocee_mib_rels_extra_int_src);
		dsaf_non_zero_printf(p->voq_rocee_start_to_over_0_int_src,
			"roc[%d] voq_rocee_start_to_over_0_int_src    : %d\n",
			i, p->voq_rocee_start_to_over_0_int_src);
		dsaf_non_zero_printf(p->voq_rocee_ecc_err_int_src,
			"roc[%d] voq_rocee_ecc_err_int_src            : %d\n",
			i, p->voq_rocee_ecc_err_int_src);
	}
}

/**
 * dsaf_int_tbl_stat_show - INT
 * void
 */
void dsaf_int_tbl_stat_show(void)
{
	struct dsaf_int_tbl_src *p;

	osal_pr("+++++++++++++++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf int tbl stat\r\n");
	osal_pr("+++++++++++++++++++++++++++++++++++++++++++\r\n");

	p = &g_dsaf_int_tbl_stat[0];
	dsaf_non_zero_printf(p->tbl_da0_mis_src,
		"tbl_da0_mis_src             : %d\r\n", p->tbl_da0_mis_src);
	dsaf_non_zero_printf(p->tbl_da1_mis_src,
		"tbl_da1_mis_src             : %d\r\n", p->tbl_da1_mis_src);
	dsaf_non_zero_printf(p->tbl_da2_mis_src,
		"tbl_da2_mis_src             : %d\r\n", p->tbl_da2_mis_src);
	dsaf_non_zero_printf(p->tbl_da3_mis_src,
		"tbl_da3_mis_src             : %d\r\n", p->tbl_da3_mis_src);
	dsaf_non_zero_printf(p->tbl_da4_mis_src,
		"tbl_da4_mis_src             : %d\r\n", p->tbl_da4_mis_src);
	dsaf_non_zero_printf(p->tbl_da5_mis_src,
		"tbl_da5_mis_src             : %d\r\n", p->tbl_da5_mis_src);
	dsaf_non_zero_printf(p->tbl_da6_mis_src,
		"tbl_da6_mis_src             : %d\r\n", p->tbl_da6_mis_src);
	dsaf_non_zero_printf(p->tbl_da7_mis_src,
		"tbl_da7_mis_src             : %d\r\n", p->tbl_da7_mis_src);
	dsaf_non_zero_printf(p->tbl_sa_mis_src,
		"tbl_sa_mis_src          : %d\r\n", p->tbl_sa_mis_src);
	dsaf_non_zero_printf(p->tbl_old_sech_end_src,
		"tbl_old_sech_end_src    : %d\r\n", p->tbl_old_sech_end_src);
	dsaf_non_zero_printf(p->lram_ecc_err1_src,
		"lram_ecc_err1_src       : %d\r\n", p->lram_ecc_err1_src);
	dsaf_non_zero_printf(p->lram_ecc_err2_src,
		"lram_ecc_err2_src       : %d\r\n", p->lram_ecc_err2_src);
	dsaf_non_zero_printf(p->tram_ecc_err1_src,
		"tram_ecc_err1_src       : %d\r\n", p->tram_ecc_err1_src);
	dsaf_non_zero_printf(p->tram_ecc_err2_src,
		"tram_ecc_err2_src       : %d\r\n", p->tram_ecc_err2_src);
	dsaf_non_zero_printf(p->tbl_ucast_bcast_xge0_src,
		"tbl_ucast_bcast_xge0_src  : %d\r\n",
		p->tbl_ucast_bcast_xge0_src);
	dsaf_non_zero_printf(p->tbl_ucast_bcast_xge1_src,
		"tbl_ucast_bcast_xge1_src  : %d\r\n",
		p->tbl_ucast_bcast_xge1_src);
	dsaf_non_zero_printf(p->tbl_ucast_bcast_xge2_src,
		"tbl_ucast_bcast_xge2_src  : %d\r\n",
		p->tbl_ucast_bcast_xge2_src);
	dsaf_non_zero_printf(p->tbl_ucast_bcast_xge3_src,
		"tbl_ucast_bcast_xge3_src  : %d\r\n",
		p->tbl_ucast_bcast_xge3_src);
	dsaf_non_zero_printf(p->tbl_ucast_bcast_xge4_src,
		"tbl_ucast_bcast_xge4_src  : %d\r\n",
		p->tbl_ucast_bcast_xge4_src);
	dsaf_non_zero_printf(p->tbl_ucast_bcast_xge5_src,
		"tbl_ucast_bcast_xge5_src  : %d\r\n",
		p->tbl_ucast_bcast_xge5_src);
	dsaf_non_zero_printf(p->tbl_ucast_bcast_ppe_src,
		"tbl_ucast_bcast_ppe_src     : %d\r\n",
		p->tbl_ucast_bcast_ppe_src);
	dsaf_non_zero_printf(p->tbl_ucast_bcast_rocee_src,
		"tbl_ucast_bcast_rocee_src : %d\r\n",
		p->tbl_ucast_bcast_rocee_src);
}

/**
 * dsaf_int_stat_show - INT
 * void
 */
void dsaf_int_stat_show(void)
{
	dsaf_int_xge_stat_show();
	dsaf_int_ppe_stat_show();
	dsaf_int_rocee_stat_show();
	dsaf_int_tbl_stat_show();
}

/**
 * dsaf_tbl_mis_info_show - INT
 * @dsaf_id: dsa fabric id
 */
void dsaf_tbl_mis_info_show(u32 dsaf_id)
{
	u32 i;
	union dsaf_tbl_da0_mis_info1 da_mis_info1;
	union dsaf_tbl_da0_mis_info0 da_mis_info0;
	union dsaf_tbl_ucast_bcast_mis_info_0 uc_bc_mis_info;
	union dsaf_tbl_sa_mis_info2 sa_mis_info2;
	union dsaf_tbl_sa_mis_info1 sa_mis_info1;
	union dsaf_tbl_sa_mis_info0 sa_mis_info0;

	osal_pr("tal da miss serch info show:\r\n");
	osal_pr("	 |da mis high|da mis low|da mis vlan");
	osal_pr("|u/bcast mis vlan|\r\n");
	for (i = 0; i < 8; i++) {
		da_mis_info1.u32 = dsaf_read(dsaf_id,
			DSAF_TBL_DA0_MIS_INFO1_0_REG + 0x8 * (u64)i);
		da_mis_info0.u32 = dsaf_read(dsaf_id,
			DSAF_TBL_DA0_MIS_INFO0_0_REG + 0x8 * (u64)i);
		uc_bc_mis_info.u32 = dsaf_read(dsaf_id,
			DSAF_TBL_UCAST_BCAST_MIS_INFO_0_0_REG + 0x4 * (u64)i);
		if (i < DSAF_XGE_NUM)
			osal_pr("xge%d|0x%8x|0x%8x |0x%8x |  0x%4x  |\r\n",
				i, da_mis_info1.bits.tbl_da0_mis_dah,
				da_mis_info0.bits.tbl_da0_mis_dal,
				da_mis_info0.bits.tbl_da0_mis_vlan,
				uc_bc_mis_info.bits.tbl_ucast_bcast_mis_vlan_0);
		else if(i == 6)
			osal_pr("ppe |0x%8x|0x%8x |0x%8x |  0x%4x  |\r\n",
				da_mis_info1.bits.tbl_da0_mis_dah,
				da_mis_info0.bits.tbl_da0_mis_dal,
				da_mis_info0.bits.tbl_da0_mis_vlan,
				uc_bc_mis_info.bits.tbl_ucast_bcast_mis_vlan_0);
		else
			osal_pr("roce|0x%8x|0x%8x |0x%8x |  0x%4x  |\r\n",
				da_mis_info1.bits.tbl_da0_mis_dah,
				da_mis_info0.bits.tbl_da0_mis_dal,
				da_mis_info0.bits.tbl_da0_mis_vlan,
				uc_bc_mis_info.bits.tbl_ucast_bcast_mis_vlan_0);
	}

	osal_pr("tal sa miss serch info show:\r\n");
	osal_pr("port|sa mis high|sa mis low|sa mis vlan|\r\n");

	sa_mis_info2.u32 = dsaf_read(dsaf_id, DSAF_TBL_SA_MIS_INFO2_0_REG);
	sa_mis_info1.u32 = dsaf_read(dsaf_id, DSAF_TBL_SA_MIS_INFO1_0_REG);
	sa_mis_info0.u32 = dsaf_read(dsaf_id, DSAF_TBL_SA_MIS_INFO0_0_REG);

	osal_pr("%4d|%4d|0x%8x|0x%8x |0x%8x |\r\n",
	sa_mis_info2.bits.tbl_sa_mis_qid, sa_mis_info1.bits.tbl_sa_mis_port,
	sa_mis_info1.bits.tbl_sa_mis_sah, sa_mis_info0.bits.tbl_sa_mis_sal,
	sa_mis_info0.bits.tbl_sa_mis_vlan);
}

/**
 * dsaf_tbl_tcam_ucast_single_show - INT
 * @dsaf_id: dsa fabric id
 * @address
 */
void dsaf_tbl_tcam_ucast_single_show(u32 dsaf_id, u32 address)
{
	union dsaf_tbl_tcam_ucast_cfg1 tcam_read_data0;
	union dsaf_tbl_tcam_mcast_cfg_4 tcam_read_data4;
	u32 dah;
	u32 dal;
	u32 vlan_id;
	u32 old_en;
	u32 item_vld;
	u32 port_num;

	/*Write Addr*/
	dsaf_tbl_tcam_addr_cfg(dsaf_id, address);

	/*read tcam item puls*/
	dsaf_tbl_tcam_load_pul(dsaf_id);

	/*read tcam data*/
	dah = dsaf_read(dsaf_id, DSAF_TBL_TCAM_RDATA_HIGH_0_REG);
	dal = dsaf_read(dsaf_id, DSAF_TBL_TCAM_RDATA_LOW_0_REG);
	vlan_id = (dal & 0x0000fff0) >> 4;
	port_num = dal & 0xf;
	dal = (dal & 0xffff0000) >> 16;

	/*read tcam mcast*/
	tcam_read_data4.u32
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA4_0_REG);
	old_en = tcam_read_data4.bits.tbl_mcast_old_en;/*136bit*/
	item_vld = tcam_read_data4.bits.tbl_mcast_item_vld;/*135bit*/

	tcam_read_data0.u32
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA0_0_REG);

	osal_pr("%04d|0x%08x|0x%04x|0x%03x | %d  |  %d  ",
		address, dah, dal, vlan_id,
		port_num, old_en);
	osal_pr("| %d |  %d   |%d | %03d   |\r\n",
		item_vld, tcam_read_data0.bits.tbl_ucast_mac_discard,
		tcam_read_data0.bits.tbl_ucast_dvc,
		tcam_read_data0.bits.tbl_ucast_out_port);
}

/**
 * dsaf_tbl_tcam_mcast_single_show - INT
 * @dsaf_id: dsa fabric id
 * @address
 * @num
 */
void dsaf_tbl_tcam_mcast_single_show(u32 dsaf_id, u32 address)
{
	union dsaf_tbl_tcam_mcast_cfg_0 tcam_read_data0;
	u32 tcam_read_data1;
	u32 tcam_read_data2;
	u32 tcam_read_data3;
	union dsaf_tbl_tcam_mcast_cfg_4 tcam_read_data4;

	u32 dah;
	u32 dal;
	u32 port_num;
	u32 vlan_id;
	u32 xge5_0;
	u32 vm[13];
	u32 *pvm = vm;

	u32 i;

	/*Write Addr*/
	dsaf_tbl_tcam_addr_cfg(dsaf_id, address);

	/*read tcam item puls*/
	dsaf_tbl_tcam_load_pul(dsaf_id);

	/*read tcam data*/
	dah = dsaf_read(dsaf_id, DSAF_TBL_TCAM_RDATA_HIGH_0_REG);
	dal = dsaf_read(dsaf_id, DSAF_TBL_TCAM_RDATA_LOW_0_REG);
	vlan_id = (dal & 0x0000fff0) >> 4;
	port_num = dal & 0xf;
	dal = (dal & 0xffff0000) >> 16;

	/*read tcam mcast*/
	tcam_read_data0.u32
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA0_0_REG);
	tcam_read_data1
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA1_0_REG);
	tcam_read_data2
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA2_0_REG);
	tcam_read_data3
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA3_0_REG);
	tcam_read_data4.u32
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA4_0_REG);

	osal_pr("tcam_read_data0: 0x%x\r\n", tcam_read_data0.u32);
	osal_pr("tcam_read_data1: 0x%x\r\n", tcam_read_data1);
	osal_pr("tcam_read_data2: 0x%x\r\n", tcam_read_data2);
	osal_pr("tcam_read_data3: 0x%x\r\n", tcam_read_data3);
	osal_pr("tcam_read_data4: 0x%x\r\n", tcam_read_data4.u32);

	vm[12] = (tcam_read_data4.bits.tbl_mcast_vm128_122 << 2) |
			((tcam_read_data3 & 0xc0000000) >> 30);
	osal_pr("vm[12]: 0x%x\r\n", vm[12]);
	vm[11] = (tcam_read_data3 & 0x3ff00000) >> 20;
	osal_pr("vm[11]: 0x%x\r\n", vm[11]);
	vm[10] = (tcam_read_data3 & 0x000ffc00) >> 10;
	osal_pr("vm[10]: 0x%x\r\n", vm[10]);
	vm[9] = tcam_read_data3 & 0x000003ff;
	osal_pr("vm[9]: 0x%x\r\n", vm[9]);
	vm[8] = (tcam_read_data2 & 0xffc00000) >> 22;
	osal_pr("vm[8]: 0x%x\r\n", vm[8]);
	vm[7] = (tcam_read_data2 & 0x003ff000) >> 12;
	osal_pr("vm[7]: 0x%x\r\n", vm[7]);
	vm[6] = (tcam_read_data2 & 0x00000ffc) >> 2;
	osal_pr("vm[6]: 0x%x\r\n", vm[6]);
	vm[5] = ((tcam_read_data2 & 0x00000003) << 8) |
			((tcam_read_data1 & 0xff000000) >> 24);
	osal_pr("vm[5]: 0x%x\r\n", vm[5]);
	vm[4] = ((tcam_read_data1 & 0x00ffc000) >> 14);
	osal_pr("vm[4]: 0x%x\r\n", vm[4]);
	vm[3] = ((tcam_read_data1 & 0x00003ff0) >> 4);
	osal_pr("vm[3]: 0x%x\r\n", vm[3]);
	vm[2] = ((tcam_read_data1 & 0x0000000f) << 6) |
			((tcam_read_data0.u32 & 0xfc000000) >> 26);
	osal_pr("vm[2]: 0x%x\r\n", vm[2]);
	vm[1] = ((tcam_read_data0.u32 & 0x03ff0000) >> 16);
	osal_pr("vm[1]: 0x%x\r\n", vm[1]);
	vm[0] = ((tcam_read_data0.u32 & 0x0000ffc0) >> 6);
	osal_pr("vm[0]: 0x%x\r\n", vm[0]);
	xge5_0 = tcam_read_data0.bits.tbl_mcast_xge5_0;
	osal_pr("xge5_0: 0x%x\r\n", xge5_0);


	osal_pr("tcam mcast item: \r\n");
	osal_pr("addr|dah        |dal   |vlanId|port|oldEn|vld|\r\n");
	osal_pr("%4d|0x%08x|0x%04x|0x%03x | %d  |   %d  | %d |\r\n",
		address, dah, dal, vlan_id, port_num,
		tcam_read_data4.bits.tbl_mcast_old_en,
		tcam_read_data4.bits.tbl_mcast_item_vld);

	osal_pr("          ");
	dsaf_print_char_format(10, 10);

	osal_pr("xge 0_  5 : ");
	dsaf_print_2hex_format(6, xge5_0);

	for (i = 0; i < 13; i++) {
		osal_pr("vm%3d_%3d : ", (i * 10), (i * 10 + 9));
		dsaf_print_2hex_format(10, *pvm);
		pvm++;
	}
}

/**
 * dsaf_tbl_tcam_ucast_show - INT
 * @dsaf_id: dsa fabric id
 * @address
 * @num
 */
void dsaf_tbl_tcam_ucast_show(u32 dsaf_id, u32 address, u32 num)
{
	u32 i;

	osal_pr("tcam ucast item: \r\n");
	osal_pr("addr|dah        |dal   |vlanId|port|oldEn|");
	osal_pr("vld|macDis|vc|outPort|\r\n");

	for (i = 0; i < num; i++)
		dsaf_tbl_tcam_ucast_single_show(dsaf_id, address + i);
}

/**
 * dsaf_tbl_tcam_mcast_show - INT
 * @dsaf_id: dsa fabric id
 * @address
 * @num
 */
void dsaf_tbl_tcam_mcast_show(u32 dsaf_id, u32 address, u32 num)
{
	union dsaf_tbl_tcam_mcast_cfg_0 tcam_read_data0;
	u32 tcam_read_data1;
	u32 tcam_read_data2;
	u32 tcam_read_data3;
	union dsaf_tbl_tcam_mcast_cfg_4 tcam_read_data4;

	u32 dah;
	u32 dal;
	u32 port_num;
	u32 vlan_id;

	u32 i;

	osal_pr("tcam mcast item: \r\n");
	osal_pr("addr|dah        |dal   |vlanId|port|old");
	osal_pr("|vld|vmgegxge135bitmsk\r\n");

	for (i = 0; i < num; i++) {
		/*Write Addr*/
		dsaf_tbl_tcam_addr_cfg(dsaf_id, (address + i));

		/*read tcam item puls*/
		dsaf_tbl_tcam_load_pul(dsaf_id);

		/*read tcam data*/
		dah = dsaf_read(dsaf_id, DSAF_TBL_TCAM_RDATA_HIGH_0_REG);
		dal = dsaf_read(dsaf_id, DSAF_TBL_TCAM_RDATA_LOW_0_REG);
		vlan_id = (dal & 0x0000fff0) >> 4;
		port_num = dal & 0xf;
		dal = (dal & 0xffff0000) >> 16;

		/*read tcam mcast*/
		tcam_read_data0.u32
			= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA0_0_REG);
		tcam_read_data1
			= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA1_0_REG);
		tcam_read_data2
			= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA2_0_REG);
		tcam_read_data3
			= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA3_0_REG);
		tcam_read_data4.u32
			= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA4_0_REG);

		osal_pr("%4d|0x%08x|0x%04x|0x%03x | %d  | %d | %d ",
			(address + i), dah, dal, vlan_id, port_num,
			tcam_read_data4.bits.tbl_mcast_old_en,
			tcam_read_data4.bits.tbl_mcast_item_vld);
		osal_pr("|0x%02x%08x%08x%08x%08x|\r\n",
			tcam_read_data4.bits.tbl_mcast_vm128_122,
			tcam_read_data3, tcam_read_data2,
			tcam_read_data1, tcam_read_data0.u32);
	}
}

/**
 * dsaf_tbl_line_single_show - INT
 * @dsaf_id: dsa fabric id
 * @address
 * @num
 */
void dsaf_tbl_line_single_show(u32 dsaf_id, u32 address)
{
	union dsaf_tbl_lin_cfg o_tbl_line_read_data;

	/*Write Addr*/
	dsaf_tbl_line_addr_cfg(dsaf_id, address);

	/*read tcam item puls*/
	dsaf_tbl_line_load_pul(dsaf_id);

	/*read line*/
	o_tbl_line_read_data.u32 = dsaf_read(dsaf_id, DSAF_TBL_LIN_RDATA_0_REG);


	osal_pr("%5d|   %d   |%d |  %3d |\r\n",
		address, o_tbl_line_read_data.bits.tbl_line_mac_discard,
		o_tbl_line_read_data.bits.tbl_line_dvc,
		o_tbl_line_read_data.bits.tbl_line_out_port);
}

/**
 * dsaf_tbl_line_show - INT
 * @dsaf_id: dsa fabric id
 * @address
 * @num
 */
void dsaf_tbl_line_show(u32 dsaf_id, u32 address, u32 num)
{
	u32 i;
	union dsaf_tbl_lin_cfg o_tbl_line_read_data;

	osal_pr("line item: \r\n");
	osal_pr("|addr |dis|vc|out||addr |dis|vc|out||addr ");
	osal_pr("|dis|vc|out||addr |dis|vc|out|\r\n");

	for (i = 0; i < num; i++) {
		/*Write Addr*/
		dsaf_tbl_line_addr_cfg(dsaf_id, (address + i));

		/*read tcam item puls*/
		dsaf_tbl_line_load_pul(dsaf_id);

		/*read line*/
		o_tbl_line_read_data.u32
			= dsaf_read(dsaf_id, DSAF_TBL_LIN_RDATA_0_REG);

		osal_pr("|%5d|%3d|%2d|%3d|", (address + i),
			o_tbl_line_read_data.bits.tbl_line_mac_discard,
			o_tbl_line_read_data.bits.tbl_line_dvc,
			o_tbl_line_read_data.bits.tbl_line_out_port);

		if (3 == (address + i) % 4)
			osal_pr("\r\n");
	}
}



/**
 * dsaf_dfx - show all dfx
 * @dsaf_id: dsa fabric id
 * @vm_id
 */
void  dsaf_dfx(struct dsaf_device *dsaf_dev)
{
	u32 i;
	u32 dsaf_id = dsaf_dev->chip_id;

	osal_pr("++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf comm module:\r\n");
	osal_pr("++++++++++++++++++++++++++++++\r\n");
	dsaf_comm_state_show(dsaf_id);
	dsaf_sram_init_over_show(dsaf_id);
	dsaf_vm_def_vlan_show(dsaf_id, 0, 129);
	dsaf_pfc_en_show(dsaf_id);

	osal_pr("++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf inode module:\r\n");
	osal_pr("++++++++++++++++++++++++++++++\r\n");
	dsaf_inode_state_show(dsaf_id);

	osal_pr("++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf sbm module:\r\n");
	osal_pr("++++++++++++++++++++++++++++++\r\n");
	dsaf_sbm_state_show(dsaf_id);

	osal_pr("++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf voq module:\r\n");
	osal_pr("++++++++++++++++++++++++++++++\r\n");
	dsaf_voq_state_show(dsaf_id);
	dsaf_voq_xod_req_show(dsaf_id);
	dsaf_voq_bp_thrd_cfg_show(dsaf_id);

	osal_pr("++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf xod module:\r\n");
	osal_pr("++++++++++++++++++++++++++++++\r\n");
	dsaf_xod_pause_para_cfg_show(dsaf_id);
	dsaf_xod_connect_state_show(dsaf_id);
	dsaf_xod_ets_cfg_show(dsaf_id);
	dsaf_xod_fifo_state_show(dsaf_id);
	dsaf_xod_grant_signle_show(dsaf_id);

	osal_pr("++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf tbl module:\r\n");
	osal_pr("++++++++++++++++++++++++++++++\r\n");
	dsaf_tbl_cfg_show(dsaf_id);
	dsaf_tbl_old_scan_val_show(dsaf_id);
	dsaf_tbl_mis_info_show(dsaf_id);

	osal_pr("++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf other module:\r\n");
	osal_pr("++++++++++++++++++++++++++++++\r\n");
	dsaf_ge_xge_cfg_show(dsaf_id);
	dsaf_netport_state_show(dsaf_id);
	dsaf_asyn_fifo_wl_show(dsaf_id);

	osal_pr("++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf interupt:\r\n");
	osal_pr("++++++++++++++++++++++++++++++\r\n");
	dsaf_ge_xge_int_show(dsaf_id);
	dsaf_ppe_int_show(dsaf_id);
	dsaf_rocee_int_show(dsaf_id);
	dsaf_tbl_int_show(dsaf_id);

	osal_pr("++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf ecc:\r\n");
	osal_pr("++++++++++++++++++++++++++++++\r\n");
	dsaf_ecc_err_state_show(dsaf_id);

	osal_pr("++++++++++++++++++++++++++++++\r\n");
	osal_pr("dsaf statistics:\r\n");
	osal_pr("++++++++++++++++++++++++++++++\r\n");

	for (i = 0; i < 18; i++) {
		dsaf_inode_stat_show(dsaf_dev, i);
		dsaf_sbm_stat_show(dsaf_id, i);
		dsaf_voq_stat_show(dsaf_id, i);
		dsaf_xod_stat_show(dsaf_id, i);
	}

	for (i = 0; i < DSAF_TBL_NUM; i++)
		dsaf_tbl_stat_show(dsaf_id, i);

}


/*********************** SYSFS attr def *************************/

/*none param file attr and ayyr-list,
			micro contain 2 op-fun anf 1 golbo-att-value*/

sysfs_dsaf_fun0_define(comm_state, dsaf_comm_state_show, NULL);
sysfs_dsaf_fun0_define(ge_xge_cfg, dsaf_ge_xge_cfg_show, NULL);
sysfs_dsaf_fun0_define(sbm_state, dsaf_sbm_state_show, NULL);
sysfs_dsaf_fun0_define(inode_state, dsaf_inode_state_show, NULL);
sysfs_dsaf_fun0_define(voq_state, dsaf_voq_state_show, NULL);

sysfs_dsaf_dev_fun0_define(dfx_all, dsaf_dfx, NULL);

/*for tcam add or del**/
/*
echo 0x0180c2000002,0,2,0,1 > modify_tcam
echo 0x0180c2000002,0,3,0,1 > modify_tcam
echo 0x0180c2000002,0,2,127,1 > modify_tcam
echo 0x0180c2000002,0,3,127,1 > modify_tcam

*/
void sysfs_dsaf_add_tcam(struct device *dev, const char *buf)
/*int dsaf_modify_tcam(struct dsaf_device *dsaf_dev,
	u64 mac_addr,  u32 vlan_id, u32 in_port_num, u32 out_port_num, u32 en)*/
{
	int ret = 0, i = 0;
	u8 addr[MAC_NUM_OCTETS_PER_ADDR]
		= {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct dsaf_drv_mac_single_dest_entry mac_entry;
	struct dsaf_device *dsaf_dev =
		(struct dsaf_device *)dev_get_drvdata(dev);

	u64 mac_addr;
	u32 vlan_id;
	u32 in_port_num;
	u32 out_port_num;
	u32 en;

	sscanf(buf, "0x%llx,%u,%u,%u,%u", &mac_addr,
		&vlan_id, &in_port_num, &out_port_num, &en);

	for (i = 0; i < MAC_NUM_OCTETS_PER_ADDR; i++)
		addr[MAC_NUM_OCTETS_PER_ADDR - 1 - i] = (u8)((mac_addr >> (8 * i)) & 0xff);

	log_info(dsaf_dev->dev,
		"dsaf_dev(=0x%p) set MAC address %pM, mac_addr=0x%llx\n",
		dsaf_dev, addr, mac_addr);

	if ((NULL != dsaf_dev) && (NULL != dsaf_dev->del_mac_mc_port)
		&& (NULL != dsaf_dev->add_mac_mc_port)) {
		memcpy(mac_entry.addr, addr, sizeof(mac_entry.addr));
		mac_entry.in_vlan_id = vlan_id;
		mac_entry.in_port_num = in_port_num;
		mac_entry.port_num = out_port_num;

		if (!en)
			ret = dsaf_dev->del_mac_mc_port(dsaf_dev, &mac_entry);
		else
			ret = dsaf_dev->add_mac_mc_port(dsaf_dev, &mac_entry);

		if (ret)
			log_err(dsaf_dev->dev,
				"dsaf_modify_tcam faild, ret = %#x!\n", ret);
	}

}
void sysfs_dsaf_tcam(struct device *dev)
{
}
/*g_sysfs_dsaf_tcam*/
sysyfs_dsaf_file_attr_def(modify_tcam,
	sysfs_dsaf_tcam, sysfs_dsaf_add_tcam);


static struct attribute *g_dsaf_sys_attrs_list_top[] = {
	&g_sysfs_dsaf_comm_state_show.attr,
	&g_sysfs_dsaf_ge_xge_cfg_show.attr,
	&g_sysfs_dsaf_sbm_state_show.attr,
	&g_sysfs_dsaf_inode_state_show.attr,
	&g_sysfs_dsaf_voq_state_show.attr,
	&g_sysfs_dsaf_dfx.attr,
	&g_sysfs_dsaf_tcam.attr,
	NULL
};

/*one param file attr and ayyr-list,
				micro contain 2 op-fun anf 1 golbo-att-value*/
sysfs_dsaf_dev_fun1_define(0, dsaf_inode_stat_show, NULL, 0);
sysfs_dsaf_dev_fun1_define(1, dsaf_inode_stat_show, NULL, 1);
sysfs_dsaf_dev_fun1_define(2, dsaf_inode_stat_show, NULL, 2);
sysfs_dsaf_dev_fun1_define(3, dsaf_inode_stat_show, NULL, 3);
sysfs_dsaf_dev_fun1_define(4, dsaf_inode_stat_show, NULL, 4);
sysfs_dsaf_dev_fun1_define(5, dsaf_inode_stat_show, NULL, 5);
sysfs_dsaf_dev_fun1_define(6, dsaf_inode_stat_show, NULL, 6);
sysfs_dsaf_dev_fun1_define(7, dsaf_inode_stat_show, NULL, 7);
sysfs_dsaf_dev_fun1_define(8, dsaf_inode_stat_show, NULL, 8);
sysfs_dsaf_dev_fun1_define(9, dsaf_inode_stat_show, NULL, 9);
sysfs_dsaf_dev_fun1_define(10, dsaf_inode_stat_show, NULL, 10);
sysfs_dsaf_dev_fun1_define(11, dsaf_inode_stat_show, NULL, 11);
sysfs_dsaf_dev_fun1_define(12, dsaf_inode_stat_show, NULL, 12);
sysfs_dsaf_dev_fun1_define(13, dsaf_inode_stat_show, NULL, 13);
sysfs_dsaf_dev_fun1_define(14, dsaf_inode_stat_show, NULL, 14);
sysfs_dsaf_dev_fun1_define(15, dsaf_inode_stat_show, NULL, 15);
sysfs_dsaf_dev_fun1_define(16, dsaf_inode_stat_show, NULL, 16);
sysfs_dsaf_dev_fun1_define(17, dsaf_inode_stat_show, NULL, 17);
static struct attribute *g_dsaf_sys_attrs_list_inode_single[] = {
	&g_sysfs_dsaf_inode_stat_show0.attr,
	&g_sysfs_dsaf_inode_stat_show1.attr,
	&g_sysfs_dsaf_inode_stat_show2.attr,
	&g_sysfs_dsaf_inode_stat_show3.attr,
	&g_sysfs_dsaf_inode_stat_show4.attr,
	&g_sysfs_dsaf_inode_stat_show5.attr,
	&g_sysfs_dsaf_inode_stat_show6.attr,
	&g_sysfs_dsaf_inode_stat_show7.attr,
	&g_sysfs_dsaf_inode_stat_show8.attr,
	&g_sysfs_dsaf_inode_stat_show9.attr,
	&g_sysfs_dsaf_inode_stat_show10.attr,
	&g_sysfs_dsaf_inode_stat_show11.attr,
	&g_sysfs_dsaf_inode_stat_show12.attr,
	&g_sysfs_dsaf_inode_stat_show13.attr,
	&g_sysfs_dsaf_inode_stat_show14.attr,
	&g_sysfs_dsaf_inode_stat_show15.attr,
	&g_sysfs_dsaf_inode_stat_show16.attr,
	&g_sysfs_dsaf_inode_stat_show17.attr,
	NULL
};
sysfs_dsaf_fun1_define(0, dsaf_xod_stat_show, NULL, 0);
sysfs_dsaf_fun1_define(1, dsaf_xod_stat_show, NULL, 1);
sysfs_dsaf_fun1_define(2, dsaf_xod_stat_show, NULL, 2);
sysfs_dsaf_fun1_define(3, dsaf_xod_stat_show, NULL, 3);
sysfs_dsaf_fun1_define(4, dsaf_xod_stat_show, NULL, 4);
sysfs_dsaf_fun1_define(5, dsaf_xod_stat_show, NULL, 5);
sysfs_dsaf_fun1_define(6, dsaf_xod_stat_show, NULL, 6);
sysfs_dsaf_fun1_define(7, dsaf_xod_stat_show, NULL, 7);
sysfs_dsaf_fun1_define(8, dsaf_xod_stat_show, NULL, 8);
sysfs_dsaf_fun1_define(9, dsaf_xod_stat_show, NULL, 9);
sysfs_dsaf_fun1_define(10, dsaf_xod_stat_show, NULL, 10);
sysfs_dsaf_fun1_define(11, dsaf_xod_stat_show, NULL, 11);
sysfs_dsaf_fun1_define(12, dsaf_xod_stat_show, NULL, 12);
sysfs_dsaf_fun1_define(13, dsaf_xod_stat_show, NULL, 13);
sysfs_dsaf_fun1_define(14, dsaf_xod_stat_show, NULL, 14);
sysfs_dsaf_fun1_define(15, dsaf_xod_stat_show, NULL, 15);
sysfs_dsaf_fun1_define(16, dsaf_xod_stat_show, NULL, 16);
sysfs_dsaf_fun1_define(17, dsaf_xod_stat_show, NULL, 17);
static struct attribute *g_dsaf_sys_attrs_list_xod_single[] = {
	&g_sysfs_dsaf_xod_stat_show0.attr,
	&g_sysfs_dsaf_xod_stat_show1.attr,
	&g_sysfs_dsaf_xod_stat_show2.attr,
	&g_sysfs_dsaf_xod_stat_show3.attr,
	&g_sysfs_dsaf_xod_stat_show4.attr,
	&g_sysfs_dsaf_xod_stat_show5.attr,
	&g_sysfs_dsaf_xod_stat_show6.attr,
	&g_sysfs_dsaf_xod_stat_show7.attr,
	&g_sysfs_dsaf_xod_stat_show8.attr,
	&g_sysfs_dsaf_xod_stat_show9.attr,
	&g_sysfs_dsaf_xod_stat_show10.attr,
	&g_sysfs_dsaf_xod_stat_show11.attr,
	&g_sysfs_dsaf_xod_stat_show12.attr,
	&g_sysfs_dsaf_xod_stat_show13.attr,
	&g_sysfs_dsaf_xod_stat_show14.attr,
	&g_sysfs_dsaf_xod_stat_show15.attr,
	&g_sysfs_dsaf_xod_stat_show16.attr,
	&g_sysfs_dsaf_xod_stat_show17.attr,
	NULL
};
sysfs_dsaf_fun1_define(0, dsaf_sbm_stat_show, NULL, 0);
sysfs_dsaf_fun1_define(1, dsaf_sbm_stat_show, NULL, 1);
sysfs_dsaf_fun1_define(2, dsaf_sbm_stat_show, NULL, 2);
sysfs_dsaf_fun1_define(3, dsaf_sbm_stat_show, NULL, 3);
sysfs_dsaf_fun1_define(4, dsaf_sbm_stat_show, NULL, 4);
sysfs_dsaf_fun1_define(5, dsaf_sbm_stat_show, NULL, 5);
sysfs_dsaf_fun1_define(6, dsaf_sbm_stat_show, NULL, 6);
sysfs_dsaf_fun1_define(7, dsaf_sbm_stat_show, NULL, 7);
sysfs_dsaf_fun1_define(8, dsaf_sbm_stat_show, NULL, 8);
sysfs_dsaf_fun1_define(9, dsaf_sbm_stat_show, NULL, 9);
sysfs_dsaf_fun1_define(10, dsaf_sbm_stat_show, NULL, 10);
sysfs_dsaf_fun1_define(11, dsaf_sbm_stat_show, NULL, 11);
sysfs_dsaf_fun1_define(12, dsaf_sbm_stat_show, NULL, 12);
sysfs_dsaf_fun1_define(13, dsaf_sbm_stat_show, NULL, 13);
sysfs_dsaf_fun1_define(14, dsaf_sbm_stat_show, NULL, 14);
sysfs_dsaf_fun1_define(15, dsaf_sbm_stat_show, NULL, 15);
sysfs_dsaf_fun1_define(16, dsaf_sbm_stat_show, NULL, 16);
sysfs_dsaf_fun1_define(17, dsaf_sbm_stat_show, NULL, 17);
static struct attribute *g_dsaf_sys_attrs_list_sbm_single[] = {
	&g_sysfs_dsaf_sbm_stat_show0.attr,
	&g_sysfs_dsaf_sbm_stat_show1.attr,
	&g_sysfs_dsaf_sbm_stat_show2.attr,
	&g_sysfs_dsaf_sbm_stat_show3.attr,
	&g_sysfs_dsaf_sbm_stat_show4.attr,
	&g_sysfs_dsaf_sbm_stat_show5.attr,
	&g_sysfs_dsaf_sbm_stat_show6.attr,
	&g_sysfs_dsaf_sbm_stat_show7.attr,
	&g_sysfs_dsaf_sbm_stat_show8.attr,
	&g_sysfs_dsaf_sbm_stat_show9.attr,
	&g_sysfs_dsaf_sbm_stat_show10.attr,
	&g_sysfs_dsaf_sbm_stat_show11.attr,
	&g_sysfs_dsaf_sbm_stat_show12.attr,
	&g_sysfs_dsaf_sbm_stat_show13.attr,
	&g_sysfs_dsaf_sbm_stat_show14.attr,
	&g_sysfs_dsaf_sbm_stat_show15.attr,
	&g_sysfs_dsaf_sbm_stat_show16.attr,
	&g_sysfs_dsaf_sbm_stat_show17.attr,
	NULL
};

sysfs_dsaf_fun1_define(0, dsaf_voq_stat_show, NULL, 0);
sysfs_dsaf_fun1_define(1, dsaf_voq_stat_show, NULL, 1);
sysfs_dsaf_fun1_define(2, dsaf_voq_stat_show, NULL, 2);
sysfs_dsaf_fun1_define(3, dsaf_voq_stat_show, NULL, 3);
sysfs_dsaf_fun1_define(4, dsaf_voq_stat_show, NULL, 4);
sysfs_dsaf_fun1_define(5, dsaf_voq_stat_show, NULL, 5);
sysfs_dsaf_fun1_define(6, dsaf_voq_stat_show, NULL, 6);
sysfs_dsaf_fun1_define(7, dsaf_voq_stat_show, NULL, 7);
sysfs_dsaf_fun1_define(8, dsaf_voq_stat_show, NULL, 8);
sysfs_dsaf_fun1_define(9, dsaf_voq_stat_show, NULL, 9);
sysfs_dsaf_fun1_define(10, dsaf_voq_stat_show, NULL, 10);
sysfs_dsaf_fun1_define(11, dsaf_voq_stat_show, NULL, 11);
sysfs_dsaf_fun1_define(12, dsaf_voq_stat_show, NULL, 12);
sysfs_dsaf_fun1_define(13, dsaf_voq_stat_show, NULL, 13);
sysfs_dsaf_fun1_define(14, dsaf_voq_stat_show, NULL, 14);
sysfs_dsaf_fun1_define(15, dsaf_voq_stat_show, NULL, 15);
sysfs_dsaf_fun1_define(16, dsaf_voq_stat_show, NULL, 16);
sysfs_dsaf_fun1_define(17, dsaf_voq_stat_show, NULL, 17);
static struct attribute *g_dsaf_sys_attrs_list_voq_single[] = {
	&g_sysfs_dsaf_voq_stat_show0.attr,
	&g_sysfs_dsaf_voq_stat_show1.attr,
	&g_sysfs_dsaf_voq_stat_show2.attr,
	&g_sysfs_dsaf_voq_stat_show3.attr,
	&g_sysfs_dsaf_voq_stat_show4.attr,
	&g_sysfs_dsaf_voq_stat_show5.attr,
	&g_sysfs_dsaf_voq_stat_show6.attr,
	&g_sysfs_dsaf_voq_stat_show7.attr,
	&g_sysfs_dsaf_voq_stat_show8.attr,
	&g_sysfs_dsaf_voq_stat_show9.attr,
	&g_sysfs_dsaf_voq_stat_show10.attr,
	&g_sysfs_dsaf_voq_stat_show11.attr,
	&g_sysfs_dsaf_voq_stat_show12.attr,
	&g_sysfs_dsaf_voq_stat_show13.attr,
	&g_sysfs_dsaf_voq_stat_show14.attr,
	&g_sysfs_dsaf_voq_stat_show15.attr,
	&g_sysfs_dsaf_voq_stat_show16.attr,
	&g_sysfs_dsaf_voq_stat_show17.attr,
	NULL
};

sysfs_dsaf_fun1_define(0, dsaf_tbl_stat_show, NULL, 0);
sysfs_dsaf_fun1_define(1, dsaf_tbl_stat_show, NULL, 1);
sysfs_dsaf_fun1_define(2, dsaf_tbl_stat_show, NULL, 2);
sysfs_dsaf_fun1_define(3, dsaf_tbl_stat_show, NULL, 3);
sysfs_dsaf_fun1_define(4, dsaf_tbl_stat_show, NULL, 4);
sysfs_dsaf_fun1_define(5, dsaf_tbl_stat_show, NULL, 5);
sysfs_dsaf_fun1_define(6, dsaf_tbl_stat_show, NULL, 6);
sysfs_dsaf_fun1_define(7, dsaf_tbl_stat_show, NULL, 7);

static struct attribute *g_dsaf_sys_attrs_list_tbl_single[] = {
	&g_sysfs_dsaf_tbl_stat_show0.attr,
	&g_sysfs_dsaf_tbl_stat_show1.attr,
	&g_sysfs_dsaf_tbl_stat_show2.attr,
	&g_sysfs_dsaf_tbl_stat_show3.attr,
	&g_sysfs_dsaf_tbl_stat_show4.attr,
	&g_sysfs_dsaf_tbl_stat_show5.attr,
	&g_sysfs_dsaf_tbl_stat_show6.attr,
	&g_sysfs_dsaf_tbl_stat_show7.attr,
	NULL
};



/*two params file attr and ayyr-list,
	micro contain 2 op-fun anf 1 golbo-att-value
for tbl is here:
call fun sipmle cmd :
call "dsaf_tbl_tcam_ucast_show", 0, 0x000, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x010, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x020, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x030, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x040, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x050, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x060, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x070, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x080, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x090, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x0a0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x0b0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x0c0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x0d0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x0e0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x0f0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x100, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x110, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x120, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x130, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x140, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x150, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x160, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x170, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x180, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x190, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x1a0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x1b0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x1c0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x1d0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x1e0, 16
call "dsaf_tbl_tcam_ucast_show", 0, 0x1f0, 16*/
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x000, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x010, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x020, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x030, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x040, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x050, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x060, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x070, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x080, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x090, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x0a0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x0b0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x0c0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x0d0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x0e0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x0f0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x100, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x110, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x120, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x130, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x140, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x150, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x160, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x170, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x180, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x190, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x1a0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x1b0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x1c0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x1d0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x1e0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_ucast_show, NULL, 0x1f0, 16);
static struct attribute *g_dsaf_sys_attrs_list_tcam_ucast_addr_16[][2] = {
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x00016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x01016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x02016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x03016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x04016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x05016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x06016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x07016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x08016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x09016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x0a016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x0b016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x0c016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x0d016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x0e016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x0f016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x10016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x11016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x12016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x13016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x14016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x15016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x16016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x17016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x18016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x19016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x1a016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x1b016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x1c016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x1d016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x1e016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_ucast_show0x1f016.attr, NULL},

};
/*the same to mcast*/
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x000, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x010, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x020, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x030, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x040, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x050, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x060, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x070, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x080, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x090, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x0a0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x0b0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x0c0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x0d0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x0e0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x0f0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x100, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x110, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x120, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x130, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x140, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x150, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x160, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x170, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x180, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x190, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x1a0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x1b0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x1c0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x1d0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x1e0, 16);
sysfs_dsaf_fun2_define(16, dsaf_tbl_tcam_mcast_show, NULL, 0x1f0, 16);
static struct attribute *g_dsaf_sys_attrs_list_tcam_mcast_addr_16[][2] = {
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x00016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x01016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x02016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x03016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x04016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x05016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x06016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x07016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x08016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x09016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x0a016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x0b016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x0c016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x0d016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x0e016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x0f016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x10016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x11016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x12016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x13016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x14016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x15016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x16016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x17016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x18016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x19016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x1a016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x1b016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x1c016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x1d016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x1e016.attr, NULL},
		{&g_sysfs_dsaf_tbl_tcam_mcast_show0x1f016.attr, NULL},

};

/*the same to line tbl*/
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x0000, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x0800, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x1000, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x1800, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x2000, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x2800, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x3000, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x3800, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x4000, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x4800, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x5000, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x5800, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x6000, 2048);
sysfs_dsaf_fun2_define(2048, dsaf_tbl_line_show, NULL, 0x6800, 2048);

static struct attribute *g_dsaf_sys_attrs_list_line_addr_2048[][2] = {
		{&g_sysfs_dsaf_tbl_line_show0x00002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x08002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x10002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x18002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x20002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x28002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x30002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x38002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x40002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x48002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x50002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x58002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x60002048.attr, NULL},
		{&g_sysfs_dsaf_tbl_line_show0x68002048.attr, NULL},
};


/**public opt fun,	opt struct**/
static const struct sysfs_ops g_dsaf_sys_ops = {
	.show = dsaf_sys_show,
	.store = dsaf_sys_store,
};

/**this is the dir name config array*/
static struct dsaf_sysfs_config g_dsaf_sysfs_cfg[] = {
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	= g_dsaf_sys_attrs_list_top,
		},
		.obj_name = "user_control",
		.father_idx = 0xff,
	},
	{/*dir idx = 1*/
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= NULL,
			.default_attrs	= NULL,
		},
		.obj_name = "tcam_ucast",
		.father_idx = 0x0,
	},
	{/*dir idx = 2*/
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= NULL,
			.default_attrs	= NULL,
		},
		.obj_name = "tcam_mcast",
		.father_idx = 0x0,
	},
	{/*dir idx = 3*/
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= NULL,
			.default_attrs	= NULL,
		},
		.obj_name = "line_tbl",
		.father_idx = 0x0,
	},

	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	= g_dsaf_sys_attrs_list_inode_single,
		},
		.obj_name = "inode_single",
		.father_idx = 0x0,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	= g_dsaf_sys_attrs_list_xod_single,
		},
		.obj_name = "xod_single",
		.father_idx = 0x0,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	= g_dsaf_sys_attrs_list_sbm_single,
		},
		.obj_name = "sbm_single",
		.father_idx = 0x0,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	= g_dsaf_sys_attrs_list_voq_single,
		},
		.obj_name = "voq_single",
		.father_idx = 0x0,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	= g_dsaf_sys_attrs_list_tbl_single,
		},
		.obj_name = "tbl_single",
		.father_idx = 0x0,
	},

#if 1
	/*uc-tab dir -sysfs --- begine 32 entrys **/
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs =
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[0],
		},
		.obj_name = "0x000",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[1],
		},
		.obj_name = "0x010",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[2],
		},
		.obj_name = "0x020",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[3],
		},
		.obj_name = "0x030",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[4],
		},
		.obj_name = "0x040",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[5],
		},
		.obj_name = "0x050",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[6],
		},
		.obj_name = "0x060",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[7],
		},
		.obj_name = "0x070",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[8],
		},
		.obj_name = "0x080",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[9],
		},
		.obj_name = "0x090",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[10],
		},
		.obj_name = "0x0a0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[11],
		},
		.obj_name = "0x0b0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[12],
		},
		.obj_name = "0x0c0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[13],
		},
		.obj_name = "0x0d0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[14],
		},
		.obj_name = "0x0e0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[15],
		},
		.obj_name = "0x0f0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[16],
		},
		.obj_name = "0x100",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[17],
		},
		.obj_name = "0x110",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[18],
		},
		.obj_name = "0x120",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[19],
		},
		.obj_name = "0x130",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[20],
		},
		.obj_name = "0x140",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[21],
		},
		.obj_name = "0x150",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[22],
		},
		.obj_name = "0x160",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[23],
		},
		.obj_name = "0x170",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[24],
		},
		.obj_name = "0x180",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[25],
		},
		.obj_name = "0x190",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[26],
		},
		.obj_name = "0x1a0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[27],
		},
		.obj_name = "0x1b0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[28],
		},
		.obj_name = "0x1c0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[29],
		},
		.obj_name = "0x1d0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[30],
		},
		.obj_name = "0x1e0",
		.father_idx = 0x1,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_ucast_addr_16[31],
		},
		.obj_name = "0x1f0",
		.father_idx = 0x1,
	},
	/*uc-tab dir -sysfs --- end **/
#endif

#if 1
	/*mc-tab dir -sysfs --- begine 32 entrys **/
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[0],
		},
		.obj_name = "0x000",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[1],
		},
		.obj_name = "0x010",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[2],
		},
		.obj_name = "0x020",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[3],
		},
		.obj_name = "0x030",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[4],
		},
		.obj_name = "0x040",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[5],
		},
		.obj_name = "0x050",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[6],
		},
		.obj_name = "0x060",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[7],
		},
		.obj_name = "0x070",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[8],
		},
		.obj_name = "0x080",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[9],
		},
		.obj_name = "0x090",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[10],
		},
		.obj_name = "0x0a0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[11],
		},
		.obj_name = "0x0b0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[12],
		},
		.obj_name = "0x0c0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[13],
		},
		.obj_name = "0x0d0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[14],
		},
		.obj_name = "0x0e0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[15],
		},
		.obj_name = "0x0f0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[16],
		},
		.obj_name = "0x100",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[17],
		},
		.obj_name = "0x110",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[18],
		},
		.obj_name = "0x120",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[19],
		},
		.obj_name = "0x130",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[20],
		},
		.obj_name = "0x140",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[21],
		},
		.obj_name = "0x150",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[22],
		},
		.obj_name = "0x160",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[23],
		},
		.obj_name = "0x170",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[24],
		},
		.obj_name = "0x180",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[25],
		},
		.obj_name = "0x190",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[26],
		},
		.obj_name = "0x1a0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[27],
		},
		.obj_name = "0x1b0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[28],
		},
		.obj_name = "0x1c0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[29],
		},
		.obj_name = "0x1d0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[30],
		},
		.obj_name = "0x1e0",
		.father_idx = 0x2,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_tcam_mcast_addr_16[31],
		},
		.obj_name = "0x1f0",
		.father_idx = 0x2,
	},
	/*mc-tab dir -sysfs --- end **/
#endif

#if 1
	/*line-tab dir -sysfs --- begine 2048 entrys **/
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[0],
		},
		.obj_name = "0x0000",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[1],
		},
		.obj_name = "0x0800",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[2],
		},
		.obj_name = "0x1000",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[3],
		},
		.obj_name = "0x1800",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[4],
		},
		.obj_name = "0x2000",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[5],
		},
		.obj_name = "0x2800",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[6],
		},
		.obj_name = "0x3000",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[7],
		},
		.obj_name = "0x3800",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[8],
		},
		.obj_name = "0x4000",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[9],
		},
		.obj_name = "0x4800",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[10],
		},
		.obj_name = "0x5000",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[11],
		},
		.obj_name = "0x5800",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[12],
		},
		.obj_name = "0x6000",
		.father_idx = 0x3,
	},
	{
		.obj_type = {
			.release	= NULL,
			.sysfs_ops	= &g_dsaf_sys_ops,
			.default_attrs	=
				g_dsaf_sys_attrs_list_line_addr_2048[13],
		},
		.obj_name = "0x6800",
		.father_idx = 0x3,
	},
	/*lint-tab dir -sysfs --- end **/
#endif


};

/*define mut obj and dev,  for do free and del files and dirs*/
static struct dsaf_dev_kobj
	g_dsaf_kobj[DSAF_MAX_CHIP_NUM][ARRAY_SIZE(g_dsaf_sysfs_cfg)];
static unsigned int g_dsaf_kobj_cnt[DSAF_MAX_CHIP_NUM] = {0};


/*********************************************************************/

ssize_t dsaf_sys_show(
	struct kobject *kobj,  struct attribute *attr , char *buf)
{
	struct dsaf_sys_attr *sys_attr =
		container_of(attr, struct dsaf_sys_attr, attr);
	struct dsaf_dev_kobj *dsaf_kobj =
		container_of(kobj, struct dsaf_dev_kobj, kobj);
	struct device *dev = dsaf_kobj->dev;

	/*call back self show function */
	if (sys_attr->show) {
		sys_attr->show(dev);
		return strlen(buf);
	}
	return -ENOSYS;
}

ssize_t dsaf_sys_store(struct kobject *kobj,  struct attribute *attr,
			 const char *buf,	size_t count)
{
	struct dsaf_sys_attr *sys_attr =
		container_of(attr,	struct dsaf_sys_attr,  attr);
	struct dsaf_dev_kobj *dsaf_kobj =
		container_of(kobj,	struct dsaf_dev_kobj,  kobj);
	struct device *p_dev = dsaf_kobj->dev;

	/*call back self store function */
	if (sys_attr->store) {
		sys_attr->store(p_dev,	(char *)buf);
		return count;
	}

	return -ENOSYS;
}

/***********************************************************************/

int dsaf_add_sysfs(struct device *dev)
{
	int ret = 0;
	struct dsaf_device *dsaf_dev =
	    (struct dsaf_device *)dev_get_drvdata(dev);
	u32 chip_id = 0;
	u32 i = 0;

	log_dbg(dev, "dsa add sysfs begin\n");

	if (NULL == dsaf_dev) {
		log_err(dev, "dsaf_dev is NULL!\n");
		return -ENODEV;
	}

	chip_id = dsaf_dev->chip_id;

	/*note: caller check params..*/

	g_dsaf_kobj_cnt[chip_id] = 0;
	memset(g_dsaf_kobj[chip_id], 0,
		(ARRAY_SIZE(g_dsaf_sysfs_cfg)) * sizeof(struct dsaf_dev_kobj));

	/* sysfs path is "/sys/devices/platform/[cur mod name]
		/user_control" create sysfs files */
	/* eg: /sys/bus/platform/devices/[cur mod name] link to
		/sys/devices/platform/[cur mod name] */

	for (i = 0; i < (ARRAY_SIZE(g_dsaf_sysfs_cfg)); i++) {
		struct kobject *parent = &(dev->kobj);

		if (0 != i)
			parent = &(g_dsaf_kobj[chip_id]
			[(g_dsaf_sysfs_cfg[i].father_idx)].kobj);

		ret = kobject_init_and_add(&(g_dsaf_kobj[chip_id][i].kobj),
			&g_dsaf_sysfs_cfg[i].obj_type,
			parent, g_dsaf_sysfs_cfg[i].obj_name);

		if (ret) {
			log_err(dsaf_dev->dev,
				"kobject_init_and_add failed! ret=%d\n",
				ret);
			return ret;
		}

		g_dsaf_kobj[chip_id][i].dev = dev;

	}

	g_dsaf_kobj_cnt[chip_id] = i;

	log_dbg(dsaf_dev->dev,
		">>>dsaf[%d] dsaf add %2u sysfs success\n",
		chip_id, g_dsaf_kobj_cnt[chip_id]);

	return 0;
}
/*EXPORT_SYMBOL(hello_add_sysfs_usrctrl_file); */

void dsaf_del_sysfs(struct device *dev)
{
	u32 chip_id = 0;
	int i = 0;

	struct dsaf_device *dsaf_dev =
	    (struct dsaf_device *)dev_get_drvdata(dev);

	log_dbg(dsaf_dev->dev, "dsa del sysfs begin\n");

	chip_id = dsaf_dev->chip_id;

	/*do del sysfs files*/
	for (i = g_dsaf_kobj_cnt[chip_id] - 1; i >= 0; i--)
		kobject_del(&g_dsaf_kobj[chip_id][i].kobj);

	memset(g_dsaf_kobj[chip_id], 0,
		(ARRAY_SIZE(g_dsaf_sysfs_cfg)) * sizeof(struct dsaf_dev_kobj));
	g_dsaf_kobj_cnt[chip_id] = 0;

	log_dbg(dsaf_dev->dev,
		"<<<dsaf[%d] dsaf del sysfs success\n", chip_id);
}
/*EXPORT_SYMBOL(hello_del_sysfs_usrctrl_file); */


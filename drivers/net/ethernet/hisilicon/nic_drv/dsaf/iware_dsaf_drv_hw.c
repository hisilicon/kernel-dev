/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe489NY00RmHFrcWUTviF/V5VlslDUJe7l/UVGLTI+V57J6JFEUkvm2xFHEF4Nmp7nh2qG5
URiqSnsHDhkmgsdPF8RPR7VE67+Xj0z2JYSf3pp7cRvyl0F4iyRx+JFhz9bySQ==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/*******************************************************************************

  Hisilicon dsa fabric hal - support L2 switch
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

#include "iware_dsaf_main.h"
#include "iware_error.h"
#include "iware_dsaf_drv_hw.h"
#include "iware_dsaf_reg_define.h"
#include "iware_dsaf_reg_offset.h"
#include "hrd_log.h"

struct dsaf_cfg g_dsaf_cfg = {
	.dsaf_en = HRD_DSAF_NO_DSAF_MODE,
	.dsaf_tc_mode = HRD_DSAF_8TC_MODE,
	.dsaf_crc_add_en = 0,
	.dsaf_mix_mode = 0,
	.dsaf_local_addr_en = 0,
	.cnt_clr_ce = 1
};

/* u32 gdsaf_port_rate[DSAF_COMM_CHN] = {0, 0, 0, 0, 0, 0}; */
/* u32 gdsaf_pfc_unit_cnt[DSAF_COMM_CHN] = {41, 41, 41, 41, 41, 41};*/

/*u32 gdsaf_pfc_en[DSAF_COMM_CHN] =
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};*//*TBD*/
/* u32 gdsaf_pfc_en[DSAF_COMM_CHN] = {0, 0, 0, 0, 0, 0}; */

/* u32 gdsaf_stp_port_type[DSAF_XGE_CHN] = {4, 4, 4, 4, 4, 4}; */
/* u32 gdsaf_ppe_qid_cfg[DSAF_PPE_TX_NUM] = {22, 22, 22, 22, 22, 19}; */
/* u32 gdsaf_mix_def_qid[DSAF_PPE_TX_NUM] = {0, 1, 2, 3, 4, 5}; */
/* Modified by z00142220, 2014-09-07, non-dsaf mode, all port is assess */
/* u32 gdsaf_sw_port_type[DSAF_SW_PORT_NUM] = {1, 1, 1, 1, 1, 1, 1, 1}; */
/*u32 gdsaf_sw_port_type[DSAF_SW_PORT_NUM] =
	{0, 0, 0, 0, 0, 0, 0, 0};*//*TBD*/
/* u32 gdsaf_port_def_vlan[DSAF_SW_PORT_NUM] = {0, 0, 0, 0, 0, 0, 0, 0}; */
/* struct dsaf_vm_def_vlan gdsaf_vm_def_vlan[DSAF_PPE_QID_NUM]; */

/*----------XBAR_INODE-----------*/
/*cut through - 1, not cut through - 0 */
/* u32 ginode_cut_through_cfg = 1; */

static u32 ginode_in_port_num[DSAF_INODE_NUM] = {
	0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5};

struct dsaf_inode_pritc_cfg ginode_pri_tc_cfg[MAX_SBM_CFG_TYPE] = {
	{/*4tc mode*/
	/*data tc*/ {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
	/*man tc */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	/*pri0_tc*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	/*pri1_tc*/ {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	/*pri2_tc*/ {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	/*pri3_tc*/ {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
	/*pri4_tc*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	/*pri5_tc*/ {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	/*pri6_tc*/ {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	/*pri7_tc*/ {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}
	},
	{/*8tc mode*/
	/*data tc*/ {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
	/*man tc */ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	/*pri0_tc*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	/*pri1_tc*/ {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	/*pri2_tc*/ {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	/*pri3_tc*/ {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
	/*pri4_tc*/ {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4},
	/*pri5_tc*/ {5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5},
	/*pri6_tc*/ {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
	/*pri7_tc*/ {7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7}
	}
};

struct dsaf_int_xge_src g_dsaf_int_xge_stat[DSAF_XGE_CHN];
struct dsaf_int_ppe_src g_dsaf_int_ppe_stat[DSAF_PPE_CHN];
struct dsaf_int_rocee_src g_dsaf_int_rocee_stat[DSAF_ROCEE_CHN];
struct dsaf_int_tbl_src g_dsaf_int_tbl_stat[1];

/**
 * dsaf_en - config dsaf mode
 * @dsaf_id: dsa fabric id
 * @dsaf_cfg: config value
 */
static inline void dsaf_en(u32 dsaf_id, u32 dsaf_cfg)
{
	union dsaf_cfg_reg o_dsaf_cfg;

	o_dsaf_cfg.u32 = dsaf_read(dsaf_id, DSAF_CFG_0_REG);
	o_dsaf_cfg.bits.dsaf_en = dsaf_cfg;
	dsaf_write(dsaf_id, o_dsaf_cfg.u32, DSAF_CFG_0_REG);
}

/**
 * dsaf_tc_mode_cfg - config tc mode
 * @dsaf_id: dsa fabric id
 * @tc_mode: config value
 */
static inline void dsaf_tc_mode_cfg(u32 dsaf_id, u32 tc_mode)
{
	union dsaf_cfg_reg o_tc_mode;

	o_tc_mode.u32 = dsaf_read(dsaf_id, DSAF_CFG_0_REG);
	o_tc_mode.bits.dsaf_tc_mode = tc_mode;
	dsaf_write(dsaf_id, o_tc_mode.u32, DSAF_CFG_0_REG);
}

/**
 * dsaf_crc_en - config crc mode
 * @dsaf_id: dsa fabric id
 * @crc_en: config value
 */
static inline void dsaf_crc_en(u32 dsaf_id, u32 crc_en)
{
	union dsaf_cfg_reg o_crc_en;

	o_crc_en.u32 = dsaf_read(dsaf_id, DSAF_CFG_0_REG);
	o_crc_en.bits.dsaf_crc_en = crc_en;
	dsaf_write(dsaf_id, o_crc_en.u32, DSAF_CFG_0_REG);
}

/**
 * dsaf_sbm_link_sram_init_en - config dsaf_sbm_init_en
 * @dsaf_id: dsa fabric id
 */
static inline void dsaf_sbm_link_sram_init_en(u32 dsaf_id)
{
	union dsaf_cfg_reg o_sbm_link_sram_init_en;

	o_sbm_link_sram_init_en.u32 = dsaf_read(dsaf_id, DSAF_CFG_0_REG);
	o_sbm_link_sram_init_en.bits.dsaf_sbm_init_en = 1;
	dsaf_write(dsaf_id, o_sbm_link_sram_init_en.u32, DSAF_CFG_0_REG);
}

/**
 * dsaf_mix_mode_cfg - config mode_cfg
 * @dsaf_id: dsa fabric id
 * @dsaf_mix_mode_cfg: config value
 */
void dsaf_mix_mode_cfg(u32 dsaf_id, u32 mode_cfg)
{
	union dsaf_cfg_reg o_dsaf_mix_mode_cfg;

	o_dsaf_mix_mode_cfg.u32 = dsaf_read(dsaf_id, DSAF_CFG_0_REG);
	o_dsaf_mix_mode_cfg.bits.dsaf_mix_mode = mode_cfg;
	dsaf_write(dsaf_id, o_dsaf_mix_mode_cfg.u32, DSAF_CFG_0_REG);
}

/**
 * dsaf_local_addr_en_cfg - config dsaf_local_addr_en_cfg
 * @dsaf_id: dsa fabric id
 * @dsaf_local_addr_en_cfg: config value
 */
static void inline dsaf_local_addr_en_cfg(u32 dsaf_id, u32  dsaf_local_addr_en)
{
	union dsaf_cfg_reg o_dsaf_local_addr_en;

	o_dsaf_local_addr_en.u32 = dsaf_read(dsaf_id, DSAF_CFG_0_REG);
	o_dsaf_local_addr_en.bits.dsaf_local_addr_en = dsaf_local_addr_en;
	dsaf_write(dsaf_id, o_dsaf_local_addr_en.u32, DSAF_CFG_0_REG);
}

/**
 * dsaf_reg_cnt_clr_ce - config dsaf_reg_cnt_clr_ce
 * @dsaf_id: dsa fabric id
 * @dsaf_reg_cnt_clr_ce: config value
 */
static inline void dsaf_reg_cnt_clr_ce(u32 dsaf_id, u32 reg_cnt_clr_ce)
{
	union dsaf_fabric_reg_cnt_clr_ce o_reg_cnt_clr_ce;

	o_reg_cnt_clr_ce.u32 = dsaf_read(dsaf_id, DSAF_DSA_REG_CNT_CLR_CE_REG);
	o_reg_cnt_clr_ce.bits.cnt_clr_ce = reg_cnt_clr_ce;
	dsaf_write(dsaf_id, o_reg_cnt_clr_ce.u32, DSAF_DSA_REG_CNT_CLR_CE_REG);
}
/**
 * dsaf_pfc_unit_cnt - set pfc unit count
 * @dsaf_id: dsa fabric id
 * @pport_rate:  value array
 * @pdsaf_pfc_unit_cnt:  value array
 */
static void dsaf_pfc_unit_cnt(u32 dsaf_id, u32 port,
	enum dsaf_port_rate_mode rate)
{
	union dsaf_pfc_unit_cnt pfc_unit_cnt;
	u32 unit_cnt;

	switch (rate) {
	case DSAF_PORT_RATE_10000:
		unit_cnt = 41;
		break;
	case DSAF_PORT_RATE_1000:
		unit_cnt = 410;
		break;
	case DSAF_PORT_RATE_2500:
		unit_cnt = 103;
		break;
	default:
		unit_cnt = 41;
	}

	pfc_unit_cnt.u32
		= dsaf_read(dsaf_id, (DSAF_PFC_UNIT_CNT_0_REG + 0x4 * (u64)port));
	pfc_unit_cnt.bits.dsaf_pfc_unit_cnt = unit_cnt;
	dsaf_write(dsaf_id, pfc_unit_cnt.u32,
		DSAF_PFC_UNIT_CNT_0_REG + 0x4 * (u64)port);
}

/**
 * dsaf_ppe_qid_cfg - config ppe qid
 * @dsaf_id: dsa fabric id
 * @pppe_qid_cfg: value array
 */
static inline void dsaf_ppe_qid_cfg(u32 dsaf_id, u32 qid_cfg)
{
	union dsaf_ppe_qid_cfg ppe_qid_cfg;
	u32 i;

	for (i = 0; i < DSAF_PPE_TX_NUM; i++) {
		ppe_qid_cfg.u32 = dsaf_read(dsaf_id,
			DSAF_PPE_QID_CFG_0_REG + 0x0004 * (u64)i);
		ppe_qid_cfg.bits.dsaf_ppe_qid_cfg = qid_cfg;
		dsaf_write(dsaf_id, ppe_qid_cfg.u32,
			DSAF_PPE_QID_CFG_0_REG + 0x0004 * (u64)i);
	}
}

/**
 * dsaf_sw_port_type_cfg - cfg sw type
 * @dsaf_id: dsa fabric id
 * @psw_port_type: array
 */
static inline void dsaf_sw_port_type_cfg(u32 dsaf_id,
	enum dsaf_sw_port_type port_type)
{
	union dsaf_sw_port_type_reg sw_port_type;
	u32 i;

	for (i = 0; i < DSAF_SW_PORT_NUM; i++) {
		sw_port_type.u32 = dsaf_read(dsaf_id,
			DSAF_SW_PORT_TYPE_0_REG + 0x0004 * (u64)i);
		sw_port_type.bits.dsaf_sw_port_type = port_type;
		dsaf_write(dsaf_id, sw_port_type.u32,
			DSAF_SW_PORT_TYPE_0_REG + 0x0004 * (u64)i);
	}
}

/**
 * dsaf_stp_port_type_cfg - cfg stp type
 * @dsaf_id: dsa fabric id
 * @pstp_port_type: array
 */
static inline void dsaf_stp_port_type_cfg(u32 dsaf_id,
	enum dsaf_stp_port_type port_type)
{
	union dsaf_stp_port_type_reg stp_port_type;
	u32 i;

	for (i = 0; i < DSAF_XGE_CHN; i++) {
		stp_port_type.u32 = dsaf_read(dsaf_id,
			DSAF_STP_PORT_TYPE_0_REG + 0x0004 * (u64)i);
		stp_port_type.bits.dsaf_stp_port_type = port_type;
		dsaf_write(dsaf_id, stp_port_type.u32,
			DSAF_STP_PORT_TYPE_0_REG + 0x0004 * (u64)i);
	}
}

/**
 * dsaf_mix_def_qid_cfg - set def qid
 * @dsaf_id: dsa fabric id
 * @pmix_def_qid: array
 */
void dsaf_mix_def_qid_cfg(u32 dsaf_id, u32 port, u32 mix_def_qid)
{
	union dsaf_mix_def_qid mix_def_qid_reg;

	mix_def_qid_reg.u32
		= dsaf_read(dsaf_id, DSAF_MIX_DEF_QID_0_REG + 0x0004 * (u64)port);
	mix_def_qid_reg.bits.dsaf_mix_def_qid = mix_def_qid;
	dsaf_write(dsaf_id, mix_def_qid_reg.u32,
		DSAF_MIX_DEF_QID_0_REG + 0x0004 * (u64)port);
}

/**
 * dsaf_port_def_vlan_cfg - set def qid
 * @dsaf_id: dsa fabric id
 * @port: port ID
 * @pport_def_vlan: array
 */
void dsaf_port_def_vlan_cfg(u32 dsaf_id, u32 port, u16 def_vlan)
{
	union dsaf_port_def_vlan port_def_vlan;

	port_def_vlan.u32
		= dsaf_read(dsaf_id, DSAF_PORT_DEF_VLAN_0_REG + 0x0004 * (u64)port);
	port_def_vlan.bits.dsaf_port_def_vlan = def_vlan;
	dsaf_write(dsaf_id, port_def_vlan.u32,
		DSAF_PORT_DEF_VLAN_0_REG + 0x0004 * (u64)port);
}

/**
 * dsaf_port_def_vlan_get - get  def vlan
 * @dsaf_id: dsa fabric id
 * @port: port ID
 * @pport_def_vlan : array
 */
void dsaf_port_def_vlan_get(u32 dsaf_id, u32 port, u16 *def_vlan)
{
	union dsaf_port_def_vlan port_def_vlan;

	port_def_vlan.u32
		= dsaf_read(dsaf_id, DSAF_PORT_DEF_VLAN_0_REG + 0x0004 * (u64)port);
	*def_vlan = port_def_vlan.bits.dsaf_port_def_vlan;
}

/**
 * dsaf_inode_in_port_num - cut_through_cfg
 * @dsaf_id: dsa fabric id
 * @pinode_in_port_num: pointer
 */
static inline void dsaf_inode_in_port_num(u32 dsaf_id, u32 *pinode_in_port_num)
{
	union dsaf_inode_in_port_num o_inode_in_port_num;
	u32 i;

	for (i = 0; i < DSAF_INODE_NUM; i++) {
		o_inode_in_port_num.u32 = dsaf_read(dsaf_id,
			DSAF_INODE_IN_PORT_NUM_0_REG + 0x80 * (u64)i);
		o_inode_in_port_num.bits.inode_in_port_num =
			*pinode_in_port_num;
		dsaf_write(dsaf_id, o_inode_in_port_num.u32,
			DSAF_INODE_IN_PORT_NUM_0_REG + 0x80 * (u64)i);

		pinode_in_port_num++;
	}
}

/**
 * dsaf_inode_pri_tc_cfg - pri cf
 * @dsaf_id: dsa fabric id
 * @pinode_pri_tc_cfg: pointer
 */
static inline void dsaf_inode_pri_tc_cfg(u32 dsaf_id,
	struct dsaf_inode_pritc_cfg *pinode_pri_tc_cfg)
{
	union dsaf_inode_pri_tc_cfg o_inode_pri_tc_cfg;
	u32 i;

	for (i = 0; i < DSAF_INODE_NUM; i++) {
		o_inode_pri_tc_cfg.u32 = dsaf_read(dsaf_id,
			DSAF_INODE_PRI_TC_CFG_0_REG + 0x80 * (u64)i);
		o_inode_pri_tc_cfg.bits.inode_pri0_tc =
			pinode_pri_tc_cfg->inode_pri0_tc[i];
		o_inode_pri_tc_cfg.bits.inode_pri1_tc =
			pinode_pri_tc_cfg->inode_pri1_tc[i];
		o_inode_pri_tc_cfg.bits.inode_pri2_tc =
			pinode_pri_tc_cfg->inode_pri2_tc[i];
		o_inode_pri_tc_cfg.bits.inode_pri3_tc =
			pinode_pri_tc_cfg->inode_pri3_tc[i];
		o_inode_pri_tc_cfg.bits.inode_pri4_tc =
			pinode_pri_tc_cfg->inode_pri4_tc[i];
		o_inode_pri_tc_cfg.bits.inode_pri5_tc =
			pinode_pri_tc_cfg->inode_pri5_tc[i];
		o_inode_pri_tc_cfg.bits.inode_pri6_tc =
			pinode_pri_tc_cfg->inode_pri6_tc[i];
		o_inode_pri_tc_cfg.bits.inode_pri7_tc =
			pinode_pri_tc_cfg->inode_pri7_tc[i];
		o_inode_pri_tc_cfg.bits.inode_no_vlan_tag_man_tc =
			pinode_pri_tc_cfg->inode_no_vlan_tag_man_tc[i];
		o_inode_pri_tc_cfg.bits.inode_no_vlan_tag_data_tc =
			pinode_pri_tc_cfg->inode_no_vlan_tag_data_tc[i];
		dsaf_write(dsaf_id, o_inode_pri_tc_cfg.u32,
			DSAF_INODE_PRI_TC_CFG_0_REG + 0x80 * (u64)i);
	}
}

/**
 * dsaf_sbm_cfg - config sbm
 * @dsaf_id: dsa fabric id
 */
static void dsaf_sbm_cfg(u32 dsaf_id)
{
	union dsaf_sbm_cfg_reg o_sbm_cfg;
	u32 i;

	for (i = 0; i < DSAF_SBM_NUM; i++) {
		o_sbm_cfg.u32
			= dsaf_read(dsaf_id, DSAF_SBM_CFG_REG_0_REG + 0x80 * (u64)i);
		o_sbm_cfg.bits.sbm_cfg_en = 1;
		o_sbm_cfg.bits.sbm_cfg_shcut_en = 0;
		dsaf_write(dsaf_id, o_sbm_cfg.u32,
			DSAF_SBM_CFG_REG_0_REG + 0x80 * (u64)i);
	}
}

/**
 * dsaf_sbm_cfg_mib_en - config sbm
 * @dsaf_id: dsa fabric id
 */
static int dsaf_sbm_cfg_mib_en(u32 dsaf_id)
{
	union dsaf_sbm_cfg_reg o_sbm_cfg;
	u32 i;
	u32 read_cnt = 0;

	for (i = 0; i < DSAF_SBM_NUM; i++) {
		o_sbm_cfg.u32
			= dsaf_read(dsaf_id, DSAF_SBM_CFG_REG_0_REG + 0x80 * (u64)i);
		o_sbm_cfg.bits.sbm_cfg_mib_en = 1;
		dsaf_write(dsaf_id, o_sbm_cfg.u32,
			DSAF_SBM_CFG_REG_0_REG + 0x80 * (u64)i);
	}

	/* waitint for all sbm enable finished */
	for (i = 0; i < DSAF_SBM_NUM; i++) {
		read_cnt = 0;
		do {
			udelay(1);
			o_sbm_cfg.u32 = dsaf_read(dsaf_id,
				DSAF_SBM_CFG_REG_0_REG + 0x80 * (u64)i);
			read_cnt ++;
		} while (o_sbm_cfg.bits.sbm_cfg_mib_en == 0 &&
		read_cnt < DSAF_CFG_READ_CNT);

		if(o_sbm_cfg.bits.sbm_cfg_mib_en == 0) {
			pr_info("dsaf_sbm_cfg_mib_en fail,dsaf_id=%d,dsaf_sbm_num=%d\n",
				dsaf_id,i);
			return -ENOSYS;
		}
	}

	return 0;
}

/**
 * dsaf_sbm_bp_wl_cfg - config sbm
 * @dsaf_id: dsa fabric id
 */
static void dsaf_sbm_bp_wl_cfg(u32 dsaf_id)
{
	union dsaf_sbm_bp_cfg_0_reg o_sbm_bp_cfg0;
	union dsaf_sbm_bp_cfg_1_reg o_sbm_bp_cfg1;
	union dsaf_sbm_bp_cfg_2_reg o_sbm_bp_cfg2;
	union dsaf_sbm_bp_cfg_3_reg o_sbm_bp_cfg3;
	u32 i;

	/* XGE */
	for (i = 0; i < DSAF_XGE_NUM; i++) {
		o_sbm_bp_cfg0.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_0_XGE_REG_0_REG + 0x80 * (u64)i);
		o_sbm_bp_cfg0.bits.sbm_cfg_com_max_buf_num = 512;
		o_sbm_bp_cfg0.bits.sbm_cfg_vc0_max_buf_num = 0;
		o_sbm_bp_cfg0.bits.sbm_cfg_vc1_max_buf_num = 0;
		dsaf_write(dsaf_id, o_sbm_bp_cfg0.u32,
			DSAF_SBM_BP_CFG_0_XGE_REG_0_REG + 0x80 * (u64)i);

		o_sbm_bp_cfg1.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_1_REG_0_REG + 0x80 * (u64)i);
		o_sbm_bp_cfg1.bits.sbm_cfg_tc4_max_buf_num = 0;
		o_sbm_bp_cfg1.bits.sbm_cfg_tc0_max_buf_num = 0;
		dsaf_write(dsaf_id, o_sbm_bp_cfg1.u32,
			DSAF_SBM_BP_CFG_1_REG_0_REG + 0x80 * (u64)i);

		o_sbm_bp_cfg2.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_2_XGE_REG_0_REG + 0x80 * (u64)i);
		o_sbm_bp_cfg2.bits.sbm_cfg_set_buf_num = 104;
		o_sbm_bp_cfg2.bits.sbm_cfg_reset_buf_num = 128;
		dsaf_write(dsaf_id, o_sbm_bp_cfg2.u32,
			DSAF_SBM_BP_CFG_2_XGE_REG_0_REG + 0x80 * (u64)i);

		o_sbm_bp_cfg3.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_3_REG_0_REG + 0x80 * (u64)i);
		o_sbm_bp_cfg3.bits.sbm_cfg_set_buf_num_no_pfc = 110;
		o_sbm_bp_cfg3.bits.sbm_cfg_reset_buf_num_no_pfc = 160;
		dsaf_write(dsaf_id, o_sbm_bp_cfg3.u32,
			DSAF_SBM_BP_CFG_3_REG_0_REG + 0x80 * (u64)i);

		/* for no enable pfc mode */
		o_sbm_bp_cfg3.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_4_REG_0_REG + 0x80 * (u64)i);
		o_sbm_bp_cfg3.bits.sbm_cfg_set_buf_num_no_pfc = 128;
		o_sbm_bp_cfg3.bits.sbm_cfg_reset_buf_num_no_pfc = 192;
		dsaf_write(dsaf_id, o_sbm_bp_cfg3.u32,
			DSAF_SBM_BP_CFG_4_REG_0_REG + 0x80 * (u64)i);
	}

	/* PPE */
	for (i = 0; i < DSAF_SBM_NUM; i++) {
		o_sbm_bp_cfg2.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_2_PPE_REG_0_REG + 0x80 * (u64)i);
		o_sbm_bp_cfg2.bits.sbm_cfg_set_buf_num = 10;
		o_sbm_bp_cfg2.bits.sbm_cfg_reset_buf_num = 12;
		dsaf_write(dsaf_id, o_sbm_bp_cfg2.u32,
			DSAF_SBM_BP_CFG_2_PPE_REG_0_REG + 0x80 * (u64)i);
	}

	/* RoCEE */
	for (i = 0; i < DSAF_SBM_NUM; i++) {
		o_sbm_bp_cfg2.u32 = dsaf_read(dsaf_id,
			DSAF_SBM_BP_CFG_2_ROCEE_REG_0_REG + 0x80 * (u64)i);
		o_sbm_bp_cfg2.bits.sbm_cfg_set_buf_num = 2;
		o_sbm_bp_cfg2.bits.sbm_cfg_reset_buf_num = 4;
		dsaf_write(dsaf_id, o_sbm_bp_cfg2.u32,
			DSAF_SBM_BP_CFG_2_ROCEE_REG_0_REG + 0x80 * (u64)i);
	}
}

/**
 * dsaf_voq_bp_all_thrd_cfg -  voq
 * @dsaf_id: dsa fabric id
 */
static void dsaf_voq_bp_all_thrd_cfg(u32 dsaf_id)
{
	union dsaf_voq_bp_all_thrd1 voq_bp_all_thrd;
	u32 i;

	for (i = 0; i < DSAF_VOQ_NUM; i++) {
		voq_bp_all_thrd.u32
			= dsaf_read(dsaf_id, DSAF_VOQ_BP_ALL_THRD_0_REG + 0x40 * (u64)i);
		if (i < DSAF_XGE_NUM) {
			voq_bp_all_thrd.bits.voq_bp_all_downthrd = 930;
			voq_bp_all_thrd.bits.voq_bp_all_upthrd = 950;
		} else {
			voq_bp_all_thrd.bits.voq_bp_all_downthrd = 220;
			voq_bp_all_thrd.bits.voq_bp_all_upthrd = 230;
		}
		dsaf_write(dsaf_id, voq_bp_all_thrd.u32,
			DSAF_VOQ_BP_ALL_THRD_0_REG + 0x40 * (u64)i);
	}
}

/**
 * dsaf_tbl_tcam_data_cfg - tbl
 * @dsaf_id: dsa fabric id
 * @ptbl_tcam_data: addr
 */
static inline void dsaf_tbl_tcam_data_cfg(u32 dsaf_id,
	struct dsaf_tbl_tcam_data *ptbl_tcam_data)
{
	/*u32 o_tbl_tcam_data_h;
	u32 o_tbl_tcam_data_l;*/

	/*o_tbl_tcam_data_l = ptbl_tcam_data->tbl_tcam_data_low;*/
	dsaf_write(dsaf_id, ptbl_tcam_data->tbl_tcam_data_low,
		DSAF_TBL_TCAM_LOW_0_REG);

	/* TBD why read? */
	/*o_tbl_tcam_data_l = dsaf_read(dsaf_id, DSAF_TBL_TCAM_LOW_0_REG);*/


	/*o_tbl_tcam_data_h = ptbl_tcam_data->tbl_tcam_data_high;*/
	dsaf_write(dsaf_id, ptbl_tcam_data->tbl_tcam_data_high,
		DSAF_TBL_TCAM_HIGH_0_REG);

	/*o_tbl_tcam_data_h = dsaf_read(dsaf_id, DSAF_TBL_TCAM_HIGH_0_REG);*/
}

/**
 * dsaf_tbl_tcam_mcast_cfg - tbl
 * @dsaf_id: dsa fabric id
 * @ptbl_tcam_mcast: addr
 */
static int dsaf_tbl_tcam_mcast_cfg(u32 dsaf_id,
	struct dsaf_tbl_tcam_mcast_cfg *ptbl_tcam_mcast)
{
	union dsaf_tbl_tcam_mcast_cfg_4 o_tbl_tcam_mcast_cfg4;
	u32 o_tbl_tcam_mcast_cfg;
	union dsaf_tbl_tcam_mcast_cfg_0 o_tbl_tcam_mcast_cfg0;

	o_tbl_tcam_mcast_cfg4.u32
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_MCAST_CFG_4_0_REG);
	o_tbl_tcam_mcast_cfg4.bits.tbl_mcast_item_vld =
		ptbl_tcam_mcast->tbl_mcast_item_vld;
	o_tbl_tcam_mcast_cfg4.bits.tbl_mcast_old_en =
		ptbl_tcam_mcast->tbl_mcast_old_en;
	o_tbl_tcam_mcast_cfg4.bits.tbl_mcast_vm128_122 =
		(ptbl_tcam_mcast->tbl_mcast_vm128_120 & 0x1fc) >> 2;
	dsaf_write(dsaf_id, o_tbl_tcam_mcast_cfg4.u32,
		DSAF_TBL_TCAM_MCAST_CFG_4_0_REG);

	o_tbl_tcam_mcast_cfg =
			(ptbl_tcam_mcast->tbl_mcast_vm99_90 & 0x3ff) |
			((ptbl_tcam_mcast->tbl_mcast_vm109_100 & 0x3ff) << 10) |
			((ptbl_tcam_mcast->tbl_mcast_vm119_110 & 0x3ff) << 20) |
			((ptbl_tcam_mcast->tbl_mcast_vm128_120 & 0x3) << 30);
	dsaf_write(dsaf_id, o_tbl_tcam_mcast_cfg,
		DSAF_TBL_TCAM_MCAST_CFG_3_0_REG);

	o_tbl_tcam_mcast_cfg =
			((ptbl_tcam_mcast->tbl_mcast_vm59_50 & 0x300) >> 8) |
			((ptbl_tcam_mcast->tbl_mcast_vm69_60 & 0x3ff) << 2) |
			((ptbl_tcam_mcast->tbl_mcast_vm79_70 & 0x3ff) << 12) |
			((ptbl_tcam_mcast->tbl_mcast_vm89_80 & 0x3ff) << 22);
	dsaf_write(dsaf_id, o_tbl_tcam_mcast_cfg,
		DSAF_TBL_TCAM_MCAST_CFG_2_0_REG);

	o_tbl_tcam_mcast_cfg =
			((ptbl_tcam_mcast->tbl_mcast_vm29_20 & 0x3c0) >> 6) |
			((ptbl_tcam_mcast->tbl_mcast_vm39_30 & 0x3ff) << 4) |
			((ptbl_tcam_mcast->tbl_mcast_vm49_40 & 0x3ff) << 14) |
			((ptbl_tcam_mcast->tbl_mcast_vm59_50 & 0xff) << 24);
	dsaf_write(dsaf_id, o_tbl_tcam_mcast_cfg,
		DSAF_TBL_TCAM_MCAST_CFG_1_0_REG);

	o_tbl_tcam_mcast_cfg0.u32 = dsaf_read(dsaf_id,
		DSAF_TBL_TCAM_MCAST_CFG_0_0_REG);
	o_tbl_tcam_mcast_cfg0.bits.tbl_mcast_vm25_0 =
			((ptbl_tcam_mcast->tbl_mcast_vm29_20 & 0x3f) << 20) |
			((ptbl_tcam_mcast->tbl_mcast_vm19_10 & 0x3ff) << 10) |
			(ptbl_tcam_mcast->tbl_mcast_vm9_0 & 0x3ff);
	o_tbl_tcam_mcast_cfg0.bits.tbl_mcast_xge5_0 =
		ptbl_tcam_mcast->tbl_mcast_xge5_0;
	dsaf_write(dsaf_id, o_tbl_tcam_mcast_cfg0.u32,
		DSAF_TBL_TCAM_MCAST_CFG_0_0_REG);

	return 0;
}

/**
 * dsaf_tbl_tcam_ucast_cfg - tbl
 * @dsaf_id: dsa fabric id
 * @ptbl_tcam_ucast: addr
 */
static inline void dsaf_tbl_tcam_ucast_cfg(
	u32 dsaf_id, struct dsaf_tbl_tcam_ucast_cfg *tbl_tcam_ucast)
{
	union dsaf_tbl_tcam_ucast_cfg1 tcam_ucast;

	tcam_ucast.u32 = dsaf_read(dsaf_id, DSAF_TBL_TCAM_UCAST_CFG_0_REG);
	tcam_ucast.bits.tbl_ucast_mac_discard
		= tbl_tcam_ucast->tbl_ucast_mac_discard;
	tcam_ucast.bits.tbl_ucast_item_vld
		= tbl_tcam_ucast->tbl_ucast_item_vld;
	tcam_ucast.bits.tbl_ucast_old_en
		= tbl_tcam_ucast->tbl_ucast_old_en;
	tcam_ucast.bits.tbl_ucast_dvc
		= tbl_tcam_ucast->tbl_ucast_dvc;
	tcam_ucast.bits.tbl_ucast_out_port
		= tbl_tcam_ucast->tbl_ucast_out_port;
	dsaf_write(dsaf_id, tcam_ucast.u32, DSAF_TBL_TCAM_UCAST_CFG_0_REG);
}

/**
 * dsaf_tbl_line_cfg - tbl
 * @dsaf_id: dsa fabric id
 * @ptbl_lin: addr
 */
static inline void dsaf_tbl_line_cfg(u32 dsaf_id,
	struct dsaf_tbl_line_cfg *tbl_lin)
{
	union dsaf_tbl_lin_cfg o_tbl_line;

	o_tbl_line.u32 = dsaf_read(dsaf_id, DSAF_TBL_LIN_CFG_0_REG);
	o_tbl_line.bits.tbl_line_mac_discard = tbl_lin->tbl_line_mac_discard;
	o_tbl_line.bits.tbl_line_out_port =	tbl_lin->tbl_line_out_port;
	o_tbl_line.bits.tbl_line_dvc = tbl_lin->tbl_line_dvc;
	dsaf_write(dsaf_id, o_tbl_line.u32, DSAF_TBL_LIN_CFG_0_REG);
}

/**
 * dsaf_tbl_tcam_mcast_pul - tbl
 * @dsaf_id: dsa fabric id
 */
static inline void dsaf_tbl_tcam_mcast_pul(u32 dsaf_id)
{
	union dsaf_tbl_pul o_tbl_pul;

	o_tbl_pul.u32 = dsaf_read(dsaf_id, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_mcast_vld = 1;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_mcast_vld = 0;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);
}

/**
 * dsaf_tbl_line_pul - tbl
 * @dsaf_id: dsa fabric id
 */
static inline void dsaf_tbl_line_pul(u32 dsaf_id)
{
	union dsaf_tbl_pul o_tbl_pul;

	o_tbl_pul.u32 = dsaf_read(dsaf_id, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_line_vld = 1;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_line_vld = 0;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);
}

/**
 * dsaf_tbl_tcam_data_mcast_pul - tbl
 * @dsaf_id: dsa fabric id
 */
static inline void dsaf_tbl_tcam_data_mcast_pul(u32 dsaf_id)
{
	union dsaf_tbl_pul o_tbl_pul;

	o_tbl_pul.u32 = dsaf_read(dsaf_id, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_tcam_data_vld = 1;
	o_tbl_pul.bits.tbl_mcast_vld = 1;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_tcam_data_vld = 0;
	o_tbl_pul.bits.tbl_mcast_vld = 0;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);
}

/**
 * dsaf_tbl_tcam_data_ucast_pul - tbl
 * @dsaf_id: dsa fabric id
 */
static inline void dsaf_tbl_tcam_data_ucast_pul(u32 dsaf_id)
{
	union dsaf_tbl_pul o_tbl_pul;

	o_tbl_pul.u32 = dsaf_read(dsaf_id, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_tcam_data_vld = 1;
	o_tbl_pul.bits.tbl_ucast_vld = 1;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_tcam_data_vld = 0;
	o_tbl_pul.bits.tbl_ucast_vld = 0;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);
}

/**
 * dsaf_tbl_stat_en - tbl
 * @dsaf_id: dsa fabric id
 * @ptbl_stat_en: addr
 */
static inline void dsaf_tbl_stat_en(u32 dsaf_id)
{
	union dsaf_tbl_dfx_ctrl o_tbl_ctrl;

	o_tbl_ctrl.u32 = dsaf_read(dsaf_id, DSAF_TBL_DFX_CTRL_0_REG);
	o_tbl_ctrl.bits.line_lkup_num_en = 1;
	o_tbl_ctrl.bits.uc_lkup_num_en = 1;
	o_tbl_ctrl.bits.mc_lkup_num_en = 1;
	o_tbl_ctrl.bits.bc_lkup_num_en = 1;
	dsaf_write(dsaf_id, o_tbl_ctrl.u32, DSAF_TBL_DFX_CTRL_0_REG);
}

/**
 * dsaf_port_work_rate_cfg - fifo
 * @dsaf_id: dsa fabric id
 * @xge_ge_work_mode
 */
void dsaf_port_work_rate_cfg(u32 dsaf_id, u32 port,
	enum dsaf_port_rate_mode rate_mode)
{
	union dsaf_xge_ge_work_mode port_work_mode;

	port_work_mode.u32 = dsaf_read(dsaf_id,
		DSAF_XGE_GE_WORK_MODE_0_REG + 0x4 * (u64)port);

	if (DSAF_PORT_RATE_10000 == rate_mode)
		port_work_mode.bits.xge_ge_work_mode = 1;
	else
		port_work_mode.bits.xge_ge_work_mode = 0;

	dsaf_write(dsaf_id, port_work_mode.u32,
		DSAF_XGE_GE_WORK_MODE_0_REG + 0x4 * (u64)port);

	pr_info("dsaf_id=%d port=%d rate_mode=%d\n", dsaf_id, port, rate_mode);

	dsaf_pfc_unit_cnt(dsaf_id, port, rate_mode);
}

/**
 * dsaf_ge_xge_loopback_cfg - fifo
 * @dsaf_id: dsa fabric id
 * @port_num
 * @xge_ge_loopback
 */
void dsaf_xge_loopback_cfg(u32 dsaf_id, u32 port, u32 xge_ge_loopback)
{
	union dsaf_xge_ge_work_mode loopback;

	loopback.u32 = dsaf_read(dsaf_id, DSAF_XGE_GE_WORK_MODE_0_REG + 0x4 * (u64)port);
	loopback.bits.xge_ge_loopback =	xge_ge_loopback;
	dsaf_write(dsaf_id, loopback.u32, DSAF_XGE_GE_WORK_MODE_0_REG + 0x4 * (u64)port);
}

/**
 * dsaf_rocee_bp_en - rocee back press enable
 * @dsaf_id: dsa fabric id
 */
static inline void dsaf_rocee_bp_en(u32 dsaf_id)
{
	union dsaf_xge_ctrl_sig_cfg xge_ctrl_sig_cfg;

	xge_ctrl_sig_cfg.u32 = dsaf_read(dsaf_id, DSAF_XGE_CTRL_SIG_CFG_0_REG);
	xge_ctrl_sig_cfg.bits.fc_xge_tx_pause = 1;
	dsaf_write(dsaf_id, xge_ctrl_sig_cfg.u32, DSAF_XGE_CTRL_SIG_CFG_0_REG);
}



/**
 * dsaf_int_xge_msk_get - INT
 * @dsaf_id: dsa fabric id
 * @chnn_num
 * @pint_msk
 */
static inline void dsaf_int_xge_msk_get(u32 dsaf_id, u32 chnn_num,
	union dsaf_xge_int_msk *int_msk)
{
	int_msk->u32
		= dsaf_read(dsaf_id, DSAF_XGE_INT_MSK_0_REG + 0x4 * (u64)chnn_num);
}

/**
 * dsaf_int_ppe_msk_get - INT
 * @dsaf_id: dsa fabric id
 * @chnn_num
 * @pint_msk
 */
static inline void dsaf_int_ppe_msk_get(u32 dsaf_id, u32 chnn_num,
	union dsaf_ppe_int_msk *int_msk)
{
	int_msk->u32
		= dsaf_read(dsaf_id, DSAF_PPE_INT_MSK_0_REG + 0x4 * (u64)chnn_num);
}

static inline void dsaf_int_rocee_msk_get(u32 dsaf_id, u32 chnn,
	union dsaf_rocee_int_msk *int_msk)
{
	int_msk->u32 = dsaf_read(dsaf_id, DSAF_ROCEE_INT_MSK_0_REG + 0x4*(u64)chnn);
}

void dsaf_int_tbl_msk_get(u32 dsaf_id, union dsaf_tbl_int_msk1 *int_msk)
{
	int_msk->u32 = dsaf_read(dsaf_id, DSAF_TBL_INT_MSK_0_REG);
}

static inline void dsaf_int_xge_sts_get(u32 dsaf_id, u32 chnn_num,
	union dsaf_xge_int_sts *int_sts)
{
	int_sts->u32
		= dsaf_read(dsaf_id, DSAF_XGE_INT_STS_0_REG + 0x4 * (u64)chnn_num);
}

static inline void dsaf_int_ppe_sts_get(u32 dsaf_id, u32 chnn_num,
	union dsaf_ppe_int_sts *int_sts)
{
	int_sts->u32 = dsaf_read(dsaf_id,
		DSAF_PPE_INT_STS_0_REG + 0x4 * (u64)chnn_num);
}

static inline void dsaf_int_rocee_sts_get(u32 dsaf_id, u32 chnn,
	union dsaf_rocee_int_sts *int_sts)
{
	int_sts->u32 = dsaf_read(dsaf_id, DSAF_ROCEE_INT_STS_0_REG + 0x4*(u64)chnn);
}

static inline void dsaf_int_tbl_sts_get(u32 dsaf_id,
	union dsaf_tbl_int_sts *int_sts)
{
	int_sts->u32 = dsaf_read(dsaf_id, DSAF_TBL_INT_STS_0_REG);
}

void dsaf_int_xge_msk_cfg(u32 dsaf_id, u32 chnn_num,
	union dsaf_xge_int_msk int_msk)
{
	dsaf_write(dsaf_id, int_msk.u32,
		DSAF_XGE_INT_MSK_0_REG + 0x4 * (u64)chnn_num);
}

static inline void dsaf_int_xge_msk_set(u32 dsaf_id, u32 chnn_num, u32 mask_set)
{
	dsaf_write(dsaf_id, mask_set, DSAF_XGE_INT_MSK_0_REG + 0x4*(u64)chnn_num);
}

static inline void dsaf_int_ppe_msk_cfg(u32 dsaf_id, u32 chnn_num,
	union dsaf_ppe_int_msk int_msk)
{
	dsaf_write(dsaf_id, int_msk.u32, DSAF_PPE_INT_MSK_0_REG + 0x4*(u64)chnn_num);
}

static inline void dsaf_int_ppe_msk_set(u32 dsaf_id, u32 chnn_num, u32 msk_set)
{
	dsaf_write(dsaf_id, msk_set, DSAF_PPE_INT_MSK_0_REG + 0x4*(u64)chnn_num);
}

static inline void dsaf_int_rocee_msk_cfg(u32 dsaf_id, u32 chnn,
	union dsaf_rocee_int_msk int_msk)
{
	dsaf_write(dsaf_id, int_msk.u32, DSAF_ROCEE_INT_MSK_0_REG + 0x4 * (u64)chnn);
}

static inline void dsaf_int_rocee_msk_set(u32 dsaf_id, u32 chnn,
	u32 msk_set)
{
	dsaf_write(dsaf_id, msk_set, DSAF_ROCEE_INT_MSK_0_REG + 0x4 * (u64)chnn);
}

void dsaf_int_tbl_msk_cfg(u32 dsaf_id, union dsaf_tbl_int_msk1 int_msk)
{
	dsaf_write(dsaf_id, int_msk.u32, DSAF_TBL_INT_MSK_0_REG);
}

static inline void dsaf_int_tbl_msk_set(u32 dsaf_id, u32 msk_set)
{
	dsaf_write(dsaf_id, msk_set, DSAF_TBL_INT_MSK_0_REG);
}

static inline void dsaf_int_xge_src_clr(u32 dsaf_id, u32 chnn_num,
	union dsaf_xge_int_src int_src)
{
	dsaf_write(dsaf_id, int_src.u32,
		DSAF_XGE_INT_SRC_0_REG + 0x4 * (u64)chnn_num);
}

static inline void dsaf_int_ppe_src_clr(u32 dsaf_id, u32 chnn,
	union dsaf_ppe_int_src int_src)
{
	dsaf_write(dsaf_id, int_src.u32, DSAF_PPE_INT_SRC_0_REG + 0x4 * (u64)chnn);
}

static inline void dsaf_int_rocee_src_clr(u32 dsaf_id, u32 chnn,
	union dsaf_rocee_int_src int_src)
{
	dsaf_write(dsaf_id, int_src.u32, DSAF_ROCEE_INT_SRC_0_REG + 0x4 * (u64)chnn);
}

static inline void dsaf_int_tbl_src_clr(u32 dsaf_id,
	union dsaf_tbl_int_src int_src)
{
	dsaf_write(dsaf_id, int_src.u32, DSAF_TBL_INT_SRC_0_REG);
}

/**
 * dsaf_int_stat_init - INT
 * void
 */
static void dsaf_int_stat_init(void)
{
	memset(g_dsaf_int_xge_stat, 0, sizeof(g_dsaf_int_xge_stat));
	memset(g_dsaf_int_ppe_stat, 0, sizeof(g_dsaf_int_ppe_stat));
	memset(g_dsaf_int_rocee_stat, 0, sizeof(g_dsaf_int_rocee_stat));
	memset(g_dsaf_int_tbl_stat, 0, sizeof(g_dsaf_int_tbl_stat));
}

/**
 * dsaf_xge_int_handler - INT
 * @dsaf_id: dsa fabric id
 * @dsaf_xge_chn
 */
void dsaf_xge_int_handler(u32 dsaf_id, u32 dsaf_xge_chn)
{
	union dsaf_xge_int_src int_src;
	union dsaf_xge_int_sts int_sts;
	union dsaf_xge_int_msk int_msk;
	struct dsaf_int_xge_src *p;
	u32 dsaf_int_cnt = 0;

	p = &g_dsaf_int_xge_stat[dsaf_xge_chn];

	dsaf_int_xge_sts_get(dsaf_id, dsaf_xge_chn, &int_sts);
	osal_printf("dsaf xge int,int_sts=%#x\n", int_sts.u32);

	dsaf_int_xge_msk_get(dsaf_id, dsaf_xge_chn, &int_msk);

	dsaf_int_xge_msk_set(dsaf_id, dsaf_xge_chn, 0xFFFFFFFF);

	int_src.u32 = 0;

	if (1 == int_sts.bits.xid_xge_ecc_err_int_sts) {
		int_src.bits.xid_xge_ecc_err_int_src = 1;
		p->xid_xge_ecc_err_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.xid_xge_fsm_timeout_int_sts) {
		int_src.bits.xid_xge_fsm_timeout_int_src = 1;
		p->xid_xge_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_xge_lnk_fsm_timout_int_sts) {
		int_src.bits.sbm_xge_lnk_fsm_timout_int_src = 1;
		p->sbm_xge_lnk_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_xge_lnk_ecc_2bit_int_sts) {
		int_src.bits.sbm_xge_lnk_ecc_2bit_int_src = 1;
		p->sbm_xge_lnk_ecc_2bit_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_xge_mib_req_failed_int_sts) {
		int_src.bits.sbm_xge_mib_req_failed_int_src = 1;
		p->sbm_xge_mib_req_failed_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_xge_mib_req_fsm_timout_int_sts) {
		int_src.bits.sbm_xge_mib_req_fsm_timout_int_src = 1;
		p->sbm_xge_mib_req_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_xge_mib_rels_fsm_timout_int_sts) {
		int_src.bits.sbm_xge_mib_rels_fsm_timout_int_src = 1;
		p->sbm_xge_mib_rels_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_xge_sram_ecc_2bit_int_sts) {
		int_src.bits.sbm_xge_sram_ecc_2bit_int_src = 1;
		p->sbm_xge_sram_ecc_2bit_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_xge_mib_buf_sum_err_int_sts) {
		int_src.bits.sbm_xge_mib_buf_sum_err_int_src = 1;
		p->sbm_xge_mib_buf_sum_err_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_xge_mib_req_extra_int_sts) {
		int_src.bits.sbm_xge_mib_req_extra_int_src = 1;
		p->sbm_xge_mib_req_extra_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_xge_mib_rels_extra_int_sts) {
		int_src.bits.sbm_xge_mib_rels_extra_int_src = 1;
		p->sbm_xge_mib_rels_extra_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.voq_xge_start_to_over_0_int_sts) {
		int_src.bits.voq_xge_start_to_over_0_int_src = 1;
		p->voq_xge_start_to_over_0_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.voq_xge_start_to_over_1_int_sts) {
		int_src.bits.voq_xge_start_to_over_1_int_src = 1;
		p->voq_xge_start_to_over_1_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.voq_xge_ecc_err_int_sts) {
		int_src.bits.voq_xge_ecc_err_int_src = 1;
		p->voq_xge_ecc_err_int_src++;
		dsaf_int_cnt++;
	}

	if (dsaf_int_cnt)
		dsaf_int_xge_src_clr(dsaf_id, dsaf_xge_chn, int_src);

	dsaf_int_xge_msk_cfg(dsaf_id, dsaf_xge_chn, int_msk);
}

/**
 * dsaf_ppe_int_handler - INT
 * @dsaf_id: dsa fabric id
 * @dsaf_ppe_chn
 */
void dsaf_ppe_int_handler(u32 dsaf_id, u32 dsaf_ppe_chn)
{
	union dsaf_ppe_int_sts int_sts;
	union dsaf_ppe_int_src int_src;
	union dsaf_ppe_int_msk int_msk;
	struct dsaf_int_ppe_src *p;
	u32 dsaf_int_cnt = 0;

	p = &g_dsaf_int_ppe_stat[dsaf_ppe_chn];
	dsaf_int_ppe_sts_get(dsaf_id, dsaf_ppe_chn, &int_sts);
	osal_printf("dsaf ppe int,int_sts=%#x\n", int_sts.u32);

	dsaf_int_ppe_msk_get(dsaf_id, dsaf_ppe_chn, &int_msk);

	dsaf_int_ppe_msk_set(dsaf_id, dsaf_ppe_chn, 0xFFFFFFFF);

	int_src.u32 = 0;

	if (1 == int_sts.bits.xid_ppe_fsm_timeout_int_sts) {
		int_src.bits.xid_ppe_fsm_timeout_int_src = 1;
		p->xid_ppe_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_ppe_lnk_ecc_2bit_int_sts) {
		int_src.bits.sbm_ppe_lnk_ecc_2bit_int_src = 1;
		p->sbm_ppe_lnk_ecc_2bit_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_ppe_lnk_fsm_timout_int_sts) {
		int_src.bits.sbm_ppe_lnk_fsm_timout_int_src = 1;
		p->sbm_ppe_lnk_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_ppe_mib_rels_fsm_timout_int_sts) {
		int_src.bits.sbm_ppe_mib_rels_fsm_timout_int_src = 1;
		p->sbm_ppe_mib_rels_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_ppe_mib_req_failed_int_sts) {
		int_src.bits.sbm_ppe_mib_req_failed_int_src = 1;
		p->sbm_ppe_mib_req_failed_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_ppe_mib_req_fsm_timout_int_sts) {
		int_src.bits.sbm_ppe_mib_req_fsm_timout_int_src = 1;
		p->sbm_ppe_mib_req_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_ppe_sram_ecc_2bit_int_sts) {
		int_src.bits.sbm_ppe_sram_ecc_2bit_int_src = 1;
		p->sbm_ppe_sram_ecc_2bit_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.voq_ppe_ecc_err_int_sts) {
		int_src.bits.voq_ppe_ecc_err_int_src  = 1;
		p->voq_ppe_ecc_err_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_ppe_mib_buf_sum_err_int_sts) {
		int_src.bits.sbm_ppe_mib_buf_sum_err_int_src  = 1;
		p->sbm_ppe_mib_buf_sum_err_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_ppe_mib_req_extra_int_sts) {
		int_src.bits.sbm_ppe_mib_req_extra_int_src	= 1;
		p->sbm_ppe_mib_req_extra_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.sbm_ppe_mib_rels_extra_int_sts) {
		int_src.bits.sbm_ppe_mib_rels_extra_int_src  = 1;
		p->sbm_ppe_mib_rels_extra_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.voq_ppe_start_to_over_0_int_sts) {
		int_src.bits.voq_ppe_start_to_over_0_int_src  = 1;
		p->voq_ppe_start_to_over_0_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.xod_ppe_fifo_rd_empty_int_sts) {
		int_src.bits.xod_ppe_fifo_rd_empty_int_src	= 1;
		p->xod_ppe_fifo_rd_empty_int_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.xod_ppe_fifo_wr_full_int_sts) {
		int_src.bits.xod_ppe_fifo_wr_full_int_src  = 1;
		p->xod_ppe_fifo_wr_full_int_src++;
		dsaf_int_cnt++;
	}

	if (dsaf_int_cnt)
		dsaf_int_ppe_src_clr(dsaf_id, dsaf_ppe_chn, int_src);

	dsaf_int_ppe_msk_cfg(dsaf_id, dsaf_ppe_chn, int_msk);
}

/**
 * dsaf_rocee_int_handler - INT
 * @dsaf_id: dsa fabric id
 * @dsaf_rocee_chn
 */
void dsaf_rocee_int_handler(u32 dsaf_id, u32 dsaf_rocee_chn)
{
	union dsaf_rocee_int_src int_src;
	union dsaf_rocee_int_sts int_sts;
	union dsaf_rocee_int_msk int_msk;
	struct dsaf_int_rocee_src *p;
	u32 dsaf_int_cnt = 0;

	p = &g_dsaf_int_rocee_stat[dsaf_rocee_chn];
	dsaf_int_rocee_sts_get(dsaf_id, dsaf_rocee_chn, &int_sts);
	osal_printf("rocee ppe int,int_sts=%#x\n", int_sts.u32);

	dsaf_int_rocee_msk_get(dsaf_id, dsaf_rocee_chn, &int_msk);

	dsaf_int_rocee_msk_set(dsaf_id, dsaf_rocee_chn, 0xFFFFFFFF);

	int_src.u32 = 0;

	if (int_sts.bits.xid_rocee_fsm_timeout_int_sts) {
		int_src.bits.xid_rocee_fsm_timeout_int_src  = 1;
		p->xid_rocee_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.sbm_rocee_lnk_ecc_2bit_int_sts) {
		int_src.bits.sbm_rocee_lnk_ecc_2bit_int_src  = 1;
		p->sbm_rocee_lnk_ecc_2bit_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.sbm_rocee_lnk_fsm_timout_int_sts) {
		int_src.bits.sbm_rocee_lnk_fsm_timout_int_src  = 1;
		p->sbm_rocee_lnk_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.sbm_rocee_mib_rels_fsm_timout_int_sts) {
		int_src.bits.sbm_rocee_mib_rels_fsm_timout_int_src	= 1;
		p->sbm_rocee_mib_rels_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.sbm_rocee_mib_req_failed_int_sts) {
		int_src.bits.sbm_rocee_mib_req_failed_int_src  = 1;
		p->sbm_rocee_mib_req_failed_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.sbm_rocee_mib_req_fsm_timout_int_sts) {
		int_src.bits.sbm_rocee_mib_req_fsm_timout_int_src  = 1;
		p->sbm_rocee_mib_req_fsm_timout_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.sbm_rocee_sram_ecc_2bit_int_sts) {
		int_src.bits.sbm_rocee_sram_ecc_2bit_int_src  = 1;
		p->sbm_rocee_sram_ecc_2bit_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.voq_rocee_ecc_err_int_sts) {
		int_src.bits.voq_rocee_ecc_err_int_src	= 1;
		p->voq_rocee_ecc_err_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.voq_rocee_start_to_over_0_int_sts) {
		int_src.bits.voq_rocee_start_to_over_0_int_src	= 1;
		p->voq_rocee_start_to_over_0_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.sbm_rocee_mib_buf_sum_err_int_sts) {
		int_src.bits.sbm_rocee_mib_buf_sum_err_int_src	= 1;
		p->sbm_rocee_mib_buf_sum_err_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.sbm_rocee_mib_req_extra_int_sts) {
		int_src.bits.sbm_rocee_mib_req_extra_int_src  = 1;
		p->sbm_rocee_mib_req_extra_int_src++;
		dsaf_int_cnt++;
	}
	if (int_sts.bits.sbm_rocee_mib_rels_extra_int_sts) {
		int_src.bits.sbm_rocee_mib_rels_extra_int_src  = 1;
		p->sbm_rocee_mib_rels_extra_int_src++;
		dsaf_int_cnt++;
	}

	if (dsaf_int_cnt)
		dsaf_int_rocee_src_clr(dsaf_id, dsaf_rocee_chn, int_src);

	dsaf_int_rocee_msk_cfg(dsaf_id, dsaf_rocee_chn, int_msk);
}

/**
 * dsaf_tbl_damis_int_handler - INT
 * @dsaf_id: dsa fabric id
 */
void dsaf_tbl_damis_int_handler(u32 dsaf_id)
{
	union dsaf_tbl_int_src int_src;
	union dsaf_tbl_int_sts int_sts;
	union dsaf_tbl_int_msk1 int_msk;
	struct dsaf_int_tbl_src *p;
	u32 dsaf_int_cnt = 0;

	p = &g_dsaf_int_tbl_stat[0];
	dsaf_int_tbl_sts_get(dsaf_id, &int_sts);
	osal_printf("dsaf tbl damis int,int_sts=%#x\n", int_sts.u32);

	dsaf_int_tbl_msk_get(dsaf_id, &int_msk);

	dsaf_int_tbl_msk_set(dsaf_id, 0xFFFFFFFF);

	int_src.u32 = 0;

	if (1 == int_sts.bits.tbl_da0_mis_sts) {
		int_src.bits.tbl_da0_mis_src = 1;
		p->tbl_da0_mis_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_da1_mis_sts) {
		int_src.bits.tbl_da1_mis_src = 1;
		p->tbl_da1_mis_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_da2_mis_sts) {
		int_src.bits.tbl_da2_mis_src = 1;
		p->tbl_da2_mis_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_da3_mis_sts) {
		int_src.bits.tbl_da3_mis_src = 1;
		p->tbl_da3_mis_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_da4_mis_sts) {
		int_src.bits.tbl_da4_mis_src = 1;
		p->tbl_da4_mis_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_da5_mis_sts) {
		int_src.bits.tbl_da5_mis_src = 1;
		p->tbl_da5_mis_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_da6_mis_sts) {
		int_src.bits.tbl_da6_mis_src = 1;
		p->tbl_da6_mis_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_da7_mis_sts) {
		int_src.bits.tbl_da7_mis_src = 1;
		p->tbl_da7_mis_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_ucast_bcast_xge0_sts) {
		int_src.bits.tbl_ucast_bcast_xge0_src = 1;
		p->tbl_ucast_bcast_xge0_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_ucast_bcast_xge1_sts) {
		int_src.bits.tbl_ucast_bcast_xge1_src = 1;
		p->tbl_ucast_bcast_xge1_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_ucast_bcast_xge2_sts) {
		int_src.bits.tbl_ucast_bcast_xge2_src = 1;
		p->tbl_ucast_bcast_xge2_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_ucast_bcast_xge3_sts) {
		int_src.bits.tbl_ucast_bcast_xge3_src = 1;
		p->tbl_ucast_bcast_xge3_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_ucast_bcast_xge4_sts) {
		int_src.bits.tbl_ucast_bcast_xge4_src = 1;
		p->tbl_ucast_bcast_xge4_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_ucast_bcast_xge5_sts) {
		int_src.bits.tbl_ucast_bcast_xge5_src = 1;
		p->tbl_ucast_bcast_xge5_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_ucast_bcast_ppe_sts) {
		int_src.bits.tbl_ucast_bcast_ppe_src = 1;
		p->tbl_ucast_bcast_ppe_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_ucast_bcast_rocee_sts) {
		int_src.bits.tbl_ucast_bcast_rocee_src = 1;
		p->tbl_ucast_bcast_rocee_src++;
		dsaf_int_cnt++;
	}

	if (dsaf_int_cnt)
		dsaf_int_tbl_src_clr(dsaf_id, int_src);

	dsaf_int_tbl_msk_cfg(dsaf_id, int_msk);
}

/**
 * dsaf_tbl_samis_int_handler - INT
 * @dsaf_id: dsa fabric id
 */
void dsaf_tbl_samis_int_handler(u32 dsaf_id)
{
	union dsaf_tbl_int_src int_src;
	union dsaf_tbl_int_sts int_sts;
	union dsaf_tbl_int_msk1 int_msk;
	struct dsaf_int_tbl_src *p;
	u32 dsaf_int_cnt = 0;

	p = &g_dsaf_int_tbl_stat[0];
	dsaf_int_tbl_sts_get(dsaf_id, &int_sts);
	osal_printf("dsaf tbl samis int,int_sts=%#x\n", int_sts.u32);

	dsaf_int_tbl_msk_get(dsaf_id, &int_msk);

	dsaf_int_tbl_msk_set(dsaf_id, 0xFFFFFFFF);

	int_src.u32 = 0;

	if (1 == int_sts.bits.tbl_sa_mis_sts) {
		int_src.bits.tbl_sa_mis_src = 1;
		p->tbl_sa_mis_src++;
		dsaf_int_cnt++;
	}

	/*sa miss, please cpy from m3 code TBD*/

	if (dsaf_int_cnt)
		dsaf_int_tbl_src_clr(dsaf_id, int_src);

	dsaf_int_tbl_msk_cfg(dsaf_id, int_msk);
}

/**
 * dsaf_tbl_ecc_int_handler - INT
 * @dsaf_id: dsa fabric id
 */
void dsaf_tbl_ecc_int_handler(u32 dsaf_id)
{
	union dsaf_tbl_int_src int_src;
	union dsaf_tbl_int_sts int_sts;
	union dsaf_tbl_int_msk1 int_msk;
	struct dsaf_int_tbl_src *p;
	u32 dsaf_int_cnt = 0;

	p = &g_dsaf_int_tbl_stat[0];
	dsaf_int_tbl_sts_get(dsaf_id, &int_sts);
	osal_printf("dsaf tbl ecc int,int_sts=%#x\n", int_sts.u32);

	dsaf_int_tbl_msk_get(dsaf_id, &int_msk);

	dsaf_int_tbl_msk_set(dsaf_id, 0xFFFFFFFF);

	int_src.u32 = 0;

	if (1 == int_sts.bits.lram_ecc_err1_sts) {
		int_src.bits.lram_ecc_err1_src = 1;
		p->lram_ecc_err1_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.lram_ecc_err2_sts) {
		int_src.bits.lram_ecc_err2_src = 1;
		p->lram_ecc_err2_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tram_ecc_err1_sts) {
		int_src.bits.tram_ecc_err1_src = 1;
		p->tram_ecc_err1_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tram_ecc_err2_sts) {
		int_src.bits.tram_ecc_err2_src = 1;
		p->tram_ecc_err2_src++;
		dsaf_int_cnt++;
	}
	if (1 == int_sts.bits.tbl_old_sech_end_sts) {
		int_src.bits.tbl_old_sech_end_src = 1;
		p->tbl_old_sech_end_src++;
		dsaf_int_cnt++;
	}

	if (dsaf_int_cnt)
		dsaf_int_tbl_src_clr(dsaf_id, int_src);

	dsaf_int_tbl_msk_cfg(dsaf_id, int_msk);
}

/**
 * dsaf_int_handler - INT
 * @dsaf_id: dsa fabric id
 * @irq_num: irq num
 */
void dsaf_int_handler(u32 dsaf_id, u32 irq_num)
{
	switch (irq_num) {
	case 0:
		dsaf_tbl_damis_int_handler(dsaf_id);
		break;
	case 1:
		dsaf_tbl_samis_int_handler(dsaf_id);
		break;
	case 2:
		dsaf_tbl_ecc_int_handler(dsaf_id);
		break;
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		dsaf_xge_int_handler(dsaf_id, irq_num - 3);
		break;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		dsaf_ppe_int_handler(dsaf_id, irq_num - 9);
		break;
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
		dsaf_rocee_int_handler(dsaf_id, irq_num - 15);
		break;
	default:
		pr_warn_once("interupt:irq_num(%d) err\n", irq_num);
		return;
	}
}

/**
 * dsaf_line_tbl_addr_get - INT
 * @dsaf_id: dsa fabric id
 * @port_num:
 * @vc:
 * @line_num:
 */
u32 dsaf_line_tbl_addr_get(u32 port_num, u32 vc, u32 line_num)
{
	u32 line_tbl_addr;

	line_tbl_addr =	((((port_num & 0x7) << 1)
		| (vc & 0x1)) << 11) | (line_num & 0x7FF);

	return line_tbl_addr;
}

/**
 * dsaf_single_line_tbl_cfg - INT
 * @dsaf_id: dsa fabric id
 * @address:
 * @ptbl_line:
 */
static inline void dsaf_single_line_tbl_cfg(u32 dsaf_id, u32 address,
	struct dsaf_tbl_line_cfg *ptbl_line)
{
	/*Write Addr*/
	dsaf_tbl_line_addr_cfg(dsaf_id, address);

	/*Write Line*/
	dsaf_tbl_line_cfg(dsaf_id, ptbl_line);

	/*Write Plus*/
	dsaf_tbl_line_pul(dsaf_id);
}

/**
 * dsaf_tcam_uc_cfg - INT
 * @dsaf_id: dsa fabric id
 * @address,
 * @ptbl_tcam_data,
 */
void dsaf_tcam_uc_cfg(u32 dsaf_id, u32 address,
	struct dsaf_tbl_tcam_data *ptbl_tcam_data,
	struct dsaf_tbl_tcam_ucast_cfg *ptbl_tcam_ucast)
{
	/*Write Addr*/
	dsaf_tbl_tcam_addr_cfg(dsaf_id, address);
	/*Write Tcam Data*/
	dsaf_tbl_tcam_data_cfg(dsaf_id, ptbl_tcam_data);
	/*Write Tcam Ucast*/
	dsaf_tbl_tcam_ucast_cfg(dsaf_id, ptbl_tcam_ucast);
	/*Write Plus*/
	dsaf_tbl_tcam_data_ucast_pul(dsaf_id);
}

/**
 * dsaf_tcam_mc_cfg - INT
 * @dsaf_id: dsa fabric id
 * @address,
 * @ptbl_tcam_data,
 * @ptbl_tcam_mcast,
 */
void dsaf_tcam_mc_cfg(u32 dsaf_id, u32 address,
	struct dsaf_tbl_tcam_data *ptbl_tcam_data,
	struct dsaf_tbl_tcam_mcast_cfg *ptbl_tcam_mcast)
{
	/*Write Addr*/
	dsaf_tbl_tcam_addr_cfg(dsaf_id, address);
	/*Write Tcam Data*/
	dsaf_tbl_tcam_data_cfg(dsaf_id, ptbl_tcam_data);
	/*Write Tcam Mcast*/
	(void)dsaf_tbl_tcam_mcast_cfg(dsaf_id, ptbl_tcam_mcast);
	/*Write Plus*/
	dsaf_tbl_tcam_data_mcast_pul(dsaf_id);
}

/**
 * dsaf_tcam_mc_invld - INT
 * @dsaf_id: dsa fabric id
 * @address
 */
void dsaf_tcam_mc_invld(u32 dsaf_id, u32 address)
{
   /*Write Addr*/
	dsaf_tbl_tcam_addr_cfg(dsaf_id, address);

	/*write tcam mcast*/
	dsaf_write(dsaf_id, 0, DSAF_TBL_TCAM_MCAST_CFG_0_0_REG);
	dsaf_write(dsaf_id, 0, DSAF_TBL_TCAM_MCAST_CFG_1_0_REG);
	dsaf_write(dsaf_id, 0, DSAF_TBL_TCAM_MCAST_CFG_2_0_REG);
	dsaf_write(dsaf_id, 0, DSAF_TBL_TCAM_MCAST_CFG_3_0_REG);
	dsaf_write(dsaf_id, 0, DSAF_TBL_TCAM_MCAST_CFG_4_0_REG);

	/*Write Plus*/
	dsaf_tbl_tcam_mcast_pul(dsaf_id);
}

/**
 * dsaf_tcam_uc_get - INT
 * @dsaf_id: dsa fabric id
 * @address
 * @ptbl_tcam_data
 * @ptbl_tcam_ucast
 */
void dsaf_tcam_uc_get(u32 dsaf_id, u32 address,
	struct dsaf_tbl_tcam_data *ptbl_tcam_data,
	struct dsaf_tbl_tcam_ucast_cfg *ptbl_tcam_ucast)
{
	union dsaf_tbl_tcam_ucast_cfg1 tcam_read_data0;
	union dsaf_tbl_tcam_mcast_cfg_4 tcam_read_data4;

	/*Write Addr*/
	dsaf_tbl_tcam_addr_cfg(dsaf_id, address);

	/*read tcam item puls*/
	dsaf_tbl_tcam_load_pul(dsaf_id);

	/*read tcam data*/
	ptbl_tcam_data->tbl_tcam_data_high
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RDATA_LOW_0_REG);
	ptbl_tcam_data->tbl_tcam_data_low
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RDATA_HIGH_0_REG);

	/*read tcam mcast*/
	tcam_read_data0.u32
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA0_0_REG);
	tcam_read_data4.u32
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RAM_RDATA4_0_REG);
	ptbl_tcam_ucast->tbl_ucast_item_vld
		= tcam_read_data4.bits.tbl_mcast_item_vld;
	ptbl_tcam_ucast->tbl_ucast_old_en
		= tcam_read_data4.bits.tbl_mcast_old_en;
	ptbl_tcam_ucast->tbl_ucast_mac_discard
		= tcam_read_data0.bits.tbl_ucast_mac_discard;
	ptbl_tcam_ucast->tbl_ucast_out_port
		= tcam_read_data0.bits.tbl_ucast_out_port;
	ptbl_tcam_ucast->tbl_ucast_dvc
		= tcam_read_data0.bits.tbl_ucast_dvc;
}

/**
 * dsaf_tcam_mc_get - INT
 * @dsaf_id: dsa fabric id
 * @address
 * @ptbl_tcam_data
 * @ptbl_tcam_ucast
 */
void dsaf_tcam_mc_get(u32 dsaf_id, u32 address,
	struct dsaf_tbl_tcam_data *ptbl_tcam_data,
	struct dsaf_tbl_tcam_mcast_cfg *ptbl_tcam_mcast)
{
	union dsaf_tbl_tcam_mcast_cfg_0 tcam_read_data0;
	u32 tcam_read_data1;
	u32 tcam_read_data2;
	u32 tcam_read_data3;
	union dsaf_tbl_tcam_mcast_cfg_4 tcam_read_data4;

   /*Write Addr*/
	dsaf_tbl_tcam_addr_cfg(dsaf_id, address);

	/*read tcam item puls*/
	dsaf_tbl_tcam_load_pul(dsaf_id);

	/*read tcam data*/
	ptbl_tcam_data->tbl_tcam_data_high
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RDATA_LOW_0_REG);
	ptbl_tcam_data->tbl_tcam_data_low
		= dsaf_read(dsaf_id, DSAF_TBL_TCAM_RDATA_HIGH_0_REG);
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

	ptbl_tcam_mcast->tbl_mcast_item_vld
		= tcam_read_data4.bits.tbl_mcast_item_vld;
	ptbl_tcam_mcast->tbl_mcast_old_en
		= tcam_read_data4.bits.tbl_mcast_old_en;
	ptbl_tcam_mcast->tbl_mcast_vm128_120
		= (tcam_read_data4.bits.tbl_mcast_vm128_122 << 2)
		| ((tcam_read_data3 & 0xc0000000) >> 30);
	ptbl_tcam_mcast->tbl_mcast_vm119_110
		= (tcam_read_data3 & 0x3ff00000) >> 20;
	ptbl_tcam_mcast->tbl_mcast_vm109_100
		= (tcam_read_data3 & 0x000ffc00) >> 10;
	ptbl_tcam_mcast->tbl_mcast_vm99_90
		= tcam_read_data3 & 0x000003ff;
	ptbl_tcam_mcast->tbl_mcast_vm89_80
		= (tcam_read_data2 & 0xffc00000) >> 22;
	ptbl_tcam_mcast->tbl_mcast_vm79_70
		= (tcam_read_data2 & 0x003ff000) >> 12;
	ptbl_tcam_mcast->tbl_mcast_vm69_60
		= (tcam_read_data2 & 0x00000ffc) >> 2;
	ptbl_tcam_mcast->tbl_mcast_vm59_50
		= ((tcam_read_data2 & 0x00000003) << 8)
		| ((tcam_read_data1 & 0xff000000) >> 24);
	ptbl_tcam_mcast->tbl_mcast_vm49_40
		= (tcam_read_data1 & 0x00ffc000) >> 14;
	ptbl_tcam_mcast->tbl_mcast_vm39_30
		= (tcam_read_data1 & 0x00003ff0) >> 4;
	ptbl_tcam_mcast->tbl_mcast_vm29_20
		= ((tcam_read_data1 & 0x0000000f) << 6)
		| ((tcam_read_data0.u32 & 0xfc000000) >> 26);
	ptbl_tcam_mcast->tbl_mcast_vm19_10
		= (tcam_read_data0.u32 & 0x03ff0000) >> 16;
	ptbl_tcam_mcast->tbl_mcast_vm9_0
		= (tcam_read_data0.u32 & 0x0000ffc0) >> 6;
	ptbl_tcam_mcast->tbl_mcast_xge5_0
		= tcam_read_data0.bits.tbl_mcast_xge5_0;
}

/**
 * dsaf_tbl_line_init - INT
 * @dsaf_id: dsa fabric id
 */
void dsaf_tbl_line_init(u32 dsaf_id)
{
	u32 i;
	/* defaultly set all lineal mac table entry resulting discard */
	struct dsaf_tbl_line_cfg tbl_line[] = {{1, 0, 0}};

	for (i = 0; i < DSAF_LINE_SUM; i++)
		dsaf_single_line_tbl_cfg(dsaf_id, i, tbl_line);

}

/**
 * dsaf_tbl_tcam_init - INT
 * @dsaf_id: dsa fabric id
 */
void dsaf_tbl_tcam_init(u32 dsaf_id)
{
	u32 i;

	/*out->ge/xge*/
	struct dsaf_tbl_tcam_data tbl_tcam_data[] = {{0, 0}};

	struct dsaf_tbl_tcam_ucast_cfg tbl_tcam_ucast[] = {{0, 0, 0, 0, 0}};

	/*tcam tbl*/
	for (i = 0; i < DSAF_TCAM_SUM; i++) {
		dsaf_tcam_uc_cfg(dsaf_id, i, tbl_tcam_data,	tbl_tcam_ucast);
	}
}

/**
 * dsaf_tbl_tcam_init - INT
 * @dsaf_id: dsa fabric id
 * @dsaf_mode
 */
void dsaf_comm_init(u32 dsaf_id, enum dsaf_mode dsaf_mode)
{
	/*common*/
	u32 i = 0;
	u32 mask_set = 0x0;
	union dsaf_xge_int_msk int_msk_xge;
	union dsaf_ppe_int_msk int_msk_ppe;
	union dsaf_rocee_int_msk int_msk_rocee;

	dsaf_en(dsaf_id, g_dsaf_cfg.dsaf_en);

	if ((dsaf_mode == DSAF_MODE_ENABLE_16VM)
		|| (dsaf_mode == DSAF_MODE_DISABLE_2PORT_8VM)
		|| (dsaf_mode == DSAF_MODE_DISABLE_6PORT_2VM))
		g_dsaf_cfg.dsaf_tc_mode = HRD_DSAF_8TC_MODE;
	else
		g_dsaf_cfg.dsaf_tc_mode = HRD_DSAF_4TC_MODE;

	dsaf_tc_mode_cfg(dsaf_id, g_dsaf_cfg.dsaf_tc_mode);

	dsaf_crc_en(dsaf_id, g_dsaf_cfg.dsaf_crc_add_en);
	dsaf_mix_mode_cfg(dsaf_id, g_dsaf_cfg.dsaf_mix_mode);
	dsaf_local_addr_en_cfg(dsaf_id, g_dsaf_cfg.dsaf_local_addr_en);
	dsaf_reg_cnt_clr_ce(dsaf_id, g_dsaf_cfg.cnt_clr_ce);

	dsaf_stp_port_type_cfg(dsaf_id, DSAF_STP_PORT_TYPE_FORWARD);

	/* set 22 queue per tx ppe engine, only used in switch mode */
	dsaf_ppe_qid_cfg(dsaf_id, QUEUE_PER_TX_PPE_ENGINE);

	/* in non switch mode, set all port to access mode */
	dsaf_sw_port_type_cfg(dsaf_id, DSAF_SW_PORT_TYPE_NON_VLAN);
	/*dsaf_sw_port_type_cfg(dsaf_id, DSAF_SW_PORT_TYPE_ACCESS);*/

	/* dsaf_port_def_vlan_cfg(dsaf_id, 0); TBD */

	/* dsaf_vm_def_vlan_cfg(dsaf_id); TBD */

	/*int statistic zero init*/
	dsaf_int_stat_init();

	/*shield 2bit ecc INT*/
	int_msk_xge.u32 = 0;
	int_msk_xge.bits.sbm_xge_sram_ecc_2bit_int_msk = 1;

	int_msk_ppe.u32 = 0;
	int_msk_ppe.bits.sbm_ppe_sram_ecc_2bit_int_msk = 1;

	int_msk_rocee.u32 = 0;
	int_msk_rocee.bits.sbm_rocee_sram_ecc_2bit_int_msk = 1;

	/*open int*/
	for (i = 0; i < DSAF_COMM_CHN; i++) {
		dsaf_int_xge_msk_set(dsaf_id, i, int_msk_xge.u32);
		dsaf_int_ppe_msk_set(dsaf_id, i, int_msk_ppe.u32);
		dsaf_int_rocee_msk_set(dsaf_id, i, int_msk_rocee.u32);
	}

	mask_set = 0x3FFFFF;
	dsaf_int_tbl_msk_set(dsaf_id, mask_set);
	pr_debug("dsaf_comm_init R: \r\n");

}

/**
 * dsaf_inode_init - INT
 * @dsaf_id: dsa fabric id
 */
void dsaf_inode_init(u32 dsaf_id)
{
	/*dsaf_inode_cut_through_cfg(dsaf_id, ginode_cut_through_cfg);
	pr_debug("dsaf_init_hw dsaf_inode_cut_through_cfg dsaf%d !\n", dsaf_id); */

	dsaf_inode_in_port_num(dsaf_id, ginode_in_port_num); /* TBD */
	pr_debug("dsaf_init_hw dsaf_inode_in_port_num dsaf%d !\n", dsaf_id);

	if (HRD_DSAF_4TC_MODE == g_dsaf_cfg.dsaf_tc_mode)
		dsaf_inode_pri_tc_cfg(dsaf_id, ginode_pri_tc_cfg);
	else
		dsaf_inode_pri_tc_cfg(dsaf_id, ginode_pri_tc_cfg + 1);

	pr_debug("dsaf_inode_pri_tc_cfg dsaf%d !\n", dsaf_id);
}

/**
 * dsaf_sbm_init - INT
 * @dsaf_id: dsa fabric id
 */
int dsaf_sbm_init(u32 dsaf_id)
{
	u32 sram_init_over;
	u32 read_nt = 0;
	int ret = 0;

	/* dsaf_sbm_info_fifo_thrd_cfg(dsaf_id,
	gsbm_inf_fifo_sread_thrd, gsbm_inf_fifo_aful_thrd); */
	dsaf_sbm_bp_wl_cfg(dsaf_id);

	/*step 2 :	enable sbm chanel
				disable sbm chanel shcut function*/
	dsaf_sbm_cfg(dsaf_id);

	/*step 3 :enable sbm mib */
	ret = dsaf_sbm_cfg_mib_en(dsaf_id);
	if(ret) {
		pr_info("dsaf_sbm_cfg_mib_en fail,dsaf_id=%d, ret=%d\n",
			dsaf_id, ret);
		return -ENOSYS;
	}

	/*step 4 :	enable sbm initial link sram */
	dsaf_sbm_link_sram_init_en(dsaf_id);

	do {
		udelay(200);
		sram_init_over = dsaf_read(dsaf_id, DSAF_SRAM_INIT_OVER_0_REG);
		read_nt++;
	} while (sram_init_over != 0xFF && read_nt < DSAF_CFG_READ_CNT);

	if(sram_init_over != 0xFF) {
		pr_info("dsaf_sbm_init fail dsaf_id=%d,ret=%d\n",
			dsaf_id, ret);
		return -ENOSYS;
	}

	dsaf_rocee_bp_en(dsaf_id);

	return 0;

}

/**
 * dsaf_tbl_init - INT
 * @dsaf_id: dsa fabric id
 */
void dsaf_tbl_init(u32 dsaf_id)
{
	dsaf_tbl_stat_en(dsaf_id);

	dsaf_tbl_tcam_init(dsaf_id);
	dsaf_tbl_line_init(dsaf_id);
}

/**
 * dsaf_voq_init - INT
 * @dsaf_id: dsa fabric id
 */
void dsaf_voq_init(u32 dsaf_id)
{
	dsaf_voq_bp_all_thrd_cfg(dsaf_id);
}

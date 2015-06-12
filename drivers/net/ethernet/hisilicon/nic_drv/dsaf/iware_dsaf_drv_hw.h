/*******************************************************************************

  Hisilicon dsa fabric driver
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

#ifndef _IWARE_DSAF_DRV_HW_H_
#define _IWARE_DSAF_DRV_HW_H_

#include "iware_dsaf_reg_define.h"
#include "iware_dsaf_reg_offset.h"
#include "iware_dsaf_main.h"

#define DSAF_XGE_NUM		6
#define DSAF_PPE_TX_NUM 6
#define DSAF_COMM_CHN		6
#define DSAF_XOD_NUM		8
#define DSAF_XOD_BIG_NUM	18
#define DSAF_SBM_NUM		18
#define DSAF_VOQ_NUM		18
#define DSAF_XGE_VOQ_NUM	6
#define DSAF_PPE_VOQ_NUM	6
#define DSAF_ROC_VOQ_NUM	6
#define DSAF_TBL_NUM		8
#define DSAF_XGE_CHN		6
#define DSAF_PPE_CHN		6
#define DSAF_ROCEE_CHN		  6
#define DSAF_FIFO_NUM		6
#define DSAF_SW_PORT_NUM	8
#define DSAF_PPE_QID_NUM	129

#define DSAF_TCAM_SUM		512
#define DSAF_LINE_SUM		(2048*14)

#define MAX_SBM_CFG_TYPE   	3
#define QUEUE_PER_TX_PPE_ENGINE 22

#define DSAF_CFG_READ_CNT   30



extern unsigned long long g_dsaf_reg_addr[];

#define dsaf_read(i, a)\
	readl((u32 __iomem *)(g_dsaf_reg_addr[i] + (a)))
#define dsaf_write(i, v, a)\
	writel((v), (u32 __iomem *)(g_dsaf_reg_addr[i] + (a)))

#define dsaf_non_zero_printf(value, format, args...)\
	do {\
		if (value)\
			osal_printf(format, ## args);\
	} while (0)

enum hal_dsaf_mode {
	HRD_DSAF_NO_DSAF_MODE	= 0x0,
	HRD_DSAF_MODE		= 0x1,
};

enum hal_dsaf_tc_mode {
	HRD_DSAF_4TC_MODE		= 0X0,
	HRD_DSAF_8TC_MODE		= 0X1,
};

struct dsaf_cfg {
	enum hal_dsaf_mode dsaf_en;
	enum hal_dsaf_tc_mode dsaf_tc_mode;
	u32 dsaf_crc_add_en;
	u32 dsaf_mix_mode;
	u32 dsaf_local_addr_en;
	u32 cnt_clr_ce;
};

struct dsaf_vm_def_vlan {
	u32 vm_def_vlan_id;
	u32 vm_def_vlan_cfi;
	u32 vm_def_vlan_pri;
};

struct dsaf_inode_pritc_cfg {
	u32 inode_no_vlan_tag_data_tc[DSAF_INODE_NUM];
	u32 inode_no_vlan_tag_man_tc[DSAF_INODE_NUM];
	u32 inode_pri0_tc[DSAF_INODE_NUM];
	u32 inode_pri1_tc[DSAF_INODE_NUM];
	u32 inode_pri2_tc[DSAF_INODE_NUM];
	u32 inode_pri3_tc[DSAF_INODE_NUM];
	u32 inode_pri4_tc[DSAF_INODE_NUM];
	u32 inode_pri5_tc[DSAF_INODE_NUM];
	u32 inode_pri6_tc[DSAF_INODE_NUM];
	u32 inode_pri7_tc[DSAF_INODE_NUM];
};

struct dsaf_sbm_cfg {
	u32 sbm_cfg_shcut_en[DSAF_SBM_NUM];
	u32 sbm_cfg_en[DSAF_SBM_NUM];
	u32 sbm_cfg_mib_en[DSAF_SBM_NUM];
};

struct dsaf_xod_cf_pause_sa {
	u32 xod_cf_pause_sa_h[DSAF_XGE_NUM];
	u32 xod_cf_pause_sa_l[DSAF_XGE_NUM];
};

struct dsaf_xod_cf_pause_para {
	u32 xod_cf_pause_thr[DSAF_XGE_NUM];
	u32 xod_cf_tx_timer[DSAF_XGE_NUM];
};

struct dsaf_tbl_tcam_data {
	u32 tbl_tcam_data_high;
	u32 tbl_tcam_data_low;
};

struct dsaf_tbl_tcam_mcast_cfg {
	u32 tbl_mcast_old_en;
	u32 tbl_mcast_item_vld;
	u32 tbl_mcast_vm128_120;
	u32 tbl_mcast_vm119_110;
	u32 tbl_mcast_vm109_100;
	u32 tbl_mcast_vm99_90;
	u32 tbl_mcast_vm89_80;
	u32 tbl_mcast_vm79_70;
	u32 tbl_mcast_vm69_60;
	u32 tbl_mcast_vm59_50;
	u32 tbl_mcast_vm49_40;
	u32 tbl_mcast_vm39_30;
	u32 tbl_mcast_vm29_20;
	u32 tbl_mcast_vm19_10;
	u32 tbl_mcast_vm9_0;
	u32 tbl_mcast_xge5_0;
};

struct dsaf_tbl_tcam_ucast_cfg {
	u32 tbl_ucast_old_en;
	u32 tbl_ucast_item_vld;
	u32 tbl_ucast_mac_discard;
	u32 tbl_ucast_dvc;
	u32 tbl_ucast_out_port;
};

struct dsaf_tbl_line_cfg {
	u32 tbl_line_mac_discard;
	u32 tbl_line_dvc;
	u32 tbl_line_out_port;
};

struct dsaf_int_xge_src {
	u32    xid_xge_ecc_err_int_src;
	u32    xid_xge_fsm_timout_int_src;
	u32    sbm_xge_lnk_fsm_timout_int_src;
	u32    sbm_xge_lnk_ecc_2bit_int_src;
	u32    sbm_xge_mib_req_failed_int_src;
	u32    sbm_xge_mib_req_fsm_timout_int_src;
	u32    sbm_xge_mib_rels_fsm_timout_int_src;
	u32    sbm_xge_sram_ecc_2bit_int_src;
	u32    sbm_xge_mib_buf_sum_err_int_src;
	u32    sbm_xge_mib_req_extra_int_src;
	u32    sbm_xge_mib_rels_extra_int_src;
	u32    voq_xge_start_to_over_0_int_src;
	u32    voq_xge_start_to_over_1_int_src;
	u32    voq_xge_ecc_err_int_src;
};

struct dsaf_int_ppe_src {
	u32    xid_ppe_fsm_timout_int_src;
	u32    sbm_ppe_lnk_fsm_timout_int_src;
	u32    sbm_ppe_lnk_ecc_2bit_int_src;
	u32    sbm_ppe_mib_req_failed_int_src;
	u32    sbm_ppe_mib_req_fsm_timout_int_src;
	u32    sbm_ppe_mib_rels_fsm_timout_int_src;
	u32    sbm_ppe_sram_ecc_2bit_int_src;
	u32    sbm_ppe_mib_buf_sum_err_int_src;
	u32    sbm_ppe_mib_req_extra_int_src;
	u32    sbm_ppe_mib_rels_extra_int_src;
	u32    voq_ppe_start_to_over_0_int_src;
	u32    voq_ppe_ecc_err_int_src;
	u32    xod_ppe_fifo_rd_empty_int_src;
	u32    xod_ppe_fifo_wr_full_int_src;
};

struct dsaf_int_rocee_src {
	u32    xid_rocee_fsm_timout_int_src;
	u32    sbm_rocee_lnk_fsm_timout_int_src;
	u32    sbm_rocee_lnk_ecc_2bit_int_src;
	u32    sbm_rocee_mib_req_failed_int_src;
	u32    sbm_rocee_mib_req_fsm_timout_int_src;
	u32    sbm_rocee_mib_rels_fsm_timout_int_src;
	u32    sbm_rocee_sram_ecc_2bit_int_src;
	u32    sbm_rocee_mib_buf_sum_err_int_src;
	u32    sbm_rocee_mib_req_extra_int_src;
	u32    sbm_rocee_mib_rels_extra_int_src;
	u32    voq_rocee_start_to_over_0_int_src;
	u32    voq_rocee_ecc_err_int_src;
};


struct dsaf_int_tbl_src {
	u32    tbl_da0_mis_src;
	u32    tbl_da1_mis_src;
	u32    tbl_da2_mis_src;
	u32    tbl_da3_mis_src;
	u32    tbl_da4_mis_src;
	u32    tbl_da5_mis_src;
	u32    tbl_da6_mis_src;
	u32    tbl_da7_mis_src;
	u32    tbl_sa_mis_src;
	u32    tbl_old_sech_end_src;
	u32    lram_ecc_err1_src;
	u32    lram_ecc_err2_src;
	u32    tram_ecc_err1_src;
	u32    tram_ecc_err2_src;
	u32    tbl_ucast_bcast_xge0_src;
	u32    tbl_ucast_bcast_xge1_src;
	u32    tbl_ucast_bcast_xge2_src;
	u32    tbl_ucast_bcast_xge3_src;
	u32    tbl_ucast_bcast_xge4_src;
	u32    tbl_ucast_bcast_xge5_src;
	u32    tbl_ucast_bcast_ppe_src;
	u32    tbl_ucast_bcast_rocee_src;
};

enum dsaf_port_rate_mode {
	DSAF_PORT_RATE_1000 = 0,
	DSAF_PORT_RATE_2500,
	DSAF_PORT_RATE_10000
};

enum dsaf_stp_port_type {
	DSAF_STP_PORT_TYPE_DISCARD = 0,
	DSAF_STP_PORT_TYPE_BLOCK = 1,
	DSAF_STP_PORT_TYPE_LISTEN = 2,
	DSAF_STP_PORT_TYPE_LEARN = 3,
	DSAF_STP_PORT_TYPE_FORWARD = 4
};

enum dsaf_sw_port_type {
	DSAF_SW_PORT_TYPE_NON_VLAN = 0,
	DSAF_SW_PORT_TYPE_ACCESS = 1,
	DSAF_SW_PORT_TYPE_TRUNK = 2,
};

static inline void dsaf_tbl_tcam_addr_cfg(u32 dsaf_id, u32 tab_tcam_addr)
{
	union dsaf_tbl_tcam_addr o_tbl_tcam_addr;

	o_tbl_tcam_addr.u32 = dsaf_read(dsaf_id, DSAF_TBL_TCAM_ADDR_0_REG);
	o_tbl_tcam_addr.bits.tbl_tcam_addr = tab_tcam_addr;
	dsaf_write(dsaf_id, o_tbl_tcam_addr.u32, DSAF_TBL_TCAM_ADDR_0_REG);
}

static inline void dsaf_tbl_tcam_load_pul(u32 dsaf_id)
{
	union dsaf_tbl_pul o_tbl_pul;

	o_tbl_pul.u32 = dsaf_read(dsaf_id, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_tcam_load = 1;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_tcam_load = 0;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);
}

static inline void dsaf_tbl_line_addr_cfg(u32 dsaf_id, u32 tab_line_addr)
{
	union dsaf_tbl_line_addr o_tbl_line_addr;

	o_tbl_line_addr.u32 = dsaf_read(dsaf_id, DSAF_TBL_LINE_ADDR_0_REG);
	o_tbl_line_addr.bits.tbl_line_addr = tab_line_addr;
	dsaf_write(dsaf_id, o_tbl_line_addr.u32, DSAF_TBL_LINE_ADDR_0_REG);
}

static inline void dsaf_tbl_line_load_pul(u32 dsaf_id)
{
	union dsaf_tbl_pul o_tbl_pul;

	o_tbl_pul.u32 = dsaf_read(dsaf_id, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_line_load = 1;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);

	o_tbl_pul.bits.tbl_line_load = 0;
	dsaf_write(dsaf_id, o_tbl_pul.u32, DSAF_TBL_PUL_0_REG);
}

extern struct dsaf_int_xge_src g_dsaf_int_xge_stat[];
extern struct dsaf_int_ppe_src g_dsaf_int_ppe_stat[];
extern struct dsaf_int_rocee_src g_dsaf_int_rocee_stat[];
extern struct dsaf_int_tbl_src g_dsaf_int_tbl_stat[];


#endif


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

#ifndef _IWARE_DSAF_REG_DEFINE_H_
#define _IWARE_DSAF_REG_DEFINE_H_

/* Define the union dsaf_cfg_reg */
/* COMM_BASE+0x0004 */
union dsaf_cfg_reg {
	/* Define the struct bits */
	struct {
		unsigned int dsaf_en:1;
		unsigned int dsaf_tc_mode:1;
		unsigned int dsaf_crc_en:1;
		unsigned int dsaf_sbm_init_en:1;
		unsigned int dsaf_mix_mode:1;
		unsigned int dsaf_stp_mode:1;
		unsigned int dsaf_local_addr_en:1;
		unsigned int reserved_1:25;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_fabric_reg_cnt_clr_ce */
/* 0x2C */
union dsaf_fabric_reg_cnt_clr_ce {
	/* Define the struct bits */
	struct {
		unsigned int cnt_clr_ce:1;
		unsigned int snap_en:1;
		unsigned int reserved_4:30;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_inf_fifo_tiware_u */
/* 0x30 */
union dsaf_sbm_inf_fifo_thrd {
	/* Define the struct bits */
	struct {
		unsigned int dsaf_sbm_inf_fifo_sread_thrd:5;
		unsigned int dsaf_sbm_inf_fifo_aful_thrd:6;
		unsigned int reserved_6:21;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_pfc_unit_cnt */
/* DSAF_BASE0+COMM_BASE+0x070+0x0004*comm_chn */
union dsaf_pfc_unit_cnt {
	/* Define the struct bits */
	struct {
		unsigned int dsaf_pfc_unit_cnt:9;
		unsigned int reserved_10:23;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xge_int_msk */
/* DSAF_BASE0+COMM_BASE+0x0100+0x0004*dsaf_xge_chn */
union dsaf_xge_int_msk {
	/* Define the struct bits */
	struct {
		unsigned int xid_xge_ecc_err_int_msk:1;
		unsigned int xid_xge_fsm_timeout_int_msk:1;
		unsigned int reserved_13:6;
		unsigned int sbm_xge_lnk_fsm_timout_int_msk:1;
		unsigned int sbm_xge_lnk_ecc_2bit_int_msk:1;
		unsigned int sbm_xge_mib_req_failed_int_msk:1;
		unsigned int sbm_xge_mib_req_fsm_timout_int_msk:1;
		unsigned int sbm_xge_mib_rels_fsm_timout_int_msk:1;
		unsigned int sbm_xge_sram_ecc_2bit_int_msk:1;
		unsigned int sbm_xge_mib_buf_sum_err_int_msk:1;
		unsigned int sbm_xge_mib_req_extra_int_msk:1;
		unsigned int sbm_xge_mib_rels_extra_int_msk:1;
		unsigned int reserved_12:3;
		unsigned int voq_xge_start_to_over_0_int_msk:1;
		unsigned int voq_xge_start_to_over_1_int_msk:1;
		unsigned int voq_xge_ecc_err_int_msk:1;
		unsigned int reserved_11:9;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_ppe_int_msk */
/* DSAF_BASE0+COMM_BASE+0x0120+0x0004*dsaf_ppe_chn */
union dsaf_ppe_int_msk {
	/* Define the struct bits */
	struct {
		unsigned int xid_ppe_fsm_timeout_int_msk:1;
		unsigned int reserved_17:7;
		unsigned int sbm_ppe_lnk_fsm_timout_int_msk:1;
		unsigned int sbm_ppe_lnk_ecc_2bit_int_msk:1;
		unsigned int sbm_ppe_mib_req_failed_int_msk:1;
		unsigned int sbm_ppe_mib_req_fsm_timout_int_msk:1;
		unsigned int sbm_ppe_mib_rels_fsm_timout_int_msk:1;
		unsigned int sbm_ppe_sram_ecc_2bit_int_msk:1;
		unsigned int sbm_ppe_mib_buf_sum_err_int_msk:1;
		unsigned int sbm_ppe_mib_req_extra_int_msk:1;
		unsigned int sbm_ppe_mib_rels_extra_int_msk:1;
		unsigned int reserved_16:3;
		unsigned int voq_ppe_start_to_over_0_int_msk:1;
		unsigned int voq_ppe_ecc_err_int_msk:1;
		unsigned int reserved_15:2;
		unsigned int xod_ppe_fifo_rd_empty_int_msk:1;
		unsigned int xod_ppe_fifo_wr_full_int_msk:1;
		unsigned int reserved_14:6;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_rocee_int_msk_u */
/* DSAF_BASE0+COMM_BASE+0x0140+0x0004*dsaf_rocee_chn */
union dsaf_rocee_int_msk {
	/* Define the struct bits */
	struct {
		unsigned int xid_rocee_fsm_timeout_int_msk:1;
		unsigned int reserved_20:7;
		unsigned int sbm_rocee_lnk_fsm_timout_int_msk:1;
		unsigned int sbm_rocee_lnk_ecc_2bit_int_msk:1;
		unsigned int sbm_rocee_mib_req_failed_int_msk:1;
		unsigned int sbm_rocee_mib_req_fsm_timout_int_msk:1;
		unsigned int sbm_rocee_mib_rels_fsm_timout_int_msk:1;
		unsigned int sbm_rocee_sram_ecc_2bit_int_msk:1;
		unsigned int sbm_rocee_mib_buf_sum_err_int_msk:1;
		unsigned int sbm_rocee_mib_req_extra_int_msk:1;
		unsigned int sbm_rocee_mib_rels_extra_int_msk:1;
		unsigned int reserved_19:3;
		unsigned int voq_rocee_start_to_over_0_int_msk:1;
		unsigned int voq_rocee_ecc_err_int_msk:1;
		unsigned int reserved_18:10;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xge_int_src */
/* DSAF_BASE0+COMM_BASE+0x0160+0x0004*dsaf_xge_chn */
union dsaf_xge_int_src {
	/* Define the struct bits */
	struct {
		unsigned int xid_xge_ecc_err_int_src:1;
		unsigned int xid_xge_fsm_timeout_int_src:1;
		unsigned int reserved_23:6;
		unsigned int sbm_xge_lnk_fsm_timout_int_src:1;
		unsigned int sbm_xge_lnk_ecc_2bit_int_src:1;
		unsigned int sbm_xge_mib_req_failed_int_src:1;
		unsigned int sbm_xge_mib_req_fsm_timout_int_src:1;
		unsigned int sbm_xge_mib_rels_fsm_timout_int_src:1;
		unsigned int sbm_xge_sram_ecc_2bit_int_src:1;
		unsigned int sbm_xge_mib_buf_sum_err_int_src:1;
		unsigned int sbm_xge_mib_req_extra_int_src:1;
		unsigned int sbm_xge_mib_rels_extra_int_src:1;
		unsigned int reserved_22:3;
		unsigned int voq_xge_start_to_over_0_int_src:1;
		unsigned int voq_xge_start_to_over_1_int_src:1;
		unsigned int voq_xge_ecc_err_int_src:1;
		unsigned int reserved_21:9;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_ppe_int_src */
/* DSAF_BASE0+COMM_BASE+0x0180+0x0004*dsaf_ppe_chn */
union dsaf_ppe_int_src {
	/* Define the struct bits */
	struct {
		unsigned int xid_ppe_fsm_timeout_int_src:1;
		unsigned int reserved_27:7;
		unsigned int sbm_ppe_lnk_fsm_timout_int_src:1;
		unsigned int sbm_ppe_lnk_ecc_2bit_int_src:1;
		unsigned int sbm_ppe_mib_req_failed_int_src:1;
		unsigned int sbm_ppe_mib_req_fsm_timout_int_src:1;
		unsigned int sbm_ppe_mib_rels_fsm_timout_int_src:1;
		unsigned int sbm_ppe_sram_ecc_2bit_int_src:1;
		unsigned int sbm_ppe_mib_buf_sum_err_int_src:1;
		unsigned int sbm_ppe_mib_req_extra_int_src:1;
		unsigned int sbm_ppe_mib_rels_extra_int_src:1;
		unsigned int reserved_26:3;
		unsigned int voq_ppe_start_to_over_0_int_src:1;
		unsigned int voq_ppe_ecc_err_int_src:1;
		unsigned int reserved_25:2;
		unsigned int xod_ppe_fifo_rd_empty_int_src:1;
		unsigned int xod_ppe_fifo_wr_full_int_src:1;
		unsigned int reserved_24:6;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_rocee_int_src */

/* DSAF_BASE0+COMM_BASE+0x01A0+0x0004*dsaf_rocee_chn */
union dsaf_rocee_int_src {
	/* Define the struct bits */
	struct {
		unsigned int xid_rocee_fsm_timeout_int_src:1;
		unsigned int reserved_30:7;
		unsigned int sbm_rocee_lnk_fsm_timout_int_src:1;
		unsigned int sbm_rocee_lnk_ecc_2bit_int_src:1;
		unsigned int sbm_rocee_mib_req_failed_int_src:1;
		unsigned int sbm_rocee_mib_req_fsm_timout_int_src:1;
		unsigned int sbm_rocee_mib_rels_fsm_timout_int_src:1;
		unsigned int sbm_rocee_sram_ecc_2bit_int_src:1;
		unsigned int sbm_rocee_mib_buf_sum_err_int_src:1;
		unsigned int sbm_rocee_mib_req_extra_int_src:1;
		unsigned int sbm_rocee_mib_rels_extra_int_src:1;
		unsigned int reserved_29:3;
		unsigned int voq_rocee_start_to_over_0_int_src:1;
		unsigned int voq_rocee_ecc_err_int_src:1;
		unsigned int reserved_28:10;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xge_int_sts */

/* DSAF_BASE0+COMM_BASE+0x01C0+0x0004*dsaf_xge_chn */
union dsaf_xge_int_sts {
	/* Define the struct bits */
	struct {
		unsigned int xid_xge_ecc_err_int_sts:1;
		unsigned int xid_xge_fsm_timeout_int_sts:1;
		unsigned int reserved_33:6;
		unsigned int sbm_xge_lnk_fsm_timout_int_sts:1;
		unsigned int sbm_xge_lnk_ecc_2bit_int_sts:1;
		unsigned int sbm_xge_mib_req_failed_int_sts:1;
		unsigned int sbm_xge_mib_req_fsm_timout_int_sts:1;
		unsigned int sbm_xge_mib_rels_fsm_timout_int_sts:1;
		unsigned int sbm_xge_sram_ecc_2bit_int_sts:1;
		unsigned int sbm_xge_mib_buf_sum_err_int_sts:1;
		unsigned int sbm_xge_mib_req_extra_int_sts:1;
		unsigned int sbm_xge_mib_rels_extra_int_sts:1;
		unsigned int reserved_32:3;
		unsigned int voq_xge_start_to_over_0_int_sts:1;
		unsigned int voq_xge_start_to_over_1_int_sts:1;
		unsigned int voq_xge_ecc_err_int_sts:1;
		unsigned int reserved_31:9;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_ppe_int_sts */
/* DSAF_BASE0+COMM_BASE+0x01E0+0x0004*dsaf_ppe_chn */
union dsaf_ppe_int_sts {
	/* Define the struct bits */
	struct {
		unsigned int xid_ppe_fsm_timeout_int_sts:1;
		unsigned int reserved_37:7;
		unsigned int sbm_ppe_lnk_fsm_timout_int_sts:1;
		unsigned int sbm_ppe_lnk_ecc_2bit_int_sts:1;
		unsigned int sbm_ppe_mib_req_failed_int_sts:1;
		unsigned int sbm_ppe_mib_req_fsm_timout_int_sts:1;
		unsigned int sbm_ppe_mib_rels_fsm_timout_int_sts:1;
		unsigned int sbm_ppe_sram_ecc_2bit_int_sts:1;
		unsigned int sbm_ppe_mib_buf_sum_err_int_sts:1;
		unsigned int sbm_ppe_mib_req_extra_int_sts:1;
		unsigned int sbm_ppe_mib_rels_extra_int_sts:1;
		unsigned int reserved_36:3;
		unsigned int voq_ppe_start_to_over_0_int_sts:1;
		unsigned int voq_ppe_ecc_err_int_sts:1;
		unsigned int reserved_35:2;
		unsigned int xod_ppe_fifo_rd_empty_int_sts:1;
		unsigned int xod_ppe_fifo_wr_full_int_sts:1;
		unsigned int reserved_34:6;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_rocee_int_sts */
/* DSAF_BASE0+COMM_BASE+0x0200+0x0004*dsaf_rocee_chn */
union dsaf_rocee_int_sts {
	/* Define the struct bits */
	struct {
		unsigned int xid_rocee_fsm_timeout_int_sts:1;
		unsigned int reserved_40:7;
		unsigned int sbm_rocee_lnk_fsm_timout_int_sts:1;
		unsigned int sbm_rocee_lnk_ecc_2bit_int_sts:1;
		unsigned int sbm_rocee_mib_req_failed_int_sts:1;
		unsigned int sbm_rocee_mib_req_fsm_timout_int_sts:1;
		unsigned int sbm_rocee_mib_rels_fsm_timout_int_sts:1;
		unsigned int sbm_rocee_sram_ecc_2bit_int_sts:1;
		unsigned int sbm_rocee_mib_buf_sum_err_int_sts:1;
		unsigned int sbm_rocee_mib_req_extra_int_sts:1;
		unsigned int sbm_rocee_mib_rels_extra_int_sts:1;
		unsigned int reserved_39:3;
		unsigned int voq_rocee_start_to_over_0_int_sts:1;
		unsigned int voq_rocee_ecc_err_int_sts:1;
		unsigned int reserved_38:10;
	} bits;

	/* Define an unsigned member */
	unsigned int	u32;

};

/* Define the union dsaf_ppe_qid_cfg */
/* DSAF_BASE0+COMM_BASE+0x0300+0x0004*ppe_tx_num */
union dsaf_ppe_qid_cfg {
	/* Define the struct bits */
	struct {
		unsigned int dsaf_ppe_qid_cfg:8;
		unsigned int reserved_41:24;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sw_port_type_reg */
/* DSAF_BASE0+COMM_BASE+0x0320+0x0004*sw_port_num */
union dsaf_sw_port_type_reg {
	/* Define the struct bits */
	struct {
		unsigned int dsaf_sw_port_type:2;
		unsigned int reserved_42:30;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_stp_port_type_reg */
/* DSAF_BASE0+COMM_BASE+0x0340+0x0004*dsaf_xge_chn */
union dsaf_stp_port_type_reg {
	/* Define the struct bits */
	struct {
		unsigned int dsaf_stp_port_type:3;
		unsigned int reserved_43:29;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_mix_def_qid_u */
/* DSAF_BASE0+COMM_BASE+0x0360+0x0004*ppe_tx_num */
union dsaf_mix_def_qid {
	/* Define the struct bits */
	struct {
		unsigned int dsaf_mix_def_qid:8;
		unsigned int reserved_44:24;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_port_def_vlan_u */
/* DSAF_BASE0+COMM_BASE+0x0380+0x0004*sw_port_num */
union dsaf_port_def_vlan {
	/* Define the struct bits */
	struct {
		unsigned int dsaf_port_def_vlan:12;
		unsigned int reserved_45:20;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_vm_def_vlan_reg_u */
/* DSAF_BASE0+COMM_BASE+0x0400+0x0004*ppe_qid_num */
union dsaf_vm_def_vlan_reg {
	/* Define the struct bits */
	struct {
		unsigned int dsaf_vm_def_vlan_id:12;
		unsigned int dsaf_vm_def_vlan_cfi:1;
		unsigned int dsaf_vm_def_vlan_pri:3;
		unsigned int reserved_46:16;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_inode_cut_through_cfg */
/* INODE_BASE+0x0000 */
union dsaf_inode_cut_through_cfg {
	/* Define the struct bits */
	struct {
		unsigned int dxi_cut_through_en:1;
		unsigned int reserved_47:31;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_inode_ecc_invert_en */
/* DSAF_BASE0+INODE_BASE+0x0008+0x0080*xge_num */
union dsaf_inode_ecc_invert_en {
	/* Define the struct bits */
	struct {
		unsigned int inode_sram_ecc_invert_en:1;
		unsigned int reserved_48:31;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_inode_ecc_err_addr */
/* DSAF_BASE0+INODE_BASE+0x000C+0x0080*xge_num */
union dsaf_inode_ecc_err_addr {
	/* Define the struct bits */
	struct {
		unsigned int inode_ecc_err_addr :10;
		unsigned int reserved_49 : 22;
	} bits;

	/* Define an unsigned member */
	unsigned int	u32;

};

/* Define the union dsaf_inode_in_port_num */
/* DSAF_BASE0+INODE_BASE+0x0018+0x0080*inode_num */
union dsaf_inode_in_port_num {
	/* Define the struct bits */
	struct {
		unsigned int inode_in_port_num:3;
		unsigned int reserved_50:29;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_inode_pri_tc_cfg */
/* DSAF_BASE0+INODE_BASE+0x001C+0x0080*inode_num */
union dsaf_inode_pri_tc_cfg {
	/* Define the struct bits */
	struct {
		unsigned int inode_pri0_tc:3;
		unsigned int inode_pri1_tc:3;
		unsigned int inode_pri2_tc:3;
		unsigned int inode_pri3_tc:3;
		unsigned int inode_pri4_tc:3;
		unsigned int inode_pri5_tc:3;
		unsigned int inode_pri6_tc:3;
		unsigned int inode_pri7_tc:3;
		unsigned int inode_no_vlan_tag_man_tc:3;
		unsigned int inode_no_vlan_tag_data_tc:3;
		unsigned int reserved_51:2;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_inode_bp_status1 */
/* DSAF_BASE0+INODE_BASE+0x0020+0x0080*inode_num */
union dsaf_inode_bp_status1 {
	/* Define the struct bits */
	struct {
		unsigned int inode_tbl_bp_status:1;
		unsigned int tbl_inode_bp_status:1;
		unsigned int reserved_52:30;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_cfg_reg */
/* DSAF_BASE0+SBM_BASE+0x0000+0x0080*sbm_num */
union dsaf_sbm_cfg_reg {
	/* Define the struct bits */
	struct {
		unsigned int sbm_cfg_shcut_en:1;
		unsigned int sbm_cfg_en:1;
		unsigned int sbm_cfg_mib_en:1;
		unsigned int sbm_cfg_ecc_err_invert_en:1;
		unsigned int reserved_71:28;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_bp_cfg_0_reg */
/* DSAF_BASE0+SBM_BASE+0x0004+0x0080*sbm_num */
union dsaf_sbm_bp_cfg_0_reg {
	/* Define the struct bits */
	struct {
		unsigned int sbm_cfg_vc1_max_buf_num:10;
		unsigned int sbm_cfg_vc0_max_buf_num:10;
		unsigned int sbm_cfg_com_max_buf_num:11;
		unsigned int reserved_72:1;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_bp_cfg_1_reg */
/* DSAF_BASE0+SBM_BASE+0x0008+0x0080*sbm_num */
union dsaf_sbm_bp_cfg_1_reg {
	/* Define the struct bits */
	struct {
		unsigned int sbm_cfg_tc4_max_buf_num:10;
		unsigned int sbm_cfg_tc0_max_buf_num:10;
		unsigned int reserved_73:12;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;
};

/* Define the union dsaf_sbm_bp_cfg_2_reg */
/* DSAF_BASE0+SBM_BASE+0x000C+0x0080*sbm_num */
union dsaf_sbm_bp_cfg_2_reg {
	/* Define the struct bits */
	struct {
		unsigned int sbm_cfg_set_buf_num:10;
		unsigned int	sbm_cfg_reset_buf_num  : 10;
		unsigned int reserved_74:12;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_free_cnt_0  */
/* DSAF_BASE0+SBM_BASE+0x0010+0x0080*sbm_num */
union dsaf_sbm_free_cnt_0 {
	/* Define the struct bits */
	struct {
		unsigned int sbm_mib_vc1_free_buf_cnt:10;
		unsigned int sbm_mib_vc0_free_buf_cnt:10;
		unsigned int sbm_mib_com_free_buf_cnt:10;
		unsigned int reserved_75:2;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_free_cnt_1 */

/* DSAF_BASE0+SBM_BASE+0x0014+0x0080*sbm_num */
union dsaf_sbm_free_cnt_1 {
	/* Define the struct bits */
	struct {
		unsigned int sbm_mib_tc4_free_buf_cnt:10;
		unsigned int sbm_mib_tc0_free_buf_cnt:10;
		unsigned int reserved_76:12;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_bp_cnt_0 */

/* DSAF_BASE0+SBM_BASE+0x0018+0x0080*sbm_num */
union dsaf_sbm_bp_cnt_0 {
	/* Define the struct bits */
	struct {
		unsigned int sbm_mib_tc2_used_buf_cnt:10;
		unsigned int sbm_mib_tc1_used_buf_cnt:10;
		unsigned int sbm_mib_tc0_used_buf_cnt:10;
		unsigned int reserved_77:2;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_bp_cnt_1 */
/* DSAF_BASE0+SBM_BASE+0x001C+0x0080*sbm_num */
union dsaf_sbm_bp_cnt_1 {
	/* Define the struct bits */
	struct {
		unsigned int sbm_mib_tc5_used_buf_cnt:10;
		unsigned int sbm_mib_tc4_used_buf_cnt:10;
		unsigned int sbm_mib_tc3_used_buf_cnt:10;
		unsigned int reserved_78:2;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_bp_cnt_2 */
/* DSAF_BASE0+SBM_BASE+0x0020+0x0080*sbm_num */
union dsaf_sbm_bp_cnt_2 {
	/* Define the struct bits */
	struct {
		unsigned int sbm_mib_tc7_used_buf_cnt:10;
		unsigned int sbm_mib_tc6_used_buf_cnt:10;
		unsigned int reserved_79:12;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_bp_cnt_3 */
/* DSAF_BASE0+SBM_BASE+0x0024+0x0080*sbm_num */
union dsaf_sbm_bp_cnt_3 {
	/* Define the struct bits */
	struct {
		unsigned int sbm_xge_vc0_tc0_bp:1;
		unsigned int sbm_xge_vc0_tc1_bp:1;
		unsigned int sbm_xge_vc0_tc2_bp:1;
		unsigned int sbm_xge_vc0_tc3_bp:1;
		unsigned int sbm_xge_vc1_tc0_bp:1;
		unsigned int sbm_xge_vc1_tc1_bp:1;
		unsigned int sbm_xge_vc1_tc2_bp:1;
		unsigned int sbm_xge_vc1_tc3_bp:1;
		unsigned int sbm_dxi_vc0_tc0_bp:1;
		unsigned int sbm_dxi_vc0_tc1_bp:1;
		unsigned int sbm_dxi_vc0_tc2_bp:1;
		unsigned int sbm_dxi_vc0_tc3_bp:1;
		unsigned int sbm_dxi_vc1_tc0_bp:1;
		unsigned int sbm_dxi_vc1_tc1_bp:1;
		unsigned int sbm_dxi_vc1_tc2_bp:1;
		unsigned int sbm_dxi_vc1_tc3_bp:1;
		unsigned int reserved_80:16;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_iner_st */

/* DSAF_BASE0+SBM_BASE+0x0028+0x0080*sbm_num */
union dsaf_sbm_iner_st {
	/* Define the struct bits */
	struct {
		unsigned int sbm_lnk_crt_st:6;
		unsigned int sbm_mib_req_crt_st:7;
		unsigned int sbm_mib_rels_crt_st:6;
		unsigned int reserved_81:13;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_mib_req_failed */
/* DSAF_BASE0+SBM_BASE+0x002C+0x0080*sbm_num */
union dsaf_sbm_mib_req_failed {
	/* define the struct bits */
	struct {
		unsigned int reserved_26:10;	/*[31..22]	*/
		unsigned int sbm_mib_req_failed_tc:3;	/*[21..19]	*/
		unsigned int reserved:19;	/*[18..9]  */
	} bits;

	/* define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_bp_cfg_3_reg */
/* DSAF_BASE0+SBM_BASE+0x0068+0x0080*sbm_num */
union dsaf_sbm_bp_cfg_3_reg {
	/* Define the struct bits */
	struct {
		unsigned int sbm_cfg_set_buf_num_no_pfc:10;
		unsigned int sbm_cfg_reset_buf_num_no_pfc:10;
		unsigned int reserved_96:12;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_sbm_bp_cfg_4_reg */
/* DSAF_BASE0+SBM_BASE+0x006C+0x0080*sbm_num */
union dsaf_sbm_bp_cfg_4_reg {
	/* Define the struct bits */
	struct {
		unsigned int sbm_cfg_set_buf_num_pause:10;
		unsigned int sbm_cfg_reset_buf_num_pause:10;
		unsigned int reserved_97:12;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xod_ets_tsa_tc0_tc3_cfg */
/* DSAF_BASE0+XOD_BASE+0x0000+0x0090*xod_num */
union dsaf_xod_ets_tsa_tc0_tc3_cfg {
	/* Define the struct bits */
	struct {
		unsigned int xod_tsa_tc0:8;
		unsigned int xod_tsa_tc1:8;
		unsigned int xod_tsa_tc2:8;
		unsigned int xod_tsa_tc3:8;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xod_ets_tsa_tc4_tc7_cfg */
/* DSAF_BASE0+XOD_BASE+0x0004+0x0090*xod_num */
union dsaf_xod_ets_tsa_tc4_tc7_cfg {
	/* Define the struct bits */
	struct {
		unsigned int xod_tsa_tc4:8;
		unsigned int xod_tsa_tc5:8;
		unsigned int xod_tsa_tc6:8;
		unsigned int xod_tsa_tc7:8;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xod_ets_bw_tc0_tc3_cfg */
/* DSAF_BASE0+XOD_BASE+0x0008+0x0090*xod_num */
union dsaf_xod_ets_bw_tc0_tc3_cfg {
	/* Define the struct bits */
	struct {
		unsigned int xod_ets_bw_tc0:8;
		unsigned int xod_ets_bw_tc1:8;
		unsigned int xod_ets_bw_tc2:8;
		unsigned int xod_ets_bw_tc3:8;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xod_ets_bw_tc4_tc7_cfg */
/* DSAF_BASE0+XOD_BASE+0x000C+0x0090*xod_num */
union dsaf_xod_ets_bw_tc4_tc7_cfg {
	/* Define the struct bits */
	struct {
		unsigned int xod_ets_bw_tc4:8;
		unsigned int xod_ets_bw_tc5:8;
		unsigned int xod_ets_bw_tc6:8;
		unsigned int xod_ets_bw_tc7:8;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xod_ets_bw_offset_cfg */
/* DSAF_BASE0+XOD_BASE+0x0010+0x0090*xod_num */
union dsaf_xod_ets_bw_offset_cfg {
	/* Define the struct bits */
	struct {
		unsigned int xod_ets_bw_ost_tc0:3;
		unsigned int xod_ets_bw_ost_tc1:3;
		unsigned int xod_ets_bw_ost_tc2:3;
		unsigned int xod_ets_bw_ost_tc3:3;
		unsigned int xod_ets_bw_ost_tc4:3;
		unsigned int xod_ets_bw_ost_tc5:3;
		unsigned int xod_ets_bw_ost_tc6:3;
		unsigned int xod_ets_bw_ost_tc7:3;
		unsigned int reserved_98:8;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xod_ets_token_cfg */
/* DSAF_BASE0+XOD_BASE+0x0014+0x0090*xod_num */
union dsaf_xod_ets_token_cfg {
	/* Define the struct bits */
	struct {
		unsigned int xod_ets_token_thr:13;
		unsigned int reserved_99:19;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xod_pfs_cfg_0 */
/* DSAF_BASE0+XOD_BASE+0x0018+0x0090*xge_num */
union dsaf_xod_pfs_cfg_0 {
	/* Define the struct bits */
	struct {
		unsigned int xod_cf_pause_sa_h:16;
		unsigned int reserved_100:16;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xod_pfs_cfg_2 */
/* DSAF_BASE0+XOD_BASE+0x0020+0x0090*xge_num */
union dsaf_xod_pfs_cfg_2 {
	/* Define the struct bits */
	struct {
		unsigned int xod_cf_tx_timer:16;
		unsigned int xod_cf_pause_thr:16;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xod_fifo_status */
/* DSAF_BASE0+XOD_BASE+0x007C+0x0090*xod_ppe_num */
union dsaf_xod_fifo_status {
	/* Define the struct bits */
	struct {
		unsigned int xod_rocee_fifo_pfull:1;
		unsigned int xod_rocee_fifo_full:1;
		unsigned int xod_rocee_fifo_empty:1;
		unsigned int xod_ppe_fifo_pfull:1;
		unsigned int xod_ppe_fifo_full:1;
		unsigned int xod_ppe_fifo_empty:1;
		unsigned int reserved_121:26;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_voq_ecc_invert_en */
/* DSAF_BASE0+VOQ_BASE+0x0004+0x0040*voq_num */
union dsaf_voq_ecc_invert_en {
	/* Define the struct bits */
	struct {
		unsigned int voq_sram_ecc_invert_en:1;
		unsigned int reserved_122:31;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_voq_sram_pkt_num */
/* DSAF_BASE0+VOQ_BASE+0x0008+0x0040*voq_num */
union dsaf_voq_sram_pkt_num {
	/* Define the struct bits */
	struct {
		unsigned int voq_sram_pkt_num:10;
		unsigned int reserved_123:22;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};


/* Define the union dsaf_voq_ecc_err_addr */
/* DSAF_BASE0+VOQ_BASE+0x0014+0x0040*voq_num */
union dsaf_voq_ecc_err_addr {
	/* Define the struct bits */
	struct {
		unsigned int voq_ecc_err_addr:10;
		unsigned int reserved_126:22;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_voq_bp_status */
/* DSAF_BASE0+VOQ_BASE+0x0018+0x0040*voq_num */
union dsaf_voq_bp_status {
	/* Define the struct bits */
	struct {
		unsigned int voq_dxi_rdy:1;
		unsigned int reserved_127:31;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_voq_spup_idle */
/* DSAF_BASE0+VOQ_BASE+0x001C+0x0040*voq_num */
union dsaf_voq_spup_idle {
	/* Define the struct bits */
	struct {
		unsigned int voq_spup0_idle:1;
		unsigned int voq_spup1_idle:1;
		unsigned int reserved_128:30;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};


/* Define the union dsaf_voq_bp_all_tiware_u */
/* DSAF_BASE0+VOQ_BASE+0x0034+0x0040*voq_num */
union dsaf_voq_bp_all_thrd1 {
	/* Define the struct bits */
	struct {
		unsigned int voq_bp_all_downthrd:10;
		unsigned int voq_bp_all_upthrd:10;
		unsigned int reserved_131:12;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_ctrl1 */
/* TBL_BASE+0x0000 */
union dsaf_tbl_ctrl1 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_vswitch_en:1;
		unsigned int tbl_ucast_bcast_en:1;
		unsigned int tbl_old_scan_en:1;
		unsigned int tbl_old_mask_en:1;
		unsigned int tcam_t_sel:2;
		unsigned int reserved_133:2;
		unsigned int xge0_lkup_en:1;
		unsigned int xge1_lkup_en:1;
		unsigned int xge2_lkup_en:1;
		unsigned int xge3_lkup_en:1;
		unsigned int xge4_lkup_en:1;
		unsigned int xge5_lkup_en:1;
		unsigned int ppe_lkup_en:1;
		unsigned int rocee_lkup_en:1;
		unsigned int reserved_132:16;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_int_msk1 */
/* TBL_BASE+0x0004 */
union dsaf_tbl_int_msk1 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_da0_mis_msk:1;
		unsigned int tbl_da1_mis_msk:1;
		unsigned int tbl_da2_mis_msk:1;
		unsigned int tbl_da3_mis_msk:1;
		unsigned int tbl_da4_mis_msk:1;
		unsigned int tbl_da5_mis_msk:1;
		unsigned int tbl_da6_mis_msk:1;
		unsigned int tbl_da7_mis_msk:1;
		unsigned int tbl_sa_mis_msk:1;
		unsigned int tbl_old_sech_end_msk:1;
		unsigned int lram_ecc_err1_msk:1;
		unsigned int lram_ecc_err2_msk:1;
		unsigned int tram_ecc_err1_msk:1;
		unsigned int tram_ecc_err2_msk:1;
		unsigned int tbl_ucast_bcast_xge0_msk:1;
		unsigned int tbl_ucast_bcast_xge1_msk:1;
		unsigned int tbl_ucast_bcast_xge2_msk:1;
		unsigned int tbl_ucast_bcast_xge3_msk:1;
		unsigned int tbl_ucast_bcast_xge4_msk:1;
		unsigned int tbl_ucast_bcast_xge5_msk:1;
		unsigned int tbl_ucast_bcast_ppe_msk:1;
		unsigned int tbl_ucast_bcast_rocee_msk:1;
		unsigned int reserved_134:10;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_int_src */
/* TBL_BASE+0x0008 */
union dsaf_tbl_int_src {
	/* Define the struct bits */
	struct {
		unsigned int tbl_da0_mis_src:1;
		unsigned int tbl_da1_mis_src:1;
		unsigned int tbl_da2_mis_src:1;
		unsigned int tbl_da3_mis_src:1;
		unsigned int tbl_da4_mis_src:1;
		unsigned int tbl_da5_mis_src:1;
		unsigned int tbl_da6_mis_src:1;
		unsigned int tbl_da7_mis_src:1;
		unsigned int tbl_sa_mis_src:1;
		unsigned int tbl_old_sech_end_src:1;
		unsigned int lram_ecc_err1_src:1;
		unsigned int lram_ecc_err2_src:1;
		unsigned int tram_ecc_err1_src:1;
		unsigned int tram_ecc_err2_src:1;
		unsigned int tbl_ucast_bcast_xge0_src:1;
		unsigned int tbl_ucast_bcast_xge1_src:1;
		unsigned int tbl_ucast_bcast_xge2_src:1;
		unsigned int tbl_ucast_bcast_xge3_src:1;
		unsigned int tbl_ucast_bcast_xge4_src:1;
		unsigned int tbl_ucast_bcast_xge5_src:1;
		unsigned int tbl_ucast_bcast_ppe_src:1;
		unsigned int tbl_ucast_bcast_rocee_src:1;
		unsigned int reserved_135:10;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_int_sts */
/* TBL_BASE+0x0100 */
union dsaf_tbl_int_sts {
	/* Define the struct bits */
	struct {
		unsigned int tbl_da0_mis_sts:1;
		unsigned int tbl_da1_mis_sts:1;
		unsigned int tbl_da2_mis_sts:1;
		unsigned int tbl_da3_mis_sts:1;
		unsigned int tbl_da4_mis_sts:1;
		unsigned int tbl_da5_mis_sts:1;
		unsigned int tbl_da6_mis_sts:1;
		unsigned int tbl_da7_mis_sts:1;
		unsigned int tbl_sa_mis_sts:1;
		unsigned int tbl_old_sech_end_sts:1;
		unsigned int lram_ecc_err1_sts:1;
		unsigned int lram_ecc_err2_sts:1;
		unsigned int tram_ecc_err1_sts:1;
		unsigned int tram_ecc_err2_sts:1;
		unsigned int tbl_ucast_bcast_xge0_sts:1;
		unsigned int tbl_ucast_bcast_xge1_sts:1;
		unsigned int tbl_ucast_bcast_xge2_sts:1;
		unsigned int tbl_ucast_bcast_xge3_sts:1;
		unsigned int tbl_ucast_bcast_xge4_sts:1;
		unsigned int tbl_ucast_bcast_xge5_sts:1;
		unsigned int tbl_ucast_bcast_ppe_sts:1;
		unsigned int tbl_ucast_bcast_rocee_sts:1;
		unsigned int reserved_136:10;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_tcam_addr */
/* TBL_BASE+0x000C */
union dsaf_tbl_tcam_addr {
	/* Define the struct bits */
	struct {
		unsigned int tbl_tcam_addr:9;
		unsigned int reserved_137:23;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_line_addr */
/* TBL_BASE+0x0010 */
union dsaf_tbl_line_addr {
	/* Define the struct bits */
	struct {
		unsigned int tbl_line_addr:15;
		unsigned int reserved_138:17;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_tcam_mcast_cfg_4 */

/* TBL_BASE+0x001C */
union dsaf_tbl_tcam_mcast_cfg_4 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_mcast_vm128_122:7;
		unsigned int tbl_mcast_item_vld:1;
		unsigned int tbl_mcast_old_en:1;
		unsigned int reserved_139:23;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_tcam_mcast_cfg_0 */

/* TBL_BASE+0x002C */
union dsaf_tbl_tcam_mcast_cfg_0 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_mcast_xge5_0:6;
		unsigned int tbl_mcast_vm25_0:26;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_tcam_ucast_cfg1 */

/* TBL_BASE+0x0030 */
union dsaf_tbl_tcam_ucast_cfg1 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_ucast_out_port:8;
		unsigned int tbl_ucast_dvc:1;
		unsigned int tbl_ucast_mac_discard:1;
		unsigned int tbl_ucast_item_vld:1;
		unsigned int tbl_ucast_old_en:1;
		unsigned int reserved_140:20;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_lin_cfg */
/* TBL_BASE+0x0034 */
union dsaf_tbl_lin_cfg {
	/* Define the struct bits */
	struct {
		unsigned int tbl_line_out_port:8;
		unsigned int tbl_line_dvc:1;
		unsigned int tbl_line_mac_discard:1;
		unsigned int reserved_141:22;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_da0_mis_info1 */
/* TBL_BASE+0x0058 */
union dsaf_tbl_da0_mis_info1 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_da0_mis_dah:28;
		unsigned int reserved_144:4;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_da0_mis_info0 */
/* TBL_BASE+0x005C */
union dsaf_tbl_da0_mis_info0 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_da0_mis_vlan:12;
		unsigned int tbl_da0_mis_dal:20;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_sa_mis_info2 */
/* TBL_BASE+0x0104 */
union dsaf_tbl_sa_mis_info2 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_sa_mis_qid:8;
		unsigned int reserved_152:24;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_sa_mis_info1 */
/* TBL_BASE+0x0098 */
union dsaf_tbl_sa_mis_info1 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_sa_mis_sah:28;
		unsigned int tbl_sa_mis_port:3;
		unsigned int reserved_153:1;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_sa_mis_info0 */
/* TBL_BASE+0x009C */
union dsaf_tbl_sa_mis_info0 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_sa_mis_vlan:12;
		unsigned int tbl_sa_mis_sal:20;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_pul */
/* TBL_BASE+0x00A0 */
union dsaf_tbl_pul {
	/* Define the struct bits */
	struct {
		unsigned int tbl_old_rslt_re:1;
		unsigned int tbl_mcast_vld:1;
		unsigned int tbl_tcam_data_vld:1;
		unsigned int tbl_ucast_vld:1;
		unsigned int tbl_line_vld:1;
		unsigned int tbl_tcam_load:1;
		unsigned int tbl_line_load:1;
		unsigned int reserved_154:25;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_old_rslt */
/* TBL_BASE+0x00A4 */
union dsaf_tbl_old_rslt {
	/* Define the struct bits */
	struct {
		unsigned int tbl_old_rslt:10;
		unsigned int reserved_155:22;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_dfx_ctrl */

/* TBL_BASE+0x00AC */
union dsaf_tbl_dfx_ctrl {
	/* Define the struct bits */
	struct {
		unsigned int line_lkup_num_en:1;
		unsigned int uc_lkup_num_en:1;
		unsigned int mc_lkup_num_en:1;
		unsigned int bc_lkup_num_en:1;
		unsigned int ram_err_inject_en:1;
		unsigned int reserved_156:27;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_dfx_stat */
/* TBL_BASE+0x00B0 */
union dsaf_tbl_dfx_stat {
	/* Define the struct bits */
	struct {
		unsigned int lram_ecc_err_addr:15;
		unsigned int reserved_158:1;
		unsigned int tram_ecc_err_addr:9;
		unsigned int reserved_157:7;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_tbl_ucast_bcast_mis_info_0 */
/* TBL_BASE+0x010C */
union dsaf_tbl_ucast_bcast_mis_info_0 {
	/* Define the struct bits */
	struct {
		unsigned int tbl_ucast_bcast_mis_vlan_0:12;
		unsigned int reserved_162:20;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_inode_fifo_wl1 */
/* DSAF_BASE0+FIFO_BASE+0x0000+0x0004*fifo_num */
union dsaf_inode_fifo_wl1 {
	/* Define the struct bits */
	struct {
		unsigned int inode_fifo_afull_on_th:6;
		unsigned int inode_fifo_alempt_th:6;
		unsigned int reserved_170:20;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_onode_fifo_wl1 */

/* DSAF_BASE0+FIFO_BASE+0x0020+0x0004*fifo_num */
union dsaf_onode_fifo_wl1 {
	/* Define the struct bits */
	struct {
		unsigned int onode_fifo_afull_on_th:6;
		unsigned int onode_fifo_alempt_th:6;
		unsigned int reserved_171:20;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xge_ge_work_mode */
/* DSAF_BASE0+FIFO_BASE+0x0040+0x0004*xge_num */
union dsaf_xge_ge_work_mode {
	/* Define the struct bits */
	struct {
		unsigned int xge_ge_work_mode:1;
		unsigned int xge_ge_loopback:1;
		unsigned int reserved_172:30;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xge_sds_vsemi_choose */
/* DSAF_BASE0+FIFO_BASE+0x0060 */
union dsaf_xge_sds_vsemi_choose {
	/* Define the struct bits */
	struct {
		unsigned int xge_sds_vsemi_choose:1;
		unsigned int reserved_173:31;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xge_app_rx_link_up */

/* DSAF_BASE0+FIFO_BASE+0x0080+0x0004*xge_num */
union dsaf_xge_app_rx_link_up {
	/* Define the struct bits */
	struct {
		unsigned int xge_app_rx_link_up:1;
		unsigned int reserved_174:31;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_netport_ctrl_sig */
/* DSAF_BASE0+FIFO_BASE+0x00A0+0x0004*xge_num */
union dsaf_netport_ctrl_sig {
	/* Define the struct bits */
	struct {
		unsigned int gmac_sync_ok:1;
		unsigned int gmac_led_activity:1;
		unsigned int xge_app_rx_pause:1;
		unsigned int xge_app_fc_vector:1;
		unsigned int ge_fifo_err_int:1;
		unsigned int reserved_175:27;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

/* Define the union dsaf_xge_ctrl_sig_cfg */
/* DSAF_BASE0+FIFO_BASE+0x00C0+0x0004*xge_num */
union dsaf_xge_ctrl_sig_cfg {
	/* Define the struct bits */
	struct {
		unsigned int fc_xge_tx_pause:1;
		unsigned int regs_xge_cnt_car:1;
		unsigned int reserved_176:29;
	} bits;

	/* Define an unsigned member */
	unsigned int u32;

};

#endif				/* _IWARE_DSAF_REG_DEFINE_H_ */


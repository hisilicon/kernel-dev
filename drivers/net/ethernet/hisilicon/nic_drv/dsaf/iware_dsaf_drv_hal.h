/*******************************************************************************

  Hisilicon DSAF driver
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

#ifndef _HRD_DSAF_DRV_HAL_H
#define _HRD_DSAF_DRV_HAL_H

#include "hrd_log.h"
#include "iware_dsaf_reg_define.h"

#define DSAF_ADDR_NUM 2


extern void dsaf_inode_init(u32 dsaf_id);
extern int dsaf_sbm_init(u32 dsaf_id);
extern void dsaf_voq_init(u32 dsaf_id);
extern void dsaf_tbl_init(u32 dsaf_id);
extern void dsaf_testinit(void);
extern void dsaf_tcam_mc_invld(u32 dsaf_id, u32 address);
extern void dsaf_port_work_rate_cfg(u32 dsaf_id, u32 port_num,
	enum dsaf_port_rate_mode xge_ge_work_mode);
extern void dsaf_int_handler(u32 dsaf_id, u32 irq_num);
extern void dsaf_comm_init(u32 dsaf_id, enum dsaf_mode dsaf_mode);
extern struct dsaf_cfg g_dsaf_cfg;
extern void dsaf_tcam_uc_cfg(u32 dsaf_id, u32 address,
	struct dsaf_tbl_tcam_data *ptbl_tcam_data,
	struct dsaf_tbl_tcam_ucast_cfg *ptbl_tcam_ucast);
extern void dsaf_tcam_uc_get(u32 dsaf_id, u32 address,
	struct dsaf_tbl_tcam_data *ptbl_tcam_data,
	struct dsaf_tbl_tcam_ucast_cfg *ptbl_tcam_ucast);
extern void dsaf_tcam_mc_cfg(u32 dsaf_id, u32 address,
	struct dsaf_tbl_tcam_data *ptbl_tcam_data,
	struct dsaf_tbl_tcam_mcast_cfg *ptbl_tcam_mcast);
extern void dsaf_tcam_mc_get(u32 dsaf_id, u32 address,
	struct dsaf_tbl_tcam_data *ptbl_tcam_data,
	struct dsaf_tbl_tcam_mcast_cfg *ptbl_tcam_mcast);
extern void dsaf_int_tbl_msk_get(u32 dsaf_id, union dsaf_tbl_int_msk1 *int_msk);
extern void dsaf_int_tbl_msk_cfg(u32 dsaf_id, union dsaf_tbl_int_msk1 int_msk);
extern void dsaf_mix_mode_cfg(u32 dsaf_id, u32 mode_cfg);
extern void dsaf_port_def_vlan_cfg(u32 dsaf_id, u32 port, u16 port_def_vlan);
extern void dsaf_port_def_vlan_get(u32 dsaf_id, u32 port, u16 *pport_def_vlan);
extern void dsaf_tbl_tcam_init(u32 dsaf_id);
extern void dsaf_tbl_line_init(u32 dsaf_id);

#define DSAF_MAX_TCAM_ENTRY_NUM 512

#define DSAF_INVALID_ENTRY_IDX 0xffff

struct dsaf_drv_tbl_tcam_key {
	union {
		struct {
			u8 mac_3;
			u8 mac_2;
			u8 mac_1;
			u8 mac_0;
		} bits;

		u32 val;
	} high;
	union {
		struct {
			u32 port:4; /* port id,
			dsaf-mode fixed 0, non-dsaf-mode port id*/
			u32 vlan:12; /* vlan id */
			u32 mac_5:8;
			u32 mac_4:8;
		} bits;

		u32 val;
	} low;
};

struct dsaf_drv_soft_mac_tbl {
	struct dsaf_drv_tbl_tcam_key tcam_key;
	u16 index; /*the entry's index in tcam tab*/
};

struct dsaf_drv_priv {
	/* soft tab Mac key, for hardware tab*/
	struct dsaf_drv_soft_mac_tbl *soft_mac_tbl;
};


#endif /* __HRD_DSAF_DRV_HAL_H__ */


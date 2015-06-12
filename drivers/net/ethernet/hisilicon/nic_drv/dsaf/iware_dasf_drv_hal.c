/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:64z4jYnYa5t1KtRL8a/vnMxg4uGttU/wzF06xcyNtiEfsIe4UpyXkUSy93j7U7XZDdqx2rNx
p+25Dla32ZW7osA9Q1ovzSUNJmwD2Lwb8CS3jj1e4NXnh+7DT2iIAuYHJTrgjUqp838S0X3Y
kLe48xckF9EzKOiitHAnfJ5lBdlF/ZeY6tsROnp6UhHx3W/UMCesB/RGEUj3pIdsfy1UhN36
+VNH9ewHukWy4eCW/RxY/Oc8U7wGt8c6bXt2U08wQCYaCFoDTT04CjbMzp1lTg==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/*******************************************************************************

  Hisilicon dsa fabric driver - support L2 switch
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

#include "iware_error.h"
#include "iware_log.h"
#include "iware_mac_main.h"
#include "iware_dsaf_drv_hw.h"

#include "iware_dsaf_main.h"
#include "iware_dsaf_drv_hal.h"
#include "hrd_crg_api.h"


unsigned long long g_dsaf_reg_addr[DSAF_ADDR_NUM] = {0, 0};

size_t g_dsaf_sizeof_priv = sizeof(struct dsaf_drv_priv);

/**
 * dsaf_init_hw - init dsa fabric hardware
 * @dsaf_dev: dsa fabric device struct pointer
 */
static int dsaf_init_hw(struct dsaf_device *dsaf_dev)
{
	int ret = 0;

	log_dbg(dsaf_dev->dev,
		"dsaf_init_hw begin dsaf_id=%d !\n", dsaf_dev->chip_id);

	(void)HRD_Dsaf_XbarSrst((u32)dsaf_dev->chip_id, 0);
	(void)HRD_Dsaf_NtSrst((u32)dsaf_dev->chip_id, 0);
	osal_mdelay(10);
	(void)HRD_Dsaf_XbarSrst((u32)dsaf_dev->chip_id, 1);
	(void)HRD_Dsaf_NtSrst((u32)dsaf_dev->chip_id, 1);

	/* map reg */
	g_dsaf_reg_addr[(u32)dsaf_dev->chip_id] = (u64)dsaf_dev->vaddr;
	log_dbg(dsaf_dev->dev,
				"dsaf_init_hw dsaf%d g_dsaf_reg_addr = 0x%llX !\n",
				dsaf_dev->chip_id,
				g_dsaf_reg_addr[(u32)dsaf_dev->chip_id]);

	if (dsaf_dev->dsaf_mode > DSAF_MODE_ENABLE)
		g_dsaf_cfg.dsaf_en = HRD_DSAF_NO_DSAF_MODE;
	else
		g_dsaf_cfg.dsaf_en = HRD_DSAF_MODE;

	dsaf_comm_init((u32)dsaf_dev->chip_id, dsaf_dev->dsaf_mode);
	log_dbg(dsaf_dev->dev,
				"dsaf_init_hw dsaf_comm_init dsaf%d !\n",
				dsaf_dev->chip_id);

	/*init XBAR_INODE*/
	dsaf_inode_init((u32)dsaf_dev->chip_id);
	log_dbg(dsaf_dev->dev,
				"dsaf_init_hw dsaf_inode_init dsaf%d !\n",
				dsaf_dev->chip_id);

	/*init SBM*/
	ret = dsaf_sbm_init((u32)dsaf_dev->chip_id);
	if(ret) {
		log_err(dsaf_dev->dev,
			"dsaf_init_hw dsaf_sbm_init dsaf_id=%d,ret=%d\n",
				dsaf_dev->chip_id, ret);
		return -ENOSYS;
	}

	/*init TBL*/
	dsaf_tbl_init((u32)dsaf_dev->chip_id);
	log_dbg(dsaf_dev->dev,
				"dsaf_init_hw dsaf_tbl_init dsaf%d !\n",
				dsaf_dev->chip_id);

	/*init VOQ*/
	dsaf_voq_init((u32)dsaf_dev->chip_id);
	log_dbg(dsaf_dev->dev,
				"dsaf_init_hw dsaf_voq_init dsaf%d !\n",
				dsaf_dev->chip_id);

	return 0;
}

/**
 * dsaf_remove_hw - uninit dsa fabric hardware
 * @dsaf_dev: dsa fabric device struct pointer
 */
static void dsaf_remove_hw(struct dsaf_device *dsaf_dev)
{
	/*reset*/
	(void)HRD_Dsaf_XbarSrst((u32)dsaf_dev->chip_id, 0);
	(void)HRD_Dsaf_NtSrst((u32)dsaf_dev->chip_id, 0);

	#if 0 /*if is CHIP need uninit tab entry*//*TBD*/
	dsaf_tbl_tcam_init((u32)dsaf_dev->chip_id);
	dsaf_tbl_line_init((u32)dsaf_dev->chip_id);
	#endif
}


/**
 * dsaf_init - init dsa fabric
 * @dsaf_dev: dsa fabric device struct pointer
 */
static int dsaf_init(struct dsaf_device *dsaf_dev)
{
	struct dsaf_drv_priv *priv =
	    (struct dsaf_drv_priv *)dsaf_dev_priv(dsaf_dev);
	u32 i = 0;
	int ret =0;

	ret = dsaf_init_hw(dsaf_dev);
	if(ret) {
		log_info(dsaf_dev->dev, "dsaf_init_hw fail, dsaf_id=%d,ret=%d\n",
			dsaf_dev->chip_id, ret);
		return -ENOSYS;
	}


	/* malloc mem for tcam mac key(vlan+mac) */
	priv->soft_mac_tbl
		= osal_vmalloc(sizeof(struct dsaf_drv_soft_mac_tbl)
		  * DSAF_MAX_TCAM_ENTRY_NUM);
	if (NULL == priv->soft_mac_tbl) {
		log_err(dsaf_dev->dev,
				"dsaf_init vmalloc faild, dsaf%d !\n",
				dsaf_dev->chip_id);
		return HRD_COMMON_ERR_GET_MEN_RES_FAIL;
	}

	log_info(dsaf_dev->dev, "soft_mac_tbl=%#llx!\n",
		(u64)priv->soft_mac_tbl);

	for (i = 0; i < DSAF_MAX_TCAM_ENTRY_NUM; i++)
		/*all entry invall */
		(priv->soft_mac_tbl+i)->index = DSAF_INVALID_ENTRY_IDX;

	return 0;
}

/**
 * dsaf_free - free dsa fabric
 * @dsaf_dev: dsa fabric device struct pointer
 */
static int dsaf_free(struct dsaf_device *dsaf_dev)
{
	struct dsaf_drv_priv *priv =
	    (struct dsaf_drv_priv *)dsaf_dev_priv(dsaf_dev);

	dsaf_remove_hw(dsaf_dev);

	log_info(dsaf_dev->dev, "soft_mac_tbl=%#llx!\n",
		(u64)priv->soft_mac_tbl);

	/* free all mac mem */
	osal_vfree(priv->soft_mac_tbl);

	return 0;
}

/**
 * dsaf_find_soft_mac_entry - find dsa fabric soft entry
 * @dsaf_dev: dsa fabric device struct pointer
 * @mac_key: mac entry struct pointer
 */
static u16 dsaf_find_soft_mac_entry(struct dsaf_device *dsaf_dev,
	struct dsaf_drv_tbl_tcam_key *mac_key)
{
	struct dsaf_drv_priv *priv =
	    (struct dsaf_drv_priv *)dsaf_dev_priv(dsaf_dev);
	struct dsaf_drv_soft_mac_tbl *soft_mac_entry = NULL;
	u32 i = 0;

	soft_mac_entry = priv->soft_mac_tbl;
	for (i = 0; i < DSAF_MAX_TCAM_ENTRY_NUM; i++) {
		/* invall tab entry */
		if ((DSAF_INVALID_ENTRY_IDX != soft_mac_entry->index)
			&& (soft_mac_entry->tcam_key.high.val
				== mac_key->high.val)
			&& (soft_mac_entry->tcam_key.low.val
				== mac_key->low.val))
			/* return find result --soft index */
			return soft_mac_entry->index;

		soft_mac_entry++;
	}
	return DSAF_INVALID_ENTRY_IDX;
}

/**
 * dsaf_find_empty_mac_entry - search dsa fabric soft empty-entry
 * @dsaf_dev: dsa fabric device struct pointer
 */
static u16 dsaf_find_empty_mac_entry(struct dsaf_device *dsaf_dev)
{
	struct dsaf_drv_priv *priv =
	    (struct dsaf_drv_priv *)dsaf_dev_priv(dsaf_dev);
	struct dsaf_drv_soft_mac_tbl *soft_mac_entry = NULL;
	u32 i = 0;

	soft_mac_entry = priv->soft_mac_tbl;
	for (i = 0; i < DSAF_MAX_TCAM_ENTRY_NUM; i++) {
		/* inv all entry */
		if (DSAF_INVALID_ENTRY_IDX == soft_mac_entry->index)
			/* return find result --soft index */
			/*soft_mac_entry->index = i;*//*TBD*/
			return i;

		soft_mac_entry++;
	}
	return DSAF_INVALID_ENTRY_IDX;
}

/**
 * dsaf_set_mac_key - set mac key
 * @dsaf_dev: dsa fabric device struct pointer
 * @mac_key: tcam key pointer
 * @vlan_id: vlan id
 * @in_port_num: input port num
 * @addr: mac addr
 */
static inline void dsaf_set_mac_key(struct dsaf_device *dsaf_dev,
	struct dsaf_drv_tbl_tcam_key *mac_key, u16 vlan_id, u8 in_port_num,
	u8 *addr)
{
	u8 port;

	if (dsaf_dev->dsaf_mode <= DSAF_MODE_ENABLE)
		/*DSAF mode : in port id fixed 0*/
		port = 0;
	else
		/*non-dsaf mode*/
		port = in_port_num;

	mac_key->high.bits.mac_0 = addr[0];
	mac_key->high.bits.mac_1 = addr[1];
	mac_key->high.bits.mac_2 = addr[2];
	mac_key->high.bits.mac_3 = addr[3];
	mac_key->low.bits.mac_4 = addr[4];
	mac_key->low.bits.mac_5 = addr[5];
	mac_key->low.bits.vlan = vlan_id;
	mac_key->low.bits.port = port;

}

/**
 * dsaf_set_mac_uc_entry - set mac uc-entry
 * @dsaf_dev: dsa fabric device struct pointer
 * @mac_entry: uc-mac entry
 */
static int dsaf_set_mac_uc_entry(struct dsaf_device *dsaf_dev,
	struct dsaf_drv_mac_single_dest_entry *mac_entry)
{
	u16 entry_index = DSAF_INVALID_ENTRY_IDX;
	struct dsaf_drv_tbl_tcam_key mac_key;
	struct dsaf_tbl_tcam_ucast_cfg mac_data;
	struct dsaf_drv_priv *priv =
	    (struct dsaf_drv_priv *)dsaf_dev_priv(dsaf_dev);
	struct dsaf_drv_soft_mac_tbl *soft_mac_entry = priv->soft_mac_tbl;

	/* mac addr check */
	if (MAC_IS_ALL_ZEROS(mac_entry->addr)
		|| MAC_IS_BROADCAST(mac_entry->addr)
		|| MAC_IS_MULTICAST(mac_entry->addr)) {
		log_err(dsaf_dev->dev,
			"dsaf_set_mac_uc_entry faild, dsaf%d Mac address(%02x:%02x:%02x)\n",
			dsaf_dev->chip_id, mac_entry->addr[0],
			mac_entry->addr[1], mac_entry->addr[2]);
		log_err(dsaf_dev->dev, "%02x:%02x:%02x error!\n",
			mac_entry->addr[3], mac_entry->addr[4],
			mac_entry->addr[5]);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	/* config key */
	dsaf_set_mac_key(dsaf_dev, &mac_key, mac_entry->in_vlan_id,
				mac_entry->in_port_num, mac_entry->addr);

	/* entry ie exist? */
	entry_index = dsaf_find_soft_mac_entry(dsaf_dev, &mac_key);
	if (DSAF_INVALID_ENTRY_IDX == entry_index) {
		/*if has not inv entry,find a empty entry */
		entry_index = dsaf_find_empty_mac_entry(dsaf_dev);
		if (DSAF_INVALID_ENTRY_IDX == entry_index) {
			/* has not empty,return error */
			log_err(dsaf_dev->dev,
				"dsaf_set_mac_uc_entry faild, dsaf%d Mac key(%#x:%#x)\n",
				dsaf_dev->chip_id,
				mac_key.high.val, mac_key.low.val);
			return HRD_COMMON_ERR_NOT_ENOUGH_RES;
		}
	}

	log_dbg(dsaf_dev->dev,
		"dsaf_set_mac_uc_entry, dsaf%d Mac key(%#x:%#x) entry_index%d\n",
		dsaf_dev->chip_id, mac_key.high.val,
		mac_key.low.val, entry_index);

	/* config hardware entry */
	mac_data.tbl_ucast_item_vld = 1;
	mac_data.tbl_ucast_mac_discard = 0;
	mac_data.tbl_ucast_old_en = 0;
	mac_data.tbl_ucast_out_port = mac_entry->port_num;
	/*mac_data.tbl_ucast_tag
	= (mac_entry->qos << 13 | mac_entry->out_vlan_id);*//*TBD*/
	/* qos+cfi+vlanid */
	/*mac_data.tbl_ucast_tag_add = mac_entry->vlan_mode;*/
	dsaf_tcam_uc_cfg((u32)dsaf_dev->chip_id, entry_index,
		(struct dsaf_tbl_tcam_data *)(&mac_key), &mac_data);

	/* config software entry */
	soft_mac_entry += entry_index;
	soft_mac_entry->index = entry_index;
	soft_mac_entry->tcam_key.high.val = mac_key.high.val;
	soft_mac_entry->tcam_key.low.val = mac_key.low.val;

	return 0;
}

/**
 * dsaf_set_mac_mc_entry - set mac mc-entry
 * @dsaf_dev: dsa fabric device struct pointer
 * @mac_entry: mc-mac entry
 */
static int dsaf_set_mac_mc_entry(
	struct dsaf_device *dsaf_dev,
	struct dsaf_drv_mac_multi_dest_entry *mac_entry)
{
	u16 entry_index = DSAF_INVALID_ENTRY_IDX;
	struct dsaf_drv_tbl_tcam_key mac_key;
	struct dsaf_tbl_tcam_mcast_cfg mac_data;
	struct dsaf_drv_priv *priv =
	    (struct dsaf_drv_priv *)dsaf_dev_priv(dsaf_dev);
	struct dsaf_drv_soft_mac_tbl *soft_mac_entry = priv->soft_mac_tbl;
	struct dsaf_drv_tbl_tcam_key tmp_mac_key;

	/* mac addr check */
	if (MAC_IS_ALL_ZEROS(mac_entry->addr)) {
		log_err(dsaf_dev->dev,
			"dsaf_set_mac_uc_entry faild, dsaf%d Mac address(%02x:%02x:%02x)\n",
			dsaf_dev->chip_id, mac_entry->addr[0],
			mac_entry->addr[1], mac_entry->addr[2]);
		log_err(dsaf_dev->dev, "%02x:%02x:%02x error!\n",
			mac_entry->addr[3],
			mac_entry->addr[4], mac_entry->addr[5]);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	/*config key */
	dsaf_set_mac_key(dsaf_dev, &mac_key,
			  mac_entry->in_vlan_id,
			  mac_entry->in_port_num, mac_entry->addr);

	/* entry ie exist? */
	entry_index = dsaf_find_soft_mac_entry(dsaf_dev, &mac_key);
	if (DSAF_INVALID_ENTRY_IDX == entry_index) {
		/*if hasnot, find enpty entry*/
		entry_index = dsaf_find_empty_mac_entry(dsaf_dev);
		if (DSAF_INVALID_ENTRY_IDX == entry_index) {
			/*if hasnot empty, error*/
			log_err(dsaf_dev->dev,
				"set_mac_uc_entry faild, dsaf%d Mac key(%#x:%#x)\n",
				dsaf_dev->chip_id,
				mac_key.high.val, mac_key.low.val);
			return HRD_COMMON_ERR_NOT_ENOUGH_RES;
		}

		/* config hardware entry */
		mac_data.tbl_mcast_old_en = 0;
		mac_data.tbl_mcast_item_vld = 1;
		mac_data.tbl_mcast_vm128_120 = 0;
		mac_data.tbl_mcast_vm119_110 = 0;
		mac_data.tbl_mcast_vm109_100 = 0;
		mac_data.tbl_mcast_vm99_90 = 0;
		mac_data.tbl_mcast_vm89_80 = 0;
		mac_data.tbl_mcast_vm79_70 = 0;
		mac_data.tbl_mcast_vm69_60 = 0;
		mac_data.tbl_mcast_vm59_50 = 0;
		mac_data.tbl_mcast_vm49_40 = 0;
		mac_data.tbl_mcast_vm39_30 = 0;
		mac_data.tbl_mcast_vm29_20 = 0;
		mac_data.tbl_mcast_vm19_10 = 0;
		mac_data.tbl_mcast_vm9_0 = 0;
		mac_data.tbl_mcast_xge5_0 =
			mac_entry->port_mask[0] & 0x3F; /*get GE0->GE5*/
	} else {
		/* config hardware entry */
		dsaf_tcam_mc_get(dsaf_dev->chip_id,	entry_index,
		(struct dsaf_tbl_tcam_data *)(&tmp_mac_key), &mac_data);

		mac_data.tbl_mcast_old_en = 0;
		mac_data.tbl_mcast_item_vld = 1;
		mac_data.tbl_mcast_xge5_0
			= mac_entry->port_mask[0] & 0x3F; /*get GE0->GE5*/
	}

	log_dbg(dsaf_dev->dev,
		"set_mac_uc_entry, dsaf%d key(%#x:%#x) entry_index%d\n",
		dsaf_dev->chip_id, mac_key.high.val,
		mac_key.low.val, entry_index);

	dsaf_tcam_mc_cfg((u32)dsaf_dev->chip_id, entry_index,
		(struct dsaf_tbl_tcam_data *)(&mac_key), &mac_data);

	/* config software entry */
	soft_mac_entry += entry_index;
	soft_mac_entry->index = entry_index;
	soft_mac_entry->tcam_key.high.val = mac_key.high.val;
	soft_mac_entry->tcam_key.low.val = mac_key.low.val;

	return 0;
}

/**
 * dsaf_add_mac_mc_port - add mac mc-port
 * @dsaf_dev: dsa fabric device struct pointer
 * @mac_entry: mc-mac entry
 */
static int dsaf_add_mac_mc_port(struct dsaf_device *dsaf_dev,
	struct dsaf_drv_mac_single_dest_entry *mac_entry)
{
	u16 entry_index = DSAF_INVALID_ENTRY_IDX;
	u32 vmid;
	struct dsaf_drv_tbl_tcam_key mac_key;
	struct dsaf_tbl_tcam_mcast_cfg mac_data;
	struct dsaf_drv_priv *priv =
	    (struct dsaf_drv_priv *)dsaf_dev_priv(dsaf_dev);
	struct dsaf_drv_soft_mac_tbl *soft_mac_entry = priv->soft_mac_tbl;
	struct dsaf_drv_tbl_tcam_key tmp_mac_key;

	/*chechk mac addr */
	if (MAC_IS_ALL_ZEROS(mac_entry->addr)) {
		log_err(dsaf_dev->dev,
			"set_entry faild,addr %02x:%02x:%02x:%02x:%02x:%02x!\n",
			mac_entry->addr[0], mac_entry->addr[1],
			mac_entry->addr[2], mac_entry->addr[3],
			mac_entry->addr[4], mac_entry->addr[5]);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	/*config key */
	dsaf_set_mac_key(dsaf_dev, &mac_key, mac_entry->in_vlan_id,
		mac_entry->in_port_num, mac_entry->addr);

	memset(&mac_data, 0, sizeof(struct dsaf_tbl_tcam_mcast_cfg));

	/*check exist? */
	entry_index = dsaf_find_soft_mac_entry(dsaf_dev, &mac_key);
	if (DSAF_INVALID_ENTRY_IDX == entry_index) {
		/*if hasnot , find a empty*/
		entry_index = dsaf_find_empty_mac_entry(dsaf_dev);
		if (DSAF_INVALID_ENTRY_IDX == entry_index) {
			/*if hasnot empty, error*/
			log_err(dsaf_dev->dev,
				"dsaf_set_mac_uc_entry faild, dsaf%d Mac key(%#x:%#x)\n",
				dsaf_dev->chip_id, mac_key.high.val,
				mac_key.low.val);
			return HRD_COMMON_ERR_NOT_ENOUGH_RES;
		}

		/* config hardware entry */
		if (mac_entry->port_num <= 5)
			mac_data.tbl_mcast_xge5_0 =
				0x1 << mac_entry->port_num; /*get GE0->GE5*/
		else {
			vmid = mac_entry->port_num-DSAF_BASE_INNER_PORT_NUM;
			switch (vmid/10) {
			case 0:
				mac_data.tbl_mcast_vm9_0 = 0x1<<vmid;
				break;
			case 1:
				mac_data.tbl_mcast_vm19_10 = 0x1<<(vmid-10);
				break;
			case 2:
				mac_data.tbl_mcast_vm29_20 = 0x1<<(vmid-20);
				break;
			case 3:
				mac_data.tbl_mcast_vm39_30 = 0x1<<(vmid-30);
				break;
			case 4:
				mac_data.tbl_mcast_vm49_40 = 0x1<<(vmid-40);
				break;
			case 5:
				mac_data.tbl_mcast_vm59_50 = 0x1<<(vmid-50);
				break;
			case 6:
				mac_data.tbl_mcast_vm69_60 = 0x1<<(vmid-60);
				break;
			case 7:
				mac_data.tbl_mcast_vm79_70 = 0x1<<(vmid-70);
				break;
			case 8:
				mac_data.tbl_mcast_vm89_80 = 0x1<<(vmid-80);
				break;
			case 9:
				mac_data.tbl_mcast_vm99_90 = 0x1<<(vmid-90);
				break;
			case 10:
				mac_data.tbl_mcast_vm109_100 = 0x1<<(vmid-100);
				break;
			case 11:
				mac_data.tbl_mcast_vm119_110 = 0x1<<(vmid-110);
				break;
			case 12:
				mac_data.tbl_mcast_vm128_120 = 0x1<<(vmid-120);
				break;
			default:
				log_err(dsaf_dev->dev,
					"input port_num(%d) error, dsaf%d Mac key(%#x:%#x)\n",
					dsaf_dev->chip_id, mac_entry->port_num,
					mac_key.high.val,
					mac_key.low.val);
				return HRD_COMMON_ERR_INPUT_INVALID;
			}
		}
		mac_data.tbl_mcast_old_en = 0;
		mac_data.tbl_mcast_item_vld = 1;
	} else {
		/*if exist, add in */
		dsaf_tcam_mc_get(dsaf_dev->chip_id, entry_index,
			(struct dsaf_tbl_tcam_data *)(&tmp_mac_key), &mac_data);

		/*config hardware entry */
		if (mac_entry->port_num <= 5)
			mac_data.tbl_mcast_xge5_0 |=
				0x1 << mac_entry->port_num; /*get GE0->GE5*/
		else {
			vmid = mac_entry->port_num - DSAF_BASE_INNER_PORT_NUM;
			switch (vmid/10) {
			case 0:
				mac_data.tbl_mcast_vm9_0 |= 0x1<<vmid;
				break;
			case 1:
				mac_data.tbl_mcast_vm19_10 |= 0x1<<(vmid-10);
				break;
			case 2:
				mac_data.tbl_mcast_vm29_20 |= 0x1<<(vmid-20);
				break;
			case 3:
				mac_data.tbl_mcast_vm39_30 |= 0x1<<(vmid-30);
				break;
			case 4:
				mac_data.tbl_mcast_vm49_40 |= 0x1<<(vmid-40);
				break;
			case 5:
				mac_data.tbl_mcast_vm59_50 |= 0x1<<(vmid-50);
				break;
			case 6:
				mac_data.tbl_mcast_vm69_60 |= 0x1<<(vmid-60);
				break;
			case 7:
				mac_data.tbl_mcast_vm79_70 |= 0x1<<(vmid-70);
				break;
			case 8:
				mac_data.tbl_mcast_vm89_80 |= 0x1<<(vmid-80);
				break;
			case 9:
				mac_data.tbl_mcast_vm99_90 |= 0x1<<(vmid-90);
				break;
			case 10:
				mac_data.tbl_mcast_vm109_100 |= 0x1<<(vmid-100);
				break;
			case 11:
				mac_data.tbl_mcast_vm119_110 |= 0x1<<(vmid-110);
				break;
			case 12:
				mac_data.tbl_mcast_vm128_120 |= 0x1<<(vmid-120);
				break;
			default:
				log_err(dsaf_dev->dev,
					"input port_num(%d) error, dsaf%d Mac key(%#x:%#x)\n",
					dsaf_dev->chip_id, mac_entry->port_num,
					mac_key.high.val, mac_key.low.val);
				return HRD_COMMON_ERR_INPUT_INVALID;
			}
		}
		mac_data.tbl_mcast_old_en = 0;
		mac_data.tbl_mcast_item_vld = 1;
	}

	log_dbg(dsaf_dev->dev,
		"dsaf_set_mac_uc_entry, dsaf%d Mac key(%#x:%#x) entry_index%d\n",
		dsaf_dev->chip_id, mac_key.high.val,
		mac_key.low.val, entry_index);

	dsaf_tcam_mc_cfg((u32)dsaf_dev->chip_id, entry_index,
		(struct dsaf_tbl_tcam_data *)(&mac_key), &mac_data);

	/*config software entry */
	soft_mac_entry += entry_index;
	soft_mac_entry->index = entry_index;
	soft_mac_entry->tcam_key.high.val = mac_key.high.val;
	soft_mac_entry->tcam_key.low.val = mac_key.low.val;

	return 0;
}

/**
 * dsaf_del_mac_entry - del mac mc-port
 * @dsaf_dev: dsa fabric device struct pointer
 * @vlan_id: vlian id
 * @in_port_num: input port num
 * @addr : mac addr
 */
static int dsaf_del_mac_entry(struct dsaf_device *dsaf_dev, u16 vlan_id,
			u8 in_port_num, u8 *addr)
{
	u16 entry_index = DSAF_INVALID_ENTRY_IDX;
	struct dsaf_drv_tbl_tcam_key mac_key;
	struct dsaf_drv_priv *priv =
	    (struct dsaf_drv_priv *)dsaf_dev_priv(dsaf_dev);
	struct dsaf_drv_soft_mac_tbl *soft_mac_entry = priv->soft_mac_tbl;

	/*check mac addr */
	if (MAC_IS_ALL_ZEROS(addr) || MAC_IS_BROADCAST(addr)) {
		log_err(dsaf_dev->dev,
			"del_entry faild,addr %02x:%02x:%02x:%02x:%02x:%02x!\n",
			addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	/*config key */
	dsaf_set_mac_key(dsaf_dev, &mac_key, vlan_id, in_port_num, addr);

	/*exist ?*/
	entry_index = dsaf_find_soft_mac_entry(dsaf_dev, &mac_key);
	if (DSAF_INVALID_ENTRY_IDX == entry_index) {
		/*not exist, error */
		log_err(dsaf_dev->dev,
			"del_mac_entry faild, dsaf%d Mac key(%#x:%#x)\n",
			dsaf_dev->chip_id,
			mac_key.high.val, mac_key.low.val);
		return HRD_COMMON_ERR_RES_NOT_EXIST;
	}
	log_dbg(dsaf_dev->dev,
		"del_mac_entry, dsaf%d Mac key(%#x:%#x) entry_index%d\n",
		dsaf_dev->chip_id, mac_key.high.val,
		mac_key.low.val, entry_index);

	/*do del opt*/
	dsaf_tcam_mc_invld(dsaf_dev->chip_id, entry_index);

	/*del soft emtry */
	soft_mac_entry += entry_index;
	soft_mac_entry->index = DSAF_INVALID_ENTRY_IDX;

	return 0;
}

/**
 * dsaf_del_mac_mc_port - del mac mc- port
 * @dsaf_dev: dsa fabric device struct pointer
 * @mac_entry: mac entry
 */
static int dsaf_del_mac_mc_port(struct dsaf_device *dsaf_dev,
	struct dsaf_drv_mac_single_dest_entry *mac_entry)
{
	u16 entry_index = DSAF_INVALID_ENTRY_IDX;
	struct dsaf_drv_tbl_tcam_key mac_key;
	struct dsaf_drv_priv *priv =
	    (struct dsaf_drv_priv *)dsaf_dev_priv(dsaf_dev);
	struct dsaf_drv_soft_mac_tbl *soft_mac_entry = priv->soft_mac_tbl;
	u16 vlan_id;
	u8 in_port_num;
	u32 vmid;
	struct dsaf_tbl_tcam_mcast_cfg mac_data;
	struct dsaf_drv_tbl_tcam_key tmp_mac_key;

	if (NULL == (void *)mac_entry) {
		log_err(dsaf_dev->dev,
			"dsaf_del_mac_mc_port mac_entry is NULL\n");
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	/*get key info*/
	vlan_id = mac_entry->in_vlan_id;
	in_port_num = mac_entry->in_port_num;
#if 0 /*TBD*/
	/*the max ge id is 5*/
	if (in_port_num > 5) {
		log_err(dsaf_dev->dev,
		"dsaf_del_mac_mc_port in_port_num is %d\n",
		in_port_num);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}
#endif
	/*check mac addr */
	if (MAC_IS_ALL_ZEROS(mac_entry->addr)) {
		log_err(dsaf_dev->dev,
			"del_port faild, addr %02x:%02x:%02x:%02x:%02x:%02x!\n",
 			mac_entry->addr[0], mac_entry->addr[1],
 			mac_entry->addr[2], mac_entry->addr[3],
 			mac_entry->addr[4], mac_entry->addr[5]);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	/*config key */
	dsaf_set_mac_key(dsaf_dev, &mac_key, vlan_id, in_port_num,
		mac_entry->addr);

	/*check is exist? */
	entry_index = dsaf_find_soft_mac_entry(dsaf_dev, &mac_key);
	if (DSAF_INVALID_ENTRY_IDX == entry_index) {
		/*find none */
		log_err(dsaf_dev->dev,
			"find_soft_mac_entry faild, dsaf%d Mac key(%#x:%#x)\n",
			dsaf_dev->chip_id, mac_key.high.val, mac_key.low.val);
		return HRD_COMMON_ERR_RES_NOT_EXIST;
	}

	log_dbg(dsaf_dev->dev,
		"del_mac_mc_port, dsaf%d key(%#x:%#x) index%d\n",
		dsaf_dev->chip_id, mac_key.high.val,
		mac_key.low.val, entry_index);

	/*read entry*/
	dsaf_tcam_mc_get(dsaf_dev->chip_id, entry_index,
		(struct dsaf_tbl_tcam_data *)(&tmp_mac_key), &mac_data);

	/*del the port*/
	if (mac_entry->port_num <= 5)
		mac_data.tbl_mcast_xge5_0 &= ~((0x1UL << mac_entry->port_num));
	else {
		vmid = mac_entry->port_num-DSAF_BASE_INNER_PORT_NUM;
		switch (vmid/10) {
		case 0:
			mac_data.tbl_mcast_vm9_0 &= ~(0x1UL<<vmid);
			break;
		case 1:
			mac_data.tbl_mcast_vm19_10 &= ~(0x1UL<<(vmid-10));
			break;
		case 2:
			mac_data.tbl_mcast_vm29_20 &= ~(0x1UL<<(vmid-20));
			break;
		case 3:
			mac_data.tbl_mcast_vm39_30 &= ~(0x1UL<<(vmid-30));
			break;
		case 4:
			mac_data.tbl_mcast_vm49_40 &= ~(0x1UL<<(vmid-40));
			break;
		case 5:
			mac_data.tbl_mcast_vm59_50 &= ~(0x1UL<<(vmid-50));
			break;
		case 6:
			mac_data.tbl_mcast_vm69_60 &= ~(0x1UL<<(vmid-60));
			break;
		case 7:
			mac_data.tbl_mcast_vm79_70 &= ~(0x1UL<<(vmid-70));
			break;
		case 8:
			mac_data.tbl_mcast_vm89_80 &= ~(0x1UL<<(vmid-80));
			break;
		case 9:
			mac_data.tbl_mcast_vm99_90 &= ~(0x1UL<<(vmid-90));
			break;
		case 10:
			mac_data.tbl_mcast_vm109_100 &= ~(0x1UL<<(vmid-100));
			break;
		case 11:
			mac_data.tbl_mcast_vm119_110 &= ~(0x1UL<<(vmid-110));
			break;
		case 12:
			mac_data.tbl_mcast_vm128_120 &= ~(0x1UL<<(vmid-120));
			break;
		default:
			log_err(dsaf_dev->dev,
				"input port(%d) error, dsaf%d key(%#x:%#x)\n",
				dsaf_dev->chip_id, mac_entry->port_num,
				mac_key.high.val, mac_key.low.val);
			return HRD_COMMON_ERR_INPUT_INVALID;
		}
	}

	/*check non port, do del entry */
	if ((0 == mac_data.tbl_mcast_xge5_0)
		&& (0 == mac_data.tbl_mcast_vm9_0)
		&& (0 == mac_data.tbl_mcast_vm19_10)
		&& (0 == mac_data.tbl_mcast_vm29_20)
		&& (0 == mac_data.tbl_mcast_vm39_30)
		&& (0 == mac_data.tbl_mcast_vm49_40)
		&& (0 == mac_data.tbl_mcast_vm59_50)
		&& (0 == mac_data.tbl_mcast_vm69_60)
		&& (0 == mac_data.tbl_mcast_vm79_70)
		&& (0 == mac_data.tbl_mcast_vm89_80)
		&& (0 == mac_data.tbl_mcast_vm99_90)
		&& (0 == mac_data.tbl_mcast_vm109_100)
		&& (0 == mac_data.tbl_mcast_vm119_110)
		&& (0 == mac_data.tbl_mcast_vm128_120)) {
		dsaf_tcam_mc_invld(dsaf_dev->chip_id, entry_index);

		/* del soft entry */
		soft_mac_entry += entry_index;
		soft_mac_entry->index = DSAF_INVALID_ENTRY_IDX;
	}
	/*not zer£¬just del port£¬updata*/
	else {
		dsaf_tcam_mc_cfg((u32)dsaf_dev->chip_id, entry_index,
			(struct dsaf_tbl_tcam_data *)(&mac_key), &mac_data);

	}

	return 0;
}


/**
 * dsaf_get_mac_uc_entry - get mac uc entry
 * @dsaf_dev: dsa fabric device struct pointer
 * @mac_entry: mac entry
 */
static int dsaf_get_mac_uc_entry(struct dsaf_device *dsaf_dev,
	struct dsaf_drv_mac_single_dest_entry *mac_entry)
{
	u16 entry_index = DSAF_INVALID_ENTRY_IDX;
	struct dsaf_drv_tbl_tcam_key mac_key;

	struct dsaf_tbl_tcam_ucast_cfg mac_data;

	/* check macaddr */
	if (MAC_IS_ALL_ZEROS(mac_entry->addr)
		|| MAC_IS_BROADCAST(mac_entry->addr)) {
		log_err(dsaf_dev->dev,
			"get_entry failed,addr %02x:%02x:%02x:%02x:%02x:%02x\n",
			mac_entry->addr[0], mac_entry->addr[1],
			mac_entry->addr[2], mac_entry->addr[3],
			mac_entry->addr[4], mac_entry->addr[5]);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	/*config key */
	dsaf_set_mac_key(dsaf_dev, &mac_key, mac_entry->in_vlan_id,
	mac_entry->in_port_num, mac_entry->addr);

	/*check exist? */
	entry_index = dsaf_find_soft_mac_entry(dsaf_dev, &mac_key);
	if (DSAF_INVALID_ENTRY_IDX == entry_index) {
		/*find none, error */
		log_err(dsaf_dev->dev,
			"dsaf_get_mac_uc_entry faild, dsaf%d Mac key(%#x:%#x)\n",
			dsaf_dev->chip_id,
			mac_key.high.val, mac_key.low.val);
		return HRD_COMMON_ERR_RES_NOT_EXIST;
	}
	log_dbg(dsaf_dev->dev,
		"dsaf_get_mac_uc_entry, dsaf%d Mac key(%#x:%#x) entry_index%d\n",
		dsaf_dev->chip_id, mac_key.high.val,
		mac_key.low.val, entry_index);

	/*read entry*/
	dsaf_tcam_uc_get((u32)dsaf_dev->chip_id, entry_index,
			(struct dsaf_tbl_tcam_data *)&mac_key, &mac_data);
	/*mac_entry->out_vlan_id = mac_data.tbl_ucast_tag & 0xfff;
	mac_entry->qos = (mac_data.tbl_ucast_tag & 0xe000) >> 13;
	mac_entry->vlan_mode =
	    (enum dsaf_vlan_mode)mac_data.tbl_ucast_tag_add;*//*TBD*/
	mac_entry->port_num = mac_data.tbl_ucast_out_port;

	return 0;
}

/**
 * dsaf_get_mac_mc_entry - get mac mc entry
 * @dsaf_dev: dsa fabric device struct pointer
 * @mac_entry: mac entry
 */
static int dsaf_get_mac_mc_entry(
	struct dsaf_device *dsaf_dev,
	struct dsaf_drv_mac_multi_dest_entry *mac_entry)
{
	u16 entry_index = DSAF_INVALID_ENTRY_IDX;
	struct dsaf_drv_tbl_tcam_key mac_key;

	struct dsaf_tbl_tcam_mcast_cfg mac_data;

	/*check mac addr */
	if (MAC_IS_ALL_ZEROS(mac_entry->addr)
		|| MAC_IS_BROADCAST(mac_entry->addr)) {
		log_err(dsaf_dev->dev,
			"get_entry faild,addr %02x:%02x:%02x:%02x:%02x:%02x\n",
			mac_entry->addr[0], mac_entry->addr[1],
			mac_entry->addr[2], mac_entry->addr[3],
			mac_entry->addr[4], mac_entry->addr[5]);
		return HRD_COMMON_ERR_INPUT_INVALID;
	}

	/*config key */
	dsaf_set_mac_key(dsaf_dev, &mac_key, mac_entry->in_vlan_id,
		mac_entry->in_port_num, mac_entry->addr);

	/*check exist? */
	entry_index = dsaf_find_soft_mac_entry(dsaf_dev, &mac_key);
	if (DSAF_INVALID_ENTRY_IDX == entry_index) {
		/* find none, error */
		log_err(dsaf_dev->dev,
			"get_mac_uc_entry faild, dsaf%d Mac key(%#x:%#x)\n",
			dsaf_dev->chip_id, mac_key.high.val,
			mac_key.low.val);
		return HRD_COMMON_ERR_RES_NOT_EXIST;
	}
	log_dbg(dsaf_dev->dev,
		"get_mac_uc_entry, dsaf%d Mac key(%#x:%#x) entry_index%d\n",
		dsaf_dev->chip_id, mac_key.high.val,
		mac_key.low.val, entry_index);

	/*read entry */
	dsaf_tcam_mc_get(dsaf_dev->chip_id, entry_index,
		(struct dsaf_tbl_tcam_data *)&mac_key, &mac_data);

	mac_entry->port_mask[0] = mac_data.tbl_mcast_xge5_0;
	return 0;
}

/**
 * dsaf_get_mac_entry_by_index - get mac entry by tab index
 * @dsaf_dev: dsa fabric device struct pointer
 * @entry_index: tab entry index
 * @mac_entry: mac entry
 */
static int dsaf_get_mac_entry_by_index(
	struct dsaf_device *dsaf_dev, u16 entry_index,
	struct dsaf_drv_mac_multi_dest_entry *mac_entry)
{
	struct dsaf_drv_tbl_tcam_key mac_key;

	struct dsaf_tbl_tcam_mcast_cfg mac_data;
	struct dsaf_tbl_tcam_ucast_cfg mac_uc_data;
	char mac_addr[MAC_NUM_OCTETS_PER_ADDR] = {0};

	if (entry_index >= DSAF_MAX_TCAM_ENTRY_NUM) {
		/* find none, del error */
		log_err(dsaf_dev->dev, "dsaf_get_mac_uc_entry faild, dsaf%d\n",
			dsaf_dev->chip_id);
		return -EINVAL;
	}

	/* mc entry, do read opt */
	dsaf_tcam_mc_get(dsaf_dev->chip_id, entry_index,
		(struct dsaf_tbl_tcam_data *)&mac_key, &mac_data);

	mac_entry->port_mask[0] = mac_data.tbl_mcast_xge5_0;

	/***get mac addr*/
	mac_addr[0] = mac_key.high.bits.mac_0;
	mac_addr[1] = mac_key.high.bits.mac_1;
	mac_addr[2] = mac_key.high.bits.mac_2;
	mac_addr[3] = mac_key.high.bits.mac_3;
	mac_addr[4] = mac_key.low.bits.mac_4;
	mac_addr[5] = mac_key.low.bits.mac_5;
	/**is mc or uc*/
	if (MAC_IS_MULTICAST((u8 *)mac_addr)
		|| MAC_IS_L3_MULTICAST((u8 *)mac_addr)) {
		/**mc donot do*/
	} else {
		/*is not mc, just uc... */
		dsaf_tcam_uc_get((u32)dsaf_dev->chip_id, entry_index,
			(struct dsaf_tbl_tcam_data *)&mac_key, &mac_uc_data);

		/*mac_entry->out_vlan_id = mac_data.tbl_ucast_tag & 0xfff;
		mac_entry->qos = (mac_data.tbl_ucast_tag & 0xe000) >> 13;
		mac_entry->vlan_mode =
			(enum dsaf_vlan_mode)mac_data.tbl_ucast_tag_add;
		mac_entry->port_num = mac_data.tbl_ucast_out_port*//*TBD*/
		mac_entry->port_mask[0] = (1 << mac_uc_data.tbl_ucast_out_port);
	}

	return 0;
}


/**
 * dsaf_fix_mac_mode - modify mac mode
 * @dsaf_dev: dsa fabric device struct pointer
 * @port_id: port id
 */
static int dsaf_fix_mac_mode(struct dsaf_device *dsaf_dev, u8 port_id)
{
	enum dsaf_port_rate_mode mode;

	if (MAC_PHY_INTERFACE_MODE_XGMII == dsaf_dev->mac_dev[port_id]->phy_if)
		mode = DSAF_PORT_RATE_10000;
	else
		mode = DSAF_PORT_RATE_1000;

	dsaf_port_work_rate_cfg(dsaf_dev->chip_id, port_id, mode);

	return 0;
}

/**
 * dsaf_ctl_mac_learning - modify mac learning en or disenable
 * @dsaf_dev: dsa fabric device struct pointer
 * @enable: enable vale 0=dienabele,1=enable
 */
static int dsaf_ctl_mac_learning(struct dsaf_device *dsaf_dev, u8 enable)
{
	union dsaf_tbl_int_msk1 int_msk;

	dsaf_int_tbl_msk_get(dsaf_dev->chip_id, &int_msk);

	if (enable)
		int_msk.bits.tbl_sa_mis_msk = 1;
	else
		int_msk.bits.tbl_sa_mis_msk = 0;

	dsaf_int_tbl_msk_cfg(dsaf_dev->chip_id, int_msk);

	return 0;
}

/**
 * dsaf_set_promiscuous - modify mac promiscuous en or disenable
 * @dsaf_dev: dsa fabric device struct pointer
 * @enable: enable vale 0=dienabele,1=enable
 */
static int dsaf_set_promiscuous(struct dsaf_device *dsaf_dev, u8 enable)
{
	if (enable)
		dsaf_mix_mode_cfg(dsaf_dev->chip_id, 1);
	else
		dsaf_mix_mode_cfg(dsaf_dev->chip_id, 0);

	return 0;
}

/**
 * dsaf_set_port_def_vlan - set mac vlan
 * @dsaf_dev: dsa fabric device struct pointer
 * @port_id: port id 0-5
 * @port_def_vlan: vlan
 */
static int dsaf_set_port_def_vlan(struct dsaf_device *dsaf_dev, u8 port_id,
	u32 port_def_vlan)
{
	if (port_id >= DSAF_SW_PORT_NUM) {
		log_err(dsaf_dev->dev,
			"portis error, dsaf=%d port_id=%d, port_def_vlan=%d\n",
			dsaf_dev->chip_id, port_id, port_def_vlan);
		return -EINVAL;
	}
	dsaf_port_def_vlan_cfg(dsaf_dev->chip_id, port_id, (u16)port_def_vlan);

	return 0;
}

/**
 * dsaf_get_port_def_vlan - get mac vlan
 * @dsaf_dev: dsa fabric device struct pointer
 * @port_id: port id 0-5
 * @port_def_vlan: vlan value addr
 */
static int dsaf_get_port_def_vlan(struct dsaf_device *dsaf_dev, u8 port_id,
	u16 *port_def_vlan)
{
	if ((port_id >= DSAF_SW_PORT_NUM) || (NULL == port_def_vlan)) {
		log_err(dsaf_dev->dev, "portis error , dsaf=%d port_id=%d\n",
			dsaf_dev->chip_id, port_id);
		return -EINVAL;
	}

	dsaf_port_def_vlan_get(dsaf_dev->chip_id, port_id, port_def_vlan);

	return 0;
}

/**
 * dsaf_int_proc - dsa fabric intruptet handle fun
 * @dsaf_dev: dsa fabric device struct pointer
 * @irq_num: irq id
 */
static void dsaf_int_proc(struct dsaf_device *dsaf_dev, u32 irq_num)
{
	u32 idx = 0;

	for (idx = 0; idx < DSAF_IRQ_NUM; idx++) {
		if ((int)irq_num == (int)dsaf_dev->virq[idx])
			break;
	}
	if (idx < DSAF_IRQ_NUM) {
		(void)dsaf_int_handler(dsaf_dev->chip_id, idx + 3);
		log_info(dsaf_dev->dev, "enter handle, dsaf=%d idx=%d hwirq=%d virq=%d\n",
			dsaf_dev->chip_id, idx, dsaf_dev->base_irq + idx, irq_num);
	} else {
		log_err(dsaf_dev->dev, "virq err, dsaf=%d virq=%d\n",
			dsaf_dev->chip_id, irq_num);
		for (idx = 0; idx < DSAF_IRQ_NUM; idx++)
			log_info(dsaf_dev->dev, "virq list: dsaf=%d idx=%d hwirq=%d virq=%d\n",
				dsaf_dev->chip_id, idx, dsaf_dev->base_irq + idx, dsaf_dev->virq[idx]);
	}


}

/**
 * dsaf_config - config dsa fibric device struct
 * @dsaf_dev: dsa fabric device struct pointer
 */
int dsaf_config(struct dsaf_device *dsaf_dev)
{
	dsaf_dev->dsaf_init = dsaf_init;
	dsaf_dev->dsaf_free = dsaf_free;
	dsaf_dev->ctrl_mac_learning = dsaf_ctl_mac_learning;
	dsaf_dev->set_promiscuous = dsaf_set_promiscuous;

	/* Mac uc entry config, if none add, or modify*/
	dsaf_dev->set_mac_uc_entry = dsaf_set_mac_uc_entry;
	/* Mac mc entry config , if none add , or add a port in it*/
	dsaf_dev->add_mac_mc_port = dsaf_add_mac_mc_port;
	/* Mac mc entry config, if none add, or modify*/
	dsaf_dev->set_mac_mc_entry = dsaf_set_mac_mc_entry;
	/*del match mac entry*/
	dsaf_dev->del_mac_entry = dsaf_del_mac_entry;
	/* Mac mc entry config, if just a port do del, or just do del a port*/
	dsaf_dev->del_mac_mc_port = dsaf_del_mac_mc_port;

	dsaf_dev->get_mac_uc_entry = dsaf_get_mac_uc_entry;
	dsaf_dev->get_mac_mc_entry = dsaf_get_mac_mc_entry;
	dsaf_dev->get_mac_entry_by_index = dsaf_get_mac_entry_by_index;

	dsaf_dev->fix_mac_mode = dsaf_fix_mac_mode;
	dsaf_dev->dsaf_int_proc = dsaf_int_proc;

	/**ser def vlan id*/
	dsaf_dev->set_port_def_vlan = dsaf_set_port_def_vlan;
	/**get def vlan id*/
	dsaf_dev->get_port_def_vlan = dsaf_get_port_def_vlan;

	dsaf_dev->get_sset_count = NULL;
	dsaf_dev->get_ethtool_stats = NULL;
	dsaf_dev->get_regs = NULL;
	dsaf_dev->get_strings = NULL;

	return 0;
}

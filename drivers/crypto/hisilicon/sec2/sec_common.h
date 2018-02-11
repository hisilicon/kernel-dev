/*
 * Copyright (c) 2016-2017 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/* This file is shared between user and kernel drivers of Hisilicon SEC  */

#ifndef HISI_SEC_COMMON_H
#define HISI_SEC_COMMON_H

#include <linux/types.h>

#define HISI_SEC_DRV_NAME			"hisi_sec_pci_driver"
#define LITTLE

#define HISI_SEC_DEV_NAME  "hisi_sec"
#define HISI_SEC_V1_HW_TYPE				(1)
#define HISI_SEC_V2_HW_TYPE				(2)

#define HISI_MAX_SEC_DEVICES				(4)
#define HISI_SEC_ALG_TYPES				(4)


#define HISI_SEC_Q_IRQ_NUM				(2)
#define HISI_SEC_HW_RING_NUM				(4)
#define SEC_Q_REGS					(0)
#define SEC_CMD_RING					(1)
#define SEC_OUTORDER_RING				(2)
#define SEC_DBG_RING					(3)


#define U_SEC_QUEUE_AR_FROCE_ALLOC			(0)
#define U_SEC_QUEUE_AR_FROCE_NOALLOC			(1)
#define U_SEC_QUEUE_AR_FROCE_DIS			(2)

#define U_SEC_QUEUE_AW_FROCE_ALLOC			(0)
#define U_SEC_QUEUE_AW_FROCE_NOALLOC			(1)
#define U_SEC_QUEUE_AW_FROCE_DIS			(2)


#define HISI_SEC_Q_INIT_REG						(0x0)
#define HISI_SEC_Q_CFG_REG						(0x8)
#define HISI_SEC_Q_PROC_NUM_CFG_REG					(0x10)
#define HISI_SEC_QUEUE_ENB_REG						(0x18)
#define HISI_SEC_Q_DEPTH_CFG_REG					(0x50)
#define HISI_SEC_Q_BASE_HADDR_REG					(0x54)
#define HISI_SEC_Q_BASE_LADDR_REG					(0x58)
#define HISI_SEC_Q_WR_PTR_REG						(0x5C)
#define HISI_SEC_Q_OUTORDER_BASE_HADDR_REG				(0x60)
#define HISI_SEC_Q_OUTORDER_BASE_ADDR_REG				(0x64)

#define HISI_SEC_Q_OUTORDER_RD_PTR_REG					(0x68)
#define HISI_SEC_Q_OT_TH_REG						(0x6C)
#define HISI_SEC_Q_ARUSER_CFG_REG					(0x70)
#define HISI_SEC_Q_AWUSER_CFG_REG					(0x74)
#define HISI_SEC_Q_ERR_BASE_HADDR_REG					(0x7C)
#define HISI_SEC_Q_ERR_BASE_LADDR_REG					(0x80)
#define HISI_SEC_Q_CFG_VF_NUM_REG					(0x84)
#define HISI_SEC_Q_SOFT_PROC_PTR_REG					(0x88)
#define HISI_SEC_Q_FAIL_INT_MSK_REG					(0x300)
#define HISI_SEC_Q_FLOW_INT_MKS_REG					(0x304)
#define HISI_SEC_Q_FAIL_RINT_REG					(0x400)
#define HISI_SEC_Q_FLOW_RINT_REG					(0x404)
#define HISI_SEC_Q_FAIL_INT_STATUS_REG					(0x500)
#define HISI_SEC_Q_FLOW_INT_STATUS_REG					(0x504)
#define HISI_SEC_Q_STATUS_REG						(0x600)
#define HISI_SEC_Q_RD_PTR_REG						(0x604)
#define HISI_SEC_Q_PRO_PTR_REG						(0x608)
#define HISI_SEC_Q_OUTORDER_WR_PTR_REG					(0x60C)
#define HISI_SEC_Q_OT_CNT_STATUS_REG					(0x610)
#define HISI_SEC_Q_INORDER_BD_NUM_ST_REG				(0x650)
#define HISI_SEC_Q_INORDER_GET_FLAG_ST_REG				(0x654)
#define HISI_SEC_Q_INORDER_ADD_FLAG_ST_REG				(0x658)
#define HISI_SEC_Q_INORDER_TASK_INT_NUM_LEFT_ST_REG			(0x65C)
#define HISI_SEC_Q_RD_DONE_PTR_REG					(0x660)
#define HISI_SEC_Q_CPL_Q_BD_NUM_ST_REG					(0x700)
#define HISI_SEC_Q_CPL_Q_PTR_ST_REG					(0x704)
#define HISI_SEC_Q_CPL_Q_H_ADDR_ST_REG					(0x708)
#define HISI_SEC_Q_CPL_Q_L_ADDR_ST_REG					(0x70C)
#define HISI_SEC_Q_CPL_TASK_INT_NUM_LEFT_ST_REG				(0x710)
#define HISI_SEC_Q_WRR_ID_CHECK_REG					(0x714)
#define HISI_SEC_Q_CPLQ_FULL_CHECK_REG					(0x718)
#define HISI_SEC_Q_SUCCESS_BD_CNT_REG					(0x800)
#define HISI_SEC_Q_FAIL_BD_CNT_REG					(0x804)
#define HISI_SEC_Q_GET_BD_CNT_REG					(0x808)
#define HISI_SEC_Q_IVLD_CNT_REG						(0x80C)
#define HISI_SEC_Q_BD_PROC_GET_CNT_REG					(0x810)
#define HISI_SEC_Q_BD_PROC_DONE_CNT_REG					(0x814)
#define HISI_SEC_Q_LAT_CLR_REG						(0x850)
#define HISI_SEC_Q_PKT_LAT_MAX_REG					(0x854)
#define HISI_SEC_Q_PKT_LAT_AVG_REG					(0x858)
#define HISI_SEC_Q_PKT_LAT_MIN_REG					(0x85C)
#define HISI_SEC_Q_ID_CLR_CFG_REG					(0x900)
#define HISI_SEC_Q_1ST_BD_ERR_ID_REG					(0x904)
#define HISI_SEC_Q_1ST_AUTH_FAIL_ID_REG					(0x908)
#define HISI_SEC_Q_1ST_RD_ERR_ID_REG					(0x90C)
#define HISI_SEC_Q_1ST_ECC2_ERR_ID_REG					(0x910)
#define HISI_SEC_Q_1ST_IVLD_ID_REG					(0x914)
#define HISI_SEC_Q_1ST_BD_WR_ERR_ID_REG					(0x918)
#define HISI_SEC_Q_1ST_ERR_BD_WR_ERR_ID_REG				(0x91C)
#define HISI_SEC_Q_1ST_BD_MAC_WR_ERR_ID_REG				(0x920)


/* cipher before auth */
#define SEC_SEQ_CIPHER_AUTH				(0x0)
/* auth before cipher */
#define SEC_SEQ_AUTH_CIPHER				(0x1)

struct sec_bd_info {

	/* w0 */
	u32 bd_type				: 4;
	u32 cipher				: 2;
	u32 auth				: 2;
	u32 seq					: 1;
	u32 de					: 2;
	u32 scene				: 4;
	u32 src_addr_type			: 3;
	u32 dst_addr_type			: 3;
	u32 rsv_w0				: 11;

	/* w1*/
	u32 nonce_len				: 4;
	u32 huk					: 1;
	u32 key_s				: 1;
	u32 ci_gen				: 2;
	u32 ai_gen				: 2;
	u32 auth_pad				: 2;
	u32 c_s					: 2;
	u32 rsv1_w1				: 2;
	u32 rhf					: 1;
	u32 cipher_key_type			: 2;
	u32 auth_key_type			: 2;
	u32 write_frame_len			: 3;
	u32 rsv2_w1				: 7;
	u32 bd_invalid				: 1;

	/* w2 */
	u32 mac_len				: 5;
	u32 akey_len				: 6;
	u32 a_alg				: 6;
	u32 rsv_w2				: 15;

	/* w3 */
	u32 c_icv_len				: 6;
	u32 c_width				: 3;
	u32 c_key_len				: 3;
	u32 c_mode				: 4;
	u32 c_alg				: 4;
	u32 rsv_w3				: 12;

	/* w4 */
	u32 auth_len				: 24;
	u32 rsv_w4				: 8;

	/* w5 */
	u32 cipher_len				: 24;
	u32 rsv_w5				: 8;

	/* w6 */
	u32 auth_src_offset			: 16;
	u32 cipher_src_offset			: 16;

	/* w7 */
	u32 cs_ip_headr_offset			: 16;
	u32 cs_udp_header_offset		: 16;

	/* w8 */
	u32 pass_word_len			: 16;
	u32 dk_len				: 16;

	/* w9 */
	u32 salt;

	/* w10 */
	u32 cipher_pad_type			: 4;
	u32 cipher_pad_len			: 8;
	u32 cipher_pad_data_type		: 4;
	u32 cipher_pad_len_field		: 2;
	u32 rsv_w10				: 14;

	/* w11 */
	u32 tag					: 16;
	u32 rsv_w11				: 16;

	/* w12 */
	u32 long_auth_data_len;
	/* w13 */
	u32 long_auth_data_len_hi;
	/* w14 */
	u32 auth_key_addr;
	/* w15 */
	u32 auth_key_addr_hi;
	/* w16 */
	u32 auth_ivin_addr;
	/* w17 */
	u32 auth_iviv_addr_hi;
	/* w18 */
	u32 mac_addr;
	/* w19 */
	u32 mac_addr_hi;
	/* w20 */
	u32 cipher_key_addr;
	/* w21 */
	u32 cipher_key_addr_hi;
	/* w22 */
	u32 cipher_iv_addr;
	/* w23 */
	u32 cipher_iv_addr_hi;
	/* w24 */
	u32 data_src_addr;
	/* w25 */
	u32 data_src_addr_hi;
	/* w26 */
	u32 data_dst_addr;
	/* w27 */
	u32 data_dst_addr_hi;

	/* w28 */
	u32 done				: 1;
	u32 icv					: 3;
	u32 csc					: 3;
	u32 flag				: 3;
	u32 rsv_w28				: 6;
	u32 error_type				: 8;
	u32 warning_type			: 8;

	/* w29 */
	u32 mac_i;
	/* w30 */
	u32 checksum_i				: 16;
	u32 rsv_w30				: 16;

	/* w31*/
	u32 counter;
};


#define HISI_SEC_IRQ_EN_NUM				(1)
#define HISI_SEC_SIZE_64K				(0x10000)


#ifndef PAGE_SIZE
#define PAGE_SIZE	getpagesize()
#endif


#define WPG_ALIGN(size)	((size + PAGE_SIZE - 1) & (~(PAGE_SIZE - 1)))

#define HISI_SEC_IO1_SIZE WPG_ALIGN(HISI_SEC_SIZE_64K)
#define HISI_SEC_IO2_SIZE WPG_ALIGN(HISI_SEC_QUEUE_LEN * HISI_SEC_BD_SIZE)
#define HISI_SEC_IO3_SIZE WPG_ALIGN(HISI_SEC_QUEUE_LEN * HISI_SEC_OUT_BD_SIZE)
#define HISI_SEC_IO4_SIZE WPG_ALIGN(HISI_SEC_QUEUE_LEN * HISI_SEC_DBG_BD_SIZE)

/* sizeof SEC_Q_REGS + SEC_CMD_RING + SEC_OUTORDER_RING + SEC_DBG_RING */
#define HISI_SEC_IOSPACE_SIZE	(HISI_SEC_IO1_SIZE + \
				 HISI_SEC_IO2_SIZE + \
				 HISI_SEC_IO3_SIZE + \
				 HISI_SEC_IO4_SIZE)

#define HISI_SEC_BD_SIZE	(sizeof(struct sec_bd_info))
#define HISI_SEC_OUT_BD_SIZE    (sizeof(struct sec_out_bd_info))
#define HISI_SEC_DBG_BD_SIZE    (sizeof(struct sec_debug_bd_info))

#define HISI_MAX_SGE_NUM   64
#ifndef dma_addr_t
#define dma_addr_t __u64
#endif
struct sec_hw_sge {
	dma_addr_t buf;
	unsigned int len;
	unsigned int pad;
};

struct sec_hw_sgl {
	dma_addr_t next_sgl;
	__u16 entry_sum_in_chain;
	__u16 entry_sum_in_sgl;
	__u32 flag;
	__u64 serial_num;
	__u32 cpuid;
	__u32 data_bytes_in_sgl;
	struct sec_hw_sgl *next;
	__u8  reserved[8];
	struct sec_hw_sge  sge_entrys[HISI_MAX_SGE_NUM];
	__u8 node[16];
};

enum hisi_sec_cipher_type {
	SEC_CIPHER_NULL,
	SEC_CIPHER_ENCRYPT,
	SEC_CIPHER_DECRYPT,
	SEC_CIPHER_PASS,
	SEC_CIPHER_INVALID,
};

enum hisi_sec_auth_type {
	SEC_AUTH_NULL,
	SEC_AUTH_HASH,
	SEC_AUTH_AUTH,
	SEC_AUTH_INVALID,
};

enum hisi_sec_cipher_alg {
	SEC_DES_ECB_64		=  0x0008,
	SEC_DES_CBC_64		=  0x0048,

	SEC_3DES_ECB_192_3KEY   =  0x0408,
	SEC_3DES_ECB_192_2KEY   =  0x0418,

	SEC_3DES_CBC_192_3KEY   =  0x0448,
	SEC_3DES_CBC_192_2KEY   =  0x0458,

	SEC_AES_ECB_128         =  0x0800,
	SEC_AES_ECB_192         =  0x0808,
	SEC_AES_ECB_256         =  0x0810,

	SEC_AES_CBC_128         =  0x0840,
	SEC_AES_CBC_192         =  0x0848,
	SEC_AES_CBC_256         =  0x0850,

	SEC_AES_CTR_128         =  0x0900,
	SEC_AES_CTR_192         =  0x0908,
	SEC_AES_CTR_256         =  0x0910,

	SEC_AES_CCM_128         =  0x0940,
	SEC_AES_CCM_192         =  0x0948,
	SEC_AES_CCM_256         =  0x0950,

	SEC_AES_GCM_128         =  0x0980,
	SEC_AES_GCM_192         =  0x0988,
	SEC_AES_GCM_256         =  0x0990,

	SEC_AES_XTS_128         =  0x05c0,
	SEC_AES_XTS_256         =  0x05d0,

	/* extend */
	SEC_AES_CTS_128         =  0x0a40,
	SEC_AES_CTS_192         =  0x0a48,
	SEC_AES_CTS_256         =  0x0a50,

	SEC_SM4_CBC_128         =  0x0c40,
	SEC_SM4_XTS_128         =  0x0dc0,

	SEC_CIPHER_ALG_INVALID  =  0xffff,
};

enum hisi_sec_auth_alg {
	SEC_SHA_160                = 0x0000,
	SEC_SHA_256                = 0x0400,
	SEC_MD5                    = 0x0800,
	SEC_SHA_224                = 0x0C00,
	SEC_AES_XCBC_MAC_96        = 0x3483,
	SEC_AES_XCBC_PRF_128       = 0x3484,
	SEC_AES_CMAC               = 0x3884,

	SEC_HMAC_SHA160_96         = 0x2003,
	SEC_HMAC_SHA160            = 0x2005,
	SEC_HMAC_SHA224_96         = 0x2803,
	SEC_HMAC_SHA224            = 0x2807,
	SEC_HMAC_SHA256_96         = 0x2c03,
	SEC_HMAC_SHA256            = 0x2c08,
	SEC_HMAC_MD5_96            = 0x3003,
	SEC_HMAC_MD5               = 0x3004,

	SEC_AUTH_ALG_INVALID       = 0xffff,
};

/* This queue information is shared between user and kernel sec drivers */
struct hisi_sec_queue_info {
	char mdev_name[32];
	char uuid[16];

	/* The following two units are for debug. Once wd_get_start_paddr
	 * is ok, the two can be removed.
	 */
	unsigned long long cmd_buf;
	unsigned long long mid_buf;
};


#define HISI_SEC_V2_MAX_KEY		32
#define HISI_SEC_V2_MAX_IV		16

/* This area is used for storing key or iv, which can be DMA */
#define HISI_SEC_V2_EXEREA_SIZE	(2*(HISI_SEC_V2_MAX_KEY + HISI_SEC_V2_MAX_IV))


/* wd register interface */
int sec_register_to_wd(struct hisi_sec *hisi_sec);

#endif /* HISI_SEC_COMMON_H */

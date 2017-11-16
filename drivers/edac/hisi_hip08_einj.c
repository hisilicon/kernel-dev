/*
 * Copyright (c) 2017 Hisilicon Limited.
 * Error injection facility for injecting non-standard errors
 * on Hisilicon HIP08 SoC.
 * This driver should be built as module so that it can be
 * loaded on production kernels for testing purposes.
 *
 * This file may be distributed under the terms of the GNU
 * General Public License version 2.
 */

#include <linux/kobject.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/cpu.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <asm/sysreg.h>
#include <linux/delay.h>

/* ==== Usage ====
 * mount -t debugfs none /sys/kernel/debug
 * cd sys/kernel/debug/hip08_einj/<device>
 * cat available_error_type -> List supported hw errors with error id
 * echo <error id> > error_type
 * echo 0x1 > error_inject
 */

/* Definitions */

/* SAS EINJ definitions */
#define SAS_REG_BASE		0xa2000000
#define SAS_REG_MAP_SIZE	0x0270

#define SAS_ECC_ERR_MASK0	0x01f0
#define SAS_CFG_ECC_ERR_INJ0_EN	0x0200
#define SAS_CFG_ECC_ERR_INJ1_EN	0x0204

/* HNS EINJ definitions */
/*
 * #define HNS_REG_BASE		0xc5080000
 * #define HNS_REG_MAP_SIZE	0x1400
 *
 * #define HNS_SRAM_ECC_CHK_EN	0x0428
 * #define HNS_SRAM_ECC_CHK0	0x042C
 */

/* CPU - Cache EINJ Registers */
#define L3D_REG_BASE	0x90140000
#define L3D_REG_MAP_SIZE	0x1000
#define L3D_ECC_INJECT_LEFT	0x05f0
#define L3D_ECC_INJECT_RIGHT	0x05f4

#define L3T_REG_BASE	0x90180000
#define L3T_REG_MAP_SIZE	0x1000
#define L3T_DFX_CTRL	0x00000408

/* SMMU Error Inject registers */
#define HAC_SMMU_REG_BASE	0x14000000
#define SMMU_REG_MAP_SIZE	0x1000
#define PCIE_SMMU_REG_BASE	0x14800000
#define MGMT_SMMU_REG_BASE	0x20100000
#define NIC_SMMU_REG_BASE	0x10000000
#define SMMU_ECC_INJECT	0x00000EA0

/* HHA Error Inject register */
#define HHA_REG_BASE	0x90120000
#define HHA_REG_MAP_SIZE	0x1000
#define REG_HHA_ECC_INJECT	0x0500

/* HLLC Error Inject register */
#define HLLC_REG_BASE	0x20080000
#define HLLC_REG_MAP_SIZE	0x2000
#define REG_HLLC_INJECT_ECC_TYPE	0x00001600
#define REG_HLLC_INJECT_ECC_EN	        0x00001604
#define REG_HLLC_PHY_TX_INJECT_CRC_EN	0x00001700
#define REG_HLLC_PHY_TX_INJECT_CRC_TIMES	0x00001704

/* Prevent Error Injection run simultaneously. */
static DEFINE_MUTEX(einj_mutex);

struct hisi_hw_error {
	u32 val;
	u32 reg_ecc_einj_en_val;
	const char *type;
	void __iomem *reg_base;
	u64 base_addr;
	u64 reg_offset;
};

static struct dentry *dfs_inj;
static u32 sas_error_type;
/* static u32 hns_error_type; */
static u32 cache_error_type;
static u32 smmu_error_type;
static u32 hha_error_type;
static u32 hllc_error_type;

static struct hisi_hw_error sas_errors[] = {
	{
		.val = 0x1,
		.reg_ecc_einj_en_val = BIT(0),
		.type = "SAS_HGC_IOMB_DQE_RAM0_ECC\n",
	},
	{
		.val = 0x2,
		.reg_ecc_einj_en_val = BIT(1),
		.type = "SAS_HGC_IOMB_DQE_RAM1_ECC\n",
	},
	{
		.val = 0x3,
		.reg_ecc_einj_en_val = BIT(2),
		.type = "SAS_HGC_IOMB_DQE_RAM2_ECC\n",
	},
	{
		.val = 0x4,
		.reg_ecc_einj_en_val = BIT(3),
		.type = "SAS_HGC_IOMB_DQE_RAM3_ECC\n",
	},
	{
		.val = 0x5,
		.reg_ecc_einj_en_val = BIT(4),
		.type = "SAS_HGC_IOMB_CQE_RAM_ECC\n",
	},
	{
		.val = 0x6,
		.reg_ecc_einj_en_val = BIT(5),
		.type = "SAS_HGC_IOMB_IOST_RAM0_ECC\n",
	},
	{
		.val = 0x7,
		.reg_ecc_einj_en_val = BIT(6),
		.type = "SAS_HGC_IOMB_IOST_RAM1_ECC\n",
	},
	{
		.val = 0x8,
		.reg_ecc_einj_en_val = BIT(7),
		.type = "SAS_HGC_IOMB_IOST_RAM2_ECC\n",
	},
	{
		.val = 0x9,
		.reg_ecc_einj_en_val = BIT(8),
		.type = "SAS_HGC_IOMB_IOST_RAM3_ECC\n",
	},
	{
		.val = 0xA,
		.reg_ecc_einj_en_val = BIT(9),
		.type = "SAS_HGC_IOMB_ITCT_RAM0_ECC\n",
	},
	{
		.val = 0xB,
		.reg_ecc_einj_en_val = BIT(10),
		.type = "SAS_HGC_IOMB_ITCT_RAM1_ECC\n",
	},
	{
		.val = 0xC,
		.reg_ecc_einj_en_val = BIT(11),
		.type = "SAS_HGC_IOMB_ITCT_RAM2_ECC\n",
	},
	{
		.val = 0xD,
		.reg_ecc_einj_en_val = BIT(12),
		.type = "SAS_HGC_IOMB_ITCT_RAM3_ECC\n",
	},
	{
		.val = 0xE,
		.reg_ecc_einj_en_val = BIT(13),
		.type = "RXM_MEM0_ECC\n",
	},
	{
		.val = 0xF,
		.reg_ecc_einj_en_val = BIT(14),
		.type = "RXM_MEM1_ECC\n",
	},
	{
		.val = 0x10,
		.reg_ecc_einj_en_val = BIT(15),
		.type = "RXM_MEM2_ECC\n",
	},
	{
		.val = 0x11,
		.reg_ecc_einj_en_val = BIT(16),
		.type = "RXM_MEM3_ECC\n",
	},
	{
		.val = 0x12,
		.reg_ecc_einj_en_val = BIT(17),
		.type = "LM_ITCT_MEM0_ECC\n",
	},
	{
		.val = 0x13,
		.reg_ecc_einj_en_val = BIT(18),
		.type = "LM_ITCT_MEM1_ECC\n",
	},
	{
		.val = 0x14,
		.reg_ecc_einj_en_val = BIT(19),
		.type = "LM_ITCT_MEM2_ECC\n",
	},
	{
		.val = 0x15,
		.reg_ecc_einj_en_val = BIT(20),
		.type = "LM_ITCT_MEM3_ECC\n",
	},
	{
		.val = 0x16,
		.reg_ecc_einj_en_val = BIT(21),
		.type = "LM_IOST_MEM0_ECC\n",
	},
	{
		.val = 0x17,
		.reg_ecc_einj_en_val = BIT(22),
		.type = "LM_IOST_MEM1_ECC\n",
	},
	{
		.val = 0x18,
		.reg_ecc_einj_en_val = BIT(23),
		.type = "LM_IOST_MEM2_ECC\n",
	},
	{
		.val = 0x19,
		.reg_ecc_einj_en_val = BIT(24),
		.type = "LM_IOST_MEM3_ECC\n",
	},
	{
		.val = 0x1A,
		.reg_ecc_einj_en_val = BIT(0),
		.type = "CH0_SAS_DMAC_TX_RAM_ECC\n",
	},
	{
		.val = 0x1B,
		.reg_ecc_einj_en_val = BIT(1),
		.type = "CH0_SAS_DMAC_RX_RXM_ECC\n",
	},
	{
		.val = 0x1C,
		.reg_ecc_einj_en_val = BIT(2),
		.type = "CH1_SAS_DMAC_TX_RAM_ECC\n",
	},
	{
		.val = 0x1D,
		.reg_ecc_einj_en_val = BIT(3),
		.type = "CH1_SAS_DMAC_RX_RXM_ECC\n",
	},
	{
		.val = 0x1E,
		.reg_ecc_einj_en_val = BIT(4),
		.type = "CH2_SAS_DMAC_TX_RAM_ECC\n",
	},
	{
		.val = 0x1F,
		.reg_ecc_einj_en_val = BIT(5),
		.type = "CH2_SAS_DMAC_RX_RXM_ECC\n",
	},
	{
		.val = 0x20,
		.reg_ecc_einj_en_val = BIT(6),
		.type = "CH3_SAS_DMAC_TX_RAM_ECC\n",
	},
	{
		.val = 0x21,
		.reg_ecc_einj_en_val = BIT(7),
		.type = "CH3_SAS_DMAC_RX_RXM_ECC\n",
	},
	{
		.val = 0x22,
		.reg_ecc_einj_en_val = BIT(8),
		.type = "CH4_SAS_DMAC_TX_RAM_ECC\n",
	},
	{
		.val = 0x23,
		.reg_ecc_einj_en_val = BIT(9),
		.type = "CH4_SAS_DMAC_RX_RXM_ECC\n",
	},
	{
		.val = 0x24,
		.reg_ecc_einj_en_val = BIT(10),
		.type = "CH5_SAS_DMAC_TX_RAM_ECC\n",
	},
	{
		.val = 0x25,
		.reg_ecc_einj_en_val = BIT(11),
		.type = "CH5_SAS_DMAC_RX_RXM_ECC\n",
	},
	{
		.val = 0x26,
		.reg_ecc_einj_en_val = BIT(12),
		.type = "CH6_SAS_DMAC_TX_RAM_ECC\n",
	},
	{
		.val = 0x27,
		.reg_ecc_einj_en_val = BIT(13),
		.type = "CH6_SAS_DMAC_RX_RXM_ECC\n",
	},
	{
		.val = 0x28,
		.reg_ecc_einj_en_val = BIT(14),
		.type = "CH7_SAS_DMAC_TX_RAM_ECC\n",
	},
	{
		.val = 0x29,
		.reg_ecc_einj_en_val = BIT(15),
		.type = "CH7_SAS_DMAC_RX_RAM_ECC\n",
	},
	{
		.val = 0x2A,
		.reg_ecc_einj_en_val = BIT(16),
		.type = "CH8_SAS_DMAC_TX_RAM_ECC\n",
	},
	{
		.val = 0x2B,
		.reg_ecc_einj_en_val = BIT(17),
		.type = "CH8_SAS_DMAC_RX_RXM_ECC\n",
	},
};
/*
static struct hisi_hw_error hns_errors[] = {
	{
		.val = 0x1,
		.reg_ecc_einj_en_val = 0x01,
		.type = "rcb_rx_ebd_sram_ecc\n",
	},
	{
		.val = 0x2,
		.reg_ecc_einj_en_val = 0x02,
		.type = "rcb_rx_ring_sram_ecc\n",
	},
	{
		.val = 0x3,
		.reg_ecc_einj_en_val = 0x04,
		.type = "rcb_tx_ring_sram_ecc\n",
	},
	{
		.val = 0x4,
		.reg_ecc_einj_en_val = 0x08,
		.type = "rcb_tx_fbd_sram_ecc\n",
	},
};
*/
static struct hisi_hw_error cache_errors[] = {
	{
		.val = 0x1,
		.reg_ecc_einj_en_val = 0x101,
		.type = "L3D_Cache_1BIT_ECC\n",
	},
	{
		.val = 0x2,
		.reg_ecc_einj_en_val = 0x1010201,
		.type = "L3D_Cache_2BIT_ECC\n",
	},
	{
		.val = 0x3,
		.reg_ecc_einj_en_val = 0x2000000,
		.type = "L3T_Cache_DFX_DIR_1BIT_ECC\n",
	},
	{
		.val = 0x4,
		.reg_ecc_einj_en_val = 0x4000000,
		.type = "L3T_Cache_DFX_DIR_2BIT_ECC\n",
	},
	{
		.val = 0x5,
		.reg_ecc_einj_en_val = 0x8000000,
		.type = "L3T_Cache_DFX_STD_1BIT_ECC\n",
	},
	{
		.val = 0x6,
		.reg_ecc_einj_en_val = 0x10000000,
		.type = "L3T_Cache_DFX_STD_2BIT_ECC\n"
	},
	{
		.val = 0x7,
		.type = "L1_Cache_CE\n"
	},
	{
		.val = 0x8,
		.type = "L1_Cache_UE\n"
	},
	{
		.val = 0x9,
		.type = "L2_Cache_CE\n"
	},
	{
		.val = 0xA,
		.type = "L2_Cache_UE\n"
	},
};

static struct hisi_hw_error smmu_errors[] = {
	{
		.val = 0x1,
		.base_addr = HAC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x10008,
		.type = "HAC_SMMU_1BIT_AXI_RAM_ECC\n",
	},
	{
		.val = 0x2,
		.base_addr = HAC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x20008,
		.type = "HAC_SMMU_1BIT_TBU_RAM_ECC\n",
	},
	{
		.val = 0x3,
		.base_addr = HAC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x40008,
		.type = "HAC_SMMU_1BIT_TCU_RAM_ECC\n",
	},
	{
		.val = 0x4,
		.base_addr = HAC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x90908,
		.type = "HAC_SMMU_2BIT_AXI_RAM_ECC\n",
	},
	{
		.val = 0x5,
		.base_addr = HAC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0xA0908,
		.type = "HAC_SMMU_2BIT_TBU_RAM_ECC\n",
	},
	{
		.val = 0x6,
		.base_addr = HAC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0xC0908,
		.type = "HAC_SMMU_2BIT_TCU_RAM_ECC\n",
	},
	{
		.val = 0x7,
		.base_addr = PCIE_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x10008,
		.type = "PCIE_SMMU_1BIT_AXI_RAM_ECC\n",
	},
	{
		.val = 0x8,
		.base_addr = PCIE_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x20008,
		.type = "PCIE_SMMU_1BIT_TBU_RAM_ECC\n",
	},
	{
		.val = 0x9,
		.base_addr = PCIE_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x40008,
		.type = "PCIE_SMMU_1BIT_TCU_RAM_ECC\n",
	},
	{
		.val = 0xA,
		.base_addr = PCIE_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x90908,
		.type = "PCIE_SMMU_2BIT_AXI_RAM_ECC\n",
	},
	{
		.val = 0xB,
		.base_addr = PCIE_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0xA0908,
		.type = "PCIE_SMMU_2BIT_TBU_RAM_ECC\n",
	},
	{
		.val = 0xC,
		.base_addr = PCIE_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0xC00908,
		.type = "PCIE_SMMU_2BIT_TCU_RAM_ECC\n",
	},
	{
		.val = 0xD,
		.base_addr = MGMT_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x10008,
		.type = "MGMT_SMMU_1BIT_AXI_RAM_ECC\n",
	},
	{
		.val = 0xE,
		.base_addr = MGMT_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x20008,
		.type = "MGMT_SMMU_1BIT_TBU_RAM_ECC\n",
	},
	{
		.val = 0xF,
		.base_addr = MGMT_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x40008,
		.type = "MGMT_SMMU_1BIT_TCU_RAM_ECC\n",
	},
	{
		.val = 0x10,
		.base_addr = MGMT_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x90908,
		.type = "MGMT_SMMU_2BIT_AXI_RAM_ECC\n",
	},
	{
		.val = 0x11,
		.base_addr = MGMT_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0xA0908,
		.type = "MGMT_SMMU_2BIT_TBU_RAM_ECC\n",
	},
	{
		.val = 0x12,
		.base_addr = MGMT_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0xC0908,
		.type = "MGMT_SMMU_2BIT_TCU_RAM_ECC\n",
	},
	{
		.val = 0x13,
		.base_addr = NIC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x10008,
		.type = "NIC_SMMU_1BIT_AXI_RAM_ECC\n",
	},
	{
		.val = 0x14,
		.base_addr = NIC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x20008,
		.type = "NIC_SMMU_1BIT_TBU_RAM_ECC\n",
	},
	{
		.val = 0x15,
		.base_addr = NIC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x40008,
		.type = "NIC_SMMU_1BIT_TCU_RAM_ECC\n",
	},
	{
		.val = 0x16,
		.base_addr = NIC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0x90908,
		.type = "NIC_SMMU_2BIT_AXI_RAM_ECC\n",
	},
	{
		.val = 0x17,
		.base_addr = NIC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0xA0908,
		.type = "NIC_SMMU_2BIT_TBU_RAM_ECC\n",
	},
	{
		.val = 0x18,
		.base_addr = NIC_SMMU_REG_BASE,
		.reg_ecc_einj_en_val = 0xC0908,
		.type = "NIC_SMMU_2BIT_TCU_RAM_ECC\n",
	},
};

static struct hisi_hw_error hha_errors[] = {
	{
		.val = 0x1,
		.reg_ecc_einj_en_val = 0x0108,
		.type = "HHA_CBUF_RAM_1BIT_ECC\n",
	},
	{
		.val = 0x2,
		.reg_ecc_einj_en_val = 0x0208,
		.type = "HHA_DBUF_RAM_1BIT_ECC\n",
	},
	{
		.val = 0x3,
		.reg_ecc_einj_en_val = 0x0408,
		.type = "HHA_SDIR_RAM_1BIT_ECC\n",
	},
	{
		.val = 0x4,
		.reg_ecc_einj_en_val = 0x0808,
		.type = "HHA_EDIR_RAM_1BIT_ECC\n",
	},
	{
		.val = 0x5,
		.reg_ecc_einj_en_val = 0x1008,
		.type = "HHA_SDIRINFO_RAM_1BIT_ECC\n",
	},
	{
		.val = 0x6,
		.reg_ecc_einj_en_val = 0x2008,
		.type = "HHA_EDIRINFO_RAM_1BIT_ECC\n",
	},
	{
		.val = 0x7,
		.reg_ecc_einj_en_val = 0x1090108,
		.type = "HHA_CBUF_RAM_2BIT_ECC\n",
	},
	{
		.val = 0x8,
		.reg_ecc_einj_en_val = 0x1090208,
		.type = "HHA_DBUF_RAM_2BIT_ECC\n",
	},
	{
		.val = 0x9,
		.reg_ecc_einj_en_val = 0x1090408,
		.type = "HHA_SDIR_RAM_2BIT_ECC\n",
	},
	{
		.val = 0xA,
		.reg_ecc_einj_en_val = 0x1090808,
		.type = "HHA_EDIR_RAM_2BIT_ECC\n",
	},
	{
		.val = 0xB,
		.reg_ecc_einj_en_val = 0x1091008,
		.type = "HHA_SDIRINFO_RAM_2BIT_ECC\n",
	},
	{
		.val = 0xC,
		.reg_ecc_einj_en_val = 0x1092008,
		.type = "HHA_EDIRINFO_RAM_2BIT_ECC\n",
	},
};

static struct hisi_hw_error hllc_errors[] = {
	{
		.val = 0x1,
		.reg_ecc_einj_en_val = 0x0001,
		.type = "HLLC_PHY_TX_1BIT_CRC\n",
	},
	{
		.val = 0x2,
		.reg_ecc_einj_en_val = 0x0010,
		.type = "HLLC_PHY_TX_RETRY_1BIT_ECC\n",
	},
	{
		.val = 0x3,
		.reg_ecc_einj_en_val = 0x0100,
		.type = "HLLC_HYDRA_TX_CH0_MEM_1BIT_ECC\n",
	},
	{
		.val = 0x4,
		.reg_ecc_einj_en_val = 0x0200,
		.type = "HLLC_HYDRA_TX_CH1_MEE_1BIT_ECC\n",
	},
	{
		.val = 0x5,
		.reg_ecc_einj_en_val = 0x0400,
		.type = "HLLC_HYDRA_TX_CH2_MEM_1BIT_ECC\n",
	},
	{
		.val = 0x6,
		.reg_ecc_einj_en_val = 0x0010,
		.type = "HLLC_PHY_TX_RETRY_2BIT_ECC\n",
	},
	{
		.val = 0x7,
		.reg_ecc_einj_en_val = 0x0100,
		.type = "HLLC_HYDRA_TX_CH0_MEM_2BIT_ECC\n",
	},
	{
		.val = 0x8,
		.reg_ecc_einj_en_val = 0x0200,
		.type = "HLLC_HYDRA_TX_CH1_MEM_2BIT_ECC\n",
	},
	{
		.val = 0x9,
		.reg_ecc_einj_en_val = 0x0400,
		.type = "HLLC_HYDRA_TX_CH2_MEM_2BIT_ECC\n",
	},
};

/* common functions */

/* SAS einj functions */
static int sas_available_error_type_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sas_errors); i++)
		seq_printf(m, "0x%x\t%s\n",
			   sas_errors[i].val,
			   sas_errors[i].type);
	return 0;
}

static int sas_available_error_type_open(struct inode *inode, struct file *file)
{
	return single_open(file, sas_available_error_type_show, NULL);
}

static const struct file_operations sas_avl_err_type_fops = {
	.open		= sas_available_error_type_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int sas_error_type_get(void *data, u64 *val)
{
	*val = sas_error_type;

	return 0;
}

static int sas_error_type_set(void *data, u64 val)
{
	if (val < 0x1 || val > ARRAY_SIZE(sas_errors)) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	sas_error_type = val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(sas_error_type_fops, sas_error_type_get,
			sas_error_type_set, "0x%llx\n");

static int sas_error_inject_set(void *data, u64 val)
{
	if (!sas_error_type) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	mutex_lock(&einj_mutex);
	val = sas_errors[sas_error_type - 1].reg_ecc_einj_en_val;

	if (sas_error_type <= 0x19)
		writel(val, sas_errors[0].reg_base + SAS_CFG_ECC_ERR_INJ0_EN);
	else
		writel(val, sas_errors[0].reg_base + SAS_CFG_ECC_ERR_INJ1_EN);

	writel(0x101, sas_errors[0].reg_base + SAS_ECC_ERR_MASK0);
	mutex_unlock(&einj_mutex);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(sas_error_inject_fops, NULL,
			sas_error_inject_set, "%llu\n");

/* HNS einj functions */
#if 0
static int hns_available_error_type_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hns_errors); i++)
		seq_printf(m, "0x%x\t%s\n",
			   hns_errors[i].val,
			   hns_errors[i].type);
		return 0;
}

static int hns_available_error_type_open(struct inode *inode, struct file *file)
{
	return single_open(file, hns_available_error_type_show, NULL);
}

static const struct file_operations hns_avl_err_type_fops = {
	.open           = hns_available_error_type_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int hns_error_type_get(void *data, u64 *val)
{
	*val = hns_error_type;
	return 0;
}

static int hns_error_type_set(void *data, u64 val)
{
	if (val < 0x1 || val > ARRAY_SIZE(hns_errors)) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}
	hns_error_type = val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(hns_error_type_fops, hns_error_type_get,
			hns_error_type_set, "0x%llx\n");

static int hns_error_inject_set(void *data, u64 val)
{
	if (!hns_error_type) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}
	mutex_lock(&einj_mutex);
	val = hns_errors[hns_error_type - 1].reg_ecc_einj_en_val;
	val = hns_errors[hns_error_type - 1].reg_ecc_einj_en_val;
	writel(val, hns_errors[0].reg_base + HNS_SRAM_ECC_CHK_EN);
	writel(0x11, hns_errors[0].reg_base + HNS_SRAM_ECC_CHK0);
	mutex_unlock(&einj_mutex);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(hns_error_inject_fops, NULL,
			hns_error_inject_set, "%llu\n");
#endif

/* Cache einj functions */
static int cache_available_error_type_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cache_errors); i++)
		seq_printf(m, "0x%x\t%s\n",
			   cache_errors[i].val,
			   cache_errors[i].type);
	return 0;
}

static int cache_available_error_type_open(struct inode *inode,
					   struct file *file)
{
	return single_open(file, cache_available_error_type_show, NULL);
}

static const struct file_operations cache_avl_err_type_fops = {
	.open           = cache_available_error_type_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int cache_error_type_get(void *data, u64 *val)
{
	*val = cache_error_type;

	return 0;
}

static int cache_error_type_set(void *data, u64 val)
{
	if (val < 0x1 || val > ARRAY_SIZE(cache_errors)) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	cache_error_type = val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(cache_error_type_fops, cache_error_type_get,
			cache_error_type_set, "0x%llx\n");

void print_reg(unsigned long long  reg)
{
	pr_debug("0x%llx", reg);
}

void inject_l1_cache_ce_error(void)
{
	int i;

	pr_debug("L1 error injection, CE\n");
	for (i = 0; i < 4; i++) {
		asm("LDR x1, =inject_l1_cache_ce_error;"
		    "MRS x4, s3_1_C15_C2_5;"
		    "ORR x4, x4, #(1<<32);"
		    "ORR x4, x4, #(1<<23);"
		    "MSR s3_1_C15_C2_5, x4;"/* CPUACTLR_EL1 */
		    "MRS x4, S3_1_c15_c6_4;"
		    "ORR x4, x4, #0x1;"
		    "MSR S3_1_c15_c6_4, x4;"/* CPUPRFCTLR_EL1 */
		    "DSB SY;"
		    "ISB;"
		    "PRFM PLDL1KEEP, [x1];" /* x0 virtual address */
		    "DSB SY;"
		    "MRS x0, S3_1_c15_c7_4;"
		    "ORR x0, x0, #0x1;"
		    "MSR S3_1_c15_c7_4, x0;"
		    "ISB;"
		    "LDR x1, =inject_l1_cache_ce_error;"
		    "LDR x2, [x1];"
		);
		pr_debug("L1 CE error injection,count=%d\n", i);
		msleep(1);
	}

	asm("LDR x1,=inject_l1_cache_ce_error;"
	    "LDR x2, [x1];"/* access memory */
	   );
}

void inject_l1_cache_ue_error(void)
{
	int i;

	pr_debug("L1 error injection,UE\n");
	for (i = 0; i < 4; i++) {
		asm("LDR x1, =inject_l1_cache_ue_error;"
		    "MRS x4, s3_1_C15_C2_5;"
		    "ORR x4, x4, #(1<<32);"
		    "ORR x4, x4, #(1<<23);"
		    "MSR s3_1_C15_C2_5, x4;" /* CPUACTLR_EL1 */
		    "MRS x4, S3_1_c15_c6_4;"
		    "ORR x4, x4, #0x1;"
		    "MSR S3_1_c15_c6_4, x4;"/* CPUPRFCTLR_EL1 */
		    "DSB SY;"
		    "ISB;"
		    "PRFM PLDL1KEEP, [x1];" /* x0 virtual address */
		    "DSB SY;"
		    "MRS x0, S3_1_c15_c7_4;"
		    "ORR x0, x0, #0x1;"
		    "ORR x0, x0, #(1<<7);"
		    "MSR S3_1_c15_c7_4, x0;"
		    "ISB;"
		    "LDR x1, =inject_l1_cache_ue_error;"
		    "LDR x2, [x1];"
		);
		pr_debug("L1 ue error injection,count=%d\n", i);
		msleep(1);
	}

	asm("LDR x1,=inject_l1_cache_ue_error;"
	    "LDR x2, [x1];"/* access memory */
	);
}


void inject_l2_cache_ce_error(void)
{
	int i;

	pr_debug("L2 error injection, CE\n");
	for (i = 0; i < 4; i++) {
		asm("LDR x1,=inject_l2_cache_ce_error;"
		    "MRS x4, s3_1_C15_C2_5;"
		    "ORR x4, x4, #(1<<32);"
		    "ORR x4, x4, #(1<<23);"
		    "MSR s3_1_C15_C2_5, x4;" /* CPUACTLR_EL1 */
		    "MRS x4, S3_1_c15_c6_4;"
		    "ORR x4, x4, #0x1;"
		    "MSR S3_1_c15_c6_4, x4;"/* CPUPRFCTLR_EL1 */
		    "ISB;"
		    "DSB SY;"
		    "DC	CIVAC , x1;" /* x0 virtual address */
		    "DSB SY;"
		    "MRS x0, S3_1_c15_c5_1;"
		    /* bit 7=1,double bit,bit 0=1,error injector is enabled */
		    "ORR x0, x0, #0x1;"
		    "MSR S3_1_c15_c5_1, x0;"/* L2_ERR_INJ_EL1 */
		    "PRFM PLDL2KEEP, [x1];" /* x0 virtual address */
		    "DSB SY;"
		    "LDR x1,=inject_l2_cache_ce_error;"
		    "LDR x2, [x1];"/* access memory */
		    "MOV x0, #0x3;"
		    "MSR S3_0_c5_c3_1, x0;"/* config ERRSELR_EL1.SEL=3 */
		    "MRS x0, s3_0_c5_c5_0;"
		    /* print err3misc0_el1 */
		    "BL	print_reg;"
		);
		pr_debug("L2 CE error injection,count=%d\n", i);
		msleep(1);
	}

	asm("LDR x1,=inject_l2_cache_ce_error;"
	    "LDR x2, [x1];"/* access memory */
	);
}

void inject_l2_cache_ue_error(void)
{
	int i;

	pr_debug("L2 error injection, UE\n");
	for (i = 0; i < 4; i++) {
		asm("LDR x1, =inject_l2_cache_ue_error;"
		    "MRS x4, s3_1_C15_C2_5;"
		    "ORR x4, x4, #(1<<32);"
		    "ORR x4, x4, #(1<<23);"
		    "MSR s3_1_C15_C2_5, x4;"/* CPUACTLR_EL1 */
		    "MRS x4, S3_1_c15_c6_4;"
		    "ORR x4, x4, #0x1;"
		    "MSR S3_1_c15_c6_4, x4;"/* CPUPRFCTLR_EL1 */
		    "ISB;"
		    "DSB SY;"
		    "DC	CIVAC, x1;" /* x0 virtual address */
		    "DSB SY;"
		    "MRS x0, S3_1_c15_c5_1;"
		    /* bit 7=1,double bit,bit 0=1,error injector is enabled */
		    "ORR x0, x0, #0x1;"
		    "ORR x0, x0, #(1<<7);"
		    "MSR S3_1_c15_c5_1, x0;"/* L2_ERR_INJ_EL1 */
		    /* ".long 0xD503221F;" */  /* ESB */
		    "PRFM PLDL2KEEP, [x1];" /* x0 virtual address */
		    "DSB SY;"
		    "LDR x1,=inject_l2_cache_ue_error;"
		    "LDR x2, [x1];"/*access memory */
		    "MOV x0, #0x3;"
		    "MSR S3_0_c5_c3_1, x0;" /* config ERRSELR_EL1.SEL=3 */
		    /* "MRS x0, S3_0_c5_c3_1;" */ /* print ERRSELR_EL1 */
		    /*"BL print_reg;" */
		    "MRS x0, s3_0_c5_c5_0;"/* print err3misc0_el1 */
		    "BL print_reg;"
		);
		pr_debug("L2 UE error injection,count=%d\n", i);
		msleep(1);
	}
	asm("LDR x1,=inject_l2_cache_ue_error;"
	    "LDR x2, [x1];"/* access memory */
	);
}

static int cache_error_inject_set(void *data, u64 val)
{
	void __iomem *reg_base;
	u64 base_addr;
	u64 reg_offset;
	u64 map_size;

	if (!cache_error_type) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	mutex_lock(&einj_mutex);
	val = cache_errors[cache_error_type - 1].reg_ecc_einj_en_val;

	if (cache_error_type <= 2) {
		base_addr = L3D_REG_BASE;
		map_size = L3D_REG_MAP_SIZE;
		reg_offset =  L3D_ECC_INJECT_LEFT;
	} else if (cache_error_type <= 6) {
		base_addr = L3T_REG_BASE;
		map_size = L3T_REG_MAP_SIZE;
		reg_offset =  L3T_DFX_CTRL;
	} else if (cache_error_type == 7) {
		inject_l1_cache_ce_error();
	} else if (cache_error_type == 8) {
		inject_l1_cache_ue_error();
	} else if (cache_error_type == 9) {
		inject_l2_cache_ce_error();
	} else if (cache_error_type == 10) {
		inject_l2_cache_ue_error();
	} else {
		pr_err("%s: invalid error type\n", __func__);
		return -EINVAL;
	}

	if (cache_error_type <= 6) {
		reg_base = ioremap_nocache(base_addr, map_size);
		if (reg_base == NULL) {
			pr_err("%s: cannot remap registers\n", __func__);
			return -ENOMEM;
		}
		writel(val, reg_base + reg_offset);
		iounmap(reg_base);
	}
	mutex_unlock(&einj_mutex);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(cache_error_inject_fops, NULL,
			cache_error_inject_set, "%llu\n");

/* SMMU functions */
static int smmu_available_error_type_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(smmu_errors); i++)
		seq_printf(m, "0x%x\t%s\n",
			   smmu_errors[i].val,
			   smmu_errors[i].type);
	return 0;
}

static int smmu_available_error_type_open(struct inode *inode,
					  struct file *file)
{
	return single_open(file, smmu_available_error_type_show, NULL);
}

static const struct file_operations smmu_avl_err_type_fops = {
	.open           = smmu_available_error_type_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int smmu_error_type_get(void *data, u64 *val)
{
	*val = smmu_error_type;

	return 0;
}

static int smmu_error_type_set(void *data, u64 val)
{
	if (val < 0x1 || val > ARRAY_SIZE(smmu_errors)) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	smmu_error_type = val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(smmu_error_type_fops, smmu_error_type_get,
			smmu_error_type_set, "0x%llx\n");

static int smmu_error_inject_set(void *data, u64 val)
{
	void __iomem *reg_base;

	if (!smmu_error_type) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	mutex_lock(&einj_mutex);
	val = smmu_errors[smmu_error_type - 1].reg_ecc_einj_en_val;

	reg_base = ioremap_nocache(smmu_errors[sas_error_type - 1].base_addr,
				   SMMU_REG_MAP_SIZE);
	if (reg_base == NULL) {
		pr_err("%s: cannot remap registers\n", __func__);
		return -ENOMEM;
	}
	writel(val, reg_base + SMMU_ECC_INJECT);
	iounmap(reg_base);
	mutex_unlock(&einj_mutex);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(smmu_error_inject_fops, NULL,
			smmu_error_inject_set, "%llu\n");

/* HHA functions */
static int hha_available_error_type_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hha_errors); i++)
		seq_printf(m, "0x%x\t%s\n",
			   hha_errors[i].val,
			   hha_errors[i].type);
	return 0;
}

static int hha_available_error_type_open(struct inode *inode, struct file *file)
{
	return single_open(file, hha_available_error_type_show, NULL);
}

static const struct file_operations hha_avl_err_type_fops = {
	.open           = hha_available_error_type_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int hha_error_type_get(void *data, u64 *val)
{
	*val = hha_error_type;

	return 0;
}

static int hha_error_type_set(void *data, u64 val)
{
	if (val < 0x1 || val > ARRAY_SIZE(hha_errors)) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	hha_error_type = val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(hha_error_type_fops, hha_error_type_get,
			hha_error_type_set, "0x%llx\n");

static int hha_error_inject_set(void *data, u64 val)
{
	if (!hha_error_type) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	mutex_lock(&einj_mutex);
	val = hha_errors[hha_error_type - 1].reg_ecc_einj_en_val;
	writel(val, hha_errors[0].reg_base + REG_HHA_ECC_INJECT);
	mutex_unlock(&einj_mutex);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(hha_error_inject_fops, NULL,
			hha_error_inject_set, "%llu\n");


/* HLLC functions */
static int hllc_available_error_type_show(struct seq_file *m, void *v)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hllc_errors); i++)
		seq_printf(m, "0x%x\t%s\n",
			   hllc_errors[i].val,
			   hllc_errors[i].type);
	return 0;
}

static int hllc_available_error_type_open(struct inode *inode,
					  struct file *file)
{
	return single_open(file, hllc_available_error_type_show, NULL);
}

static const struct file_operations hllc_avl_err_type_fops = {
	.open           = hllc_available_error_type_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int hllc_error_type_get(void *data, u64 *val)
{
	*val = hllc_error_type;

	return 0;
}

static int hllc_error_type_set(void *data, u64 val)
{
	if (val < 0x1 || val > ARRAY_SIZE(hllc_errors)) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	hllc_error_type = val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(hllc_error_type_fops, hllc_error_type_get,
			hllc_error_type_set, "0x%llx\n");

static int hllc_error_inject_set(void *data, u64 val)
{
	if (!hllc_error_type) {
		pr_err("%s: invalid error type set", __func__);
		return -EINVAL;
	}

	mutex_lock(&einj_mutex);
	val = hllc_errors[hllc_error_type - 1].reg_ecc_einj_en_val;
	if (hllc_error_type == 0x1) {
		/* 1 bit CRC */
		writel(val, hllc_errors[0].reg_base +
		       REG_HLLC_PHY_TX_INJECT_CRC_EN);
		writel(0x01, hllc_errors[0].reg_base +
		       REG_HLLC_PHY_TX_INJECT_CRC_TIMES);
	} else if (hllc_error_type <= 0x5) {
		/* 1 bit ECC */
		writel(val, hllc_errors[0].reg_base +
		       REG_HLLC_INJECT_ECC_EN);
		writel(0x01, hllc_errors[0].reg_base +
		       REG_HLLC_INJECT_ECC_TYPE);
	} else {
		/* 2 bit ECC */
		writel(val, hllc_errors[0].reg_base +
		       REG_HLLC_INJECT_ECC_EN);
		writel(0x11, hllc_errors[0].reg_base +
		       REG_HLLC_INJECT_ECC_TYPE);
	}
	mutex_unlock(&einj_mutex);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(hllc_error_inject_fops, NULL,
			hllc_error_inject_set, "%llu\n");

/* common structures and functions */
struct dfs_node {
	char *name;
	struct dentry *d;
	const struct file_operations *avl_err_types_fops;
	const struct file_operations *einj_type_fops;
	const struct file_operations *einj_fops;
};

static struct dfs_node dfs_dirs[] = {
	{
		.name = "sas",
		.avl_err_types_fops = &sas_avl_err_type_fops,
		.einj_type_fops = &sas_error_type_fops,
		.einj_fops = &sas_error_inject_fops,
	},
	/*{
		.name = "hns",
		.avl_err_types_fops = &hns_avl_err_type_fops,
		.einj_type_fops = &hns_error_type_fops,
		.einj_fops = &hns_error_inject_fops,
	},*/
	{
		.name = "cache",
		.avl_err_types_fops = &cache_avl_err_type_fops,
		.einj_type_fops = &cache_error_type_fops,
		.einj_fops = &cache_error_inject_fops,
	},
	{
		.name = "smmu",
		.avl_err_types_fops = &smmu_avl_err_type_fops,
		.einj_type_fops = &smmu_error_type_fops,
		.einj_fops = &smmu_error_inject_fops,
	},
	{
		.name = "hha",
		.avl_err_types_fops = &hha_avl_err_type_fops,
		.einj_type_fops = &hha_error_type_fops,
		.einj_fops = &hha_error_inject_fops,
	},
	{
		.name = "hllc",
		.avl_err_types_fops = &hllc_avl_err_type_fops,
		.einj_type_fops = &hllc_error_type_fops,
		.einj_fops = &hllc_error_inject_fops,
	},
};

static int __init init_hip08_einj(void)
{
	int i;
	struct dentry *fentry;

	/* map hw register base */
	sas_errors[0].reg_base = ioremap_nocache(SAS_REG_BASE,
						 SAS_REG_MAP_SIZE);
	if (sas_errors[0].reg_base == NULL) {
		pr_err("%s: cannot remap SAS registers\n", __func__);
		return -ENOMEM;
	}

	/*
	 * hns_errors[0].reg_base =
			ioremap_nocache(HNS_REG_BASE, HNS_REG_MAP_SIZE);
	 * if (hns_errors[0].reg_base == NULL) {
	 *	pr_err("%s: cannot remap HNS registers\n", __func__);
		iounmap(sas_errors[0].reg_base);
	 *	return -ENOMEM;
	 * }
	 */

	hha_errors[0].reg_base = ioremap_nocache(HHA_REG_BASE,
						 HHA_REG_MAP_SIZE);
	if (hha_errors[0].reg_base == NULL) {
		pr_err("%s: cannot remap HHA registers\n", __func__);
		/*iounmap(hns_errors[0].reg_base);*/
		iounmap(sas_errors[0].reg_base);
		return -ENOMEM;
	}

	hllc_errors[0].reg_base = ioremap_nocache(HLLC_REG_BASE,
						  HLLC_REG_MAP_SIZE);
	if (hllc_errors[0].reg_base == NULL) {
		pr_err("%s: cannot remap HLLC registers\n", __func__);
		iounmap(hha_errors[0].reg_base);
		/*iounmap(hns_errors[0].reg_base);*/
		iounmap(sas_errors[0].reg_base);
		return -ENOMEM;
	}

	dfs_inj = debugfs_create_dir("hip08-einj", NULL);
	if (!dfs_inj) {
		iounmap(hllc_errors[0].reg_base);
		iounmap(hha_errors[0].reg_base);
		/*iounmap(hns_errors[0].reg_base);*/
		iounmap(sas_errors[0].reg_base);
		return -ENOMEM;
	}

	for (i = 0; i < ARRAY_SIZE(dfs_dirs); i++) {
		dfs_dirs[i].d = debugfs_create_dir(dfs_dirs[i].name, dfs_inj);
		if (dfs_dirs[i].d == NULL)
			goto cleanup;

		fentry = debugfs_create_file("available_error_type", 0400,
					     dfs_dirs[i].d, NULL,
					     dfs_dirs[i].avl_err_types_fops);
		if (!fentry)
			goto cleanup;

		fentry = debugfs_create_file("error_type", 0600,
					     dfs_dirs[i].d, NULL,
					     dfs_dirs[i].einj_type_fops);
		if (!fentry)
			goto cleanup;

		fentry = debugfs_create_file("error_inject", 0200,
					     dfs_dirs[i].d, NULL,
					     dfs_dirs[i].einj_fops);
		if (!fentry)
			goto cleanup;
	}
	return 0;

cleanup:
	pr_err("%s failed\n", __func__);
	debugfs_remove_recursive(dfs_inj);
	dfs_inj = NULL;
	iounmap(hllc_errors[0].reg_base);
	iounmap(hha_errors[0].reg_base);
	/*iounmap(hns_errors[0].reg_base);*/
	iounmap(sas_errors[0].reg_base);
	return -ENOMEM;
}

static void __exit exit_hip08_einj(void)
{
	debugfs_remove_recursive(dfs_inj);
	dfs_inj = NULL;

	iounmap(hllc_errors[0].reg_base);
	iounmap(hha_errors[0].reg_base);
	/*iounmap(hns_errors[0].reg_base);*/
	iounmap(sas_errors[0].reg_base);
}
module_init(init_hip08_einj);
module_exit(exit_hip08_einj);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shiju Jose <shiju.jose@huawei.com>");
MODULE_AUTHOR("Huawei");
MODULE_DESCRIPTION("HISI HIP08 error injection facility for testing non-standard errors");

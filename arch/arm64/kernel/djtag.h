#ifndef _LINUX_DJTAG_H
#define _LINUX_DJTAG_H

/* define read register: read_reg
 * define write register:write_reg
 */
#define write_reg(addr, value) (*(volatile u32*)(addr) = value)
#define read_reg(addr, value) (*(value) = *(volatile u32*)(addr))

/* define the offset of djtag's registers for PV660
 * PV660:TOTEMv1+NIMBUSv1(t1n1)
 */
#define t1n1_djtag_mstr_start_en_reg        (0x6804)
#define t1n1_djtag_sec_acc_en_reg           (0x6808)
#define t1n1_djtag_debug_module_sel_reg     (0x680c)
#define t1n1_djtag_mstr_wr_reg              (0x6810)
#define t1n1_djtag_chain_unit_cfg_en_reg    (0x6814)
#define t1n1_djtag_mstr_addr_reg            (0x6818)
#define t1n1_djtag_mstr_data_reg            (0x681c)
#define t1n1_djtag_rd_data0_reg             (0xE800)
#define t1n1_djtag_rd_data1_reg             (0xE804)
#define t1n1_djtag_rd_data2_reg             (0xE808)
#define t1n1_djtag_rd_data3_reg             (0xE80C)
#define t1n1_djtag_rd_data4_reg             (0xE810)
#define t1n1_djtag_rd_data5_reg             (0xE814)
#define t1n1_djtag_rd_data6_reg             (0xE818)
#define t1n1_djtag_rd_data7_reg             (0xE81C)
#define t1n1_djtag_rd_data8_reg             (0xE820)
#define t1n1_djtag_rd_data9_reg             (0xE824)

/* define the union for the djtag interface
 *
 */
union t1n1_djtag_mstr_en {
	struct {
		u32 djtag_mstr_en       : 1;
		u32 djtag_nor_cfg_en    : 1;
		u32 reserved            : 30;
	};
	u32 mstr;
};

/* define the necessary marco
 *
 */
#define DJTAG_FINISH            0x0
#define DJTAG_START             0x1
#define DJTAG_MSTR_READ         0x0
#define DJTAG_MSTR_WRITE        0x1

int hisi_djtag_readreg(u32, u32, u32, u64, u32 *);
int hisi_djtag_writereg(u32, u32, u32, u32, u64);
#endif /* _LINUX_DJTAG_H */

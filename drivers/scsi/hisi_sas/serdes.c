
/*************************************************************
	 begin
***********************************************************/
#ifdef HRD_OS_LINUX
#define HILINK_REG_SIZE	(0x40000)
#define SRE_printf printk
static inline void SRE_DelayUs(unsigned int time)
{
	if (time >= 1000) {
		mdelay(time/1000);
		time = (time%1000);
	}
	if (time != 0)
		udelay(time);
}
#else
extern void SRE_DelayUs(unsigned int uwDelay);
#endif

#include "higgs_common.h"
extern struct HIGGS_BOARD_SAS_CONFIG_S g_stHiggsBoardSasCfg;

/*l00290354 add serdes struct
hilink0EM_HILINK0_PCIE1_4LANE_PCIE2_4LANE
hilink2EM_HILINK2_SAS0_8LANE
*/
enum hilink0_mode_type_e {
	EM_HILINK0_PCIE1_8LANE = 0,
	EM_HILINK0_PCIE1_4LANE_PCIE2_4LANE = 1,
};

enum hilink1_mode_type_e {
	EM_HILINK1_PCIE0_8LANE = 0,
	EM_HILINK1_HCCS_8LANE = 1,
};

enum hilink2_mode_type_e {
	EM_HILINK2_PCIE2_8LANE = 0,
	EM_HILINK2_SAS0_8LANE = 1,
};

enum hilink3_mode_type_e {
	EM_HILINK3_GE_4LANE = 0,
	EM_HILINK3_GE_2LANE_XGE_2LANE = 1, /*lane0,lane1-ge,lane2,lane3 xge*/
};


enum hilink4_mode_type_e {
	EM_HILINK4_GE_4LANE = 0,
	EM_HILINK4_XGE_4LANE = 1,
};

enum hilink5_mode_type_e {
	EM_HILINK5_SAS1_4LANE = 0,
	EM_HILINK5_PCIE3_4LANE = 1,
};

enum board_type_e {
	EM_32CORE_EVB_BOARD = 0,
	EM_16CORE_EVB_BOARD = 1,
	EM_V2R1CO5_BOARD = 2,
	EM_OTHER_BOARD			/*.2P*/
};


struct serdes_param_t {
	enum hilink0_mode_type_e hilink0_mode;
	enum hilink1_mode_type_e hilink1_mode;
	enum hilink2_mode_type_e hilink2_mode;
	enum hilink3_mode_type_e hilink3_mode;
	enum hilink4_mode_type_e hilink4_mode;
	enum hilink5_mode_type_e hilink5_mode;
	/*board_type_e board_type;*/
};


enum serdes_ret_e {
	EM_SERDES_SUCCESS = 0,
	EM_SERDES_FAIL = 1,
};


#define CsCalibTime 10
#define DsCalibTime 20

/*Firmware*/
#define MCU_CFG_OFFSET_ADDR				 ((0xFFF0)*2)
#define FIRMWARE_DOWNLOAD_OFFSET_ADDR	 ((0xC000)*2)
#define CUSTOMER_SPACE_START_OFFSET_ADDR	((0xFF6C)*2)
#define CUSTOMER_SPACE_END_OFFSET_ADDR	 ((0xFFEE)*2)
#define CS_DS_NUM_SEARCH_OFFSET_ADDR		((0xFFEE)*2)
#define CS_CALIBRATION_ENABLE_OFFSET_ADDR ((0xFFED)*2)
#define DS_CTLE_CONTROL1_OFFSET_ADDR(i)	 ((0xFFE4-(i)*8)*2)
#define DS_CTLE_CONTROL0_OFFSET_ADDR(i)	 ((0xFFE6-(i)*8)*2)
#define SRB_SYS_CTRL_BASE (0xf3e00000)
/*CS */
#define CS_CALIB_MAX_Time						(10)


#define SRE_Hilink_INIT_ERROR		 1
#define SRE_Hilink_READ_REG_ERROR	 1
#define SRE_Hilink_WRITE_REG_ERROR	 1
#define SRE_Hilink_PARAMETER_ERROR	 1
#define SRE_Hilink_CALL_FUN_ERROR	 1
#define M_common_OK							 0
#define SRE_Hilink_CS_Calib_ERROR		 1
#define SRE_Hilink_DS_Calibration_ERROR 1

/*Macro number*/
#define MACRO_0 0
#define MACRO_1 1
#define MACRO_2 2
#define MACRO_3 3
#define MACRO_4 4
#define MACRO_5 5
#define MACRO_6 6


#define SRE_OK		 0
#define SRE_ERROR	 0xffffffff


/* CS number*/
#define CS0 0
#define CS1 1
#define INVALID_CS_PARA 2
/* DS number*/
#define DS0 0
#define DS1 1
#define DS2 2
#define DS3 3
#define DS4 4
#define DS5 5
#define DS_ALL 0xff
#define INVALID_DS_PARA 6

/*line rate mode */
#define GE_1250	0
#define GE_3125	1
#define XGE_10312 2
#define PCIE_2500 3
#define PCIE_5000 4
#define PCIE_8000 5
#define SAS_1500 6
#define SAS_3000 7
#define SAS_6000 8
#define SAS_12000 9
#define HCCS_32	10
#define HCCS_40	11

#define PMA_MODE_PCIE	 0
#define PMA_MODE_SAS	 1
#define PMA_MODE_NORMAL	2

/*PRBS */
#define PRBS7 0
#define PRBS9 1
#define PRBS10 2
#define PRBS11 3
#define PRBS15 4
#define PRBS20 5
#define PRBS23 6
#define PRBS31 7
#define PRBS_MODE_CUSTOM 8

/*AC or DC couple mode flag AC0 DC1*/
#define AcModeFlag 0
#define DcModeFlag 1
#define CoupleModeFlag AcModeFlag

#define Macro0SlicesNum 10
#define Macro1SlicesNum 10
#define Macro2SlicesNum 10
#define Macro3SlicesNum 6
#define Macro4SlicesNum 6
#define Macro5SlicesNum 6
#define Macro6SlicesNum 6


/*serdes CS/DS_CLK/TX/RX*/
#define MAX_CS_CSR_NUM 61
#define MAX_DSCLK_CSR_NUM 31
#define MAX_TX_CSR_NUM 62
#define MAX_RX_CSR_NUM 63
#define CP_CSR_NUM 7

#define CS_CSR(num, reg)	 (((0x0000+(reg)*0x0002+(num)*0x0200))*2)
#define DSCLK_CSR(lane, reg)	(((0x4100+(reg)*0x0002+(lane)*0x0200))*2)
#define RX_CSR(lane, reg)	 (((0x4080+(reg)*0x0002+(lane)*0x0200))*2)
#define TX_CSR(lane, reg)	 (((0x4000+(reg)*0x0002+(lane)*0x0200))*2)
#define CP_CSR(reg)			(((0x0FFF0+(reg)*0x0002))*2)

#define REG_BROADCAST		 (0x2000 * 2)
#define VERSION_OFFSET		 (0x1800c * 2)
#define FIRMWARE_BASE		 (0x18000 * 2)
#define API_BASE			 (0x1FF6C * 2)
#define API_END				(0x20000 * 2)
#define CS_API				 (0xFFEC * 2)
#define DS_API(lane)		 ((0x1FF6c + 8*(15-lane))*2)
#define Firmware_Size		 (32620)
#define MCU_MASK			 (0x8000 * 2)


#define FULL_RATE	0x0
#define HALF_RATE	0x1
#define QUARTER_RATE 0x2

#define Width_40bit 0x3
#define Width_32bit 0x2
#define Width_20bit 0x1
#define Width_16bit 0x0

#define Ka2_Kb1 0x0
#define Ka2_Kb2 0x2
#define Ka3_Kb1 0x1
#define Ka3_Kb2 0x3


#define J_DIV_20 0x0
#define J_DIV_10 0x1
#define J_DIV_16 0x2
#define J_DIV_8 0x3


#define J_DIV_NO_PLUS2 0
#define J_DIV_PLUS2	1


enum LINERATE_CFG_ENUM {
	LineRate_GE_1250 = 0,
	LineRate_GE_3125,
	LineRate_XGE_10312,
	LineRate_PCIE_2500,
	LineRate_PCIE_5000,
	LineRate_PCIE_8000,
	LineRate_SAS_1500,
	LineRate_SAS_3000,
	LineRate_SAS_6000,
	LineRate_SAS_12000,
	LineRate_HCCS_12000,
	INVALID_LINERATE_CFG
};

/**/
struct LINE_RATE_CFG {
	/*LINERATE_CFG_ENUM rate;*/
	unsigned int number;
};


void SRE_SdsRegWrite(unsigned int ulMacroId,
	unsigned int ulRegAddrOffset,
	unsigned int ulRegValue);
void SRE_SdsRegBitsWrite(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulRegAddrOffset,
	unsigned int ulHigtBit,
	unsigned int ulLowBit,
	unsigned int ulRegValue);
unsigned int SRE_SdsRegBitsRead(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulRegAddrOffset,
	unsigned int ulHigtBit,
	unsigned int ulLowBit);
void SRE_SerdesCsCfg(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulCsNum,
	unsigned int CsCfg);
unsigned int SRE_SerdesCsCalib(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulCsNum);
void SRE_SerdesDsCfg(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulDsNum,
	unsigned int ulDsCfg,
	unsigned int ulCsSrc);
unsigned int SRE_SerdesDsCalib(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulDsNum,
	unsigned int ulDsCfg);
void Custom_Wave(unsigned int macro,
	unsigned int lane,
	unsigned int mode);
void SRE_DsCalibAdjust(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulDsNum);
unsigned int RegBitsRead(unsigned int node,
	unsigned int addr,
	unsigned int ulHigtBit,
	unsigned int ulLowBit);
void RegBitsWrite(unsigned int node,
	unsigned int addr,
	unsigned int ulHigtBit,
	unsigned int ulLowBit,
	unsigned int ulRegValue);
/*void Serdes_Delay_Us(UINT32 time);*/
void SRE_SdsRegWriteByCSR(unsigned int macro,
	unsigned int module,
	unsigned int lane,
	unsigned int CSR,
	unsigned int data);
void SRE_DsHwCalibrationInit(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulDsNum);
unsigned int SRE_DsHwCalibrationExec(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulDsNum);
void SRE_DsHwCalibrationAdjust(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulDsNum);
void SRE_DsConfigurationAfterCalibration(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulDsNum,
	unsigned int ulDsCfg,
	unsigned int ulCoupleFlag);
void SRE_HilinkFirmwareRun(unsigned int node,
	unsigned int macro,
	unsigned int lane,
	unsigned int ulDsCfg);
void SRE_RunFirmware(unsigned int node, unsigned int macro);

/* begin:2P arm server */
#define HILINK_REG_BASE_OFFSET		(0x40000000000ULL)

#define MASTER_CPU_NODE		(0)
#define SLAVE_CPU_NODE		(1)

#define HILINKMACRO_SIZE (HILINK_REG_SIZE)
#define HILINK_NAME "hilink"

#define HILINKMACRO0	 (0xB2000000)
#define HILINKMACRO1	 (0xB2080000)
#define HILINKMACRO2	 (0xB2100000)
#define HILINKMACRO3	 (0xC2200000)
#define HILINKMACRO4	 (0xC2280000)
#define HILINKMACRO5	 (0xB2180000)
#define HILINKMACRO6	 (0xB2200000)

#define SYS_CTRL_KERNEL	(0xF3000000+0xE00000)
#define SYS_CTRL_UP		(0x0f28f5000)
#define SERDES_CTRL0	 (0x0F28F4000)
#define SERDES_CTRL1	 (0x0F7802000)

#define HILINK0_MACRO_PWRDB	(0xB0002418)
#define HILINK1_MACRO_PWRDB	(0xB0002518)
#define HILINK2_MACRO_PWRDB	(0xC0002418)
#define HILINK3_MACRO_PWRDB	(0xC0002518)
#define HILINK4_MACRO_PWRDB	(0xC0002618)
#define HILINK5_MACRO_PWRDB	(0xB0002618)
#define HILINK6_MACRO_PWRDB	(0xB0002718)

#define HILINK0_MACRO_GRSTB	(0xB000241C)
#define HILINK1_MACRO_GRSTB	(0xB000251C)
#define HILINK2_MACRO_GRSTB	(0xC000241C)
#define HILINK3_MACRO_GRSTB	(0xC000251C)
#define HILINK4_MACRO_GRSTB	(0xC000261C)
#define HILINK5_MACRO_GRSTB	(0xB000261C)
#define HILINK6_MACRO_GRSTB	(0xB000271C)


#define HILINK0_MUX_CTRL		(0xB0002300)
#define HILINK1_MUX_CTRL		(0xB0002304)
#define HILINK2_MUX_CTRL		(0xB0002308)
#define HILINK5_MUX_CTRL		(0xB0002314)

#define SRE_SAS0_DSAF_CFG_BASE (0xC0000000)
#define SRE_SAS1_PCIE_CFG_BASE (0xB0000000)

#define SRE_HILINK3_CRG_CTRL0_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x180) /* HILINK CRG */
#define SRE_HILINK3_CRG_CTRL1_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x184) /* HILINK CRG */
#define SRE_HILINK3_CRG_CTRL2_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x188) /* HILINK CRG */
#define SRE_HILINK3_CRG_CTRL3_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x18C) /* HILINK CRG */
#define SRE_HILINK4_CRG_CTRL0_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x190) /* HILINK CRG */
#define SRE_HILINK4_CRG_CTRL1_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x194) /* HILINK CRG */
#define SRE_HILINK2_LRSTB_MUX_CTRL_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2340) /* HILINK2 lrstb[7:0]MUX */
#define SRE_HILINK2_MACRO_SS_REFCLK_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2400) /* HILINK */
#define SRE_HILINK2_MACRO_CS_REFCLK_DIRSEL_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2404) /* HILINK */
#define SRE_HILINK2_MACRO_LIFECLK2DIG_SEL_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2408) /* HILINK */
#define SRE_HILINK2_MACRO_CORE_CLK_SELEXT_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x240C) /* HILINK */
#define SRE_HILINK2_MACRO_CORE_CLK_SEL_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2410) /* HILINK */
#define SRE_HILINK2_MACRO_CTRL_BUS_MODE_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2414) /* HILINK */
#define SRE_HILINK2_MACRO_MACROPWRDB_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2418) /* HILINK */
#define SRE_HILINK2_MACRO_GRSTB_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x241C) /* HILINK */
#define SRE_HILINK2_MACRO_BIT_SLIP_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2420) /* HILINK */
#define SRE_HILINK2_MACRO_LRSTB_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2424) /* HILINK */
#define SRE_HILINK3_MACRO_SS_REFCLK_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2500) /* HILINK */
#define SRE_HILINK3_MACRO_CS_REFCLK_DIRSEL_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2504) /* HILINK */
#define SRE_HILINK3_MACRO_LIFECLK2DIG_SEL_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2508) /* HILINK */
#define SRE_HILINK3_MACRO_CORE_CLK_SELEXT_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x250C) /* HILINK */
#define SRE_HILINK3_MACRO_CORE_CLK_SEL_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2510) /* HILINK */
#define SRE_HILINK3_MACRO_CTRL_BUS_MODE_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2514) /* HILINK */
#define SRE_HILINK3_MACRO_MACROPWRDB_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2518) /* HILINK */
#define SRE_HILINK3_MACRO_GRSTB_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x251C) /* HILINK */
#define SRE_HILINK3_MACRO_BIT_SLIP_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2520) /* HILINK */
#define SRE_HILINK3_MACRO_LRSTB_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2524) /* HILINK */
#define SRE_HILINK4_MACRO_SS_REFCLK_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2600) /* HILINK */
#define SRE_HILINK4_MACRO_CS_REFCLK_DIRSEL_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2604) /* HILINK */
#define SRE_HILINK4_MACRO_LIFECLK2DIG_SEL_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2608) /* HILINK */
#define SRE_HILINK4_MACRO_CORE_CLK_SELEXT_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x260C) /* HILINK */
#define SRE_HILINK4_MACRO_CORE_CLK_SEL_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2610) /* HILINK */
#define SRE_HILINK4_MACRO_CTRL_BUS_MODE_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2614) /* HILINK */
#define SRE_HILINK4_MACRO_MACROPWRDB_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2618) /* HILINK */
#define SRE_HILINK4_MACRO_GRSTB_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x261C) /* HILINK */
#define SRE_HILINK4_MACRO_BIT_SLIP_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2620) /* HILINK */
#define SRE_HILINK4_MACRO_LRSTB_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x2624) /* HILINK */
#define SRE_HILINK2_MACRO_PLLOUTOFLOCK_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x6400) /* HILINK */
#define SRE_HILINK2_MACRO_PRBS_ERR_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x6404) /* HILINK */
#define SRE_HILINK2_MACRO_LOS_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x6408) /* HILINK */
#define SRE_HILINK3_MACRO_PLLOUTOFLOCK_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x6500) /* HILINK */
#define SRE_HILINK3_MACRO_PRBS_ERR_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x6504) /* HILINK */
#define SRE_HILINK3_MACRO_LOS_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x6508) /* HILINK */
#define SRE_HILINK4_MACRO_PLLOUTOFLOCK_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x6600) /* HILINK */
#define SRE_HILINK4_MACRO_PRBS_ERR_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x6604) /* HILINK */
#define SRE_HILINK4_MACRO_LOS_REG\
	(SRE_SAS0_DSAF_CFG_BASE + 0x6608) /* HILINK */


#define SRE_HILINK0_MUX_CTRL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2300) /* HILINK */
#define SRE_HILINK1_MUX_CTRL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2304) /* HILINK */
#define SRE_HILINK2_MUX_CTRL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2308) /* HILINK */
#define SRE_HILINK5_MUX_CTRL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2314) /* HILINK */
#define SRE_HILINK1_AHB_MUX_CTRL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2324) /* HILINK AHB */
#define SRE_HILINK2_AHB_MUX_CTRL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2328) /* HILINK AHB */
#define SRE_HILINK5_AHB_MUX_CTRL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2334) /* HILINK AHB */
#define SRE_HILINK5_LRSTB_MUX_CTRL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2340) /* HILINK5 lrstb[3:0]MUX */
#define SRE_HILINK6_LRSTB_MUX_CTRL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2344) /* HILINK6 lrstb[3:0]MUX */
#define SRE_HILINK0_MACRO_SS_REFCLK_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2400) /* HILINK */
#define SRE_HILINK0_MACRO_CS_REFCLK_DIRSEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2404) /* HILINK */
#define SRE_HILINK0_MACRO_LIFECLK2DIG_SEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2408) /* HILINK */
#define SRE_HILINK0_MACRO_CORE_CLK_SELEXT_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x240C) /* HILINK */
#define SRE_HILINK0_MACRO_CORE_CLK_SEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2410) /* HILINK */
#define SRE_HILINK0_MACRO_CTRL_BUS_MODE_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2414) /* HILINK */
#define SRE_HILINK0_MACRO_MACROPWRDB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2418) /* HILINK */
#define SRE_HILINK0_MACRO_GRSTB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x241C) /* HILINK */
#define SRE_HILINK0_MACRO_BIT_SLIP_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2420) /* HILINK */
#define SRE_HILINK0_MACRO_LRSTB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2424) /* HILINK */
#define SRE_HILINK1_MACRO_SS_REFCLK_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2500) /* HILINK */
#define SRE_HILINK1_MACRO_CS_REFCLK_DIRSEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2504) /* HILINK */
#define SRE_HILINK1_MACRO_LIFECLK2DIG_SEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2508) /* HILINK */
#define SRE_HILINK1_MACRO_CORE_CLK_SELEXT_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x250C) /* HILINK */
#define SRE_HILINK1_MACRO_CORE_CLK_SEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2510) /* HILINK */
#define SRE_HILINK1_MACRO_CTRL_BUS_MODE_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2514) /* HILINK */
#define SRE_HILINK1_MACRO_MACROPWRDB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2518) /* HILINK */
#define SRE_HILINK1_MACRO_GRSTB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x251C) /* HILINK */
#define SRE_HILINK1_MACRO_BIT_SLIP_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2520) /* HILINK */
#define SRE_HILINK1_MACRO_LRSTB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2524) /* HILINK */
#define SRE_HILINK5_MACRO_SS_REFCLK_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2600) /* HILINK */
#define SRE_HILINK5_MACRO_CS_REFCLK_DIRSEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2604) /* HILINK */
#define SRE_HILINK5_MACRO_LIFECLK2DIG_SEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2608) /* HILINK */
#define SRE_HILINK5_MACRO_CORE_CLK_SELEXT_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x260C) /* HILINK */
#define SRE_HILINK5_MACRO_CORE_CLK_SEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2610) /* HILINK */
#define SRE_HILINK5_MACRO_CTRL_BUS_MODE_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2614) /* HILINK */
#define SRE_HILINK5_MACRO_MACROPWRDB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2618) /* HILINK */
#define SRE_HILINK5_MACRO_GRSTB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x261C) /* HILINK */
#define SRE_HILINK5_MACRO_BIT_SLIP_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2620) /* HILINK */
#define SRE_HILINK5_MACRO_LRSTB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2624) /* HILINK */
#define SRE_HILINK6_MACRO_SS_REFCLK_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2700) /* HILINK */
#define SRE_HILINK6_MACRO_CS_REFCLK_DIRSEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2704) /* HILINK */
#define SRE_HILINK6_MACRO_LIFECLK2DIG_SEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2708) /* HILINK */
#define SRE_HILINK6_MACRO_CORE_CLK_SELEXT_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x270C) /* HILINK */
#define SRE_HILINK6_MACRO_CORE_CLK_SEL_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2710) /* HILINK */
#define SRE_HILINK6_MACRO_CTRL_BUS_MODE_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2714) /* HILINK */
#define SRE_HILINK6_MACRO_MACROPWRDB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2718) /* HILINK */
#define SRE_HILINK6_MACRO_GRSTB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x271C) /* HILINK */
#define SRE_HILINK6_MACRO_BIT_SLIP_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2720) /* HILINK */
#define SRE_HILINK6_MACRO_LRSTB_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x2724) /* HILINK */
#define SRE_HILINK0_MACRO_PLLOUTOFLOCK_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x6400) /* HILINK */
#define SRE_HILINK0_MACRO_PRBS_ERR_REG\
	(SRE_SAS1_PCIE_CFG_BASE + 0x6404) /* HILINK */
#define SRE_HILINK0_MACRO_LOS_REG\


static unsigned char sUseSSC;
/*0-Normal CDR(default), 1-SSCDR*/
static unsigned char sCDRMode;
static u8 s_hilink1_8G;
/*static board_type_e ebord_type = EM_16CORE_EVB_BOARD;*/

static u8 g_serdes_tx_polarity[2][7][8] = {{{0 } } };
static u8 g_serdes_rx_polarity[2][7][8] = {{{0 } } };

#define DSAF_SUB_BASE (0xC0000000)
#define PCIE_SUB_BASE (0xB0000000)


#define DSAF_SUB_BASE_SLAVE_CPU (0x40000000000ULL + 0xC0000000)

#define PCIE_SUB_BASE_SLAVE_CPU (0x40000000000ULL + 0xB0000000)

#define PCIE_SUB_BASE_SIZE (0x10000)
#define DSAF_SUB_BASE_SIZE (0x10000)

unsigned long long IOMAP_HILINK_MACRO0_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_MACRO1_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_MACRO2_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_MACRO3_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_MACRO4_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_MACRO5_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_MACRO6_BASE_ADDR = 0;

unsigned long long IOMAP_HILINK_SLAVE_MACRO0_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_SLAVE_MACRO1_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_SLAVE_MACRO2_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_SLAVE_MACRO3_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_SLAVE_MACRO4_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_SLAVE_MACRO5_BASE_ADDR = 0;
unsigned long long IOMAP_HILINK_SLAVE_MACRO6_BASE_ADDR = 0;

/*----------------------------------------------*
 *									 *
 *----------------------------------------------*/
unsigned long long sub_pcie_base_addr = 0;
unsigned long long sub_pcie_pa_addr = 0;

/*add by chenqilin*/
unsigned long long sub_pcie_base_addr_slavecpu = 0;
unsigned long long sub_pcie_pa_addr_slavecpu = 0;
/*end*/

unsigned long long sub_dsaf_base_addr = 0;
unsigned long long sub_dsaf_pa_addr = 0;

/*add by chenqilin*/
unsigned long long sub_dsaf_base_addr_slavecpu = 0;
unsigned long long sub_dsaf_pa_addr_slavecpu = 0;
/*end*/



/*****************************************************************************
    : hilink0_reg_init
  : hilink
  : void
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink0_reg_init(void)
{

	IOMAP_HILINK_MACRO0_BASE_ADDR =
		(u64)ioremap(HILINKMACRO0,
			HILINKMACRO_SIZE);
	if (!IOMAP_HILINK_MACRO0_BASE_ADDR) {
		pr_err(HILINK_NAME
			": fail of_iomap HL reg addr:0x%llx, sz:0x%llx\n",
			HILINKMACRO0,
			HILINKMACRO_SIZE);
		return -1;
	}

	/* begin:2P ARM SERVER, CPU */
	if (g_stHiggsBoardSasCfg.uiMaxCpuNode > 1) {
		IOMAP_HILINK_SLAVE_MACRO0_BASE_ADDR =
			(u64)ioremap(HILINKMACRO0 + 0x40000000000ULL,
				HILINKMACRO_SIZE);
		if (!IOMAP_HILINK_SLAVE_MACRO0_BASE_ADDR) {
			/* CPU unmap hilink addr */
			iounmap((void *)IOMAP_HILINK_MACRO0_BASE_ADDR);
			pr_err(HILINK_NAME
				": fail of_iomap SL HL registers addr:0x%llx, sz:0x%llx\n",
				HILINKMACRO0,
				HILINKMACRO_SIZE);
			return -1;
		}
	}
	/* end:2P ARM SERVER */

	return 0;
}

/*****************************************************************************
    : hilink1_reg_init
  : hilink
  : void
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink1_reg_init(void)
{

	IOMAP_HILINK_MACRO1_BASE_ADDR =
		(u64)ioremap(HILINKMACRO1,
			HILINKMACRO_SIZE);
	if (!IOMAP_HILINK_MACRO1_BASE_ADDR) {
		pr_err(HILINK_NAME
			": fail ioremap HL reg addr:0x%llx, sz:0x%llx\n",
			HILINKMACRO1,
			HILINKMACRO_SIZE);
		return -1;
	}
	/* begin:2P ARM SERVER, CPU */
	if (g_stHiggsBoardSasCfg.uiMaxCpuNode > 1) {
		IOMAP_HILINK_SLAVE_MACRO1_BASE_ADDR =
			(u64)ioremap(HILINKMACRO1 + 0x40000000000ULL,
				HILINKMACRO_SIZE);
		if (!IOMAP_HILINK_SLAVE_MACRO1_BASE_ADDR) {
			/* CPU unmap hilink addr */
			iounmap((void *)IOMAP_HILINK_MACRO1_BASE_ADDR);
			pr_err(HILINK_NAME
				": fail of_iomap SL HL ewg addr:0x%llx, sz:0x%llx\n",
				HILINKMACRO1,
				HILINKMACRO_SIZE);
			return -1;
		}
	}
	/* end:2P ARM SERVER */
	return 0;
}

/*****************************************************************************
    : hilink1_reg_init
  : hilink
  : void
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink2_reg_init(void)
{
	IOMAP_HILINK_MACRO2_BASE_ADDR =
		(u64)ioremap(HILINKMACRO2,
			HILINKMACRO_SIZE);
	if (!IOMAP_HILINK_MACRO2_BASE_ADDR) {
		pr_err(HILINK_NAME
			": fail HL reg addr:0x%llx, sz:0x%llx\n",
			HILINKMACRO2,
			HILINKMACRO_SIZE);
		return -1;
	}

	/* begin:2P ARM SERVER, CPU */
	if (g_stHiggsBoardSasCfg.uiMaxCpuNode > 1) {
		IOMAP_HILINK_SLAVE_MACRO2_BASE_ADDR =
			(u64)ioremap(HILINKMACRO2 + 0x40000000000ULL,
			HILINKMACRO_SIZE);
		if (!IOMAP_HILINK_SLAVE_MACRO2_BASE_ADDR) {
			/* CPU unmap hilink addr */
			iounmap((void *)IOMAP_HILINK_MACRO2_BASE_ADDR);
			pr_err(HILINK_NAME
				": fail of_iomap SL HL reg addr:0x%llx, sz:0x%llx\n",
				HILINKMACRO2,
				HILINKMACRO_SIZE);
			return -1;
		}
	}
	/* end:2P ARM SERVER */
	return 0;
}

/*****************************************************************************
    : hilink1_reg_init
  : hilink
  : void
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink3_reg_init(void)
{
	IOMAP_HILINK_MACRO3_BASE_ADDR =
		(u64)ioremap(HILINKMACRO3,
			HILINKMACRO_SIZE);
	if (!IOMAP_HILINK_MACRO3_BASE_ADDR) {
		pr_err(HILINK_NAME
			": fail ioremap HL reg addr:0x%llx, sz:0x%llx\n",
			HILINKMACRO3,
			HILINKMACRO_SIZE);
		return -1;
	}
	/* begin:2P ARM SERVER, CPU */
	if (g_stHiggsBoardSasCfg.uiMaxCpuNode > 1) {
		IOMAP_HILINK_SLAVE_MACRO3_BASE_ADDR =
			(u64)ioremap(HILINKMACRO3 + 0x40000000000ULL,
			HILINKMACRO_SIZE);
		if (!IOMAP_HILINK_SLAVE_MACRO3_BASE_ADDR) {
			/* CPU unmap hilink addr */
			iounmap((void *)IOMAP_HILINK_MACRO3_BASE_ADDR);
			pr_err(HILINK_NAME
				": fail of_iomap SL HL reg addr:0x%llx, sz:0x%llx\n",
				HILINKMACRO3,
				HILINKMACRO_SIZE);
			return -1;
		}
	}
	/* end:2P ARM SERVER */
	return 0;
}

/*****************************************************************************
    : hilink1_reg_init
  : hilink
  : void
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink4_reg_init(void)
{
	IOMAP_HILINK_MACRO4_BASE_ADDR =
		(u64)ioremap(HILINKMACRO4,
			HILINKMACRO_SIZE);
	if (!IOMAP_HILINK_MACRO4_BASE_ADDR) {
		pr_err(HILINK_NAME
			": fail ioremap HL reg addr:0x%llx, sz:0x%llx\n",
			HILINKMACRO4,
			HILINKMACRO_SIZE);
		return -1;
	}
	/* begin:2P ARM SERVER, CPU */
	if (g_stHiggsBoardSasCfg.uiMaxCpuNode > 1) {
		IOMAP_HILINK_SLAVE_MACRO4_BASE_ADDR =
			(u64)ioremap(HILINKMACRO4 + 0x40000000000ULL,
				HILINKMACRO_SIZE);
		if (!IOMAP_HILINK_SLAVE_MACRO4_BASE_ADDR) {
			/* CPU unmap hilink addr */
			iounmap((void *)IOMAP_HILINK_MACRO4_BASE_ADDR);
			pr_err(
				HILINK_NAME
				": fail of_iomap SL HL reg A:0x%llx, sz:0x%llx\n",
				HILINKMACRO4,
				HILINKMACRO_SIZE);
			return -1;
		}
	}
	/* end:2P ARM SERVER */
	return 0;
}

/*****************************************************************************
    : hilink1_reg_init
  : hilink
  : void
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink5_reg_init(void)
{
	IOMAP_HILINK_MACRO5_BASE_ADDR =
		(u64)ioremap(HILINKMACRO5,
		HILINKMACRO_SIZE);
	if (!IOMAP_HILINK_MACRO5_BASE_ADDR) {
		pr_err(HILINK_NAME
			": fail ioremap HL reg addr:0x%llx, size:0x%llx\n",
			HILINKMACRO5,
			HILINKMACRO_SIZE);
		return -1;
	}
	/* begin:2P ARM SERVER, CPU */
	if (g_stHiggsBoardSasCfg.uiMaxCpuNode > 1) {
		IOMAP_HILINK_SLAVE_MACRO5_BASE_ADDR =
			(u64)ioremap(HILINKMACRO5 + 0x40000000000ULL,
				HILINKMACRO_SIZE);
		if (!IOMAP_HILINK_SLAVE_MACRO5_BASE_ADDR) {
			/* CPU unmap hilink addr */
			iounmap((void *)IOMAP_HILINK_MACRO5_BASE_ADDR);
			pr_err(HILINK_NAME
				": fail of_iomap SLAVE HL reg addr:0x%llx, sz:0x%llx\n",
				HILINKMACRO5,
				HILINKMACRO_SIZE);
			return -1;
		}
	}
	/* end:2P ARM SERVER */
	return 0;
}

/*****************************************************************************
    : hilink1_reg_init
  : hilink
  : void
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink6_reg_init(void)
{
	IOMAP_HILINK_MACRO6_BASE_ADDR =
		(u64)ioremap(HILINKMACRO6, HILINKMACRO_SIZE);
	if (!IOMAP_HILINK_MACRO6_BASE_ADDR) {
		pr_err(HILINK_NAME ": fail ioremap HL reg addr:0x%llx, sz:0x%llx\n",
			HILINKMACRO6,
			HILINKMACRO_SIZE);
		return -1;
	}
	/* begin:2P ARM SERVER, CPU */
	if (g_stHiggsBoardSasCfg.uiMaxCpuNode > 1) {
		IOMAP_HILINK_SLAVE_MACRO6_BASE_ADDR =
			(u64)ioremap(HILINKMACRO6 + 0x40000000000ULL,
			HILINKMACRO_SIZE);
		if (!IOMAP_HILINK_SLAVE_MACRO6_BASE_ADDR) {
			/* CPU unmap hilink addr */
			iounmap((void *)IOMAP_HILINK_MACRO6_BASE_ADDR);
			pr_err(HILINK_NAME ": fail of_iomap SL HL reg addr:0x%llx, sz:0x%llx\n",
				HILINKMACRO6,
				HILINKMACRO_SIZE);
			return -1;
		}
	}
	/* end:2P ARM SERVER */
	return 0;
}


/*****************************************************************************
    : hilink0_reg_exit
  : hilink 0
  : int index
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink0_reg_exit(void)
{
	if (IOMAP_HILINK_MACRO0_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_MACRO0_BASE_ADDR);

	/* begin:2P ARM SERVER, CPU */
	if (IOMAP_HILINK_SLAVE_MACRO0_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_SLAVE_MACRO0_BASE_ADDR);
	/* end:2P ARM SERVER */
	return 0;
}


/*****************************************************************************
    : hilink1_reg_exit
  : hilink 0
  : int index
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink1_reg_exit(void)
{
	if (IOMAP_HILINK_MACRO1_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_MACRO1_BASE_ADDR);
	/* begin:2P ARM SERVER, CPU */
	if (IOMAP_HILINK_SLAVE_MACRO1_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_SLAVE_MACRO1_BASE_ADDR);
	/* end:2P ARM SERVER */

	return 0;

}
/*****************************************************************************
    : hilink2_reg_exit
  : hilink 0
  : int index
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink2_reg_exit(void)
{
	if (IOMAP_HILINK_MACRO2_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_MACRO2_BASE_ADDR);

	/* begin:2P ARM SERVER, CPU */
	if (IOMAP_HILINK_SLAVE_MACRO2_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_SLAVE_MACRO2_BASE_ADDR);
	/* end:2P ARM SERVER */

	return 0;
}
/*****************************************************************************
    : hilink3_reg_exit
  : hilink 0
  : int index
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink3_reg_exit(void)
{
	if (IOMAP_HILINK_MACRO3_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_MACRO3_BASE_ADDR);
	/* begin:2P ARM SERVER, CPU */
	if (IOMAP_HILINK_SLAVE_MACRO3_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_SLAVE_MACRO3_BASE_ADDR);
	/* end:2P ARM SERVER */
	return 0;
}
/*****************************************************************************
    : hilink4_reg_exit
  : hilink 0
  : int index
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink4_reg_exit(void)
{
	if (IOMAP_HILINK_MACRO4_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_MACRO4_BASE_ADDR);
	/* begin:2P ARM SERVER, CPU */
	if (IOMAP_HILINK_SLAVE_MACRO4_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_SLAVE_MACRO4_BASE_ADDR);
	/* end:2P ARM SERVER */
	return 0;
}
/*****************************************************************************
    : hilink5_reg_exit
  : hilink 0
  : int index
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink5_reg_exit(void)
{
	if (IOMAP_HILINK_MACRO5_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_MACRO5_BASE_ADDR);
	/* begin:2P ARM SERVER, CPU */
	if (IOMAP_HILINK_SLAVE_MACRO5_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_SLAVE_MACRO5_BASE_ADDR);
	/* end:2P ARM SERVER */
	return 0;
}

/*****************************************************************************
    : hilink6_reg_exit
  : hilink 0
  : int index
  :
    : int
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
static int hilink6_reg_exit(void)
{
	if (IOMAP_HILINK_MACRO6_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_MACRO6_BASE_ADDR);
	/* begin:2P ARM SERVER, CPU */
	if (IOMAP_HILINK_SLAVE_MACRO6_BASE_ADDR > 0)
		iounmap((void *)IOMAP_HILINK_SLAVE_MACRO6_BASE_ADDR);
	/* end:2P ARM SERVER */
	return 0;
}

int hilink_reg_init(void)
{
	int ulRet;

	ulRet = hilink0_reg_init();
	if (0 != ulRet)
		pr_err("%s fail, ulRet:0x%x\n", __func__, ulRet);

	ulRet = hilink1_reg_init();
	if (0 != ulRet) {
		pr_err("%s fail, ulRet:0x%x\n", __func__, ulRet);

		(void)hilink0_reg_exit();
	}

	ulRet = hilink2_reg_init();
	if (0 != ulRet) {
		pr_err("%s fail, ulRet:0x%x\n", __func__, ulRet);

		(void)hilink1_reg_exit();
		(void)hilink0_reg_exit();
	}

	ulRet = hilink3_reg_init();
	if (0 != ulRet) {
		pr_err("%s fail, ulRet:0x%x\n", __func__, ulRet);

		(void)hilink2_reg_exit();
		(void)hilink1_reg_exit();
		(void)hilink0_reg_exit();
	}

	ulRet = hilink4_reg_init();
	if (0 != ulRet) {
		pr_err("%s fail, ulRet:0x%x\n", __func__, ulRet);

		(void)hilink3_reg_exit();
		(void)hilink2_reg_exit();
		(void)hilink1_reg_exit();
		(void)hilink0_reg_exit();
	}

	ulRet = hilink5_reg_init();
	if (0 != ulRet) {
		pr_err("%s fail, ulRet:0x%x\n", __func__, ulRet);

		(void)hilink4_reg_exit();
		(void)hilink3_reg_exit();
		(void)hilink2_reg_exit();
		(void)hilink1_reg_exit();
		(void)hilink0_reg_exit();
	}

	ulRet = hilink6_reg_init();
	if (0 != ulRet) {
		pr_err("%s fail, ulRet:0x%x\n", __func__, ulRet);

		(void)hilink5_reg_exit();
		(void)hilink4_reg_exit();
		(void)hilink3_reg_exit();
		(void)hilink2_reg_exit();
		(void)hilink1_reg_exit();
		(void)hilink0_reg_exit();
	}

	return 0;
}


/*****************************************************************************
    : hilink_reg_exit
  : hilink
  : void
  :
    : static
  :
  :

	 :
 1.	 : 201266
		 : z00176027
	 :

*****************************************************************************/
void hilink_reg_exit(void)
{
	(void)hilink6_reg_exit();
	(void)hilink5_reg_exit();
	(void)hilink4_reg_exit();
	(void)hilink3_reg_exit();
	(void)hilink2_reg_exit();
	(void)hilink1_reg_exit();
	(void)hilink0_reg_exit();
}


static void Serdes_Delay_Us(unsigned int time)
{

	if (time >= 1000) {
		mdelay(time/1000);
		time = (time%1000);

	}
	if (time != 0)
		udelay(time);
}

unsigned int HRD_SubDsafInit(void)
{
	sub_dsaf_pa_addr = DSAF_SUB_BASE;

	/* SUB ALGio */

	sub_dsaf_base_addr =
		(unsigned long long)ioremap(sub_dsaf_pa_addr,
			DSAF_SUB_BASE_SIZE);
	if (!sub_dsaf_base_addr) {
		/*release_mem_region(sub_dsaf_pa_addr, DSAF_SUB_BASE_SIZE);*/
		pr_err("could not ioremap SUB DSA registers\n");

		return OS_ERROR;
	}

	/* add by chenqilin, for slave cpu of arm server */
	if (g_stHiggsBoardSasCfg.uiMaxCpuNode > 1) {
		sub_dsaf_pa_addr_slavecpu = DSAF_SUB_BASE_SLAVE_CPU;

		/* SUB ALGio */

		sub_dsaf_base_addr_slavecpu =
			(unsigned long long)ioremap(sub_dsaf_pa_addr_slavecpu,
				DSAF_SUB_BASE_SIZE);
		if (!sub_dsaf_base_addr_slavecpu) {
			/*release_mem_region(sub_dsaf_pa_addr_slavecpu, */
			/* DSAF_SUB_BASE_SIZE);*/
			pr_err("could not ioremap SLAVE CPU SUB DSAF registers\n");

			return OS_ERROR;
		}
	}
	/* end */

	return OS_SUCCESS;
}


/*****************************************************************************
    : HRD_SubAlgExit
  : SUB DSAF CRG
  : void
  :
    :
  :
  :

	 :
 1.	 : 20131121
		 : z00176027
	 :

*****************************************************************************/
void HRD_SubDsafExit(void)
{
	/* SUB dsaf  */
	if (sub_dsaf_base_addr > 0)
		/*release_mem_region(sub_dsaf_pa_addr, DSAF_SUB_BASE_SIZE);*/
		iounmap((void *)sub_dsaf_base_addr);

	if (sub_dsaf_base_addr_slavecpu > 0)
		/*release_mem_region(sub_dsaf_pa_addr_slavecpu,*/
		/* DSAF_SUB_BASE_SIZE);*/
		iounmap((void *)sub_dsaf_base_addr_slavecpu);
}

static unsigned long long HRD_CommonSubDsafGetBase(unsigned int node)
{
	/* node=0 */
	if (node == MASTER_CPU_NODE)
		return sub_dsaf_base_addr;

	return sub_dsaf_base_addr_slavecpu;
}


/*****************************************************************************
    : HRD_SubPcieInit
  : SUB PCIE CRG linux
  : void
  :
    :
  :
  :

	 :
 1.	 : 20131121
		 : z00176027
	 :

*****************************************************************************/
unsigned int HRD_SubPcieInit(void)
{
	sub_pcie_pa_addr = PCIE_SUB_BASE;

	/* SUB ALGio */

	sub_pcie_base_addr =
		(unsigned long long)ioremap(sub_pcie_pa_addr,
			PCIE_SUB_BASE_SIZE);
	if (!sub_pcie_base_addr) {
		/*release_mem_region(sub_pcie_pa_addr, PCIE_SUB_BASE_SIZE);*/
		pr_err("could not ioremap SUB PCIE registers\n");

		return OS_ERROR;
	}

	/* add by chenqilin, for slave cpu of arm server */
	if (g_stHiggsBoardSasCfg.uiMaxCpuNode > 1) {

		sub_pcie_pa_addr_slavecpu = PCIE_SUB_BASE_SLAVE_CPU;

		sub_pcie_base_addr_slavecpu =
			(unsigned long long)ioremap(sub_pcie_pa_addr_slavecpu,
				PCIE_SUB_BASE_SIZE);
		if (!sub_pcie_base_addr_slavecpu) {
			/*release_mem_region(sub_pcie_pa_addr_slavecpu, */
			/* PCIE_SUB_BASE_SIZE);*/
			pr_err("could not ioremap SLAVE CPU SUB PCIE registers\n");

			return OS_ERROR;
		}
	}
	/* end */
	return OS_SUCCESS;
}


/*****************************************************************************
    : HRD_SubPcieExit
  : SUB PCIE CRG
  : void
  :
    :
  :
  :

	 :
 1.	 : 20131121
		 : z00176027
	 :

*****************************************************************************/
void HRD_SubPcieExit(void)
{
	/* POU */
	if (sub_pcie_base_addr > 0)
		/*release_mem_region(sub_pcie_pa_addr,*/
		/* PCIE_SUB_BASE_SIZE);*/
		iounmap((void *)sub_pcie_base_addr);

	if (sub_pcie_base_addr_slavecpu > 0)
		/*release_mem_region(sub_pcie_pa_addr_slavecpu,*/
		/* PCIE_SUB_BASE_SIZE);*/
		iounmap((void *)sub_pcie_base_addr_slavecpu);
}

static unsigned long long HRD_CommonSubPcieGetBase(unsigned int node)
{
	/* node=0 */
	if (node == MASTER_CPU_NODE)
		return sub_pcie_base_addr;

	return sub_pcie_base_addr_slavecpu;
}

static inline unsigned int SYSTEM_REG_READ(unsigned int node,
	unsigned long long pRegBase)
{
	unsigned int temp;
	unsigned long long addr = 0;
	unsigned long long addr_reg = 0;

	if ((pRegBase >= HILINKMACRO0) &&
		(pRegBase < HILINKMACRO0 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO0_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO0_BASE_ADDR);
		addr_reg = HILINKMACRO0;
	} else if ((pRegBase >= HILINKMACRO1) &&
		(pRegBase < HILINKMACRO1 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO1_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO1_BASE_ADDR);
		addr_reg = HILINKMACRO1;
	} else if ((pRegBase >= HILINKMACRO2) &&
		(pRegBase < HILINKMACRO2 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO2_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO2_BASE_ADDR);
		addr_reg = HILINKMACRO2;
	} else if ((pRegBase >= HILINKMACRO3) &&
		(pRegBase < HILINKMACRO3 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO3_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO3_BASE_ADDR);
		addr_reg = HILINKMACRO3;

	} else if ((pRegBase >= HILINKMACRO4) &&
		(pRegBase < HILINKMACRO4 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO4_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO4_BASE_ADDR);
		addr_reg = HILINKMACRO4;
	} else if ((pRegBase >= HILINKMACRO5) &&
		(pRegBase < HILINKMACRO5 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO5_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO5_BASE_ADDR);
		addr_reg = HILINKMACRO5;
	} else if ((pRegBase >= HILINKMACRO6) &&
		(pRegBase < HILINKMACRO6 + HILINK_REG_SIZE)) {
		addr_reg = HILINKMACRO6;
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO6_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO6_BASE_ADDR);
	} else if ((pRegBase >= SRE_SAS0_DSAF_CFG_BASE) &&
		(pRegBase < SRE_SAS0_DSAF_CFG_BASE + DSAF_SUB_BASE_SIZE)) {
		addr_reg = SRE_SAS0_DSAF_CFG_BASE;
		addr = HRD_CommonSubDsafGetBase(node);
	} else if ((pRegBase >= SRE_SAS1_PCIE_CFG_BASE) &&
		(pRegBase < SRE_SAS1_PCIE_CFG_BASE + PCIE_SUB_BASE_SIZE)) {
		addr_reg = SRE_SAS1_PCIE_CFG_BASE;
		addr = HRD_CommonSubPcieGetBase(node);
	} else {
		pr_err("Reg Physical Address:0x%llx\n", pRegBase);
	}

	temp = ioread32((void __iomem *)(addr + (pRegBase - addr_reg)));

	return temp;
}

static inline void SYSTEM_REG_WRITE(unsigned int node,
	unsigned long long pRegBase,
	unsigned int ulValue)
{
	/*volatile UINT32 temp;*/
	unsigned long long addr = 0;
	unsigned long long addr_reg = 0;

	if ((pRegBase >= HILINKMACRO0) &&
		(pRegBase < HILINKMACRO0 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO0_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO0_BASE_ADDR);
		addr_reg = HILINKMACRO0;
	} else if ((pRegBase >= HILINKMACRO1) &&
		(pRegBase < HILINKMACRO1 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO1_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO1_BASE_ADDR);
		addr_reg = HILINKMACRO1;
	} else if ((pRegBase >= HILINKMACRO2) &&
		(pRegBase < HILINKMACRO2 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO2_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO2_BASE_ADDR);
		addr_reg = HILINKMACRO2;
	} else if ((pRegBase >= HILINKMACRO3) &&
		(pRegBase < HILINKMACRO3 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO3_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO3_BASE_ADDR);
		addr_reg = HILINKMACRO3;

	} else if ((pRegBase >= HILINKMACRO4) &&
		(pRegBase < HILINKMACRO4 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO4_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO4_BASE_ADDR);
		addr_reg = HILINKMACRO4;
	} else if ((pRegBase >= HILINKMACRO5) &&
		(pRegBase < HILINKMACRO5 + HILINK_REG_SIZE)) {
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO5_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO5_BASE_ADDR);
		addr_reg = HILINKMACRO5;
	} else if ((pRegBase >= HILINKMACRO6) &&
		(pRegBase < HILINKMACRO6 + HILINK_REG_SIZE)) {
		addr_reg = HILINKMACRO6;
		addr = ((node == MASTER_CPU_NODE) ?
			IOMAP_HILINK_MACRO6_BASE_ADDR :
			IOMAP_HILINK_SLAVE_MACRO6_BASE_ADDR);
	} else if ((pRegBase >= SRE_SAS0_DSAF_CFG_BASE) &&
		(pRegBase < SRE_SAS0_DSAF_CFG_BASE + DSAF_SUB_BASE_SIZE)) {
		addr_reg = SRE_SAS0_DSAF_CFG_BASE;
		addr = HRD_CommonSubDsafGetBase(node);
	} else if ((pRegBase >= SRE_SAS1_PCIE_CFG_BASE) &&
		(pRegBase < SRE_SAS1_PCIE_CFG_BASE + PCIE_SUB_BASE_SIZE)) {
		addr_reg = SRE_SAS1_PCIE_CFG_BASE;
		addr = HRD_CommonSubPcieGetBase(node);
	} else {
		pr_err("pRegBae:0x%llx\n", pRegBase);
	}


	iowrite32(ulValue, (void __iomem *)(addr + (pRegBase - addr_reg)));

	/*return temp;*/
}

/*************************************************************
Prototype	: SRE_SdsRegRead
Description :serdes
Input		:	 UINT32 ulMacroId
					UINT32 ulRegAddrOffset
Output	 :
Return Value : UINT32
Calls		:
Called By	:

History		:
1.Date		 : 2013/10/15
 Author	 : w00244733
 Modification : Created function

***********************************************************/
unsigned int SRE_SdsRegRead(unsigned int node,
			unsigned int ulMacroId,
			unsigned int ulRegAddrOffset)
{
	unsigned int ulRegValue = 0;
	unsigned long long uwRegAddr = 0;

	if (ulMacroId > MACRO_6) {
		SRE_printf(
			"SRE_SdsRegRead]:MacroId %d error\n",
			ulMacroId);
		uwRegAddr = HILINKMACRO6+ulRegAddrOffset;
	}

	if (ulMacroId == 0)
		uwRegAddr = HILINKMACRO0+ulRegAddrOffset;
	if (ulMacroId == 1)
		uwRegAddr = HILINKMACRO1+ulRegAddrOffset;
	if (ulMacroId == 2)
		uwRegAddr = HILINKMACRO2+ulRegAddrOffset;
	if (ulMacroId == 3)
		uwRegAddr = HILINKMACRO3+ulRegAddrOffset;
	if (ulMacroId == 4)
		uwRegAddr = HILINKMACRO4+ulRegAddrOffset;
	if (ulMacroId == 5)
		uwRegAddr = HILINKMACRO5+ulRegAddrOffset;
	if (ulMacroId == 6)
		uwRegAddr = HILINKMACRO6+ulRegAddrOffset;

	/**/
	ulRegValue = SYSTEM_REG_READ(node, uwRegAddr);
	return ulRegValue;
}

/*************************************************************
Prototype	: SRE_SdsRegWrite
Description :serdes
Input		:	 UINT32 ulMacroId Lane
					 UINT32 ulRegAddrOffset
					 UINT32 ulRegValue
Output	 :
Return Value :
Calls		:
Called By	:

History		:
1.Date		 : 2013/10/10
 Author	 : w00244733
 Modification : adapt to hi1381

***********************************************************/
void SRE_SdsRegWrite(unsigned int ulMacroId,
				unsigned int ulRegAddrOffset,
				unsigned int ulRegValue)
{
	unsigned long long uwRegAddr = 0;

	if (ulMacroId > MACRO_6) {
		SRE_printf(
			"SRE_SdsRegWrite:MacroId %d error\n",
			ulMacroId);
		uwRegAddr = HILINKMACRO6+ulRegAddrOffset;
	}

	if (ulMacroId == 0)
		uwRegAddr = HILINKMACRO0+ulRegAddrOffset;
	if (ulMacroId == 1)
		uwRegAddr = HILINKMACRO1+ulRegAddrOffset;
	if (ulMacroId == 2)
		uwRegAddr = HILINKMACRO2+ulRegAddrOffset;
	if (ulMacroId == 3)
		uwRegAddr = HILINKMACRO3+ulRegAddrOffset;
	if (ulMacroId == 4)
		uwRegAddr = HILINKMACRO4+ulRegAddrOffset;
	if (ulMacroId == 5)
		uwRegAddr = HILINKMACRO5+ulRegAddrOffset;
	if (ulMacroId == 6)
		uwRegAddr = HILINKMACRO6+ulRegAddrOffset;

	/**/
	SYSTEM_REG_WRITE(MASTER_CPU_NODE, uwRegAddr, ulRegValue);
}

static void SRE_InternalSdsRegWrite(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulRegAddrOffset,
	unsigned int ulRegValue)
{
	unsigned long long uwRegAddr = 0;

	if (ulMacroId > MACRO_6) {
		SRE_printf("SRE_SdsRegWrite:MacroId %d error\n", ulMacroId);
		uwRegAddr = HILINKMACRO6+ulRegAddrOffset;
	}

	if (ulMacroId == 0)
		uwRegAddr = HILINKMACRO0+ulRegAddrOffset;
	if (ulMacroId == 1)
		uwRegAddr = HILINKMACRO1+ulRegAddrOffset;
	if (ulMacroId == 2)
		uwRegAddr = HILINKMACRO2+ulRegAddrOffset;
	if (ulMacroId == 3)
		uwRegAddr = HILINKMACRO3+ulRegAddrOffset;
	if (ulMacroId == 4)
		uwRegAddr = HILINKMACRO4+ulRegAddrOffset;
	if (ulMacroId == 5)
		uwRegAddr = HILINKMACRO5+ulRegAddrOffset;
	if (ulMacroId == 6)
		uwRegAddr = HILINKMACRO6+ulRegAddrOffset;


	/**/
	SYSTEM_REG_WRITE(node, uwRegAddr, ulRegValue);
}
/*************************************************************
Prototype	: SRE_SdsRegBitsWrite
Description :serdes
Input		:	 UINT32 ulMacroId
					 UINT32	ulRegAddrOffset
					 UINT32 ulHigtBit
					 UINT32	ulLowBit
					 UINT32	ulRegValue
Output	 :
Return Value :
Calls		:
Called By	:

History		:
1.Date		 : 2013/10/10
 Author	 : w00244733
 Modification : adapt to hi1381

***********************************************************/
/*Serdes */
void SRE_SdsRegBitsWrite(unsigned int node,
				unsigned int ulMacroId,
				unsigned int ulRegAddrOffset,
				unsigned int ulHigtBit,
				unsigned int ulLowBit,
				unsigned int ulRegValue)
{
	unsigned int reg_cfg_max_val;
	unsigned int orign_reg_val;
	unsigned int final_val;
	unsigned int mask;
	unsigned int add;

	if (ulMacroId > MACRO_6) {
		SRE_printf(
			"[SRE_SdsRBWr]:M %d A %d [%d:%d] val %x(M id is too big\n",
			ulMacroId,
			ulRegAddrOffset,
			ulHigtBit,
			ulLowBit,
			ulRegValue);
		return;
	}
	if (ulHigtBit < ulLowBit) {
		ulHigtBit ^= ulLowBit;
		ulLowBit ^= ulHigtBit;
		ulHigtBit ^= ulLowBit;
	}
	reg_cfg_max_val = (0x1 << (ulHigtBit - ulLowBit + 1)) - 1;
	if (ulRegValue > reg_cfg_max_val) {
		SRE_printf
			("[SRE_SdsRBsWr]:M%d A 0x%x [%d:%d] val %x is 2 big\n",
			ulMacroId,
			ulRegAddrOffset,
			ulHigtBit,
			ulLowBit,
			ulRegValue);
		return;
	}

	orign_reg_val = SRE_SdsRegRead(node, ulMacroId, ulRegAddrOffset);
	mask = (~(reg_cfg_max_val << ulLowBit)) & 0xffff;
	orign_reg_val &= mask;
	add = ulRegValue << ulLowBit;
	final_val	 = orign_reg_val + add;
	SRE_InternalSdsRegWrite(node,
		ulMacroId,
		ulRegAddrOffset,
		final_val);
}

/*************************************************************
Prototype	: SRE_SdsRegBitsRead
Description :serdes
Input		:	 UINT32 ulMacroId
					 UINT32	ulRegAddrOffset
					 UINT32 ulHigtBit
					 UINT32	ulLowBit

Output	 :
Return Value :
Calls		:
Called By	:

History		:
1.Date		 : 2013/10/10
 Author	 : w00244733
 Modification : adapt to hi1381

***********************************************************/
unsigned int SRE_SdsRegBitsRead(unsigned int node,
				unsigned int ulMacroId,
				unsigned int ulRegAddrOffset,
				unsigned int ulHigtBit,
				unsigned int ulLowBit)
{
	unsigned int orign_val;
	unsigned int mask;
	unsigned int final;

	/**/
	if (ulMacroId > MACRO_6) {
		SRE_printf(
			"[SRE_SdsRegBitsRead]:M %d A 0x%x M ID is too big\n",
			ulMacroId,
			ulRegAddrOffset);
	}
	if (ulHigtBit < ulLowBit) {
		ulHigtBit ^= ulLowBit;
		ulLowBit ^= ulHigtBit;
		ulHigtBit ^= ulLowBit;
	}
	orign_val = SRE_SdsRegRead(node,
		ulMacroId,
		ulRegAddrOffset);
	orign_val >>= ulLowBit;
	mask =	(0x1 << (ulHigtBit - ulLowBit + 1)) - 1;
	final = (orign_val & mask);
	return	final;
}


unsigned int serdes_check_param(unsigned int macro, unsigned int lane)
{
	switch (macro) {
	case MACRO_0:
	case MACRO_1:
	case MACRO_2:
	{
		if (lane > 7) {
			SRE_printf("macro:%u, lane:%u is invalid\n",
				macro,
				lane);
			return EM_SERDES_FAIL;
		}
		break;
	}
	case MACRO_3:
	case MACRO_4:
	case MACRO_5:
	case MACRO_6:
	{
		if (lane > 3) {
			SRE_printf("macro:%u, lane:%u is invalid\n",
				macro,
				lane);
			return EM_SERDES_FAIL;
		}
		break;
	}
	default:
	{
		SRE_printf("macro is invalid:%u\n",
			macro);
		return EM_SERDES_FAIL;
	}
	}
	return EM_SERDES_SUCCESS;
}



/*************************************************************
Prototype	: SRE_SerdesCsCfg
Description :serdes CS
Input		:
			 UINT32 ulMacroId Macro ID:	 Macro0/Macro1/Macro2
			 UINT32 ulCsNum	Cs Number: CS0/CS1
			 UINT32 ulCsCfg CS
Output	 :
Return Value :
Calls		:
Called By	:

History		:
1.Date		 : 2013/10/11
 Author	 : w00244733
 Modification : adapt to hi1381

***********************************************************/
void SRE_SerdesCsCfg(unsigned int node,
					unsigned int ulMacroId,
					unsigned int ulCsNum,
					unsigned int CsCfg)
{
	/* Select correct clock source for */
	/* Serviceslice (if this macro contains Serviceslice)*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 0),
		14,
		14,
		0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 0),
		13,
		13,
		0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 0),
		12,
		12,
		0);

	/* Select correct clock source for Clockslice*/
	if ((GE_1250 == CsCfg) || (GE_3125 == CsCfg) || (XGE_10312 == CsCfg)) {
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 0),
			11,
			11,
			0);
	} else {
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 0),
			11,
			11,
			1);
	}

	/*SRE_SdsRegBitsWrite(node, ulMacroId, CS_CSR(ulCsNum, 0),10,10, 1);*/
	/*1Firmware,DS(8GFirmware,XGE)*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 0),
		10,
		10,
		0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 0),
		9,
		9,
		0); /*The highest freq is 400M.*/

	/* Set Clockslice divider ratio*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 1),
		15,
		15,
		0); /* W divider setting*/

	switch (CsCfg) {
	case GE_1250:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka2_Kb2);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0xA);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0x9);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0);
		break;
	case GE_3125:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka3_Kb1);
		/*PS N=2*P+S+3*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0x6);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0x5); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0x1);
		break;
	case XGE_10312:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka2_Kb1);
		/*PS N=2*P+S+3*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0xA);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0xA); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0x3);
		break;
	case PCIE_2500:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka2_Kb2);
		/*PS N=2*P+S+3*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0x10);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0xF); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0);
		break;
	case PCIE_5000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka2_Kb2);
		/*PS N=2*P+S+3*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0x10);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0xF); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0);
		break;
	case PCIE_8000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka3_Kb1);
		/*PS N=2*P+S+3*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0xd);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0xb); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0x3);
		break;
	case SAS_1500:
		/*2014_12_3 modify for sas DTS2014121600518*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka3_Kb1);/*CMLDIV*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0x9);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0x9); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			1);/*WORDCLKDIV*/
		break;
	case SAS_3000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka2_Kb2);
		/*PS N=2*P+S+3*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0x15);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0xF); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0);
		break;
	case SAS_6000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka2_Kb2);
		/*PS N=2*P+S+3*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0x15);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0xF); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0);
		break;
	case SAS_12000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka2_Kb1);
		/*PS N=2*P+S+3*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0x15);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0xF); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0x1);
		break;
	case HCCS_32:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka2_Kb1);
		/*PS N=2*P+S+3*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0x15);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0xF); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0x3);
		break;
	case HCCS_40:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			14,
			13,
			Ka2_Kb1);
		/*PS N=2*P+S+3*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			12,
			8,
			0x15);/*P*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			7,
			4,
			0xF); /*S*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 1),
			1,
			0,
			0x1);
		break;
	default:
		break;
	}

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 2),
		15,
		14,
		0);
	if ((ulCsNum == CS0) && ((CsCfg == SAS_1500) ||
		(CsCfg == PCIE_2500)))
		/*0x0MCUFirmWare,DTS2014121600518*/
		SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 2),
		13,
		12,
		0x0);

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 44),
		6,
		6,
		0); /* V divider setting*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 45),
		7,
		7,
		0);

	/*
		1.HiLink15BP Programming User Guide DraftV1.11.pdf modify
		2.SSC
	*/
	if (sUseSSC) {
		/* SSC Mode of Clock Slice ONLY prepares for */
		/* the project using SSC Function	SAS*/
		if ((SAS_1500 == CsCfg) ||
			(SAS_3000 == CsCfg) ||
			(SAS_6000 == CsCfg) ||
			(SAS_12000 == CsCfg)) {
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				CS_CSR(ulCsNum, 56),
				8,
				0,
				0x2e);
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				CS_CSR(ulCsNum, 60),
				14,
				12,
				0x1);
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				CS_CSR(ulCsNum, 57),
				6,
				0,
				0x23);
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				CS_CSR(ulCsNum, 58),
				15,
				14,
				0x1);
			/*SRE_SdsRegBitsWrite(node, ulMacroId, */
			/* CS_CSR(ulCsNum,57),7,7, 0x1);*/
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				CS_CSR(ulCsNum, 58),
				13,
				13,
				0x0); /*0 :auto mode; 1:manual mode*/
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				CS_CSR(ulCsNum, 58),
				12,
				12,
				0x1); /*decrease the intg/prop cp current*/
			/*SRE_SdsRegBitsWrite(node, ulMacroId, */
			/* CS_CSR(ulCsNum,60),8,8, 0x0); */
			/* power on the ssc PI module*/
			/*CPCURRENT 2014/10/29 modify*/
			SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 4),
			9,
			4,
			0x2);	 /*decrease the prop chargepump current*/

			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				CS_CSR(ulCsNum, 57),
				7,
				7,
				0x1);
		}
	} else {
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			CS_CSR(ulCsNum, 58),
			9,
			9,
			0x1);

	}
}



void SRE_CsHwCalibrationOptionV2Init(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulCsNum)
{
	/* Set charge pump related register*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 44),
		4,
		0,
		0x5);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 4),
		12,
		10,
		0x0); /*CPCURRENTINT*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 11),
		7,
		0,
		0x10);/*VREGPLLLVL*/

	/*if it is a low frequency clock slice*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 30),
		11,
		7,
		0xe);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 28),
		9,
		5,
		0xe);

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 28),
		4,
		0,
		0x3);

	/* Set peak level for amplitude calibration*/
	SRE_InternalSdsRegWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 46),
		0x22c0); /*CS_DEBUGGING_CTRL1*/

	/* Set freq detector threshold to minimum value*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 35),
		10,
		1,
		0x0);

	/* Active low DLF common mode active common mode circuitry power down*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 5),
		8,
		8,
		0x1);

	/* Set the LCVCO Temperature compensation ON*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 58),
		10,
		10,
		0x1);

	/* Reset clock slice calibration bit*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 20),
		2,
		2,
		0x0);/*CS_CALIB_SOFT_RST_N*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 20),
		2,
		2,
		0x1);/*CS_CALIB_SOFT_RST_N*/
}

unsigned int SRE_CsHwCalibrationOptionV2Exec(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulCsNum,
	unsigned int time)
{
	unsigned int ulTimerLog;
	unsigned int result = M_common_OK;

	/* Set calibration mode to 0 for auto calibration*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 13),
		15,
		15,
		0x0);/*LCVCOCALMODE*/

	/* Active clock slice auto calibration by rising */
	/* edge of CS_CALIB_START*/
	SRE_SdsRegBitsWrite(node,
	ulMacroId,
	CS_CSR(ulCsNum, 20),
		8,
		8,
		0x0);/*CS_CALIB_START*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		CS_CSR(ulCsNum, 20),
		8,
		8,
		0x1);/*CS_CALIB_START*/

	/*	Check whether calibration successfully done*/
	ulTimerLog = 0;
	while (ulTimerLog <= CsCalibTime) {
		Serdes_Delay_Us(1000);
		if (0x1 == SRE_SdsRegBitsRead(node,
			ulMacroId,
			CS_CSR(ulCsNum, 13),
			4,
			4)) { /*LCVCOCALDONE*/
			if (time == 0)
				SRE_printf("S_CHC:M%d Cs%d Lc V C d(L) %d\n",
				ulMacroId,
				ulCsNum,
				ulTimerLog);

			if (time > 0)
				SRE_printf("S_C:M%d CS%d LC V RC d(L %d) %d\n",
					ulMacroId,
					ulCsNum,
					time,
					ulTimerLog);
			break;
		}
		ulTimerLog++;
	}

	if (ulTimerLog > CsCalibTime) {
		result = SRE_Hilink_CS_Calib_ERROR;
		if (time == 0)
			SRE_printf(
			"[SRE_CsHwCalib.]:M%d CS%d LC Vco Cal fail(L)\n",
				ulMacroId,
				ulCsNum);
		if (time > 0)
			SRE_printf("[RE_CsHwCab.]:M%d CS%d LC V RCl F(L %d)\n",
				ulMacroId,
				ulCsNum,
				time);
	}

	return result;

}


unsigned int SRE_SerdesCsHwCalibrationOptionV2(unsigned int node,
			unsigned int ulMacroId,
			unsigned int ulCsNum)
{
	unsigned int Cal_Result = M_common_OK;
	unsigned int result = M_common_OK;
	unsigned int ulTimerLog = 0;

	/* First do initialization before calibration*/
	SRE_CsHwCalibrationOptionV2Init(node, ulMacroId, ulCsNum);

	/*step2 : Execution function of Cs hardware calibration*/
	Cal_Result = SRE_CsHwCalibrationOptionV2Exec(node,
					ulMacroId,
					ulCsNum,
					0);
	if (Cal_Result == SRE_Hilink_CS_Calib_ERROR) {
		result = SRE_Hilink_CS_Calib_ERROR;
		return result;
	}

	/*PLL Lock*/
	ulTimerLog = 0;
	while (ulTimerLog <= 10) {
		Serdes_Delay_Us(1000);
		if (0x0 == SRE_SdsRegBitsRead(node,
				ulMacroId,
				CS_CSR(ulCsNum, 2),
				1,
				1)) {
			/*CS PLL LOCK*/
			SRE_printf(
				"[SRE_SerdesC...]:Macro%d CS%d PLL lock success!(%d ms)\n",
				ulMacroId,
				ulCsNum,
				ulTimerLog);
			break;
		}
		ulTimerLog++;
	}
	if (ulTimerLog > 10)
		SRE_printf(
			"[SRE_SerdesCsCalib]:Macro%d CS%d PLL out of lock!\n",
			ulMacroId,
			ulCsNum);

	return result;
}



unsigned int SRE_SerdesCsCalib(unsigned int node,
	unsigned int ulMacroId,
	unsigned int ulCsNum)
{
	unsigned int result;
	unsigned int timer = 0;

	for (timer = 0; timer < 10; timer++) {
		result = SRE_SerdesCsHwCalibrationOptionV2(node,
						ulMacroId,
						ulCsNum);
		if (result == M_common_OK)
			break;
	}
	if (timer >= 10)
		result = SRE_Hilink_CS_Calib_ERROR;
	return result;
}
/*************************************************************
Prototype	: SRE_SerdesDsCfg
Description :serdes DS
Input		:
			 UINT32 ulMacroId Macro ID:	 Macro0/Macro1/Macro2
			 UINT32 ulDsNum	Ds Number
			 DS_CFG_ENUM ulCsCfg DS
			 UINT32 ulCsSrc	CS0CS1
Output	 :
Return Value :
Calls		:
Called By	:

History		:
1.Date		 : 2013/10/11
 Author	 : w00244733
 Modification : adapt to hi1381

***********************************************************/
void SRE_SerdesDsCfg(unsigned int node,
		unsigned int ulMacroId,
		unsigned int ulDsNum,
		unsigned int ulDsCfg,
		unsigned int ulCsSrc)
{
	unsigned int ulRegValue;
	unsigned long long uwRegAddr = 0;
	unsigned int ulRateMode = 0;

	if (ulMacroId == 0)
		uwRegAddr = SRE_HILINK0_MACRO_LRSTB_REG;
	if (ulMacroId == 1)
		uwRegAddr = SRE_HILINK1_MACRO_LRSTB_REG;
	if (ulMacroId == 2)
		uwRegAddr = SRE_HILINK2_MACRO_LRSTB_REG;
	if (ulMacroId == 3)
		uwRegAddr = SRE_HILINK3_MACRO_LRSTB_REG;
	if (ulMacroId == 4)
		uwRegAddr = SRE_HILINK4_MACRO_LRSTB_REG;
	if (ulMacroId == 5)
		uwRegAddr = SRE_HILINK5_MACRO_LRSTB_REG;
	if (ulMacroId == 6)
		uwRegAddr = SRE_HILINK6_MACRO_LRSTB_REG;

	/*release lane reset*/
	ulRegValue = SYSTEM_REG_READ(node, uwRegAddr);
	ulRegValue |= (0x1 << ulDsNum);
	SYSTEM_REG_WRITE(node, uwRegAddr, ulRegValue);
	Serdes_Delay_Us(10);
	/*
		1.HiLink15BP Programming User Guide DraftV1.11.pdf modify
	*/
	/* Power up DSCLK*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 1),
		10,
		10,
		0x1);
	/* Power up Tx*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 0),
		15,
		15,
		0x1);
	/* Power up Rx*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 0),
		15,
		15,
		0x1);
	Serdes_Delay_Us(100);
	/*disable remote EyeMetric*/
	/*SRE_SdsRegBitsWrite(node, ulMacroId, RX_CSR(ulDsNum,61),13,13,0);*/
	/* Dsclk configuration start*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 0),
		14,
		14,
		ulCsSrc);

	/* Choose whether bypass Tx phase interpolator*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 0),
		10,
		10,
		0);

	/* Select Tx spine clock source*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 0),
		13,
		13,
		ulCsSrc);

	/*
		1.HiLink15BP Programming User Guide DraftV1.11.pdf modify
	*/
	/*SRE_SdsRegBitsWrite(node, ulMacroId, DSCLK_CSR(ulDsNum,1),9,9,0);*/

	/*DTS2014091107380*/
	Serdes_Delay_Us(200);

	if (g_serdes_tx_polarity[node][ulMacroId][ulDsNum] == 1)
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			8,
			8,
			1);

	/* Set Tx clock and data source*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 2),
		7,
		6,
		0x0);

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 2),
		5,
		4,
		0x0);

	/* Set Tx align window dead band and Tx align mode*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 2),
		3,
		3,
		0x0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 2),
		2,
		0,
		0x0);


	switch (ulDsCfg) {
	case GE_1250:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			QUARTER_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_40bit); /*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			QUARTER_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_40bit); /*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x3f);
		ulRateMode = QUARTER_RATE;
		break;
	case GE_3125:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			HALF_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_20bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			HALF_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_20bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x3f);
		ulRateMode = HALF_RATE;
		break;
	case XGE_10312:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			FULL_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_32bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			FULL_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_32bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x3f);
		ulRateMode = FULL_RATE;
		break;
	case PCIE_2500:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			HALF_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_40bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			HALF_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_40bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x34);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			9,
			5,
			0x1b);
		ulRateMode = HALF_RATE;
		break;
	case PCIE_5000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			FULL_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_20bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			FULL_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_20bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x34);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			9,
			5,
			0x1b);
		ulRateMode = FULL_RATE;
		break;
	case PCIE_8000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			FULL_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_16bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			FULL_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_16bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x34);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			9,
			5,
			0x1b);
		ulRateMode = FULL_RATE;
		break;
	case SAS_1500:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			QUARTER_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_40bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			QUARTER_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_40bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x37);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			9,
			5,
			0x18);
		ulRateMode = QUARTER_RATE;
		break;
	case SAS_3000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			HALF_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_16bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			HALF_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_16bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x37);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			9,
			5,
			0x18);
		ulRateMode = HALF_RATE;
		break;
	case SAS_6000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			FULL_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_16bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			FULL_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_16bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x37);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			9,
			5,
			0x18);
		ulRateMode = FULL_RATE;
		break;
	case SAS_12000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			FULL_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_16bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			FULL_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_16bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x37);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			9,
			5,
			0x18);
		ulRateMode = FULL_RATE;
		break;
	case HCCS_32:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			FULL_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_32bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			FULL_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_32bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x3f);
		ulRateMode = FULL_RATE;
		break;
	case HCCS_40:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			11,
			10,
			FULL_RATE);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			14,
			12,
			Width_40bit);/*Rx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			11,
			10,
			FULL_RATE);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 2),
			14,
			12,
			Width_40bit);/*Tx*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 1),
			15,
			10,
			0x3f);
		ulRateMode = FULL_RATE;
		break;
	default:
		break;
	}

	/* Set Tx FIR coefficients //*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 0),
		9,
		5,
		0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 0),
		4,
		0,
		0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 1),
		4,
		0,
		0);

	/* Set Tx tap pwrdnb according to settings */
	/* of pre2/post2 tap setting*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 10),
		7,
		0,
		0xf6);

	/* Set Tx amplitude to 3*/
	 SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 11),
		2,
		0,
		0x3);

	/* TX termination calibration target resist value choice*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 34),
		1,
		0,
		0x2);

	if (ulDsCfg == PCIE_2500 ||
		ulDsCfg == PCIE_5000 ||
		ulDsCfg == PCIE_8000) {
		/*select deemph from register txdrv_map_sel,PCIE*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			TX_CSR(ulDsNum, 34),
			4,
			4,
			0x1);
	}


	/* Set center phase offset according to rate mode*/
	if (FULL_RATE == ulRateMode) {
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 12),
			15,
			8,
			0x20);
	} else {
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 12),
			15,
			8,
			0x0);
	}

	/* Rx termination calibration target resist value choice. 0-50Ohms*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 31),
		5,
		4,
		0x1);

	if (ulDsCfg == PCIE_2500 ||
		ulDsCfg == SAS_1500 ||
		ulDsCfg == SAS_3000 ||
		ulDsCfg == SAS_6000 ||
		ulDsCfg == GE_1250 ||
		ulDsCfg == GE_3125)
		/*pugv1.11  ,xls rxctlerefsel*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 39),
			5,
			3,
			0x4);

	if (g_serdes_rx_polarity[node][ulMacroId][ulDsNum] == 1)
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			8,
			8,
			0x1);

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 1),
		15,
		0,
		0xbaa7);
	/*P660 Programming User Configuration.xlsx modify*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 2),
		15,
		0,
		0x3aa7);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 3),
		15,
		0,
		0x3aa7);

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 1),
		9,
		9,
		0x1);
}


/* DS Configuration after DS calibration*/
void SRE_DsConfigurationAfterCalibration(unsigned int node,
					unsigned int ulMacroId,
					unsigned int ulDsNum,
					unsigned int ulDsCfg,
					unsigned int ulCoupleFlag)
{
	unsigned int ulRegValue;

	if (ulCoupleFlag == AcModeFlag)	{
		/* Use AC couple mode (cap bypass mode), */
		/* suggested mode for most customer*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 0),
			13,
			13,
			0x0);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 40),
			9,
			9,
			0x0);
	} else {
		/* Use DC couple mode (cap bypass mode), */
		/* suggested mode for most customer*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 0),
			13,
			13,
			0x1);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 40),
			9,
			9,
			0x1);

		ulRegValue = SRE_SdsRegBitsRead(node,
			ulMacroId,
			RX_CSR(ulDsNum, 0),
			2,
			0);
		if (ulRegValue == 0)
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				RX_CSR(ulDsNum, 0),
				2,
				0,
				0x1);
			/* CTLEPASSGN cannot set to 0 in DC couple mode*/
	}

	/* Rx termination floating set*/
	/*RXTERM_FLOATING (RX_CSR27 bit[12]) = User Setting;*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 27),
		12,
		12,
		0x1);

	/* signal detection and read the result from Pin SQUELCH_DET and ALOS*/
	/*SIGDET_ENABLE (RX_CSR60 bit[15]) = User Setting;*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 60),
		15,
		15,
		0x1);

	if ((SAS_1500 == ulDsCfg) ||
		(SAS_3000 == ulDsCfg) ||
		(SAS_6000 == ulDsCfg) ||
		(SAS_12000 == ulDsCfg)) {
		/*RX_SIGDET_BLD_DLY_SEL (RX_CSR31 bit[0]) = User Setting;*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 31),
			0,
			0,
			0x1);
		/*SIGDET_WIN (RX_CSR60 bit[14:11])*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			0);
	} else if ((PCIE_2500 == ulDsCfg) ||
		(PCIE_5000 == ulDsCfg) ||
		(PCIE_8000 == ulDsCfg)) {
		/*RX_SIGDET_BLD_DLY_SEL (RX_CSR31 bit[0]) = User Setting;*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 31),
			0,
			0,
			0x0);
		/*SIGDET_WIN (RX_CSR60 bit[14:11])*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			0x5);
	} else {
		/*RX_SIGDET_BLD_DLY_SEL (RX_CSR31 bit[0]) = User Setting;*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 31),
			0,
			0,
			0x0);
		/*SIGDET_WIN (RX_CSR60 bit[14:11])*/
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			0x6);
	}

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 61),
		10,
		0,
		2);

	switch (ulDsCfg) {
	case GE_1250:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			6);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 62),
			10,
			0,
			0x40);
		break;
	case GE_3125:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			6);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 62),
			10,
			0,
			0x40);
		break;
	case XGE_10312:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			6);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 62),
			10,
			0,
			0x40);
		break;
	case PCIE_2500:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			5);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 62),
			10,
			0,
			0x28);
		break;
	case PCIE_5000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			5);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 62),
			10,
			0,
			0x28);
		break;
	case PCIE_8000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			5);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 62),
			10,
			0,
			0x28);
		break;
	case SAS_1500:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			0);
		break;
	case SAS_3000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			0);
		break;
	case SAS_6000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			0);
		break;
	case SAS_12000:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			0);
		break;
	case HCCS_32:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			6);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 62),
			10,
			0,
			0x40);
		break;
	case HCCS_40:
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 60),
			14,
			11,
			6);
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 62),
			10,
			0,
			0x40);
		break;
	default:
		break;
	}

	/* Other specified Dataslice settings */
	/* for certain application added below*/
	/* Customer should confirm with HiLink */
	/* support team for extra settings*/
}




void SRE_DsHwCalibrationInit(unsigned int node,
		unsigned int ulMacroId,
		unsigned int ulDsNum)
{
	/* Power up eye monitor*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 15),
		15,
		15,
		0x1);

	/* Set RX, DSCLK and TX to auto calibration mode*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 27),
		15,
		15,
		0x0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 9),
		13,
		13,
		0x0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 9),
		12,
		10,
		0x0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 14),
		5,
		4,
		0x0);

	/* Make sure Tx2Rx loopback is disabled*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 0),
		10,
		10,
		0x0);

	/* Set Rx to AC couple mode for calibration*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 0),
		13,
		13,
		0x0);

	/*ECOARSEALIGNSTEP (RX_CSR45 bit[6:4]) = 0x0;, pug v1.2*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 45),
		6,
		4,
		0x0);
}

unsigned int SRE_DsHwCalibrationExec(unsigned int node,
				unsigned int ulMacroId,
				unsigned int ulDsNum)
{
	unsigned int ulCheckCount;

	/* Start data slice calibration by rising edge of DSCALSTART*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 27),
		14,
		14,
		0x0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 27),
		14,
		14,
		0x1);


	/* Check whether calibration complete*/
	ulCheckCount = 0;
	/* temp local variable for time out count of DS calibration down check*/
	do {
		ulCheckCount++;
		Serdes_Delay_Us(1000);
	} while (ulCheckCount <= 100 &&
		(SRE_SdsRegBitsRead(node,
			ulMacroId,
			TX_CSR(ulDsNum, 26),
			15,
			15)
			== 0 ||
		SRE_SdsRegBitsRead(node,
			ulMacroId,
			TX_CSR(ulDsNum, 26),
			14,
			14)
			== 0));

	if (ulCheckCount > 100) {
		SRE_printf(
			"[SRE_DsHwCalibrationExec]:M%d DS%d count>100 cal not complete\n",
			ulMacroId,
			ulDsNum);
		return SRE_Hilink_DS_Calibration_ERROR; /* Calibration failed*/
	}
	/* Set the loss-of-lock detector to continuous running mode*/
	SRE_SdsRegBitsWrite(node,
				ulMacroId,
				DSCLK_CSR(ulDsNum, 15),
				0,
				0,
				0x0);

	/* Start dsclk loss of lock detect*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 10),
		9,
		9,
		0x0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 10),
		9,
		9,
		0x1);
	/* Calibration succeed*/
	return M_common_OK;

}


void SRE_DsHwCalibrationAdjust(unsigned int node,
			unsigned int ulMacroId,
			unsigned int ulDsNum)
{
	int i;
	unsigned int ulReadReg = 0;

	/* Power down eye monitor*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 15),
		15,
		15,
		0x0);

	/* Transfer Rx auto calibration value to */
	/* registers and adjust Rx termination*/
	for (i = 1; i <= 6; i++) {
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 27),
			5,
			0,
			i);
		ulReadReg = SRE_SdsRegBitsRead(node,
						ulMacroId,
						TX_CSR(ulDsNum, 42),
						5,
						0);
		ulReadReg = ulReadReg & 0x1f;

		switch (i) {
		case 1:
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				RX_CSR(ulDsNum, 28),
				14,
				10,
				ulReadReg);
			break;
		case 2:
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				RX_CSR(ulDsNum, 28),
				9,
				5,
				ulReadReg);
			break;
		case 3:
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				RX_CSR(ulDsNum, 28),
				4,
				0,
				ulReadReg);
			break;
		case 4:
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				RX_CSR(ulDsNum, 29),
				14,
				10,
				ulReadReg);
			break;
		case 5:
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				RX_CSR(ulDsNum, 29),
				9,
				5,
				ulReadReg);
			break;
		case 6:
			SRE_SdsRegBitsWrite(node,
				ulMacroId,
				RX_CSR(ulDsNum, 29),
				4,
				0,
				ulReadReg);
			break;
		default:
			break;
		}
	}

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 9),
		13,
		13,
		0x1);
}


/*************************************************************
Prototype	: SRE_SerdesDsCalib
Description :serdes DS
Input		:
			 UINT32 ulMacroId Macro ID:	 Macro0/Macro1/Macro2
			 UINT32 ulDsNum	Ds Number
			 UINT32 ulDsCfg DS
Output	 :
Return Value :
Calls		:
Called By	:

History		:
1.Date		 : 2013/10/11
 Author	 : w00244733
 Modification : adapt to hi1381

***********************************************************/
unsigned int SRE_SerdesDsCalib(unsigned int node,
				unsigned int ulMacroId,
				unsigned int ulDsNum,
				unsigned int ulDsCfg)
{
	unsigned int ulRet = M_common_OK;

	/* First do initialization before calibration*/
	SRE_DsHwCalibrationInit(node, ulMacroId, ulDsNum);

	/* Then execute Ds hardware calibration*/
	ulRet = SRE_DsHwCalibrationExec(node, ulMacroId, ulDsNum);
	if (ulRet != M_common_OK) {
		SRE_printf(
			"[SRE_SerdesDsCalib]:M%d DS%d call SRE_DsHwCalibrationExec err\n",
			ulMacroId,
			ulDsNum);
		return SRE_Hilink_DS_Calibration_ERROR;
	}

	/* Finally do Ds hardware calibration adjustment*/
	SRE_DsHwCalibrationAdjust(node, ulMacroId, ulDsNum);

	/* Check calibration results*/

	Serdes_Delay_Us(1000);
	/* Check out of lock when in ring VCO mode*/

	if (SRE_SdsRegBitsRead(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 0),
		1,
		1) == 0)
		return M_common_OK; /* calibration success*/

	SRE_printf(
		"[SRE_SerdesDsCalib]:Macro%d DS%d calibration results error\n",
		ulMacroId,
		ulDsNum);
	return SRE_Hilink_DS_Calibration_ERROR; /* calibration false*/
}


/*************************************************************
Prototype	: SRE_PMAInit
Description :
Input		:

					UINT32 ulFlag PCIE

Output	 :
Calls		:
Called By	:

History		:
1.Date		 : 2013/10/11
 Author	 : w00244733
 Modification : adapt to hi1381,6602

***********************************************************/
void SRE_PMAInit(unsigned int node,
		unsigned int ulMacroId,
		unsigned int ulDsNum,
		unsigned int ulDsCfg,
		unsigned int ulCsSrc,
		unsigned int ulPmaMode)
{

	/* PMA mode configure:0-PCIe, 1-SAS, 2-Normal Mode*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 48),
		13,
		12,
		ulPmaMode);

	if (PMA_MODE_SAS < ulPmaMode)
		return;

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 54),
		15,
		13,
		0x0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 54),
		11,
		8,
		0x9);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 55),
		15,
		8,
		0xa);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 55),
		7,
		0,
		0x0);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 56),
		15,
		8,
		0xa);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 56),
		7,
		0,
		0x14);

	/* Enable eye metric function*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 15),
		15,
		15,
		0x1);

	/* Set CDR_MODE. 0-Normal CDR(default), 1-SSCDR*/
	if (ulPmaMode == PMA_MODE_SAS) {
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			6,
			6,
			sCDRMode);
	} else if (ulPmaMode == PMA_MODE_PCIE) {
		SRE_SdsRegBitsWrite(node,
			ulMacroId,
			RX_CSR(ulDsNum, 10),
			6,
			6,
			0x0);
	}

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		DSCLK_CSR(ulDsNum, 0),
		9,
		5,
		0x1f);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 53),
		15,
		11,
		0x4);

	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 48),
		11,
		11,
		0x0);

	/* Configure for pull up rxtx_status and enable pin_en*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 48),
		14,
		14,
		0x1);
/*	SRE_SdsRegBitsWrite(node, ulMacroId, TX_CSR(ulDsNum,48),3,3, 0x1);*/
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		RX_CSR(ulDsNum, 60),
		15,
		15,
		0x1);
	SRE_SdsRegBitsWrite(node,
		ulMacroId,
		TX_CSR(ulDsNum, 48),
		3,
		3,
		0x1);
}


unsigned int SRE_Serdes2LaneReset(unsigned int node,
	unsigned int ulDsNum,
	unsigned int ulDsCfg)
{
	unsigned int ulCsSrc;
	unsigned int ulRegValue;
	unsigned int uwRegAddr = 0;
	unsigned int uwRepeatTimes = 0;

	/*disable CTLE/DFE*/
	SRE_SdsRegBitsWrite(node,
		MACRO_2,
		DS_API(ulDsNum)+0,
		6,
		6,
		0);
	SRE_SdsRegBitsWrite(node,
		MACRO_2,
		DS_API(ulDsNum)+4,
		7,
		4,
		0);

	/*CTLE and DFE adaptation reset bit11 in the PUG control0*/
	SRE_SdsRegBitsWrite(node,
		MACRO_2,
		DS_API(ulDsNum)+4,
		3,
		3,
		1);

	/*CTLE and DFE adaptation reset status*/
	uwRepeatTimes = 1000;
	ulRegValue = SRE_SdsRegBitsRead(node,
					MACRO_2,
					DS_API(ulDsNum)+12,
					3,
					3);
	while ((1 != ulRegValue) && uwRepeatTimes) {
		Serdes_Delay_Us(100);
		uwRepeatTimes--;
		ulRegValue = SRE_SdsRegBitsRead(node,
						MACRO_2,
						DS_API(ulDsNum)+12,
						3,
						3);
	}

	if (0 == uwRepeatTimes)
		SRE_printf(
			"[SRE_Serdes2LaneReset]:CTLE/DFE reset timeout\n");

	/*CTLE and DFE adaptation reset release bit11 in the PUG control0*/
	SRE_SdsRegBitsWrite(node,
		MACRO_2,
		DS_API(ulDsNum)+4,
		3,
		3,
		0);

	uwRegAddr = SRE_HILINK2_MACRO_LRSTB_REG;
	/*lane reset*/
	ulRegValue = SYSTEM_REG_READ(node, uwRegAddr);
	ulRegValue &= (~(0x1 << ulDsNum));
	SYSTEM_REG_WRITE(node, uwRegAddr, ulRegValue);

	ulCsSrc = CS0;
	SRE_SerdesDsCfg(node, MACRO_2, ulDsNum, ulDsCfg, ulCsSrc);
	if (M_common_OK != SRE_SerdesDsCalib(node,
						MACRO_2,
						ulDsNum,
						ulDsCfg)) {
		SRE_printf(
			"[SRE_Serdes2LaneReset]:Macro2 Ds%d Calibrate fail!\n",
			ulDsNum);
		return EM_SERDES_FAIL;
	}

	SRE_DsConfigurationAfterCalibration(node,
		MACRO_2,
		ulDsNum,
		ulDsCfg,
		DcModeFlag);
	SRE_PMAInit(node,
		MACRO_2,
		ulDsNum,
		ulDsCfg,
		ulCsSrc,
		PMA_MODE_SAS);

	return EM_SERDES_SUCCESS;
}



unsigned int SRE_Serdes5LaneReset(unsigned int node,
	unsigned int ulDsNum,
	unsigned int ulDsCfg)
{
	unsigned int ulCsSrc;
	unsigned int ulRegValue;
	unsigned long long uwRegAddr = 0;
	unsigned int uwRepeatTimes = 0;

	/*disable CTLE/DFE*/
	SRE_SdsRegBitsWrite(node,
		MACRO_5,
		DS_API(ulDsNum)+0,
		6,
		6,
		0);
	SRE_SdsRegBitsWrite(node,
		MACRO_5,
		DS_API(ulDsNum)+4,
		7,
		4,
		0);

	/*CTLE and DFE adaptation reset bit11 in the PUG control0*/
	SRE_SdsRegBitsWrite(node,
		MACRO_5,
		DS_API(ulDsNum)+4,
		3,
		3,
		1);

	/*CTLE and DFE adaptation reset status*/
	uwRepeatTimes = 1000;
	ulRegValue = SRE_SdsRegBitsRead(node,
					MACRO_5,
					DS_API(ulDsNum)+12,
					3,
					3);
	while ((1 != ulRegValue) && uwRepeatTimes) {
		Serdes_Delay_Us(100);
		uwRepeatTimes--;
		ulRegValue =
			SRE_SdsRegBitsRead(node,
				MACRO_5,
				DS_API(ulDsNum)+12,
				3,
				3);
	}

	if (0 == uwRepeatTimes)
		SRE_printf("[SRE_Serdes5LaneReset]:CTLE/DFE reset timeout!\n");

	/*CTLE and DFE adaptation reset release bit11 in the PUG control0*/
	SRE_SdsRegBitsWrite(node,
		MACRO_5,
		DS_API(ulDsNum)+4,
		3,
		3,
		0);


	uwRegAddr = SRE_HILINK5_MACRO_LRSTB_REG;
	/*lane reset*/
	ulRegValue = SYSTEM_REG_READ(node, uwRegAddr);
	ulRegValue &= (~(0x1 << ulDsNum));
	SYSTEM_REG_WRITE(node, uwRegAddr, ulRegValue);
	Serdes_Delay_Us(10);

	ulCsSrc = CS0;
	SRE_SerdesDsCfg(node, MACRO_5, ulDsNum, ulDsCfg, ulCsSrc);
	if (M_common_OK !=
		SRE_SerdesDsCalib(node, MACRO_5, ulDsNum, ulDsCfg)) {
		SRE_printf(
			"[SRE_Serdes5LaneReset]:Macro5 Ds%d Calibrate fail!\n",
			ulDsNum);
		return EM_SERDES_FAIL;
	}

	SRE_DsConfigurationAfterCalibration(node,
		MACRO_5,
		ulDsNum,
		ulDsCfg,
		DcModeFlag);
	SRE_PMAInit(node,
		MACRO_5,
		ulDsNum,
		ulDsCfg,
		ulCsSrc,
		PMA_MODE_SAS);

	return EM_SERDES_SUCCESS;
}


unsigned int SRE_Serdes6LaneReset(unsigned int node,
					unsigned int ulDsNum,
					unsigned int ulDsCfg)
{
	unsigned int ulCsSrc;
	unsigned int ulRegValue;
	unsigned long long uwRegAddr = 0;
	unsigned int uwRepeatTimes = 0;

	/*disable CTLE/DFE*/
	SRE_SdsRegBitsWrite(node,
			MACRO_6,
			DS_API(ulDsNum)+0,
			6,
			6,
			0);
	SRE_SdsRegBitsWrite(node,
			MACRO_6,
			DS_API(ulDsNum)+4,
			7,
			4,
			0);

	/*CTLE and DFE adaptation reset bit11 in the PUG control0*/
	SRE_SdsRegBitsWrite(node,
			MACRO_6,
			DS_API(ulDsNum)+4,
			3,
			3,
			1);

	/*CTLE and DFE adaptation reset status*/
	uwRepeatTimes = 1000;
	ulRegValue =
		SRE_SdsRegBitsRead(node,
			MACRO_6,
			DS_API(ulDsNum)+12,
			3,
			3);
	while ((1 != ulRegValue) && uwRepeatTimes) {
		Serdes_Delay_Us(100);
		uwRepeatTimes--;
		ulRegValue =
			SRE_SdsRegBitsRead(node,
				MACRO_6,
				DS_API(ulDsNum)+12,
				3,
				3);
	}

	if (0 == uwRepeatTimes)
		SRE_printf("[SRE_Serdes6LaneReset]:CTLE/DFE reset timeout!\n");

	/*CTLE and DFE adaptation reset release bit11 in the PUG control0*/
	SRE_SdsRegBitsWrite(node,
		MACRO_6,
		DS_API(ulDsNum)+4,
		3,
		3,
		0);

	uwRegAddr = SRE_HILINK6_MACRO_LRSTB_REG;
	/*lane reset*/
	ulRegValue = SYSTEM_REG_READ(node, uwRegAddr);
	ulRegValue &= (~(0x1 << ulDsNum));
	SYSTEM_REG_WRITE(node, uwRegAddr, ulRegValue);
	Serdes_Delay_Us(10);


	ulCsSrc = CS0;
	SRE_SerdesDsCfg(node, MACRO_6, ulDsNum, ulDsCfg, ulCsSrc);
	if (M_common_OK != SRE_SerdesDsCalib(node,
		MACRO_6,
		ulDsNum,
		ulDsCfg)) {
		SRE_printf(
			"[SRE_Serdes6LaneReset]:Macro6 Ds%d Calibrate fail!\r\n",
			ulDsNum);
		return EM_SERDES_FAIL;
	}

	SRE_DsConfigurationAfterCalibration(node,
				MACRO_6,
				ulDsNum,
				ulDsCfg,
				DcModeFlag);
	SRE_PMAInit(node,
				MACRO_6,
				ulDsNum,
				ulDsCfg,
				ulCsSrc,
				PMA_MODE_SAS);
	return EM_SERDES_SUCCESS;
}


unsigned int SRE_CommonSerdesLaneReset(unsigned int node,
					unsigned int ulMacroId,
					unsigned int ulDsNum,
					unsigned int ulDsCfg)
{
	unsigned int result = EM_SERDES_SUCCESS;

	if (EM_SERDES_SUCCESS != serdes_check_param(ulMacroId,
			ulDsNum)) {
		SRE_printf("macro%d lane %d param invalid!\n",
			ulMacroId,
			ulDsNum);
		return EM_SERDES_FAIL;
	}
	if (ulDsCfg != SAS_1500)
		SRE_printf("ulDsCfg %u invalid not support!\n",
			ulDsCfg);
	switch (ulMacroId) {
	case MACRO_0:
	case MACRO_1:
	{
		break;
	}
	case MACRO_2:
	{
		if (1 == SYSTEM_REG_READ(node,
				SRE_HILINK2_MUX_CTRL_REG))
			result = SRE_Serdes2LaneReset(node,
				ulDsNum,
				ulDsCfg);
		break;
	}
	case MACRO_3:
	case MACRO_4:
	{
		break;
	}
	case MACRO_5:
	{
		if (1 == SYSTEM_REG_READ(node,
				SRE_HILINK5_MUX_CTRL_REG))
			result = SRE_Serdes5LaneReset(node,
				ulDsNum,
				ulDsCfg);
		break;
	}
	case MACRO_6:
	{
		result = SRE_Serdes6LaneReset(node,
					ulDsNum,
					ulDsCfg);
		break;
	}
	default:
	{
		SRE_printf("invaild macro\n");
		break;
	}
	}
	return result;
}

void SRE_CommonSerdesEnableCTLEDFE(unsigned int node,
		unsigned int macro,
		unsigned int lane,
		unsigned int ulDsCfg)
{
	if ((HCCS_32 == ulDsCfg) || (HCCS_40 == ulDsCfg)) {
		SRE_SdsRegBitsWrite(node,
			macro,
			DS_API(lane)+0,
			15,
			0,
			0x783d);
		/*control_0*/
		SRE_SdsRegBitsWrite(node,
			macro,
			DS_API(lane)+4,
			15,
			0,
			0xc851);
	}
	if (PCIE_8000 == ulDsCfg) {
		/*step2 Enable CTLE/DFE*/
		/*control_0*/
		SRE_SdsRegBitsWrite(node,
			macro,
			DS_API(lane)+4,
			15,
			0,
			0x8851);
	}

	if (SAS_12000 == ulDsCfg) {
		/*step2 Enable CTLE/DFE*/
		SRE_SdsRegBitsWrite(node,
			macro,
			DS_API(lane)+0,
			15,
			0,
			0x5664);
		/*control_0*/
		SRE_SdsRegBitsWrite(node,
			macro,
			DS_API(lane)+4,
			15,
			0,
			0x7851);
	}
}


void Higgs_SerdesEnableCTLEDFE(struct HIGGS_CARD_S *v_pstLLCard,
	uint32_t v_uiPhyId)
{
	uint32_t uiDsApi = 0;
#if defined(PV660_ARM_SERVER)
	uint32_t uiHilinkId;
#endif

	HIGGS_ASSERT(NULL != v_pstLLCard, return);
	HIGGS_ASSERT(v_uiPhyId < HIGGS_MAX_PHY_NUM, return);

#if defined(EVB_VERSION_TEST)
	if (P660_SAS_CORE_PCIE_ID == v_pstLLCard->uiCardId) {
		uiDsApi = (v_uiPhyId % 4); /* phy 4~7 <-> lane 0-3 */
		SRE_SerdesEnableCTLEDFE(6, uiDsApi, 9); /* HiLink6 */
	}
#elif defined(C05_VERSION_TEST)
	if (P660_SAS_CORE_DSAF_ID == v_pstLLCard->uiCardId) {
		uiDsApi = v_uiPhyId; /* phy 0~7 <-> lane 0-7 */
		SRE_SerdesEnableCTLEDFE(2, uiDsApi, 9); /* HiLink2 */
	}

/* BEGIN: Added by c00257296, 2015/1/21 PN:arm_server*/
#elif defined(PV660_ARM_SERVER)

/* 1p arm server3.19,2P ARM SERVER3.19 */

	/* cpu node 0 sas0 */
	if (HIGGS_HILINK_TYPE_DSAF == v_pstLLCard->eHilinkType) {
		uiDsApi = v_uiPhyId; /* phy 0~7 <-> lane 0-7 */
		(void)SRE_CommonSerdesEnableCTLEDFE(
			v_pstLLCard->stSasControllerCfg.uiCpuNode,
			2,
			uiDsApi,
			9); /* HiLink2 */
	}

	/* cpu node 0 sas1 */
	if (HIGGS_HILINK_TYPE_PCIE == v_pstLLCard->eHilinkType) {
		if (v_uiPhyId < 4)
			uiHilinkId = 5;
		else
			uiHilinkId = 6;

		uiDsApi = v_uiPhyId % 4;
		/* phy 0~3 <-> hilink5 lane 0-3 , phy 4~7 : hilink6 lane 0-3*/
		(void)SRE_CommonSerdesEnableCTLEDFE(
			v_pstLLCard->stSasControllerCfg.uiCpuNode,
			uiHilinkId,
			uiDsApi,
			9);
	}
	/*chip1sas*/
/* END: Added by c00257296, 2015/1/21 */

#else
	HIGGS_REF(v_pstLLCard);
	HIGGS_REF(v_uiPhyId);
	HIGGS_REF(uiDsApi);
#endif
}

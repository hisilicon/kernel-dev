/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpaISWHqOHbuE99WnZz8Urp0KNb0WWl+qRa4EjbbH6IvvJ8uAZ7aN4Vd0NGho
uLjlYiRWhPCX+uYNGv9feR+PgmQNYQK7GaKvd4ZrFgH+7G28htvuod4u1/PyYXa3H6iWz+VH
MsAbVyR+AEFa2mPFrgpwHSGRfaZ3JLcpZdR8VS2HzW0Z8mfutK646XoP7TQGtA==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  版权所有 (C), 2011-2021, 华为技术有限公司

 ******************************************************************************
  文 件 名   : sre_crs9.h
  版 本 号   : 初稿
  作    者   : z00176027
  生成日期   : 2013年10月17日
  最近修改   :
  功能描述   : p660子系统9 复位及解复位相关头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2013年10月17日
    作    者   : z00176027
    修改内容   : 创建文件

******************************************************************************/
#ifndef __C_SRE_SUB_ALG_H__
#define __C_SRE_SUB_ALG_H__

#include "hrdCommon.h"
#include "hrdOs.h"
#include "osal_api.h"
#include "sre_crg_api.h"

#ifdef HRD_OS_LINUX
#include "iware_comm_kernel_api.h"
#endif


/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/* alg_hccs_subctrl 模块寄存器基地址 */
#define ALG_SUB_BASE                              (0xD0000000)
#define ALG_SUB_BASE_SIZE                         (0x10000)


/******************************************************************************/
/*                      PHOSPHOR ALG_SUB Registers' Definitions                            */
/******************************************************************************/

#define ALG_SUB_SC_PLLFCTRL0_REG               (ALG_SUB_BASE + 0x0)    /* PLL0控制寄存器0。 */
#define ALG_SUB_SC_PLLFCTRL1_REG               (ALG_SUB_BASE + 0x4)    /* PLL0控制寄存器1。 */
#define ALG_SUB_SC_PLLFCTRL2_REG               (ALG_SUB_BASE + 0x8)    /* PLL1控制寄存器0。 */
#define ALG_SUB_SC_PLLFCTRL3_REG               (ALG_SUB_BASE + 0xC)    /* PLL1控制寄存器1。 */
#define ALG_SUB_SC_PLLFCTRL4_REG               (ALG_SUB_BASE + 0x10)   /* PLL2控制寄存器0。 */
#define ALG_SUB_SC_PLLFCTRL5_REG               (ALG_SUB_BASE + 0x14)   /* PLL2控制寄存器1。 */
#define ALG_SUB_SC_PLLFCTRL6_REG               (ALG_SUB_BASE + 0x18)   /* PLL3控制寄存器0。 */
#define ALG_SUB_SC_PLLFCTRL7_REG               (ALG_SUB_BASE + 0x1C)   /* PLL3控制寄存器1。 */
#define ALG_SUB_SC_PLL_CLK_BYPASS0_REG         (ALG_SUB_BASE + 0x20)   /* PLL相关工作时钟选择。 */
#define ALG_SUB_SC_PLLFCTRL8_REG               (ALG_SUB_BASE + 0x24)   /* pll4控制寄存器0。 */
#define ALG_SUB_SC_PLLFCTRL9_REG               (ALG_SUB_BASE + 0x28)   /* pll4控制寄存器1。 */
#define ALG_SUB_SC_PLL_CLK_BYPASS1_REG         (ALG_SUB_BASE + 0x2C)   /* PLL相关工作时钟选择。 */
#define ALG_SUB_SC_GPIO_DB_CTRL_REG            (ALG_SUB_BASE + 0x100)  /* 配置gpip db时钟选择 */
#define ALG_SUB_SC_SFC_CLK_CTRL_REG            (ALG_SUB_BASE + 0x104)  /* 配置SFC时钟选择 */
#define ALG_SUB_SC_ALL_SCAN_CTRL_REG           (ALG_SUB_BASE + 0x150)  /* ALL SCAN控制寄存器 */
#define ALG_SUB_SC_SYSRST_CTRL_REG             (ALG_SUB_BASE + 0x200)  /* 系统软复位控制寄存器 */
#define ALG_SUB_SC_ITS_CLK_EN_REG              (ALG_SUB_BASE + 0x318)  /* ITS时钟使能寄存器 */
#define ALG_SUB_SC_ITS_CLK_DIS_REG             (ALG_SUB_BASE + 0x31C)  /* ITS时钟禁止寄存器 */
#define ALG_SUB_SC_PA_CLK_EN_REG               (ALG_SUB_BASE + 0x358)  /* PA时钟使能寄存器 */
#define ALG_SUB_SC_PA_CLK_DIS_REG              (ALG_SUB_BASE + 0x35C)  /* PA时钟禁止寄存器 */
#define ALG_SUB_SC_HLLC_CLK_EN_REG             (ALG_SUB_BASE + 0x360)  /* HLLC时钟使能寄存器 */
#define ALG_SUB_SC_HLLC_CLK_DIS_REG            (ALG_SUB_BASE + 0x364)  /* HLLC时钟禁止寄存器 */
#define ALG_SUB_SC_REF_CLK_EN_REG              (ALG_SUB_BASE + 0x370)  /* ref时钟使能寄存器 */
#define ALG_SUB_SC_REF_CLK_DIS_REG             (ALG_SUB_BASE + 0x374)  /* ref时钟禁止寄存器 */
#define ALG_SUB_SC_GPIO_DB_CLK_EN_REG          (ALG_SUB_BASE + 0x378)  /* gpio_db时钟使能寄存器 */
#define ALG_SUB_SC_GPIO_DB_CLK_DIS_REG         (ALG_SUB_BASE + 0x37C)  /* gpio_db时钟禁止寄存器 */
#define ALG_SUB_SC_DJTAG_CLK_EN_REG            (ALG_SUB_BASE + 0x380)  /* djtag时钟使能寄存器 */
#define ALG_SUB_SC_DJTAG_CLK_DIS_REG           (ALG_SUB_BASE + 0x384)  /* djtag时钟禁止寄存器 */
#define ALG_SUB_SC_HLLC_RXTX_CLK_EN_REG        (ALG_SUB_BASE + 0x390)  /* hllc rxtx时钟使能寄存器 */
#define ALG_SUB_SC_HLLC_RXTX_CLK_DIS_REG       (ALG_SUB_BASE + 0x394)  /* hllc rxtx时钟禁止寄存器 */
#define ALG_SUB_SC_RSA_CLK_EN_REG              (ALG_SUB_BASE + 0x3A0)  /* RSA时钟使能寄存器 */
#define ALG_SUB_SC_RSA_CLK_DIS_REG             (ALG_SUB_BASE + 0x3A4)  /* RSA时钟禁止寄存器 */
#define ALG_SUB_SC_RDE_CLK_EN_REG              (ALG_SUB_BASE + 0x3A8)  /* RDE时钟使能寄存器 */
#define ALG_SUB_SC_RDE_CLK_DIS_REG             (ALG_SUB_BASE + 0x3AC)  /* RDE时钟禁止寄存器 */
#define ALG_SUB_SC_RNG_CLK_EN_REG              (ALG_SUB_BASE + 0x3B0)  /* RNG时钟使能寄存器 */
#define ALG_SUB_SC_RNG_CLK_DIS_REG             (ALG_SUB_BASE + 0x3B4)  /* RNG时钟禁止寄存器 */
#define ALG_SUB_SC_SEC_CLK_EN_REG              (ALG_SUB_BASE + 0x3B8)  /* SEC时钟使能寄存器 */
#define ALG_SUB_SC_SEC_CLK_DIS_REG             (ALG_SUB_BASE + 0x3BC)  /* SEC时钟禁止寄存器 */
#define ALG_SUB_SC_ITS_RESET_REQ_REG           (ALG_SUB_BASE + 0xA18)  /* ITS软复位请求控制寄存器 */
#define ALG_SUB_SC_ITS_RESET_DREQ_REG          (ALG_SUB_BASE + 0xA1C)  /* ITS软复位去请求控制寄存器 */
#define ALG_SUB_SC_HILINK_PCS_RESET_REQ_REG    (ALG_SUB_BASE + 0xA58)  /* HILINK PCS软复位请求控制寄存器 */
#define ALG_SUB_SC_HILINK_PCS_RESET_DREQ_REG   (ALG_SUB_BASE + 0xA5C)  /* HILINK PCS软复位去请求控制寄存器 */
#define ALG_SUB_SC_HLLC_RXTX_RESET_REQ_REG     (ALG_SUB_BASE + 0xA60)  /* hllc rxtx软复位请求控制寄存器 */
#define ALG_SUB_SC_HLLC_RXTX_RESET_DREQ_REG    (ALG_SUB_BASE + 0xA64)  /* hllc rxtx软复位去请求控制寄存器 */
#define ALG_SUB_SC_PA_RESET_REQ_REG            (ALG_SUB_BASE + 0xA78)  /* PA软复位请求控制寄存器 */
#define ALG_SUB_SC_PA_RESET_DREQ_REG           (ALG_SUB_BASE + 0xA7C)  /* PA软复位去请求控制寄存器 */
#define ALG_SUB_SC_HLLC_RESET_REQ_REG          (ALG_SUB_BASE + 0xA80)  /* HLLC软复位请求控制寄存器 */
#define ALG_SUB_SC_HLLC_RESET_DREQ_REG         (ALG_SUB_BASE + 0xA84)  /* HLLC软复位去请求控制寄存器 */
#define ALG_SUB_SC_DJTAG_RESET_REQ_REG         (ALG_SUB_BASE + 0xA90)  /* DJTAG软复位请求控制寄存器 */
#define ALG_SUB_SC_DJTAG_RESET_DREQ_REG        (ALG_SUB_BASE + 0xA94)  /* DJTAG软复位去请求控制寄存器 */
#define ALG_SUB_SC_RSA_RESET_REQ_REG           (ALG_SUB_BASE + 0xA98)  /* RSA软复位请求控制寄存器 */
#define ALG_SUB_SC_RSA_RESET_DREQ_REG          (ALG_SUB_BASE + 0xA9C)  /* RSA软复位去请求控制寄存器 */
#define ALG_SUB_SC_RDE_RESET_REQ_REG           (ALG_SUB_BASE + 0xAA0)  /* RDE软复位请求控制寄存器 */
#define ALG_SUB_SC_RDE_RESET_DREQ_REG          (ALG_SUB_BASE + 0xAA4)  /* RDE软复位去请求控制寄存器 */
#define ALG_SUB_SC_SEC_RESET_REQ_REG           (ALG_SUB_BASE + 0xAA8)  /* SEC软复位请求控制寄存器 */
#define ALG_SUB_SC_SEC_RESET_DREQ_REG          (ALG_SUB_BASE + 0xAAC)  /* SEC软复位去请求控制寄存器 */
#define ALG_SUB_SC_RNG_RESET_REQ_REG           (ALG_SUB_BASE + 0xAB0)  /* RNG软复位请求控制寄存器 */
#define ALG_SUB_SC_RNG_RESET_DREQ_REG          (ALG_SUB_BASE + 0xAB4)  /* RNG软复位去请求控制寄存器 */
#define ALG_SUB_SC_SEC_BUILD_RESET_REQ_REG     (ALG_SUB_BASE + 0xAB8)  /* SEC BUILD软复位请求控制寄存器 */
#define ALG_SUB_SC_SEC_BUILD_RESET_DREQ_REG    (ALG_SUB_BASE + 0xABC)  /* SEC BUILD软复位去请求控制寄存器 */
#define ALG_SUB_SC_PCS_LOCAL_RESET_REQ_REG     (ALG_SUB_BASE + 0xAC0)  /* PCS LOCAL软复位请求控制寄存器 */
#define ALG_SUB_SC_PCS_LOCAL_RESET_DREQ_REG    (ALG_SUB_BASE + 0xAC4)  /* PCS LOCAL软复位去请求控制寄存器 */
#define ALG_SUB_SC_DISPATCH_DAW_EN_REG         (ALG_SUB_BASE + 0x1000) /* dispatch daw en配置 */
#define ALG_SUB_SC_DISPATCH_DAW_ARRAY0_REG     (ALG_SUB_BASE + 0x1004) /* dispatch daw配置阵列0 */
#define ALG_SUB_SC_DISPATCH_DAW_ARRAY1_REG     (ALG_SUB_BASE + 0x1008) /* dispatch daw配置阵列1 */
#define ALG_SUB_SC_DISPATCH_DAW_ARRAY2_REG     (ALG_SUB_BASE + 0x100C) /* dispatch daw配置阵列2 */
#define ALG_SUB_SC_DISPATCH_DAW_ARRAY3_REG     (ALG_SUB_BASE + 0x1010) /* dispatch daw配置阵列3 */
#define ALG_SUB_SC_DISPATCH_DAW_ARRAY4_REG     (ALG_SUB_BASE + 0x1014) /* dispatch daw配置阵列4 */
#define ALG_SUB_SC_DISPATCH_DAW_ARRAY5_REG     (ALG_SUB_BASE + 0x1018) /* dispatch daw配置阵列5 */
#define ALG_SUB_SC_DISPATCH_DAW_ARRAY6_REG     (ALG_SUB_BASE + 0x101C) /* dispatch daw配置阵列6 */
#define ALG_SUB_SC_DISPATCH_DAW_ARRAY7_REG     (ALG_SUB_BASE + 0x1020) /* dispatch daw配置阵列7 */
#define ALG_SUB_SC_DISPATCH_RETRY_CONTROL_REG  (ALG_SUB_BASE + 0x1030) /* dispatch retry控制寄存器 */
#define ALG_SUB_SC_DISPATCH_INTMASK_REG        (ALG_SUB_BASE + 0x1100) /* dispatch的中断屏蔽寄存器 */
#define ALG_SUB_SC_DISPATCH_RAWINT_REG         (ALG_SUB_BASE + 0x1104) /* dispatch的原始中断状态寄存器 */
#define ALG_SUB_SC_DISPATCH_INTSTAT_REG        (ALG_SUB_BASE + 0x1108) /* dispatch的屏蔽后的中断状态寄存器 */
#define ALG_SUB_SC_DISPATCH_INTCLR_REG         (ALG_SUB_BASE + 0x110C) /* dispatch的中断清除寄存器 */
#define ALG_SUB_SC_DISPATCH_ERRSTAT_REG        (ALG_SUB_BASE + 0x1110) /* dispatch的ERR状态寄存器 */
#define ALG_SUB_SC_DISPATCH_REMAP_CTRL_REG     (ALG_SUB_BASE + 0x1200) /* subsys的启动Remap寄存器 */
#define ALG_SUB_SC_TSENSOR_CTRL0_REG           (ALG_SUB_BASE + 0x20D0) /* TSENSOR控制寄存器0 */
#define ALG_SUB_SC_TSENSOR_CTRL1_REG           (ALG_SUB_BASE + 0x20D4) /* TSENSOR控制寄存器1 */
#define ALG_SUB_SC_BOOTROM_CTRL_REG            (ALG_SUB_BASE + 0x20F0) /* BOOTROM控制寄存器 */
#define ALG_SUB_SC_FTE_MUX_CTRL_REG            (ALG_SUB_BASE + 0x2200) /* FTE复用选择寄存器 */
#define ALG_SUB_SC_RSA_ADDR_H_REG              (ALG_SUB_BASE + 0x2E00) /* RSA高20位地址补充寄存器 */
#define ALG_SUB_SC_SMMU_MEM_CTRL0_REG          (ALG_SUB_BASE + 0x3000) /* smmu mem控制寄存器0 */
#define ALG_SUB_SC_SMMU_MEM_CTRL1_REG          (ALG_SUB_BASE + 0x3004) /* smmu mem控制寄存器1 */
#define ALG_SUB_SC_SMMU_MEM_CTRL2_REG          (ALG_SUB_BASE + 0x3008) /* smmu mem控制寄存器2 */
#define ALG_SUB_SC_RSA_MEM_CTRL_REG            (ALG_SUB_BASE + 0x3030) /* rsa mem控制寄存器 */
#define ALG_SUB_SC_HLLC_MEM_CTRL_REG           (ALG_SUB_BASE + 0x3040) /* hllc mem控制寄存器 */
#define ALG_SUB_SC_PA_MEM_CTRL0_REG            (ALG_SUB_BASE + 0x3050) /* pa mem控制寄存器0 */
#define ALG_SUB_SC_PA_MEM_CTRL1_REG            (ALG_SUB_BASE + 0x3054) /* pa mem控制寄存器1 */
#define ALG_SUB_SC_RDE_MEM_CTRL_REG            (ALG_SUB_BASE + 0x3060) /* rde mem控制寄存器 */
#define ALG_SUB_SC_SEC_MEM_CTRL_REG            (ALG_SUB_BASE + 0x3070) /* sec mem控制寄存器 */
#define ALG_SUB_SC_PLL_LOCK_ST_REG             (ALG_SUB_BASE + 0x5000) /* PLL锁定状态寄存器 */
#define ALG_SUB_SC_ITS_CLK_ST_REG              (ALG_SUB_BASE + 0x530C) /* ITS时钟状态寄存器 */
#define ALG_SUB_SC_PA_CLK_ST_REG               (ALG_SUB_BASE + 0x532C) /* PA时钟状态寄存器 */
#define ALG_SUB_SC_HLLC_CLK_ST_REG             (ALG_SUB_BASE + 0x5330) /* HLLC时钟状态寄存器 */
#define ALG_SUB_SC_REF_CLK_ST_REG              (ALG_SUB_BASE + 0x5338) /* ref时钟状态寄存器 */
#define ALG_SUB_SC_GPIO_DB_CLK_ST_REG          (ALG_SUB_BASE + 0x533C) /* gpio_db时钟状态寄存器 */
#define ALG_SUB_SC_DJTAG_CLK_ST_REG            (ALG_SUB_BASE + 0x5340) /* djtag时钟状态寄存器 */
#define ALG_SUB_SC_HLLC_RXTX_CLK_ST_REG        (ALG_SUB_BASE + 0x5348) /* hllc rxtx时钟状态寄存器 */
#define ALG_SUB_SC_RSA_CLK_ST_REG              (ALG_SUB_BASE + 0x5350) /* RSA时钟状态寄存器 */
#define ALG_SUB_SC_RDE_CLK_ST_REG              (ALG_SUB_BASE + 0x5354) /* RDE时钟状态寄存器 */
#define ALG_SUB_SC_RNG_CLK_ST_REG              (ALG_SUB_BASE + 0x5358) /* RNG时钟状态寄存器 */
#define ALG_SUB_SC_SEC_CLK_ST_REG              (ALG_SUB_BASE + 0x535C) /* SEC时钟状态寄存器 */
#define ALG_SUB_SC_ITS_RESET_ST_REG            (ALG_SUB_BASE + 0x5A0C) /* ITS复位状态寄存器 */
#define ALG_SUB_SC_HILINK_PCS_RESET_ST_REG     (ALG_SUB_BASE + 0x5A2C) /* HILINK_PCS复位状态寄存器 */
#define ALG_SUB_SC_HLLC_RXTX_RESET_ST_REG      (ALG_SUB_BASE + 0x5A30) /* hllc rxtx复位状态寄存器 */
#define ALG_SUB_SC_PA_RESET_ST_REG             (ALG_SUB_BASE + 0x5A3C) /* PA复位状态寄存器 */
#define ALG_SUB_SC_HLLC_RESET_ST_REG           (ALG_SUB_BASE + 0x5A40) /* HLLC复位状态寄存器 */
#define ALG_SUB_SC_DJTAG_RESET_ST_REG          (ALG_SUB_BASE + 0x5A48) /* DJTAG复位状态寄存器 */
#define ALG_SUB_SC_RSA_RESET_ST_REG            (ALG_SUB_BASE + 0x5A4C) /* RSA复位状态寄存器 */
#define ALG_SUB_SC_RDE_RESET_ST_REG            (ALG_SUB_BASE + 0x5A50) /* RDE复位状态寄存器 */
#define ALG_SUB_SC_SEC_RESET_ST_REG            (ALG_SUB_BASE + 0x5A54) /* SEC复位状态寄存器 */
#define ALG_SUB_SC_RNG_RESET_ST_REG            (ALG_SUB_BASE + 0x5A58) /* RNG复位状态寄存器 */
#define ALG_SUB_SC_SEC_BUILD_RESET_ST_REG      (ALG_SUB_BASE + 0x5A5C) /* SEC BUILD复位状态寄存器 */
#define ALG_SUB_SC_PCS_LOCAL_RESET_ST_REG      (ALG_SUB_BASE + 0x5A60) /* PCS LOCAL软复位请求状态寄存器 */
#define ALG_SUB_SC_TSENSOR_ST_REG              (ALG_SUB_BASE + 0x6040) /* TSENSOR状态寄存器 */
#define ALG_SUB_SC_DJTAG_INT_REG               (ALG_SUB_BASE + 0x6600) /* DJTAG Master内状态机超时中断源寄存器 */
#define ALG_SUB_SC_DBG_AUTH_CTRL_REG           (ALG_SUB_BASE + 0x6700) /* 安全等级授权控制寄存器。（注意：1，此寄存器空间必须是安全访问才能成功；2，默认均是打开调试模式，有漏洞，在正常工作时请初始化关闭调试功能） */
#define ALG_SUB_SC_DJTAG_MSTR_EN_REG           (ALG_SUB_BASE + 0x6800) /* SC_DJTAG_MSTR_EN为DJTAG Master使能控制 */
#define ALG_SUB_SC_DJTAG_MSTR_START_EN_REG     (ALG_SUB_BASE + 0x6804) /* SC_DJTAG_MSTR_START_EN为DJTAG Master访问chain链的start使能 */
#define ALG_SUB_SC_DJTAG_SEC_ACC_EN_REG        (ALG_SUB_BASE + 0x6808) /* DJTAG访问各模块内安全世界寄存器的安全访问控制使能。（注意：1，此寄存器空间必须是安全访问才能成功。） */
#define ALG_SUB_SC_DJTAG_DEBUG_MODULE_SEL_REG  (ALG_SUB_BASE + 0x680C) /* SC_DJTAG_DEBUG_MODULE_SEL为DJTAG Master配置访问模块寄存器 */
#define ALG_SUB_SC_DJTAG_MSTR_WR_REG           (ALG_SUB_BASE + 0x6810) /* SC_DJTAG_MSTR_WR为DJTAG Master的读写控制信号 */
#define ALG_SUB_SC_DJTAG_CHAIN_UNIT_CFG_EN_REG (ALG_SUB_BASE + 0x6814) /* SC_DJTAG_CHAIN_UNIT_CFG_EN为DJTAG Master的chain链上单元的访问使能 */
#define ALG_SUB_SC_DJTAG_MSTR_ADDR_REG         (ALG_SUB_BASE + 0x6818) /* SC_DJTAG_MSTR_ADDR为DJTAG Master配置访问模块的偏移地址寄存器 */
#define ALG_SUB_SC_DJTAG_MSTR_DATA_REG         (ALG_SUB_BASE + 0x681C) /* SC_DJTAG_MSTR_DATA为DJTAG Master的写数据 */
#define ALG_SUB_SC_DJTAG_TMOUT_REG             (ALG_SUB_BASE + 0x6820) /* SC_DJTAG_TMOUT为IDIE中DJTAG Master的状态机超时阈值 */
#define ALG_SUB_SC_TDRE_OP_ADDR_REG            (ALG_SUB_BASE + 0x6824) /* SC_TDRE_OP_ADDR为DJTAG Master配置TDRE模块的偏移地址寄存器和操作类型寄存器 */
#define ALG_SUB_SC_TDRE_WDATA_REG              (ALG_SUB_BASE + 0x6828) /* SC_TDRE_WDATA为DJTAG Master的写TDRE数据 */
#define ALG_SUB_SC_TDRE_REPAIR_EN_REG          (ALG_SUB_BASE + 0x682C) /* SC_TDRE_REPAIR_EN为DJTAG Master的TDRE REPAIR使能 */
#define ALG_SUB_SC_ECO_RSV0_REG                (ALG_SUB_BASE + 0x8000) /* ECO 寄存器0 */
#define ALG_SUB_SC_ECO_RSV1_REG                (ALG_SUB_BASE + 0x8004) /* ECO 寄存器1 */
#define ALG_SUB_SC_ECO_RSV2_REG                (ALG_SUB_BASE + 0x8008) /* ECO 寄存器2 */
#define ALG_SUB_SC_DJTAG_RD_DATA0_REG          (ALG_SUB_BASE + 0xE800) /* DJTAG扫描链上第一个模块读数据寄存器 */
#define ALG_SUB_SC_DJTAG_RD_DATA1_REG          (ALG_SUB_BASE + 0xE804) /* DJTAG扫描链上第二个模块读数据寄存器 */
#define ALG_SUB_SC_DJTAG_RD_DATA2_REG          (ALG_SUB_BASE + 0xE808) /* DJTAG扫描链上第三个模块读数据寄存器 */
#define ALG_SUB_SC_DJTAG_RD_DATA3_REG          (ALG_SUB_BASE + 0xE80C) /* DJTAG扫描链上第四个模块读数据寄存器 */
#define ALG_SUB_SC_DJTAG_RD_DATA4_REG          (ALG_SUB_BASE + 0xE810) /* DJTAG扫描链上第五个模块读数据寄存器 */
#define ALG_SUB_SC_DJTAG_RD_DATA5_REG          (ALG_SUB_BASE + 0xE814) /* DJTAG扫描链上第六个模块读数据寄存器 */
#define ALG_SUB_SC_DJTAG_RD_DATA6_REG          (ALG_SUB_BASE + 0xE818) /* DJTAG扫描链上第七个模块读数据寄存器 */
#define ALG_SUB_SC_DJTAG_RD_DATA7_REG          (ALG_SUB_BASE + 0xE81C) /* DJTAG扫描链上第八个模块读数据寄存器 */
#define ALG_SUB_SC_DJTAG_RD_DATA8_REG          (ALG_SUB_BASE + 0xE820) /* DJTAG扫描链上第九个模块读数据寄存器 */
#define ALG_SUB_SC_DJTAG_RD_DATA9_REG          (ALG_SUB_BASE + 0xE824) /* DJTAG扫描链上第十个模块读数据寄存器 */
#define ALG_SUB_SC_DJTAG_OP_ST_REG             (ALG_SUB_BASE + 0xE828) /* DJTAG访问状态寄存器 */
#define ALG_SUB_SC_TDRE_RDATA1_REG             (ALG_SUB_BASE + 0xE82C) /* DJTAG访问TDRE模块的读数据高32bit */
#define ALG_SUB_SC_TDRE_RDATA0_REG             (ALG_SUB_BASE + 0xE830) /* DJTAG访问TDRE模块的读数据低32bit */


#if(ENDNESS == ENDNESS_BIG)
/* Define the union U_SC_PLLFCTRL0_U */
/* PLL0控制寄存器0。 */
/* 0x0 */
typedef union tagScPllfctrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    pll0_foutvcopd        : 1   ; /* [30]  */
        unsigned int    pll0_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll0_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll0_dacpd            : 1   ; /* [27]  */
        unsigned int    pll0_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll0_pd               : 1   ; /* [25]  */
        unsigned int    pll0_bypass           : 1   ; /* [24]  */
        unsigned int    pll0_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll0_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll0_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll0_refdiv           : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL0_U;

/* Define the union U_SC_PLLFCTRL1_U */
/* PLL0控制寄存器1。 */
/* 0x4 */
typedef union tagScPllfctrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
        unsigned int    pll0_frac             : 24  ; /* [23..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL1_U;

/* Define the union U_SC_PLLFCTRL2_U */
/* PLL1控制寄存器0。 */
/* 0x8 */
typedef union tagScPllfctrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    pll1_foutvcopd        : 1   ; /* [30]  */
        unsigned int    pll1_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll1_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll1_dacpd            : 1   ; /* [27]  */
        unsigned int    pll1_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll1_pd               : 1   ; /* [25]  */
        unsigned int    pll1_bypass           : 1   ; /* [24]  */
        unsigned int    pll1_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll1_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll1_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll1_refdiv           : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL2_U;

/* Define the union U_SC_PLLFCTRL3_U */
/* PLL1控制寄存器1。 */
/* 0xC */
typedef union tagScPllfctrl3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
        unsigned int    pll1_frac             : 24  ; /* [23..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL3_U;

/* Define the union U_SC_PLLFCTRL4_U */
/* PLL2控制寄存器0。 */
/* 0x10 */
typedef union tagScPllfctrl4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    pll2_foutvcopd        : 1   ; /* [30]  */
        unsigned int    pll2_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll2_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll2_dacpd            : 1   ; /* [27]  */
        unsigned int    pll2_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll2_pd               : 1   ; /* [25]  */
        unsigned int    pll2_bypass           : 1   ; /* [24]  */
        unsigned int    pll2_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll2_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll2_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll2_refdiv           : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL4_U;

/* Define the union U_SC_PLLFCTRL5_U */
/* PLL2控制寄存器1。 */
/* 0x14 */
typedef union tagScPllfctrl5
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
        unsigned int    pll2_frac             : 24  ; /* [23..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL5_U;

/* Define the union U_SC_PLLFCTRL6_U */
/* PLL3控制寄存器0。 */
/* 0x18 */
typedef union tagScPllfctrl6
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    pll3_foutvcopd        : 1   ; /* [30]  */
        unsigned int    pll3_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll3_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll3_dacpd            : 1   ; /* [27]  */
        unsigned int    pll3_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll3_pd               : 1   ; /* [25]  */
        unsigned int    pll3_bypass           : 1   ; /* [24]  */
        unsigned int    pll3_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll3_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll3_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll3_refdiv           : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL6_U;

/* Define the union U_SC_PLLFCTRL7_U */
/* PLL3控制寄存器1。 */
/* 0x1C */
typedef union tagScPllfctrl7
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
        unsigned int    pll3_frac             : 24  ; /* [23..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL7_U;

/* Define the union U_SC_PLL_CLK_BYPASS_U */
/* PLL相关工作时钟选择。 */
/* 0x20 */
typedef union tagScPllClkBypass0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
        unsigned int    pll4_peri_mode        : 1   ; /* [20]  */
        unsigned int    pll3_peri_mode        : 1   ; /* [19]  */
        unsigned int    pll2_peri_mode        : 1   ; /* [18]  */
        unsigned int    pll1_peri_mode        : 1   ; /* [17]  */
        unsigned int    pll0_peri_mode        : 1   ; /* [16]  */
        unsigned int    reserved_1            : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_CLK_BYPASS0_U;

/* Define the union U_SC_PLLFCTRL8_U */
/* pll4控制寄存器0。 */
/* 0x24 */
typedef union tagScPllfctrl8
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    pll4_foutvcopd        : 1   ; /* [30]  */
        unsigned int    pll4_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll4_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll4_dacpd            : 1   ; /* [27]  */
        unsigned int    pll4_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll4_pd               : 1   ; /* [25]  */
        unsigned int    pll4_bypass           : 1   ; /* [24]  */
        unsigned int    pll4_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll4_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll4_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll4_refdiv           : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL8_U;

/* Define the union U_SC_PLLFCTRL9_U */
/* pll4控制寄存器1。 */
/* 0x28 */
typedef union tagScPllfctrl9
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
        unsigned int    pll4_frac             : 24  ; /* [23..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL9_U;

/* Define the union U_SC_PLL_CLK_BYPASS1_U */
/* PLL相关工作时钟选择。 */
/* 0x2C */
typedef union tagScPllClkBypass1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    pll4_bypass_external_n : 1   ; /* [4]  */
        unsigned int    pll3_bypass_external_n : 1   ; /* [3]  */
        unsigned int    pll2_bypass_external_n : 1   ; /* [2]  */
        unsigned int    pll1_bypass_external_n : 1   ; /* [1]  */
        unsigned int    pll0_bypass_external_n : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_CLK_BYPASS1_U;

/* Define the union U_SC_GPIO_DB_CTRL_U */
/* 配置gpip db时钟选择 */
/* 0x100 */
typedef union tagScGpioDbCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    gpio_db_clk_sel       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_DB_CTRL_U;

/* Define the union U_SC_SFC_CLK_CTRL_U */
/* 配置SFC时钟选择 */
/* 0x104 */
typedef union tagScSfcClkCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sfc_clk_sel           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_CLK_CTRL_U;

/* Define the union U_SC_ALL_SCAN_CTRL_U */
/* ALL SCAN控制寄存器 */
/* 0x150 */
typedef union tagScAllScanCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    all_scan_sys_int      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ALL_SCAN_CTRL_U;

/* Define the union U_SC_ITS_CLK_EN_U */
/* ITS时钟使能寄存器 */
/* 0x318 */
typedef union tagScItsClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_its_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_CLK_EN_U;

/* Define the union U_SC_ITS_CLK_DIS_U */
/* ITS时钟禁止寄存器 */
/* 0x31C */
typedef union tagScItsClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_its_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_CLK_DIS_U;

/* Define the union U_SC_PA_CLK_EN_U */
/* PA时钟使能寄存器 */
/* 0x358 */
typedef union tagScPaClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_pa_enb            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_CLK_EN_U;

/* Define the union U_SC_PA_CLK_DIS_U */
/* PA时钟禁止寄存器 */
/* 0x35C */
typedef union tagScPaClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_pa_dsb            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_CLK_DIS_U;

/* Define the union U_SC_HLLC_CLK_EN_U */
/* HLLC时钟使能寄存器 */
/* 0x360 */
typedef union tagScHllcClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_hllc_enb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_CLK_EN_U;

/* Define the union U_SC_HLLC_CLK_DIS_U */
/* HLLC时钟禁止寄存器 */
/* 0x364 */
typedef union tagScHllcClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_hllc_dsb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_CLK_DIS_U;

/* Define the union U_SC_REF_CLK_EN_U */
/* ref时钟使能寄存器 */
/* 0x370 */
typedef union tagScRefClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_ref_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REF_CLK_EN_U;

/* Define the union U_SC_REF_CLK_DIS_U */
/* ref时钟禁止寄存器 */
/* 0x374 */
typedef union tagScRefClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_ref_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REF_CLK_DIS_U;

/* Define the union U_SC_GPIO_DB_CLK_EN_U */
/* gpio_db时钟使能寄存器 */
/* 0x378 */
typedef union tagScGpioDbClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_gpio_db_enb       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_DB_CLK_EN_U;

/* Define the union U_SC_GPIO_DB_CLK_DIS_U */
/* gpio_db时钟禁止寄存器 */
/* 0x37C */
typedef union tagScGpioDbClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_gpio_db_dsb       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_DB_CLK_DIS_U;

/* Define the union U_SC_DJTAG_CLK_EN_U */
/* djtag时钟使能寄存器 */
/* 0x380 */
typedef union tagScDjtagClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_djtag_enb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CLK_EN_U;

/* Define the union U_SC_DJTAG_CLK_DIS_U */
/* djtag时钟禁止寄存器 */
/* 0x384 */
typedef union tagScDjtagClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_djtag_dsb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CLK_DIS_U;

/* Define the union U_SC_HLLC_RXTX_CLK_EN_U */
/* hllc rxtx时钟使能寄存器 */
/* 0x390 */
typedef union tagScHllcRxtxClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    clk_hllc_hilink_rx7_enb : 1   ; /* [15]  */
        unsigned int    clk_hllc_hilink_rx6_enb : 1   ; /* [14]  */
        unsigned int    clk_hllc_hilink_rx5_enb : 1   ; /* [13]  */
        unsigned int    clk_hllc_hilink_rx4_enb : 1   ; /* [12]  */
        unsigned int    clk_hllc_hilink_rx3_enb : 1   ; /* [11]  */
        unsigned int    clk_hllc_hilink_rx2_enb : 1   ; /* [10]  */
        unsigned int    clk_hllc_hilink_rx1_enb : 1   ; /* [9]  */
        unsigned int    clk_hllc_hilink_rx0_enb : 1   ; /* [8]  */
        unsigned int    clk_hllc_hilink_tx7_enb : 1   ; /* [7]  */
        unsigned int    clk_hllc_hilink_tx6_enb : 1   ; /* [6]  */
        unsigned int    clk_hllc_hilink_tx5_enb : 1   ; /* [5]  */
        unsigned int    clk_hllc_hilink_tx4_enb : 1   ; /* [4]  */
        unsigned int    clk_hllc_hilink_tx3_enb : 1   ; /* [3]  */
        unsigned int    clk_hllc_hilink_tx2_enb : 1   ; /* [2]  */
        unsigned int    clk_hllc_hilink_tx1_enb : 1   ; /* [1]  */
        unsigned int    clk_hllc_hilink_tx0_enb : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_CLK_EN_U;

/* Define the union U_SC_HLLC_RXTX_CLK_DIS_U */
/* hllc rxtx时钟禁止寄存器 */
/* 0x394 */
typedef union tagScHllcRxtxClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    clk_hllc_hilink_rx7_dsb : 1   ; /* [15]  */
        unsigned int    clk_hllc_hilink_rx6_dsb : 1   ; /* [14]  */
        unsigned int    clk_hllc_hilink_rx5_dsb : 1   ; /* [13]  */
        unsigned int    clk_hllc_hilink_rx4_dsb : 1   ; /* [12]  */
        unsigned int    clk_hllc_hilink_rx3_dsb : 1   ; /* [11]  */
        unsigned int    clk_hllc_hilink_rx2_dsb : 1   ; /* [10]  */
        unsigned int    clk_hllc_hilink_rx1_dsb : 1   ; /* [9]  */
        unsigned int    clk_hllc_hilink_rx0_dsb : 1   ; /* [8]  */
        unsigned int    clk_hllc_hilink_tx7_dsb : 1   ; /* [7]  */
        unsigned int    clk_hllc_hilink_tx6_dsb : 1   ; /* [6]  */
        unsigned int    clk_hllc_hilink_tx5_dsb : 1   ; /* [5]  */
        unsigned int    clk_hllc_hilink_tx4_dsb : 1   ; /* [4]  */
        unsigned int    clk_hllc_hilink_tx3_dsb : 1   ; /* [3]  */
        unsigned int    clk_hllc_hilink_tx2_dsb : 1   ; /* [2]  */
        unsigned int    clk_hllc_hilink_tx1_dsb : 1   ; /* [1]  */
        unsigned int    clk_hllc_hilink_tx0_dsb : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_CLK_DIS_U;

/* Define the union U_SC_RSA_CLK_EN_U */
/* RSA时钟使能寄存器 */
/* 0x3A0 */
typedef union tagScRsaClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rsa_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_CLK_EN_U;

/* Define the union U_SC_RSA_CLK_DIS_U */
/* RSA时钟禁止寄存器 */
/* 0x3A4 */
typedef union tagScRsaClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rsa_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_CLK_DIS_U;

/* Define the union U_SC_RDE_CLK_EN_U */
/* RDE时钟使能寄存器 */
/* 0x3A8 */
typedef union tagScRdeClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rde_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_CLK_EN_U;

/* Define the union U_SC_RDE_CLK_DIS_U */
/* RDE时钟禁止寄存器 */
/* 0x3AC */
typedef union tagScRdeClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rde_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_CLK_DIS_U;

/* Define the union U_SC_RNG_CLK_EN_U */
/* RNG时钟使能寄存器 */
/* 0x3B0 */
typedef union tagScRngClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rng_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_CLK_EN_U;

/* Define the union U_SC_RNG_CLK_DIS_U */
/* RNG时钟禁止寄存器 */
/* 0x3B4 */
typedef union tagScRngClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rng_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_CLK_DIS_U;

/* Define the union U_SC_SEC_CLK_EN_U */
/* SEC时钟使能寄存器 */
/* 0x3B8 */
typedef union tagScSecClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    clk_sec_trace_dbg_enb : 1   ; /* [2]  */
        unsigned int    clk_sec_ahb_enb       : 1   ; /* [1]  */
        unsigned int    clk_sec_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_CLK_EN_U;

/* Define the union U_SC_SEC_CLK_DIS_U */
/* SEC时钟禁止寄存器 */
/* 0x3BC */
typedef union tagScSecClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    clk_sec_trace_dbg_dsb : 1   ; /* [2]  */
        unsigned int    clk_sec_ahb_dsb       : 1   ; /* [1]  */
        unsigned int    clk_sec_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_CLK_DIS_U;

/* Define the union U_SC_ITS_RESET_REQ_U */
/* ITS软复位请求控制寄存器 */
/* 0xA18 */
typedef union tagScItsResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    its_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_RESET_REQ_U;

/* Define the union U_SC_ITS_RESET_DREQ_U */
/* ITS软复位去请求控制寄存器 */
/* 0xA1C */
typedef union tagScItsResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    its_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_RESET_DREQ_U;

/* Define the union U_SC_HILINK_PCS_RESET_REQ_U */
/* HILINK PCS软复位请求控制寄存器 */
/* 0xA58 */
typedef union tagScHilinkPcsResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hllc_hilink_pcs_lane7_srst_req : 1   ; /* [7]  */
        unsigned int    hllc_hilink_pcs_lane6_srst_req : 1   ; /* [6]  */
        unsigned int    hllc_hilink_pcs_lane5_srst_req : 1   ; /* [5]  */
        unsigned int    hllc_hilink_pcs_lane4_srst_req : 1   ; /* [4]  */
        unsigned int    hllc_hilink_pcs_lane3_srst_req : 1   ; /* [3]  */
        unsigned int    hllc_hilink_pcs_lane2_srst_req : 1   ; /* [2]  */
        unsigned int    hllc_hilink_pcs_lane1_srst_req : 1   ; /* [1]  */
        unsigned int    hllc_hilink_pcs_lane0_srst_req : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK_PCS_RESET_REQ_U;

/* Define the union U_SC_HILINK_PCS_RESET_DREQ_U */
/* HILINK PCS软复位去请求控制寄存器 */
/* 0xA5C */
typedef union tagScHilinkPcsResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hllc_hilink_pcs_lane7_srst_dreq : 1   ; /* [7]  */
        unsigned int    hllc_hilink_pcs_lane6_srst_dreq : 1   ; /* [6]  */
        unsigned int    hllc_hilink_pcs_lane5_srst_dreq : 1   ; /* [5]  */
        unsigned int    hllc_hilink_pcs_lane4_srst_dreq : 1   ; /* [4]  */
        unsigned int    hllc_hilink_pcs_lane3_srst_dreq : 1   ; /* [3]  */
        unsigned int    hllc_hilink_pcs_lane2_srst_dreq : 1   ; /* [2]  */
        unsigned int    hllc_hilink_pcs_lane1_srst_dreq : 1   ; /* [1]  */
        unsigned int    hllc_hilink_pcs_lane0_srst_dreq : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK_PCS_RESET_DREQ_U;

/* Define the union U_SC_HLLC_RXTX_RESET_REQ_U */
/* hllc rxtx软复位请求控制寄存器 */
/* 0xA60 */
typedef union tagScHllcRxtxResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    hllc_tx7_srst_req     : 1   ; /* [15]  */
        unsigned int    hllc_tx6_srst_req     : 1   ; /* [14]  */
        unsigned int    hllc_tx5_srst_req     : 1   ; /* [13]  */
        unsigned int    hllc_tx4_srst_req     : 1   ; /* [12]  */
        unsigned int    hllc_tx3_srst_req     : 1   ; /* [11]  */
        unsigned int    hllc_tx2_srst_req     : 1   ; /* [10]  */
        unsigned int    hllc_tx1_srst_req     : 1   ; /* [9]  */
        unsigned int    hllc_tx0_srst_req     : 1   ; /* [8]  */
        unsigned int    hllc_rx7_srst_req     : 1   ; /* [7]  */
        unsigned int    hllc_rx6_srst_req     : 1   ; /* [6]  */
        unsigned int    hllc_rx5_srst_req     : 1   ; /* [5]  */
        unsigned int    hllc_rx4_srst_req     : 1   ; /* [4]  */
        unsigned int    hllc_rx3_srst_req     : 1   ; /* [3]  */
        unsigned int    hllc_rx2_srst_req     : 1   ; /* [2]  */
        unsigned int    hllc_rx1_srst_req     : 1   ; /* [1]  */
        unsigned int    hllc_rx0_srst_req     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_RESET_REQ_U;

/* Define the union U_SC_HLLC_RXTX_RESET_DREQ_U */
/* hllc rxtx软复位去请求控制寄存器 */
/* 0xA64 */
typedef union tagScHllcRxtxResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    hllc_tx7_srst_dreq    : 1   ; /* [15]  */
        unsigned int    hllc_tx6_srst_dreq    : 1   ; /* [14]  */
        unsigned int    hllc_tx5_srst_dreq    : 1   ; /* [13]  */
        unsigned int    hllc_tx4_srst_dreq    : 1   ; /* [12]  */
        unsigned int    hllc_tx3_srst_dreq    : 1   ; /* [11]  */
        unsigned int    hllc_tx2_srst_dreq    : 1   ; /* [10]  */
        unsigned int    hllc_tx1_srst_dreq    : 1   ; /* [9]  */
        unsigned int    hllc_tx0_srst_dreq    : 1   ; /* [8]  */
        unsigned int    hllc_rx7_srst_dreq    : 1   ; /* [7]  */
        unsigned int    hllc_rx6_srst_dreq    : 1   ; /* [6]  */
        unsigned int    hllc_rx5_srst_dreq    : 1   ; /* [5]  */
        unsigned int    hllc_rx4_srst_dreq    : 1   ; /* [4]  */
        unsigned int    hllc_rx3_srst_dreq    : 1   ; /* [3]  */
        unsigned int    hllc_rx2_srst_dreq    : 1   ; /* [2]  */
        unsigned int    hllc_rx1_srst_dreq    : 1   ; /* [1]  */
        unsigned int    hllc_rx0_srst_dreq    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_RESET_DREQ_U;

/* Define the union U_SC_PA_RESET_REQ_U */
/* PA软复位请求控制寄存器 */
/* 0xA78 */
typedef union tagScPaResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pa_srst_req           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_RESET_REQ_U;

/* Define the union U_SC_PA_RESET_DREQ_U */
/* PA软复位去请求控制寄存器 */
/* 0xA7C */
typedef union tagScPaResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pa_srst_dreq          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_RESET_DREQ_U;

/* Define the union U_SC_HLLC_RESET_REQ_U */
/* HLLC软复位请求控制寄存器 */
/* 0xA80 */
typedef union tagScHllcResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hllc_srst_req         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RESET_REQ_U;

/* Define the union U_SC_HLLC_RESET_DREQ_U */
/* HLLC软复位去请求控制寄存器 */
/* 0xA84 */
typedef union tagScHllcResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hllc_srst_dreq        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RESET_DREQ_U;

/* Define the union U_SC_DJTAG_RESET_REQ_U */
/* DJTAG软复位请求控制寄存器 */
/* 0xA90 */
typedef union tagScDjtagResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    djtag_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_RESET_REQ_U;

/* Define the union U_SC_DJTAG_RESET_DREQ_U */
/* DJTAG软复位去请求控制寄存器 */
/* 0xA94 */
typedef union tagScDjtagResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    djtag_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_RESET_DREQ_U;

/* Define the union U_SC_RSA_RESET_REQ_U */
/* RSA软复位请求控制寄存器 */
/* 0xA98 */
typedef union tagScRsaResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rsa_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_RESET_REQ_U;

/* Define the union U_SC_RSA_RESET_DREQ_U */
/* RSA软复位去请求控制寄存器 */
/* 0xA9C */
typedef union tagScRsaResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rsa_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_RESET_DREQ_U;

/* Define the union U_SC_RDE_RESET_REQ_U */
/* RDE软复位请求控制寄存器 */
/* 0xAA0 */
typedef union tagScRdeResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rde_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_RESET_REQ_U;

/* Define the union U_SC_RDE_RESET_DREQ_U */
/* RDE软复位去请求控制寄存器 */
/* 0xAA4 */
typedef union tagScRdeResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rde_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_RESET_DREQ_U;

/* Define the union U_SC_SEC_RESET_REQ_U */
/* SEC软复位请求控制寄存器 */
/* 0xAA8 */
typedef union tagScSecResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sec_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_RESET_REQ_U;

/* Define the union U_SC_SEC_RESET_DREQ_U */
/* SEC软复位去请求控制寄存器 */
/* 0xAAC */
typedef union tagScSecResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sec_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_RESET_DREQ_U;

/* Define the union U_SC_RNG_RESET_REQ_U */
/* RNG软复位请求控制寄存器 */
/* 0xAB0 */
typedef union tagScRngResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rng_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_RESET_REQ_U;

/* Define the union U_SC_RNG_RESET_DREQ_U */
/* RNG软复位去请求控制寄存器 */
/* 0xAB4 */
typedef union tagScRngResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rng_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_RESET_DREQ_U;

/* Define the union U_SC_SEC_BUILD_RESET_REQ_U */
/* SEC BUILD软复位请求控制寄存器 */
/* 0xAB8 */
typedef union tagScSecBuildResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
        unsigned int    sec_ahb_srst_req      : 1   ; /* [20]  */
        unsigned int    sec_build_ahb_srst_req7 : 1   ; /* [19]  */
        unsigned int    sec_build_srst_req7   : 1   ; /* [18]  */
        unsigned int    sec_build_ahb_srst_req6 : 1   ; /* [17]  */
        unsigned int    sec_build_srst_req6   : 1   ; /* [16]  */
        unsigned int    sec_build_ahb_srst_req5 : 1   ; /* [15]  */
        unsigned int    sec_build_srst_req5   : 1   ; /* [14]  */
        unsigned int    sec_build_ahb_srst_req4 : 1   ; /* [13]  */
        unsigned int    sec_build_srst_req4   : 1   ; /* [12]  */
        unsigned int    sec_cluster1_ahb_srst_req : 1   ; /* [11]  */
        unsigned int    sec_cluster1_srst_req : 1   ; /* [10]  */
        unsigned int    sec_build_ahb_srst_req3 : 1   ; /* [9]  */
        unsigned int    sec_build_srst_req3   : 1   ; /* [8]  */
        unsigned int    sec_build_ahb_srst_req2 : 1   ; /* [7]  */
        unsigned int    sec_build_srst_req2   : 1   ; /* [6]  */
        unsigned int    sec_build_ahb_srst_req1 : 1   ; /* [5]  */
        unsigned int    sec_build_srst_req1   : 1   ; /* [4]  */
        unsigned int    sec_build_ahb_srst_req0 : 1   ; /* [3]  */
        unsigned int    sec_build_srst_req0   : 1   ; /* [2]  */
        unsigned int    sec_cluster0_ahb_srst_req : 1   ; /* [1]  */
        unsigned int    sec_cluster0_srst_req : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_BUILD_RESET_REQ_U;

/* Define the union U_SC_SEC_BUILD_RESET_DREQ_U */
/* SEC BUILD软复位去请求控制寄存器 */
/* 0xABC */
typedef union tagScSecBuildResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
        unsigned int    sec_ahb_srst_dreq     : 1   ; /* [20]  */
        unsigned int    sec_build_ahb_srst_dreq7 : 1   ; /* [19]  */
        unsigned int    sec_build_srst_dreq7  : 1   ; /* [18]  */
        unsigned int    sec_build_ahb_srst_dreq6 : 1   ; /* [17]  */
        unsigned int    sec_build_srst_dreq6  : 1   ; /* [16]  */
        unsigned int    sec_build_ahb_srst_dreq5 : 1   ; /* [15]  */
        unsigned int    sec_build_srst_dreq5  : 1   ; /* [14]  */
        unsigned int    sec_build_ahb_srst_dreq4 : 1   ; /* [13]  */
        unsigned int    sec_build_srst_dreq4  : 1   ; /* [12]  */
        unsigned int    sec_cluster1_ahb_srst_dreq : 1   ; /* [11]  */
        unsigned int    sec_cluster1_srst_dreq : 1   ; /* [10]  */
        unsigned int    sec_build_ahb_srst_dreq3 : 1   ; /* [9]  */
        unsigned int    sec_build_srst_dreq3  : 1   ; /* [8]  */
        unsigned int    sec_build_ahb_srst_dreq2 : 1   ; /* [7]  */
        unsigned int    sec_build_srst_dreq2  : 1   ; /* [6]  */
        unsigned int    sec_build_ahb_srst_dreq1 : 1   ; /* [5]  */
        unsigned int    sec_build_srst_dreq1  : 1   ; /* [4]  */
        unsigned int    sec_build_ahb_srst_dreq0 : 1   ; /* [3]  */
        unsigned int    sec_build_srst_dreq0  : 1   ; /* [2]  */
        unsigned int    sec_cluster0_ahb_srst_dreq : 1   ; /* [1]  */
        unsigned int    sec_cluster0_srst_dreq : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_BUILD_RESET_DREQ_U;

/* Define the union U_SC_PCS_LOCAL_RESET_REQ_U */
/* PCS LOCAL软复位请求控制寄存器 */
/* 0xAC0 */
typedef union tagScPcsLocalResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcs_local_srst_req    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_REQ_U;

/* Define the union U_SC_PCS_LOCAL_RESET_DREQ_U */
/* PCS LOCAL软复位去请求控制寄存器 */
/* 0xAC4 */
typedef union tagScPcsLocalResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcs_local_srst_dreq   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_DREQ_U;

/* Define the union U_SC_DISPATCH_DAW_EN_U */
/* dispatch daw en配置 */
/* 0x1000 */
typedef union tagScDispatchDawEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    dispatch_daw_en       : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_EN_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY0_U */
/* dispatch daw配置阵列0 */
/* 0x1004 */
typedef union tagScDispatchDawArray0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array0_addr       : 19  ; /* [31..13]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array0_sync       : 1   ; /* [8]  */
        unsigned int    daw_array0_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array0_did        : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY0_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY1_U */
/* dispatch daw配置阵列1 */
/* 0x1008 */
typedef union tagScDispatchDawArray1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array1_addr       : 19  ; /* [31..13]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array1_sync       : 1   ; /* [8]  */
        unsigned int    daw_array1_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array1_did        : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY1_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY2_U */
/* dispatch daw配置阵列2 */
/* 0x100C */
typedef union tagScDispatchDawArray2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array2_addr       : 19  ; /* [31..13]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array2_sync       : 1   ; /* [8]  */
        unsigned int    daw_array2_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array2_did        : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY2_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY3_U */
/* dispatch daw配置阵列3 */
/* 0x1010 */
typedef union tagScDispatchDawArray3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array3_addr       : 19  ; /* [31..13]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array3_sync       : 1   ; /* [8]  */
        unsigned int    daw_array3_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array3_did        : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY3_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY4_U */
/* dispatch daw配置阵列4 */
/* 0x1014 */
typedef union tagScDispatchDawArray4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array4_addr       : 19  ; /* [31..13]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array4_sync       : 1   ; /* [8]  */
        unsigned int    daw_array4_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array4_did        : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY4_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY5_U */
/* dispatch daw配置阵列5 */
/* 0x1018 */
typedef union tagScDispatchDawArray5
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array5_addr       : 19  ; /* [31..13]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array5_sync       : 1   ; /* [8]  */
        unsigned int    daw_array5_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array5_did        : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY5_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY6_U */
/* dispatch daw配置阵列6 */
/* 0x101C */
typedef union tagScDispatchDawArray6
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array6_addr       : 19  ; /* [31..13]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array6_sync       : 1   ; /* [8]  */
        unsigned int    daw_array6_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array6_did        : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY6_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY7_U */
/* dispatch daw配置阵列7 */
/* 0x1020 */
typedef union tagScDispatchDawArray7
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array7_addr       : 19  ; /* [31..13]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array7_sync       : 1   ; /* [8]  */
        unsigned int    daw_array7_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array7_did        : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY7_U;

/* Define the union U_SC_DISPATCH_RETRY_CONTROL_U */
/* dispatch retry控制寄存器 */
/* 0x1030 */
typedef union tagScDispatchRetryControl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
        unsigned int    retry_en              : 1   ; /* [16]  */
        unsigned int    retry_num_limit       : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_RETRY_CONTROL_U;

/* Define the union U_SC_DISPATCH_INTMASK_U */
/* dispatch的中断屏蔽寄存器 */
/* 0x1100 */
typedef union tagScDispatchIntmask
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    intmask               : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_INTMASK_U;

/* Define the union U_SC_DISPATCH_RAWINT_U */
/* dispatch的原始中断状态寄存器 */
/* 0x1104 */
typedef union tagScDispatchRawint
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rawint                : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_RAWINT_U;

/* Define the union U_SC_DISPATCH_INTSTAT_U */
/* dispatch的屏蔽后的中断状态寄存器 */
/* 0x1108 */
typedef union tagScDispatchIntstat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    intsts                : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_INTSTAT_U;

/* Define the union U_SC_DISPATCH_INTCLR_U */
/* dispatch的中断清除寄存器 */
/* 0x110C */
typedef union tagScDispatchIntclr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    intclr                : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_INTCLR_U;

/* Define the union U_SC_DISPATCH_ERRSTAT_U */
/* dispatch的ERR状态寄存器 */
/* 0x1110 */
typedef union tagScDispatchErrstat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
        unsigned int    err_addr              : 17  ; /* [21..5]  */
        unsigned int    err_opcode            : 5   ; /* [4..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_ERRSTAT_U;

/* Define the union U_SC_DISPATCH_REMAP_CTRL_U */
/* subsys的启动Remap寄存器 */
/* 0x1200 */
typedef union tagScDispatchRemapCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sys_remap_vld         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_REMAP_CTRL_U;

/* Define the union U_SC_TSENSOR_CTRL0_U */
/* TSENSOR控制寄存器0 */
/* 0x20D0 */
typedef union tagScTsensorCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    tsensor_ct_sel        : 2   ; /* [9..8]  */
        unsigned int    reserved_1            : 7   ; /* [7..1]  */
        unsigned int    tsensor_temp_en       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_CTRL0_U;

/* Define the union U_SC_TSENSOR_CTRL1_U */
/* TSENSOR控制寄存器1 */
/* 0x20D4 */
typedef union tagScTsensorCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    tsensor_test          : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_CTRL1_U;

/* Define the union U_SC_BOOTROM_CTRL_U */
/* BOOTROM控制寄存器 */
/* 0x20F0 */
typedef union tagScBootromCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
        unsigned int    bootrom_trb           : 2   ; /* [5..4]  */
        unsigned int    bootrom_ptsel         : 2   ; /* [3..2]  */
        unsigned int    bootrom_rtsel         : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BOOTROM_CTRL_U;

/* Define the union U_SC_FTE_MUX_CTRL_U */
/* FTE复用选择寄存器 */
/* 0x2200 */
typedef union tagScFteMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mux_sel_fte           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_FTE_MUX_CTRL_U;

/* Define the union U_SC_RSA_ADDR_H_U */
/* RSA高20位地址补充寄存器 */
/* 0x2E00 */
typedef union tagScRsaAddrH
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    rsa_addr_h            : 20  ; /* [19..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_ADDR_H_U;

/* Define the union U_SC_SMMU_MEM_CTRL0_U */
/* smmu mem控制寄存器0 */
/* 0x3000 */
typedef union tagScSmmuMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_smmu         : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SMMU_MEM_CTRL0_U;

/* Define the union U_SC_SMMU_MEM_CTRL1_U */
/* smmu mem控制寄存器1 */
/* 0x3004 */
typedef union tagScSmmuMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    tsel_hc_smmu          : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SMMU_MEM_CTRL1_U;

/* Define the union U_SC_SMMU_MEM_CTRL2_U */
/* smmu mem控制寄存器2 */
/* 0x3008 */
typedef union tagScSmmuMemCtrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    test_hc_smmu          : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SMMU_MEM_CTRL2_U;

/* Define the union U_SC_RSA_MEM_CTRL_U */
/* rsa mem控制寄存器 */
/* 0x3030 */
typedef union tagScRsaMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_rsa          : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_MEM_CTRL_U;

/* Define the union U_SC_HLLC_MEM_CTRL_U */
/* hllc mem控制寄存器 */
/* 0x3040 */
typedef union tagScHllcMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    ctrl_rft_hllc         : 10  ; /* [9..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_MEM_CTRL_U;

/* Define the union U_SC_PA_MEM_CTRL0_U */
/* pa mem控制寄存器0 */
/* 0x3050 */
typedef union tagScPaMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_pa           : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_MEM_CTRL0_U;

/* Define the union U_SC_PA_MEM_CTRL1_U */
/* pa mem控制寄存器1 */
/* 0x3054 */
typedef union tagScPaMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    ctrl_rft_pa           : 10  ; /* [9..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_MEM_CTRL1_U;

/* Define the union U_SC_RDE_MEM_CTRL_U */
/* rde mem控制寄存器 */
/* 0x3060 */
typedef union tagScRdeMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_rde          : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_MEM_CTRL_U;

/* Define the union U_SC_SEC_MEM_CTRL_U */
/* sec mem控制寄存器 */
/* 0x3070 */
typedef union tagScSecMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_sec          : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_MEM_CTRL_U;

/* Define the union U_SC_PLL_LOCK_ST_U */
/* PLL锁定状态寄存器 */
/* 0x5000 */
typedef union tagScPllLockSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    pll4_lock             : 1   ; /* [4]  */
        unsigned int    pll3_lock             : 1   ; /* [3]  */
        unsigned int    pll2_lock             : 1   ; /* [2]  */
        unsigned int    pll1_lock             : 1   ; /* [1]  */
        unsigned int    pll0_lock             : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_LOCK_ST_U;

/* Define the union U_SC_ITS_CLK_ST_U */
/* ITS时钟状态寄存器 */
/* 0x530C */
typedef union tagScItsClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_its_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_CLK_ST_U;

/* Define the union U_SC_PA_CLK_ST_U */
/* PA时钟状态寄存器 */
/* 0x532C */
typedef union tagScPaClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_pa_st             : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_CLK_ST_U;

/* Define the union U_SC_HLLC_CLK_ST_U */
/* HLLC时钟状态寄存器 */
/* 0x5330 */
typedef union tagScHllcClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_hllc_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_CLK_ST_U;

/* Define the union U_SC_REF_CLK_ST_U */
/* ref时钟状态寄存器 */
/* 0x5338 */
typedef union tagScRefClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_ref_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REF_CLK_ST_U;

/* Define the union U_SC_GPIO_DB_CLK_ST_U */
/* gpio_db时钟状态寄存器 */
/* 0x533C */
typedef union tagScGpioDbClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_gpio_db_st        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_DB_CLK_ST_U;

/* Define the union U_SC_DJTAG_CLK_ST_U */
/* djtag时钟状态寄存器 */
/* 0x5340 */
typedef union tagScDjtagClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_djtag_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CLK_ST_U;

/* Define the union U_SC_HLLC_RXTX_CLK_ST_U */
/* hllc rxtx时钟状态寄存器 */
/* 0x5348 */
typedef union tagScHllcRxtxClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    clk_hllc_hilink_rx7_st : 1   ; /* [15]  */
        unsigned int    clk_hllc_hilink_rx6_st : 1   ; /* [14]  */
        unsigned int    clk_hllc_hilink_rx5_st : 1   ; /* [13]  */
        unsigned int    clk_hllc_hilink_rx4_st : 1   ; /* [12]  */
        unsigned int    clk_hllc_hilink_rx3_st : 1   ; /* [11]  */
        unsigned int    clk_hllc_hilink_rx2_st : 1   ; /* [10]  */
        unsigned int    clk_hllc_hilink_rx1_st : 1   ; /* [9]  */
        unsigned int    clk_hllc_hilink_rx0_st : 1   ; /* [8]  */
        unsigned int    clk_hllc_hilink_tx7_st : 1   ; /* [7]  */
        unsigned int    clk_hllc_hilink_tx6_st : 1   ; /* [6]  */
        unsigned int    clk_hllc_hilink_tx5_st : 1   ; /* [5]  */
        unsigned int    clk_hllc_hilink_tx4_st : 1   ; /* [4]  */
        unsigned int    clk_hllc_hilink_tx3_st : 1   ; /* [3]  */
        unsigned int    clk_hllc_hilink_tx2_st : 1   ; /* [2]  */
        unsigned int    clk_hllc_hilink_tx1_st : 1   ; /* [1]  */
        unsigned int    clk_hllc_hilink_tx0_st : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_CLK_ST_U;

/* Define the union U_SC_RSA_CLK_ST_U */
/* RSA时钟状态寄存器 */
/* 0x5350 */
typedef union tagScRsaClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rsa_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_CLK_ST_U;

/* Define the union U_SC_RDE_CLK_ST_U */
/* RDE时钟状态寄存器 */
/* 0x5354 */
typedef union tagScRdeClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rde_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_CLK_ST_U;

/* Define the union U_SC_RNG_CLK_ST_U */
/* RNG时钟状态寄存器 */
/* 0x5358 */
typedef union tagScRngClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rng_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_CLK_ST_U;

/* Define the union U_SC_SEC_CLK_ST_U */
/* SEC时钟状态寄存器 */
/* 0x535C */
typedef union tagScSecClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    clk_sec_trace_dbg_st  : 1   ; /* [2]  */
        unsigned int    clk_sec_ahb_st        : 1   ; /* [1]  */
        unsigned int    clk_sec_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_CLK_ST_U;

/* Define the union U_SC_ITS_RESET_ST_U */
/* ITS复位状态寄存器 */
/* 0x5A0C */
typedef union tagScItsResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    its_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_RESET_ST_U;

/* Define the union U_SC_HILINK_PCS_RESET_ST_U */
/* HILINK_PCS复位状态寄存器 */
/* 0x5A2C */
typedef union tagScHilinkPcsResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hllc_hilink_pcs_lane7_srst_st : 1   ; /* [7]  */
        unsigned int    hllc_hilink_pcs_lane6_srst_st : 1   ; /* [6]  */
        unsigned int    hllc_hilink_pcs_lane5_srst_st : 1   ; /* [5]  */
        unsigned int    hllc_hilink_pcs_lane4_srst_st : 1   ; /* [4]  */
        unsigned int    hllc_hilink_pcs_lane3_srst_st : 1   ; /* [3]  */
        unsigned int    hllc_hilink_pcs_lane2_srst_st : 1   ; /* [2]  */
        unsigned int    hllc_hilink_pcs_lane1_srst_st : 1   ; /* [1]  */
        unsigned int    hllc_hilink_pcs_lane0_srst_st : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK_PCS_RESET_ST_U;

/* Define the union U_SC_HLLC_RXTX_RESET_ST_U */
/* hllc rxtx复位状态寄存器 */
/* 0x5A30 */
typedef union tagScHllcRxtxResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    hllc_tx7_srst_st      : 1   ; /* [15]  */
        unsigned int    hllc_tx6_srst_st      : 1   ; /* [14]  */
        unsigned int    hllc_tx5_srst_st      : 1   ; /* [13]  */
        unsigned int    hllc_tx4_srst_st      : 1   ; /* [12]  */
        unsigned int    hllc_tx3_srst_st      : 1   ; /* [11]  */
        unsigned int    hllc_tx2_srst_st      : 1   ; /* [10]  */
        unsigned int    hllc_tx1_srst_st      : 1   ; /* [9]  */
        unsigned int    hllc_tx0_srst_st      : 1   ; /* [8]  */
        unsigned int    hllc_rx7_srst_st      : 1   ; /* [7]  */
        unsigned int    hllc_rx6_srst_st      : 1   ; /* [6]  */
        unsigned int    hllc_rx5_srst_st      : 1   ; /* [5]  */
        unsigned int    hllc_rx4_srst_st      : 1   ; /* [4]  */
        unsigned int    hllc_rx3_srst_st      : 1   ; /* [3]  */
        unsigned int    hllc_rx2_srst_st      : 1   ; /* [2]  */
        unsigned int    hllc_rx1_srst_st      : 1   ; /* [1]  */
        unsigned int    hllc_rx0_srst_st      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_RESET_ST_U;

/* Define the union U_SC_PA_RESET_ST_U */
/* PA复位状态寄存器 */
/* 0x5A3C */
typedef union tagScPaResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pa_srst_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_RESET_ST_U;

/* Define the union U_SC_HLLC_RESET_ST_U */
/* HLLC复位状态寄存器 */
/* 0x5A40 */
typedef union tagScHllcResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hllc_srst_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RESET_ST_U;

/* Define the union U_SC_DJTAG_RESET_ST_U */
/* DJTAG复位状态寄存器 */
/* 0x5A48 */
typedef union tagScDjtagResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    djtag_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_RESET_ST_U;

/* Define the union U_SC_RSA_RESET_ST_U */
/* RSA复位状态寄存器 */
/* 0x5A4C */
typedef union tagScRsaResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rsa_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_RESET_ST_U;

/* Define the union U_SC_RDE_RESET_ST_U */
/* RDE复位状态寄存器 */
/* 0x5A50 */
typedef union tagScRdeResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rde_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_RESET_ST_U;

/* Define the union U_SC_SEC_RESET_ST_U */
/* SEC复位状态寄存器 */
/* 0x5A54 */
typedef union tagScSecResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sec_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_RESET_ST_U;

/* Define the union U_SC_RNG_RESET_ST_U */
/* RNG复位状态寄存器 */
/* 0x5A58 */
typedef union tagScRngResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rng_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_RESET_ST_U;

/* Define the union U_SC_SEC_BUILD_RESET_ST_U */
/* SEC BUILD复位状态寄存器 */
/* 0x5A5C */
typedef union tagScSecBuildResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
        unsigned int    sec_ahb_srst_st       : 1   ; /* [20]  */
        unsigned int    sec_build_ahb_srst_st7 : 1   ; /* [19]  */
        unsigned int    sec_build_srst_st7    : 1   ; /* [18]  */
        unsigned int    sec_build_ahb_srst_st6 : 1   ; /* [17]  */
        unsigned int    sec_build_srst_st6    : 1   ; /* [16]  */
        unsigned int    sec_build_ahb_srst_st5 : 1   ; /* [15]  */
        unsigned int    sec_build_srst_st5    : 1   ; /* [14]  */
        unsigned int    sec_build_ahb_srst_st4 : 1   ; /* [13]  */
        unsigned int    sec_build_srst_st4    : 1   ; /* [12]  */
        unsigned int    sec_cluster1_ahb_srst_st : 1   ; /* [11]  */
        unsigned int    sec_cluster1_srst_st  : 1   ; /* [10]  */
        unsigned int    sec_build_ahb_srst_st3 : 1   ; /* [9]  */
        unsigned int    sec_build_srst_st3    : 1   ; /* [8]  */
        unsigned int    sec_build_ahb_srst_st2 : 1   ; /* [7]  */
        unsigned int    sec_build_srst_st2    : 1   ; /* [6]  */
        unsigned int    sec_build_ahb_srst_st1 : 1   ; /* [5]  */
        unsigned int    sec_build_srst_st1    : 1   ; /* [4]  */
        unsigned int    sec_build_ahb_srst_st0 : 1   ; /* [3]  */
        unsigned int    sec_build_srst_st0    : 1   ; /* [2]  */
        unsigned int    sec_cluster0_ahb_srst_st : 1   ; /* [1]  */
        unsigned int    sec_cluster0_srst_st  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_BUILD_RESET_ST_U;

/* Define the union U_SC_PCS_LOCAL_RESET_ST_U */
/* PCS LOCAL软复位请求状态寄存器 */
/* 0x5A60 */
typedef union tagScPcsLocalResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcs_local_srst_st     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_ST_U;

/* Define the union U_SC_TSENSOR_ST_U */
/* TSENSOR状态寄存器 */
/* 0x6040 */
typedef union tagScTsensorSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
        unsigned int    data_ready_tsensor    : 1   ; /* [12]  */
        unsigned int    reserved_1            : 4   ; /* [11..8]  */
        unsigned int    temp_out_tsensor      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_ST_U;

/* Define the union U_SC_DJTAG_INT_U */
/* DJTAG Master内状态机超时中断源寄存器 */
/* 0x6600 */
typedef union tagScDjtagInt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    djtag_sta_timeout     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_INT_U;

/* Define the union U_SC_DBG_AUTH_CTRL_U */
/* 安全等级授权控制寄存器。
（注意：1，此寄存器空间必须是安全访问才能成功；2，默认均是打开调试模式，有漏洞，在正常工作时请初始化关闭调试功能） */
/* 0x6700 */
typedef union tagScDbgAuthCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    rsa_secure_cfg        : 1   ; /* [1]  */
        unsigned int    rsa_djtag_sec_acc_en  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_AUTH_CTRL_U;

/* Define the union U_SC_DJTAG_MSTR_EN_U */
/* SC_DJTAG_MSTR_EN为DJTAG Master使能控制 */
/* 0x6800 */
typedef union tagScDjtagMstrEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    djtag_mstr_en         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_EN_U;

/* Define the union U_SC_DJTAG_MSTR_START_EN_U */
/* SC_DJTAG_MSTR_START_EN为DJTAG Master访问chain链的start使能 */
/* 0x6804 */
typedef union tagScDjtagMstrStartEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    djtag_mstr_start_en   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_START_EN_U;

/* Define the union U_SC_DJTAG_SEC_ACC_EN_U */
/* DJTAG访问各模块内安全世界寄存器的安全访问控制使能。（注意：1，此寄存器空间必须是安全访问才能成功。） */
/* 0x6808 */
typedef union tagScDjtagSecAccEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    djtag_sec_acc_en      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_SEC_ACC_EN_U;

/* Define the union U_SC_DJTAG_DEBUG_MODULE_SEL_U */
/* SC_DJTAG_DEBUG_MODULE_SEL为DJTAG Master配置访问模块寄存器 */
/* 0x680C */
typedef union tagScDjtagDebugModuleSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
        unsigned int    debug_module_sel      : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_DEBUG_MODULE_SEL_U;

/* Define the union U_SC_DJTAG_MSTR_WR_U */
/* SC_DJTAG_MSTR_WR为DJTAG Master的读写控制信号 */
/* 0x6810 */
typedef union tagScDjtagMstrWr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    djtag_mstr_wr         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_WR_U;

/* Define the union U_SC_DJTAG_CHAIN_UNIT_CFG_EN_U */
/* SC_DJTAG_CHAIN_UNIT_CFG_EN为DJTAG Master的chain链上单元的访问使能 */
/* 0x6814 */
typedef union tagScDjtagChainUnitCfgEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    chain_unit_cfg_en     : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CHAIN_UNIT_CFG_EN_U;

/* Define the union U_SC_DJTAG_MSTR_ADDR_U */
/* SC_DJTAG_MSTR_ADDR为DJTAG Master配置访问模块的偏移地址寄存器 */
/* 0x6818 */
typedef union tagScDjtagMstrAddr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    djtag_mstr_addr       : 31  ; /* [30..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_ADDR_U;

/* Define the union U_SC_TDRE_OP_ADDR_U */
/* SC_TDRE_OP_ADDR为DJTAG Master配置TDRE模块的偏移地址寄存器和操作类型寄存器 */
/* 0x6824 */
typedef union tagScTdreOpAddr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
        unsigned int    tdre_op_addr          : 18  ; /* [17..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TDRE_OP_ADDR_U;

/* Define the union U_SC_TDRE_REPAIR_EN_U */
/* SC_TDRE_REPAIR_EN为DJTAG Master的TDRE REPAIR使能 */
/* 0x682C */
typedef union tagScTdreRepairEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    tdre_repair_en        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TDRE_REPAIR_EN_U;

/* Define the union U_SC_ECO_RSV0_U */
/* ECO 寄存器0 */
/* 0x8000 */
typedef union tagScEcoRsv0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    eco_rsv0              : 31  ; /* [31..1]  */
        unsigned int    djtag_nor_cfg_en      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ECO_RSV0_U;

/* Define the union U_SC_DJTAG_OP_ST_U */
/* DJTAG访问状态寄存器 */
/* 0xE828 */
typedef union tagScDjtagOpSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
        unsigned int    rdata_changed         : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [15..10]  */
        unsigned int    debug_bus_en          : 1   ; /* [9]  */
        unsigned int    djtag_op_done         : 1   ; /* [8]  */
        unsigned int    unit_conflict         : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_OP_ST_U;

#else
/* Define the union U_SC_PLLFCTRL0_U */
/* PLL0控制寄存器0。 */
/* 0x0 */
typedef union tagScPllfctrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll0_refdiv           : 6   ; /* [5..0]  */
        unsigned int    pll0_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll0_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll0_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll0_bypass           : 1   ; /* [24]  */
        unsigned int    pll0_pd               : 1   ; /* [25]  */
        unsigned int    pll0_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll0_dacpd            : 1   ; /* [27]  */
        unsigned int    pll0_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll0_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll0_foutvcopd        : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL0_U;

/* Define the union U_SC_PLLFCTRL1_U */
/* PLL0控制寄存器1。 */
/* 0x4 */
typedef union tagScPllfctrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll0_frac             : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL1_U;

/* Define the union U_SC_PLLFCTRL2_U */
/* PLL1控制寄存器0。 */
/* 0x8 */
typedef union tagScPllfctrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll1_refdiv           : 6   ; /* [5..0]  */
        unsigned int    pll1_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll1_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll1_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll1_bypass           : 1   ; /* [24]  */
        unsigned int    pll1_pd               : 1   ; /* [25]  */
        unsigned int    pll1_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll1_dacpd            : 1   ; /* [27]  */
        unsigned int    pll1_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll1_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll1_foutvcopd        : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL2_U;

/* Define the union U_SC_PLLFCTRL3_U */
/* PLL1控制寄存器1。 */
/* 0xC */
typedef union tagScPllfctrl3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll1_frac             : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL3_U;

/* Define the union U_SC_PLLFCTRL4_U */
/* PLL2控制寄存器0。 */
/* 0x10 */
typedef union tagScPllfctrl4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll2_refdiv           : 6   ; /* [5..0]  */
        unsigned int    pll2_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll2_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll2_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll2_bypass           : 1   ; /* [24]  */
        unsigned int    pll2_pd               : 1   ; /* [25]  */
        unsigned int    pll2_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll2_dacpd            : 1   ; /* [27]  */
        unsigned int    pll2_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll2_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll2_foutvcopd        : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL4_U;

/* Define the union U_SC_PLLFCTRL5_U */
/* PLL2控制寄存器1。 */
/* 0x14 */
typedef union tagScPllfctrl5
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll2_frac             : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL5_U;

/* Define the union U_SC_PLLFCTRL6_U */
/* PLL3控制寄存器0。 */
/* 0x18 */
typedef union tagScPllfctrl6
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll3_refdiv           : 6   ; /* [5..0]  */
        unsigned int    pll3_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll3_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll3_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll3_bypass           : 1   ; /* [24]  */
        unsigned int    pll3_pd               : 1   ; /* [25]  */
        unsigned int    pll3_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll3_dacpd            : 1   ; /* [27]  */
        unsigned int    pll3_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll3_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll3_foutvcopd        : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL6_U;

/* Define the union U_SC_PLLFCTRL7_U */
/* PLL3控制寄存器1。 */
/* 0x1C */
typedef union tagScPllfctrl7
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll3_frac             : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL7_U;

/* Define the union U_SC_PLL_CLK_BYPASS0_U */
/* PLL相关工作时钟选择。 */
/* 0x20 */
typedef union tagScPllClkBypass0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_1            : 16  ; /* [15..0]  */
        unsigned int    pll0_peri_mode        : 1   ; /* [16]  */
        unsigned int    pll1_peri_mode        : 1   ; /* [17]  */
        unsigned int    pll2_peri_mode        : 1   ; /* [18]  */
        unsigned int    pll3_peri_mode        : 1   ; /* [19]  */
        unsigned int    pll4_peri_mode        : 1   ; /* [20]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_CLK_BYPASS0_U;

/* Define the union U_SC_PLLFCTRL8_U */
/* pll4控制寄存器0。 */
/* 0x24 */
typedef union tagScPllfctrl8
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll4_refdiv           : 6   ; /* [5..0]  */
        unsigned int    pll4_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll4_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll4_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll4_bypass           : 1   ; /* [24]  */
        unsigned int    pll4_pd               : 1   ; /* [25]  */
        unsigned int    pll4_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll4_dacpd            : 1   ; /* [27]  */
        unsigned int    pll4_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll4_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll4_foutvcopd        : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL8_U;

/* Define the union U_SC_PLLFCTRL9_U */
/* pll4控制寄存器1。 */
/* 0x28 */
typedef union tagScPllfctrl9
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll4_frac             : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL9_U;

/* Define the union U_SC_PLL_CLK_BYPASS1_U */
/* PLL相关工作时钟选择。 */
/* 0x2C */
typedef union tagScPllClkBypass1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll0_bypass_external_n : 1   ; /* [0]  */
        unsigned int    pll1_bypass_external_n : 1   ; /* [1]  */
        unsigned int    pll2_bypass_external_n : 1   ; /* [2]  */
        unsigned int    pll3_bypass_external_n : 1   ; /* [3]  */
        unsigned int    pll4_bypass_external_n : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_CLK_BYPASS1_U;

/* Define the union U_SC_GPIO_DB_CTRL_U */
/* 配置gpip db时钟选择 */
/* 0x100 */
typedef union tagScGpioDbCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gpio_db_clk_sel       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_DB_CTRL_U;

/* Define the union U_SC_SFC_CLK_CTRL_U */
/* 配置SFC时钟选择 */
/* 0x104 */
typedef union tagScSfcClkCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfc_clk_sel           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_CLK_CTRL_U;

/* Define the union U_SC_ALL_SCAN_CTRL_U */
/* ALL SCAN控制寄存器 */
/* 0x150 */
typedef union tagScAllScanCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    all_scan_sys_int      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ALL_SCAN_CTRL_U;

/* Define the union U_SC_ITS_CLK_EN_U */
/* ITS时钟使能寄存器 */
/* 0x318 */
typedef union tagScItsClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_its_enb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_CLK_EN_U;

/* Define the union U_SC_ITS_CLK_DIS_U */
/* ITS时钟禁止寄存器 */
/* 0x31C */
typedef union tagScItsClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_its_dsb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_CLK_DIS_U;

/* Define the union U_SC_PA_CLK_EN_U */
/* PA时钟使能寄存器 */
/* 0x358 */
typedef union tagScPaClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pa_enb            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_CLK_EN_U;

/* Define the union U_SC_PA_CLK_DIS_U */
/* PA时钟禁止寄存器 */
/* 0x35C */
typedef union tagScPaClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pa_dsb            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_CLK_DIS_U;

/* Define the union U_SC_HLLC_CLK_EN_U */
/* HLLC时钟使能寄存器 */
/* 0x360 */
typedef union tagScHllcClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_hllc_enb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_CLK_EN_U;

/* Define the union U_SC_HLLC_CLK_DIS_U */
/* HLLC时钟禁止寄存器 */
/* 0x364 */
typedef union tagScHllcClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_hllc_dsb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_CLK_DIS_U;

/* Define the union U_SC_REF_CLK_EN_U */
/* ref时钟使能寄存器 */
/* 0x370 */
typedef union tagScRefClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_ref_enb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REF_CLK_EN_U;

/* Define the union U_SC_REF_CLK_DIS_U */
/* ref时钟禁止寄存器 */
/* 0x374 */
typedef union tagScRefClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_ref_dsb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REF_CLK_DIS_U;

/* Define the union U_SC_GPIO_DB_CLK_EN_U */
/* gpio_db时钟使能寄存器 */
/* 0x378 */
typedef union tagScGpioDbClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gpio_db_enb       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_DB_CLK_EN_U;

/* Define the union U_SC_GPIO_DB_CLK_DIS_U */
/* gpio_db时钟禁止寄存器 */
/* 0x37C */
typedef union tagScGpioDbClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gpio_db_dsb       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_DB_CLK_DIS_U;

/* Define the union U_SC_DJTAG_CLK_EN_U */
/* djtag时钟使能寄存器 */
/* 0x380 */
typedef union tagScDjtagClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_djtag_enb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CLK_EN_U;

/* Define the union U_SC_DJTAG_CLK_DIS_U */
/* djtag时钟禁止寄存器 */
/* 0x384 */
typedef union tagScDjtagClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_djtag_dsb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CLK_DIS_U;

/* Define the union U_SC_HLLC_RXTX_CLK_EN_U */
/* hllc rxtx时钟使能寄存器 */
/* 0x390 */
typedef union tagScHllcRxtxClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_hllc_hilink_tx0_enb : 1   ; /* [0]  */
        unsigned int    clk_hllc_hilink_tx1_enb : 1   ; /* [1]  */
        unsigned int    clk_hllc_hilink_tx2_enb : 1   ; /* [2]  */
        unsigned int    clk_hllc_hilink_tx3_enb : 1   ; /* [3]  */
        unsigned int    clk_hllc_hilink_tx4_enb : 1   ; /* [4]  */
        unsigned int    clk_hllc_hilink_tx5_enb : 1   ; /* [5]  */
        unsigned int    clk_hllc_hilink_tx6_enb : 1   ; /* [6]  */
        unsigned int    clk_hllc_hilink_tx7_enb : 1   ; /* [7]  */
        unsigned int    clk_hllc_hilink_rx0_enb : 1   ; /* [8]  */
        unsigned int    clk_hllc_hilink_rx1_enb : 1   ; /* [9]  */
        unsigned int    clk_hllc_hilink_rx2_enb : 1   ; /* [10]  */
        unsigned int    clk_hllc_hilink_rx3_enb : 1   ; /* [11]  */
        unsigned int    clk_hllc_hilink_rx4_enb : 1   ; /* [12]  */
        unsigned int    clk_hllc_hilink_rx5_enb : 1   ; /* [13]  */
        unsigned int    clk_hllc_hilink_rx6_enb : 1   ; /* [14]  */
        unsigned int    clk_hllc_hilink_rx7_enb : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_CLK_EN_U;

/* Define the union U_SC_HLLC_RXTX_CLK_DIS_U */
/* hllc rxtx时钟禁止寄存器 */
/* 0x394 */
typedef union tagScHllcRxtxClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_hllc_hilink_tx0_dsb : 1   ; /* [0]  */
        unsigned int    clk_hllc_hilink_tx1_dsb : 1   ; /* [1]  */
        unsigned int    clk_hllc_hilink_tx2_dsb : 1   ; /* [2]  */
        unsigned int    clk_hllc_hilink_tx3_dsb : 1   ; /* [3]  */
        unsigned int    clk_hllc_hilink_tx4_dsb : 1   ; /* [4]  */
        unsigned int    clk_hllc_hilink_tx5_dsb : 1   ; /* [5]  */
        unsigned int    clk_hllc_hilink_tx6_dsb : 1   ; /* [6]  */
        unsigned int    clk_hllc_hilink_tx7_dsb : 1   ; /* [7]  */
        unsigned int    clk_hllc_hilink_rx0_dsb : 1   ; /* [8]  */
        unsigned int    clk_hllc_hilink_rx1_dsb : 1   ; /* [9]  */
        unsigned int    clk_hllc_hilink_rx2_dsb : 1   ; /* [10]  */
        unsigned int    clk_hllc_hilink_rx3_dsb : 1   ; /* [11]  */
        unsigned int    clk_hllc_hilink_rx4_dsb : 1   ; /* [12]  */
        unsigned int    clk_hllc_hilink_rx5_dsb : 1   ; /* [13]  */
        unsigned int    clk_hllc_hilink_rx6_dsb : 1   ; /* [14]  */
        unsigned int    clk_hllc_hilink_rx7_dsb : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_CLK_DIS_U;

/* Define the union U_SC_RSA_CLK_EN_U */
/* RSA时钟使能寄存器 */
/* 0x3A0 */
typedef union tagScRsaClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rsa_enb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_CLK_EN_U;

/* Define the union U_SC_RSA_CLK_DIS_U */
/* RSA时钟禁止寄存器 */
/* 0x3A4 */
typedef union tagScRsaClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rsa_dsb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_CLK_DIS_U;

/* Define the union U_SC_RDE_CLK_EN_U */
/* RDE时钟使能寄存器 */
/* 0x3A8 */
typedef union tagScRdeClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rde_enb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_CLK_EN_U;

/* Define the union U_SC_RDE_CLK_DIS_U */
/* RDE时钟禁止寄存器 */
/* 0x3AC */
typedef union tagScRdeClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rde_dsb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_CLK_DIS_U;

/* Define the union U_SC_RNG_CLK_EN_U */
/* RNG时钟使能寄存器 */
/* 0x3B0 */
typedef union tagScRngClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rng_enb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_CLK_EN_U;

/* Define the union U_SC_RNG_CLK_DIS_U */
/* RNG时钟禁止寄存器 */
/* 0x3B4 */
typedef union tagScRngClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rng_dsb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_CLK_DIS_U;

/* Define the union U_SC_SEC_CLK_EN_U */
/* SEC时钟使能寄存器 */
/* 0x3B8 */
typedef union tagScSecClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sec_enb           : 1   ; /* [0]  */
        unsigned int    clk_sec_ahb_enb       : 1   ; /* [1]  */
        unsigned int    clk_sec_trace_dbg_enb : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_CLK_EN_U;

/* Define the union U_SC_SEC_CLK_DIS_U */
/* SEC时钟禁止寄存器 */
/* 0x3BC */
typedef union tagScSecClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sec_dsb           : 1   ; /* [0]  */
        unsigned int    clk_sec_ahb_dsb       : 1   ; /* [1]  */
        unsigned int    clk_sec_trace_dbg_dsb : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_CLK_DIS_U;

/* Define the union U_SC_ITS_RESET_REQ_U */
/* ITS软复位请求控制寄存器 */
/* 0xA18 */
typedef union tagScItsResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    its_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_RESET_REQ_U;

/* Define the union U_SC_ITS_RESET_DREQ_U */
/* ITS软复位去请求控制寄存器 */
/* 0xA1C */
typedef union tagScItsResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    its_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_RESET_DREQ_U;

/* Define the union U_SC_HILINK_PCS_RESET_REQ_U */
/* HILINK PCS软复位请求控制寄存器 */
/* 0xA58 */
typedef union tagScHilinkPcsResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hllc_hilink_pcs_lane0_srst_req : 1   ; /* [0]  */
        unsigned int    hllc_hilink_pcs_lane1_srst_req : 1   ; /* [1]  */
        unsigned int    hllc_hilink_pcs_lane2_srst_req : 1   ; /* [2]  */
        unsigned int    hllc_hilink_pcs_lane3_srst_req : 1   ; /* [3]  */
        unsigned int    hllc_hilink_pcs_lane4_srst_req : 1   ; /* [4]  */
        unsigned int    hllc_hilink_pcs_lane5_srst_req : 1   ; /* [5]  */
        unsigned int    hllc_hilink_pcs_lane6_srst_req : 1   ; /* [6]  */
        unsigned int    hllc_hilink_pcs_lane7_srst_req : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK_PCS_RESET_REQ_U;

/* Define the union U_SC_HILINK_PCS_RESET_DREQ_U */
/* HILINK PCS软复位去请求控制寄存器 */
/* 0xA5C */
typedef union tagScHilinkPcsResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hllc_hilink_pcs_lane0_srst_dreq : 1   ; /* [0]  */
        unsigned int    hllc_hilink_pcs_lane1_srst_dreq : 1   ; /* [1]  */
        unsigned int    hllc_hilink_pcs_lane2_srst_dreq : 1   ; /* [2]  */
        unsigned int    hllc_hilink_pcs_lane3_srst_dreq : 1   ; /* [3]  */
        unsigned int    hllc_hilink_pcs_lane4_srst_dreq : 1   ; /* [4]  */
        unsigned int    hllc_hilink_pcs_lane5_srst_dreq : 1   ; /* [5]  */
        unsigned int    hllc_hilink_pcs_lane6_srst_dreq : 1   ; /* [6]  */
        unsigned int    hllc_hilink_pcs_lane7_srst_dreq : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK_PCS_RESET_DREQ_U;

/* Define the union U_SC_HLLC_RXTX_RESET_REQ_U */
/* hllc rxtx软复位请求控制寄存器 */
/* 0xA60 */
typedef union tagScHllcRxtxResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hllc_rx0_srst_req     : 1   ; /* [0]  */
        unsigned int    hllc_rx1_srst_req     : 1   ; /* [1]  */
        unsigned int    hllc_rx2_srst_req     : 1   ; /* [2]  */
        unsigned int    hllc_rx3_srst_req     : 1   ; /* [3]  */
        unsigned int    hllc_rx4_srst_req     : 1   ; /* [4]  */
        unsigned int    hllc_rx5_srst_req     : 1   ; /* [5]  */
        unsigned int    hllc_rx6_srst_req     : 1   ; /* [6]  */
        unsigned int    hllc_rx7_srst_req     : 1   ; /* [7]  */
        unsigned int    hllc_tx0_srst_req     : 1   ; /* [8]  */
        unsigned int    hllc_tx1_srst_req     : 1   ; /* [9]  */
        unsigned int    hllc_tx2_srst_req     : 1   ; /* [10]  */
        unsigned int    hllc_tx3_srst_req     : 1   ; /* [11]  */
        unsigned int    hllc_tx4_srst_req     : 1   ; /* [12]  */
        unsigned int    hllc_tx5_srst_req     : 1   ; /* [13]  */
        unsigned int    hllc_tx6_srst_req     : 1   ; /* [14]  */
        unsigned int    hllc_tx7_srst_req     : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_RESET_REQ_U;

/* Define the union U_SC_HLLC_RXTX_RESET_DREQ_U */
/* hllc rxtx软复位去请求控制寄存器 */
/* 0xA64 */
typedef union tagScHllcRxtxResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hllc_rx0_srst_dreq    : 1   ; /* [0]  */
        unsigned int    hllc_rx1_srst_dreq    : 1   ; /* [1]  */
        unsigned int    hllc_rx2_srst_dreq    : 1   ; /* [2]  */
        unsigned int    hllc_rx3_srst_dreq    : 1   ; /* [3]  */
        unsigned int    hllc_rx4_srst_dreq    : 1   ; /* [4]  */
        unsigned int    hllc_rx5_srst_dreq    : 1   ; /* [5]  */
        unsigned int    hllc_rx6_srst_dreq    : 1   ; /* [6]  */
        unsigned int    hllc_rx7_srst_dreq    : 1   ; /* [7]  */
        unsigned int    hllc_tx0_srst_dreq    : 1   ; /* [8]  */
        unsigned int    hllc_tx1_srst_dreq    : 1   ; /* [9]  */
        unsigned int    hllc_tx2_srst_dreq    : 1   ; /* [10]  */
        unsigned int    hllc_tx3_srst_dreq    : 1   ; /* [11]  */
        unsigned int    hllc_tx4_srst_dreq    : 1   ; /* [12]  */
        unsigned int    hllc_tx5_srst_dreq    : 1   ; /* [13]  */
        unsigned int    hllc_tx6_srst_dreq    : 1   ; /* [14]  */
        unsigned int    hllc_tx7_srst_dreq    : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_RESET_DREQ_U;

/* Define the union U_SC_PA_RESET_REQ_U */
/* PA软复位请求控制寄存器 */
/* 0xA78 */
typedef union tagScPaResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pa_srst_req           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_RESET_REQ_U;

/* Define the union U_SC_PA_RESET_DREQ_U */
/* PA软复位去请求控制寄存器 */
/* 0xA7C */
typedef union tagScPaResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pa_srst_dreq          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_RESET_DREQ_U;

/* Define the union U_SC_HLLC_RESET_REQ_U */
/* HLLC软复位请求控制寄存器 */
/* 0xA80 */
typedef union tagScHllcResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hllc_srst_req         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RESET_REQ_U;

/* Define the union U_SC_HLLC_RESET_DREQ_U */
/* HLLC软复位去请求控制寄存器 */
/* 0xA84 */
typedef union tagScHllcResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hllc_srst_dreq        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RESET_DREQ_U;

/* Define the union U_SC_DJTAG_RESET_REQ_U */
/* DJTAG软复位请求控制寄存器 */
/* 0xA90 */
typedef union tagScDjtagResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_RESET_REQ_U;

/* Define the union U_SC_DJTAG_RESET_DREQ_U */
/* DJTAG软复位去请求控制寄存器 */
/* 0xA94 */
typedef union tagScDjtagResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_RESET_DREQ_U;

/* Define the union U_SC_RSA_RESET_REQ_U */
/* RSA软复位请求控制寄存器 */
/* 0xA98 */
typedef union tagScRsaResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rsa_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_RESET_REQ_U;

/* Define the union U_SC_RSA_RESET_DREQ_U */
/* RSA软复位去请求控制寄存器 */
/* 0xA9C */
typedef union tagScRsaResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rsa_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_RESET_DREQ_U;

/* Define the union U_SC_RDE_RESET_REQ_U */
/* RDE软复位请求控制寄存器 */
/* 0xAA0 */
typedef union tagScRdeResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rde_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_RESET_REQ_U;

/* Define the union U_SC_RDE_RESET_DREQ_U */
/* RDE软复位去请求控制寄存器 */
/* 0xAA4 */
typedef union tagScRdeResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rde_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_RESET_DREQ_U;

/* Define the union U_SC_SEC_RESET_REQ_U */
/* SEC软复位请求控制寄存器 */
/* 0xAA8 */
typedef union tagScSecResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sec_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_RESET_REQ_U;

/* Define the union U_SC_SEC_RESET_DREQ_U */
/* SEC软复位去请求控制寄存器 */
/* 0xAAC */
typedef union tagScSecResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sec_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_RESET_DREQ_U;

/* Define the union U_SC_RNG_RESET_REQ_U */
/* RNG软复位请求控制寄存器 */
/* 0xAB0 */
typedef union tagScRngResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rng_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_RESET_REQ_U;

/* Define the union U_SC_RNG_RESET_DREQ_U */
/* RNG软复位去请求控制寄存器 */
/* 0xAB4 */
typedef union tagScRngResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rng_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_RESET_DREQ_U;

/* Define the union U_SC_SEC_BUILD_RESET_REQ_U */
/* SEC BUILD软复位请求控制寄存器 */
/* 0xAB8 */
typedef union tagScSecBuildResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sec_cluster0_srst_req : 1   ; /* [0]  */
        unsigned int    sec_cluster0_ahb_srst_req : 1   ; /* [1]  */
        unsigned int    sec_build_srst_req0   : 1   ; /* [2]  */
        unsigned int    sec_build_ahb_srst_req0 : 1   ; /* [3]  */
        unsigned int    sec_build_srst_req1   : 1   ; /* [4]  */
        unsigned int    sec_build_ahb_srst_req1 : 1   ; /* [5]  */
        unsigned int    sec_build_srst_req2   : 1   ; /* [6]  */
        unsigned int    sec_build_ahb_srst_req2 : 1   ; /* [7]  */
        unsigned int    sec_build_srst_req3   : 1   ; /* [8]  */
        unsigned int    sec_build_ahb_srst_req3 : 1   ; /* [9]  */
        unsigned int    sec_cluster1_srst_req : 1   ; /* [10]  */
        unsigned int    sec_cluster1_ahb_srst_req : 1   ; /* [11]  */
        unsigned int    sec_build_srst_req4   : 1   ; /* [12]  */
        unsigned int    sec_build_ahb_srst_req4 : 1   ; /* [13]  */
        unsigned int    sec_build_srst_req5   : 1   ; /* [14]  */
        unsigned int    sec_build_ahb_srst_req5 : 1   ; /* [15]  */
        unsigned int    sec_build_srst_req6   : 1   ; /* [16]  */
        unsigned int    sec_build_ahb_srst_req6 : 1   ; /* [17]  */
        unsigned int    sec_build_srst_req7   : 1   ; /* [18]  */
        unsigned int    sec_build_ahb_srst_req7 : 1   ; /* [19]  */
        unsigned int    sec_ahb_srst_req      : 1   ; /* [20]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_BUILD_RESET_REQ_U;

/* Define the union U_SC_SEC_BUILD_RESET_DREQ_U */
/* SEC BUILD软复位去请求控制寄存器 */
/* 0xABC */
typedef union tagScSecBuildResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sec_cluster0_srst_dreq : 1   ; /* [0]  */
        unsigned int    sec_cluster0_ahb_srst_dreq : 1   ; /* [1]  */
        unsigned int    sec_build_srst_dreq0  : 1   ; /* [2]  */
        unsigned int    sec_build_ahb_srst_dreq0 : 1   ; /* [3]  */
        unsigned int    sec_build_srst_dreq1  : 1   ; /* [4]  */
        unsigned int    sec_build_ahb_srst_dreq1 : 1   ; /* [5]  */
        unsigned int    sec_build_srst_dreq2  : 1   ; /* [6]  */
        unsigned int    sec_build_ahb_srst_dreq2 : 1   ; /* [7]  */
        unsigned int    sec_build_srst_dreq3  : 1   ; /* [8]  */
        unsigned int    sec_build_ahb_srst_dreq3 : 1   ; /* [9]  */
        unsigned int    sec_cluster1_srst_dreq : 1   ; /* [10]  */
        unsigned int    sec_cluster1_ahb_srst_dreq : 1   ; /* [11]  */
        unsigned int    sec_build_srst_dreq4  : 1   ; /* [12]  */
        unsigned int    sec_build_ahb_srst_dreq4 : 1   ; /* [13]  */
        unsigned int    sec_build_srst_dreq5  : 1   ; /* [14]  */
        unsigned int    sec_build_ahb_srst_dreq5 : 1   ; /* [15]  */
        unsigned int    sec_build_srst_dreq6  : 1   ; /* [16]  */
        unsigned int    sec_build_ahb_srst_dreq6 : 1   ; /* [17]  */
        unsigned int    sec_build_srst_dreq7  : 1   ; /* [18]  */
        unsigned int    sec_build_ahb_srst_dreq7 : 1   ; /* [19]  */
        unsigned int    sec_ahb_srst_dreq     : 1   ; /* [20]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_BUILD_RESET_DREQ_U;

/* Define the union U_SC_PCS_LOCAL_RESET_REQ_U */
/* PCS LOCAL软复位请求控制寄存器 */
/* 0xAC0 */
typedef union tagScPcsLocalResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcs_local_srst_req    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_REQ_U;

/* Define the union U_SC_PCS_LOCAL_RESET_DREQ_U */
/* PCS LOCAL软复位去请求控制寄存器 */
/* 0xAC4 */
typedef union tagScPcsLocalResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcs_local_srst_dreq   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_DREQ_U;

/* Define the union U_SC_DISPATCH_DAW_EN_U */
/* dispatch daw en配置 */
/* 0x1000 */
typedef union tagScDispatchDawEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dispatch_daw_en       : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_EN_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY0_U */
/* dispatch daw配置阵列0 */
/* 0x1004 */
typedef union tagScDispatchDawArray0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array0_did        : 3   ; /* [2..0]  */
        unsigned int    daw_array0_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array0_sync       : 1   ; /* [8]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array0_addr       : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY0_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY1_U */
/* dispatch daw配置阵列1 */
/* 0x1008 */
typedef union tagScDispatchDawArray1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array1_did        : 3   ; /* [2..0]  */
        unsigned int    daw_array1_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array1_sync       : 1   ; /* [8]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array1_addr       : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY1_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY2_U */
/* dispatch daw配置阵列2 */
/* 0x100C */
typedef union tagScDispatchDawArray2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array2_did        : 3   ; /* [2..0]  */
        unsigned int    daw_array2_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array2_sync       : 1   ; /* [8]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array2_addr       : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY2_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY3_U */
/* dispatch daw配置阵列3 */
/* 0x1010 */
typedef union tagScDispatchDawArray3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array3_did        : 3   ; /* [2..0]  */
        unsigned int    daw_array3_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array3_sync       : 1   ; /* [8]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array3_addr       : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY3_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY4_U */
/* dispatch daw配置阵列4 */
/* 0x1014 */
typedef union tagScDispatchDawArray4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array4_did        : 3   ; /* [2..0]  */
        unsigned int    daw_array4_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array4_sync       : 1   ; /* [8]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array4_addr       : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY4_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY5_U */
/* dispatch daw配置阵列5 */
/* 0x1018 */
typedef union tagScDispatchDawArray5
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array5_did        : 3   ; /* [2..0]  */
        unsigned int    daw_array5_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array5_sync       : 1   ; /* [8]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array5_addr       : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY5_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY6_U */
/* dispatch daw配置阵列6 */
/* 0x101C */
typedef union tagScDispatchDawArray6
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array6_did        : 3   ; /* [2..0]  */
        unsigned int    daw_array6_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array6_sync       : 1   ; /* [8]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array6_addr       : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY6_U;

/* Define the union U_SC_DISPATCH_DAW_ARRAY7_U */
/* dispatch daw配置阵列7 */
/* 0x1020 */
typedef union tagScDispatchDawArray7
{
    /* Define the struct bits */
    struct
    {
        unsigned int    daw_array7_did        : 3   ; /* [2..0]  */
        unsigned int    daw_array7_size       : 5   ; /* [7..3]  */
        unsigned int    daw_array7_sync       : 1   ; /* [8]  */
        unsigned int    reserved_0            : 4   ; /* [12..9]  */
        unsigned int    daw_array7_addr       : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_ARRAY7_U;

/* Define the union U_SC_DISPATCH_RETRY_CONTROL_U */
/* dispatch retry控制寄存器 */
/* 0x1030 */
typedef union tagScDispatchRetryControl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    retry_num_limit       : 16  ; /* [15..0]  */
        unsigned int    retry_en              : 1   ; /* [16]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_RETRY_CONTROL_U;

/* Define the union U_SC_DISPATCH_INTMASK_U */
/* dispatch的中断屏蔽寄存器 */
/* 0x1100 */
typedef union tagScDispatchIntmask
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intmask               : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_INTMASK_U;

/* Define the union U_SC_DISPATCH_RAWINT_U */
/* dispatch的原始中断状态寄存器 */
/* 0x1104 */
typedef union tagScDispatchRawint
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rawint                : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_RAWINT_U;

/* Define the union U_SC_DISPATCH_INTSTAT_U */
/* dispatch的屏蔽后的中断状态寄存器 */
/* 0x1108 */
typedef union tagScDispatchIntstat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intsts                : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_INTSTAT_U;

/* Define the union U_SC_DISPATCH_INTCLR_U */
/* dispatch的中断清除寄存器 */
/* 0x110C */
typedef union tagScDispatchIntclr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intclr                : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_INTCLR_U;

/* Define the union U_SC_DISPATCH_ERRSTAT_U */
/* dispatch的ERR状态寄存器 */
/* 0x1110 */
typedef union tagScDispatchErrstat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    err_opcode            : 5   ; /* [4..0]  */
        unsigned int    err_addr              : 17  ; /* [21..5]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_ERRSTAT_U;

/* Define the union U_SC_DISPATCH_REMAP_CTRL_U */
/* subsys的启动Remap寄存器 */
/* 0x1200 */
typedef union tagScDispatchRemapCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sys_remap_vld         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_REMAP_CTRL_U;

/* Define the union U_SC_TSENSOR_CTRL0_U */
/* TSENSOR控制寄存器0 */
/* 0x20D0 */
typedef union tagScTsensorCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_temp_en       : 1   ; /* [0]  */
        unsigned int    reserved_1            : 7   ; /* [7..1]  */
        unsigned int    tsensor_ct_sel        : 2   ; /* [9..8]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_CTRL0_U;

/* Define the union U_SC_TSENSOR_CTRL1_U */
/* TSENSOR控制寄存器1 */
/* 0x20D4 */
typedef union tagScTsensorCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_test          : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_CTRL1_U;

/* Define the union U_SC_BOOTROM_CTRL_U */
/* BOOTROM控制寄存器 */
/* 0x20F0 */
typedef union tagScBootromCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    bootrom_rtsel         : 2   ; /* [1..0]  */
        unsigned int    bootrom_ptsel         : 2   ; /* [3..2]  */
        unsigned int    bootrom_trb           : 2   ; /* [5..4]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BOOTROM_CTRL_U;

/* Define the union U_SC_FTE_MUX_CTRL_U */
/* FTE复用选择寄存器 */
/* 0x2200 */
typedef union tagScFteMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mux_sel_fte           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_FTE_MUX_CTRL_U;

/* Define the union U_SC_RSA_ADDR_H_U */
/* RSA高20位地址补充寄存器 */
/* 0x2E00 */
typedef union tagScRsaAddrH
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rsa_addr_h            : 20  ; /* [19..0]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_ADDR_H_U;

/* Define the union U_SC_SMMU_MEM_CTRL0_U */
/* smmu mem控制寄存器0 */
/* 0x3000 */
typedef union tagScSmmuMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_smmu         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SMMU_MEM_CTRL0_U;

/* Define the union U_SC_SMMU_MEM_CTRL1_U */
/* smmu mem控制寄存器1 */
/* 0x3004 */
typedef union tagScSmmuMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsel_hc_smmu          : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SMMU_MEM_CTRL1_U;

/* Define the union U_SC_SMMU_MEM_CTRL2_U */
/* smmu mem控制寄存器2 */
/* 0x3008 */
typedef union tagScSmmuMemCtrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    test_hc_smmu          : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SMMU_MEM_CTRL2_U;

/* Define the union U_SC_RSA_MEM_CTRL_U */
/* rsa mem控制寄存器 */
/* 0x3030 */
typedef union tagScRsaMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_rsa          : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_MEM_CTRL_U;

/* Define the union U_SC_HLLC_MEM_CTRL_U */
/* hllc mem控制寄存器 */
/* 0x3040 */
typedef union tagScHllcMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rft_hllc         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_MEM_CTRL_U;

/* Define the union U_SC_PA_MEM_CTRL0_U */
/* pa mem控制寄存器0 */
/* 0x3050 */
typedef union tagScPaMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_pa           : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_MEM_CTRL0_U;

/* Define the union U_SC_PA_MEM_CTRL1_U */
/* pa mem控制寄存器1 */
/* 0x3054 */
typedef union tagScPaMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rft_pa           : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_MEM_CTRL1_U;

/* Define the union U_SC_RDE_MEM_CTRL_U */
/* rde mem控制寄存器 */
/* 0x3060 */
typedef union tagScRdeMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_rde          : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_MEM_CTRL_U;

/* Define the union U_SC_SEC_MEM_CTRL_U */
/* sec mem控制寄存器 */
/* 0x3070 */
typedef union tagScSecMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_sec          : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_MEM_CTRL_U;

/* Define the union U_SC_PLL_LOCK_ST_U */
/* PLL锁定状态寄存器 */
/* 0x5000 */
typedef union tagScPllLockSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll0_lock             : 1   ; /* [0]  */
        unsigned int    pll1_lock             : 1   ; /* [1]  */
        unsigned int    pll2_lock             : 1   ; /* [2]  */
        unsigned int    pll3_lock             : 1   ; /* [3]  */
        unsigned int    pll4_lock             : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_LOCK_ST_U;

/* Define the union U_SC_ITS_CLK_ST_U */
/* ITS时钟状态寄存器 */
/* 0x530C */
typedef union tagScItsClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_its_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_CLK_ST_U;

/* Define the union U_SC_PA_CLK_ST_U */
/* PA时钟状态寄存器 */
/* 0x532C */
typedef union tagScPaClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pa_st             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_CLK_ST_U;

/* Define the union U_SC_HLLC_CLK_ST_U */
/* HLLC时钟状态寄存器 */
/* 0x5330 */
typedef union tagScHllcClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_hllc_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_CLK_ST_U;

/* Define the union U_SC_REF_CLK_ST_U */
/* ref时钟状态寄存器 */
/* 0x5338 */
typedef union tagScRefClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_ref_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REF_CLK_ST_U;

/* Define the union U_SC_GPIO_DB_CLK_ST_U */
/* gpio_db时钟状态寄存器 */
/* 0x533C */
typedef union tagScGpioDbClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gpio_db_st        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_DB_CLK_ST_U;

/* Define the union U_SC_DJTAG_CLK_ST_U */
/* djtag时钟状态寄存器 */
/* 0x5340 */
typedef union tagScDjtagClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_djtag_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CLK_ST_U;

/* Define the union U_SC_HLLC_RXTX_CLK_ST_U */
/* hllc rxtx时钟状态寄存器 */
/* 0x5348 */
typedef union tagScHllcRxtxClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_hllc_hilink_tx0_st : 1   ; /* [0]  */
        unsigned int    clk_hllc_hilink_tx1_st : 1   ; /* [1]  */
        unsigned int    clk_hllc_hilink_tx2_st : 1   ; /* [2]  */
        unsigned int    clk_hllc_hilink_tx3_st : 1   ; /* [3]  */
        unsigned int    clk_hllc_hilink_tx4_st : 1   ; /* [4]  */
        unsigned int    clk_hllc_hilink_tx5_st : 1   ; /* [5]  */
        unsigned int    clk_hllc_hilink_tx6_st : 1   ; /* [6]  */
        unsigned int    clk_hllc_hilink_tx7_st : 1   ; /* [7]  */
        unsigned int    clk_hllc_hilink_rx0_st : 1   ; /* [8]  */
        unsigned int    clk_hllc_hilink_rx1_st : 1   ; /* [9]  */
        unsigned int    clk_hllc_hilink_rx2_st : 1   ; /* [10]  */
        unsigned int    clk_hllc_hilink_rx3_st : 1   ; /* [11]  */
        unsigned int    clk_hllc_hilink_rx4_st : 1   ; /* [12]  */
        unsigned int    clk_hllc_hilink_rx5_st : 1   ; /* [13]  */
        unsigned int    clk_hllc_hilink_rx6_st : 1   ; /* [14]  */
        unsigned int    clk_hllc_hilink_rx7_st : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_CLK_ST_U;

/* Define the union U_SC_RSA_CLK_ST_U */
/* RSA时钟状态寄存器 */
/* 0x5350 */
typedef union tagScRsaClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rsa_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_CLK_ST_U;

/* Define the union U_SC_RDE_CLK_ST_U */
/* RDE时钟状态寄存器 */
/* 0x5354 */
typedef union tagScRdeClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rde_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_CLK_ST_U;

/* Define the union U_SC_RNG_CLK_ST_U */
/* RNG时钟状态寄存器 */
/* 0x5358 */
typedef union tagScRngClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rng_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_CLK_ST_U;

/* Define the union U_SC_SEC_CLK_ST_U */
/* SEC时钟状态寄存器 */
/* 0x535C */
typedef union tagScSecClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sec_st            : 1   ; /* [0]  */
        unsigned int    clk_sec_ahb_st        : 1   ; /* [1]  */
        unsigned int    clk_sec_trace_dbg_st  : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_CLK_ST_U;

/* Define the union U_SC_ITS_RESET_ST_U */
/* ITS复位状态寄存器 */
/* 0x5A0C */
typedef union tagScItsResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    its_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_RESET_ST_U;

/* Define the union U_SC_HILINK_PCS_RESET_ST_U */
/* HILINK_PCS复位状态寄存器 */
/* 0x5A2C */
typedef union tagScHilinkPcsResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hllc_hilink_pcs_lane0_srst_st : 1   ; /* [0]  */
        unsigned int    hllc_hilink_pcs_lane1_srst_st : 1   ; /* [1]  */
        unsigned int    hllc_hilink_pcs_lane2_srst_st : 1   ; /* [2]  */
        unsigned int    hllc_hilink_pcs_lane3_srst_st : 1   ; /* [3]  */
        unsigned int    hllc_hilink_pcs_lane4_srst_st : 1   ; /* [4]  */
        unsigned int    hllc_hilink_pcs_lane5_srst_st : 1   ; /* [5]  */
        unsigned int    hllc_hilink_pcs_lane6_srst_st : 1   ; /* [6]  */
        unsigned int    hllc_hilink_pcs_lane7_srst_st : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK_PCS_RESET_ST_U;

/* Define the union U_SC_HLLC_RXTX_RESET_ST_U */
/* hllc rxtx复位状态寄存器 */
/* 0x5A30 */
typedef union tagScHllcRxtxResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hllc_rx0_srst_st      : 1   ; /* [0]  */
        unsigned int    hllc_rx1_srst_st      : 1   ; /* [1]  */
        unsigned int    hllc_rx2_srst_st      : 1   ; /* [2]  */
        unsigned int    hllc_rx3_srst_st      : 1   ; /* [3]  */
        unsigned int    hllc_rx4_srst_st      : 1   ; /* [4]  */
        unsigned int    hllc_rx5_srst_st      : 1   ; /* [5]  */
        unsigned int    hllc_rx6_srst_st      : 1   ; /* [6]  */
        unsigned int    hllc_rx7_srst_st      : 1   ; /* [7]  */
        unsigned int    hllc_tx0_srst_st      : 1   ; /* [8]  */
        unsigned int    hllc_tx1_srst_st      : 1   ; /* [9]  */
        unsigned int    hllc_tx2_srst_st      : 1   ; /* [10]  */
        unsigned int    hllc_tx3_srst_st      : 1   ; /* [11]  */
        unsigned int    hllc_tx4_srst_st      : 1   ; /* [12]  */
        unsigned int    hllc_tx5_srst_st      : 1   ; /* [13]  */
        unsigned int    hllc_tx6_srst_st      : 1   ; /* [14]  */
        unsigned int    hllc_tx7_srst_st      : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RXTX_RESET_ST_U;

/* Define the union U_SC_PA_RESET_ST_U */
/* PA复位状态寄存器 */
/* 0x5A3C */
typedef union tagScPaResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pa_srst_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PA_RESET_ST_U;

/* Define the union U_SC_HLLC_RESET_ST_U */
/* HLLC复位状态寄存器 */
/* 0x5A40 */
typedef union tagScHllcResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hllc_srst_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HLLC_RESET_ST_U;

/* Define the union U_SC_DJTAG_RESET_ST_U */
/* DJTAG复位状态寄存器 */
/* 0x5A48 */
typedef union tagScDjtagResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_RESET_ST_U;

/* Define the union U_SC_RSA_RESET_ST_U */
/* RSA复位状态寄存器 */
/* 0x5A4C */
typedef union tagScRsaResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rsa_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RSA_RESET_ST_U;

/* Define the union U_SC_RDE_RESET_ST_U */
/* RDE复位状态寄存器 */
/* 0x5A50 */
typedef union tagScRdeResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rde_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RDE_RESET_ST_U;

/* Define the union U_SC_SEC_RESET_ST_U */
/* SEC复位状态寄存器 */
/* 0x5A54 */
typedef union tagScSecResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sec_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_RESET_ST_U;

/* Define the union U_SC_RNG_RESET_ST_U */
/* RNG复位状态寄存器 */
/* 0x5A58 */
typedef union tagScRngResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rng_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RNG_RESET_ST_U;

/* Define the union U_SC_SEC_BUILD_RESET_ST_U */
/* SEC BUILD复位状态寄存器 */
/* 0x5A5C */
typedef union tagScSecBuildResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sec_cluster0_srst_st  : 1   ; /* [0]  */
        unsigned int    sec_cluster0_ahb_srst_st : 1   ; /* [1]  */
        unsigned int    sec_build_srst_st0    : 1   ; /* [2]  */
        unsigned int    sec_build_ahb_srst_st0 : 1   ; /* [3]  */
        unsigned int    sec_build_srst_st1    : 1   ; /* [4]  */
        unsigned int    sec_build_ahb_srst_st1 : 1   ; /* [5]  */
        unsigned int    sec_build_srst_st2    : 1   ; /* [6]  */
        unsigned int    sec_build_ahb_srst_st2 : 1   ; /* [7]  */
        unsigned int    sec_build_srst_st3    : 1   ; /* [8]  */
        unsigned int    sec_build_ahb_srst_st3 : 1   ; /* [9]  */
        unsigned int    sec_cluster1_srst_st  : 1   ; /* [10]  */
        unsigned int    sec_cluster1_ahb_srst_st : 1   ; /* [11]  */
        unsigned int    sec_build_srst_st4    : 1   ; /* [12]  */
        unsigned int    sec_build_ahb_srst_st4 : 1   ; /* [13]  */
        unsigned int    sec_build_srst_st5    : 1   ; /* [14]  */
        unsigned int    sec_build_ahb_srst_st5 : 1   ; /* [15]  */
        unsigned int    sec_build_srst_st6    : 1   ; /* [16]  */
        unsigned int    sec_build_ahb_srst_st6 : 1   ; /* [17]  */
        unsigned int    sec_build_srst_st7    : 1   ; /* [18]  */
        unsigned int    sec_build_ahb_srst_st7 : 1   ; /* [19]  */
        unsigned int    sec_ahb_srst_st       : 1   ; /* [20]  */
        unsigned int    reserved_0            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SEC_BUILD_RESET_ST_U;

/* Define the union U_SC_PCS_LOCAL_RESET_ST_U */
/* PCS LOCAL软复位请求状态寄存器 */
/* 0x5A60 */
typedef union tagScPcsLocalResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcs_local_srst_st     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_ST_U;

/* Define the union U_SC_TSENSOR_ST_U */
/* TSENSOR状态寄存器 */
/* 0x6040 */
typedef union tagScTsensorSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    temp_out_tsensor      : 8   ; /* [7..0]  */
        unsigned int    reserved_1            : 4   ; /* [11..8]  */
        unsigned int    data_ready_tsensor    : 1   ; /* [12]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_ST_U;

/* Define the union U_SC_DJTAG_INT_U */
/* DJTAG Master内状态机超时中断源寄存器 */
/* 0x6600 */
typedef union tagScDjtagInt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_sta_timeout     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_INT_U;

/* Define the union U_SC_DBG_AUTH_CTRL_U */
/* 安全等级授权控制寄存器。
（注意：1，此寄存器空间必须是安全访问才能成功；） */
/* 0x6700 */
typedef union tagScDbgAuthCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rsa_djtag_sec_acc_en  : 1   ; /* [0]  */
        unsigned int    rsa_secure_cfg        : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_AUTH_CTRL_U;

/* Define the union U_SC_DJTAG_MSTR_EN_U */
/* SC_DJTAG_MSTR_EN为DJTAG Master使能控制 */
/* 0x6800 */
typedef union tagScDjtagMstrEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_mstr_en         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_EN_U;

/* Define the union U_SC_DJTAG_MSTR_START_EN_U */
/* SC_DJTAG_MSTR_START_EN为DJTAG Master访问chain链的start使能 */
/* 0x6804 */
typedef union tagScDjtagMstrStartEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_mstr_start_en   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_START_EN_U;

/* Define the union U_SC_DJTAG_SEC_ACC_EN_U */
/* DJTAG访问各模块内安全世界寄存器的安全访问控制使能。（注意：1，此寄存器空间必须是安全访问才能成功。） */
/* 0x6808 */
typedef union tagScDjtagSecAccEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_sec_acc_en      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_SEC_ACC_EN_U;

/* Define the union U_SC_DJTAG_DEBUG_MODULE_SEL_U */
/* SC_DJTAG_DEBUG_MODULE_SEL为DJTAG Master配置访问模块寄存器 */
/* 0x680C */
typedef union tagScDjtagDebugModuleSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    debug_module_sel      : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_DEBUG_MODULE_SEL_U;

/* Define the union U_SC_DJTAG_MSTR_WR_U */
/* SC_DJTAG_MSTR_WR为DJTAG Master的读写控制信号 */
/* 0x6810 */
typedef union tagScDjtagMstrWr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_mstr_wr         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_WR_U;

/* Define the union U_SC_DJTAG_CHAIN_UNIT_CFG_EN_U */
/* SC_DJTAG_CHAIN_UNIT_CFG_EN为DJTAG Master的chain链上单元的访问使能 */
/* 0x6814 */
typedef union tagScDjtagChainUnitCfgEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    chain_unit_cfg_en     : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CHAIN_UNIT_CFG_EN_U;

/* Define the union U_SC_DJTAG_MSTR_ADDR_U */
/* SC_DJTAG_MSTR_ADDR为DJTAG Master配置访问模块的偏移地址寄存器 */
/* 0x6818 */
typedef union tagScDjtagMstrAddr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_mstr_addr       : 31  ; /* [30..0]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_ADDR_U;

/* Define the union U_SC_TDRE_OP_ADDR_U */
/* SC_TDRE_OP_ADDR为DJTAG Master配置TDRE模块的偏移地址寄存器和操作类型寄存器 */
/* 0x6824 */
typedef union tagScTdreOpAddr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tdre_op_addr          : 18  ; /* [17..0]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TDRE_OP_ADDR_U;

/* Define the union U_SC_TDRE_REPAIR_EN_U */
/* SC_TDRE_REPAIR_EN为DJTAG Master的TDRE REPAIR使能 */
/* 0x682C */
typedef union tagScTdreRepairEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tdre_repair_en        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TDRE_REPAIR_EN_U;

/* Define the union U_SC_ECO_RSV0_U */
/* ECO 寄存器0 */
/* 0x8000 */
typedef union tagScEcoRsv0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_nor_cfg_en      : 1   ; /* [0]  */
        unsigned int    eco_rsv0              : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ECO_RSV0_U;

/* Define the union U_SC_DJTAG_OP_ST_U */
/* DJTAG访问状态寄存器 */
/* 0xE828 */
typedef union tagScDjtagOpSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    unit_conflict         : 8   ; /* [7..0]  */
        unsigned int    djtag_op_done         : 1   ; /* [8]  */
        unsigned int    debug_bus_en          : 1   ; /* [9]  */
        unsigned int    reserved_1            : 6   ; /* [15..10]  */
        unsigned int    rdata_changed         : 10  ; /* [25..16]  */
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_OP_ST_U;


#endif

#endif // __ALG_SUB_REG_OFFSET_H__

/******************************************************************************

                  版权所有 (C), 2011-2021, 华为技术有限公司

 ******************************************************************************
  文 件 名   : sre_sys_ctrl.h
  版 本 号   : 初稿
  作    者   : g00220740
  生成日期   : 2014年6月26日
  最近修改   :
  功能描述   : sysctrl/poe子系统头文件定义
  函数列表   :
  修改历史   :
  1.日    期   : 2014年6月26日
    作    者   : g00220740
    修改内容   : 创建文件

******************************************************************************/

#ifndef __SYS_CTRL_REG_OFFSET_H__
#define __SYS_CTRL_REG_OFFSET_H__


#include "hrdCommon.h"
#include "hrdOs.h"
#include "osal_api.h"
#include "sre_crg_api.h"
#include "sre_sub_alg.h"
#include "HRD_hccs_hal.h"
#include "SRE_common.h"

#ifdef HRD_OS_LINUX
#include "iware_comm_kernel_api.h"
#endif

/*系统晶振时钟频率，默认50M*/
#define SYS_FREF  (50)
#define TOTEMC_PEOSUB_BASE (0x80000000)
#define TOTEMA_PEOSUB_BASE (0xE0000000)

/*定义PLL类型*/
#define SLICION_PLL (0x0)
#define TSMC_PLL    (0x1)
#define BIT_I(bitmap,i) ((bitmap>>(i))&(0x1))

typedef enum tagPllSel
{
    /*TOTEMC的各个PLL*/
    SRE_TOTEMC_PLL0 = 0x0,
    SRE_TOTEMC_PLL1,
    SRE_TOTEMC_PLL2,
    SRE_TOTEMC_PLL3,
    SRE_TOTEMC_PLL4,

    /*TOTEMA的各个PLL*/
    SRE_TOTEMA_PLL0,
    SRE_TOTEMA_PLL1,
    SRE_TOTEMA_PLL2,
    SRE_TOTEMA_PLL3,
    SRE_TOTEMA_PLL4,

    /*NIMBUS的各个PLL*/
    SRE_NIMBUS_PLL0,
    SRE_NIMBUS_PLL1,
    SRE_NIMBUS_PLL2,
    SRE_NIMBUS_PLL3,
    SRE_NIMBUS_PLL4,
}SRE_PLL_SEL_E;

#define SYSCTRL_BASE                       (0x40010000)
#define POE_SUB_BASE                       (0x0)

/******************************************************************************/
/*                      XXX SYSCTRL Registers' Definitions                            */
/******************************************************************************/

#define SYSCTRL_SC_CPU_EDBGRQ_CTRL_REG          (SYSCTRL_BASE + 0x0)    /* 对应cluster的Cortex-A57 CPU core debug请求。 */
#define SYSCTRL_SC_CPU_CP15SDIS_CRTL_REG        (SYSCTRL_BASE + 0x8)    /* 对应cluster的Cortex-A57 CPU core 某些安全寄存器CP15配置使能。 */
#define SYSCTRL_SC_CPU_PMU_CTRL_REG             (SYSCTRL_BASE + 0xC)    /* 对应cluster的PMU snapshot trigger request. */
#define SYSCTRL_SC_CLUS_L2_CLKEN_REG            (SYSCTRL_BASE + 0x10)   /* 对应cluster的Cortex-A57 subsys中L2部分时钟使能 */
#define SYSCTRL_SC_CPU_GICDIS_CTRL_REG          (SYSCTRL_BASE + 0x14)   /* 关闭对应cluster的Cortex-A57 CPU core的GIC接口逻辑，同时把nIRQ，nFIQ，nVIRQ，nVFIQ信号直接路由至处理器。 */
#define SYSCTRL_SC_CPU_CLKEN_REG                (SYSCTRL_BASE + 0x18)   /* 对应cluster的Cortex-A57 CPU core的全局时钟使能。 */
#define SYSCTRL_SC_CPU_L2FSHREQ_CTRL_REG        (SYSCTRL_BASE + 0x1C)   /* 对应cluster的L2 hardware flush request。 */
#define SYSCTRL_SC_CPU_CLREXMONREQ_CTRL_REG     (SYSCTRL_BASE + 0x20)   /* 对应cluster的clearing of the external global exclusive monitor request. */
#define SYSCTRL_SC_CPU_SYSBROADCAST_CTRL_REG    (SYSCTRL_BASE + 0x24)   /* SC_CPU_SYSBROADCAST_CTRL为系统广播控制 */
#define SYSCTRL_SC_CPU_SINACT_CTRL_REG          (SYSCTRL_BASE + 0x28)   /* SC_CPU_SINACT_CTRL为对应cluster的skyros接口使能 */
#define SYSCTRL_SC_CLUS0_CTRL_RVBA0_REG         (SYSCTRL_BASE + 0x34)   /* SC_CLUS0_CTRL_RVBA表示AArch64下CPU0复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS0_CTRL_RVBA1_REG         (SYSCTRL_BASE + 0x3C)   /* SC_CLUS0_CTRL_RVBA表示AArch64下CPU1复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS0_CTRL_RVBA2_REG         (SYSCTRL_BASE + 0x44)   /* SC_CLUS0_CTRL_RVBA表示AArch64下CPU2复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS0_CTRL_RVBA3_REG         (SYSCTRL_BASE + 0x4C)   /* SC_CLUS0_CTRL_RVBA表示AArch64下CPU3复位基地址的设置。此寄存器为安全寄存器 */
#define SYSCTRL_SC_CLUS1_CTRL_RVBA0_REG         (SYSCTRL_BASE + 0x54)   /* SC_CLUS1_CTRL_RVBA表示AArch64下CPU0复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS1_CTRL_RVBA1_REG         (SYSCTRL_BASE + 0x5C)   /* SC_CLUS1_CTRL_RVBA表示AArch64下CPU1复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS1_CTRL_RVBA2_REG         (SYSCTRL_BASE + 0x64)   /* SC_CLUS1_CTRL_RVBA表示AArch64下CPU2复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS1_CTRL_RVBA3_REG         (SYSCTRL_BASE + 0x6C)   /* SC_CLUS1_CTRL_RVBA表示AArch64下CPU3复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS2_CTRL_RVBA0_REG         (SYSCTRL_BASE + 0x74)   /* SC_CLUS2_CTRL_RVBA表示AArch64下CPU0复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS2_CTRL_RVBA1_REG         (SYSCTRL_BASE + 0x7C)   /* SC_CLUS2_CTRL_RVBA表示AArch64下CPU1复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS2_CTRL_RVBA2_REG         (SYSCTRL_BASE + 0x84)   /* SC_CLUS2_CTRL_RVBA表示AArch64下CPU2复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS2_CTRL_RVBA3_REG         (SYSCTRL_BASE + 0x8C)   /* SC_CLUS2_CTRL_RVBA表示AArch64下CPU3复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS3_CTRL_RVBA0_REG         (SYSCTRL_BASE + 0x94)   /* SC_CLUS3_CTRL_RVBA表示AArch64下CPU0复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS3_CTRL_RVBA1_REG         (SYSCTRL_BASE + 0x9C)   /* SC_CLUS3_CTRL_RVBA表示AArch64下CPU1复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS3_CTRL_RVBA2_REG         (SYSCTRL_BASE + 0xA4)   /* SC_CLUS3_CTRL_RVBA表示AArch64下CPU2复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_CLUS3_CTRL_RVBA3_REG         (SYSCTRL_BASE + 0xAC)   /* SC_CLUS3_CTRL_RVBA表示AArch64下CPU3复位基地址的设置。此寄存器为安全寄存器，只有安全CPU可配 */
#define SYSCTRL_SC_PERIPHBASE_REG               (SYSCTRL_BASE + 0xB0)   /* SC_PERIPHBASE表示CPU内部GICC起始地址 */
#define SYSCTRL_SC_DBG_AUTH_CTRL_REG            (SYSCTRL_BASE + 0xB4)   /* 安全等级授权控制寄存器。（注意：1，此寄存器空间必须是安全访问才能成功；2，默认均是打开调试模式，有漏洞，在正常工作时请初始化关闭调试功能） */
#define SYSCTRL_SC_CLUS_CRYPTODISABLE_REG       (SYSCTRL_BASE + 0xB8)   /* SC_CLUS_CRYPTODISABLE表示Individual processor Cryptography engine disable */
#define SYSCTRL_SC_CORE0_EVENT_EN_REG           (SYSCTRL_BASE + 0x930)  /* 到Core0的Event源头选择使能 */
#define SYSCTRL_SC_CORE1_EVENT_EN_REG           (SYSCTRL_BASE + 0x934)  /* 到Core1的Event源头选择使能 */
#define SYSCTRL_SC_CORE2_EVENT_EN_REG           (SYSCTRL_BASE + 0x938)  /* 到Core2的Event源头选择使能 */
#define SYSCTRL_SC_CORE3_EVENT_EN_REG           (SYSCTRL_BASE + 0x93C)  /* 到Core3的Event源头选择使能 */
#define SYSCTRL_SC_CORE4_EVENT_EN_REG           (SYSCTRL_BASE + 0x940)  /* 到Core4的Event源头选择使能 */
#define SYSCTRL_SC_CORE5_EVENT_EN_REG           (SYSCTRL_BASE + 0x944)  /* 到Core5的Event源头选择使能 */
#define SYSCTRL_SC_CORE6_EVENT_EN_REG           (SYSCTRL_BASE + 0x948)  /* 到Core6的Event源头选择使能 */
#define SYSCTRL_SC_CORE7_EVENT_EN_REG           (SYSCTRL_BASE + 0x94C)  /* 到Core7的Event源头选择使能 */
#define SYSCTRL_SC_CORE8_EVENT_EN_REG           (SYSCTRL_BASE + 0x950)  /* 到Core8的Event源头选择使能 */
#define SYSCTRL_SC_CORE9_EVENT_EN_REG           (SYSCTRL_BASE + 0x954)  /* 到Core9的Event源头选择使能 */
#define SYSCTRL_SC_CORE10_EVENT_EN_REG          (SYSCTRL_BASE + 0x958)  /* 到Core10的Event源头选择使能 */
#define SYSCTRL_SC_CORE11_EVENT_EN_REG          (SYSCTRL_BASE + 0x95C)  /* 到Core11的Event源头选择使能 */
#define SYSCTRL_SC_CORE12_EVENT_EN_REG          (SYSCTRL_BASE + 0x960)  /* 到Core12的Event源头选择使能 */
#define SYSCTRL_SC_CORE13_EVENT_EN_REG          (SYSCTRL_BASE + 0x964)  /* 到Core13的Event源头选择使能 */
#define SYSCTRL_SC_CORE14_EVENT_EN_REG          (SYSCTRL_BASE + 0x968)  /* 到Core14的Event源头选择使能 */
#define SYSCTRL_SC_CORE15_EVENT_EN_REG          (SYSCTRL_BASE + 0x96C)  /* 到Core15的Event源头选择使能 */
#define SYSCTRL_SC_CPU_DBGL1RSTDISABLE_CTRL_REG (SYSCTRL_BASE + 0x980)  /* 对应cluster的Cortex-A57 CPU core debug请求。 */
#define SYSCTRL_SC_SYSMODE_CTRL_REG             (SYSCTRL_BASE + 0x3000) /* SC_SYSMODE_CTRL为CDIE_CRG系统时钟工作模式控制寄存器 */
#define SYSCTRL_SC_SYSSOFTRST_CTRL_REG          (SYSCTRL_BASE + 0x3004) /* SC_SYSSOFTRST_CTRL为CDIE_CRG系统软复位寄存器。 */
#define SYSCTRL_SC_PLLCTRL_REG                  (SYSCTRL_BASE + 0x3010) /* SC_PLLCTRL为PLL控制寄存器。 */
#define SYSCTRL_SC_PLLFCTRL4_REG                (SYSCTRL_BASE + 0x3024) /* SC_PLLFCTRL4为CIDE_CRG的PLL2控制寄存器0。 */
#define SYSCTRL_SC_PLLFCTRL5_REG                (SYSCTRL_BASE + 0x3028) /* SC_PLLFCTRL5为CIDE_CRG的PLL2控制寄存器1。 */
#define SYSCTRL_SC_PLLFCTRL6_REG                (SYSCTRL_BASE + 0x302C) /* SC_PLLFCTRL6为CIDE_CRG的PLL3控制寄存器0。 */
#define SYSCTRL_SC_PLLFCTRL7_REG                (SYSCTRL_BASE + 0x3030) /* SC_PLLFCTRL7为CIDE_CRG的PLL3控制寄存器1。 */
#define SYSCTRL_SC_PLL_CLK_BYPASS_DDR_REG       (SYSCTRL_BASE + 0x3044) /* SC_PLL_CLK_BYPASS_DDR为CDIE_CRG的DDR PLL相关工作时钟选择。 */
#define SYSCTRL_SC_REF_CLK_EN_REG               (SYSCTRL_BASE + 0x3050) /* SC_REF_CLK_EN是clk_ref的时钟使能寄存器。 */
#define SYSCTRL_SC_REF_CLK_DIS_REG              (SYSCTRL_BASE + 0x3054) /* SC_REF_CLK_DIS是clk_ref时钟禁止寄存器。 */
#define SYSCTRL_SC_GPIO_CLK_EN_REG              (SYSCTRL_BASE + 0x3058) /* SC_GPIO_CLK_EN是clk_gpio_db的时钟使能寄存器。 */
#define SYSCTRL_SC_GPIO_CLK_DIS_REG             (SYSCTRL_BASE + 0x305C) /* SC_GPIO_CLK_DIS是clk_gpio_db时钟禁止寄存器。 */
#define SYSCTRL_SC_CLUS0_CRG_CLK_EN_REG         (SYSCTRL_BASE + 0x3060) /* SC_CLUS0_CRG_CLK_EN表示对当前CLUSTER内CRG的CLKEN信号的配置。 */
#define SYSCTRL_SC_CLUS0_CRG_CLK_DIS_REG        (SYSCTRL_BASE + 0x3064) /* SC_CLUS0_CRG_CLK_DIS表示对当前CLUSTER内CRG的时钟禁止使能配置。 */
#define SYSCTRL_SC_CLUS0_CRG_RESET_REQ_REG      (SYSCTRL_BASE + 0x3068) /* SC_CLUS0_CRG_RESET_REQ表示对当前CLUSTER内CRG的RST_REQ信号的配置。 */
#define SYSCTRL_SC_CLUS0_CRG_RESET_DREQ_REG     (SYSCTRL_BASE + 0x306C) /* SC_CLUS0_CRG_RESET_DEREQ表示对当前CLUSTER内CRG的RST_REQ信号的去复位。 */
#define SYSCTRL_SC_CLUS1_CRG_CLK_EN_REG         (SYSCTRL_BASE + 0x3070) /* SC_CLUS1_CRG_CLK_EN表示对当前CLUSTER内CRG的CLKEN信号的配置。 */
#define SYSCTRL_SC_CLUS1_CRG_CLK_DIS_REG        (SYSCTRL_BASE + 0x3074) /* SC_CLUS1_CRG_CLK_DIS表示对当前CLUSTER内CRG的CLKEN信号的关闭。 */
#define SYSCTRL_SC_CLUS1_CRG_RESET_REQ_REG      (SYSCTRL_BASE + 0x3078) /* SC_CLUS1_CRG_RESET_REQ表示对当前CLUSTER内CRG的RST_REQ信号的配置。 */
#define SYSCTRL_SC_CLUS1_CRG_RESET_DREQ_REG     (SYSCTRL_BASE + 0x307C) /* SC_CLUS1_CRG_RESET_DEREQ表示对当前CLUSTER内CRG的RST_REQ信号的去复位。 */
#define SYSCTRL_SC_CLUS2_CRG_CLK_EN_REG         (SYSCTRL_BASE + 0x3080) /* SC_CLUS2_CRG_CLK_EN表示对当前CLUSTER内CRG的CLKEN信号的配置。 */
#define SYSCTRL_SC_CLUS2_CRG_CLK_DIS_REG        (SYSCTRL_BASE + 0x3084) /* SC_CLUS2_CRG_CLK_DIS表示对当前CLUSTER内CRG的CLKEN信号的关闭。 */
#define SYSCTRL_SC_CLUS2_CRG_RESET_REQ_REG      (SYSCTRL_BASE + 0x3088) /* SC_CLUS2_CRG_RESET_REQ表示对当前CLUSTER内CRG的RST_REQ信号的配置。 */
#define SYSCTRL_SC_CLUS2_CRG_RESET_DREQ_REG     (SYSCTRL_BASE + 0x308C) /* SC_CLUS2_CRG_RESET_DEREQ表示对当前CLUSTER内CRG的RST_REQ信号的去复位。 */
#define SYSCTRL_SC_CLUS3_CRG_CLK_EN_REG         (SYSCTRL_BASE + 0x3090) /* SC_CLUS3_CRG_CLK_EN表示对当前CLUSTER内CRG的CLKEN信号的配置。 */
#define SYSCTRL_SC_CLUS3_CRG_CLK_DIS_REG        (SYSCTRL_BASE + 0x3094) /* SC_CLUS3_CRG_CLK_DIS表示对当前CLUSTER内CRG的CLKEN信号的关闭。 */
#define SYSCTRL_SC_CLUS3_CRG_RESET_REQ_REG      (SYSCTRL_BASE + 0x3098) /* SC_CLUS3_CRG_RESET_REQ表示对当前CLUSTER内CRG的RST_REQ信号的配置。 */
#define SYSCTRL_SC_CLUS3_CRG_RESET_DREQ_REG     (SYSCTRL_BASE + 0x309C) /* SC_CLUS3_CRG_RESET_DEREQ表示对当前CLUSTER内CRG的RST_REQ信号的去复位。 */
#define SYSCTRL_SC_LLC_CLK_EN_REG               (SYSCTRL_BASE + 0x30A0) /* SC_LLC_CLK_EN为LLC的时钟使能寄存器 */
#define SYSCTRL_SC_LLC_CLK_DIS_REG              (SYSCTRL_BASE + 0x30A4) /* SC_LLC_CLK_DIS为LLC的时钟禁止寄存器 */
#define SYSCTRL_SC_LLC_RESET_REQ_REG            (SYSCTRL_BASE + 0x30A8) /* SC_LLC_RESET_REQ为CDIE内LLC软复位请求控制寄存器 */
#define SYSCTRL_SC_LLC_RESET_DREQ_REG           (SYSCTRL_BASE + 0x30AC) /* SC_LLC_RESET_DREQ为CDIE内LLC软复位去请求控制寄存器 */
#define SYSCTRL_SC_DDRC_CLK_EN_REG              (SYSCTRL_BASE + 0x30B0) /* SC_DDRC_CLK_EN为DDRC的时钟使能寄存器 */
#define SYSCTRL_SC_DDRC_CLK_DIS_REG             (SYSCTRL_BASE + 0x30B4) /* SC_DDRC_CLK_DIS为DDRC的时钟禁止寄存器 */
#define SYSCTRL_SC_DDRC_RESET_REQ_REG           (SYSCTRL_BASE + 0x30B8) /* SC_DDRC_RESET_REQ为CDIE内DDRC软复位请求控制寄存器 */
#define SYSCTRL_SC_DDRC_RESET_DREQ_REG          (SYSCTRL_BASE + 0x30BC) /* SC_DDRC_RESET_DREQ为CDIE内DDRC软复位去请求控制寄存器 */
#define SYSCTRL_SC_SLLC_CLK_EN_REG              (SYSCTRL_BASE + 0x30D0) /* SC_SLLC_CLK_EN表示SLLC逻辑时钟使能配置寄存器 */
#define SYSCTRL_SC_SLLC_CLK_DIS_REG             (SYSCTRL_BASE + 0x30D4) /* SC_SLLC_CLK_DIS表示SLLC逻辑时钟禁止寄存器 */
#define SYSCTRL_SC_SLLC_RESET_REQ_REG           (SYSCTRL_BASE + 0x30D8) /* SC_SLLC_RESET_REQ表示SLLC的软复位请求控制寄存器 */
#define SYSCTRL_SC_SLLC_RESET_DREQ_REG          (SYSCTRL_BASE + 0x30DC) /* SC_SLLC_RESET_DREQ表示SLLC的软复位去请求控制寄存器 */
#define SYSCTRL_SC_SLLC_TSVRX_RESET_REQ_REG     (SYSCTRL_BASE + 0x30E0) /* SC_SLLC_TSVRX_RESET_REQ为SLLC TSVRX通道软复位请求控制寄存器 */
#define SYSCTRL_SC_SLLC_TSVRX_RESET_DREQ_REG    (SYSCTRL_BASE + 0x30E4) /* SC_SLLC_TSVRX_RESET_DREQ为SLLC TSVRX通道软复位请求控制寄存器 */
#define SYSCTRL_SC_HPM_CLK_EN_REG               (SYSCTRL_BASE + 0x30E8) /* SC_HPM_CLK_EN表示HPM逻辑时钟使能配置寄存器 */
#define SYSCTRL_SC_HPM_CLK_DIS_REG              (SYSCTRL_BASE + 0x30EC) /* SC_HPM_CLK_DIS表示HPM逻辑时钟禁止配置寄存器 */
#define SYSCTRL_SC_HPM_RESET_REQ_REG            (SYSCTRL_BASE + 0x30F0) /* SC_HPM_RESET_REQ为CDIE内HPM软复位请求控制寄存器 */
#define SYSCTRL_SC_HPM_RESET_DREQ_REG           (SYSCTRL_BASE + 0x30F4) /* SC_HPM_RESET_DREQ为CDIE内HPM软复位去请求控制寄存器 */
#define SYSCTRL_SC_DJTAG_CLK_EN_REG             (SYSCTRL_BASE + 0x3100) /* SC_DJTAG_CLKEN表示DJTAG逻辑时钟使能配置寄存器 */
#define SYSCTRL_SC_DJTAG_CLK_DIS_REG            (SYSCTRL_BASE + 0x3104) /* SC_DJTAG_CLK_DIS表示DJTAG逻辑时钟禁止寄存器 */
#define SYSCTRL_SC_DJTAG_SRST_REQ_REG           (SYSCTRL_BASE + 0x3108) /* SC_DJTAG_SRST_REQ表示DJTAG逻辑复位请求寄存器 */
#define SYSCTRL_SC_DJTAG_SRST_DREQ_REG          (SYSCTRL_BASE + 0x310C) /* SC_DJTAG_SRST_DREQ表示DJTAG逻辑复位去请求寄存器 */
#define SYSCTRL_SC_HHA_MN_CLK_EN_REG            (SYSCTRL_BASE + 0x3110) /* SC_HHA_MN_CLK_EN表示HHA和MN逻辑时钟使能配置寄存器 */
#define SYSCTRL_SC_HHA_MN_CLK_DIS_REG           (SYSCTRL_BASE + 0x3114) /* SC_HHA_MN_CLK_DIS表示HHA和MN逻辑时钟禁止使能配置寄存器 */
#define SYSCTRL_SC_HHA_MN_RESET_REQ_REG         (SYSCTRL_BASE + 0x3118) /* SC_HHA_MN_RESET_REQ为HHA和MN软复位请求控制寄存器 */
#define SYSCTRL_SC_HHA_MN_RESET_DREQ_REG        (SYSCTRL_BASE + 0x311C) /* SC_HHA_MN_RESET_DREQ为HHA和MN软复位去请求控制寄存器 */
#define SYSCTRL_SC_WDOG_RESET_CFG_REG           (SYSCTRL_BASE + 0x3180) /* 看门狗溢出复位范围控制寄存器。 */
#define SYSCTRL_SC_TSMC_PLLCTRL2_REG            (SYSCTRL_BASE + 0x3208) /* SC_TSMC_PLLCTRL2为CIDE CRG的PLL2对应的TSMC备份PLL的控制寄存器。 */
#define SYSCTRL_SC_TSMC_PLLCTRL3_REG            (SYSCTRL_BASE + 0x320C) /* SC_TSMC_PLLCTRL3为CIDE CRG的PLL3对应的TSMC备份PLL的控制寄存器。 */
#define SYSCTRL_SC_PLL_SEL_DDR_REG              (SYSCTRL_BASE + 0x3214) /* SC_PLL_SEL为CIDE内的PLL选择控制寄存器。 */
#define SYSCTRL_SC_ALL_SCAN_CTRL_REG            (SYSCTRL_BASE + 0x3220) /* SC_ALL_SCAN_CTRL为ALL_SCAN模式OCC时钟关断控制寄存器。 */
#define SYSCTRL_SC_TSENSOR_CTRL_REG             (SYSCTRL_BASE + 0x5000) /* 片上温度传感器工作参数配置。 */
#define SYSCTRL_SC_TSENSOR_TEST_REG             (SYSCTRL_BASE + 0x5004) /* Tsensor测试使用，请正常用户不要配置修改。 */
#define SYSCTRL_SC_TSENSOR_ALARM_REG            (SYSCTRL_BASE + 0x5008) /* 片上温度传感器温度门限配置寄存器 */
#define SYSCTRL_SC_DDRC_APB_CLK_EN_REG          (SYSCTRL_BASE + 0x5400) /* DDRC的APB总线CLKEN控制寄存器。 */
#define SYSCTRL_SC_MBIST_CPUI_ENABLE_REG        (SYSCTRL_BASE + 0x5C00) /* CPU启动MBIST选择信号；芯片电路测试使用。 */
#define SYSCTRL_SC_MBIST_CPUI_RESET_N_REG       (SYSCTRL_BASE + 0x5C04) /* CPU启动MBIST复位信号 */
#define SYSCTRL_SC_MBIST_CPUI_DATAIN_REG        (SYSCTRL_BASE + 0x5C08) /* CPU启动MBIST，TMB电路数据和指令输入接口寄存器 */
#define SYSCTRL_SC_MBIST_CPUI_WRITE_EN_REG      (SYSCTRL_BASE + 0x5C0C) /* DFT MBIST模式下TMB电路CPU2TAP接口配置寄存器；芯片电路测试使用。 */
#define SYSCTRL_SC_SRC_INT_REG                  (SYSCTRL_BASE + 0x6000) /* 系统控制器中断源寄存器 */
#define SYSCTRL_SC_INT_MASK_REG                 (SYSCTRL_BASE + 0x6004) /* 系统控制器中断屏蔽寄存器 */
#define SYSCTRL_SC_IM_CTRL_REG                  (SYSCTRL_BASE + 0x6008)
#define SYSCTRL_SC_IM_STAT_REG                  (SYSCTRL_BASE + 0x600C)
#define SYSCTRL_SC_XTAL_CTRL_REG                (SYSCTRL_BASE + 0x6010)
#define SYSCTRL_SC_ITCR_REG                     (SYSCTRL_BASE + 0x6014)
#define SYSCTRL_SC_ITIR0_REG                    (SYSCTRL_BASE + 0x6018)
#define SYSCTRL_SC_ITOR_REG                     (SYSCTRL_BASE + 0x601C)
#define SYSCTRL_SC_CNT_CTRL_REG                 (SYSCTRL_BASE + 0x6020)
#define SYSCTRL_SC_BOOT_CTRL_CFG_EN_REG         (SYSCTRL_BASE + 0x6030) /* SC_BOOT_CTRL_CFG_EN表示CPU BOOT管脚控制信号的配置使能，此寄存器为安全寄存器，只有安全CPU可访问 */
#define SYSCTRL_SC_BOOT_CTRL_CFG_REG            (SYSCTRL_BASE + 0x6034) /* SC_BOOT_CTRL_CFG表示CPU启动控制信号的配置寄存器，此寄存器为安全寄存器，只有安全CPU可访问 */
#define SYSCTRL_SC_SOCKET_INITIALED_REG         (SYSCTRL_BASE + 0x6040) /* SC_SOCKET_INITIALED表示socket的初始化完成标记寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_EN_REG           (SYSCTRL_BASE + 0x6100) /* SC_SYSCNT_SYNC_EN表示同步system counter的启动使能寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_DST_ADDR1_REG    (SYSCTRL_BASE + 0x6104) /* SC_SYSCNT_SYNC_DST_ADDR1表示同步system counter的地址寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_DST_ADDR0_REG    (SYSCTRL_BASE + 0x6108) /* SC_SYSCNT_SYNC_DST_ADDR0表示同步system counter的地址寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_SRC_ADDR1_REG    (SYSCTRL_BASE + 0x610C) /* SC_SYSCNT_SYNC_SRC_ADDR1表示同步system counter的地址寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_SRC_ADDR0_REG    (SYSCTRL_BASE + 0x6110) /* SC_SYSCNT_SYNC_SRC_ADDR0表示同步system counter的地址寄存器 */
#define SYSCTRL_SC_CORESIGHT_SYNC_DST_ADDR1_REG (SYSCTRL_BASE + 0x6114) /* SC_CORESIGHT_SYNC_DST_ADDR1表示同步system counter的地址寄存器 */
#define SYSCTRL_SC_CORESIGHT_SYNC_DST_ADDR0_REG (SYSCTRL_BASE + 0x6118) /* SC_CORESIGHT_SYNC_DST_ADDR0表示同步system counter的地址寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_AVE_EN_REG       (SYSCTRL_BASE + 0x6120) /* SC_SYSCNT_SYNC_AVE_EN表示system counter同步的延时平均使能寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_DST_DATA_EN_REG  (SYSCTRL_BASE + 0x6140) /* SC_SYSCNT_SYNC_DST_DATA_EN表示system counter同步数据使能的寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_DST_DATA0_REG    (SYSCTRL_BASE + 0x6148) /* SC_SYSCNT_SYNC_DST_DATA0表示system counter同步数据的寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_DST_DATA1_REG    (SYSCTRL_BASE + 0x614C) /* SC_SYSCNT_SYNC_DST_DATA1表示system counter同步数据的寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_SRC_DATA_EN_REG  (SYSCTRL_BASE + 0x6180) /* SC_SYSCNT_SYNC_SRC_DATA_EN表示system counter同步数据使能的寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_SRC_DATA0_REG    (SYSCTRL_BASE + 0x6188) /* SC_SYSCNT_SYNC_SRC_DATA0表示system counter同步数据的寄存器 */
#define SYSCTRL_SC_SYSCNT_SYNC_SRC_DATA1_REG    (SYSCTRL_BASE + 0x618C) /* SC_SYSCNT_SYNC_SRC_DATA1表示system counter同步数据的寄存器 */
#define SYSCTRL_SC_SYSCNT_VAL_CFG_EN_REG        (SYSCTRL_BASE + 0x6200) /* SC_SYSCNT_VAL_CFG_EN表示system counter同步时统计延迟的配置使能寄存器 */
#define SYSCTRL_SC_SYSCNT_VAL_CFG_DATA_REG      (SYSCTRL_BASE + 0x6204) /* SC_SYSCNT_VAL_CFG_DATA1表示system counter同步时统计延迟高32bit的配置寄存器 */
#define SYSCTRL_SC_PROBE_SYSCNT_EN_REG          (SYSCTRL_BASE + 0x6210) /* SC_PROBE_SYSCNT_EN表示system counter同步时统计延迟的probe使能寄存器 */
#define SYSCTRL_SC_PROBE_SYSCNT_DATA_REG        (SYSCTRL_BASE + 0x6214) /* SC_PROBE_SYSCNT_DATA表示system counter同步时统计延迟的probe数据寄存器 */
#define SYSCTRL_SC_BROADCAST_EN_REG             (SYSCTRL_BASE + 0x6220) /* SC_BROADCAST_EN表示CPU CORE EVENT_EN的broadcast使能 */
#define SYSCTRL_SC_BROADCAST_DIE1_ADDR0_REG     (SYSCTRL_BASE + 0x6230) /* SC_BROADCAST_DIE1_ADDR0表示CPU CORE EVENT_EN的广播到DIE1地址 */
#define SYSCTRL_SC_BROADCAST_DIE1_ADDR1_REG     (SYSCTRL_BASE + 0x6234) /* SC_BROADCAST_DIE1_ADDR1表示CPU CORE EVENT_EN的广播到DIE1地址 */
#define SYSCTRL_SC_BROADCAST_DIE2_ADDR0_REG     (SYSCTRL_BASE + 0x6238) /* SC_BROADCAST_DIE2_ADDR0表示CPU CORE EVENT_EN的广播到DIE2地址 */
#define SYSCTRL_SC_BROADCAST_DIE2_ADDR1_REG     (SYSCTRL_BASE + 0x623C) /* SC_BROADCAST_DIE2_ADDR1表示CPU CORE EVENT_EN的广播到DIE2地址 */
#define SYSCTRL_SC_BROADCAST_DIE3_ADDR0_REG     (SYSCTRL_BASE + 0x6240) /* SC_BROADCAST_DIE3_ADDR0表示CPU CORE EVENT_EN的广播到DIE3地址 */
#define SYSCTRL_SC_BROADCAST_DIE3_ADDR1_REG     (SYSCTRL_BASE + 0x6244) /* SC_BROADCAST_DIE3_ADDR1表示CPU CORE EVENT_EN的广播到DIE3地址 */
#define SYSCTRL_SC_BROADCAST_DIE4_ADDR0_REG     (SYSCTRL_BASE + 0x6248) /* SC_BROADCAST_DIE4_ADDR0表示CPU CORE EVENT_EN的广播到DIE4地址 */
#define SYSCTRL_SC_BROADCAST_DIE4_ADDR1_REG     (SYSCTRL_BASE + 0x624C) /* SC_BROADCAST_DIE4_ADDR1表示CPU CORE EVENT_EN的广播到DIE4地址 */
#define SYSCTRL_SC_BROADCAST_DIE5_ADDR0_REG     (SYSCTRL_BASE + 0x6250) /* SC_BROADCAST_DIE5_ADDR0表示CPU CORE EVENT_EN的广播到DIE5地址 */
#define SYSCTRL_SC_BROADCAST_DIE5_ADDR1_REG     (SYSCTRL_BASE + 0x6254) /* SC_BROADCAST_DIE5_ADDR1表示CPU CORE EVENT_EN的广播到DIE5地址 */
#define SYSCTRL_SC_BROADCAST_CORE_EVENT_REG     (SYSCTRL_BASE + 0x6260) /* SC_BROADCAST_CORE_EVENT表示SYSCTRL接收来自其他CPU CORE的 EVENT_EN广播 */
#define SYSCTRL_SC_DJTAG_MSTR_EN_REG            (SYSCTRL_BASE + 0x6800) /* SC_DJTAG_MSTR_EN为SYS_CTRL内DJTAG Master使能控制 */
#define SYSCTRL_SC_DJTAG_MSTR_START_EN_REG      (SYSCTRL_BASE + 0x6804) /* SC_DJTAG_MSTR_START_EN为DJTAG Master访问chain链的start使能 */
#define SYSCTRL_SC_DJTAG_SEC_ACC_EN_REG         (SYSCTRL_BASE + 0x6808) /* DJTAG访问各模块内安全世界寄存器的安全访问控制使能。（注意：1，此寄存器空间必须是安全访问才能成功。） */
#define SYSCTRL_SC_DJTAG_DEBUG_MODULE_SEL_REG   (SYSCTRL_BASE + 0x680C) /* SC_DJTAG_DEBUG_MODULE_SEL为SYS_CTRL内DJTAG Master配置访问模块寄存器 */
#define SYSCTRL_SC_DJTAG_MSTR_WR_REG            (SYSCTRL_BASE + 0x6810) /* SC_DJTAG_MSTR_WR为SYS_CTRL内DJTAG Master的读写控制信号 */
#define SYSCTRL_SC_DJTAG_CHAIN_UNIT_CFG_EN_REG  (SYSCTRL_BASE + 0x6814) /* SC_DJTAG_CHAIN_UNIT_CFG_EN为DJTAG Master的chain链上单元的访问使能 */
#define SYSCTRL_SC_DJTAG_MSTR_ADDR_REG          (SYSCTRL_BASE + 0x6818) /* SC_DJTAG_MSTR_ADDR为SYS_CTRL内DJTAG Master配置访问模块的偏移地址寄存器 */
#define SYSCTRL_SC_DJTAG_MSTR_DATA_REG          (SYSCTRL_BASE + 0x681C) /* SC_DJTAG_MSTR_DATA为SYS_CTRL内DJTAG Master的写数据 */
#define SYSCTRL_SC_DJTAG_TMOUT_REG              (SYSCTRL_BASE + 0x6820) /* SC_DJTAG_TMOUT为SYS_CTRL内DJTAG Master的状态机超时阈值 */
#define SYSCTRL_SC_TDRE_OP_ADDR_REG             (SYSCTRL_BASE + 0x6824) /* SC_TDRE_OP_ADDR为SYS_CTRL内DJTAG Master配置TDRE模块的偏移地址寄存器和操作类型寄存器 */
#define SYSCTRL_SC_TDRE_WDATA_REG               (SYSCTRL_BASE + 0x6828) /* SC_TDRE_DATA为SYS_CTRL内DJTAG Master的写TDRE数据 */
#define SYSCTRL_SC_TDRE_REPAIR_EN_REG           (SYSCTRL_BASE + 0x682C) /* SC_TDRE_REPAIR_EN为SYS_CTRL内DJTAG Master的TDRE REPAIR使能 */
#define SYSCTRL_SC_AXI_MSTR_START_EN_REG        (SYSCTRL_BASE + 0x6830) /* SC_AXI_MSTR_START_EN表示SYS_CTRL内AXI Master的start使能 */
#define SYSCTRL_SC_AXI_MSTR_SEC_ACC_EN_REG      (SYSCTRL_BASE + 0x6834) /* sysctrl内AXI Master访问DDR安全地址空间的安全访问控制使能。（注意：1，此寄存器空间必须是安全访问才能成功。） */
#define SYSCTRL_SC_AXI_MSTR_WR_REG              (SYSCTRL_BASE + 0x6838) /* SC_AXI_MSTR_WR为SYS_CTRL内AXI Master读写使能 */
#define SYSCTRL_SC_AXI_MSTR_USER_REG            (SYSCTRL_BASE + 0x683C) /* SC_AXI_MSTR_USER表示SYS_CTRL内AXI Master的虚拟机ID */
#define SYSCTRL_SC_AXI_MSTR_ADDR1_REG           (SYSCTRL_BASE + 0x6840) /* SC_AXI_MSTR_ADDR1为SYS_CTRL内AXI Master的访问地址bit[63:32] */
#define SYSCTRL_SC_AXI_MSTR_ADDR0_REG           (SYSCTRL_BASE + 0x6844) /* SC_AXI_MSTR_ADDR0为SYS_CTRL内AXI Master的访问地址bit[31:0] */
#define SYSCTRL_SC_AXI_MSTR_DATA3_REG           (SYSCTRL_BASE + 0x6848) /* SC_AXI_MSTR_DATA3为SYS_CTRL内AXI Master的访问数据bit[127:96] */
#define SYSCTRL_SC_AXI_MSTR_DATA2_REG           (SYSCTRL_BASE + 0x684C) /* SC_AXI_MSTR_DATA2为SYS_CTRL内AXI Master的访问数据bit[95:64] */
#define SYSCTRL_SC_AXI_MSTR_DATA1_REG           (SYSCTRL_BASE + 0x6850) /* SC_AXI_MSTR_DATA1为SYS_CTRL内AXI Master的访问数据bit[63:32] */
#define SYSCTRL_SC_AXI_MSTR_DATA0_REG           (SYSCTRL_BASE + 0x6854) /* SC_AXI_MSTR_DATA0为SYS_CTRL内AXI Master的访问数据bit[31:0] */
#define SYSCTRL_SC_CTRL_HC_HHA_REG              (SYSCTRL_BASE + 0x8400) /* HHA HC memory控制寄存器 */
#define SYSCTRL_SC_CTRL_RASHDE_HHA_REG          (SYSCTRL_BASE + 0x840C) /* HHA HDE RAS memory控制寄存器 */
#define SYSCTRL_SC_CTRL_HC_LLC_REG              (SYSCTRL_BASE + 0x8410) /* LLC HC memory控制寄存器 */
#define SYSCTRL_SC_CTRL_RASHDE_LLC_REG          (SYSCTRL_BASE + 0x8414) /* LLC HDE RAS memory控制寄存器 */
#define SYSCTRL_SC_HS_MEM_ADJUST_HC_REG         (SYSCTRL_BASE + 0x8418) /* CPU HC memory 控制寄存器 */
#define SYSCTRL_SC_HS_MEM_ADJUST_REG            (SYSCTRL_BASE + 0x841C) /* CPU RFS memory 控制寄存器 */
#define SYSCTRL_SC_CTRL_RASHDE_CSYS_REG         (SYSCTRL_BASE + 0x8420) /* Coresight HDE RAS memory控制寄存器 */
#define SYSCTRL_SC_ECO_RSV0_REG                 (SYSCTRL_BASE + 0x8800) /* ECO 寄存器0 */
#define SYSCTRL_SC_ECO_RSV1_REG                 (SYSCTRL_BASE + 0x8804) /* ECO 寄存器1 */
#define SYSCTRL_SC_ECO_RSV2_REG                 (SYSCTRL_BASE + 0x8810) /* ECO 寄存器2 */
#define SYSCTRL_SC_CPU_PMUSNAPSHOTACK_ST_REG    (SYSCTRL_BASE + 0xA000) /* 对应cluster的PMU snapshot trigger acknowledge */
#define SYSCTRL_SC_CPU_SMPEN_ST_REG             (SYSCTRL_BASE + 0xA004) /* 对应cluster的Cortex-A57 CPU core处于smp模式指示。 */
#define SYSCTRL_SC_CPU_STANDBYWFE_ST_REG        (SYSCTRL_BASE + 0xA008) /* 对应cluster的Cortex-A57 CPU core处于WFE待命模式指示。 */
#define SYSCTRL_SC_CPU_STANDBYWFI_ST_REG        (SYSCTRL_BASE + 0xA00C) /* 对应cluster的Cortex-A57 CPU core处于WFI待命模式指示。 */
#define SYSCTRL_SC_CPU_DBGACK_ST_REG            (SYSCTRL_BASE + 0xA010) /* 对应cluster的Cortex-A57 CPU core debug请求响应指示。 */
#define SYSCTRL_SC_CPU_CLREXMONACK_ST_REG       (SYSCTRL_BASE + 0xA014) /* 对应cluster的clearing of the external global exclusive monitor acknowledge. */
#define SYSCTRL_SC_CPU_STANDBYWFIL2_ST_REG      (SYSCTRL_BASE + 0xA018) /* 对应cluster的L2处于WFI待命模式指示。 */
#define SYSCTRL_SC_CPU_L2FSHDONE_ST_REG         (SYSCTRL_BASE + 0xA01C) /* 对应cluster的L2 hardware flush request指示。 */
#define SYSCTRL_SC_SYSMODE_STATUS_REG           (SYSCTRL_BASE + 0xC000) /* SC_CTRL为系统时钟工作状态寄存器 */
#define SYSCTRL_SC_PLL_LOCK_STATUS_REG          (SYSCTRL_BASE + 0xC004) /* PLL锁定状态寄存器。 */
#define SYSCTRL_SC_PLLCTRL_ST_REG               (SYSCTRL_BASE + 0xC008) /* SC_PLLCTRL_ST为PLL状态寄存器。 */
#define SYSCTRL_SC_REF_CLK_ST_REG               (SYSCTRL_BASE + 0xC00C) /* SC_REF_CLK_ST表示CDIE_CRG内ref_clk的时钟状态。 */
#define SYSCTRL_SC_GPIO_CLK_ST_REG              (SYSCTRL_BASE + 0xC010) /* SC_GPIO_CLK_ST表示CDIE_CRG内gpio_db_clk的时钟状态。 */
#define SYSCTRL_SC_CLUS0_CRG_CLK_ST_REG         (SYSCTRL_BASE + 0xC014) /* SC_CLUS0_CRG_CLK_ST表示当前CLUSTER内CRG的时钟状态。 */
#define SYSCTRL_SC_CLUS0_CRG_RESET_ST_REG       (SYSCTRL_BASE + 0xC018) /* SC_CLUS0_CRG_RESET_ST表示当前CLUSTER内RESET的状态。 */
#define SYSCTRL_SC_CLUS1_CRG_CLK_ST_REG         (SYSCTRL_BASE + 0xC01C) /* SC_CLUS1_CRG_CLK_ST表示当前CLUSTER内CRG的时钟状态。 */
#define SYSCTRL_SC_CLUS1_CRG_RESET_ST_REG       (SYSCTRL_BASE + 0xC020) /* SC_CLUS1_CRG_RESET_ST表示当前CLUSTER内RESET的状态。 */
#define SYSCTRL_SC_CLUS2_CRG_CLK_ST_REG         (SYSCTRL_BASE + 0xC024) /* SC_CLUS2_CRG_CLK_ST表示当前CLUSTER内CRG的时钟状态。 */
#define SYSCTRL_SC_CLUS2_CRG_RESET_ST_REG       (SYSCTRL_BASE + 0xC028) /* SC_CLUS2_CRG_RESET_ST表示当前CLUSTER内RESET的状态。 */
#define SYSCTRL_SC_CLUS3_CRG_CLK_ST_REG         (SYSCTRL_BASE + 0xC02C) /* SC_CLUS3_CRG_CLK_ST表示当前CLUSTER内CRG的时钟状态。 */
#define SYSCTRL_SC_CLUS3_CRG_RESET_ST_REG       (SYSCTRL_BASE + 0xC030) /* SC_CLUS3_CRG_RESET_ST表示当前CLUSTER内RESET的状态。 */
#define SYSCTRL_SC_LLC_CLK_ST_REG               (SYSCTRL_BASE + 0xC034) /* SC_LLC_CLK_ST为LLC的时钟状态寄存器 */
#define SYSCTRL_SC_LLC_RESET_ST_REG             (SYSCTRL_BASE + 0xC038) /* SC_LLC_RESET_ST为CDIE内LLC软复位状态寄存器 */
#define SYSCTRL_SC_DDRC_CLK_ST_REG              (SYSCTRL_BASE + 0xC03C) /* SC_DDRC_CLK_ST为DDRC的时钟状态寄存器 */
#define SYSCTRL_SC_DDRC_RESET_ST_REG            (SYSCTRL_BASE + 0xC040) /* SC_DDRC_RESET_ST为CDIE内DDRC软复位状态寄存器 */
#define SYSCTRL_SC_SLLC_CLK_ST_REG              (SYSCTRL_BASE + 0xC100) /* SC_SLLC_CLK_ST表示SLLC的当前时钟状态 */
#define SYSCTRL_SC_SLLC_RESET_ST_REG            (SYSCTRL_BASE + 0xC104) /* SC_SLLC_RESET_STAT表示SLLC的当前软复位状态 */
#define SYSCTRL_SC_SLLC_TSVRX_RESET_ST_REG      (SYSCTRL_BASE + 0xC108) /* SC_SLLC_TSVRX_RESET_STAT为SLLC TSVRX通道软复位状态寄存器 */
#define SYSCTRL_SC_HPM_CLK_ST_REG               (SYSCTRL_BASE + 0xC110) /* SC_HPM_CLK_ST表示HPM当前时钟状态 */
#define SYSCTRL_SC_HPM_SRST_ST_REG              (SYSCTRL_BASE + 0xC114) /* SC_HPM_SRST_ST表示HPM当前软复位状态状态 */
#define SYSCTRL_SC_DJTAG_CLK_ST_REG             (SYSCTRL_BASE + 0xC120) /* SC_DJTAG_CLK_ST表示DJTAG相关逻辑时钟状态 */
#define SYSCTRL_SC_DJTAG_SRST_ST_REG            (SYSCTRL_BASE + 0xC124) /* SC_DJTAG_SRST_ST表示DJTAG相关逻辑复位状态 */
#define SYSCTRL_SC_HHA_MN_CLK_ST_REG            (SYSCTRL_BASE + 0xC128) /* SC_HHA_MN_CLK_ST表示HHA和MN逻辑时钟状态 */
#define SYSCTRL_SC_HHA_MN_SRST_ST_REG           (SYSCTRL_BASE + 0xC12C) /* SC_HHA_MN_SRST_ST表示HHA和MN逻辑复位状态 */
#define SYSCTRL_SC_DDRC_WARM_RST_ACKED_REG      (SYSCTRL_BASE + 0xC160) /* SC_DDRC_WARM_RST_ACKED表示CRG检测到非上电的系统复位的warn_rst_req请求的DDRC握手信号 */
#define SYSCTRL_SC_TSENSOR_SYSSTAT_REG          (SYSCTRL_BASE + 0xD000) /* tsensor系统状态寄存器。 */
#define SYSCTRL_SC_TSENSOR_TEMP_SAMPLE_REG      (SYSCTRL_BASE + 0xD004) /* 芯片温度采样值 */
#define SYSCTRL_SC_MDDRC0_AC_IOCTL_IOTEST0_REG  (SYSCTRL_BASE + 0xD400)
#define SYSCTRL_SC_MDDRC0_AC_IOCTL_IOTEST1_REG  (SYSCTRL_BASE + 0xD404)
#define SYSCTRL_SC_MDDRC1_AC_IOCTL_IOTEST0_REG  (SYSCTRL_BASE + 0xD408)
#define SYSCTRL_SC_MDDRC1_AC_IOCTL_IOTEST1_REG  (SYSCTRL_BASE + 0xD40C)
#define SYSCTRL_SC_MDDRC0_DX_IOCTL_IOTEST0_REG  (SYSCTRL_BASE + 0xD410)
#define SYSCTRL_SC_MDDRC0_DX_IOCTL_IOTEST1_REG  (SYSCTRL_BASE + 0xD414)
#define SYSCTRL_SC_MDDRC0_DX_IOCTL_IOTEST2_REG  (SYSCTRL_BASE + 0xD418)
#define SYSCTRL_SC_MDDRC0_DX_IOCTL_IOTEST3_REG  (SYSCTRL_BASE + 0xD41C)
#define SYSCTRL_SC_MDDRC1_DX_IOCTL_IOTEST0_REG  (SYSCTRL_BASE + 0xD420)
#define SYSCTRL_SC_MDDRC1_DX_IOCTL_IOTEST1_REG  (SYSCTRL_BASE + 0xD424)
#define SYSCTRL_SC_MDDRC1_DX_IOCTL_IOTEST2_REG  (SYSCTRL_BASE + 0xD428)
#define SYSCTRL_SC_MDDRC1_DX_IOCTL_IOTEST3_REG  (SYSCTRL_BASE + 0xD42C)
#define SYSCTRL_SC_MBIST_CPUI_DATAOUT_REG       (SYSCTRL_BASE + 0xDC00) /* DFT MBIST模式下读数据寄存器 */
#define SYSCTRL_SC_INT_STATUS_REG               (SYSCTRL_BASE + 0xE000) /* 系统控制器中断状态寄存器 */
#define SYSCTRL_SC_DIE_ID0_REG                  (SYSCTRL_BASE + 0xE004) /* DIE ID 寄存器bit[31:0]。 */
#define SYSCTRL_SC_DIE_ID1_REG                  (SYSCTRL_BASE + 0xE008) /* DIE ID 寄存器bit[63:32]。 */
#define SYSCTRL_SC_DIE_ID2_REG                  (SYSCTRL_BASE + 0xE00C) /* DIE ID 寄存器bit[95:64]。 */
#define SYSCTRL_SC_DIE_ID3_REG                  (SYSCTRL_BASE + 0xE010) /* DIE ID 寄存器bit[127:96]。 */
#define SYSCTRL_SC_DIE_ID4_REG                  (SYSCTRL_BASE + 0xE014) /* DIE ID 寄存器bit[159:128]。 */
#define SYSCTRL_SC_DIE_ID5_REG                  (SYSCTRL_BASE + 0xE018) /* DIE ID 寄存器bit[191:160]。 */
#define SYSCTRL_SC_DIE_ID6_REG                  (SYSCTRL_BASE + 0xE01C) /* DIE ID 寄存器bit[223:192]。 */
#define SYSCTRL_SC_DIE_ID7_REG                  (SYSCTRL_BASE + 0xE020) /* DIE ID 寄存器bit[255:224]。 */
#define SYSCTRL_SC_DIE_ID8_REG                  (SYSCTRL_BASE + 0xE024) /* DIE ID 寄存器bit[287:256]。 */
#define SYSCTRL_SC_DIE_ID9_REG                  (SYSCTRL_BASE + 0xE028) /* DIE ID 寄存器bit[319:288]。 */
#define SYSCTRL_SC_DIE_ID10_REG                 (SYSCTRL_BASE + 0xE02C) /* DIE ID 寄存器bit[351:320]。 */
#define SYSCTRL_SC_DIE_ID11_REG                 (SYSCTRL_BASE + 0xE030) /* DIE ID 寄存器bit[383:352]。 */
#define SYSCTRL_SC_DIE_ID12_REG                 (SYSCTRL_BASE + 0xE034) /* DIE ID 寄存器bit[415:384]。 */
#define SYSCTRL_SC_DIE_ID13_REG                 (SYSCTRL_BASE + 0xE038) /* DIE ID 寄存器bit[447:416]。 */
#define SYSCTRL_SC_DIE_ID14_REG                 (SYSCTRL_BASE + 0xE03C) /* DIE ID 寄存器bit[479:448]。 */
#define SYSCTRL_SC_DIE_ID15_REG                 (SYSCTRL_BASE + 0xE040) /* DIE ID 寄存器bit[511:480]。 */
#define SYSCTRL_SC_DIE_ID16_REG                 (SYSCTRL_BASE + 0xE044) /* DIE ID 寄存器bit[543:512]。 */
#define SYSCTRL_SC_DIE_ID17_REG                 (SYSCTRL_BASE + 0xE048) /* DIE ID 寄存器bit[575:544]。 */
#define SYSCTRL_SC_DIE_ID18_REG                 (SYSCTRL_BASE + 0xE04C) /* DIE ID 寄存器bit[607:576]。 */
#define SYSCTRL_SC_DIE_ID19_REG                 (SYSCTRL_BASE + 0xE050) /* DIE ID 寄存器bit[639:608]。 */
#define SYSCTRL_SC_SOCKET_ID_REG                (SYSCTRL_BASE + 0xE080) /* SC_SOCKET_ID表示socket的管脚指示信号 */
#define SYSCTRL_SC_BOOT_CTRL_ST_REG             (SYSCTRL_BASE + 0xE084) /* SC_BOOT_CTRL_ST表示CPU启动控制的PAD和eFuse信号状态 */
#define SYSCTRL_SC_BOOT_CTRL_CFG_ST_REG         (SYSCTRL_BASE + 0xE088) /* SC_BOOT_CTRL_CFG_ST表示CPU启动控制信号经过配置后最终状态 */
#define SYSCTRL_SC_LAST_RST_STATUS_REG          (SYSCTRL_BASE + 0xE090) /* SC_LAST_RST_STATUS表示芯片最后一次复位的复位状态 */
#define SYSCTRL_SC_XTAL_ST_REG                  (SYSCTRL_BASE + 0xE100)
#define SYSCTRL_SC_CNT_ST_REG                   (SYSCTRL_BASE + 0xE104)
#define SYSCTRL_SC_ITIR0_TEST_REG               (SYSCTRL_BASE + 0xE108)
#define SYSCTRL_SC_ITOR_TEST_REG                (SYSCTRL_BASE + 0xE10C)
#define SYSCTRL_SC_CNT_DATA_REG                 (SYSCTRL_BASE + 0xE110)
#define SYSCTRL_SC_CNT_STEP_RSV_REG             (SYSCTRL_BASE + 0xE114)
#define SYSCTRL_SC_AXI_ACC_ST_REG               (SYSCTRL_BASE + 0xE200)
#define SYSCTRL_SC_SYSCNT_SYNC_OFFSET_REG       (SYSCTRL_BASE + 0xE204) /* System Counter同步功能统计的路径延迟寄存器 */
#define SYSCTRL_SC_DJTAG_RD_DATA0_REG           (SYSCTRL_BASE + 0xE800) /* DJTAG扫描链上第一个模块读数据寄存器 */
#define SYSCTRL_SC_DJTAG_RD_DATA1_REG           (SYSCTRL_BASE + 0xE804) /* DJTAG扫描链上第二个模块读数据寄存器 */
#define SYSCTRL_SC_DJTAG_RD_DATA2_REG           (SYSCTRL_BASE + 0xE808) /* DJTAG扫描链上第三个模块读数据寄存器 */
#define SYSCTRL_SC_DJTAG_RD_DATA3_REG           (SYSCTRL_BASE + 0xE80C) /* DJTAG扫描链上第四个模块读数据寄存器 */
#define SYSCTRL_SC_DJTAG_RD_DATA4_REG           (SYSCTRL_BASE + 0xE810) /* DJTAG扫描链上第五个模块读数据寄存器 */
#define SYSCTRL_SC_DJTAG_RD_DATA5_REG           (SYSCTRL_BASE + 0xE814) /* DJTAG扫描链上第六个模块读数据寄存器 */
#define SYSCTRL_SC_DJTAG_RD_DATA6_REG           (SYSCTRL_BASE + 0xE818) /* DJTAG扫描链上第七个模块读数据寄存器 */
#define SYSCTRL_SC_DJTAG_RD_DATA7_REG           (SYSCTRL_BASE + 0xE81C) /* DJTAG扫描链上第八个模块读数据寄存器 */
#define SYSCTRL_SC_DJTAG_RD_DATA8_REG           (SYSCTRL_BASE + 0xE820) /* DJTAG扫描链上第九个模块读数据寄存器 */
#define SYSCTRL_SC_DJTAG_RD_DATA9_REG           (SYSCTRL_BASE + 0xE824) /* DJTAG扫描链上第十个模块读数据寄存器 */
#define SYSCTRL_SC_DJTAG_OP_ST_REG              (SYSCTRL_BASE + 0xE828) /* DJTAG访问状态寄存器 */
#define SYSCTRL_SC_TDRE_RDATA1_REG              (SYSCTRL_BASE + 0xE82C) /* DJTAG访问TDRE模块的读数据高32bit */
#define SYSCTRL_SC_TDRE_RDATA0_REG              (SYSCTRL_BASE + 0xE830) /* DJTAG访问TDRE模块的读数据低32bit */
#define SYSCTRL_SC_AXI_RD_DATA3_REG             (SYSCTRL_BASE + 0xE840) /* AXI访问DDR的读数据寄存器 */
#define SYSCTRL_SC_AXI_RD_DATA2_REG             (SYSCTRL_BASE + 0xE844) /* AXI访问DDR的读数据寄存器 */
#define SYSCTRL_SC_AXI_RD_DATA1_REG             (SYSCTRL_BASE + 0xE848) /* AXI访问DDR的读数据寄存器 */
#define SYSCTRL_SC_AXI_RD_DATA0_REG             (SYSCTRL_BASE + 0xE84C) /* AXI访问DDR的读数据寄存器 */
#define SYSCTRL_SC_AXI_MSTR_ST_REG              (SYSCTRL_BASE + 0xE850) /* AXI访问DDR返回状态寄存器 */
#define SYSCTRL_SC_BOARD_CFG_INFO_REG           (SYSCTRL_BASE + 0xF000) /* 单板配置信息状态寄存器 */


/******************************************************************************/
/*                      PHOSPHOR POE_SUB Registers' Definitions                            */
/******************************************************************************/

#define POE_SUB_SC_SRAM_CLK_EN_REG            (POE_SUB_BASE + 0x340)  /* SRAM时钟使能寄存器 */
#define POE_SUB_SC_SRAM_CLK_DIS_REG           (POE_SUB_BASE + 0x344)  /* SRAM时钟禁止寄存器 */
#define POE_SUB_SC_POE_CLK_EN_REG             (POE_SUB_BASE + 0x500)  /* POE时钟使能寄存器 */
#define POE_SUB_SC_POE_CLK_DIS_REG            (POE_SUB_BASE + 0x504)  /* POE时钟禁止寄存器 */
#define POE_SUB_SC_SRAM_RESET_REQ_REG         (POE_SUB_BASE + 0xA40)  /* SRAM软复位请求控制寄存器 */
#define POE_SUB_SC_SRAM_RESET_DREQ_REG        (POE_SUB_BASE + 0xA44)  /* SRAM软复位去请求控制寄存器 */
#define POE_SUB_SC_POE_RESET_REQ_REG          (POE_SUB_BASE + 0xB18)  /* poe软复位请求控制寄存器 */
#define POE_SUB_SC_POE_RESET_DREQ_REG         (POE_SUB_BASE + 0xB1C)  /* poe软复位去请求控制寄存器 */
#define POE_SUB_SC_DISPATCH_DAW_EN_REG        (POE_SUB_BASE + 0x1000) /* dispatch daw en配置 */
#define POE_SUB_SC_DISPATCH_DAW_ARRAY0_REG    (POE_SUB_BASE + 0x1004) /* dispatch daw配置阵列0 */
#define POE_SUB_SC_DISPATCH_DAW_ARRAY1_REG    (POE_SUB_BASE + 0x1008) /* dispatch daw配置阵列1 */
#define POE_SUB_SC_DISPATCH_DAW_ARRAY2_REG    (POE_SUB_BASE + 0x100C) /* dispatch daw配置阵列2 */
#define POE_SUB_SC_DISPATCH_DAW_ARRAY3_REG    (POE_SUB_BASE + 0x1010) /* dispatch daw配置阵列3 */
#define POE_SUB_SC_DISPATCH_DAW_ARRAY4_REG    (POE_SUB_BASE + 0x1014) /* dispatch daw配置阵列4 */
#define POE_SUB_SC_DISPATCH_DAW_ARRAY5_REG    (POE_SUB_BASE + 0x1018) /* dispatch daw配置阵列5 */
#define POE_SUB_SC_DISPATCH_DAW_ARRAY6_REG    (POE_SUB_BASE + 0x101C) /* dispatch daw配置阵列6 */
#define POE_SUB_SC_DISPATCH_DAW_ARRAY7_REG    (POE_SUB_BASE + 0x1020) /* dispatch daw配置阵列7 */
#define POE_SUB_SC_DISPATCH_RETRY_CONTROL_REG (POE_SUB_BASE + 0x1030) /* dispatch retry控制寄存器 */
#define POE_SUB_SC_DISPATCH_INTMASK_REG       (POE_SUB_BASE + 0x1100) /* dispatch的中断屏蔽寄存器 */
#define POE_SUB_SC_DISPATCH_RAWINT_REG        (POE_SUB_BASE + 0x1104) /* dispatch的原始中断状态寄存器 */
#define POE_SUB_SC_DISPATCH_INTSTAT_REG       (POE_SUB_BASE + 0x1108) /* dispatch的屏蔽后的中断状态寄存器 */
#define POE_SUB_SC_DISPATCH_INTCLR_REG        (POE_SUB_BASE + 0x110C) /* dispatch的中断清除寄存器 */
#define POE_SUB_SC_DISPATCH_ERRSTAT_REG       (POE_SUB_BASE + 0x1110) /* dispatch的ERR状态寄存器 */
#define POE_SUB_SC_DISPATCH_REMAP_CTRL_REG    (POE_SUB_BASE + 0x1200) /* subsys的启动Remap寄存器 */
#define POE_SUB_SC_SRAM_CTRL0_REG             (POE_SUB_BASE + 0x2030) /* sram控制寄存器0 */
#define POE_SUB_SC_SRAM_CTRL1_REG             (POE_SUB_BASE + 0x2034) /* sram控制寄存器1（安全寄存器） */
#define POE_SUB_SC_SRAM_CTRL2_REG             (POE_SUB_BASE + 0x2038) /* sram控制寄存器2（安全寄存器） */
#define POE_SUB_SC_SRAM_CTRL3_REG             (POE_SUB_BASE + 0x203C) /* sram控制寄存器3（安全寄存器） */
#define POE_SUB_SC_SMMU_MEM_CTRL1_REG         (POE_SUB_BASE + 0x3004) /* smmu mem控制寄存器1 */
#define POE_SUB_SC_SMMU_MEM_CTRL2_REG         (POE_SUB_BASE + 0x3008) /* smmu mem控制寄存器2 */
#define POE_SUB_SC_SRAM_MEM_CTRL_REG          (POE_SUB_BASE + 0x3010) /* sram mem控制寄存器 */
#define POE_SUB_SC_PLLFCTRL0_REG              (POE_SUB_BASE + 0x3014) /* SC_PLLFCTRL0为CIDE_CRG的PLL0控制寄存器0。 */
#define POE_SUB_SC_PLLFCTRL1_REG              (POE_SUB_BASE + 0x3018) /* SC_PLLFCTRL1为CIDE_CRG的PLL0控制寄存器1。 */
#define POE_SUB_SC_PLLFCTRL8_REG              (POE_SUB_BASE + 0x3034) /* SC_PLLFCTRL8为CIDE_CRG的PLL4控制寄存器0。 */
#define POE_SUB_SC_PLLFCTRL9_REG              (POE_SUB_BASE + 0x3038) /* SC_PLLFCTRL9为CIDE_CRG的PLL4控制寄存器1。 */
#define POE_SUB_SC_PLL_CLK_BYPASS0_REG        (POE_SUB_BASE + 0x3040) /* SC_PLL_CLK_BYPASS0为CDIE_CRG的PLL相关工作时钟选择。 */
#define POE_SUB_SC_PLL_CLK_BYPASS1_REG        (POE_SUB_BASE + 0x3044) /* SC_PLL_CLK_BYPASS1为CDIE_CRG的PLL相关工作时钟选择。 */
#define POE_SUB_SC_CLK_SEL_REG                (POE_SUB_BASE + 0x3048) /* SC_CLK_SEL是CDIE_CRG的处理器和系统互联时钟频率选择寄存器。 */
#define POE_SUB_SC_TSMC_PLLCTRL1_REG          (POE_SUB_BASE + 0x3204) /* SC_TSMC_PLLCTRL1为CIDE和PLL1对应的TSMC备份PLL的控制寄存器。 */
#define POE_SUB_SC_TSMC_PLLCTRL4_REG          (POE_SUB_BASE + 0x3210) /* SC_TSMC_PLLCTRL4为CIDE CRG的PLL4对应的TSMC备份PLL的控制寄存器。 */
#define POE_SUB_SC_PLL_SEL_REG                (POE_SUB_BASE + 0x3214) /* SC_PLL_SEL为CIDE内的PLL选择控制寄存器。 */
#define POE_SUB_SC_SRAM_CLK_ST_REG            (POE_SUB_BASE + 0x5320) /* SRAM时钟状态寄存器 */
#define POE_SUB_SC_POE_CLK_ST_REG             (POE_SUB_BASE + 0x5400) /* POE时钟状态寄存器 */
#define POE_SUB_SC_SRAM_RESET_ST_REG          (POE_SUB_BASE + 0x5A20) /* SRAM复位状态寄存器 */
#define POE_SUB_SC_POE_RESET_ST_REG           (POE_SUB_BASE + 0x5B00) /* poe复位状态寄存器 */
#define POE_SUB_SC_SRAM_ST0_REG               (POE_SUB_BASE + 0x6000) /* sram状态寄存器0 */
#define POE_SUB_SC_SRAM_ST1_REG               (POE_SUB_BASE + 0x6004) /* sram状态寄存器1 */
#define POE_SUB_SC_SRAM_ST2_REG               (POE_SUB_BASE + 0x6008) /* sram状态寄存器2 */
#define POE_SUB_SC_SRAM_ST3_REG               (POE_SUB_BASE + 0x600C) /* sram状态寄存器3 */
#define POE_SUB_SC_SRAM_ST4_REG               (POE_SUB_BASE + 0x6010) /* sram状态寄存器4 */
#define POE_SUB_SC_SRAM_ST5_REG               (POE_SUB_BASE + 0x6014) /* sram状态寄存器5 */
#define POE_SUB_SC_ECO_RSV0_REG               (POE_SUB_BASE + 0x8000) /* ECO 寄存器0 */
#define POE_SUB_SC_ECO_RSV1_REG               (POE_SUB_BASE + 0x8004) /* ECO 寄存器1 */
#define POE_SUB_SC_ECO_RSV2_REG               (POE_SUB_BASE + 0x8008) /* ECO 寄存器2 */





/* Define the union U_SC_CPU_EDBGRQ_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    edbgrq_cluster0       : 4   ; /* [3..0]  */
        unsigned int    edbgrq_cluster1       : 4   ; /* [7..4]  */
        unsigned int    edbgrq_cluster2       : 4   ; /* [11..8]  */
        unsigned int    edbgrq_cluster3       : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_EDBGRQ_CTRL;

/* Define the union U_SC_CPU_CP15SDIS_CRTL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cp15sdisable_cluster0 : 4   ; /* [3..0]  */
        unsigned int    cp15sdisable_cluster1 : 4   ; /* [7..4]  */
        unsigned int    cp15sdisable_cluster2 : 4   ; /* [11..8]  */
        unsigned int    cp15sdisable_cluster3 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_CP15SDIS_CRTL;

/* Define the union U_SC_CPU_PMU_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pmusnapshotreq_cluster0 : 4   ; /* [3..0]  */
        unsigned int    pmusnapshotreq_cluster1 : 4   ; /* [7..4]  */
        unsigned int    pmusnapshotreq_cluster2 : 4   ; /* [11..8]  */
        unsigned int    pmusnapshotreq_cluster3 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_PMU_CTRL;

/* Define the union U_SC_CLUS_L2_CLKEN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster0_clken        : 1   ; /* [0]  */
        unsigned int    cluster1_clken        : 1   ; /* [1]  */
        unsigned int    cluster2_clken        : 1   ; /* [2]  */
        unsigned int    cluster3_clken        : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS_L2_CLKEN;

/* Define the union U_SC_CPU_GICDIS_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    giccdisable_cluster0  : 1   ; /* [0]  */
        unsigned int    giccdisable_cluster1  : 1   ; /* [1]  */
        unsigned int    giccdisable_cluster2  : 1   ; /* [2]  */
        unsigned int    giccdisable_cluster3  : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_GICDIS_CTRL;

/* Define the union U_SC_CPU_CLKEN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpu0_cluster0_clken   : 1   ; /* [0]  */
        unsigned int    cpu1_cluster0_clken   : 1   ; /* [1]  */
        unsigned int    cpu2_cluster0_clken   : 1   ; /* [2]  */
        unsigned int    cpu3_cluster0_clken   : 1   ; /* [3]  */
        unsigned int    cpu0_cluster1_clken   : 1   ; /* [4]  */
        unsigned int    cpu1_cluster1_clken   : 1   ; /* [5]  */
        unsigned int    cpu2_cluster1_clken   : 1   ; /* [6]  */
        unsigned int    cpu3_cluster1_clken   : 1   ; /* [7]  */
        unsigned int    cpu0_cluster2_clken   : 1   ; /* [8]  */
        unsigned int    cpu1_cluster2_clken   : 1   ; /* [9]  */
        unsigned int    cpu2_cluster2_clken   : 1   ; /* [10]  */
        unsigned int    cpu3_cluster2_clken   : 1   ; /* [11]  */
        unsigned int    cpu0_cluster3_clken   : 1   ; /* [12]  */
        unsigned int    cpu1_cluster3_clken   : 1   ; /* [13]  */
        unsigned int    cpu2_cluster3_clken   : 1   ; /* [14]  */
        unsigned int    cpu3_cluster3_clken   : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_CLKEN;

/* Define the union U_SC_CPU_L2FSHREQ_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    l2flushreq_cluster0   : 1   ; /* [0]  */
        unsigned int    l2flushreq_cluster1   : 1   ; /* [1]  */
        unsigned int    l2flushreq_cluster2   : 1   ; /* [2]  */
        unsigned int    l2flushreq_cluster3   : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_L2FSHREQ_CTRL;

/* Define the union U_SC_CPU_CLREXMONREQ_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clrexmonreq_cluster0  : 1   ; /* [0]  */
        unsigned int    clrexmonreq_cluster1  : 1   ; /* [1]  */
        unsigned int    clrexmonreq_cluster2  : 1   ; /* [2]  */
        unsigned int    clrexmonreq_cluster3  : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_CLREXMONREQ_CTRL;

/* Define the union U_SC_CPU_SYSBROADCAST_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sysbardisable_cluster0 : 1   ; /* [0]  */
        unsigned int    broadcastinner_cluster0 : 1   ; /* [1]  */
        unsigned int    broadcastouter_cluster0 : 1   ; /* [2]  */
        unsigned int    broadcastcachemaint_cluster0 : 1   ; /* [3]  */
        unsigned int    sysbardisable_cluster1 : 1   ; /* [4]  */
        unsigned int    broadcastinner_cluster1 : 1   ; /* [5]  */
        unsigned int    broadcastouter_cluster1 : 1   ; /* [6]  */
        unsigned int    broadcastcachemaint_cluster1 : 1   ; /* [7]  */
        unsigned int    sysbardisable_cluster2 : 1   ; /* [8]  */
        unsigned int    broadcastinner_cluster2 : 1   ; /* [9]  */
        unsigned int    broadcastouter_cluster2 : 1   ; /* [10]  */
        unsigned int    broadcastcachemaint_cluster2 : 1   ; /* [11]  */
        unsigned int    sysbardisable_cluster3 : 1   ; /* [12]  */
        unsigned int    broadcastinner_cluster3 : 1   ; /* [13]  */
        unsigned int    broadcastouter_cluster3 : 1   ; /* [14]  */
        unsigned int    broadcastcachemaint_cluster3 : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_SYSBROADCAST_CTRL;

/* Define the union U_SC_CPU_SINACT_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sinact_cluster0       : 1   ; /* [0]  */
        unsigned int    sinact_cluster1       : 1   ; /* [1]  */
        unsigned int    sinact_cluster2       : 1   ; /* [2]  */
        unsigned int    sinact_cluster3       : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_SINACT_CTRL;

/* Define the union U_SC_CLUS0_CTRL_RVBA0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr0_cluster0   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS0_CTRL_RVBA0;

/* Define the union U_SC_CLUS0_CTRL_RVBA1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr1_cluster0   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS0_CTRL_RVBA1;

/* Define the union U_SC_CLUS0_CTRL_RVBA2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr2_cluster0   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS0_CTRL_RVBA2;

/* Define the union U_SC_CLUS0_CTRL_RVBA3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr3_cluster0   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS0_CTRL_RVBA3;

/* Define the union U_SC_CLUS1_CTRL_RVBA0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr0_cluster1   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS1_CTRL_RVBA0;

/* Define the union U_SC_CLUS1_CTRL_RVBA1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr1_cluster1   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS1_CTRL_RVBA1;

/* Define the union U_SC_CLUS1_CTRL_RVBA2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr2_cluster1   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS1_CTRL_RVBA2;

/* Define the union U_SC_CLUS1_CTRL_RVBA3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr3_cluster1   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS1_CTRL_RVBA3;

/* Define the union U_SC_CLUS2_CTRL_RVBA0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr0_cluster2   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS2_CTRL_RVBA0;

/* Define the union U_SC_CLUS2_CTRL_RVBA1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr1_cluster2   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS2_CTRL_RVBA1;

/* Define the union U_SC_CLUS2_CTRL_RVBA2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr2_cluster2   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS2_CTRL_RVBA2;

/* Define the union U_SC_CLUS2_CTRL_RVBA3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr3_cluster2   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS2_CTRL_RVBA3;

/* Define the union U_SC_CLUS3_CTRL_RVBA0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr0_cluster3   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS3_CTRL_RVBA0;

/* Define the union U_SC_CLUS3_CTRL_RVBA1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr1_cluster3   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS3_CTRL_RVBA1;

/* Define the union U_SC_CLUS3_CTRL_RVBA2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr2_cluster3   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS3_CTRL_RVBA2;

/* Define the union U_SC_CLUS3_CTRL_RVBA3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rvbaraddr3_cluster3   : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS3_CTRL_RVBA3;

/* Define the union U_SC_PERIPHBASE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    periph_base           : 26  ; /* [25..0]  */
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PERIPHBASE;

/* Define the union U_SC_DBG_AUTH_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpu_dbgen             : 1   ; /* [0]  */
        unsigned int    cpu_niden             : 1   ; /* [1]  */
        unsigned int    cpu_spiden            : 1   ; /* [2]  */
        unsigned int    cpu_spniden           : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_AUTH_CTRL;

/* Define the union U_SC_CLUS_CRYPTODISABLE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster0_cryptodisable : 4   ; /* [3..0]  */
        unsigned int    cluster1_cryptodisable : 4   ; /* [7..4]  */
        unsigned int    cluster2_cryptodisable : 4   ; /* [11..8]  */
        unsigned int    cluster3_cryptodisable : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS_CRYPTODISABLE;

/* Define the union U_SC_CORE0_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core0_event_en        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE0_EVENT_EN;

/* Define the union U_SC_CORE1_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core1_event_en        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE1_EVENT_EN;

/* Define the union U_SC_CORE2_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core2_event_en        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE2_EVENT_EN;

/* Define the union U_SC_CORE3_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core3_event_en        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE3_EVENT_EN;

/* Define the union U_SC_CORE4_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core4_event_en        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE4_EVENT_EN;

/* Define the union U_SC_CORE5_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core5_event_en        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE5_EVENT_EN;

/* Define the union U_SC_CORE6_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core6_event_en        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE6_EVENT_EN;

/* Define the union U_SC_CORE7_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core7_event_en        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE7_EVENT_EN;

/* Define the union U_SC_CORE8_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core8_event_en        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE8_EVENT_EN;

/* Define the union U_SC_CORE9_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core9_event_en        : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE9_EVENT_EN;

/* Define the union U_SC_CORE10_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core10_event_en       : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE10_EVENT_EN;

/* Define the union U_SC_CORE11_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core11_event_en       : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE11_EVENT_EN;

/* Define the union U_SC_CORE12_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core12_event_en       : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE12_EVENT_EN;

/* Define the union U_SC_CORE13_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core13_event_en       : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE13_EVENT_EN;

/* Define the union U_SC_CORE14_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core14_event_en       : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE14_EVENT_EN;

/* Define the union U_SC_CORE15_EVENT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    core15_event_en       : 17  ; /* [16..0]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CORE15_EVENT_EN;

/* Define the union U_SC_CPU_DBGL1RSTDISABLE_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dbgl1rstdisable_cluster0 : 1   ; /* [0]  */
        unsigned int    dbgl1rstdisable_cluster1 : 1   ; /* [1]  */
        unsigned int    dbgl1rstdisable_cluster2 : 1   ; /* [2]  */
        unsigned int    dbgl1rstdisable_cluster3 : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_DBGL1RSTDISABLE_CTRL;

/* Define the union U_SC_SYSMODE_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    modectrl              : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SYSMODE_CTRL;

/* Define the union U_SC_PLLCTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll_over              : 1   ; /* [0]  */
        unsigned int    pll_en_sw             : 1   ; /* [1]  */
        unsigned int    reserved_0            : 1   ; /* [2]  */
        unsigned int    pll_time              : 25  ; /* [27..3]  */
        unsigned int    reserved_1            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLCTRL;

/* Define the union U_SC_PLLFCTRL4 */
typedef union
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

} SC_PLLFCTRL4;

/* Define the union U_SC_PLLFCTRL5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll2_frac             : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL5;

/* Define the union U_SC_PLLFCTRL6 */
typedef union
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

} SC_PLLFCTRL6;

/* Define the union U_SC_PLLFCTRL7 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll3_frac             : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL7;

/* Define the union U_SC_PLL_CLK_BYPASS_DDR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [0]  */
        unsigned int    reserved_1            : 1   ; /* [1]  */
        unsigned int    pll2_bypass_external_n : 1   ; /* [2]  */
        unsigned int    pll3_bypass_external_n : 1   ; /* [3]  */
        unsigned int    reserved_2            : 13  ; /* [16..4]  */
        unsigned int    pll2_peri_mode        : 1   ; /* [17]  */
        unsigned int    pll3_peri_mode        : 1   ; /* [18]  */
        unsigned int    reserved_3            : 1   ; /* [19]  */
        unsigned int    reserved_4            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_CLK_BYPASS_DDR;

/* Define the union U_SC_REF_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ref_clken             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REF_CLK_EN;

/* Define the union U_SC_REF_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ref_clk_dis           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REF_CLK_DIS;

/* Define the union U_SC_GPIO_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gpio_db_clken         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_CLK_EN;

/* Define the union U_SC_GPIO_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gpio_db_clk_dis       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_CLK_DIS;

/* Define the union U_SC_CLUS0_CRG_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster0_dbg_clken    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS0_CRG_CLK_EN;

/* Define the union U_SC_CLUS0_CRG_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster0_dbg_clk_dis  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS0_CRG_CLK_DIS;

/* Define the union U_SC_CLUS0_CRG_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster0_l2_srst_req  : 1   ; /* [0]  */
        unsigned int    cluster0_cpu0_por_srst_req : 1   ; /* [1]  */
        unsigned int    cluster0_cpu1_por_srst_req : 1   ; /* [2]  */
        unsigned int    cluster0_cpu2_por_srst_req : 1   ; /* [3]  */
        unsigned int    cluster0_cpu3_por_srst_req : 1   ; /* [4]  */
        unsigned int    cluster0_cpu0_srst_req : 1   ; /* [5]  */
        unsigned int    cluster0_cpu1_srst_req : 1   ; /* [6]  */
        unsigned int    cluster0_cpu2_srst_req : 1   ; /* [7]  */
        unsigned int    cluster0_cpu3_srst_req : 1   ; /* [8]  */
        unsigned int    cluster0_dbg_srst_req : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS0_CRG_RESET_REQ;

/* Define the union U_SC_CLUS0_CRG_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster0_l2_srst_dreq : 1   ; /* [0]  */
        unsigned int    cluster0_cpu0_por_srst_dreq : 1   ; /* [1]  */
        unsigned int    cluster0_cpu1_por_srst_dreq : 1   ; /* [2]  */
        unsigned int    cluster0_cpu2_por_srst_dreq : 1   ; /* [3]  */
        unsigned int    cluster0_cpu3_por_srst_dreq : 1   ; /* [4]  */
        unsigned int    cluster0_cpu0_srst_dreq : 1   ; /* [5]  */
        unsigned int    cluster0_cpu1_srst_dreq : 1   ; /* [6]  */
        unsigned int    cluster0_cpu2_srst_dreq : 1   ; /* [7]  */
        unsigned int    cluster0_cpu3_srst_dreq : 1   ; /* [8]  */
        unsigned int    cluster0_dbg_srst_dreq : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS0_CRG_RESET_DREQ;

/* Define the union U_SC_CLUS1_CRG_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster1_dbg_clken    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS1_CRG_CLK_EN;

/* Define the union U_SC_CLUS1_CRG_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster1_dbg_clk_dis  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS1_CRG_CLK_DIS;

/* Define the union U_SC_CLUS1_CRG_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster1_l2_srst_req  : 1   ; /* [0]  */
        unsigned int    cluster1_cpu0_por_srst_req : 1   ; /* [1]  */
        unsigned int    cluster1_cpu1_por_srst_req : 1   ; /* [2]  */
        unsigned int    cluster1_cpu2_por_srst_req : 1   ; /* [3]  */
        unsigned int    cluster1_cpu3_por_srst_req : 1   ; /* [4]  */
        unsigned int    cluster1_cpu0_srst_req : 1   ; /* [5]  */
        unsigned int    cluster1_cpu1_srst_req : 1   ; /* [6]  */
        unsigned int    cluster1_cpu2_srst_req : 1   ; /* [7]  */
        unsigned int    cluster1_cpu3_srst_req : 1   ; /* [8]  */
        unsigned int    cluster1_dbg_srst_req : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS1_CRG_RESET_REQ;

/* Define the union U_SC_CLUS1_CRG_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster1_l2_srst_dreq : 1   ; /* [0]  */
        unsigned int    cluster1_cpu0_por_srst_dreq : 1   ; /* [1]  */
        unsigned int    cluster1_cpu1_por_srst_dreq : 1   ; /* [2]  */
        unsigned int    cluster1_cpu2_por_srst_dreq : 1   ; /* [3]  */
        unsigned int    cluster1_cpu3_por_srst_dreq : 1   ; /* [4]  */
        unsigned int    cluster1_cpu0_srst_dreq : 1   ; /* [5]  */
        unsigned int    cluster1_cpu1_srst_dreq : 1   ; /* [6]  */
        unsigned int    cluster1_cpu2_srst_dreq : 1   ; /* [7]  */
        unsigned int    cluster1_cpu3_srst_dreq : 1   ; /* [8]  */
        unsigned int    cluster1_dbg_srst_dreq : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS1_CRG_RESET_DREQ;

/* Define the union U_SC_CLUS2_CRG_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster2_dbg_clken    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS2_CRG_CLK_EN;

/* Define the union U_SC_CLUS2_CRG_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster2_dbg_clk_dis  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS2_CRG_CLK_DIS;

/* Define the union U_SC_CLUS2_CRG_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster2_l2_srst_req  : 1   ; /* [0]  */
        unsigned int    cluster2_cpu0_por_srst_req : 1   ; /* [1]  */
        unsigned int    cluster2_cpu1_por_srst_req : 1   ; /* [2]  */
        unsigned int    cluster2_cpu2_por_srst_req : 1   ; /* [3]  */
        unsigned int    cluster2_cpu3_por_srst_req : 1   ; /* [4]  */
        unsigned int    cluster2_cpu0_srst_req : 1   ; /* [5]  */
        unsigned int    cluster2_cpu1_srst_req : 1   ; /* [6]  */
        unsigned int    cluster2_cpu2_srst_req : 1   ; /* [7]  */
        unsigned int    cluster2_cpu3_srst_req : 1   ; /* [8]  */
        unsigned int    cluster2_dbg_srst_req : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS2_CRG_RESET_REQ;

/* Define the union U_SC_CLUS2_CRG_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster2_l2_srst_dreq : 1   ; /* [0]  */
        unsigned int    cluster2_cpu0_por_srst_dreq : 1   ; /* [1]  */
        unsigned int    cluster2_cpu1_por_srst_dreq : 1   ; /* [2]  */
        unsigned int    cluster2_cpu2_por_srst_dreq : 1   ; /* [3]  */
        unsigned int    cluster2_cpu3_por_srst_dreq : 1   ; /* [4]  */
        unsigned int    cluster2_cpu0_srst_dreq : 1   ; /* [5]  */
        unsigned int    cluster2_cpu1_srst_dreq : 1   ; /* [6]  */
        unsigned int    cluster2_cpu2_srst_dreq : 1   ; /* [7]  */
        unsigned int    cluster2_cpu3_srst_dreq : 1   ; /* [8]  */
        unsigned int    cluster2_dbg_srst_dreq : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS2_CRG_RESET_DREQ;

/* Define the union U_SC_CLUS3_CRG_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster3_dbg_clken    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS3_CRG_CLK_EN;

/* Define the union U_SC_CLUS3_CRG_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster3_dbg_clk_dis  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS3_CRG_CLK_DIS;

/* Define the union U_SC_CLUS3_CRG_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster3_l2_srst_req  : 1   ; /* [0]  */
        unsigned int    cluster3_cpu0_por_srst_req : 1   ; /* [1]  */
        unsigned int    cluster3_cpu1_por_srst_req : 1   ; /* [2]  */
        unsigned int    cluster3_cpu2_por_srst_req : 1   ; /* [3]  */
        unsigned int    cluster3_cpu3_por_srst_req : 1   ; /* [4]  */
        unsigned int    cluster3_cpu0_srst_req : 1   ; /* [5]  */
        unsigned int    cluster3_cpu1_srst_req : 1   ; /* [6]  */
        unsigned int    cluster3_cpu2_srst_req : 1   ; /* [7]  */
        unsigned int    cluster3_cpu3_srst_req : 1   ; /* [8]  */
        unsigned int    cluster3_dbg_srst_req : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS3_CRG_RESET_REQ;

/* Define the union U_SC_CLUS3_CRG_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster3_l2_srst_dreq : 1   ; /* [0]  */
        unsigned int    cluster3_cpu0_por_srst_dreq : 1   ; /* [1]  */
        unsigned int    cluster3_cpu1_por_srst_dreq : 1   ; /* [2]  */
        unsigned int    cluster3_cpu2_por_srst_dreq : 1   ; /* [3]  */
        unsigned int    cluster3_cpu3_por_srst_dreq : 1   ; /* [4]  */
        unsigned int    cluster3_cpu0_srst_dreq : 1   ; /* [5]  */
        unsigned int    cluster3_cpu1_srst_dreq : 1   ; /* [6]  */
        unsigned int    cluster3_cpu2_srst_dreq : 1   ; /* [7]  */
        unsigned int    cluster3_cpu3_srst_dreq : 1   ; /* [8]  */
        unsigned int    cluster3_dbg_srst_dreq : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS3_CRG_RESET_DREQ;

/* Define the union U_SC_LLC_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    llc0_clken            : 1   ; /* [0]  */
        unsigned int    llc1_clken            : 1   ; /* [1]  */
        unsigned int    llc2_clken            : 1   ; /* [2]  */
        unsigned int    llc3_clken            : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LLC_CLK_EN;

/* Define the union U_SC_LLC_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    llc0_clk_dis          : 1   ; /* [0]  */
        unsigned int    llc1_clk_dis          : 1   ; /* [1]  */
        unsigned int    llc2_clk_dis          : 1   ; /* [2]  */
        unsigned int    llc3_clk_dis          : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LLC_CLK_DIS;

/* Define the union U_SC_LLC_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    llc0_srst_req         : 1   ; /* [0]  */
        unsigned int    llc1_srst_req         : 1   ; /* [1]  */
        unsigned int    llc2_srst_req         : 1   ; /* [2]  */
        unsigned int    llc3_srst_req         : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LLC_RESET_REQ;

/* Define the union U_SC_LLC_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    llc0_srst_dreq        : 1   ; /* [0]  */
        unsigned int    llc1_srst_dreq        : 1   ; /* [1]  */
        unsigned int    llc2_srst_dreq        : 1   ; /* [2]  */
        unsigned int    llc3_srst_dreq        : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LLC_RESET_DREQ;

/* Define the union U_SC_DDRC_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddrc0_clken           : 1   ; /* [0]  */
        unsigned int    ddrc1_clken           : 1   ; /* [1]  */
        unsigned int    ddrphy0_bypass_clken  : 1   ; /* [2]  */
        unsigned int    ddrphy1_bypass_clken  : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DDRC_CLK_EN;

/* Define the union U_SC_DDRC_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddrc0_clk_dis         : 1   ; /* [0]  */
        unsigned int    ddrc1_clk_dis         : 1   ; /* [1]  */
        unsigned int    ddrphy0_bypass_clk_dis : 1   ; /* [2]  */
        unsigned int    ddrphy1_bypass_clk_dis : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DDRC_CLK_DIS;

/* Define the union U_SC_DDRC_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddrc0_srst_req        : 1   ; /* [0]  */
        unsigned int    ddrc1_srst_req        : 1   ; /* [1]  */
        unsigned int    ddrphy0_srst_req      : 1   ; /* [2]  */
        unsigned int    ddrphy1_srst_req      : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DDRC_RESET_REQ;

/* Define the union U_SC_DDRC_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddrc0_srst_dreq       : 1   ; /* [0]  */
        unsigned int    ddrc1_srst_dreq       : 1   ; /* [1]  */
        unsigned int    ddrphy0_srst_dreq     : 1   ; /* [2]  */
        unsigned int    ddrphy1_srst_dreq     : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DDRC_RESET_DREQ;

/* Define the union U_SC_SLLC_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc0_clken           : 1   ; /* [0]  */
        unsigned int    sllc1_clken           : 1   ; /* [1]  */
        unsigned int    sllc2_clken           : 1   ; /* [2]  */
        unsigned int    sllc3_clken           : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_CLK_EN;

/* Define the union U_SC_SLLC_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc0_clk_dis         : 1   ; /* [0]  */
        unsigned int    sllc1_clk_dis         : 1   ; /* [1]  */
        unsigned int    sllc2_clk_dis         : 1   ; /* [2]  */
        unsigned int    sllc3_clk_dis         : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_CLK_DIS;

/* Define the union U_SC_SLLC_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc0_srst_req        : 1   ; /* [0]  */
        unsigned int    sllc1_srst_req        : 1   ; /* [1]  */
        unsigned int    sllc2_srst_req        : 1   ; /* [2]  */
        unsigned int    sllc3_srst_req        : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_RESET_REQ;

/* Define the union U_SC_SLLC_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc0_srst_dreq       : 1   ; /* [0]  */
        unsigned int    sllc1_srst_dreq       : 1   ; /* [1]  */
        unsigned int    sllc2_srst_dreq       : 1   ; /* [2]  */
        unsigned int    sllc3_srst_dreq       : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_RESET_DREQ;

/* Define the union U_SC_SLLC_TSVRX_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc0_tsvrx0_srst_req : 1   ; /* [0]  */
        unsigned int    sllc0_tsvrx1_srst_req : 1   ; /* [1]  */
        unsigned int    sllc0_tsvrx2_srst_req : 1   ; /* [2]  */
        unsigned int    sllc0_tsvrx3_srst_req : 1   ; /* [3]  */
        unsigned int    sllc1_tsvrx0_srst_req : 1   ; /* [4]  */
        unsigned int    sllc1_tsvrx1_srst_req : 1   ; /* [5]  */
        unsigned int    sllc1_tsvrx2_srst_req : 1   ; /* [6]  */
        unsigned int    sllc1_tsvrx3_srst_req : 1   ; /* [7]  */
        unsigned int    sllc2_tsvrx0_srst_req : 1   ; /* [8]  */
        unsigned int    sllc2_tsvrx1_srst_req : 1   ; /* [9]  */
        unsigned int    sllc2_tsvrx2_srst_req : 1   ; /* [10]  */
        unsigned int    sllc2_tsvrx3_srst_req : 1   ; /* [11]  */
        unsigned int    sllc3_tsvrx0_srst_req : 1   ; /* [12]  */
        unsigned int    sllc3_tsvrx1_srst_req : 1   ; /* [13]  */
        unsigned int    sllc3_tsvrx2_srst_req : 1   ; /* [14]  */
        unsigned int    sllc3_tsvrx3_srst_req : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_TSVRX_RESET_REQ;

/* Define the union U_SC_SLLC_TSVRX_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc0_tsvrx0_srst_dreq : 1   ; /* [0]  */
        unsigned int    sllc0_tsvrx1_srst_dreq : 1   ; /* [1]  */
        unsigned int    sllc0_tsvrx2_srst_dreq : 1   ; /* [2]  */
        unsigned int    sllc0_tsvrx3_srst_dreq : 1   ; /* [3]  */
        unsigned int    sllc1_tsvrx0_srst_dreq : 1   ; /* [4]  */
        unsigned int    sllc1_tsvrx1_srst_dreq : 1   ; /* [5]  */
        unsigned int    sllc1_tsvrx2_srst_dreq : 1   ; /* [6]  */
        unsigned int    sllc1_tsvrx3_srst_dreq : 1   ; /* [7]  */
        unsigned int    sllc2_tsvrx0_srst_dreq : 1   ; /* [8]  */
        unsigned int    sllc2_tsvrx1_srst_dreq : 1   ; /* [9]  */
        unsigned int    sllc2_tsvrx2_srst_dreq : 1   ; /* [10]  */
        unsigned int    sllc2_tsvrx3_srst_dreq : 1   ; /* [11]  */
        unsigned int    sllc3_tsvrx0_srst_dreq : 1   ; /* [12]  */
        unsigned int    sllc3_tsvrx1_srst_dreq : 1   ; /* [13]  */
        unsigned int    sllc3_tsvrx2_srst_dreq : 1   ; /* [14]  */
        unsigned int    sllc3_tsvrx3_srst_dreq : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_TSVRX_RESET_DREQ;

/* Define the union U_SC_HPM_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hpm_clken             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HPM_CLK_EN;

/* Define the union U_SC_HPM_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hpm_clk_dis           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HPM_CLK_DIS;

/* Define the union U_SC_HPM_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hpm_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HPM_RESET_REQ;

/* Define the union U_SC_HPM_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hpm_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HPM_RESET_DREQ;

/* Define the union U_SC_DJTAG_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_clken           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CLK_EN;

/* Define the union U_SC_DJTAG_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_clk_dis         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CLK_DIS;

/* Define the union U_SC_DJTAG_SRST_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_SRST_REQ;

/* Define the union U_SC_DJTAG_SRST_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_SRST_DREQ;

/* Define the union U_SC_HHA_MN_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hha0_clken            : 1   ; /* [0]  */
        unsigned int    hha1_clken            : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HHA_MN_CLK_EN;

/* Define the union U_SC_HHA_MN_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hha0_clk_dis          : 1   ; /* [0]  */
        unsigned int    hha1_clk_dis          : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HHA_MN_CLK_DIS;

/* Define the union U_SC_HHA_MN_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hha0_srst_req         : 1   ; /* [0]  */
        unsigned int    hha1_srst_req         : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HHA_MN_RESET_REQ;

/* Define the union U_SC_HHA_MN_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hha0_srst_dreq        : 1   ; /* [0]  */
        unsigned int    hha1_srst_dreq        : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HHA_MN_RESET_DREQ;

/* Define the union U_SC_WDOG_RESET_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wdog0_reset_cfg       : 1   ; /* [0]  */
        unsigned int    wdog1_reset_cfg       : 1   ; /* [1]  */
        unsigned int    wdog2_reset_cfg       : 1   ; /* [2]  */
        unsigned int    wdog3_reset_cfg       : 1   ; /* [3]  */
        unsigned int    wdog4_reset_cfg       : 1   ; /* [4]  */
        unsigned int    wdog5_reset_cfg       : 1   ; /* [5]  */
        unsigned int    wdog6_reset_cfg       : 1   ; /* [6]  */
        unsigned int    wdog7_reset_cfg       : 1   ; /* [7]  */
        unsigned int    wdog8_reset_cfg       : 1   ; /* [8]  */
        unsigned int    wdog9_reset_cfg       : 1   ; /* [9]  */
        unsigned int    wdog10_reset_cfg      : 1   ; /* [10]  */
        unsigned int    wdog11_reset_cfg      : 1   ; /* [11]  */
        unsigned int    wdog12_reset_cfg      : 1   ; /* [12]  */
        unsigned int    wdog13_reset_cfg      : 1   ; /* [13]  */
        unsigned int    wdog14_reset_cfg      : 1   ; /* [14]  */
        unsigned int    wdog15_reset_cfg      : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDOG_RESET_CFG;

/* Define the union U_SC_TSMC_PLLCTRL2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsmc_pll2_refdiv      : 5   ; /* [4..0]  */
        unsigned int    tsmc_pll2_fbdiv       : 8   ; /* [12..5]  */
        unsigned int    tsmc_pll2_pd          : 1   ; /* [13]  */
        unsigned int    tsmc_pll2_bypass      : 1   ; /* [14]  */
        unsigned int    tsmc_pll2_postdiv     : 3   ; /* [17..15]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSMC_PLLCTRL2;

/* Define the union U_SC_TSMC_PLLCTRL3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsmc_pll3_refdiv      : 5   ; /* [4..0]  */
        unsigned int    tsmc_pll3_fbdiv       : 8   ; /* [12..5]  */
        unsigned int    tsmc_pll3_pd          : 1   ; /* [13]  */
        unsigned int    tsmc_pll3_bypass      : 1   ; /* [14]  */
        unsigned int    tsmc_pll3_postdiv     : 3   ; /* [17..15]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSMC_PLLCTRL3;

/* Define the union U_SC_PLL_SEL_DDR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll_sel_ddr           : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_SEL_DDR;

/* Define the union U_SC_ALL_SCAN_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    all_scan_sys_int      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ALL_SCAN_CTRL;

/* Define the union U_SC_TSENSOR_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_temp_en       : 1   ; /* [0]  */
        unsigned int    tsensor_calib         : 1   ; /* [1]  */
        unsigned int    reserved_0            : 2   ; /* [3..2]  */
        unsigned int    tsensor_temp_trim     : 8   ; /* [11..4]  */
        unsigned int    tsensor_ct_sel        : 2   ; /* [13..12]  */
        unsigned int    reserved_1            : 18  ; /* [31..14]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_CTRL;

/* Define the union U_SC_TSENSOR_TEST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_test          : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_TEST;

/* Define the union U_SC_TSENSOR_ALARM */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_low           : 10  ; /* [9..0]  */
        unsigned int    tsensor_high          : 10  ; /* [19..10]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_ALARM;

/* Define the union U_SC_DDRC_APB_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddrc0_apb_gt_en       : 1   ; /* [0]  */
        unsigned int    ddrc1_apb_gt_en       : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DDRC_APB_CLK_EN;

/* Define the union U_SC_MBIST_CPUI_ENABLE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mbist_cpui_enable     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MBIST_CPUI_ENABLE;

/* Define the union U_SC_MBIST_CPUI_RESET_N */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mbist_cpui_reset_n    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MBIST_CPUI_RESET_N;

/* Define the union U_SC_MBIST_CPUI_WRITE_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mbist_cpui_write_en   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MBIST_CPUI_WRITE_EN;

/* Define the union U_SC_SRC_INT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_under         : 1   ; /* [0]  */
        unsigned int    tsensor_over          : 1   ; /* [1]  */
        unsigned int    pll0_unlock           : 1   ; /* [2]  */
        unsigned int    pll1_unlock           : 1   ; /* [3]  */
        unsigned int    pll2_unlock           : 1   ; /* [4]  */
        unsigned int    pll3_unlock           : 1   ; /* [5]  */
        unsigned int    pll4_unlock           : 1   ; /* [6]  */
        unsigned int    djtag_sta_timeout     : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRC_INT;

/* Define the union U_SC_INT_MASK */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_under_int_mask : 1   ; /* [0]  */
        unsigned int    tsensor_over_int_mask : 1   ; /* [1]  */
        unsigned int    pll_unlock_int_mask   : 1   ; /* [2]  */
        unsigned int    djtag_sta_timeout_int_mask : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_INT_MASK;

/* Define the union U_SC_IM_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    it_md_en              : 1   ; /* [0]  */
        unsigned int    it_md_ctrl            : 3   ; /* [3..1]  */
        unsigned int    it_md_clk             : 3   ; /* [6..4]  */
        unsigned int    in_md_type            : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IM_CTRL;

/* Define the union U_SC_IM_STAT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    it_md_stat            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IM_STAT;

/* Define the union U_SC_XTAL_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    xtal_over             : 1   ; /* [0]  */
        unsigned int    xtal_en_sw            : 1   ; /* [1]  */
        unsigned int    reserved_0            : 1   ; /* [2]  */
        unsigned int    xtal_time             : 16  ; /* [18..3]  */
        unsigned int    reserved_1            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XTAL_CTRL;

/* Define the union U_SC_ITCR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sc_itcr               : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITCR;

/* Define the union U_SC_ITIR0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sc_itir0              : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITIR0;

/* Define the union U_SC_ITOR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sc_itor               : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITOR;

/* Define the union U_SC_CNT_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    test_mode_en          : 1   ; /* [0]  */
        unsigned int    test_pll_en           : 1   ; /* [1]  */
        unsigned int    test_load_time        : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CNT_CTRL;

/* Define the union U_SC_BOOT_CTRL_CFG_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    boot_ctrl_cfg_en      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BOOT_CTRL_CFG_EN;

/* Define the union U_SC_BOOT_CTRL_CFG */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    boot_sel_cfg          : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 14  ; /* [15..2]  */
        unsigned int    boot_64b_cfg          : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BOOT_CTRL_CFG;

/* Define the union U_SC_SOCKET_INITIALED */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    socket_initialed      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SOCKET_INITIALED;

/* Define the union U_SC_SYSCNT_SYNC_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    syscnt_sync_en        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SYSCNT_SYNC_EN;

/* Define the union U_SC_SYSCNT_SYNC_AVE_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    syscnt_sync_ave_en    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SYSCNT_SYNC_AVE_EN;

/* Define the union U_SC_SYSCNT_SYNC_DST_DATA_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    syscnt_sync_dst_data_en : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SYSCNT_SYNC_DST_DATA_EN;

/* Define the union U_SC_SYSCNT_SYNC_SRC_DATA_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    syscnt_sync_src_data_en : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SYSCNT_SYNC_SRC_DATA_EN;

/* Define the union U_SC_SYSCNT_VAL_CFG_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    syscnt_val_cfg_en     : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SYSCNT_VAL_CFG_EN;

/* Define the union U_SC_PROBE_SYSCNT_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    probe_syscnt_en       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PROBE_SYSCNT_EN;

/* Define the union U_SC_PROBE_SYSCNT_DATA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    probe_syscnt_data     : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PROBE_SYSCNT_DATA;

/* Define the union U_SC_BROADCAST_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    broadcast_en          : 5   ; /* [4..0]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BROADCAST_EN;

/* Define the union U_SC_BROADCAST_CORE_EVENT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    broadcast_core_event  : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BROADCAST_CORE_EVENT;

/* Define the union U_SC_DJTAG_MSTR_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_mstr_en         : 1   ; /* [0]  */
        unsigned int    djtag_nor_cfg_en      : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_EN;

/* Define the union U_SC_DJTAG_MSTR_START_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_mstr_start_en   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_START_EN;

/* Define the union U_SC_DJTAG_SEC_ACC_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_sec_acc_en      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_SEC_ACC_EN;

/* Define the union U_SC_DJTAG_DEBUG_MODULE_SEL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    debug_module_sel      : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_DEBUG_MODULE_SEL;

/* Define the union U_SC_DJTAG_MSTR_WR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_mstr_wr         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_WR;

/* Define the union U_SC_DJTAG_CHAIN_UNIT_CFG_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    chain_unit_cfg_en     : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CHAIN_UNIT_CFG_EN;

/* Define the union U_SC_DJTAG_MSTR_ADDR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_mstr_addr       : 31  ; /* [30..0]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_MSTR_ADDR;

/* Define the union U_SC_TDRE_OP_ADDR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tdre_op_addr          : 18  ; /* [17..0]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TDRE_OP_ADDR;

/* Define the union U_SC_TDRE_REPAIR_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tdre_repair_en        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TDRE_REPAIR_EN;

/* Define the union U_SC_AXI_MSTR_START_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    axi_mstr_start_en     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_AXI_MSTR_START_EN;

/* Define the union U_SC_AXI_MSTR_SEC_ACC_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    axi_mstr_sec_acc_en   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_AXI_MSTR_SEC_ACC_EN;

/* Define the union U_SC_AXI_MSTR_WR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    axi_mstr_wr           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_AXI_MSTR_WR;

/* Define the union U_SC_AXI_MSTR_USER */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    axi_mstr_vmid         : 8   ; /* [7..0]  */
        unsigned int    axi_mstr_asid         : 8   ; /* [15..8]  */
        unsigned int    axi_mstr_user21_16    : 6   ; /* [21..16]  */
        unsigned int    reserved_0            : 10  ; /* [31..22]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_AXI_MSTR_USER;

/* Define the union U_SC_CTRL_HC_HHA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsel_hc_hha           : 3   ; /* [2..0]  */
        unsigned int    test_hc_hha           : 2   ; /* [4..3]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CTRL_HC_HHA;

/* Define the union U_SC_CTRL_RASHDE_HHA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rashde_hha       : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CTRL_RASHDE_HHA;

/* Define the union U_SC_CTRL_HC_LLC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsel_hc_llc           : 3   ; /* [2..0]  */
        unsigned int    test_hc_llc           : 2   ; /* [4..3]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CTRL_HC_LLC;

/* Define the union U_SC_CTRL_RASHDE_LLC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rashde_llc       : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CTRL_RASHDE_LLC;

/* Define the union U_SC_HS_MEM_ADJUST_HC */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hs_mem_adjust_hc      : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HS_MEM_ADJUST_HC;

/* Define the union U_SC_HS_MEM_ADJUST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hs_mem_adjust         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HS_MEM_ADJUST;

/* Define the union U_SC_CTRL_RASHDE_CSYS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rashde_csys      : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CTRL_RASHDE_CSYS;

/* Define the union U_SC_CPU_PMUSNAPSHOTACK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pmusnapshotack_cluster0 : 4   ; /* [3..0]  */
        unsigned int    pmusnapshotack_cluster1 : 4   ; /* [7..4]  */
        unsigned int    pmusnapshotack_cluster2 : 4   ; /* [11..8]  */
        unsigned int    pmusnapshotack_cluster3 : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_PMUSNAPSHOTACK_ST;

/* Define the union U_SC_CPU_SMPEN_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    smpen_cluster0        : 4   ; /* [3..0]  */
        unsigned int    smpen_cluster1        : 4   ; /* [7..4]  */
        unsigned int    smpen_cluster2        : 4   ; /* [11..8]  */
        unsigned int    smpen_cluster3        : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_SMPEN_ST;

/* Define the union U_SC_CPU_STANDBYWFE_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    standbywfe_cluster0   : 4   ; /* [3..0]  */
        unsigned int    standbywfe_cluster1   : 4   ; /* [7..4]  */
        unsigned int    standbywfe_cluster2   : 4   ; /* [11..8]  */
        unsigned int    standbywfe_cluster3   : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_STANDBYWFE_ST;

/* Define the union U_SC_CPU_STANDBYWFI_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    standbywfi_cluster0   : 4   ; /* [3..0]  */
        unsigned int    standbywfi_cluster1   : 4   ; /* [7..4]  */
        unsigned int    standbywfi_cluster2   : 4   ; /* [11..8]  */
        unsigned int    standbywfi_cluster3   : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_STANDBYWFI_ST;

/* Define the union U_SC_CPU_DBGACK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dbgack_cluster0       : 4   ; /* [3..0]  */
        unsigned int    dbgack_cluster1       : 4   ; /* [7..4]  */
        unsigned int    dbgack_cluster2       : 4   ; /* [11..8]  */
        unsigned int    dbgack_cluster3       : 4   ; /* [15..12]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_DBGACK_ST;

/* Define the union U_SC_CPU_CLREXMONACK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clrexmonack_cluster0  : 1   ; /* [0]  */
        unsigned int    clrexmonack_cluster1  : 1   ; /* [1]  */
        unsigned int    clrexmonack_cluster2  : 1   ; /* [2]  */
        unsigned int    clrexmonack_cluster3  : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_CLREXMONACK_ST;

/* Define the union U_SC_CPU_STANDBYWFIL2_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    standbywfil2_cluster0 : 1   ; /* [0]  */
        unsigned int    standbywfil2_cluster1 : 1   ; /* [1]  */
        unsigned int    standbywfil2_cluster2 : 1   ; /* [2]  */
        unsigned int    standbywfil2_cluster3 : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_STANDBYWFIL2_ST;

/* Define the union U_SC_CPU_L2FSHDONE_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    l2flushdone_cluster0  : 1   ; /* [0]  */
        unsigned int    l2flushdone_cluster1  : 1   ; /* [1]  */
        unsigned int    l2flushdone_cluster2  : 1   ; /* [2]  */
        unsigned int    l2flushdone_cluster3  : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_L2FSHDONE_ST;

/* Define the union U_SC_SYSMODE_STATUS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sys_mode              : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SYSMODE_STATUS;

/* Define the union U_SC_PLL_LOCK_STATUS */
typedef union
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

} SC_PLL_LOCK_STATUS;

/* Define the union U_SC_PLLCTRL_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll_on                : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLCTRL_ST;

/* Define the union U_SC_REF_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ref_clk_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REF_CLK_ST;

/* Define the union U_SC_GPIO_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gpio_db_clk_st        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_CLK_ST;

/* Define the union U_SC_CLUS0_CRG_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster0_dbg_clk_st   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS0_CRG_CLK_ST;

/* Define the union U_SC_CLUS0_CRG_RESET_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster0_l2_srst_st   : 1   ; /* [0]  */
        unsigned int    cluster0_cpu0_por_srst_st : 1   ; /* [1]  */
        unsigned int    cluster0_cpu1_por_srst_st : 1   ; /* [2]  */
        unsigned int    cluster0_cpu2_por_srst_st : 1   ; /* [3]  */
        unsigned int    cluster0_cpu3_por_srst_st : 1   ; /* [4]  */
        unsigned int    cluster0_cpu0_srst_st : 1   ; /* [5]  */
        unsigned int    cluster0_cpu1_srst_st : 1   ; /* [6]  */
        unsigned int    cluster0_cpu2_srst_st : 1   ; /* [7]  */
        unsigned int    cluster0_cpu3_srst_st : 1   ; /* [8]  */
        unsigned int    cluster0_dbg_srst_st  : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS0_CRG_RESET_ST;

/* Define the union U_SC_CLUS1_CRG_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster1_dbg_clk_st   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS1_CRG_CLK_ST;

/* Define the union U_SC_CLUS1_CRG_RESET_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster1_l2_srst_st   : 1   ; /* [0]  */
        unsigned int    cluster1_cpu0_por_srst_st : 1   ; /* [1]  */
        unsigned int    cluster1_cpu1_por_srst_st : 1   ; /* [2]  */
        unsigned int    cluster1_cpu2_por_srst_st : 1   ; /* [3]  */
        unsigned int    cluster1_cpu3_por_srst_st : 1   ; /* [4]  */
        unsigned int    cluster1_cpu0_srst_st : 1   ; /* [5]  */
        unsigned int    cluster1_cpu1_srst_st : 1   ; /* [6]  */
        unsigned int    cluster1_cpu2_srst_st : 1   ; /* [7]  */
        unsigned int    cluster1_cpu3_srst_st : 1   ; /* [8]  */
        unsigned int    cluster1_dbg_srst_st  : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS1_CRG_RESET_ST;

/* Define the union U_SC_CLUS2_CRG_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster2_dbg_clk_st   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS2_CRG_CLK_ST;

/* Define the union U_SC_CLUS2_CRG_RESET_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster2_l2_srst_st   : 1   ; /* [0]  */
        unsigned int    cluster2_cpu0_por_srst_st : 1   ; /* [1]  */
        unsigned int    cluster2_cpu1_por_srst_st : 1   ; /* [2]  */
        unsigned int    cluster2_cpu2_por_srst_st : 1   ; /* [3]  */
        unsigned int    cluster2_cpu3_por_srst_st : 1   ; /* [4]  */
        unsigned int    cluster2_cpu0_srst_st : 1   ; /* [5]  */
        unsigned int    cluster2_cpu1_srst_st : 1   ; /* [6]  */
        unsigned int    cluster2_cpu2_srst_st : 1   ; /* [7]  */
        unsigned int    cluster2_cpu3_srst_st : 1   ; /* [8]  */
        unsigned int    cluster2_dbg_srst_st  : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS2_CRG_RESET_ST;

/* Define the union U_SC_CLUS3_CRG_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster3_dbg_clk_st   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS3_CRG_CLK_ST;

/* Define the union U_SC_CLUS3_CRG_RESET_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cluster3_l2_srst_st   : 1   ; /* [0]  */
        unsigned int    cluster3_cpu0_por_srst_st : 1   ; /* [1]  */
        unsigned int    cluster3_cpu1_por_srst_st : 1   ; /* [2]  */
        unsigned int    cluster3_cpu2_por_srst_st : 1   ; /* [3]  */
        unsigned int    cluster3_cpu3_por_srst_st : 1   ; /* [4]  */
        unsigned int    cluster3_cpu0_srst_st : 1   ; /* [5]  */
        unsigned int    cluster3_cpu1_srst_st : 1   ; /* [6]  */
        unsigned int    cluster3_cpu2_srst_st : 1   ; /* [7]  */
        unsigned int    cluster3_cpu3_srst_st : 1   ; /* [8]  */
        unsigned int    cluster3_dbg_srst_st  : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLUS3_CRG_RESET_ST;

/* Define the union U_SC_LLC_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    llc0_clk_st           : 1   ; /* [0]  */
        unsigned int    llc1_clk_st           : 1   ; /* [1]  */
        unsigned int    llc2_clk_st           : 1   ; /* [2]  */
        unsigned int    llc3_clk_st           : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LLC_CLK_ST;

/* Define the union U_SC_LLC_RESET_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    llc0_srst_st          : 1   ; /* [0]  */
        unsigned int    llc1_srst_st          : 1   ; /* [1]  */
        unsigned int    llc2_srst_st          : 1   ; /* [2]  */
        unsigned int    llc3_srst_st          : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LLC_RESET_ST;

/* Define the union U_SC_DDRC_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddrc0_clk_st          : 1   ; /* [0]  */
        unsigned int    ddrc1_clk_st          : 1   ; /* [1]  */
        unsigned int    ddrphy0_bypass_clk_st : 1   ; /* [2]  */
        unsigned int    ddrphy1_bypass_clk_st : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DDRC_CLK_ST;

/* Define the union U_SC_DDRC_RESET_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ddrc0_srst_st         : 1   ; /* [0]  */
        unsigned int    ddrc1_srst_st         : 1   ; /* [1]  */
        unsigned int    ddrphy0_srst_st       : 1   ; /* [2]  */
        unsigned int    ddrphy1_srst_st       : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DDRC_RESET_ST;

/* Define the union U_SC_SLLC_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc0_clk_st          : 1   ; /* [0]  */
        unsigned int    sllc1_clk_st          : 1   ; /* [1]  */
        unsigned int    sllc2_clk_st          : 1   ; /* [2]  */
        unsigned int    sllc3_clk_st          : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_CLK_ST;

/* Define the union U_SC_SLLC_RESET_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc0_srst_st         : 1   ; /* [0]  */
        unsigned int    sllc1_srst_st         : 1   ; /* [1]  */
        unsigned int    sllc2_srst_st         : 1   ; /* [2]  */
        unsigned int    sllc3_srst_st         : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_RESET_ST;

/* Define the union U_SC_SLLC_TSVRX_RESET_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc0_tsvrx0_srst_st  : 1   ; /* [0]  */
        unsigned int    sllc0_tsvrx1_srst_st  : 1   ; /* [1]  */
        unsigned int    sllc0_tsvrx2_srst_st  : 1   ; /* [2]  */
        unsigned int    sllc0_tsvrx3_srst_st  : 1   ; /* [3]  */
        unsigned int    sllc1_tsvrx0_srst_st  : 1   ; /* [4]  */
        unsigned int    sllc1_tsvrx1_srst_st  : 1   ; /* [5]  */
        unsigned int    sllc1_tsvrx2_srst_st  : 1   ; /* [6]  */
        unsigned int    sllc1_tsvrx3_srst_st  : 1   ; /* [7]  */
        unsigned int    sllc2_tsvrx0_srst_st  : 1   ; /* [8]  */
        unsigned int    sllc2_tsvrx1_srst_st  : 1   ; /* [9]  */
        unsigned int    sllc2_tsvrx2_srst_st  : 1   ; /* [10]  */
        unsigned int    sllc2_tsvrx3_srst_st  : 1   ; /* [11]  */
        unsigned int    sllc3_tsvrx0_srst_st  : 1   ; /* [12]  */
        unsigned int    sllc3_tsvrx1_srst_st  : 1   ; /* [13]  */
        unsigned int    sllc3_tsvrx2_srst_st  : 1   ; /* [14]  */
        unsigned int    sllc3_tsvrx3_srst_st  : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_TSVRX_RESET_ST;

/* Define the union U_SC_HPM_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hpm_clk_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HPM_CLK_ST;

/* Define the union U_SC_HPM_SRST_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hpm_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HPM_SRST_ST;

/* Define the union U_SC_DJTAG_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_clk_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_CLK_ST;

/* Define the union U_SC_DJTAG_SRST_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    djtag_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_SRST_ST;

/* Define the union U_SC_HHA_MN_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hha0_clk_st           : 1   ; /* [0]  */
        unsigned int    hha1_clk_st           : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HHA_MN_CLK_ST;

/* Define the union U_SC_HHA_MN_SRST_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hha0_srst_st          : 1   ; /* [0]  */
        unsigned int    hha1_srst_st          : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HHA_MN_SRST_ST;

/* Define the union U_SC_DDRC_WARM_RST_ACKED */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    warm_rst_acked        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DDRC_WARM_RST_ACKED;

/* Define the union U_SC_TSENSOR_SYSSTAT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    temp_out_tsensor      : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 2   ; /* [11..10]  */
        unsigned int    data_ready_tsensor    : 1   ; /* [12]  */
        unsigned int    efuse_parameter_trim  : 8   ; /* [20..13]  */
        unsigned int    reserved_1            : 11  ; /* [31..21]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_SYSSTAT;

/* Define the union U_SC_TSENSOR_TEMP_SAMPLE */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_sample        : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSENSOR_TEMP_SAMPLE;

/* Define the union U_SC_MDDRC0_AC_IOCTL_IOTEST1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mddrc0_ac_ioctl_iotest37_32 : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDDRC0_AC_IOCTL_IOTEST1;

/* Define the union U_SC_MDDRC1_AC_IOCTL_IOTEST1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mddrc1_ac_ioctl_iotest37_32 : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDDRC1_AC_IOCTL_IOTEST1;

/* Define the union U_SC_MDDRC0_DX_IOCTL_IOTEST3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mddrc0_dx_ioctl_iotest98_96 : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDDRC0_DX_IOCTL_IOTEST3;

/* Define the union U_SC_MDDRC1_DX_IOCTL_IOTEST3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mddrc1_dx_ioctl_iotest98_96 : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDDRC1_DX_IOCTL_IOTEST3;

/* Define the union U_SC_INT_STATUS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsensor_under_int_status : 1   ; /* [0]  */
        unsigned int    tsensor_over_int_status : 1   ; /* [1]  */
        unsigned int    pll_unlock_int_status : 1   ; /* [2]  */
        unsigned int    djtag_timeout_int_status : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_INT_STATUS;

/* Define the union U_SC_SOCKET_ID */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    socket_id             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SOCKET_ID;

/* Define the union U_SC_BOOT_CTRL_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pad_die_mode          : 2   ; /* [1..0]  */
        unsigned int    efuse_ns_forbid       : 1   ; /* [2]  */
        unsigned int    pad_boot_sel          : 2   ; /* [4..3]  */
        unsigned int    pad_boot_64b          : 1   ; /* [5]  */
        unsigned int    pad_rst_mode          : 1   ; /* [6]  */
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BOOT_CTRL_ST;

/* Define the union U_SC_BOOT_CTRL_CFG_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    boot_sel_muxed        : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 14  ; /* [15..2]  */
        unsigned int    boot_64b_muxed        : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BOOT_CTRL_CFG_ST;

/* Define the union U_SC_LAST_RST_STATUS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    last_rst_status       : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LAST_RST_STATUS;

/* Define the union U_SC_XTAL_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    xtal_on               : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XTAL_ST;

/* Define the union U_SC_CNT_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    xtal_pll_timeout      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CNT_ST;

/* Define the union U_SC_ITIR0_TEST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sc_itir0_tst          : 13  ; /* [12..0]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITIR0_TEST;

/* Define the union U_SC_ITOR_TEST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sc_itor_tst           : 11  ; /* [10..0]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITOR_TEST;

/* Define the union U_SC_CNT_DATA */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sc_cnt_data           : 25  ; /* [24..0]  */
        unsigned int    reserved_0            : 7   ; /* [31..25]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CNT_DATA;

/* Define the union U_SC_AXI_ACC_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    axi_syscnt_wr_err     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    syscnt_curr_stat      : 4   ; /* [7..4]  */
        unsigned int    broadcast_berror      : 1   ; /* [8]  */
        unsigned int    reserved_1            : 23  ; /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_AXI_ACC_ST;

/* Define the union U_SC_DJTAG_OP_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    unit_conflict         : 8   ; /* [7..0]  */
        unsigned int    djtag_op_done         : 1   ; /* [8]  */
        unsigned int    debug_bus_en          : 1   ; /* [9]  */
        unsigned int    reserved_0            : 6   ; /* [15..10]  */
        unsigned int    rdata_changed         : 10  ; /* [25..16]  */
        unsigned int    reserved_1            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DJTAG_OP_ST;

/* Define the union U_SC_AXI_MSTR_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    axi_op_done           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 3   ; /* [3..1]  */
        unsigned int    axi_rd_error          : 1   ; /* [4]  */
        unsigned int    axi_wr_error          : 1   ; /* [5]  */
        unsigned int    axi_op_conflict       : 1   ; /* [6]  */
        unsigned int    reserved_1            : 25  ; /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_AXI_MSTR_ST;

/* Define the union U_SC_BOARD_CFG_INFO */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    board_cfg_info        : 28  ; /* [27..0]  */
        unsigned int    reserved_0            : 4   ; /* [31..28]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BOARD_CFG_INFO;


/*以下是POE寄存器结构定义*/

/* Define the union U_SC_SRAM_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sram_enb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CLK_EN;

/* Define the union U_SC_SRAM_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sram_dsb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CLK_DIS;

/* Define the union U_SC_POE_CLK_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_poe_enb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_POE_CLK_EN;

/* Define the union U_SC_POE_CLK_DIS */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_poe_dsb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_POE_CLK_DIS;

/* Define the union U_SC_SRAM_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_srst_req         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_RESET_REQ;

/* Define the union U_SC_SRAM_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_srst_dreq        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_RESET_DREQ;

/* Define the union U_SC_POE_RESET_REQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    poe_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_POE_RESET_REQ;

/* Define the union U_SC_POE_RESET_DREQ */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    poe_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_POE_RESET_DREQ;

/* Define the union U_SC_DISPATCH_DAW_EN */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dispatch_daw_en       : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_DAW_EN;

/* Define the union U_SC_DISPATCH_DAW_ARRAY0 */
typedef union
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

} SC_DISPATCH_DAW_ARRAY0;

/* Define the union U_SC_DISPATCH_DAW_ARRAY1 */
typedef union
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

} SC_DISPATCH_DAW_ARRAY1;

/* Define the union U_SC_DISPATCH_DAW_ARRAY2 */
typedef union
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

} SC_DISPATCH_DAW_ARRAY2;

/* Define the union U_SC_DISPATCH_DAW_ARRAY3 */
typedef union
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

} SC_DISPATCH_DAW_ARRAY3;

/* Define the union U_SC_DISPATCH_DAW_ARRAY4 */
typedef union
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

} SC_DISPATCH_DAW_ARRAY4;

/* Define the union U_SC_DISPATCH_DAW_ARRAY5 */
typedef union
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

} SC_DISPATCH_DAW_ARRAY5;

/* Define the union U_SC_DISPATCH_DAW_ARRAY6 */
typedef union
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

} SC_DISPATCH_DAW_ARRAY6;

/* Define the union U_SC_DISPATCH_DAW_ARRAY7 */
typedef union
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

} SC_DISPATCH_DAW_ARRAY7;

/* Define the union U_SC_DISPATCH_RETRY_CONTROL */
typedef union
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

} SC_DISPATCH_RETRY_CONTROL;

/* Define the union U_SC_DISPATCH_INTMASK */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intmask               : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_INTMASK;

/* Define the union U_SC_DISPATCH_RAWINT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rawint                : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_RAWINT;

/* Define the union U_SC_DISPATCH_INTSTAT */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intsts                : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_INTSTAT;

/* Define the union U_SC_DISPATCH_INTCLR */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    intclr                : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_INTCLR;

/* Define the union U_SC_DISPATCH_ERRSTAT */
typedef union
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

} SC_DISPATCH_ERRSTAT;

/* Define the union U_SC_DISPATCH_REMAP_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sys_remap_vld         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISPATCH_REMAP_CTRL;

/* Define the union U_SC_SRAM_CTRL0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_ecc_en           : 1   ; /* [0]  */
        unsigned int    sram_ecc_err_trans    : 1   ; /* [1]  */
        unsigned int    sram_err_clr          : 1   ; /* [2]  */
        unsigned int    sram_err_cnt_clr      : 1   ; /* [3]  */
        unsigned int    sram_sglerr_addr_clr  : 1   ; /* [4]  */
        unsigned int    sram_mulerr_addr_clr  : 1   ; /* [5]  */
        unsigned int    sram_swap_en          : 1   ; /* [6]  */
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CTRL0;

/* Define the union U_SC_SRAM_CTRL1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_prot_ctrl_en     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CTRL1;

/* Define the union U_SC_SMMU_MEM_CTRL1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsel_hc_smmu          : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SMMU_MEM_CTRL1;

/* Define the union U_SC_SMMU_MEM_CTRL2 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    test_hc_smmu          : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SMMU_MEM_CTRL2;

/* Define the union U_SC_SRAM_MEM_CTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rashde_sram      : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_MEM_CTRL;

/* Define the union U_SC_PLLFCTRL0 */
typedef union
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

} SC_PLLFCTRL0;

/*Silicon Creation PLL公用寄存器*/
/* Define the union U_SC_PLLFCTRL0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll_refdiv           : 6   ; /* [5..0]  */
        unsigned int    pll_fbdiv            : 12  ; /* [17..6]  */
        unsigned int    pll_postdiv1         : 3   ; /* [20..18]  */
        unsigned int    pll_postdiv2         : 3   ; /* [23..21]  */
        unsigned int    pll_bypass           : 1   ; /* [24]  */
        unsigned int    pll_pd               : 1   ; /* [25]  */
        unsigned int    pll_dsmpd            : 1   ; /* [26]  */
        unsigned int    pll_dacpd            : 1   ; /* [27]  */
        unsigned int    pll_fout4phasepd     : 1   ; /* [28]  */
        unsigned int    pll_foutpostdivpd    : 1   ; /* [29]  */
        unsigned int    pll_foutvcopd        : 1   ; /* [30]  */
        unsigned int    reserved             : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SlicionCrePLLTRL;

/* Define the union U_SC_PLLFCTRL1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll0_frac             : 26  ; /* [25..0]  */
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL1;

/* Define the union U_SC_PLLFCTRL8 */
typedef union
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

} SC_PLLFCTRL8;

/* Define the union U_SC_PLLFCTRL9 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll4_frac             : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLLFCTRL9;

/* Define the union U_SC_PLL_CLK_BYPASS0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll4_peri_mode        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_CLK_BYPASS0;

/* Define the union U_SC_PLL_CLK_BYPASS1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pll0_bypass_external_n : 1   ; /* [0]  */
        unsigned int    pll1_bypass_external_n : 1   ; /* [1]  */
        unsigned int    reserved_0            : 2   ; /* [3..2]  */
        unsigned int    pll4_bypass_external_n : 1   ; /* [4]  */
        unsigned int    reserved_1            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_CLK_BYPASS1;

/* Define the union U_SC_CLK_SEL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpu_clk_source_sel    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 7   ; /* [7..1]  */
        unsigned int    gpio_db_clk_sel       : 1   ; /* [8]  */
        unsigned int    clk_div_sel           : 1   ; /* [9]  */
        unsigned int    reserved_1            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CLK_SEL;

/* Define the union U_SC_TSMC_PLLCTRL1 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsmc_pll1_refdiv      : 5   ; /* [4..0]  */
        unsigned int    tsmc_pll1_fbdiv       : 8   ; /* [12..5]  */
        unsigned int    tsmc_pll1_pd          : 1   ; /* [13]  */
        unsigned int    tsmc_pll1_bypass      : 1   ; /* [14]  */
        unsigned int    tsmc_pll1_postdiv     : 3   ; /* [17..15]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSMC_PLLCTRL1;

/*TSMC类型PLL公用接口*/
/* Define the union U_SC_TSMC_PLLCTRL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsmc_pll_refdiv      : 5   ; /* [4..0]  */
        unsigned int    tsmc_pll_fbdiv       : 8   ; /* [12..5]  */
        unsigned int    tsmc_pll_pd          : 1   ; /* [13]  */
        unsigned int    tsmc_pll_bypass      : 1   ; /* [14]  */
        unsigned int    tsmc_pll_postdiv     : 3   ; /* [17..15]  */
        unsigned int    reserved             : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSMC_PLLCTRL;

/* Define the union U_SC_TSMC_PLLCTRL4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsmc_pll4_refdiv      : 5   ; /* [4..0]  */
        unsigned int    tsmc_pll4_fbdiv       : 8   ; /* [12..5]  */
        unsigned int    tsmc_pll4_pd          : 1   ; /* [13]  */
        unsigned int    tsmc_pll4_bypass      : 1   ; /* [14]  */
        unsigned int    tsmc_pll4_postdiv     : 3   ; /* [17..15]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TSMC_PLLCTRL4;

/* Define the union U_SC_PLL_SEL */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [1..0]  */
        unsigned int    pll_sel               : 1   ; /* [2]  */
        unsigned int    reserved_1            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_SEL;

/* Define the union U_SC_SRAM_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sram_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CLK_ST;

/* Define the union U_SC_POE_CLK_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_poe_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_POE_CLK_ST;

/* Define the union U_SC_SRAM_RESET_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_srst_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_RESET_ST;

/* Define the union U_SC_POE_RESET_ST */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    poe_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_POE_RESET_ST;

/* Define the union U_SC_SRAM_ST0 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_single_err       : 1   ; /* [0]  */
        unsigned int    sram_double_err       : 1   ; /* [1]  */
        unsigned int    sram_multi_err        : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST0;

/* Define the union U_SC_SRAM_ST3 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_count_ecc_sglerr : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST3;

/* Define the union U_SC_SRAM_ST4 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_count_ecc_mulerr : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST4;

/* Define the union U_SC_SRAM_ST5 */
typedef union
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_dfx_dbg          : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST5;

#endif


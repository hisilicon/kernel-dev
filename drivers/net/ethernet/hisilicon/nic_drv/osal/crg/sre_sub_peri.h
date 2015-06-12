/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpaISWHqOHbuE99WnZz8Urp0KNb0WWl+qRa4EjbbH6IvvJ8uAZ7aN4Vd0NGho
uLjlYkbiU8e+l5PfMxBjpmtDK0/NS6KkTblkR5xTrHIyQ3bXlivPLge5aWzbwZCVXPK7/NRn
YG+28t00G8/fW6A6gLNgHOdLGPlrDisFVnUSf++n6f1HZwowgcvoO9DKKpc4Ig==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  °æÈ¨ËùÓÐ (C), 2011-2021, »ªÎª¼¼ÊõÓÐÏÞ¹«Ë¾

 ******************************************************************************
  ÎÄ ¼þ Ãû   : sre_sub_peri.h
  °æ ±¾ ºÅ   : ³õ¸å
  ×÷    Õß   : z00176027
  Éú³ÉÈÕÆÚ   : 2013Äê10ÔÂ17ÈÕ
  ×î½üÐÞ¸Ä   :
  ¹¦ÄÜÃèÊö   : p660×ÓÏµÍ³¸´Î»¼°½â¸´Î»Ïà¹ØÍ·ÎÄ¼þ
  º¯ÊýÁÐ±í   :
  ÐÞ¸ÄÀúÊ·   :
  1.ÈÕ    ÆÚ   : 2013Äê10ÔÂ17ÈÕ
    ×÷    Õß   : z00176027
    ÐÞ¸ÄÄÚÈÝ   : ´´½¨ÎÄ¼þ

******************************************************************************/
#ifndef __C_SRE_SUB_PERI_H__
#define __C_SRE_SUB_PERI_H__


#include "hrdCommon.h"
#include "hrdOs.h"
#include "osal_api.h"
#include "sre_crg_api.h"

#ifdef HRD_OS_LINUX
#include "iware_comm_kernel_api.h"
#endif



/*----------------------------------------------*
 * Íâ²¿±äÁ¿ËµÃ÷                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Íâ²¿º¯ÊýÔ­ÐÍËµÃ÷                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ÄÚ²¿º¯ÊýÔ­ÐÍËµÃ÷                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * È«¾Ö±äÁ¿                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * Ä£¿é¼¶±äÁ¿                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ³£Á¿¶¨Òå                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ºê¶¨Òå                                       *
 *----------------------------------------------*/

    /* peri_subctrl Ä£¿é¼Ä´æÆ÷»ùµØÖ· */
#define PERI_SUB_BASE                             (0x60000000)

#define PERI_SUB_BASE_SIZE                        (0x10000)

/******************************************************************************/
/*                      PHOSPHOR PERI_SUB Registers' Definitions                            */
/******************************************************************************/

#define PERI_SUB_SC_MMC_CLKMODE_CTRL_REG       (PERI_SUB_BASE + 0x100)  /* ÅäÖÃmmc²ÉÑùÊ±ÖÓ */
#define PERI_SUB_SC_NANDC_CLKMODE_CTRL_REG     (PERI_SUB_BASE + 0x104)  /* ÅäÖÃnandcÊ±ÖÓÑ¡Ôñ */
#define PERI_SUB_SC_TRACE_CLKMODE_CTRL_REG     (PERI_SUB_BASE + 0x108)  /* ÅäÖÃclk_traceinÊ±ÖÓÑ¡Ôñ */
#define PERI_SUB_SC_DMAC_CLK_EN_REG            (PERI_SUB_BASE + 0x300)  /* DMACÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_DMAC_CLK_DIS_REG           (PERI_SUB_BASE + 0x304)  /* DMACÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_NANDC_CLK_EN_REG           (PERI_SUB_BASE + 0x308)  /* NANDCÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_NANDC_CLK_DIS_REG          (PERI_SUB_BASE + 0x30C)  /* NANDCÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_MMC_CLK_EN_REG             (PERI_SUB_BASE + 0x310)  /* MMCÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_MMC_CLK_DIS_REG            (PERI_SUB_BASE + 0x314)  /* MMCÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_LOCALBUS_CLK_EN_REG        (PERI_SUB_BASE + 0x318)  /* LOCALBUSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_LOCALBUS_CLK_DIS_REG       (PERI_SUB_BASE + 0x31C)  /* LOCALBUSÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_GPIO_CLK_EN_REG            (PERI_SUB_BASE + 0x320)  /* GPIOÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_GPIO_CLK_DIS_REG           (PERI_SUB_BASE + 0x324)  /* GPIOÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_UART_CLK_EN_REG            (PERI_SUB_BASE + 0x328)  /* UARTÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_UART_CLK_DIS_REG           (PERI_SUB_BASE + 0x32C)  /* UARTÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_I2C_CLK_EN_REG             (PERI_SUB_BASE + 0x330)  /* I2CÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_I2C_CLK_DIS_REG            (PERI_SUB_BASE + 0x334)  /* I2CÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_MDIO_CLK_EN_REG            (PERI_SUB_BASE + 0x338)  /* MDIOÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_MDIO_CLK_DIS_REG           (PERI_SUB_BASE + 0x33C)  /* MDIOÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_DBG_CLK_EN_REG             (PERI_SUB_BASE + 0x348)  /* DBGÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_DBG_CLK_DIS_REG            (PERI_SUB_BASE + 0x34C)  /* DBGÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_TIMER_CLK_EN_REG           (PERI_SUB_BASE + 0x350)  /* TIMERÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_TIMER_CLK_DIS_REG          (PERI_SUB_BASE + 0x354)  /* TIMERÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_WATCHDOG_CLK_EN_REG        (PERI_SUB_BASE + 0x358)  /* WATCHDOGÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_WATCHDOG_CLK_DIS_REG       (PERI_SUB_BASE + 0x35C)  /* WATCHDOGÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_ITS_CLK_EN_REG             (PERI_SUB_BASE + 0x360)  /* ITSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_ITS_CLK_DIS_REG            (PERI_SUB_BASE + 0x364)  /* ITSÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_GIC_CLK_EN_REG             (PERI_SUB_BASE + 0x368)  /* GICÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PERI_SUB_SC_GIC_CLK_DIS_REG            (PERI_SUB_BASE + 0x36C)  /* GICÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PERI_SUB_SC_DMAC_RESET_REQ_REG         (PERI_SUB_BASE + 0xA00)  /* DMACÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_DMAC_RESET_DREQ_REG        (PERI_SUB_BASE + 0xA04)  /* DMACÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_NANDC_RESET_REQ_REG        (PERI_SUB_BASE + 0xA08)  /* NANDCÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_NANDC_RESET_DREQ_REG       (PERI_SUB_BASE + 0xA0C)  /* NANDCÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_MMC_RESET_REQ_REG          (PERI_SUB_BASE + 0xA10)  /* MMCÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_MMC_RESET_DREQ_REG         (PERI_SUB_BASE + 0xA14)  /* MMCÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_LOCALBUS_RESET_REQ_REG     (PERI_SUB_BASE + 0xA18)  /* LOCALBUSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_LOCALBUS_RESET_DREQ_REG    (PERI_SUB_BASE + 0xA1C)  /* LOCALBUSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_GPIO_RESET_REQ_REG         (PERI_SUB_BASE + 0xA20)  /* GPIOÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_GPIO_RESET_DREQ_REG        (PERI_SUB_BASE + 0xA24)  /* GPIOÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_UART_RESET_REQ_REG         (PERI_SUB_BASE + 0xA28)  /* UARTÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_UART_RESET_DREQ_REG        (PERI_SUB_BASE + 0xA2C)  /* UARTÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_I2C_RESET_REQ_REG          (PERI_SUB_BASE + 0xA30)  /* I2CÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_I2C_RESET_DREQ_REG         (PERI_SUB_BASE + 0xA34)  /* I2CÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_MDIO_RESET_REQ_REG         (PERI_SUB_BASE + 0xA38)  /* MDIOÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_MDIO_RESET_DREQ_REG        (PERI_SUB_BASE + 0xA3C)  /* MDIOÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_DBG_RESET_REQ_REG          (PERI_SUB_BASE + 0xA48)  /* DBGÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_DBG_RESET_DREQ_REG         (PERI_SUB_BASE + 0xA4C)  /* DBGÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_WATCHDOG_RESET_REQ_REG     (PERI_SUB_BASE + 0xA58)  /* WATCHDOGÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_WATCHDOG_RESET_DREQ_REG    (PERI_SUB_BASE + 0xA5C)  /* WATCHDOGÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_ITS_RESET_REQ_REG          (PERI_SUB_BASE + 0xA60)  /* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_ITS_RESET_DREQ_REG         (PERI_SUB_BASE + 0xA64)  /* ITSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_GIC_RESET_REQ_REG          (PERI_SUB_BASE + 0xA68)  /* GICÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_GIC_RESET_DREQ_REG         (PERI_SUB_BASE + 0xA6C)  /* GICÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_DISPATCH_DAW_EN_REG        (PERI_SUB_BASE + 0x1000) /* dispatch daw enÅäÖÃ */
#define PERI_SUB_SC_DISPATCH_DAW_ARRAY0_REG    (PERI_SUB_BASE + 0x1004) /* dispatch dawÅäÖÃÕóÁÐ0 */
#define PERI_SUB_SC_DISPATCH_DAW_ARRAY1_REG    (PERI_SUB_BASE + 0x1008) /* dispatch dawÅäÖÃÕóÁÐ1 */
#define PERI_SUB_SC_DISPATCH_DAW_ARRAY2_REG    (PERI_SUB_BASE + 0x100C) /* dispatch dawÅäÖÃÕóÁÐ2 */
#define PERI_SUB_SC_DISPATCH_DAW_ARRAY3_REG    (PERI_SUB_BASE + 0x1010) /* dispatch dawÅäÖÃÕóÁÐ3 */
#define PERI_SUB_SC_DISPATCH_DAW_ARRAY4_REG    (PERI_SUB_BASE + 0x1014) /* dispatch dawÅäÖÃÕóÁÐ4 */
#define PERI_SUB_SC_DISPATCH_DAW_ARRAY5_REG    (PERI_SUB_BASE + 0x1018) /* dispatch dawÅäÖÃÕóÁÐ5 */
#define PERI_SUB_SC_DISPATCH_DAW_ARRAY6_REG    (PERI_SUB_BASE + 0x101C) /* dispatch dawÅäÖÃÕóÁÐ6 */
#define PERI_SUB_SC_DISPATCH_DAW_ARRAY7_REG    (PERI_SUB_BASE + 0x1020) /* dispatch dawÅäÖÃÕóÁÐ7 */
#define PERI_SUB_SC_DISPATCH_RETRY_CONTROL_REG (PERI_SUB_BASE + 0x1030) /* dispatch retry¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_DISPATCH_INTMASK_REG       (PERI_SUB_BASE + 0x1100) /* dispatchµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
#define PERI_SUB_SC_DISPATCH_RAWINT_REG        (PERI_SUB_BASE + 0x1104) /* dispatchµÄÔ­Ê¼ÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_DISPATCH_INTSTAT_REG       (PERI_SUB_BASE + 0x1108) /* dispatchµÄÆÁ±ÎºóµÄÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_DISPATCH_INTCLR_REG        (PERI_SUB_BASE + 0x110C) /* dispatchµÄÖÐ¶ÏÇå³ý¼Ä´æÆ÷ */
#define PERI_SUB_SC_DISPATCH_ERRSTAT_REG       (PERI_SUB_BASE + 0x1110) /* dispatchµÄERR×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_REMAP_CTRL_REG             (PERI_SUB_BASE + 0x1200) /* subsysµÄÆô¶¯Remap¼Ä´æÆ÷ */
#define PERI_SUB_SC_EXTINT_CTRL_REG            (PERI_SUB_BASE + 0x2000) /* Íâ²¿ÖÐ¶Ï¼«ÐÔÅäÖÃ¼Ä´æÆ÷ */
#define PERI_SUB_SC_MDIO_CTRL_REG              (PERI_SUB_BASE + 0x2010) /* mdio¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_I2C_CTRL_REG               (PERI_SUB_BASE + 0x2020) /* i2c¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_VMID_CTRL0_REG             (PERI_SUB_BASE + 0x203C) /* VMID¿ØÖÆ¼Ä´æÆ÷0£¨DMAC0£© */
#define PERI_SUB_SC_VMID_CTRL1_REG             (PERI_SUB_BASE + 0x2040) /* VMID¿ØÖÆ¼Ä´æÆ÷1£¨DMAC1£© */
#define PERI_SUB_SC_VMID_CTRL2_REG             (PERI_SUB_BASE + 0x2044) /* VMID¿ØÖÆ¼Ä´æÆ÷2£¨TOP_CSSYS£© */
#define PERI_SUB_SC_VMID_CTRL3_REG             (PERI_SUB_BASE + 0x2048) /* VMID¿ØÖÆ¼Ä´æÆ÷3£¨MMC£© */
#define PERI_SUB_SC_VMID_CTRL4_REG             (PERI_SUB_BASE + 0x204C) /* VMID¿ØÖÆ¼Ä´æÆ÷4£¨NANDC£© */
#define PERI_SUB_SC_TIMER_CLKEN_CTRL0_REG      (PERI_SUB_BASE + 0x2050) /* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷0 */
#define PERI_SUB_SC_TIMER_CLKEN_CTRL1_REG      (PERI_SUB_BASE + 0x2054) /* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷1 */
#define PERI_SUB_SC_WDOG_CLKEN_CTRL_REG        (PERI_SUB_BASE + 0x2060) /* WDOGµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_FTE_MUX_CTRL_REG           (PERI_SUB_BASE + 0x2200) /* FTE¸´ÓÃÑ¡Ôñ¼Ä´æÆ÷ */
#define PERI_SUB_SC_MMC_ADDR_H_REG             (PERI_SUB_BASE + 0x2E00) /* MMC¸ß32Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
#define PERI_SUB_SC_DMAC0_ADDR_H_REG           (PERI_SUB_BASE + 0x2E04) /* DMAC0¸ß32Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
#define PERI_SUB_SC_DMAC1_ADDR_H_REG           (PERI_SUB_BASE + 0x2E08) /* DMAC1¸ß32Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
#define PERI_SUB_SC_CSSYS_ADDR_H_REG           (PERI_SUB_BASE + 0x2E0C) /* CSSYS¸ß24Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
#define PERI_SUB_SC_SMMU_MEM_CTRL1_REG         (PERI_SUB_BASE + 0x3004) /* smmu mem¿ØÖÆ¼Ä´æÆ÷1 */
#define PERI_SUB_SC_SMMU_MEM_CTRL2_REG         (PERI_SUB_BASE + 0x3008) /* smmu mem¿ØÖÆ¼Ä´æÆ÷2 */
#define PERI_SUB_SC_NANDC_MEM_CTRL_REG         (PERI_SUB_BASE + 0x3010) /* nandc mem¿ØÖÆ¼Ä´æÆ÷ */
#define PERI_SUB_SC_GIC_MEM_CTRL0_REG          (PERI_SUB_BASE + 0x3020) /* gic mem¿ØÖÆ¼Ä´æÆ÷0 */
#define PERI_SUB_SC_GIC_MEM_CTRL1_REG          (PERI_SUB_BASE + 0x3024) /* gic mem¿ØÖÆ¼Ä´æÆ÷1 */
#define PERI_SUB_SC_DMAC_MEM_CTRL0_REG         (PERI_SUB_BASE + 0x3030) /* dmac mem¿ØÖÆ¼Ä´æÆ÷1 */
#define PERI_SUB_SC_DMAC_MEM_CTRL1_REG         (PERI_SUB_BASE + 0x3034) /* dmac mem¿ØÖÆ¼Ä´æÆ÷2 */
#define PERI_SUB_SC_DMAC_CLK_ST_REG            (PERI_SUB_BASE + 0x5300) /* DMACÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_NANDC_CLK_ST_REG           (PERI_SUB_BASE + 0x5304) /* NANDCÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_MMC_CLK_ST_REG             (PERI_SUB_BASE + 0x5308) /* MMCÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_LOCALBUS_CLK_ST_REG        (PERI_SUB_BASE + 0x530C) /* LOCALBUSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_GPIO_CLK_ST_REG            (PERI_SUB_BASE + 0x5310) /* GPIOÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_UART_CLK_ST_REG            (PERI_SUB_BASE + 0x5314) /* UARTÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_I2C_CLK_ST_REG             (PERI_SUB_BASE + 0x5318) /* I2CÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_MDIO_CLK_ST_REG            (PERI_SUB_BASE + 0x531C) /* MDIOÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_DBG_CLK_ST_REG             (PERI_SUB_BASE + 0x5324) /* DBGÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_TIMER_CLK_ST_REG           (PERI_SUB_BASE + 0x5328) /* TIMERÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_WATCHDOG_CLK_ST_REG        (PERI_SUB_BASE + 0x532C) /* WADTHDOGÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_ITS_CLK_ST_REG             (PERI_SUB_BASE + 0x5330) /* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_GIC_CLK_ST_REG             (PERI_SUB_BASE + 0x5334) /* GICÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_DMAC_RESET_ST_REG          (PERI_SUB_BASE + 0x5A00) /* DMAC¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_NANDC_RESET_ST_REG         (PERI_SUB_BASE + 0x5A04) /* NANDC¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_MMC_RESET_ST_REG           (PERI_SUB_BASE + 0x5A08) /* MMC¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_LOCALBUS_RESET_ST_REG      (PERI_SUB_BASE + 0x5A0C) /* LOCALBUS¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_GPIO_RESET_ST_REG          (PERI_SUB_BASE + 0x5A10) /* GPIO¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_UART_RESET_ST_REG          (PERI_SUB_BASE + 0x5A14) /* UART¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_I2C_RESET_ST_REG           (PERI_SUB_BASE + 0x5A18) /* I2C¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_MDIO_RESET_ST_REG          (PERI_SUB_BASE + 0x5A1C) /* MDIO¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_DBG_RESET_ST_REG           (PERI_SUB_BASE + 0x5A24) /* DBG¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_WATCHDOG_RESET_ST_REG      (PERI_SUB_BASE + 0x5A2C) /* WATCHDOG¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_ITS_RESET_ST_REG           (PERI_SUB_BASE + 0x5A30) /* ITS¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_GIC_RESET_ST_REG           (PERI_SUB_BASE + 0x5A34) /* GIC¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PERI_SUB_SC_ECO_RSV0_REG               (PERI_SUB_BASE + 0x8000) /* ECO ¼Ä´æÆ÷0 */
#define PERI_SUB_SC_ECO_RSV1_REG               (PERI_SUB_BASE + 0x8004) /* ECO ¼Ä´æÆ÷1 */
#define PERI_SUB_SC_ECO_RSV2_REG               (PERI_SUB_BASE + 0x8008) /* ECO ¼Ä´æÆ÷2 */

/* poe_subctrl Ä£¿é¼Ä´æÆ÷»ùµØÖ· */
#define POE_SUB_BASE                             (0x80000000)
#define POE_SUB_BASE_SIZE                        (0x50000)

/******************************************************************************/
/*                      PHOSPHOR POE_SUB Registers' Definitions                            */
/******************************************************************************/
#define POE_SUB_SC_PLL_CLK_BYPASS1               (POE_SUB_BASE + 0x3044) /* ECO ¼Ä´æÆ÷2 */
#define POE_SUB_SC_CLK_SEL               (POE_SUB_BASE + 0x3048) /* ECO ¼Ä´æÆ÷2 */


#if(ENDNESS == ENDNESS_BIG)

/* Define the union U_SC_MMC_CLKMODE_CTRL_U */
/* ÅäÖÃmmc²ÉÑùÊ±ÖÓ */
/* 0x100 */
typedef union tagScMmcClkmodeCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mmc_card_clk_sel      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_CLKMODE_CTRL_U;

/* Define the union U_SC_NANDC_CLKMODE_CTRL_U */
/* ÅäÖÃnandcÊ±ÖÓÑ¡Ôñ */
/* 0x104 */
typedef union tagScNandcClkmodeCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    nandc0_clk_sel        : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_CLKMODE_CTRL_U;

/* Define the union U_SC_TRACE_CLKMODE_CTRL_U */
/* ÅäÖÃclk_traceinÊ±ÖÓÑ¡Ôñ */
/* 0x108 */
typedef union tagScTraceClkmodeCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    cs_tracein_clk_mode   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TRACE_CLKMODE_CTRL_U;

/* Define the union U_SC_DMAC_CLK_EN_U */
/* DMACÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x300 */
typedef union tagScDmacClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_dmac1_enb         : 1   ; /* [1]  */
        unsigned int    clk_dmac0_enb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_CLK_EN_U;

/* Define the union U_SC_DMAC_CLK_DIS_U */
/* DMACÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x304 */
typedef union tagScDmacClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_dmac1_dsb         : 1   ; /* [1]  */
        unsigned int    clk_dmac0_dsb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_CLK_DIS_U;

/* Define the union U_SC_NANDC_CLK_EN_U */
/* NANDCÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x308 */
typedef union tagScNandcClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_nandc0_enb        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_CLK_EN_U;

/* Define the union U_SC_NANDC_CLK_DIS_U */
/* NANDCÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x30C */
typedef union tagScNandcClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_nandc0_dsb        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_CLK_DIS_U;

/* Define the union U_SC_MMC_CLK_EN_U */
/* MMCÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x310 */
typedef union tagScMmcClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_mmc_card_enb      : 1   ; /* [1]  */
        unsigned int    clk_mmc_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_CLK_EN_U;

/* Define the union U_SC_MMC_CLK_DIS_U */
/* MMCÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x314 */
typedef union tagScMmcClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_mmc_card_dsb      : 1   ; /* [1]  */
        unsigned int    clk_mmc_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_CLK_DIS_U;

/* Define the union U_SC_LOCALBUS_CLK_EN_U */
/* LOCALBUSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x318 */
typedef union tagScLocalbusClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_localbus_enb      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_CLK_EN_U;

/* Define the union U_SC_LOCALBUS_CLK_DIS_U */
/* LOCALBUSÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x31C */
typedef union tagScLocalbusClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_localbus_dsb      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_CLK_DIS_U;

/* Define the union U_SC_GPIO_CLK_EN_U */
/* GPIOÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x320 */
typedef union tagScGpioClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    clk_gpio4_enb         : 1   ; /* [4]  */
        unsigned int    clk_gpio3_enb         : 1   ; /* [3]  */
        unsigned int    clk_gpio2_enb         : 1   ; /* [2]  */
        unsigned int    clk_gpio1_enb         : 1   ; /* [1]  */
        unsigned int    clk_gpio0_enb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_CLK_EN_U;

/* Define the union U_SC_GPIO_CLK_DIS_U */
/* GPIOÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x324 */
typedef union tagScGpioClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    clk_gpio4_dsb         : 1   ; /* [4]  */
        unsigned int    clk_gpio3_dsb         : 1   ; /* [3]  */
        unsigned int    clk_gpio2_dsb         : 1   ; /* [2]  */
        unsigned int    clk_gpio1_dsb         : 1   ; /* [1]  */
        unsigned int    clk_gpio0_dsb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_CLK_DIS_U;

/* Define the union U_SC_UART_CLK_EN_U */
/* UARTÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x328 */
typedef union tagScUartClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_uart1_enb         : 1   ; /* [1]  */
        unsigned int    clk_uart0_enb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_CLK_EN_U;

/* Define the union U_SC_UART_CLK_DIS_U */
/* UARTÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x32C */
typedef union tagScUartClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_uart1_dsb         : 1   ; /* [1]  */
        unsigned int    clk_uart0_dsb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_CLK_DIS_U;

/* Define the union U_SC_I2C_CLK_EN_U */
/* I2CÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x330 */
typedef union tagScI2cClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    clk_i2c7_enb          : 1   ; /* [7]  */
        unsigned int    clk_i2c6_enb          : 1   ; /* [6]  */
        unsigned int    clk_i2c5_enb          : 1   ; /* [5]  */
        unsigned int    clk_i2c4_enb          : 1   ; /* [4]  */
        unsigned int    clk_i2c3_enb          : 1   ; /* [3]  */
        unsigned int    clk_i2c2_enb          : 1   ; /* [2]  */
        unsigned int    clk_i2c1_enb          : 1   ; /* [1]  */
        unsigned int    clk_i2c0_enb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_EN_U;

/* Define the union U_SC_I2C_CLK_DIS_U */
/* I2CÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x334 */
typedef union tagScI2cClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    clk_i2c7_dsb          : 1   ; /* [7]  */
        unsigned int    clk_i2c6_dsb          : 1   ; /* [6]  */
        unsigned int    clk_i2c5_dsb          : 1   ; /* [5]  */
        unsigned int    clk_i2c4_dsb          : 1   ; /* [4]  */
        unsigned int    clk_i2c3_dsb          : 1   ; /* [3]  */
        unsigned int    clk_i2c2_dsb          : 1   ; /* [2]  */
        unsigned int    clk_i2c1_dsb          : 1   ; /* [1]  */
        unsigned int    clk_i2c0_dsb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_DIS_U;

/* Define the union U_SC_MDIO_CLK_EN_U */
/* MDIOÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x338 */
typedef union tagScMdioClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_mdio_enb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_CLK_EN_U;

/* Define the union U_SC_MDIO_CLK_DIS_U */
/* MDIOÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x33C */
typedef union tagScMdioClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_mdio_dsb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_CLK_DIS_U;

/* Define the union U_SC_DBG_CLK_EN_U */
/* DBGÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x348 */
typedef union tagScDbgClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_cs_tracein_enb    : 1   ; /* [1]  */
        unsigned int    clk_dbg_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_CLK_EN_U;

/* Define the union U_SC_DBG_CLK_DIS_U */
/* DBGÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x34C */
typedef union tagScDbgClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_cs_tracein_dsb    : 1   ; /* [1]  */
        unsigned int    clk_dbg_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_CLK_DIS_U;

/* Define the union U_SC_TIMER_CLK_EN_U */
/* TIMERÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x350 */
typedef union tagScTimerClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
        unsigned int    clk_timer23_enb       : 1   ; /* [23]  */
        unsigned int    clk_timer22_enb       : 1   ; /* [22]  */
        unsigned int    clk_timer21_enb       : 1   ; /* [21]  */
        unsigned int    clk_timer20_enb       : 1   ; /* [20]  */
        unsigned int    clk_timer19_enb       : 1   ; /* [19]  */
        unsigned int    clk_timer18_enb       : 1   ; /* [18]  */
        unsigned int    clk_timer17_enb       : 1   ; /* [17]  */
        unsigned int    clk_timer16_enb       : 1   ; /* [16]  */
        unsigned int    clk_timer15_enb       : 1   ; /* [15]  */
        unsigned int    clk_timer14_enb       : 1   ; /* [14]  */
        unsigned int    clk_timer13_enb       : 1   ; /* [13]  */
        unsigned int    clk_timer12_enb       : 1   ; /* [12]  */
        unsigned int    clk_timer11_enb       : 1   ; /* [11]  */
        unsigned int    clk_timer10_enb       : 1   ; /* [10]  */
        unsigned int    clk_timer9_enb        : 1   ; /* [9]  */
        unsigned int    clk_timer8_enb        : 1   ; /* [8]  */
        unsigned int    clk_timer7_enb        : 1   ; /* [7]  */
        unsigned int    clk_timer6_enb        : 1   ; /* [6]  */
        unsigned int    clk_timer5_enb        : 1   ; /* [5]  */
        unsigned int    clk_timer4_enb        : 1   ; /* [4]  */
        unsigned int    clk_timer3_enb        : 1   ; /* [3]  */
        unsigned int    clk_timer2_enb        : 1   ; /* [2]  */
        unsigned int    clk_timer1_enb        : 1   ; /* [1]  */
        unsigned int    clk_timer0_enb        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_EN_U;

/* Define the union U_SC_TIMER_CLK_DIS_U */
/* TIMERÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x354 */
typedef union tagScTimerClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
        unsigned int    clk_timer23_dsb       : 1   ; /* [23]  */
        unsigned int    clk_timer22_dsb       : 1   ; /* [22]  */
        unsigned int    clk_timer21_dsb       : 1   ; /* [21]  */
        unsigned int    clk_timer20_dsb       : 1   ; /* [20]  */
        unsigned int    clk_timer19_dsb       : 1   ; /* [19]  */
        unsigned int    clk_timer18_dsb       : 1   ; /* [18]  */
        unsigned int    clk_timer17_dsb       : 1   ; /* [17]  */
        unsigned int    clk_timer16_dsb       : 1   ; /* [16]  */
        unsigned int    clk_timer15_dsb       : 1   ; /* [15]  */
        unsigned int    clk_timer14_dsb       : 1   ; /* [14]  */
        unsigned int    clk_timer13_dsb       : 1   ; /* [13]  */
        unsigned int    clk_timer12_dsb       : 1   ; /* [12]  */
        unsigned int    clk_timer11_dsb       : 1   ; /* [11]  */
        unsigned int    clk_timer10_dsb       : 1   ; /* [10]  */
        unsigned int    clk_timer9_dsb        : 1   ; /* [9]  */
        unsigned int    clk_timer8_dsb        : 1   ; /* [8]  */
        unsigned int    clk_timer7_dsb        : 1   ; /* [7]  */
        unsigned int    clk_timer6_dsb        : 1   ; /* [6]  */
        unsigned int    clk_timer5_dsb        : 1   ; /* [5]  */
        unsigned int    clk_timer4_dsb        : 1   ; /* [4]  */
        unsigned int    clk_timer3_dsb        : 1   ; /* [3]  */
        unsigned int    clk_timer2_dsb        : 1   ; /* [2]  */
        unsigned int    clk_timer1_dsb        : 1   ; /* [1]  */
        unsigned int    clk_timer0_dsb        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_DIS_U;

/* Define the union U_SC_WATCHDOG_CLK_EN_U */
/* WATCHDOGÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x358 */
typedef union tagScWatchdogClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    clk_wdg15_enb         : 1   ; /* [15]  */
        unsigned int    clk_wdg14_enb         : 1   ; /* [14]  */
        unsigned int    clk_wdg13_enb         : 1   ; /* [13]  */
        unsigned int    clk_wdg12_enb         : 1   ; /* [12]  */
        unsigned int    clk_wdg11_enb         : 1   ; /* [11]  */
        unsigned int    clk_wdg10_enb         : 1   ; /* [10]  */
        unsigned int    clk_wdg9_enb          : 1   ; /* [9]  */
        unsigned int    clk_wdg8_enb          : 1   ; /* [8]  */
        unsigned int    clk_wdg7_enb          : 1   ; /* [7]  */
        unsigned int    clk_wdg6_enb          : 1   ; /* [6]  */
        unsigned int    clk_wdg5_enb          : 1   ; /* [5]  */
        unsigned int    clk_wdg4_enb          : 1   ; /* [4]  */
        unsigned int    clk_wdg3_enb          : 1   ; /* [3]  */
        unsigned int    clk_wdg2_enb          : 1   ; /* [2]  */
        unsigned int    clk_wdg1_enb          : 1   ; /* [1]  */
        unsigned int    clk_wdg0_enb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_CLK_EN_U;

/* Define the union U_SC_WATCHDOG_CLK_DIS_U */
/* WATCHDOGÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x35C */
typedef union tagScWatchdogClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    clk_wdg15_dsb         : 1   ; /* [15]  */
        unsigned int    clk_wdg14_dsb         : 1   ; /* [14]  */
        unsigned int    clk_wdg13_dsb         : 1   ; /* [13]  */
        unsigned int    clk_wdg12_dsb         : 1   ; /* [12]  */
        unsigned int    clk_wdg11_dsb         : 1   ; /* [11]  */
        unsigned int    clk_wdg10_dsb         : 1   ; /* [10]  */
        unsigned int    clk_wdg9_dsb          : 1   ; /* [9]  */
        unsigned int    clk_wdg8_dsb          : 1   ; /* [8]  */
        unsigned int    clk_wdg7_dsb          : 1   ; /* [7]  */
        unsigned int    clk_wdg6_dsb          : 1   ; /* [6]  */
        unsigned int    clk_wdg5_dsb          : 1   ; /* [5]  */
        unsigned int    clk_wdg4_dsb          : 1   ; /* [4]  */
        unsigned int    clk_wdg3_dsb          : 1   ; /* [3]  */
        unsigned int    clk_wdg2_dsb          : 1   ; /* [2]  */
        unsigned int    clk_wdg1_dsb          : 1   ; /* [1]  */
        unsigned int    clk_wdg0_dsb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_CLK_DIS_U;

/* Define the union U_SC_ITS_CLK_EN_U */
/* ITSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x360 */
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
/* ITSÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x364 */
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

/* Define the union U_SC_GIC_CLK_EN_U */
/* GICÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x368 */
typedef union tagScGicClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_gic_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_CLK_EN_U;

/* Define the union U_SC_GIC_CLK_DIS_U */
/* GICÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x36C */
typedef union tagScGicClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_gic_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_CLK_DIS_U;

/* Define the union U_SC_DMAC_RESET_REQ_U */
/* DMACÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA00 */
typedef union tagScDmacResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    dmac1_srst_req        : 1   ; /* [1]  */
        unsigned int    dmac0_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_RESET_REQ_U;

/* Define the union U_SC_DMAC_RESET_DREQ_U */
/* DMACÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA04 */
typedef union tagScDmacResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    dmac1_srst_dreq       : 1   ; /* [1]  */
        unsigned int    dmac0_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_RESET_DREQ_U;

/* Define the union U_SC_NANDC_RESET_REQ_U */
/* NANDCÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA08 */
typedef union tagScNandcResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    nandc0_srst_req       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_RESET_REQ_U;

/* Define the union U_SC_NANDC_RESET_DREQ_U */
/* NANDCÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA0C */
typedef union tagScNandcResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    nandc0_srst_dreq      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_RESET_DREQ_U;

/* Define the union U_SC_MMC_RESET_REQ_U */
/* MMCÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA10 */
typedef union tagScMmcResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mmc_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_RESET_REQ_U;

/* Define the union U_SC_MMC_RESET_DREQ_U */
/* MMCÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA14 */
typedef union tagScMmcResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mmc_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_RESET_DREQ_U;

/* Define the union U_SC_LOCALBUS_RESET_REQ_U */
/* LOCALBUSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA18 */
typedef union tagScLocalbusResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    localbus_srst_req     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_RESET_REQ_U;

/* Define the union U_SC_LOCALBUS_RESET_DREQ_U */
/* LOCALBUSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA1C */
typedef union tagScLocalbusResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    localbus_srst_dreq    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_RESET_DREQ_U;

/* Define the union U_SC_GPIO_RESET_REQ_U */
/* GPIOÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA20 */
typedef union tagScGpioResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    gpio4_srst_req        : 1   ; /* [4]  */
        unsigned int    gpio3_srst_req        : 1   ; /* [3]  */
        unsigned int    gpio2_srst_req        : 1   ; /* [2]  */
        unsigned int    gpio1_srst_req        : 1   ; /* [1]  */
        unsigned int    gpio0_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_RESET_REQ_U;

/* Define the union U_SC_GPIO_RESET_DREQ_U */
/* GPIOÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA24 */
typedef union tagScGpioResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    gpio4_srst_dreq       : 1   ; /* [4]  */
        unsigned int    gpio3_srst_dreq       : 1   ; /* [3]  */
        unsigned int    gpio2_srst_dreq       : 1   ; /* [2]  */
        unsigned int    gpio1_srst_dreq       : 1   ; /* [1]  */
        unsigned int    gpio0_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_RESET_DREQ_U;

/* Define the union U_SC_UART_RESET_REQ_U */
/* UARTÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA28 */
typedef union tagScUartResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    uart1_srst_req        : 1   ; /* [1]  */
        unsigned int    uart0_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_RESET_REQ_U;

/* Define the union U_SC_UART_RESET_DREQ_U */
/* UARTÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA2C */
typedef union tagScUartResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    uart1_srst_dreq       : 1   ; /* [1]  */
        unsigned int    uart0_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_RESET_DREQ_U;

/* Define the union U_SC_I2C_RESET_REQ_U */
/* I2CÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA30 */
typedef union tagScI2cResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    i2c7_srst_req         : 1   ; /* [7]  */
        unsigned int    i2c6_srst_req         : 1   ; /* [6]  */
        unsigned int    i2c5_srst_req         : 1   ; /* [5]  */
        unsigned int    i2c4_srst_req         : 1   ; /* [4]  */
        unsigned int    i2c3_srst_req         : 1   ; /* [3]  */
        unsigned int    i2c2_srst_req         : 1   ; /* [2]  */
        unsigned int    i2c1_srst_req         : 1   ; /* [1]  */
        unsigned int    i2c0_srst_req         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_REQ_U;

/* Define the union U_SC_I2C_RESET_DREQ_U */
/* I2CÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA34 */
typedef union tagScI2cResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    i2c7_srst_dreq        : 1   ; /* [7]  */
        unsigned int    i2c6_srst_dreq        : 1   ; /* [6]  */
        unsigned int    i2c5_srst_dreq        : 1   ; /* [5]  */
        unsigned int    i2c4_srst_dreq        : 1   ; /* [4]  */
        unsigned int    i2c3_srst_dreq        : 1   ; /* [3]  */
        unsigned int    i2c2_srst_dreq        : 1   ; /* [2]  */
        unsigned int    i2c1_srst_dreq        : 1   ; /* [1]  */
        unsigned int    i2c0_srst_dreq        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_DREQ_U;

/* Define the union U_SC_MDIO_RESET_REQ_U */
/* MDIOÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA38 */
typedef union tagScMdioResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mdio_srst_req         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_RESET_REQ_U;

/* Define the union U_SC_MDIO_RESET_DREQ_U */
/* MDIOÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA3C */
typedef union tagScMdioResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mdio_srst_dreq        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_RESET_DREQ_U;

/* Define the union U_SC_DBG_RESET_REQ_U */
/* DBGÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA48 */
typedef union tagScDbgResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    cs_tracein_srst_req   : 1   ; /* [1]  */
        unsigned int    dbg_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_RESET_REQ_U;

/* Define the union U_SC_DBG_RESET_DREQ_U */
/* DBGÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA4C */
typedef union tagScDbgResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    cs_tracein_srst_dreq  : 1   ; /* [1]  */
        unsigned int    dbg_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_RESET_DREQ_U;

/* Define the union U_SC_WATCHDOG_RESET_REQ_U */
/* WATCHDOGÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA58 */
typedef union tagScWatchdogResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    wdg15_srst_req        : 1   ; /* [15]  */
        unsigned int    wdg14_srst_req        : 1   ; /* [14]  */
        unsigned int    wdg13_srst_req        : 1   ; /* [13]  */
        unsigned int    wdg12_srst_req        : 1   ; /* [12]  */
        unsigned int    wdg11_srst_req        : 1   ; /* [11]  */
        unsigned int    wdg10_srst_req        : 1   ; /* [10]  */
        unsigned int    wdg9_srst_req         : 1   ; /* [9]  */
        unsigned int    wdg8_srst_req         : 1   ; /* [8]  */
        unsigned int    wdg7_srst_req         : 1   ; /* [7]  */
        unsigned int    wdg6_srst_req         : 1   ; /* [6]  */
        unsigned int    wdg5_srst_req         : 1   ; /* [5]  */
        unsigned int    wdg4_srst_req         : 1   ; /* [4]  */
        unsigned int    wdg3_srst_req         : 1   ; /* [3]  */
        unsigned int    wdg2_srst_req         : 1   ; /* [2]  */
        unsigned int    wdg1_srst_req         : 1   ; /* [1]  */
        unsigned int    wdg0_srst_req         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_RESET_REQ_U;

/* Define the union U_SC_WATCHDOG_RESET_DREQ_U */
/* WATCHDOGÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA5C */
typedef union tagScWatchdogResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    wdg15_srst_dreq       : 1   ; /* [15]  */
        unsigned int    wdg14_srst_dreq       : 1   ; /* [14]  */
        unsigned int    wdg13_srst_dreq       : 1   ; /* [13]  */
        unsigned int    wdg12_srst_dreq       : 1   ; /* [12]  */
        unsigned int    wdg11_srst_dreq       : 1   ; /* [11]  */
        unsigned int    wdg10_srst_dreq       : 1   ; /* [10]  */
        unsigned int    wdg9_srst_dreq        : 1   ; /* [9]  */
        unsigned int    wdg8_srst_dreq        : 1   ; /* [8]  */
        unsigned int    wdg7_srst_dreq        : 1   ; /* [7]  */
        unsigned int    wdg6_srst_dreq        : 1   ; /* [6]  */
        unsigned int    wdg5_srst_dreq        : 1   ; /* [5]  */
        unsigned int    wdg4_srst_dreq        : 1   ; /* [4]  */
        unsigned int    wdg3_srst_dreq        : 1   ; /* [3]  */
        unsigned int    wdg2_srst_dreq        : 1   ; /* [2]  */
        unsigned int    wdg1_srst_dreq        : 1   ; /* [1]  */
        unsigned int    wdg0_srst_dreq        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_RESET_DREQ_U;

/* Define the union U_SC_ITS_RESET_REQ_U */
/* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA60 */
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
/* ITSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA64 */
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

/* Define the union U_SC_GIC_RESET_REQ_U */
/* GICÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA68 */
typedef union tagScGicResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    gic_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_RESET_REQ_U;

/* Define the union U_SC_GIC_RESET_DREQ_U */
/* GICÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA6C */
typedef union tagScGicResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    gic_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_RESET_DREQ_U;

/* Define the union U_SC_DISPATCH_DAW_EN_U */
/* dispatch daw enÅäÖÃ */
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
/* dispatch dawÅäÖÃÕóÁÐ0 */
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
/* dispatch dawÅäÖÃÕóÁÐ1 */
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
/* dispatch dawÅäÖÃÕóÁÐ2 */
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
/* dispatch dawÅäÖÃÕóÁÐ3 */
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
/* dispatch dawÅäÖÃÕóÁÐ4 */
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
/* dispatch dawÅäÖÃÕóÁÐ5 */
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
/* dispatch dawÅäÖÃÕóÁÐ6 */
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
/* dispatch dawÅäÖÃÕóÁÐ7 */
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
/* dispatch retry¿ØÖÆ¼Ä´æÆ÷ */
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
/* dispatchµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
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
/* dispatchµÄÔ­Ê¼ÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
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
/* dispatchµÄÆÁ±ÎºóµÄÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
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
/* dispatchµÄÖÐ¶ÏÇå³ý¼Ä´æÆ÷ */
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
/* dispatchµÄERR×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_REMAP_CTRL_U */
/* subsysµÄÆô¶¯Remap¼Ä´æÆ÷ */
/* 0x1200 */
typedef union tagScRemapCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sys_remap_vld         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REMAP_CTRL_U;

/* Define the union U_SC_EXTINT_CTRL_U */
/* Íâ²¿ÖÐ¶Ï¼«ÐÔÅäÖÃ¼Ä´æÆ÷ */
/* 0x2000 */
typedef union tagScExtintCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ext_int7_polarity     : 1   ; /* [7]  */
        unsigned int    ext_int6_polarity     : 1   ; /* [6]  */
        unsigned int    ext_int5_polarity     : 1   ; /* [5]  */
        unsigned int    ext_int4_polarity     : 1   ; /* [4]  */
        unsigned int    ext_int3_polarity     : 1   ; /* [3]  */
        unsigned int    ext_int2_polarity     : 1   ; /* [2]  */
        unsigned int    ext_int1_polarity     : 1   ; /* [1]  */
        unsigned int    ext_int0_polarity     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_EXTINT_CTRL_U;

/* Define the union U_SC_MDIO_CTRL_U */
/* mdio¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2010 */
typedef union tagScMdioCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    mdio_intf_mode_port4  : 1   ; /* [4]  */
        unsigned int    mdio_intf_mode_port3  : 1   ; /* [3]  */
        unsigned int    mdio_intf_mode_port2  : 1   ; /* [2]  */
        unsigned int    mdio_intf_mode_port1  : 1   ; /* [1]  */
        unsigned int    mdio_intf_mode_port0  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_CTRL_U;

/* Define the union U_SC_I2C_CTRL_U */
/* i2c¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2020 */
typedef union tagScI2cCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
        unsigned int    i2c1_sda_cfg          : 1   ; /* [11]  */
        unsigned int    i2c1_dat_oe_cfg       : 1   ; /* [10]  */
        unsigned int    i2c1_dat_mux_sel      : 1   ; /* [9]  */
        unsigned int    i2c1_scl_cfg          : 1   ; /* [8]  */
        unsigned int    i2c1_clk_oe_cfg       : 1   ; /* [7]  */
        unsigned int    i2c1_clk_mux_sel      : 1   ; /* [6]  */
        unsigned int    i2c0_sda_cfg          : 1   ; /* [5]  */
        unsigned int    i2c0_dat_oe_cfg       : 1   ; /* [4]  */
        unsigned int    i2c0_dat_mux_sel      : 1   ; /* [3]  */
        unsigned int    i2c0_scl_cfg          : 1   ; /* [2]  */
        unsigned int    i2c0_clk_oe_cfg       : 1   ; /* [1]  */
        unsigned int    i2c0_clk_mux_sel      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CTRL_U;

/* Define the union U_SC_VMID_CTRL0_U */
/* VMID¿ØÖÆ¼Ä´æÆ÷0£¨DMAC0£© */
/* 0x203C */
typedef union tagScVmidCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    dmac0_vmid            : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL0_U;

/* Define the union U_SC_VMID_CTRL1_U */
/* VMID¿ØÖÆ¼Ä´æÆ÷1£¨DMAC1£© */
/* 0x2040 */
typedef union tagScVmidCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    dmac1_vmid            : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL1_U;

/* Define the union U_SC_VMID_CTRL2_U */
/* VMID¿ØÖÆ¼Ä´æÆ÷2£¨TOP_CSSYS£© */
/* 0x2044 */
typedef union tagScVmidCtrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    top_cssys_vmid        : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL2_U;

/* Define the union U_SC_VMID_CTRL3_U */
/* VMID¿ØÖÆ¼Ä´æÆ÷3£¨MMC£© */
/* 0x2048 */
typedef union tagScVmidCtrl3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    mmc_vmid              : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL3_U;

/* Define the union U_SC_VMID_CTRL4_U */
/* VMID¿ØÖÆ¼Ä´æÆ÷4£¨NANDC£© */
/* 0x204C */
typedef union tagScVmidCtrl4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    nandc_vmid            : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL4_U;

/* Define the union U_SC_TIMER_CLKEN_CTRL0_U */
/* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x2050 */
typedef union tagScTimerClkenCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    timeren7ov            : 1   ; /* [15]  */
        unsigned int    timeren7sel           : 1   ; /* [14]  */
        unsigned int    timeren6ov            : 1   ; /* [13]  */
        unsigned int    timeren6sel           : 1   ; /* [12]  */
        unsigned int    timeren5ov            : 1   ; /* [11]  */
        unsigned int    timeren5sel           : 1   ; /* [10]  */
        unsigned int    timeren4ov            : 1   ; /* [9]  */
        unsigned int    timeren4sel           : 1   ; /* [8]  */
        unsigned int    timeren3ov            : 1   ; /* [7]  */
        unsigned int    timeren3sel           : 1   ; /* [6]  */
        unsigned int    timeren2ov            : 1   ; /* [5]  */
        unsigned int    timeren2sel           : 1   ; /* [4]  */
        unsigned int    timeren1ov            : 1   ; /* [3]  */
        unsigned int    timeren1sel           : 1   ; /* [2]  */
        unsigned int    timeren0ov            : 1   ; /* [1]  */
        unsigned int    timeren0sel           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLKEN_CTRL0_U;

/* Define the union U_SC_TIMER_CLKEN_CTRL1_U */
/* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x2054 */
typedef union tagScTimerClkenCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    timeren23ov           : 1   ; /* [31]  */
        unsigned int    timeren23sel          : 1   ; /* [30]  */
        unsigned int    timeren22ov           : 1   ; /* [29]  */
        unsigned int    timeren22sel          : 1   ; /* [28]  */
        unsigned int    timeren21ov           : 1   ; /* [27]  */
        unsigned int    timeren21sel          : 1   ; /* [26]  */
        unsigned int    timeren20ov           : 1   ; /* [25]  */
        unsigned int    timeren20sel          : 1   ; /* [24]  */
        unsigned int    timeren19ov           : 1   ; /* [23]  */
        unsigned int    timeren19sel          : 1   ; /* [22]  */
        unsigned int    timeren18ov           : 1   ; /* [21]  */
        unsigned int    timeren18sel          : 1   ; /* [20]  */
        unsigned int    timeren17ov           : 1   ; /* [19]  */
        unsigned int    timeren17sel          : 1   ; /* [18]  */
        unsigned int    timeren16ov           : 1   ; /* [17]  */
        unsigned int    timeren16sel          : 1   ; /* [16]  */
        unsigned int    timeren15ov           : 1   ; /* [15]  */
        unsigned int    timeren15sel          : 1   ; /* [14]  */
        unsigned int    timeren14ov           : 1   ; /* [13]  */
        unsigned int    timeren14sel          : 1   ; /* [12]  */
        unsigned int    timeren13ov           : 1   ; /* [11]  */
        unsigned int    timeren13sel          : 1   ; /* [10]  */
        unsigned int    timeren12ov           : 1   ; /* [9]  */
        unsigned int    timeren12sel          : 1   ; /* [8]  */
        unsigned int    timeren11ov           : 1   ; /* [7]  */
        unsigned int    timeren11sel          : 1   ; /* [6]  */
        unsigned int    timeren10ov           : 1   ; /* [5]  */
        unsigned int    timeren10sel          : 1   ; /* [4]  */
        unsigned int    timeren9ov            : 1   ; /* [3]  */
        unsigned int    timeren9sel           : 1   ; /* [2]  */
        unsigned int    timeren8ov            : 1   ; /* [1]  */
        unsigned int    timeren8sel           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLKEN_CTRL1_U;

/* Define the union U_SC_WDOG_CLKEN_CTRL_U */
/* WDOGµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2060 */
typedef union tagScWdogClkenCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    wdogen15ov            : 1   ; /* [15]  */
        unsigned int    wdogen14ov            : 1   ; /* [14]  */
        unsigned int    wdogen13ov            : 1   ; /* [13]  */
        unsigned int    wdogen12ov            : 1   ; /* [12]  */
        unsigned int    wdogen11ov            : 1   ; /* [11]  */
        unsigned int    wdogen10ov            : 1   ; /* [10]  */
        unsigned int    wdogen9ov             : 1   ; /* [9]  */
        unsigned int    wdogen8ov             : 1   ; /* [8]  */
        unsigned int    wdogen7ov             : 1   ; /* [7]  */
        unsigned int    wdogen6ov             : 1   ; /* [6]  */
        unsigned int    wdogen5ov             : 1   ; /* [5]  */
        unsigned int    wdogen4ov             : 1   ; /* [4]  */
        unsigned int    wdogen3ov             : 1   ; /* [3]  */
        unsigned int    wdogen2ov             : 1   ; /* [2]  */
        unsigned int    wdogen1ov             : 1   ; /* [1]  */
        unsigned int    wdogen0ov             : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDOG_CLKEN_CTRL_U;

/* Define the union U_SC_FTE_MUX_CTRL_U */
/* FTE¸´ÓÃÑ¡Ôñ¼Ä´æÆ÷ */
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

/* Define the union U_SC_CSSYS_ADDR_H_U */
/* CSSYS¸ß24Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
/* 0x2E0C */
typedef union tagScCssysAddrH
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
        unsigned int    cssys_addr_h          : 24  ; /* [23..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CSSYS_ADDR_H_U;

/* Define the union U_SC_SMMU_MEM_CTRL1_U */
/* smmu mem¿ØÖÆ¼Ä´æÆ÷1 */
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
/* smmu mem¿ØÖÆ¼Ä´æÆ÷2 */
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

/* Define the union U_SC_NANDC_MEM_CTRL_U */
/* nandc mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3010 */
typedef union tagScNandcMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rashde_nandc     : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_MEM_CTRL_U;

/* Define the union U_SC_GIC_MEM_CTRL0_U */
/* gic mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3020 */
typedef union tagScGicMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    tsel_hc_gic           : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_MEM_CTRL0_U;

/* Define the union U_SC_GIC_MEM_CTRL1_U */
/* gic mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3024 */
typedef union tagScGicMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    test_hc_gic           : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_MEM_CTRL1_U;

/* Define the union U_SC_DMAC_MEM_CTRL0_U */
/* dmac mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3030 */
typedef union tagScDmacMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    tsel_hc_dmac          : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_MEM_CTRL0_U;

/* Define the union U_SC_DMAC_MEM_CTRL1_U */
/* dmac mem¿ØÖÆ¼Ä´æÆ÷2 */
/* 0x3034 */
typedef union tagScDmacMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    test_hc_dmac          : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_MEM_CTRL1_U;

/* Define the union U_SC_DMAC_CLK_ST_U */
/* DMACÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5300 */
typedef union tagScDmacClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_dmac1_st          : 1   ; /* [1]  */
        unsigned int    clk_dmac0_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_CLK_ST_U;

/* Define the union U_SC_NANDC_CLK_ST_U */
/* NANDCÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5304 */
typedef union tagScNandcClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_nandc0_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_CLK_ST_U;

/* Define the union U_SC_MMC_CLK_ST_U */
/* MMCÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5308 */
typedef union tagScMmcClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_mmc_card_st       : 1   ; /* [1]  */
        unsigned int    clk_mmc_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_CLK_ST_U;

/* Define the union U_SC_LOCALBUS_CLK_ST_U */
/* LOCALBUSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x530C */
typedef union tagScLocalbusClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_localbus_st       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_CLK_ST_U;

/* Define the union U_SC_GPIO_CLK_ST_U */
/* GPIOÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5310 */
typedef union tagScGpioClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    clk_gpio4_st          : 1   ; /* [4]  */
        unsigned int    clk_gpio3_st          : 1   ; /* [3]  */
        unsigned int    clk_gpio2_st          : 1   ; /* [2]  */
        unsigned int    clk_gpio1_st          : 1   ; /* [1]  */
        unsigned int    clk_gpio0_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_CLK_ST_U;

/* Define the union U_SC_UART_CLK_ST_U */
/* UARTÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5314 */
typedef union tagScUartClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_uart1_st          : 1   ; /* [1]  */
        unsigned int    clk_uart0_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_CLK_ST_U;

/* Define the union U_SC_I2C_CLK_ST_U */
/* I2CÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5318 */
typedef union tagScI2cClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    clk_i2c7_st           : 1   ; /* [7]  */
        unsigned int    clk_i2c6_st           : 1   ; /* [6]  */
        unsigned int    clk_i2c5_st           : 1   ; /* [5]  */
        unsigned int    clk_i2c4_st           : 1   ; /* [4]  */
        unsigned int    clk_i2c3_st           : 1   ; /* [3]  */
        unsigned int    clk_i2c2_st           : 1   ; /* [2]  */
        unsigned int    clk_i2c1_st           : 1   ; /* [1]  */
        unsigned int    clk_i2c0_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_ST_U;

/* Define the union U_SC_MDIO_CLK_ST_U */
/* MDIOÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x531C */
typedef union tagScMdioClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_mdio_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_CLK_ST_U;

/* Define the union U_SC_DBG_CLK_ST_U */
/* DBGÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5324 */
typedef union tagScDbgClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_tracein_st        : 1   ; /* [1]  */
        unsigned int    clk_dbg_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_CLK_ST_U;

/* Define the union U_SC_TIMER_CLK_ST_U */
/* TIMERÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5328 */
typedef union tagScTimerClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
        unsigned int    clk_timer23_st        : 1   ; /* [23]  */
        unsigned int    clk_timer22_st        : 1   ; /* [22]  */
        unsigned int    clk_timer21_st        : 1   ; /* [21]  */
        unsigned int    clk_timer20_st        : 1   ; /* [20]  */
        unsigned int    clk_timer19_st        : 1   ; /* [19]  */
        unsigned int    clk_timer18_st        : 1   ; /* [18]  */
        unsigned int    clk_timer17_st        : 1   ; /* [17]  */
        unsigned int    clk_timer16_st        : 1   ; /* [16]  */
        unsigned int    clk_timer15_st        : 1   ; /* [15]  */
        unsigned int    clk_timer14_st        : 1   ; /* [14]  */
        unsigned int    clk_timer13_st        : 1   ; /* [13]  */
        unsigned int    clk_timer12_st        : 1   ; /* [12]  */
        unsigned int    clk_timer11_st        : 1   ; /* [11]  */
        unsigned int    clk_timer10_st        : 1   ; /* [10]  */
        unsigned int    clk_timer9_st         : 1   ; /* [9]  */
        unsigned int    clk_timer8_st         : 1   ; /* [8]  */
        unsigned int    clk_timer7_st         : 1   ; /* [7]  */
        unsigned int    clk_timer6_st         : 1   ; /* [6]  */
        unsigned int    clk_timer5_st         : 1   ; /* [5]  */
        unsigned int    clk_timer4_st         : 1   ; /* [4]  */
        unsigned int    clk_timer3_st         : 1   ; /* [3]  */
        unsigned int    clk_timer2_st         : 1   ; /* [2]  */
        unsigned int    clk_timer1_st         : 1   ; /* [1]  */
        unsigned int    clk_timer0_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_ST_U;

/* Define the union U_SC_WATCHDOG_CLK_ST_U */
/* WADTHDOGÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x532C */
typedef union tagScWatchdogClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    clk_wdg15_st          : 1   ; /* [15]  */
        unsigned int    clk_wdg14_st          : 1   ; /* [14]  */
        unsigned int    clk_wdg13_st          : 1   ; /* [13]  */
        unsigned int    clk_wdg12_st          : 1   ; /* [12]  */
        unsigned int    clk_wdg11_st          : 1   ; /* [11]  */
        unsigned int    clk_wdg10_st          : 1   ; /* [10]  */
        unsigned int    clk_wdg9_st           : 1   ; /* [9]  */
        unsigned int    clk_wdg8_st           : 1   ; /* [8]  */
        unsigned int    clk_wdg7_st           : 1   ; /* [7]  */
        unsigned int    clk_wdg6_st           : 1   ; /* [6]  */
        unsigned int    clk_wdg5_st           : 1   ; /* [5]  */
        unsigned int    clk_wdg4_st           : 1   ; /* [4]  */
        unsigned int    clk_wdg3_st           : 1   ; /* [3]  */
        unsigned int    clk_wdg2_st           : 1   ; /* [2]  */
        unsigned int    clk_wdg1_st           : 1   ; /* [1]  */
        unsigned int    clk_wdg0_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_CLK_ST_U;

/* Define the union U_SC_ITS_CLK_ST_U */
/* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5330 */
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

/* Define the union U_SC_GIC_CLK_ST_U */
/* GICÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5334 */
typedef union tagScGicClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_gic_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_CLK_ST_U;

/* Define the union U_SC_DMAC_RESET_ST_U */
/* DMAC¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A00 */
typedef union tagScDmacResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    dmac1_srst_st         : 1   ; /* [1]  */
        unsigned int    dmac0_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_RESET_ST_U;

/* Define the union U_SC_NANDC_RESET_ST_U */
/* NANDC¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A04 */
typedef union tagScNandcResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    nandc0_srst_st        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_RESET_ST_U;

/* Define the union U_SC_MMC_RESET_ST_U */
/* MMC¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A08 */
typedef union tagScMmcResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mmc_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_RESET_ST_U;

/* Define the union U_SC_LOCALBUS_RESET_ST_U */
/* LOCALBUS¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A0C */
typedef union tagScLocalbusResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    localbus_srst_st      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_RESET_ST_U;

/* Define the union U_SC_GPIO_RESET_ST_U */
/* GPIO¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A10 */
typedef union tagScGpioResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    gpio4_srst_st         : 1   ; /* [4]  */
        unsigned int    gpio3_srst_st         : 1   ; /* [3]  */
        unsigned int    gpio2_srst_st         : 1   ; /* [2]  */
        unsigned int    gpio1_srst_st         : 1   ; /* [1]  */
        unsigned int    gpio0_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_RESET_ST_U;

/* Define the union U_SC_UART_RESET_ST_U */
/* UART¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A14 */
typedef union tagScUartResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    uart1_srst_st         : 1   ; /* [1]  */
        unsigned int    uart0_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_RESET_ST_U;

/* Define the union U_SC_I2C_RESET_ST_U */
/* I2C¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A18 */
typedef union tagScI2cResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    i2c7_srst_st          : 1   ; /* [7]  */
        unsigned int    i2c6_srst_st          : 1   ; /* [6]  */
        unsigned int    i2c5_srst_st          : 1   ; /* [5]  */
        unsigned int    i2c4_srst_st          : 1   ; /* [4]  */
        unsigned int    i2c3_srst_st          : 1   ; /* [3]  */
        unsigned int    i2c2_srst_st          : 1   ; /* [2]  */
        unsigned int    i2c1_srst_st          : 1   ; /* [1]  */
        unsigned int    i2c0_srst_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_ST_U;

/* Define the union U_SC_MDIO_RESET_ST_U */
/* MDIO¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A1C */
typedef union tagScMdioResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mdio_srst_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_RESET_ST_U;

/* Define the union U_SC_DBG_RESET_ST_U */
/* DBG¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A24 */
typedef union tagScDbgResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    cs_tracein_srst_st    : 1   ; /* [1]  */
        unsigned int    dbg_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_RESET_ST_U;

/* Define the union U_SC_WATCHDOG_RESET_ST_U */
/* WATCHDOG¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A2C */
typedef union tagScWatchdogResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    wdg15_srst_st         : 1   ; /* [15]  */
        unsigned int    wdg14_srst_st         : 1   ; /* [14]  */
        unsigned int    wdg13_srst_st         : 1   ; /* [13]  */
        unsigned int    wdg12_srst_st         : 1   ; /* [12]  */
        unsigned int    wdg11_srst_st         : 1   ; /* [11]  */
        unsigned int    wdg10_srst_st         : 1   ; /* [10]  */
        unsigned int    wdg9_srst_st          : 1   ; /* [9]  */
        unsigned int    wdg8_srst_st          : 1   ; /* [8]  */
        unsigned int    wdg7_srst_st          : 1   ; /* [7]  */
        unsigned int    wdg6_srst_st          : 1   ; /* [6]  */
        unsigned int    wdg5_srst_st          : 1   ; /* [5]  */
        unsigned int    wdg4_srst_st          : 1   ; /* [4]  */
        unsigned int    wdg3_srst_st          : 1   ; /* [3]  */
        unsigned int    wdg2_srst_st          : 1   ; /* [2]  */
        unsigned int    wdg1_srst_st          : 1   ; /* [1]  */
        unsigned int    wdg0_srst_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_RESET_ST_U;

/* Define the union U_SC_ITS_RESET_ST_U */
/* ITS¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A30 */
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

/* Define the union U_SC_GIC_RESET_ST_U */
/* GIC¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A34 */
typedef union tagScGicResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    gic_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_RESET_ST_U;


#else

/* Define the union U_SC_MMC_CLKMODE_CTRL_U */
/* ÅäÖÃmmc²ÉÑùÊ±ÖÓ */
/* 0x100 */
typedef union tagScMmcClkmodeCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mmc_card_clk_sel      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_CLKMODE_CTRL_U;

/* Define the union U_SC_NANDC_CLKMODE_CTRL_U */
/* ÅäÖÃnandcÊ±ÖÓÑ¡Ôñ */
/* 0x104 */
typedef union tagScNandcClkmodeCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nandc0_clk_sel        : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_CLKMODE_CTRL_U;

/* Define the union U_SC_TRACE_CLKMODE_CTRL_U */
/* ÅäÖÃclk_traceinÊ±ÖÓÑ¡Ôñ */
/* 0x108 */
typedef union tagScTraceClkmodeCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cs_tracein_clk_mode   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TRACE_CLKMODE_CTRL_U;

/* Define the union U_SC_DMAC_CLK_EN_U */
/* DMACÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x300 */
typedef union tagScDmacClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_dmac0_enb         : 1   ; /* [0]  */
        unsigned int    clk_dmac1_enb         : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_CLK_EN_U;

/* Define the union U_SC_DMAC_CLK_DIS_U */
/* DMACÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x304 */
typedef union tagScDmacClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_dmac0_dsb         : 1   ; /* [0]  */
        unsigned int    clk_dmac1_dsb         : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_CLK_DIS_U;

/* Define the union U_SC_NANDC_CLK_EN_U */
/* NANDCÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x308 */
typedef union tagScNandcClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_nandc0_enb        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_CLK_EN_U;

/* Define the union U_SC_NANDC_CLK_DIS_U */
/* NANDCÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x30C */
typedef union tagScNandcClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_nandc0_dsb        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_CLK_DIS_U;

/* Define the union U_SC_MMC_CLK_EN_U */
/* MMCÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x310 */
typedef union tagScMmcClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_mmc_enb           : 1   ; /* [0]  */
        unsigned int    clk_mmc_card_enb      : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_CLK_EN_U;

/* Define the union U_SC_MMC_CLK_DIS_U */
/* MMCÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x314 */
typedef union tagScMmcClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_mmc_dsb           : 1   ; /* [0]  */
        unsigned int    clk_mmc_card_dsb      : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_CLK_DIS_U;

/* Define the union U_SC_LOCALBUS_CLK_EN_U */
/* LOCALBUSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x318 */
typedef union tagScLocalbusClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_localbus_enb      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_CLK_EN_U;

/* Define the union U_SC_LOCALBUS_CLK_DIS_U */
/* LOCALBUSÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x31C */
typedef union tagScLocalbusClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_localbus_dsb      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_CLK_DIS_U;

/* Define the union U_SC_GPIO_CLK_EN_U */
/* GPIOÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x320 */
typedef union tagScGpioClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gpio0_enb         : 1   ; /* [0]  */
        unsigned int    clk_gpio1_enb         : 1   ; /* [1]  */
        unsigned int    clk_gpio2_enb         : 1   ; /* [2]  */
        unsigned int    clk_gpio3_enb         : 1   ; /* [3]  */
        unsigned int    clk_gpio4_enb         : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_CLK_EN_U;

/* Define the union U_SC_GPIO_CLK_DIS_U */
/* GPIOÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x324 */
typedef union tagScGpioClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gpio0_dsb         : 1   ; /* [0]  */
        unsigned int    clk_gpio1_dsb         : 1   ; /* [1]  */
        unsigned int    clk_gpio2_dsb         : 1   ; /* [2]  */
        unsigned int    clk_gpio3_dsb         : 1   ; /* [3]  */
        unsigned int    clk_gpio4_dsb         : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_CLK_DIS_U;

/* Define the union U_SC_UART_CLK_EN_U */
/* UARTÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x328 */
typedef union tagScUartClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_uart0_enb         : 1   ; /* [0]  */
        unsigned int    clk_uart1_enb         : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_CLK_EN_U;

/* Define the union U_SC_UART_CLK_DIS_U */
/* UARTÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x32C */
typedef union tagScUartClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_uart0_dsb         : 1   ; /* [0]  */
        unsigned int    clk_uart1_dsb         : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_CLK_DIS_U;

/* Define the union U_SC_I2C_CLK_EN_U */
/* I2CÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x330 */
typedef union tagScI2cClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_i2c0_enb          : 1   ; /* [0]  */
        unsigned int    clk_i2c1_enb          : 1   ; /* [1]  */
        unsigned int    clk_i2c2_enb          : 1   ; /* [2]  */
        unsigned int    clk_i2c3_enb          : 1   ; /* [3]  */
        unsigned int    clk_i2c4_enb          : 1   ; /* [4]  */
        unsigned int    clk_i2c5_enb          : 1   ; /* [5]  */
        unsigned int    clk_i2c6_enb          : 1   ; /* [6]  */
        unsigned int    clk_i2c7_enb          : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_EN_U;

/* Define the union U_SC_I2C_CLK_DIS_U */
/* I2CÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x334 */
typedef union tagScI2cClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_i2c0_dsb          : 1   ; /* [0]  */
        unsigned int    clk_i2c1_dsb          : 1   ; /* [1]  */
        unsigned int    clk_i2c2_dsb          : 1   ; /* [2]  */
        unsigned int    clk_i2c3_dsb          : 1   ; /* [3]  */
        unsigned int    clk_i2c4_dsb          : 1   ; /* [4]  */
        unsigned int    clk_i2c5_dsb          : 1   ; /* [5]  */
        unsigned int    clk_i2c6_dsb          : 1   ; /* [6]  */
        unsigned int    clk_i2c7_dsb          : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_DIS_U;

/* Define the union U_SC_MDIO_CLK_EN_U */
/* MDIOÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x338 */
typedef union tagScMdioClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_mdio_enb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_CLK_EN_U;

/* Define the union U_SC_MDIO_CLK_DIS_U */
/* MDIOÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x33C */
typedef union tagScMdioClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_mdio_dsb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_CLK_DIS_U;

/* Define the union U_SC_DBG_CLK_EN_U */
/* DBGÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x348 */
typedef union tagScDbgClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_dbg_enb           : 1   ; /* [0]  */
        unsigned int    clk_cs_tracein_enb    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_CLK_EN_U;

/* Define the union U_SC_DBG_CLK_DIS_U */
/* DBGÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x34C */
typedef union tagScDbgClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_dbg_dsb           : 1   ; /* [0]  */
        unsigned int    clk_cs_tracein_dsb    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_CLK_DIS_U;

/* Define the union U_SC_TIMER_CLK_EN_U */
/* TIMERÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x350 */
typedef union tagScTimerClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_timer0_enb        : 1   ; /* [0]  */
        unsigned int    clk_timer1_enb        : 1   ; /* [1]  */
        unsigned int    clk_timer2_enb        : 1   ; /* [2]  */
        unsigned int    clk_timer3_enb        : 1   ; /* [3]  */
        unsigned int    clk_timer4_enb        : 1   ; /* [4]  */
        unsigned int    clk_timer5_enb        : 1   ; /* [5]  */
        unsigned int    clk_timer6_enb        : 1   ; /* [6]  */
        unsigned int    clk_timer7_enb        : 1   ; /* [7]  */
        unsigned int    clk_timer8_enb        : 1   ; /* [8]  */
        unsigned int    clk_timer9_enb        : 1   ; /* [9]  */
        unsigned int    clk_timer10_enb       : 1   ; /* [10]  */
        unsigned int    clk_timer11_enb       : 1   ; /* [11]  */
        unsigned int    clk_timer12_enb       : 1   ; /* [12]  */
        unsigned int    clk_timer13_enb       : 1   ; /* [13]  */
        unsigned int    clk_timer14_enb       : 1   ; /* [14]  */
        unsigned int    clk_timer15_enb       : 1   ; /* [15]  */
        unsigned int    clk_timer16_enb       : 1   ; /* [16]  */
        unsigned int    clk_timer17_enb       : 1   ; /* [17]  */
        unsigned int    clk_timer18_enb       : 1   ; /* [18]  */
        unsigned int    clk_timer19_enb       : 1   ; /* [19]  */
        unsigned int    clk_timer20_enb       : 1   ; /* [20]  */
        unsigned int    clk_timer21_enb       : 1   ; /* [21]  */
        unsigned int    clk_timer22_enb       : 1   ; /* [22]  */
        unsigned int    clk_timer23_enb       : 1   ; /* [23]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_EN_U;

/* Define the union U_SC_TIMER_CLK_DIS_U */
/* TIMERÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x354 */
typedef union tagScTimerClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_timer0_dsb        : 1   ; /* [0]  */
        unsigned int    clk_timer1_dsb        : 1   ; /* [1]  */
        unsigned int    clk_timer2_dsb        : 1   ; /* [2]  */
        unsigned int    clk_timer3_dsb        : 1   ; /* [3]  */
        unsigned int    clk_timer4_dsb        : 1   ; /* [4]  */
        unsigned int    clk_timer5_dsb        : 1   ; /* [5]  */
        unsigned int    clk_timer6_dsb        : 1   ; /* [6]  */
        unsigned int    clk_timer7_dsb        : 1   ; /* [7]  */
        unsigned int    clk_timer8_dsb        : 1   ; /* [8]  */
        unsigned int    clk_timer9_dsb        : 1   ; /* [9]  */
        unsigned int    clk_timer10_dsb       : 1   ; /* [10]  */
        unsigned int    clk_timer11_dsb       : 1   ; /* [11]  */
        unsigned int    clk_timer12_dsb       : 1   ; /* [12]  */
        unsigned int    clk_timer13_dsb       : 1   ; /* [13]  */
        unsigned int    clk_timer14_dsb       : 1   ; /* [14]  */
        unsigned int    clk_timer15_dsb       : 1   ; /* [15]  */
        unsigned int    clk_timer16_dsb       : 1   ; /* [16]  */
        unsigned int    clk_timer17_dsb       : 1   ; /* [17]  */
        unsigned int    clk_timer18_dsb       : 1   ; /* [18]  */
        unsigned int    clk_timer19_dsb       : 1   ; /* [19]  */
        unsigned int    clk_timer20_dsb       : 1   ; /* [20]  */
        unsigned int    clk_timer21_dsb       : 1   ; /* [21]  */
        unsigned int    clk_timer22_dsb       : 1   ; /* [22]  */
        unsigned int    clk_timer23_dsb       : 1   ; /* [23]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_DIS_U;

/* Define the union U_SC_WATCHDOG_CLK_EN_U */
/* WATCHDOGÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x358 */
typedef union tagScWatchdogClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_wdg0_enb          : 1   ; /* [0]  */
        unsigned int    clk_wdg1_enb          : 1   ; /* [1]  */
        unsigned int    clk_wdg2_enb          : 1   ; /* [2]  */
        unsigned int    clk_wdg3_enb          : 1   ; /* [3]  */
        unsigned int    clk_wdg4_enb          : 1   ; /* [4]  */
        unsigned int    clk_wdg5_enb          : 1   ; /* [5]  */
        unsigned int    clk_wdg6_enb          : 1   ; /* [6]  */
        unsigned int    clk_wdg7_enb          : 1   ; /* [7]  */
        unsigned int    clk_wdg8_enb          : 1   ; /* [8]  */
        unsigned int    clk_wdg9_enb          : 1   ; /* [9]  */
        unsigned int    clk_wdg10_enb         : 1   ; /* [10]  */
        unsigned int    clk_wdg11_enb         : 1   ; /* [11]  */
        unsigned int    clk_wdg12_enb         : 1   ; /* [12]  */
        unsigned int    clk_wdg13_enb         : 1   ; /* [13]  */
        unsigned int    clk_wdg14_enb         : 1   ; /* [14]  */
        unsigned int    clk_wdg15_enb         : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_CLK_EN_U;

/* Define the union U_SC_WATCHDOG_CLK_DIS_U */
/* WATCHDOGÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x35C */
typedef union tagScWatchdogClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_wdg0_dsb          : 1   ; /* [0]  */
        unsigned int    clk_wdg1_dsb          : 1   ; /* [1]  */
        unsigned int    clk_wdg2_dsb          : 1   ; /* [2]  */
        unsigned int    clk_wdg3_dsb          : 1   ; /* [3]  */
        unsigned int    clk_wdg4_dsb          : 1   ; /* [4]  */
        unsigned int    clk_wdg5_dsb          : 1   ; /* [5]  */
        unsigned int    clk_wdg6_dsb          : 1   ; /* [6]  */
        unsigned int    clk_wdg7_dsb          : 1   ; /* [7]  */
        unsigned int    clk_wdg8_dsb          : 1   ; /* [8]  */
        unsigned int    clk_wdg9_dsb          : 1   ; /* [9]  */
        unsigned int    clk_wdg10_dsb         : 1   ; /* [10]  */
        unsigned int    clk_wdg11_dsb         : 1   ; /* [11]  */
        unsigned int    clk_wdg12_dsb         : 1   ; /* [12]  */
        unsigned int    clk_wdg13_dsb         : 1   ; /* [13]  */
        unsigned int    clk_wdg14_dsb         : 1   ; /* [14]  */
        unsigned int    clk_wdg15_dsb         : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_CLK_DIS_U;

/* Define the union U_SC_ITS_CLK_EN_U */
/* ITSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x360 */
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
/* ITSÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x364 */
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

/* Define the union U_SC_GIC_CLK_EN_U */
/* GICÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x368 */
typedef union tagScGicClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gic_enb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_CLK_EN_U;

/* Define the union U_SC_GIC_CLK_DIS_U */
/* GICÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x36C */
typedef union tagScGicClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gic_dsb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_CLK_DIS_U;

/* Define the union U_SC_DMAC_RESET_REQ_U */
/* DMACÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA00 */
typedef union tagScDmacResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dmac0_srst_req        : 1   ; /* [0]  */
        unsigned int    dmac1_srst_req        : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_RESET_REQ_U;

/* Define the union U_SC_DMAC_RESET_DREQ_U */
/* DMACÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA04 */
typedef union tagScDmacResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dmac0_srst_dreq       : 1   ; /* [0]  */
        unsigned int    dmac1_srst_dreq       : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_RESET_DREQ_U;

/* Define the union U_SC_NANDC_RESET_REQ_U */
/* NANDCÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA08 */
typedef union tagScNandcResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nandc0_srst_req       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_RESET_REQ_U;

/* Define the union U_SC_NANDC_RESET_DREQ_U */
/* NANDCÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA0C */
typedef union tagScNandcResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nandc0_srst_dreq      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_RESET_DREQ_U;

/* Define the union U_SC_MMC_RESET_REQ_U */
/* MMCÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA10 */
typedef union tagScMmcResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mmc_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_RESET_REQ_U;

/* Define the union U_SC_MMC_RESET_DREQ_U */
/* MMCÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA14 */
typedef union tagScMmcResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mmc_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_RESET_DREQ_U;

/* Define the union U_SC_LOCALBUS_RESET_REQ_U */
/* LOCALBUSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA18 */
typedef union tagScLocalbusResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    localbus_srst_req     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_RESET_REQ_U;

/* Define the union U_SC_LOCALBUS_RESET_DREQ_U */
/* LOCALBUSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA1C */
typedef union tagScLocalbusResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    localbus_srst_dreq    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_RESET_DREQ_U;

/* Define the union U_SC_GPIO_RESET_REQ_U */
/* GPIOÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA20 */
typedef union tagScGpioResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gpio0_srst_req        : 1   ; /* [0]  */
        unsigned int    gpio1_srst_req        : 1   ; /* [1]  */
        unsigned int    gpio2_srst_req        : 1   ; /* [2]  */
        unsigned int    gpio3_srst_req        : 1   ; /* [3]  */
        unsigned int    gpio4_srst_req        : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_RESET_REQ_U;

/* Define the union U_SC_GPIO_RESET_DREQ_U */
/* GPIOÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA24 */
typedef union tagScGpioResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gpio0_srst_dreq       : 1   ; /* [0]  */
        unsigned int    gpio1_srst_dreq       : 1   ; /* [1]  */
        unsigned int    gpio2_srst_dreq       : 1   ; /* [2]  */
        unsigned int    gpio3_srst_dreq       : 1   ; /* [3]  */
        unsigned int    gpio4_srst_dreq       : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_RESET_DREQ_U;

/* Define the union U_SC_UART_RESET_REQ_U */
/* UARTÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA28 */
typedef union tagScUartResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    uart0_srst_req        : 1   ; /* [0]  */
        unsigned int    uart1_srst_req        : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_RESET_REQ_U;

/* Define the union U_SC_UART_RESET_DREQ_U */
/* UARTÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA2C */
typedef union tagScUartResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    uart0_srst_dreq       : 1   ; /* [0]  */
        unsigned int    uart1_srst_dreq       : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_RESET_DREQ_U;

/* Define the union U_SC_I2C_RESET_REQ_U */
/* I2CÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA30 */
typedef union tagScI2cResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c0_srst_req         : 1   ; /* [0]  */
        unsigned int    i2c1_srst_req         : 1   ; /* [1]  */
        unsigned int    i2c2_srst_req         : 1   ; /* [2]  */
        unsigned int    i2c3_srst_req         : 1   ; /* [3]  */
        unsigned int    i2c4_srst_req         : 1   ; /* [4]  */
        unsigned int    i2c5_srst_req         : 1   ; /* [5]  */
        unsigned int    i2c6_srst_req         : 1   ; /* [6]  */
        unsigned int    i2c7_srst_req         : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_REQ_U;

/* Define the union U_SC_I2C_RESET_DREQ_U */
/* I2CÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA34 */
typedef union tagScI2cResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c0_srst_dreq        : 1   ; /* [0]  */
        unsigned int    i2c1_srst_dreq        : 1   ; /* [1]  */
        unsigned int    i2c2_srst_dreq        : 1   ; /* [2]  */
        unsigned int    i2c3_srst_dreq        : 1   ; /* [3]  */
        unsigned int    i2c4_srst_dreq        : 1   ; /* [4]  */
        unsigned int    i2c5_srst_dreq        : 1   ; /* [5]  */
        unsigned int    i2c6_srst_dreq        : 1   ; /* [6]  */
        unsigned int    i2c7_srst_dreq        : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_DREQ_U;

/* Define the union U_SC_MDIO_RESET_REQ_U */
/* MDIOÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA38 */
typedef union tagScMdioResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mdio_srst_req         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_RESET_REQ_U;

/* Define the union U_SC_MDIO_RESET_DREQ_U */
/* MDIOÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA3C */
typedef union tagScMdioResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mdio_srst_dreq        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_RESET_DREQ_U;

/* Define the union U_SC_DBG_RESET_REQ_U */
/* DBGÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA48 */
typedef union tagScDbgResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dbg_srst_req          : 1   ; /* [0]  */
        unsigned int    cs_tracein_srst_req   : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_RESET_REQ_U;

/* Define the union U_SC_DBG_RESET_DREQ_U */
/* DBGÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA4C */
typedef union tagScDbgResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dbg_srst_dreq         : 1   ; /* [0]  */
        unsigned int    cs_tracein_srst_dreq  : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_RESET_DREQ_U;

/* Define the union U_SC_WATCHDOG_RESET_REQ_U */
/* WATCHDOGÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA58 */
typedef union tagScWatchdogResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wdg0_srst_req         : 1   ; /* [0]  */
        unsigned int    wdg1_srst_req         : 1   ; /* [1]  */
        unsigned int    wdg2_srst_req         : 1   ; /* [2]  */
        unsigned int    wdg3_srst_req         : 1   ; /* [3]  */
        unsigned int    wdg4_srst_req         : 1   ; /* [4]  */
        unsigned int    wdg5_srst_req         : 1   ; /* [5]  */
        unsigned int    wdg6_srst_req         : 1   ; /* [6]  */
        unsigned int    wdg7_srst_req         : 1   ; /* [7]  */
        unsigned int    wdg8_srst_req         : 1   ; /* [8]  */
        unsigned int    wdg9_srst_req         : 1   ; /* [9]  */
        unsigned int    wdg10_srst_req        : 1   ; /* [10]  */
        unsigned int    wdg11_srst_req        : 1   ; /* [11]  */
        unsigned int    wdg12_srst_req        : 1   ; /* [12]  */
        unsigned int    wdg13_srst_req        : 1   ; /* [13]  */
        unsigned int    wdg14_srst_req        : 1   ; /* [14]  */
        unsigned int    wdg15_srst_req        : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_RESET_REQ_U;

/* Define the union U_SC_WATCHDOG_RESET_DREQ_U */
/* WATCHDOGÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA5C */
typedef union tagScWatchdogResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wdg0_srst_dreq        : 1   ; /* [0]  */
        unsigned int    wdg1_srst_dreq        : 1   ; /* [1]  */
        unsigned int    wdg2_srst_dreq        : 1   ; /* [2]  */
        unsigned int    wdg3_srst_dreq        : 1   ; /* [3]  */
        unsigned int    wdg4_srst_dreq        : 1   ; /* [4]  */
        unsigned int    wdg5_srst_dreq        : 1   ; /* [5]  */
        unsigned int    wdg6_srst_dreq        : 1   ; /* [6]  */
        unsigned int    wdg7_srst_dreq        : 1   ; /* [7]  */
        unsigned int    wdg8_srst_dreq        : 1   ; /* [8]  */
        unsigned int    wdg9_srst_dreq        : 1   ; /* [9]  */
        unsigned int    wdg10_srst_dreq       : 1   ; /* [10]  */
        unsigned int    wdg11_srst_dreq       : 1   ; /* [11]  */
        unsigned int    wdg12_srst_dreq       : 1   ; /* [12]  */
        unsigned int    wdg13_srst_dreq       : 1   ; /* [13]  */
        unsigned int    wdg14_srst_dreq       : 1   ; /* [14]  */
        unsigned int    wdg15_srst_dreq       : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_RESET_DREQ_U;

/* Define the union U_SC_ITS_RESET_REQ_U */
/* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA60 */
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
/* ITSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA64 */
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

/* Define the union U_SC_GIC_RESET_REQ_U */
/* GICÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA68 */
typedef union tagScGicResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gic_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_RESET_REQ_U;

/* Define the union U_SC_GIC_RESET_DREQ_U */
/* GICÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA6C */
typedef union tagScGicResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gic_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_RESET_DREQ_U;

/* Define the union U_SC_DISPATCH_DAW_EN_U */
/* dispatch daw enÅäÖÃ */
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
/* dispatch dawÅäÖÃÕóÁÐ0 */
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
/* dispatch dawÅäÖÃÕóÁÐ1 */
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
/* dispatch dawÅäÖÃÕóÁÐ2 */
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
/* dispatch dawÅäÖÃÕóÁÐ3 */
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
/* dispatch dawÅäÖÃÕóÁÐ4 */
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
/* dispatch dawÅäÖÃÕóÁÐ5 */
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
/* dispatch dawÅäÖÃÕóÁÐ6 */
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
/* dispatch dawÅäÖÃÕóÁÐ7 */
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
/* dispatch retry¿ØÖÆ¼Ä´æÆ÷ */
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
/* dispatchµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
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
/* dispatchµÄÔ­Ê¼ÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
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
/* dispatchµÄÆÁ±ÎºóµÄÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
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
/* dispatchµÄÖÐ¶ÏÇå³ý¼Ä´æÆ÷ */
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
/* dispatchµÄERR×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_REMAP_CTRL_U */
/* subsysµÄÆô¶¯Remap¼Ä´æÆ÷ */
/* 0x1200 */
typedef union tagScRemapCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sys_remap_vld         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REMAP_CTRL_U;

/* Define the union U_SC_EXTINT_CTRL_U */
/* Íâ²¿ÖÐ¶Ï¼«ÐÔÅäÖÃ¼Ä´æÆ÷ */
/* 0x2000 */
typedef union tagScExtintCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ext_int0_polarity     : 1   ; /* [0]  */
        unsigned int    ext_int1_polarity     : 1   ; /* [1]  */
        unsigned int    ext_int2_polarity     : 1   ; /* [2]  */
        unsigned int    ext_int3_polarity     : 1   ; /* [3]  */
        unsigned int    ext_int4_polarity     : 1   ; /* [4]  */
        unsigned int    ext_int5_polarity     : 1   ; /* [5]  */
        unsigned int    ext_int6_polarity     : 1   ; /* [6]  */
        unsigned int    ext_int7_polarity     : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_EXTINT_CTRL_U;

/* Define the union U_SC_MDIO_CTRL_U */
/* mdio¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2010 */
typedef union tagScMdioCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mdio_intf_mode_port0  : 1   ; /* [0]  */
        unsigned int    mdio_intf_mode_port1  : 1   ; /* [1]  */
        unsigned int    mdio_intf_mode_port2  : 1   ; /* [2]  */
        unsigned int    mdio_intf_mode_port3  : 1   ; /* [3]  */
        unsigned int    mdio_intf_mode_port4  : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_CTRL_U;

/* Define the union U_SC_I2C_CTRL_U */
/* i2c¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2020 */
typedef union tagScI2cCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c0_clk_mux_sel      : 1   ; /* [0]  */
        unsigned int    i2c0_clk_oe_cfg       : 1   ; /* [1]  */
        unsigned int    i2c0_scl_cfg          : 1   ; /* [2]  */
        unsigned int    i2c0_dat_mux_sel      : 1   ; /* [3]  */
        unsigned int    i2c0_dat_oe_cfg       : 1   ; /* [4]  */
        unsigned int    i2c0_sda_cfg          : 1   ; /* [5]  */
        unsigned int    i2c1_clk_mux_sel      : 1   ; /* [6]  */
        unsigned int    i2c1_clk_oe_cfg       : 1   ; /* [7]  */
        unsigned int    i2c1_scl_cfg          : 1   ; /* [8]  */
        unsigned int    i2c1_dat_mux_sel      : 1   ; /* [9]  */
        unsigned int    i2c1_dat_oe_cfg       : 1   ; /* [10]  */
        unsigned int    i2c1_sda_cfg          : 1   ; /* [11]  */
        unsigned int    reserved_0            : 20  ; /* [31..12]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CTRL_U;

/* Define the union U_SC_VMID_CTRL0_U */
/* VMID¿ØÖÆ¼Ä´æÆ÷0£¨DMAC0£© */
/* 0x203C */
typedef union tagScVmidCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dmac0_vmid            : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL0_U;

/* Define the union U_SC_VMID_CTRL1_U */
/* VMID¿ØÖÆ¼Ä´æÆ÷1£¨DMAC1£© */
/* 0x2040 */
typedef union tagScVmidCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dmac1_vmid            : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL1_U;

/* Define the union U_SC_VMID_CTRL2_U */
/* VMID¿ØÖÆ¼Ä´æÆ÷2£¨TOP_CSSYS£© */
/* 0x2044 */
typedef union tagScVmidCtrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    top_cssys_vmid        : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL2_U;

/* Define the union U_SC_VMID_CTRL3_U */
/* VMID¿ØÖÆ¼Ä´æÆ÷3£¨MMC£© */
/* 0x2048 */
typedef union tagScVmidCtrl3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mmc_vmid              : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL3_U;

/* Define the union U_SC_VMID_CTRL4_U */
/* VMID¿ØÖÆ¼Ä´æÆ÷4£¨NANDC£© */
/* 0x204C */
typedef union tagScVmidCtrl4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nandc_vmid            : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL4_U;

/* Define the union U_SC_TIMER_CLKEN_CTRL0_U */
/* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x2050 */
typedef union tagScTimerClkenCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    timeren0sel           : 1   ; /* [0]  */
        unsigned int    timeren0ov            : 1   ; /* [1]  */
        unsigned int    timeren1sel           : 1   ; /* [2]  */
        unsigned int    timeren1ov            : 1   ; /* [3]  */
        unsigned int    timeren2sel           : 1   ; /* [4]  */
        unsigned int    timeren2ov            : 1   ; /* [5]  */
        unsigned int    timeren3sel           : 1   ; /* [6]  */
        unsigned int    timeren3ov            : 1   ; /* [7]  */
        unsigned int    timeren4sel           : 1   ; /* [8]  */
        unsigned int    timeren4ov            : 1   ; /* [9]  */
        unsigned int    timeren5sel           : 1   ; /* [10]  */
        unsigned int    timeren5ov            : 1   ; /* [11]  */
        unsigned int    timeren6sel           : 1   ; /* [12]  */
        unsigned int    timeren6ov            : 1   ; /* [13]  */
        unsigned int    timeren7sel           : 1   ; /* [14]  */
        unsigned int    timeren7ov            : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLKEN_CTRL0_U;

/* Define the union U_SC_TIMER_CLKEN_CTRL1_U */
/* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x2054 */
typedef union tagScTimerClkenCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    timeren8sel           : 1   ; /* [0]  */
        unsigned int    timeren8ov            : 1   ; /* [1]  */
        unsigned int    timeren9sel           : 1   ; /* [2]  */
        unsigned int    timeren9ov            : 1   ; /* [3]  */
        unsigned int    timeren10sel          : 1   ; /* [4]  */
        unsigned int    timeren10ov           : 1   ; /* [5]  */
        unsigned int    timeren11sel          : 1   ; /* [6]  */
        unsigned int    timeren11ov           : 1   ; /* [7]  */
        unsigned int    timeren12sel          : 1   ; /* [8]  */
        unsigned int    timeren12ov           : 1   ; /* [9]  */
        unsigned int    timeren13sel          : 1   ; /* [10]  */
        unsigned int    timeren13ov           : 1   ; /* [11]  */
        unsigned int    timeren14sel          : 1   ; /* [12]  */
        unsigned int    timeren14ov           : 1   ; /* [13]  */
        unsigned int    timeren15sel          : 1   ; /* [14]  */
        unsigned int    timeren15ov           : 1   ; /* [15]  */
        unsigned int    timeren16sel          : 1   ; /* [16]  */
        unsigned int    timeren16ov           : 1   ; /* [17]  */
        unsigned int    timeren17sel          : 1   ; /* [18]  */
        unsigned int    timeren17ov           : 1   ; /* [19]  */
        unsigned int    timeren18sel          : 1   ; /* [20]  */
        unsigned int    timeren18ov           : 1   ; /* [21]  */
        unsigned int    timeren19sel          : 1   ; /* [22]  */
        unsigned int    timeren19ov           : 1   ; /* [23]  */
        unsigned int    timeren20sel          : 1   ; /* [24]  */
        unsigned int    timeren20ov           : 1   ; /* [25]  */
        unsigned int    timeren21sel          : 1   ; /* [26]  */
        unsigned int    timeren21ov           : 1   ; /* [27]  */
        unsigned int    timeren22sel          : 1   ; /* [28]  */
        unsigned int    timeren22ov           : 1   ; /* [29]  */
        unsigned int    timeren23sel          : 1   ; /* [30]  */
        unsigned int    timeren23ov           : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLKEN_CTRL1_U;

/* Define the union U_SC_WDOG_CLKEN_CTRL_U */
/* WDOGµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2060 */
typedef union tagScWdogClkenCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wdogen0ov             : 1   ; /* [0]  */
        unsigned int    wdogen1ov             : 1   ; /* [1]  */
        unsigned int    wdogen2ov             : 1   ; /* [2]  */
        unsigned int    wdogen3ov             : 1   ; /* [3]  */
        unsigned int    wdogen4ov             : 1   ; /* [4]  */
        unsigned int    wdogen5ov             : 1   ; /* [5]  */
        unsigned int    wdogen6ov             : 1   ; /* [6]  */
        unsigned int    wdogen7ov             : 1   ; /* [7]  */
        unsigned int    wdogen8ov             : 1   ; /* [8]  */
        unsigned int    wdogen9ov             : 1   ; /* [9]  */
        unsigned int    wdogen10ov            : 1   ; /* [10]  */
        unsigned int    wdogen11ov            : 1   ; /* [11]  */
        unsigned int    wdogen12ov            : 1   ; /* [12]  */
        unsigned int    wdogen13ov            : 1   ; /* [13]  */
        unsigned int    wdogen14ov            : 1   ; /* [14]  */
        unsigned int    wdogen15ov            : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDOG_CLKEN_CTRL_U;

/* Define the union U_SC_FTE_MUX_CTRL_U */
/* FTE¸´ÓÃÑ¡Ôñ¼Ä´æÆ÷ */
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

/* Define the union U_SC_CSSYS_ADDR_H_U */
/* CSSYS¸ß24Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
/* 0x2E0C */
typedef union tagScCssysAddrH
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cssys_addr_h          : 24  ; /* [23..0]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CSSYS_ADDR_H_U;

/* Define the union U_SC_SMMU_MEM_CTRL1_U */
/* smmu mem¿ØÖÆ¼Ä´æÆ÷1 */
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
/* smmu mem¿ØÖÆ¼Ä´æÆ÷2 */
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

/* Define the union U_SC_NANDC_MEM_CTRL_U */
/* nandc mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3010 */
typedef union tagScNandcMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rashde_nandc     : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_MEM_CTRL_U;

/* Define the union U_SC_GIC_MEM_CTRL0_U */
/* gic mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3020 */
typedef union tagScGicMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsel_hc_gic           : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_MEM_CTRL0_U;

/* Define the union U_SC_GIC_MEM_CTRL1_U */
/* gic mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3024 */
typedef union tagScGicMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    test_hc_gic           : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_MEM_CTRL1_U;

/* Define the union U_SC_DMAC_MEM_CTRL0_U */
/* dmac mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3030 */
typedef union tagScDmacMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tsel_hc_dmac          : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_MEM_CTRL0_U;

/* Define the union U_SC_DMAC_MEM_CTRL1_U */
/* dmac mem¿ØÖÆ¼Ä´æÆ÷2 */
/* 0x3034 */
typedef union tagScDmacMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    test_hc_dmac          : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_MEM_CTRL1_U;

/* Define the union U_SC_DMAC_CLK_ST_U */
/* DMACÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5300 */
typedef union tagScDmacClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_dmac0_st          : 1   ; /* [0]  */
        unsigned int    clk_dmac1_st          : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_CLK_ST_U;

/* Define the union U_SC_NANDC_CLK_ST_U */
/* NANDCÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5304 */
typedef union tagScNandcClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_nandc0_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_CLK_ST_U;

/* Define the union U_SC_MMC_CLK_ST_U */
/* MMCÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5308 */
typedef union tagScMmcClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_mmc_st            : 1   ; /* [0]  */
        unsigned int    clk_mmc_card_st       : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_CLK_ST_U;

/* Define the union U_SC_LOCALBUS_CLK_ST_U */
/* LOCALBUSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x530C */
typedef union tagScLocalbusClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_localbus_st       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_CLK_ST_U;

/* Define the union U_SC_GPIO_CLK_ST_U */
/* GPIOÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5310 */
typedef union tagScGpioClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gpio0_st          : 1   ; /* [0]  */
        unsigned int    clk_gpio1_st          : 1   ; /* [1]  */
        unsigned int    clk_gpio2_st          : 1   ; /* [2]  */
        unsigned int    clk_gpio3_st          : 1   ; /* [3]  */
        unsigned int    clk_gpio4_st          : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_CLK_ST_U;

/* Define the union U_SC_UART_CLK_ST_U */
/* UARTÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5314 */
typedef union tagScUartClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_uart0_st          : 1   ; /* [0]  */
        unsigned int    clk_uart1_st          : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_CLK_ST_U;

/* Define the union U_SC_I2C_CLK_ST_U */
/* I2CÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5318 */
typedef union tagScI2cClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_i2c0_st           : 1   ; /* [0]  */
        unsigned int    clk_i2c1_st           : 1   ; /* [1]  */
        unsigned int    clk_i2c2_st           : 1   ; /* [2]  */
        unsigned int    clk_i2c3_st           : 1   ; /* [3]  */
        unsigned int    clk_i2c4_st           : 1   ; /* [4]  */
        unsigned int    clk_i2c5_st           : 1   ; /* [5]  */
        unsigned int    clk_i2c6_st           : 1   ; /* [6]  */
        unsigned int    clk_i2c7_st           : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_ST_U;

/* Define the union U_SC_MDIO_CLK_ST_U */
/* MDIOÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x531C */
typedef union tagScMdioClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_mdio_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_CLK_ST_U;

/* Define the union U_SC_DBG_CLK_ST_U */
/* DBGÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5324 */
typedef union tagScDbgClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_dbg_st            : 1   ; /* [0]  */
        unsigned int    clk_tracein_st        : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_CLK_ST_U;

/* Define the union U_SC_TIMER_CLK_ST_U */
/* TIMERÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5328 */
typedef union tagScTimerClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_timer0_st         : 1   ; /* [0]  */
        unsigned int    clk_timer1_st         : 1   ; /* [1]  */
        unsigned int    clk_timer2_st         : 1   ; /* [2]  */
        unsigned int    clk_timer3_st         : 1   ; /* [3]  */
        unsigned int    clk_timer4_st         : 1   ; /* [4]  */
        unsigned int    clk_timer5_st         : 1   ; /* [5]  */
        unsigned int    clk_timer6_st         : 1   ; /* [6]  */
        unsigned int    clk_timer7_st         : 1   ; /* [7]  */
        unsigned int    clk_timer8_st         : 1   ; /* [8]  */
        unsigned int    clk_timer9_st         : 1   ; /* [9]  */
        unsigned int    clk_timer10_st        : 1   ; /* [10]  */
        unsigned int    clk_timer11_st        : 1   ; /* [11]  */
        unsigned int    clk_timer12_st        : 1   ; /* [12]  */
        unsigned int    clk_timer13_st        : 1   ; /* [13]  */
        unsigned int    clk_timer14_st        : 1   ; /* [14]  */
        unsigned int    clk_timer15_st        : 1   ; /* [15]  */
        unsigned int    clk_timer16_st        : 1   ; /* [16]  */
        unsigned int    clk_timer17_st        : 1   ; /* [17]  */
        unsigned int    clk_timer18_st        : 1   ; /* [18]  */
        unsigned int    clk_timer19_st        : 1   ; /* [19]  */
        unsigned int    clk_timer20_st        : 1   ; /* [20]  */
        unsigned int    clk_timer21_st        : 1   ; /* [21]  */
        unsigned int    clk_timer22_st        : 1   ; /* [22]  */
        unsigned int    clk_timer23_st        : 1   ; /* [23]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_ST_U;

/* Define the union U_SC_WATCHDOG_CLK_ST_U */
/* WADTHDOGÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x532C */
typedef union tagScWatchdogClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_wdg0_st           : 1   ; /* [0]  */
        unsigned int    clk_wdg1_st           : 1   ; /* [1]  */
        unsigned int    clk_wdg2_st           : 1   ; /* [2]  */
        unsigned int    clk_wdg3_st           : 1   ; /* [3]  */
        unsigned int    clk_wdg4_st           : 1   ; /* [4]  */
        unsigned int    clk_wdg5_st           : 1   ; /* [5]  */
        unsigned int    clk_wdg6_st           : 1   ; /* [6]  */
        unsigned int    clk_wdg7_st           : 1   ; /* [7]  */
        unsigned int    clk_wdg8_st           : 1   ; /* [8]  */
        unsigned int    clk_wdg9_st           : 1   ; /* [9]  */
        unsigned int    clk_wdg10_st          : 1   ; /* [10]  */
        unsigned int    clk_wdg11_st          : 1   ; /* [11]  */
        unsigned int    clk_wdg12_st          : 1   ; /* [12]  */
        unsigned int    clk_wdg13_st          : 1   ; /* [13]  */
        unsigned int    clk_wdg14_st          : 1   ; /* [14]  */
        unsigned int    clk_wdg15_st          : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_CLK_ST_U;

/* Define the union U_SC_ITS_CLK_ST_U */
/* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5330 */
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

/* Define the union U_SC_GIC_CLK_ST_U */
/* GICÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5334 */
typedef union tagScGicClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gic_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_CLK_ST_U;

/* Define the union U_SC_DMAC_RESET_ST_U */
/* DMAC¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A00 */
typedef union tagScDmacResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dmac0_srst_st         : 1   ; /* [0]  */
        unsigned int    dmac1_srst_st         : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DMAC_RESET_ST_U;

/* Define the union U_SC_NANDC_RESET_ST_U */
/* NANDC¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A04 */
typedef union tagScNandcResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nandc0_srst_st        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NANDC_RESET_ST_U;

/* Define the union U_SC_MMC_RESET_ST_U */
/* MMC¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A08 */
typedef union tagScMmcResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mmc_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MMC_RESET_ST_U;

/* Define the union U_SC_LOCALBUS_RESET_ST_U */
/* LOCALBUS¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A0C */
typedef union tagScLocalbusResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    localbus_srst_st      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LOCALBUS_RESET_ST_U;

/* Define the union U_SC_GPIO_RESET_ST_U */
/* GPIO¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A10 */
typedef union tagScGpioResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gpio0_srst_st         : 1   ; /* [0]  */
        unsigned int    gpio1_srst_st         : 1   ; /* [1]  */
        unsigned int    gpio2_srst_st         : 1   ; /* [2]  */
        unsigned int    gpio3_srst_st         : 1   ; /* [3]  */
        unsigned int    gpio4_srst_st         : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GPIO_RESET_ST_U;

/* Define the union U_SC_UART_RESET_ST_U */
/* UART¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A14 */
typedef union tagScUartResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    uart0_srst_st         : 1   ; /* [0]  */
        unsigned int    uart1_srst_st         : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_UART_RESET_ST_U;

/* Define the union U_SC_I2C_RESET_ST_U */
/* I2C¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A18 */
typedef union tagScI2cResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c0_srst_st          : 1   ; /* [0]  */
        unsigned int    i2c1_srst_st          : 1   ; /* [1]  */
        unsigned int    i2c2_srst_st          : 1   ; /* [2]  */
        unsigned int    i2c3_srst_st          : 1   ; /* [3]  */
        unsigned int    i2c4_srst_st          : 1   ; /* [4]  */
        unsigned int    i2c5_srst_st          : 1   ; /* [5]  */
        unsigned int    i2c6_srst_st          : 1   ; /* [6]  */
        unsigned int    i2c7_srst_st          : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_ST_U;

/* Define the union U_SC_MDIO_RESET_ST_U */
/* MDIO¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A1C */
typedef union tagScMdioResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mdio_srst_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MDIO_RESET_ST_U;

/* Define the union U_SC_DBG_RESET_ST_U */
/* DBG¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A24 */
typedef union tagScDbgResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dbg_srst_st           : 1   ; /* [0]  */
        unsigned int    cs_tracein_srst_st    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_RESET_ST_U;

/* Define the union U_SC_WATCHDOG_RESET_ST_U */
/* WATCHDOG¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A2C */
typedef union tagScWatchdogResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wdg0_srst_st          : 1   ; /* [0]  */
        unsigned int    wdg1_srst_st          : 1   ; /* [1]  */
        unsigned int    wdg2_srst_st          : 1   ; /* [2]  */
        unsigned int    wdg3_srst_st          : 1   ; /* [3]  */
        unsigned int    wdg4_srst_st          : 1   ; /* [4]  */
        unsigned int    wdg5_srst_st          : 1   ; /* [5]  */
        unsigned int    wdg6_srst_st          : 1   ; /* [6]  */
        unsigned int    wdg7_srst_st          : 1   ; /* [7]  */
        unsigned int    wdg8_srst_st          : 1   ; /* [8]  */
        unsigned int    wdg9_srst_st          : 1   ; /* [9]  */
        unsigned int    wdg10_srst_st         : 1   ; /* [10]  */
        unsigned int    wdg11_srst_st         : 1   ; /* [11]  */
        unsigned int    wdg12_srst_st         : 1   ; /* [12]  */
        unsigned int    wdg13_srst_st         : 1   ; /* [13]  */
        unsigned int    wdg14_srst_st         : 1   ; /* [14]  */
        unsigned int    wdg15_srst_st         : 1   ; /* [15]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WATCHDOG_RESET_ST_U;

/* Define the union U_SC_ITS_RESET_ST_U */
/* ITS¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A30 */
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

/* Define the union U_SC_GIC_RESET_ST_U */
/* GIC¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A34 */
typedef union tagScGicResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gic_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GIC_RESET_ST_U;


/* Define the union U_SC_GIC_RESET_REQ_U */
/* ÎªCDIE_CRGµÄPLLÏà¹Ø¹¤×÷Ê±ÖÓÑ¡Ôñ */
/* 0x3044 */
typedef union tagPllClkByPass1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    pll4_bypass_external_n          : 1   ; /* [4]  */
        unsigned int    reserved_2            : 2   ;   /* [3:2] */
        unsigned int    pll1_bypass_external_n : 1  ;
        unsigned int    pll0_bypass_external_n : 1  ;
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PLL_CLK_BYPASS1_U;



/* Define the union U_SC_GIC_RESET_REQ_U */
/* ´¦ÀíÆ÷ºÍÏµÍ³»¥ÁªÊ±ÖÓÆµÂÊÑ¡Ôñ¼Ä´æÆ÷ */
/* 0x3048 */
typedef union tagScClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    clk_div_sel           : 1   ; /* [9]  */
        unsigned int    gpio_db_clk_sel       : 1   ;   /* [8] */
        unsigned int    reserved              : 7  ;
        unsigned int    cpu_clk_source_sel    : 1  ;
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SC_CLK_SEL_U;



#endif

#endif // __PERI_SUB_REG_OFFSET_H__

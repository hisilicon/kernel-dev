/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpaISWHqOHbuE99WnZz8Urp0KNb0WWl+qRa4EjbbH6IvvJ8uAZ7aN4Vd0NGho
uLjlYjzrCZjeceNi92LM42tGBRYFJBp1yStjH3m68LQINaOzzLW5Urhd5dGekhqTwGXT00tX
EEVJwSfyVU1RjdeT1DF0a894++3eGa3aZ5jgfqQOZ9XvrskxBGMm/eLsCD2MmQ==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  °æÈ¨ËùÓÐ (C), 2011-2021, »ªÎª¼¼ÊõÓÐÏÞ¹«Ë¾

 ******************************************************************************
  ÎÄ ¼þ Ãû   : sre_sub_m3.h
  °æ ±¾ ºÅ   : ³õ¸å
  ×÷    Õß   : z00176027
  Éú³ÉÈÕÆÚ   : 2013Äê10ÔÂ17ÈÕ
  ×î½üÐÞ¸Ä   :
  ¹¦ÄÜÃèÊö   : p660 M3×ÓÏµÍ³ ¸´Î»¼°½â¸´Î»Ïà¹ØÍ·ÎÄ¼þ
  º¯ÊýÁÐ±í   :
  ÐÞ¸ÄÀúÊ·   :
  1.ÈÕ    ÆÚ   : 2013Äê10ÔÂ17ÈÕ
    ×÷    Õß   : z00176027
    ÐÞ¸ÄÄÚÈÝ   : ´´½¨ÎÄ¼þ

******************************************************************************/
#ifndef __C_SRE_SUB_M3_H__
#define __C_SRE_SUB_M3_H__


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

/* m3_subctrl Ä£¿é¼Ä´æÆ÷»ùµØÖ· */
#define M3_SUB_BASE                               (0xA0000000)
#define M3_SUB_BASE_SIZE                          (0x10000)

/******************************************************************************/
/*                      PHOSPHOR M3_SUB Registers' Definitions                            */
/******************************************************************************/

#define M3_SUB_SC_WDG_CLK_EN_REG                 (M3_SUB_BASE + 0x300)  /* WDGÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_WDG_CLK_DIS_REG                (M3_SUB_BASE + 0x304)  /* WDGÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_TIMER_CLK_EN_REG               (M3_SUB_BASE + 0x308)  /* TIMERÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_TIMER_CLK_DIS_REG              (M3_SUB_BASE + 0x30C)  /* TIMERÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_UART_CLK_EN_REG                (M3_SUB_BASE + 0x310)  /* UARTÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_UART_CLK_DIS_REG               (M3_SUB_BASE + 0x314)  /* UARTÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_ITS_CLK_EN_REG                 (M3_SUB_BASE + 0x318)  /* ITSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_ITS_CLK_DIS_REG                (M3_SUB_BASE + 0x31C)  /* ITSÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_IPCM_CLK_EN_REG                (M3_SUB_BASE + 0x320)  /* IPCMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_IPCM_CLK_DIS_REG               (M3_SUB_BASE + 0x324)  /* IPCMÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_SRAM_CLK_EN_REG                (M3_SUB_BASE + 0x328)  /* SRAMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_SRAM_CLK_DIS_REG               (M3_SUB_BASE + 0x32C)  /* SRAMÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_SFC_CLK_EN_REG                 (M3_SUB_BASE + 0x330)  /* SFCÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_SFC_CLK_DIS_REG                (M3_SUB_BASE + 0x334)  /* SFCÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_I2CM_CLK_EN_REG                (M3_SUB_BASE + 0x338)  /* I2CMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_I2CM_CLK_DIS_REG               (M3_SUB_BASE + 0x33C)  /* I2CMÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_I2C_CLK_EN_REG                 (M3_SUB_BASE + 0x340)  /* I2CÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_I2C_CLK_DIS_REG                (M3_SUB_BASE + 0x344)  /* I2CÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_GPIO_CLK_EN_REG                (M3_SUB_BASE + 0x348)  /* GPIOÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_GPIO_CLK_DIS_REG               (M3_SUB_BASE + 0x34C)  /* GPIOÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_CPU_CLK_EN_REG                 (M3_SUB_BASE + 0x350)  /* M3 CPUÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_CPU_CLK_DIS_REG                (M3_SUB_BASE + 0x354)  /* M3 CPUÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_USB_CLK_EN_REG                 (M3_SUB_BASE + 0x368)  /* USBÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_USB_CLK_DIS_REG                (M3_SUB_BASE + 0x36C)  /* USBÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_SPI_CLK_EN_REG                 (M3_SUB_BASE + 0x388)  /* spiÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_SPI_CLK_DIS_REG                (M3_SUB_BASE + 0x38C)  /* spiÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_SEC_CLK_EN_REG                 (M3_SUB_BASE + 0x3B8)  /* SECÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_SEC_CLK_DIS_REG                (M3_SUB_BASE + 0x3BC)  /* SECÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define M3_SUB_SC_WDG_RESET_REQ_REG              (M3_SUB_BASE + 0xA00)  /* WDGÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_WDG_RESET_DREQ_REG             (M3_SUB_BASE + 0xA04)  /* WDGÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_TIMER_RESET_REQ_REG            (M3_SUB_BASE + 0xA08)  /* TIMERÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_TIMER_RESET_DREQ_REG           (M3_SUB_BASE + 0xA0C)  /* TIMERÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_UART_RESET_REQ_REG             (M3_SUB_BASE + 0xA10)  /* UARTÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_UART_RESET_DREQ_REG            (M3_SUB_BASE + 0xA14)  /* UARTÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_ITS_RESET_REQ_REG              (M3_SUB_BASE + 0xA18)  /* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_ITS_RESET_DREQ_REG             (M3_SUB_BASE + 0xA1C)  /* ITSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_IPCM_RESET_REQ_REG             (M3_SUB_BASE + 0xA20)  /* IPCMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_IPCM_RESET_DREQ_REG            (M3_SUB_BASE + 0xA24)  /* IPCMÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SRAM_RESET_REQ_REG             (M3_SUB_BASE + 0xA28)  /* SRAMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SRAM_RESET_DREQ_REG            (M3_SUB_BASE + 0xA2C)  /* SRAMÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SFC_RESET_REQ_REG              (M3_SUB_BASE + 0xA30)  /* SFCÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SFC_RESET_DREQ_REG             (M3_SUB_BASE + 0xA34)  /* SFCÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_I2CM_RESET_REQ_REG             (M3_SUB_BASE + 0xA38)  /* I2CMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_I2CM_RESET_DREQ_REG            (M3_SUB_BASE + 0xA3C)  /* I2CMÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_I2C_RESET_REQ_REG              (M3_SUB_BASE + 0xA40)  /* I2CÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_I2C_RESET_DREQ_REG             (M3_SUB_BASE + 0xA44)  /* I2CÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_GPIO_RESET_REQ_REG             (M3_SUB_BASE + 0xA48)  /* GPIOÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_GPIO_RESET_DREQ_REG            (M3_SUB_BASE + 0xA4C)  /* GPIOÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_CPU_RESET_REQ_REG              (M3_SUB_BASE + 0xA50)  /* M3 CPUÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_CPU_RESET_DREQ_REG             (M3_SUB_BASE + 0xA54)  /* M3 CPUÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_USB_RESET_REQ_REG              (M3_SUB_BASE + 0xA88)  /* USBÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_USB_RESET_DREQ_REG             (M3_SUB_BASE + 0xA8C)  /* USBÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SPI_RESET_REQ_REG              (M3_SUB_BASE + 0xA98)  /* SPIÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SPI_RESET_DREQ_REG             (M3_SUB_BASE + 0xA9C)  /* SPIÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SEC_RESET_REQ_REG              (M3_SUB_BASE + 0xAA8)  /* SECÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SEC_RESET_DREQ_REG             (M3_SUB_BASE + 0xAAC)  /* SECÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SEC_BUILD_RESET_REQ_REG        (M3_SUB_BASE + 0xAB8)  /* SEC BUILDÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SEC_BUILD_RESET_DREQ_REG       (M3_SUB_BASE + 0xABC)  /* SEC BUILDÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_PW_CTRL_RESET_REQ_REG      (M3_SUB_BASE + 0xAC0)  /* PW_CTRLÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_PW_CTRL_RESET_DREQ_REG     (M3_SUB_BASE + 0xAC4)  /* PW_CTRLÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_DISPATCH_DAW_EN_REG        (M3_SUB_BASE + 0x1000) /* dispatch daw enÅäÖÃ */
#define M3_SUB_SC_DISPATCH_DAW_ARRAY0_REG    (M3_SUB_BASE + 0x1004) /* dispatch dawÅäÖÃÕóÁÐ0 */
#define M3_SUB_SC_DISPATCH_DAW_ARRAY1_REG    (M3_SUB_BASE + 0x1008) /* dispatch dawÅäÖÃÕóÁÐ1 */
#define M3_SUB_SC_DISPATCH_DAW_ARRAY2_REG    (M3_SUB_BASE + 0x100C) /* dispatch dawÅäÖÃÕóÁÐ2 */
#define M3_SUB_SC_DISPATCH_DAW_ARRAY3_REG    (M3_SUB_BASE + 0x1010) /* dispatch dawÅäÖÃÕóÁÐ3 */
#define M3_SUB_SC_DISPATCH_DAW_ARRAY4_REG    (M3_SUB_BASE + 0x1014) /* dispatch dawÅäÖÃÕóÁÐ4 */
#define M3_SUB_SC_DISPATCH_DAW_ARRAY5_REG    (M3_SUB_BASE + 0x1018) /* dispatch dawÅäÖÃÕóÁÐ5 */
#define M3_SUB_SC_DISPATCH_DAW_ARRAY6_REG    (M3_SUB_BASE + 0x101C) /* dispatch dawÅäÖÃÕóÁÐ6 */
#define M3_SUB_SC_DISPATCH_DAW_ARRAY7_REG    (M3_SUB_BASE + 0x1020) /* dispatch dawÅäÖÃÕóÁÐ7 */
#define M3_SUB_SC_DISPATCH_RETRY_CONTROL_REG (M3_SUB_BASE + 0x1030) /* dispatch retry¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_DISPATCH_INTMASK_REG       (M3_SUB_BASE + 0x1100) /* dispatchµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
#define M3_SUB_SC_DISPATCH_RAWINT_REG        (M3_SUB_BASE + 0x1104) /* dispatchµÄÔ­Ê¼ÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_DISPATCH_INTSTAT_REG       (M3_SUB_BASE + 0x1108) /* dispatchµÄÆÁ±ÎºóµÄÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_DISPATCH_INTCLR_REG        (M3_SUB_BASE + 0x110C) /* dispatchµÄÖÐ¶ÏÇå³ý¼Ä´æÆ÷ */
#define M3_SUB_SC_DISPATCH_ERRSTAT_REG       (M3_SUB_BASE + 0x1110) /* dispatchµÄERR×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_REMAP_CTRL_REG             (M3_SUB_BASE + 0x1200) /* subsysµÄÆô¶¯Remap¼Ä´æÆ÷ */
#define M3_SUB_SC_EXTINT_CTRL_REG            (M3_SUB_BASE + 0x2000) /* Íâ²¿ÖÐ¶Ï¼«ÐÔÅäÖÃ¼Ä´æÆ÷ */
#define M3_SUB_SC_SPI_CTRL_REG               (M3_SUB_BASE + 0x2020) /* spi¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_I2C_CTRL_REG               (M3_SUB_BASE + 0x2024) /* i2c¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SRAM_CTRL0_REG             (M3_SUB_BASE + 0x2030) /* sram¿ØÖÆ¼Ä´æÆ÷0 */
#define M3_SUB_SC_SRAM_CTRL1_REG             (M3_SUB_BASE + 0x2034) /* sram¿ØÖÆ¼Ä´æÆ÷1£¨°²È«¼Ä´æÆ÷£© */
#define M3_SUB_SC_SRAM_CTRL2_REG             (M3_SUB_BASE + 0x2038) /* sram¿ØÖÆ¼Ä´æÆ÷2£¨°²È«¼Ä´æÆ÷£© */
#define M3_SUB_SC_SRAM_CTRL3_REG             (M3_SUB_BASE + 0x203C) /* sram¿ØÖÆ¼Ä´æÆ÷3£¨°²È«¼Ä´æÆ÷£© */
#define M3_SUB_SC_TIMER_CLKEN_CTRL_REG       (M3_SUB_BASE + 0x2040) /* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_WDOG_CLKEN_CTRL_REG        (M3_SUB_BASE + 0x2050) /* WDOGµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_USBHOST_CTRL0_REG          (M3_SUB_BASE + 0x2060) /* usbÅäÖÃ */
#define M3_SUB_SC_ISO_EN_TSV_SLLC_SRST_REG   (M3_SUB_BASE + 0x2080) /* IDIE isolation CELLÊ¹ÄÜÐÅºÅ£»TSV_SLLC_IDIEÕûÌåÈí¸´Î»ÇëÇó¿ØÖÆ */
#define M3_SUB_SC_USBPHY_CTRL0_REG           (M3_SUB_BASE + 0x20A0) /* USBPHY¿ØÖÆ¼Ä´æÆ÷0 */
#define M3_SUB_SC_USBPHY_CTRL1_REG           (M3_SUB_BASE + 0x20A4) /* USBPHY¿ØÖÆ¼Ä´æÆ÷1 */
#define M3_SUB_SC_M3_CTRL0_REG               (M3_SUB_BASE + 0x20B0) /* M3¿ØÖÆ¼Ä´æÆ÷0 */
#define M3_SUB_SC_M3_CTRL1_REG               (M3_SUB_BASE + 0x20B4) /* M3¿ØÖÆ¼Ä´æÆ÷1 */
#define M3_SUB_SC_VMID_CTRL0_REG             (M3_SUB_BASE + 0x20E0) /* m3 vmidÅäÖÃ */
#define M3_SUB_SC_VMID_CTRL1_REG             (M3_SUB_BASE + 0x20E4) /* usb vmidÅäÖÃ */
#define M3_SUB_SC_I2C_M_VMID_REG             (M3_SUB_BASE + 0x20E8) /* i2c_m vmidÅäÖÃ */
#define M3_SUB_SC_M3_REMAP_ADDR_EN_REG       (M3_SUB_BASE + 0x2180) /* m3µØÖ·Ó³ÉäÊ¹ÄÜ */
#define M3_SUB_SC_M3_REMAP_ADDR_0_SECURE_REG (M3_SUB_BASE + 0x2190) /* ÕâÒ»¼Ä´æÆ÷±£Áô²»ÓÃ */
#define M3_SUB_SC_M3_REMAP_ADDR_0_REG        (M3_SUB_BASE + 0x2194) /* M3 remapµ½spi flashµÄ»ùµØÖ·ÅäÖÃ */
#define M3_SUB_SC_M3_REMAP_ADDR_1_REG        (M3_SUB_BASE + 0x2198) /* M3 remapµ½sramµÄ»ùµØÖ·ÅäÖÃ */
#define M3_SUB_SC_M3_REMAP_ADDR_2_REG        (M3_SUB_BASE + 0x219C) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_3_REG        (M3_SUB_BASE + 0x21A0) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_4_REG        (M3_SUB_BASE + 0x21A4) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_5_REG        (M3_SUB_BASE + 0x21A8) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_6_REG        (M3_SUB_BASE + 0x21AC) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_7_REG        (M3_SUB_BASE + 0x21B0) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_8_REG        (M3_SUB_BASE + 0x21B4) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_9_REG        (M3_SUB_BASE + 0x21B8) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_10_REG       (M3_SUB_BASE + 0x21BC) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_11_REG       (M3_SUB_BASE + 0x21C0) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_12_REG       (M3_SUB_BASE + 0x21C4) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_13_REG       (M3_SUB_BASE + 0x21C8) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_14_REG       (M3_SUB_BASE + 0x21CC) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_M3_REMAP_ADDR_15_REG       (M3_SUB_BASE + 0x21D0) /* M3µØÖ·ÖØÓ³Éä¼Ä´æÆ÷ */
#define M3_SUB_SC_ITS_M3_INT_MUX_SEL_REG     (M3_SUB_BASE + 0x21F0) /* ¶ÔÖÐ¶ÏËÍµ½ITS»¹ÊÇM3£¬×öMUXÑ¡Ôñ */
#define M3_SUB_SC_FTE_MUX_CTRL_REG           (M3_SUB_BASE + 0x2200) /* FTE¸´ÓÃÑ¡Ôñ¼Ä´æÆ÷ */
#define M3_SUB_SC_PM_CTRL0_REG               (M3_SUB_BASE + 0x2380) /* PW_STAT_CTRLµÄ¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_PM_CTRL1_REG               (M3_SUB_BASE + 0x2384) /* PW_STAT_CTRLµÄ¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_PM_CTRL2_REG               (M3_SUB_BASE + 0x2388) /* PW_STAT_CTRLµÄ¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_PM_INT_REG                 (M3_SUB_BASE + 0x238C) /* PW_STAT_CTRLÄÚÖÐ¶ÏÔ´¼Ä´æÆ÷ */
#define M3_SUB_SC_PM_INT_MASK_REG            (M3_SUB_BASE + 0x2390) /* PW_STAT_CTRLµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
#define M3_SUB_SC_PM_CTRL3_REG               (M3_SUB_BASE + 0x2394) /* ¿ØÖÆ¹Ü½ÅPM_CTL */
#define M3_SUB_SC_PM_ST_REG                  (M3_SUB_BASE + 0x2398) /* ÉÏµç¸´Î»Ö¸Ê¾×´Ì¬ÐÅºÅ */
#define M3_SUB_SC_BISR_RE_REPAIR_EN_REG      (M3_SUB_BASE + 0x2D80) /* BISRµÄRE REPAIRÊ¹ÄÜ¼Ä´æÆ÷ */
#define M3_SUB_SC_BISR_INT_REG               (M3_SUB_BASE + 0x2D90) /* BISR_TOP_IDIEµÄÖÐ¶ÏÔ´¼Ä´æÆ÷ */
#define M3_SUB_SC_BISR_INT_MASK_REG          (M3_SUB_BASE + 0x2DA0) /* BISR_TOP_IDIEµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
#define M3_SUB_SC_USB_EHCI_ADDR_H_REG        (M3_SUB_BASE + 0x2E0C) /* M3Ö¸ÁîÍ¨µÀ¸ß32Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
#define M3_SUB_SC_USB_OHCI_ADDR_H_REG        (M3_SUB_BASE + 0x2E10) /* M3Êý¾ÝÍ¨µÀ¸ß32Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
#define M3_SUB_SC_I2C_M_ADDR_H_REG           (M3_SUB_BASE + 0x2E14) /* M3ÍâÉèÍ¨µÀ¸ß32Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
#define M3_SUB_SC_SMMU_MEM_CTRL0_REG         (M3_SUB_BASE + 0x3000) /* smmu mem¿ØÖÆ¼Ä´æÆ÷0 */
#define M3_SUB_SC_SMMU_MEM_CTRL1_REG         (M3_SUB_BASE + 0x3004) /* smmu mem¿ØÖÆ¼Ä´æÆ÷1 */
#define M3_SUB_SC_SMMU_MEM_CTRL2_REG         (M3_SUB_BASE + 0x3008) /* smmu mem¿ØÖÆ¼Ä´æÆ÷2 */
#define M3_SUB_SC_SRAM_MEM_CTRL_REG          (M3_SUB_BASE + 0x3020) /* sram mem¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SFC_MEM_CTRL_REG           (M3_SUB_BASE + 0x3030) /* sfc mem¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_SEC_MEM_CTRL_REG           (M3_SUB_BASE + 0x3070) /* sec mem¿ØÖÆ¼Ä´æÆ÷ */
#define M3_SUB_SC_WDG_CLK_ST_REG             (M3_SUB_BASE + 0x5300) /* WDGÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_TIMER_CLK_ST_REG           (M3_SUB_BASE + 0x5304) /* TIMERÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_UART_CLK_ST_REG            (M3_SUB_BASE + 0x5308) /* UARTÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_ITS_CLK_ST_REG             (M3_SUB_BASE + 0x530C) /* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_IPCM_CLK_ST_REG            (M3_SUB_BASE + 0x5310) /* IPCMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_SRAM_CLK_ST_REG            (M3_SUB_BASE + 0x5314) /* SRAMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_SFC_CLK_ST_REG             (M3_SUB_BASE + 0x5318) /* SFCÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_I2CM_CLK_ST_REG            (M3_SUB_BASE + 0x531C) /* I2CMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_I2C_CLK_ST_REG             (M3_SUB_BASE + 0x5320) /* I2CÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_GPIO_CLK_ST_REG            (M3_SUB_BASE + 0x5324) /* GPIOÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_CPU_CLK_ST_REG             (M3_SUB_BASE + 0x5328) /* M3 CPUÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_USB_CLK_ST_REG             (M3_SUB_BASE + 0x5334) /* USBÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_SPI_CLK_ST_REG             (M3_SUB_BASE + 0x5344) /* SPIÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_SEC_CLK_ST_REG             (M3_SUB_BASE + 0x535C) /* SECÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_WDG_RESET_ST_REG           (M3_SUB_BASE + 0x5A00) /* WDG¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_TIMER_RESET_ST_REG         (M3_SUB_BASE + 0x5A04) /* TIMER¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_UART_RESET_ST_REG          (M3_SUB_BASE + 0x5A08) /* UART¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_ITS_RESET_ST_REG           (M3_SUB_BASE + 0x5A0C) /* ITS¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_IPCM_RESET_ST_REG          (M3_SUB_BASE + 0x5A10) /* IPCM¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_SRAM_RESET_ST_REG          (M3_SUB_BASE + 0x5A14) /* SRAM¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_SFC_RESET_ST_REG           (M3_SUB_BASE + 0x5A18) /* SFC¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_I2CM_RESET_ST_REG          (M3_SUB_BASE + 0x5A1C) /* I2CM¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_I2C_RESET_ST_REG           (M3_SUB_BASE + 0x5A20) /* I2C¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_GPIO_RESET_ST_REG          (M3_SUB_BASE + 0x5A24) /* GPIO¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_CPU_RESET_ST_REG           (M3_SUB_BASE + 0x5A28) /* M3 CPU¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_USB_RESET_ST_REG           (M3_SUB_BASE + 0x5A44) /* USB¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_SPI_RESET_ST_REG           (M3_SUB_BASE + 0x5A4C) /* SPI¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_SEC_RESET_ST_REG           (M3_SUB_BASE + 0x5A54) /* SEC¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_SEC_BUILD_RESET_ST_REG     (M3_SUB_BASE + 0x5A5C) /* SEC BUILD¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_PW_CTRL_RESET_ST_REG       (M3_SUB_BASE + 0x5A60) /* PW_CTRL¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define M3_SUB_SC_MBIST_CPUI_ENABLE_REG      (M3_SUB_BASE + 0x5C00) /* CPUÆô¶¯MBISTÑ¡ÔñÐÅºÅ£»Ð¾Æ¬µçÂ·²âÊÔÊ¹ÓÃ¡£ */
#define M3_SUB_SC_MBIST_CPUI_RESET_N_REG     (M3_SUB_BASE + 0x5C04) /* CPUÆô¶¯MBIST¸´Î»ÐÅºÅ */
#define M3_SUB_SC_MBIST_CPUI_DATAIN_REG      (M3_SUB_BASE + 0x5C08) /* CPUÆô¶¯MBIST£¬TMBµçÂ·Êý¾ÝºÍÖ¸ÁîÊäÈë½Ó¿Ú¼Ä´æÆ÷ */
#define M3_SUB_SC_MBIST_CPUI_WRITE_EN_REG    (M3_SUB_BASE + 0x5C0C) /* DFT MBISTÄ£Ê½ÏÂTMBµçÂ·CPU2TAP½Ó¿ÚÅäÖÃ¼Ä´æÆ÷£»Ð¾Æ¬µçÂ·²âÊÔÊ¹ÓÃ¡£ */
#define M3_SUB_SC_SRAM_ST0_REG               (M3_SUB_BASE + 0x6000) /* sram×´Ì¬¼Ä´æÆ÷0 */
#define M3_SUB_SC_SRAM_ST1_REG               (M3_SUB_BASE + 0x6004) /* sram×´Ì¬¼Ä´æÆ÷1 */
#define M3_SUB_SC_SRAM_ST2_REG               (M3_SUB_BASE + 0x6008) /* sram×´Ì¬¼Ä´æÆ÷2 */
#define M3_SUB_SC_SRAM_ST3_REG               (M3_SUB_BASE + 0x600C) /* sram×´Ì¬¼Ä´æÆ÷3 */
#define M3_SUB_SC_SRAM_ST4_REG               (M3_SUB_BASE + 0x6010) /* sram×´Ì¬¼Ä´æÆ÷4 */
#define M3_SUB_SC_SRAM_ST5_REG               (M3_SUB_BASE + 0x6014) /* sram×´Ì¬¼Ä´æÆ÷5 */
#define M3_SUB_SC_M3_STAT_REG                (M3_SUB_BASE + 0x6100)
#define M3_SUB_SC_PM_ST0_REG                 (M3_SUB_BASE + 0x6380) /* PW_STAT_CTRLµÄÄÚ²¿×´Ì¬ */
#define M3_SUB_SC_PM_ST1_REG                 (M3_SUB_BASE + 0x6384) /* À´×Ô¹Ü½ÅµÄPWRGD×´Ì¬ */
#define M3_SUB_SC_PM_ST2_REG                 (M3_SUB_BASE + 0x6388) /* Êä³ö¸ø¹Ü½ÅµÄPWR_CTRL2~0 */
#define M3_SUB_SC_PM_ST3_REG                 (M3_SUB_BASE + 0x638C) /* MEÏà¹Ø¹Ü½Å×´Ì¬ */
#define M3_SUB_SC_DBG_AUTH_CTRL_REG          (M3_SUB_BASE + 0x6700) /* °²È«µÈ¼¶ÊÚÈ¨¿ØÖÆ¼Ä´æÆ÷¡££¨×¢Òâ£º1£¬´Ë¼Ä´æÆ÷¿Õ¼ä±ØÐëÊÇ°²È«·ÃÎÊ²ÅÄÜ³É¹¦£»2£¬Ä¬ÈÏ¾ùÊÇ´ò¿ªµ÷ÊÔÄ£Ê½£¬ÓÐÂ©¶´£¬ÔÚÕý³£¹¤×÷Ê±Çë³õÊ¼»¯¹Ø±Õµ÷ÊÔ¹¦ÄÜ£© */
#define M3_SUB_SC_ECO_RSV0_REG               (M3_SUB_BASE + 0x8000) /* ECO ¼Ä´æÆ÷0 */
#define M3_SUB_SC_ECO_RSV1_REG               (M3_SUB_BASE + 0x8004) /* ECO ¼Ä´æÆ÷1 */
#define M3_SUB_SC_ECO_RSV2_REG               (M3_SUB_BASE + 0x8008) /* ECO ¼Ä´æÆ÷2 */
#define M3_SUB_SC_MBIST_CPUI_DATAOUT_REG     (M3_SUB_BASE + 0xDC00) /* DFT MBISTÄ£Ê½ÏÂ¶ÁÊý¾Ý¼Ä´æÆ÷ */
#define M3_SUB_SC_DIE_ID7_REG                (M3_SUB_BASE + 0xE004) /* DIE ID ¼Ä´æÆ÷bit[255:224]¡£ */
#define M3_SUB_SC_DIE_ID6_REG                (M3_SUB_BASE + 0xE008) /* DIE ID ¼Ä´æÆ÷bit[223:192]¡£ */
#define M3_SUB_SC_DIE_ID5_REG                (M3_SUB_BASE + 0xE00C) /* DIE ID ¼Ä´æÆ÷bit[191:160]¡£ */
#define M3_SUB_SC_DIE_ID4_REG                (M3_SUB_BASE + 0xE010) /* DIE ID ¼Ä´æÆ÷bit[159:128]¡£ */
#define M3_SUB_SC_DIE_ID3_REG                (M3_SUB_BASE + 0xE014) /* DIE ID ¼Ä´æÆ÷bit[127:96]¡£ */
#define M3_SUB_SC_DIE_ID2_REG                (M3_SUB_BASE + 0xE018) /* DIE ID ¼Ä´æÆ÷bit[95:64]¡£ */
#define M3_SUB_SC_DIE_ID1_REG                (M3_SUB_BASE + 0xE01C) /* DIE ID ¼Ä´æÆ÷bit[63:32]¡£ */
#define M3_SUB_SC_DIE_ID0_REG                (M3_SUB_BASE + 0xE020) /* DIE ID ¼Ä´æÆ÷bit[31:0]¡£ */
#define M3_SUB_SC_BOOT_CTRL_ST_REG           (M3_SUB_BASE + 0xE084) /* SC_BOOT_CTRL_ST±íÊ¾CPUÆô¶¯¿ØÖÆµÄPADºÍeFuseÐÅºÅ×´Ì¬ */
#define M3_SUB_SC_EFUSE_RD_ADDR_REG          (M3_SUB_BASE + 0xE100) /* ¶ÁDIEIDÊ±ËùÓÃµØÖ·£¨°²È«¼Ä´æÆ÷£© */
#define M3_SUB_SC_EFUSE_RD_DATA_REG          (M3_SUB_BASE + 0xE180) /* ¶ÁDIEIDÊ±µÃµ½µÄÊý¾Ý£¨°²È«¼Ä´æÆ÷£© */


#if(ENDNESS == ENDNESS_BIG)

/* Define the union U_SC_WDG_CLK_EN_U */
/* WDGÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x300 */
typedef union tagScWdgClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_wdg_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_CLK_EN_U;

/* Define the union U_SC_WDG_CLK_DIS_U */
/* WDGÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x304 */
typedef union tagScWdgClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_wdg_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_CLK_DIS_U;

/* Define the union U_SC_TIMER_CLK_EN_U */
/* TIMERÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x308 */
typedef union tagScTimerClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_timer_enb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_EN_U;

/* Define the union U_SC_TIMER_CLK_DIS_U */
/* TIMERÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x30C */
typedef union tagScTimerClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_timer_dsb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_DIS_U;

/* Define the union U_SC_UART_CLK_EN_U */
/* UARTÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x310 */
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
/* 0x314 */
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

/* Define the union U_SC_ITS_CLK_EN_U */
/* ITSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
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
/* ITSÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
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

/* Define the union U_SC_IPCM_CLK_EN_U */
/* IPCMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x320 */
typedef union tagScIpcmClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_ipcm_enb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_CLK_EN_U;

/* Define the union U_SC_IPCM_CLK_DIS_U */
/* IPCMÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x324 */
typedef union tagScIpcmClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_ipcm_dsb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_CLK_DIS_U;

/* Define the union U_SC_SRAM_CLK_EN_U */
/* SRAMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x328 */
typedef union tagScSramClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_sram_enb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CLK_EN_U;

/* Define the union U_SC_SRAM_CLK_DIS_U */
/* SRAMÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x32C */
typedef union tagScSramClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_sram_dsb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CLK_DIS_U;

/* Define the union U_SC_SFC_CLK_EN_U */
/* SFCÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x330 */
typedef union tagScSfcClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_sfc_bus_enb       : 1   ; /* [1]  */
        unsigned int    clk_sfc_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_CLK_EN_U;

/* Define the union U_SC_SFC_CLK_DIS_U */
/* SFCÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x334 */
typedef union tagScSfcClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_sfc_bus_dsb       : 1   ; /* [1]  */
        unsigned int    clk_sfc_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_CLK_DIS_U;

/* Define the union U_SC_I2CM_CLK_EN_U */
/* I2CMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x338 */
typedef union tagScI2cmClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_i2c_m_enb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_CLK_EN_U;

/* Define the union U_SC_I2CM_CLK_DIS_U */
/* I2CMÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x33C */
typedef union tagScI2cmClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_i2c_m_dsb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_CLK_DIS_U;

/* Define the union U_SC_I2C_CLK_EN_U */
/* I2CÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x340 */
typedef union tagScI2cClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    clk_i2c2_enb          : 1   ; /* [2]  */
        unsigned int    clk_i2c1_enb          : 1   ; /* [1]  */
        unsigned int    clk_i2c0_enb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_EN_U;

/* Define the union U_SC_I2C_CLK_DIS_U */
/* I2CÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x344 */
typedef union tagScI2cClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    clk_i2c2_dsb          : 1   ; /* [2]  */
        unsigned int    clk_i2c1_dsb          : 1   ; /* [1]  */
        unsigned int    clk_i2c0_dsb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_DIS_U;

/* Define the union U_SC_GPIO_CLK_EN_U */
/* GPIOÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x348 */
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
/* 0x34C */
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

/* Define the union U_SC_CPU_CLK_EN_U */
/* M3 CPUÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x350 */
typedef union tagScCpuClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_cpu_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_CLK_EN_U;

/* Define the union U_SC_CPU_CLK_DIS_U */
/* M3 CPUÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x354 */
typedef union tagScCpuClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_cpu_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_CLK_DIS_U;

/* Define the union U_SC_USB_CLK_EN_U */
/* USBÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x368 */
typedef union tagScUsbClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    clk_usb_ohci_12m_enb  : 1   ; /* [3]  */
        unsigned int    clk_usb_ohci_48m_enb  : 1   ; /* [2]  */
        unsigned int    clk_usb_phy_core_enb  : 1   ; /* [1]  */
        unsigned int    clk_usb_ahb_enb       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_CLK_EN_U;

/* Define the union U_SC_USB_CLK_DIS_U */
/* USBÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x36C */
typedef union tagScUsbClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    clk_usb_ohci_12m_dsb  : 1   ; /* [3]  */
        unsigned int    clk_usb_ohci_48m_dsb  : 1   ; /* [2]  */
        unsigned int    clk_usb_phy_core_dsb  : 1   ; /* [1]  */
        unsigned int    clk_usb_ahb_dsb       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_CLK_DIS_U;

/* Define the union U_SC_SPI_CLK_EN_U */
/* spiÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x388 */
typedef union tagScSpiClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_spi_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_CLK_EN_U;

/* Define the union U_SC_SPI_CLK_DIS_U */
/* spiÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x38C */
typedef union tagScSpiClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_spi_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_CLK_DIS_U;

/* Define the union U_SC_SEC_CLK_EN_U */
/* SECÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
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
/* SECÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
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

/* Define the union U_SC_WDG_RESET_REQ_U */
/* WDGÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA00 */
typedef union tagScWdgResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    wdg_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_RESET_REQ_U;

/* Define the union U_SC_WDG_RESET_DREQ_U */
/* WDGÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA04 */
typedef union tagScWdgResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    wdg_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_RESET_DREQ_U;

/* Define the union U_SC_TIMER_RESET_REQ_U */
/* TIMERÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA08 */
typedef union tagScTimerResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    timer_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_RESET_REQ_U;

/* Define the union U_SC_TIMER_RESET_DREQ_U */
/* TIMERÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA0C */
typedef union tagScTimerResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    timer_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_RESET_DREQ_U;

/* Define the union U_SC_UART_RESET_REQ_U */
/* UARTÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA10 */
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
/* 0xA14 */
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

/* Define the union U_SC_ITS_RESET_REQ_U */
/* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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
/* ITSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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

/* Define the union U_SC_IPCM_RESET_REQ_U */
/* IPCMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA20 */
typedef union tagScIpcmResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    ipcm_srst_req         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_RESET_REQ_U;

/* Define the union U_SC_IPCM_RESET_DREQ_U */
/* IPCMÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA24 */
typedef union tagScIpcmResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    ipcm_srst_dreq        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_RESET_DREQ_U;

/* Define the union U_SC_SRAM_RESET_REQ_U */
/* SRAMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA28 */
typedef union tagScSramResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sram_srst_req         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_RESET_REQ_U;

/* Define the union U_SC_SRAM_RESET_DREQ_U */
/* SRAMÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA2C */
typedef union tagScSramResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sram_srst_dreq        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_RESET_DREQ_U;

/* Define the union U_SC_SFC_RESET_REQ_U */
/* SFCÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA30 */
typedef union tagScSfcResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    sfc_bus_srst_req      : 1   ; /* [1]  */
        unsigned int    sfc_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_RESET_REQ_U;

/* Define the union U_SC_SFC_RESET_DREQ_U */
/* SFCÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA34 */
typedef union tagScSfcResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    sfc_bus_srst_dreq     : 1   ; /* [1]  */
        unsigned int    sfc_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_RESET_DREQ_U;

/* Define the union U_SC_I2CM_RESET_REQ_U */
/* I2CMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA38 */
typedef union tagScI2cmResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    i2c_m_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_RESET_REQ_U;

/* Define the union U_SC_I2CM_RESET_DREQ_U */
/* I2CMÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA3C */
typedef union tagScI2cmResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    i2c_m_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_RESET_DREQ_U;

/* Define the union U_SC_I2C_RESET_REQ_U */
/* I2CÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA40 */
typedef union tagScI2cResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    i2c2_srst_req         : 1   ; /* [2]  */
        unsigned int    i2c1_srst_req         : 1   ; /* [1]  */
        unsigned int    i2c0_srst_req         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_REQ_U;

/* Define the union U_SC_I2C_RESET_DREQ_U */
/* I2CÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA44 */
typedef union tagScI2cResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    i2c2_srst_dreq        : 1   ; /* [2]  */
        unsigned int    i2c1_srst_dreq        : 1   ; /* [1]  */
        unsigned int    i2c0_srst_dreq        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_DREQ_U;

/* Define the union U_SC_GPIO_RESET_REQ_U */
/* GPIOÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA48 */
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
/* 0xA4C */
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

/* Define the union U_SC_CPU_RESET_REQ_U */
/* M3 CPUÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA50 */
typedef union tagScCpuResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    cpu_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_RESET_REQ_U;

/* Define the union U_SC_CPU_RESET_DREQ_U */
/* M3 CPUÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA54 */
typedef union tagScCpuResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    cpu_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_RESET_DREQ_U;

/* Define the union U_SC_USB_RESET_REQ_U */
/* USBÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA88 */
typedef union tagScUsbResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    usbhost_phy0_por_srst_req : 1   ; /* [4]  */
        unsigned int    usbhost_phy0_port_srst_req : 1   ; /* [3]  */
        unsigned int    usbhost_ahb_srst_req  : 1   ; /* [2]  */
        unsigned int    usbhost_phy0_srst_req : 1   ; /* [1]  */
        unsigned int    usbhost_utmi0_srst_req : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_RESET_REQ_U;

/* Define the union U_SC_USB_RESET_DREQ_U */
/* USBÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA8C */
typedef union tagScUsbResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    usbhost_phy0_por_srst_dreq : 1   ; /* [4]  */
        unsigned int    usbhost_phy0_port_srst_dreq : 1   ; /* [3]  */
        unsigned int    usbhost_ahb_srst_dreq : 1   ; /* [2]  */
        unsigned int    usbhost_phy0_srst_dreq : 1   ; /* [1]  */
        unsigned int    usbhost_utmi0_srst_dreq : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_RESET_DREQ_U;

/* Define the union U_SC_SPI_RESET_REQ_U */
/* SPIÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA98 */
typedef union tagScSpiResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    spi_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_RESET_REQ_U;

/* Define the union U_SC_SPI_RESET_DREQ_U */
/* SPIÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA9C */
typedef union tagScSpiResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    spi_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_RESET_DREQ_U;

/* Define the union U_SC_SEC_RESET_REQ_U */
/* SECÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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
/* SECÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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

/* Define the union U_SC_SEC_BUILD_RESET_REQ_U */
/* SEC BUILDÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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
/* SEC BUILDÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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

/* Define the union U_SC_PW_CTRL_RESET_REQ_U */
/* PW_CTRLÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAC0 */
typedef union tagScPwCtrlResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pw_ctrl_srst_req      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PW_CTRL_RESET_REQ_U;

/* Define the union U_SC_PW_CTRL_RESET_DREQ_U */
/* PW_CTRLÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAC4 */
typedef union tagScPwCtrlResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pw_ctrl_srst_dreq     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PW_CTRL_RESET_DREQ_U;

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
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    ext_int2_polarity     : 1   ; /* [2]  */
        unsigned int    ext_int1_polarity     : 1   ; /* [1]  */
        unsigned int    ext_int0_polarity     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_EXTINT_CTRL_U;

/* Define the union U_SC_SPI_CTRL_U */
/* spi¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2020 */
typedef union tagScSpiCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
        unsigned int    spi_ss_in_n           : 1   ; /* [6]  */
        unsigned int    spi_rcv_only          : 1   ; /* [5]  */
        unsigned int    spi_cs_polarity3      : 1   ; /* [4]  */
        unsigned int    spi_cs_polarity2      : 1   ; /* [3]  */
        unsigned int    spi_cs_polarity1      : 1   ; /* [2]  */
        unsigned int    spi_cs_polarity0      : 1   ; /* [1]  */
        unsigned int    spi_wire_mode         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_CTRL_U;

/* Define the union U_SC_I2C_CTRL_U */
/* i2c¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2024 */
typedef union tagScI2cCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
        unsigned int    i2c2_sda_cfg          : 1   ; /* [17]  */
        unsigned int    i2c2_dat_oe_cfg       : 1   ; /* [16]  */
        unsigned int    i2c2_dat_mux_sel      : 1   ; /* [15]  */
        unsigned int    i2c2_scl_cfg          : 1   ; /* [14]  */
        unsigned int    i2c2_clk_oe_cfg       : 1   ; /* [13]  */
        unsigned int    i2c2_clk_mux_sel      : 1   ; /* [12]  */
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

/* Define the union U_SC_SRAM_CTRL0_U */
/* sram¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x2030 */
typedef union tagScSramCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
        unsigned int    sram_swap_en          : 1   ; /* [6]  */
        unsigned int    sram_mulerr_addr_clr  : 1   ; /* [5]  */
        unsigned int    sram_sglerr_addr_clr  : 1   ; /* [4]  */
        unsigned int    sram_err_cnt_clr      : 1   ; /* [3]  */
        unsigned int    sram_err_clr          : 1   ; /* [2]  */
        unsigned int    sram_ecc_err_trans    : 1   ; /* [1]  */
        unsigned int    sram_ecc_en           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CTRL0_U;

/* Define the union U_SC_SRAM_CTRL1_U */
/* sram¿ØÖÆ¼Ä´æÆ÷1£¨°²È«¼Ä´æÆ÷£© */
/* 0x2034 */
typedef union tagScSramCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sram_prot_ctrl_en     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CTRL1_U;

/* Define the union U_SC_TIMER_CLKEN_CTRL_U */
/* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2040 */
typedef union tagScTimerClkenCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    timeren0ov            : 1   ; /* [1]  */
        unsigned int    timeren0sel           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLKEN_CTRL_U;

/* Define the union U_SC_WDOG_CLKEN_CTRL_U */
/* WDOGµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2050 */
typedef union tagScWdogClkenCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    wdogen0ov             : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDOG_CLKEN_CTRL_U;

/* Define the union U_SC_USBHOST_CTRL0_U */
/* usbÅäÖÃ */
/* 0x2060 */
typedef union tagScUsbhostCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    usbhost_pwr_ctrl      : 10  ; /* [19..10]  */
        unsigned int    usbhost_app_prt_ovrcur_i : 1   ; /* [9]  */
        unsigned int    usbhost_ohci_susp_lgcy_i : 1   ; /* [8]  */
        unsigned int    usbhost_app_start_clk_i : 1   ; /* [7]  */
        unsigned int    usbhost_ss_hubsetup_min_i : 1   ; /* [6]  */
        unsigned int    usbhost_ss_ena_incrx_align_i : 1   ; /* [5]  */
        unsigned int    usbhost_ss_ena_incr4_i : 1   ; /* [4]  */
        unsigned int    usbhost_ss_ena_incr8_i : 1   ; /* [3]  */
        unsigned int    usbhost_ss_ena_incr16_i : 1   ; /* [2]  */
        unsigned int    usbhost_ss_autoppd_on_overcur_en_i : 1   ; /* [1]  */
        unsigned int    usbhost_ss_word_if_i  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USBHOST_CTRL0_U;

/* Define the union U_SC_ISO_EN_TSV_SLLC_SRST_U */
/* IDIE isolation CELLÊ¹ÄÜÐÅºÅ£»TSV_SLLC_IDIEÕûÌåÈí¸´Î»ÇëÇó¿ØÖÆ */
/* 0x2080 */
typedef union tagScIsoEnTsvSllcSrst
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    tsv_sllc_all_srst_req : 1   ; /* [1]  */
        unsigned int    iso_en                : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ISO_EN_TSV_SLLC_SRST_U;

/* Define the union U_SC_USBPHY_CTRL0_U */
/* USBPHY¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x20A0 */
typedef union tagScUsbphyCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
        unsigned int    usbphy_txpreemppulsetune : 1   ; /* [25]  */
        unsigned int    usbphy_txpreempamptune : 2   ; /* [24..23]  */
        unsigned int    usbphy_txrestune      : 2   ; /* [22..21]  */
        unsigned int    usbphy_txrisetune     : 2   ; /* [20..19]  */
        unsigned int    usbphy_txvreftune     : 4   ; /* [18..15]  */
        unsigned int    usbphy_txfslstune     : 4   ; /* [14..11]  */
        unsigned int    usbphy_txhsxvtune     : 2   ; /* [10..9]  */
        unsigned int    usbphy_otgtune        : 3   ; /* [8..6]  */
        unsigned int    usbphy_sqrxtune       : 3   ; /* [5..3]  */
        unsigned int    usbphy_compdistune    : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USBPHY_CTRL0_U;

/* Define the union U_SC_USBPHY_CTRL1_U */
/* USBPHY¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x20A4 */
typedef union tagScUsbphyCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
        unsigned int    usbphy_wordinterface0 : 1   ; /* [10]  */
        unsigned int    usbphy_sleepm         : 1   ; /* [9]  */
        unsigned int    usbphy_otgdisabled    : 1   ; /* [8]  */
        unsigned int    usbphy_retenablen     : 1   ; /* [7]  */
        unsigned int    usbphy_commononn      : 1   ; /* [6]  */
        unsigned int    usbphy_fsel           : 3   ; /* [5..3]  */
        unsigned int    usbphy_refclksel      : 2   ; /* [2..1]  */
        unsigned int    usbphy_siddq          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USBPHY_CTRL1_U;

/* Define the union U_SC_M3_CTRL0_U */
/* M3¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x20B0 */
typedef union tagScM3Ctrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    m3_dbgen              : 1   ; /* [30]  */
        unsigned int    m3_bigend             : 1   ; /* [29]  */
        unsigned int    m3_wicenreq           : 1   ; /* [28]  */
        unsigned int    m3_rxev               : 1   ; /* [27]  */
        unsigned int    m3_intnmi             : 1   ; /* [26]  */
        unsigned int    m3_stcalib            : 26  ; /* [25..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_M3_CTRL0_U;

/* Define the union U_SC_M3_CTRL1_U */
/* M3¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x20B4 */
typedef union tagScM3Ctrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    m3_rst_status         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_M3_CTRL1_U;

/* Define the union U_SC_VMID_CTRL0_U */
/* m3 vmidÅäÖÃ */
/* 0x20E0 */
typedef union tagScVmidCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
        unsigned int    m3_vmid2              : 8   ; /* [23..16]  */
        unsigned int    m3_vmid1              : 8   ; /* [15..8]  */
        unsigned int    m3_vmid0              : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL0_U;

/* Define the union U_SC_VMID_CTRL1_U */
/* usb vmidÅäÖÃ */
/* 0x20E4 */
typedef union tagScVmidCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    usbhost_ochi_vmid     : 8   ; /* [15..8]  */
        unsigned int    usbhost_echi_vmid     : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL1_U;

/* Define the union U_SC_I2C_M_VMID_U */
/* i2c_m vmidÅäÖÃ */
/* 0x20E8 */
typedef union tagScI2cMVmid
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    i2c_m_vmid            : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_M_VMID_U;

/* Define the union U_SC_M3_REMAP_ADDR_EN_U */
/* m3µØÖ·Ó³ÉäÊ¹ÄÜ */
/* 0x2180 */
typedef union tagScM3RemapAddrEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    m3_remap_addr_en      : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_M3_REMAP_ADDR_EN_U;

/* Define the union U_SC_ITS_M3_INT_MUX_SEL_U */
/* ¶ÔÖÐ¶ÏËÍµ½ITS»¹ÊÇM3£¬×öMUXÑ¡Ôñ */
/* 0x21F0 */
typedef union tagScItsM3IntMuxSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
        unsigned int    int_ext0_mux_sel      : 1   ; /* [10]  */
        unsigned int    int_ext1_mux_sel      : 1   ; /* [9]  */
        unsigned int    int_ext2_mux_sel      : 1   ; /* [8]  */
        unsigned int    int_timer1_mux_sel    : 1   ; /* [7]  */
        unsigned int    int_timer2_mux_sel    : 1   ; /* [6]  */
        unsigned int    int_uart0_mux_sel     : 1   ; /* [5]  */
        unsigned int    int_uart1_mux_sel     : 1   ; /* [4]  */
        unsigned int    int_i2c0_mux_sel      : 1   ; /* [3]  */
        unsigned int    int_i2c1_mux_sel      : 1   ; /* [2]  */
        unsigned int    int_i2c2_mux_sel      : 1   ; /* [1]  */
        unsigned int    int_spi_mux_sel       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_M3_INT_MUX_SEL_U;

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

/* Define the union U_SC_PM_CTRL0_U */
/* PW_STAT_CTRLµÄ¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2380 */
typedef union tagScPmCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    pw_mode_ctrl          : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_CTRL0_U;

/* Define the union U_SC_PM_CTRL1_U */
/* PW_STAT_CTRLµÄ¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2384 */
typedef union tagScPmCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    pw_dn_4s_hard_invalid : 1   ; /* [1]  */
        unsigned int    pwr_m3_ctrl_en        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_CTRL1_U;

/* Define the union U_SC_PM_CTRL2_U */
/* PW_STAT_CTRLµÄ¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2388 */
typedef union tagScPmCtrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    pwr_m3_ctrl           : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_CTRL2_U;

/* Define the union U_SC_PM_INT_U */
/* PW_STAT_CTRLÄÚÖÐ¶ÏÔ´¼Ä´æÆ÷ */
/* 0x238C */
typedef union tagScPmInt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
        unsigned int    pw_btn_pw_dn_4s_hard  : 1   ; /* [6]  */
        unsigned int    pw_btn_pw_dn_4s_soft  : 1   ; /* [5]  */
        unsigned int    pw_btn_pw_up          : 1   ; /* [4]  */
        unsigned int    pw_btn_pw_dn          : 1   ; /* [3]  */
        unsigned int    timeout_10ms          : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_INT_U;

/* Define the union U_SC_PM_INT_MASK_U */
/* PW_STAT_CTRLµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
/* 0x2390 */
typedef union tagScPmIntMask
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    pw_btn_pw_dn_4s_hard_int_mask : 1   ; /* [4]  */
        unsigned int    pw_btn_pw_dn_4s_soft_int_mask : 1   ; /* [3]  */
        unsigned int    pw_btn_pw_up_int_mask : 1   ; /* [2]  */
        unsigned int    pw_btn_pw_dn_int_mask : 1   ; /* [1]  */
        unsigned int    timeout_10ms_int_mask : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_INT_MASK_U;

/* Define the union U_SC_PM_CTRL3_U */
/* ¿ØÖÆ¹Ü½ÅPM_CTL */
/* 0x2394 */
typedef union tagScPmCtrl3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pad_pm_ctl_out        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_CTRL3_U;

/* Define the union U_SC_PM_ST_U */
/* ÉÏµç¸´Î»Ö¸Ê¾×´Ì¬ÐÅºÅ */
/* 0x2398 */
typedef union tagScPmSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pw_ctrl_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_ST_U;

/* Define the union U_SC_BISR_RE_REPAIR_EN_U */
/* BISRµÄRE REPAIRÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x2D80 */
typedef union tagScBisrReRepairEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    bisr_re_repair_en     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BISR_RE_REPAIR_EN_U;

/* Define the union U_SC_BISR_INT_U */
/* BISR_TOP_IDIEµÄÖÐ¶ÏÔ´¼Ä´æÆ÷ */
/* 0x2D90 */
typedef union tagScBisrInt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hard_repair_done      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BISR_INT_U;

/* Define the union U_SC_BISR_INT_MASK_U */
/* BISR_TOP_IDIEµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
/* 0x2DA0 */
typedef union tagScBisrIntMask
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hard_repair_done_int_mask : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BISR_INT_MASK_U;

/* Define the union U_SC_SMMU_MEM_CTRL0_U */
/* smmu mem¿ØÖÆ¼Ä´æÆ÷0 */
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

/* Define the union U_SC_SRAM_MEM_CTRL_U */
/* sram mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3020 */
typedef union tagScSramMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rashsd_sram      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_MEM_CTRL_U;

/* Define the union U_SC_SFC_MEM_CTRL_U */
/* sfc mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3030 */
typedef union tagScSfcMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    ctrl_rft_sfc          : 10  ; /* [9..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_MEM_CTRL_U;

/* Define the union U_SC_SEC_MEM_CTRL_U */
/* sec mem¿ØÖÆ¼Ä´æÆ÷ */
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

/* Define the union U_SC_WDG_CLK_ST_U */
/* WDGÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5300 */
typedef union tagScWdgClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_wdg_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_CLK_ST_U;

/* Define the union U_SC_TIMER_CLK_ST_U */
/* TIMERÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5304 */
typedef union tagScTimerClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_timer_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_ST_U;

/* Define the union U_SC_UART_CLK_ST_U */
/* UARTÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5308 */
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

/* Define the union U_SC_ITS_CLK_ST_U */
/* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_IPCM_CLK_ST_U */
/* IPCMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5310 */
typedef union tagScIpcmClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_ipcm_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_CLK_ST_U;

/* Define the union U_SC_SRAM_CLK_ST_U */
/* SRAMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5314 */
typedef union tagScSramClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_sram_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CLK_ST_U;

/* Define the union U_SC_SFC_CLK_ST_U */
/* SFCÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5318 */
typedef union tagScSfcClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_sfc_bus_st        : 1   ; /* [1]  */
        unsigned int    clk_sfc_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_CLK_ST_U;

/* Define the union U_SC_I2CM_CLK_ST_U */
/* I2CMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x531C */
typedef union tagScI2cmClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_i2c_m_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_CLK_ST_U;

/* Define the union U_SC_I2C_CLK_ST_U */
/* I2CÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5320 */
typedef union tagScI2cClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    clk_i2c2_st           : 1   ; /* [2]  */
        unsigned int    clk_i2c1_st           : 1   ; /* [1]  */
        unsigned int    clk_i2c0_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_ST_U;

/* Define the union U_SC_GPIO_CLK_ST_U */
/* GPIOÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5324 */
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

/* Define the union U_SC_CPU_CLK_ST_U */
/* M3 CPUÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5328 */
typedef union tagScCpuClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_cpu_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_CLK_ST_U;

/* Define the union U_SC_USB_CLK_ST_U */
/* USBÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5334 */
typedef union tagScUsbClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    clk_usb_ohci_12m_st   : 1   ; /* [3]  */
        unsigned int    clk_usb_ohci_48m_st   : 1   ; /* [2]  */
        unsigned int    clk_usb_phy_core_st   : 1   ; /* [1]  */
        unsigned int    clk_usb_ahb_st        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_CLK_ST_U;

/* Define the union U_SC_SPI_CLK_ST_U */
/* SPIÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5344 */
typedef union tagScSpiClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_spi_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_CLK_ST_U;

/* Define the union U_SC_SEC_CLK_ST_U */
/* SECÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_WDG_RESET_ST_U */
/* WDG¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A00 */
typedef union tagScWdgResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    wdg_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_RESET_ST_U;

/* Define the union U_SC_TIMER_RESET_ST_U */
/* TIMER¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A04 */
typedef union tagScTimerResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    timer_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_RESET_ST_U;

/* Define the union U_SC_UART_RESET_ST_U */
/* UART¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A08 */
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

/* Define the union U_SC_ITS_RESET_ST_U */
/* ITS¸´Î»×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_IPCM_RESET_ST_U */
/* IPCM¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A10 */
typedef union tagScIpcmResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    ipcm_srst_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_RESET_ST_U;

/* Define the union U_SC_SRAM_RESET_ST_U */
/* SRAM¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A14 */
typedef union tagScSramResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sram_srst_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_RESET_ST_U;

/* Define the union U_SC_SFC_RESET_ST_U */
/* SFC¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A18 */
typedef union tagScSfcResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    sfc_bus_srst_st       : 1   ; /* [1]  */
        unsigned int    sfc_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_RESET_ST_U;

/* Define the union U_SC_I2CM_RESET_ST_U */
/* I2CM¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A1C */
typedef union tagScI2cmResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    i2c_m_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_RESET_ST_U;

/* Define the union U_SC_I2C_RESET_ST_U */
/* I2C¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A20 */
typedef union tagScI2cResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    i2c2_srst_st          : 1   ; /* [2]  */
        unsigned int    i2c1_srst_st          : 1   ; /* [1]  */
        unsigned int    i2c0_srst_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_ST_U;

/* Define the union U_SC_GPIO_RESET_ST_U */
/* GPIO¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A24 */
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

/* Define the union U_SC_CPU_RESET_ST_U */
/* M3 CPU¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A28 */
typedef union tagScCpuResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    cpu_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_RESET_ST_U;

/* Define the union U_SC_USB_RESET_ST_U */
/* USB¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A44 */
typedef union tagScUsbResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    usbhost_phy0_por_srst_st : 1   ; /* [4]  */
        unsigned int    usbhost_phy0_port_srst_st : 1   ; /* [3]  */
        unsigned int    usbhost_ahb_srst_st   : 1   ; /* [2]  */
        unsigned int    usbhost_phy0_srst_st  : 1   ; /* [1]  */
        unsigned int    usbhost_utmi0_srst_st : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_RESET_ST_U;

/* Define the union U_SC_SPI_RESET_ST_U */
/* SPI¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A4C */
typedef union tagScSpiResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    spi_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_RESET_ST_U;

/* Define the union U_SC_SEC_RESET_ST_U */
/* SEC¸´Î»×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_SEC_BUILD_RESET_ST_U */
/* SEC BUILD¸´Î»×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_PW_CTRL_RESET_ST_U */
/* PW_CTRL¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A60 */
typedef union tagScPwCtrlResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pw_ctrl_srst_st       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PW_CTRL_RESET_ST_U;

/* Define the union U_SC_MBIST_CPUI_ENABLE_U */
/* CPUÆô¶¯MBISTÑ¡ÔñÐÅºÅ£»Ð¾Æ¬µçÂ·²âÊÔÊ¹ÓÃ¡£ */
/* 0x5C00 */
typedef union tagScMbistCpuiEnable
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mbist_cpui_enable     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MBIST_CPUI_ENABLE_U;

/* Define the union U_SC_MBIST_CPUI_RESET_N_U */
/* CPUÆô¶¯MBIST¸´Î»ÐÅºÅ */
/* 0x5C04 */
typedef union tagScMbistCpuiResetN
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mbist_cpui_reset_n    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MBIST_CPUI_RESET_N_U;

/* Define the union U_SC_MBIST_CPUI_WRITE_EN_U */
/* DFT MBISTÄ£Ê½ÏÂTMBµçÂ·CPU2TAP½Ó¿ÚÅäÖÃ¼Ä´æÆ÷£»Ð¾Æ¬µçÂ·²âÊÔÊ¹ÓÃ¡£ */
/* 0x5C0C */
typedef union tagScMbistCpuiWriteEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mbist_cpui_write_en   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MBIST_CPUI_WRITE_EN_U;

/* Define the union U_SC_SRAM_ST0_U */
/* sram×´Ì¬¼Ä´æÆ÷0 */
/* 0x6000 */
typedef union tagScSramSt0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    sram_multi_err        : 1   ; /* [2]  */
        unsigned int    sram_double_err       : 1   ; /* [1]  */
        unsigned int    sram_single_err       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST0_U;

/* Define the union U_SC_SRAM_ST3_U */
/* sram×´Ì¬¼Ä´æÆ÷3 */
/* 0x600C */
typedef union tagScSramSt3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    sram_count_ecc_sglerr : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST3_U;

/* Define the union U_SC_SRAM_ST4_U */
/* sram×´Ì¬¼Ä´æÆ÷4 */
/* 0x6010 */
typedef union tagScSramSt4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    sram_count_ecc_mulerr : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST4_U;

/* Define the union U_SC_SRAM_ST5_U */
/* sram×´Ì¬¼Ä´æÆ÷5 */
/* 0x6014 */
typedef union tagScSramSt5
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    sram_dfx_dbg          : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST5_U;

/* Define the union U_SC_M3_STAT_U */
/*  */
/* 0x6100 */
typedef union tagScM3Stat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
        unsigned int    m3_wakeup             : 1   ; /* [16]  */
        unsigned int    m3_wicenack           : 1   ; /* [15]  */
        unsigned int    m3_currpri            : 8   ; /* [14..7]  */
        unsigned int    m3_sleeping           : 1   ; /* [6]  */
        unsigned int    m3_lockup             : 1   ; /* [5]  */
        unsigned int    m3_halted             : 1   ; /* [4]  */
        unsigned int    m3_brchstat           : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_M3_STAT_U;

/* Define the union U_SC_PM_ST0_U */
/* PW_STAT_CTRLµÄÄÚ²¿×´Ì¬ */
/* 0x6380 */
typedef union tagScPmSt0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    pw_curr_stat          : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_ST0_U;

/* Define the union U_SC_PM_ST1_U */
/* À´×Ô¹Ü½ÅµÄPWRGD×´Ì¬ */
/* 0x6384 */
typedef union tagScPmSt1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    pwrgd                 : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_ST1_U;

/* Define the union U_SC_PM_ST2_U */
/* Êä³ö¸ø¹Ü½ÅµÄPWR_CTRL2~0 */
/* 0x6388 */
typedef union tagScPmSt2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    pwr_ctrl              : 3   ; /* [2..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_ST2_U;

/* Define the union U_SC_PM_ST3_U */
/* MEÏà¹Ø¹Ü½Å×´Ì¬ */
/* 0x638C */
typedef union tagScPmSt3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    pad_me_mst_in         : 1   ; /* [4]  */
        unsigned int    pad_ext_int2_in       : 1   ; /* [3]  */
        unsigned int    pad_ext_int1_in       : 1   ; /* [2]  */
        unsigned int    pad_ext_int0_in       : 1   ; /* [1]  */
        unsigned int    pad_sm_alert_n_in     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_ST3_U;

/* Define the union U_SC_DBG_AUTH_CTRL_U */
/* °²È«µÈ¼¶ÊÚÈ¨¿ØÖÆ¼Ä´æÆ÷¡£
£¨×¢Òâ£º1£¬´Ë¼Ä´æÆ÷¿Õ¼ä±ØÐëÊÇ°²È«·ÃÎÊ²ÅÄÜ³É¹¦£»2£¬Ä¬ÈÏ¾ùÊÇ´ò¿ªµ÷ÊÔÄ£Ê½£¬ÓÐÂ©¶´£¬ÔÚÕý³£¹¤×÷Ê±Çë³õÊ¼»¯¹Ø±Õµ÷ÊÔ¹¦ÄÜ£© */
/* 0x6700 */
typedef union tagScDbgAuthCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rsa_djtag_sec_acc_en  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_AUTH_CTRL_U;

/* Define the union U_SC_BOOT_CTRL_ST_U */
/* SC_BOOT_CTRL_ST±íÊ¾CPUÆô¶¯¿ØÖÆµÄPADºÍeFuseÐÅºÅ×´Ì¬ */
/* 0xE084 */
typedef union tagScBootCtrlSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    efuse_ns_forbid       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BOOT_CTRL_ST_U;

/* Define the union U_SC_EFUSE_RD_ADDR_U */
/* ¶ÁDIEIDÊ±ËùÓÃµØÖ·£¨°²È«¼Ä´æÆ÷£© */
/* 0xE100 */
typedef union tagScEfuseRdAddr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
        unsigned int    efuse_rd_addr         : 7   ; /* [6..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_EFUSE_RD_ADDR_U;

#else

/* Define the union U_SC_WDG_CLK_EN_U */
/* WDGÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x300 */
typedef union tagScWdgClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_wdg_enb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_CLK_EN_U;

/* Define the union U_SC_WDG_CLK_DIS_U */
/* WDGÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x304 */
typedef union tagScWdgClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_wdg_dsb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_CLK_DIS_U;

/* Define the union U_SC_TIMER_CLK_EN_U */
/* TIMERÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x308 */
typedef union tagScTimerClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_timer_enb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_EN_U;

/* Define the union U_SC_TIMER_CLK_DIS_U */
/* TIMERÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x30C */
typedef union tagScTimerClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_timer_dsb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_DIS_U;

/* Define the union U_SC_UART_CLK_EN_U */
/* UARTÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x310 */
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
/* 0x314 */
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

/* Define the union U_SC_ITS_CLK_EN_U */
/* ITSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
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
/* ITSÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
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

/* Define the union U_SC_IPCM_CLK_EN_U */
/* IPCMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x320 */
typedef union tagScIpcmClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_ipcm_enb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_CLK_EN_U;

/* Define the union U_SC_IPCM_CLK_DIS_U */
/* IPCMÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x324 */
typedef union tagScIpcmClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_ipcm_dsb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_CLK_DIS_U;

/* Define the union U_SC_SRAM_CLK_EN_U */
/* SRAMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x328 */
typedef union tagScSramClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sram_enb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CLK_EN_U;

/* Define the union U_SC_SRAM_CLK_DIS_U */
/* SRAMÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x32C */
typedef union tagScSramClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sram_dsb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CLK_DIS_U;

/* Define the union U_SC_SFC_CLK_EN_U */
/* SFCÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x330 */
typedef union tagScSfcClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sfc_enb           : 1   ; /* [0]  */
        unsigned int    clk_sfc_bus_enb       : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_CLK_EN_U;

/* Define the union U_SC_SFC_CLK_DIS_U */
/* SFCÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x334 */
typedef union tagScSfcClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sfc_dsb           : 1   ; /* [0]  */
        unsigned int    clk_sfc_bus_dsb       : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_CLK_DIS_U;

/* Define the union U_SC_I2CM_CLK_EN_U */
/* I2CMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x338 */
typedef union tagScI2cmClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_i2c_m_enb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_CLK_EN_U;

/* Define the union U_SC_I2CM_CLK_DIS_U */
/* I2CMÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x33C */
typedef union tagScI2cmClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_i2c_m_dsb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_CLK_DIS_U;

/* Define the union U_SC_I2C_CLK_EN_U */
/* I2CÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x340 */
typedef union tagScI2cClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_i2c0_enb          : 1   ; /* [0]  */
        unsigned int    clk_i2c1_enb          : 1   ; /* [1]  */
        unsigned int    clk_i2c2_enb          : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_EN_U;

/* Define the union U_SC_I2C_CLK_DIS_U */
/* I2CÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x344 */
typedef union tagScI2cClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_i2c0_dsb          : 1   ; /* [0]  */
        unsigned int    clk_i2c1_dsb          : 1   ; /* [1]  */
        unsigned int    clk_i2c2_dsb          : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_DIS_U;

/* Define the union U_SC_GPIO_CLK_EN_U */
/* GPIOÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x348 */
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
/* 0x34C */
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

/* Define the union U_SC_CPU_CLK_EN_U */
/* M3 CPUÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x350 */
typedef union tagScCpuClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_cpu_enb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_CLK_EN_U;

/* Define the union U_SC_CPU_CLK_DIS_U */
/* M3 CPUÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x354 */
typedef union tagScCpuClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_cpu_dsb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_CLK_DIS_U;

/* Define the union U_SC_USB_CLK_EN_U */
/* USBÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x368 */
typedef union tagScUsbClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_usb_ahb_enb       : 1   ; /* [0]  */
        unsigned int    clk_usb_phy_core_enb  : 1   ; /* [1]  */
        unsigned int    clk_usb_ohci_48m_enb  : 1   ; /* [2]  */
        unsigned int    clk_usb_ohci_12m_enb  : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_CLK_EN_U;

/* Define the union U_SC_USB_CLK_DIS_U */
/* USBÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x36C */
typedef union tagScUsbClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_usb_ahb_dsb       : 1   ; /* [0]  */
        unsigned int    clk_usb_phy_core_dsb  : 1   ; /* [1]  */
        unsigned int    clk_usb_ohci_48m_dsb  : 1   ; /* [2]  */
        unsigned int    clk_usb_ohci_12m_dsb  : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_CLK_DIS_U;

/* Define the union U_SC_SPI_CLK_EN_U */
/* spiÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x388 */
typedef union tagScSpiClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_spi_enb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_CLK_EN_U;

/* Define the union U_SC_SPI_CLK_DIS_U */
/* spiÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x38C */
typedef union tagScSpiClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_spi_dsb           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_CLK_DIS_U;

/* Define the union U_SC_SEC_CLK_EN_U */
/* SECÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
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
/* SECÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
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

/* Define the union U_SC_WDG_RESET_REQ_U */
/* WDGÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA00 */
typedef union tagScWdgResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wdg_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_RESET_REQ_U;

/* Define the union U_SC_WDG_RESET_DREQ_U */
/* WDGÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA04 */
typedef union tagScWdgResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wdg_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_RESET_DREQ_U;

/* Define the union U_SC_TIMER_RESET_REQ_U */
/* TIMERÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA08 */
typedef union tagScTimerResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    timer_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_RESET_REQ_U;

/* Define the union U_SC_TIMER_RESET_DREQ_U */
/* TIMERÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA0C */
typedef union tagScTimerResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    timer_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_RESET_DREQ_U;

/* Define the union U_SC_UART_RESET_REQ_U */
/* UARTÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA10 */
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
/* 0xA14 */
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

/* Define the union U_SC_ITS_RESET_REQ_U */
/* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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
/* ITSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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

/* Define the union U_SC_IPCM_RESET_REQ_U */
/* IPCMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA20 */
typedef union tagScIpcmResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ipcm_srst_req         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_RESET_REQ_U;

/* Define the union U_SC_IPCM_RESET_DREQ_U */
/* IPCMÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA24 */
typedef union tagScIpcmResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ipcm_srst_dreq        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_RESET_DREQ_U;

/* Define the union U_SC_SRAM_RESET_REQ_U */
/* SRAMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA28 */
typedef union tagScSramResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_srst_req         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_RESET_REQ_U;

/* Define the union U_SC_SRAM_RESET_DREQ_U */
/* SRAMÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA2C */
typedef union tagScSramResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_srst_dreq        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_RESET_DREQ_U;

/* Define the union U_SC_SFC_RESET_REQ_U */
/* SFCÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA30 */
typedef union tagScSfcResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfc_srst_req          : 1   ; /* [0]  */
        unsigned int    sfc_bus_srst_req      : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_RESET_REQ_U;

/* Define the union U_SC_SFC_RESET_DREQ_U */
/* SFCÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA34 */
typedef union tagScSfcResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfc_srst_dreq         : 1   ; /* [0]  */
        unsigned int    sfc_bus_srst_dreq     : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_RESET_DREQ_U;

/* Define the union U_SC_I2CM_RESET_REQ_U */
/* I2CMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA38 */
typedef union tagScI2cmResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c_m_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_RESET_REQ_U;

/* Define the union U_SC_I2CM_RESET_DREQ_U */
/* I2CMÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA3C */
typedef union tagScI2cmResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c_m_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_RESET_DREQ_U;

/* Define the union U_SC_I2C_RESET_REQ_U */
/* I2CÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA40 */
typedef union tagScI2cResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c0_srst_req         : 1   ; /* [0]  */
        unsigned int    i2c1_srst_req         : 1   ; /* [1]  */
        unsigned int    i2c2_srst_req         : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_REQ_U;

/* Define the union U_SC_I2C_RESET_DREQ_U */
/* I2CÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA44 */
typedef union tagScI2cResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c0_srst_dreq        : 1   ; /* [0]  */
        unsigned int    i2c1_srst_dreq        : 1   ; /* [1]  */
        unsigned int    i2c2_srst_dreq        : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_DREQ_U;

/* Define the union U_SC_GPIO_RESET_REQ_U */
/* GPIOÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA48 */
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
/* 0xA4C */
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

/* Define the union U_SC_CPU_RESET_REQ_U */
/* M3 CPUÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA50 */
typedef union tagScCpuResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpu_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_RESET_REQ_U;

/* Define the union U_SC_CPU_RESET_DREQ_U */
/* M3 CPUÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA54 */
typedef union tagScCpuResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpu_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_RESET_DREQ_U;

/* Define the union U_SC_USB_RESET_REQ_U */
/* USBÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA88 */
typedef union tagScUsbResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    usbhost_utmi0_srst_req : 1   ; /* [0]  */
        unsigned int    usbhost_phy0_srst_req : 1   ; /* [1]  */
        unsigned int    usbhost_ahb_srst_req  : 1   ; /* [2]  */
        unsigned int    usbhost_phy0_port_srst_req : 1   ; /* [3]  */
        unsigned int    usbhost_phy0_por_srst_req : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_RESET_REQ_U;

/* Define the union U_SC_USB_RESET_DREQ_U */
/* USBÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA8C */
typedef union tagScUsbResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    usbhost_utmi0_srst_dreq : 1   ; /* [0]  */
        unsigned int    usbhost_phy0_srst_dreq : 1   ; /* [1]  */
        unsigned int    usbhost_ahb_srst_dreq : 1   ; /* [2]  */
        unsigned int    usbhost_phy0_port_srst_dreq : 1   ; /* [3]  */
        unsigned int    usbhost_phy0_por_srst_dreq : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_RESET_DREQ_U;

/* Define the union U_SC_SPI_RESET_REQ_U */
/* SPIÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA98 */
typedef union tagScSpiResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    spi_srst_req          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_RESET_REQ_U;

/* Define the union U_SC_SPI_RESET_DREQ_U */
/* SPIÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA9C */
typedef union tagScSpiResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    spi_srst_dreq         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_RESET_DREQ_U;

/* Define the union U_SC_SEC_RESET_REQ_U */
/* SECÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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
/* SECÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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

/* Define the union U_SC_SEC_BUILD_RESET_REQ_U */
/* SEC BUILDÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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
/* SEC BUILDÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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

/* Define the union U_SC_PW_CTRL_RESET_REQ_U */
/* PW_CTRLÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAC0 */
typedef union tagScPwCtrlResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pw_ctrl_srst_req      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PW_CTRL_RESET_REQ_U;

/* Define the union U_SC_PW_CTRL_RESET_DREQ_U */
/* PW_CTRLÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAC4 */
typedef union tagScPwCtrlResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pw_ctrl_srst_dreq     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PW_CTRL_RESET_DREQ_U;

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
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_EXTINT_CTRL_U;

/* Define the union U_SC_SPI_CTRL_U */
/* spi¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2020 */
typedef union tagScSpiCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    spi_wire_mode         : 1   ; /* [0]  */
        unsigned int    spi_cs_polarity0      : 1   ; /* [1]  */
        unsigned int    spi_cs_polarity1      : 1   ; /* [2]  */
        unsigned int    spi_cs_polarity2      : 1   ; /* [3]  */
        unsigned int    spi_cs_polarity3      : 1   ; /* [4]  */
        unsigned int    spi_rcv_only          : 1   ; /* [5]  */
        unsigned int    spi_ss_in_n           : 1   ; /* [6]  */
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_CTRL_U;

/* Define the union U_SC_I2C_CTRL_U */
/* i2c¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2024 */
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
        unsigned int    i2c2_clk_mux_sel      : 1   ; /* [12]  */
        unsigned int    i2c2_clk_oe_cfg       : 1   ; /* [13]  */
        unsigned int    i2c2_scl_cfg          : 1   ; /* [14]  */
        unsigned int    i2c2_dat_mux_sel      : 1   ; /* [15]  */
        unsigned int    i2c2_dat_oe_cfg       : 1   ; /* [16]  */
        unsigned int    i2c2_sda_cfg          : 1   ; /* [17]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CTRL_U;

/* Define the union U_SC_SRAM_CTRL0_U */
/* sram¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x2030 */
typedef union tagScSramCtrl0
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

} SC_SRAM_CTRL0_U;

/* Define the union U_SC_SRAM_CTRL1_U */
/* sram¿ØÖÆ¼Ä´æÆ÷1£¨°²È«¼Ä´æÆ÷£© */
/* 0x2034 */
typedef union tagScSramCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_prot_ctrl_en     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CTRL1_U;

/* Define the union U_SC_TIMER_CLKEN_CTRL_U */
/* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2040 */
typedef union tagScTimerClkenCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    timeren0sel           : 1   ; /* [0]  */
        unsigned int    timeren0ov            : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLKEN_CTRL_U;

/* Define the union U_SC_WDOG_CLKEN_CTRL_U */
/* WDOGµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2050 */
typedef union tagScWdogClkenCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wdogen0ov             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDOG_CLKEN_CTRL_U;

/* Define the union U_SC_USBHOST_CTRL0_U */
/* usbÅäÖÃ */
/* 0x2060 */
typedef union tagScUsbhostCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    usbhost_ss_word_if_i  : 1   ; /* [0]  */
        unsigned int    usbhost_ss_autoppd_on_overcur_en_i : 1   ; /* [1]  */
        unsigned int    usbhost_ss_ena_incr16_i : 1   ; /* [2]  */
        unsigned int    usbhost_ss_ena_incr8_i : 1   ; /* [3]  */
        unsigned int    usbhost_ss_ena_incr4_i : 1   ; /* [4]  */
        unsigned int    usbhost_ss_ena_incrx_align_i : 1   ; /* [5]  */
        unsigned int    usbhost_ss_hubsetup_min_i : 1   ; /* [6]  */
        unsigned int    usbhost_app_start_clk_i : 1   ; /* [7]  */
        unsigned int    usbhost_ohci_susp_lgcy_i : 1   ; /* [8]  */
        unsigned int    usbhost_app_prt_ovrcur_i : 1   ; /* [9]  */
        unsigned int    usbhost_pwr_ctrl      : 10  ; /* [19..10]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USBHOST_CTRL0_U;

/* Define the union U_SC_ISO_EN_TSV_SLLC_SRST_U */
/* IDIE isolation CELLÊ¹ÄÜÐÅºÅ£»TSV_SLLC_IDIEÕûÌåÈí¸´Î»ÇëÇó¿ØÖÆ */
/* 0x2080 */
typedef union tagScIsoEnTsvSllcSrst
{
    /* Define the struct bits */
    struct
    {
        unsigned int    iso_en                : 1   ; /* [0]  */
        unsigned int    tsv_sllc_all_srst_req : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ISO_EN_TSV_SLLC_SRST_U;

/* Define the union U_SC_USBPHY_CTRL0_U */
/* USBPHY¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x20A0 */
typedef union tagScUsbphyCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    usbphy_compdistune    : 3   ; /* [2..0]  */
        unsigned int    usbphy_sqrxtune       : 3   ; /* [5..3]  */
        unsigned int    usbphy_otgtune        : 3   ; /* [8..6]  */
        unsigned int    usbphy_txhsxvtune     : 2   ; /* [10..9]  */
        unsigned int    usbphy_txfslstune     : 4   ; /* [14..11]  */
        unsigned int    usbphy_txvreftune     : 4   ; /* [18..15]  */
        unsigned int    usbphy_txrisetune     : 2   ; /* [20..19]  */
        unsigned int    usbphy_txrestune      : 2   ; /* [22..21]  */
        unsigned int    usbphy_txpreempamptune : 2   ; /* [24..23]  */
        unsigned int    usbphy_txpreemppulsetune : 1   ; /* [25]  */
        unsigned int    reserved_0            : 6   ; /* [31..26]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USBPHY_CTRL0_U;

/* Define the union U_SC_USBPHY_CTRL1_U */
/* USBPHY¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x20A4 */
typedef union tagScUsbphyCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    usbphy_siddq          : 1   ; /* [0]  */
        unsigned int    usbphy_refclksel      : 2   ; /* [2..1]  */
        unsigned int    usbphy_fsel           : 3   ; /* [5..3]  */
        unsigned int    usbphy_commononn      : 1   ; /* [6]  */
        unsigned int    usbphy_retenablen     : 1   ; /* [7]  */
        unsigned int    usbphy_otgdisabled    : 1   ; /* [8]  */
        unsigned int    usbphy_sleepm         : 1   ; /* [9]  */
        unsigned int    usbphy_wordinterface0 : 1   ; /* [10]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USBPHY_CTRL1_U;

/* Define the union U_SC_M3_CTRL0_U */
/* M3¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x20B0 */
typedef union tagScM3Ctrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    m3_stcalib            : 26  ; /* [25..0]  */
        unsigned int    m3_intnmi             : 1   ; /* [26]  */
        unsigned int    m3_rxev               : 1   ; /* [27]  */
        unsigned int    m3_wicenreq           : 1   ; /* [28]  */
        unsigned int    m3_bigend             : 1   ; /* [29]  */
        unsigned int    m3_dbgen              : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_M3_CTRL0_U;

/* Define the union U_SC_M3_CTRL1_U */
/* M3¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x20B4 */
typedef union tagScM3Ctrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    m3_rst_status         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_M3_CTRL1_U;

/* Define the union U_SC_VMID_CTRL0_U */
/* m3 vmidÅäÖÃ */
/* 0x20E0 */
typedef union tagScVmidCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    m3_vmid0              : 8   ; /* [7..0]  */
        unsigned int    m3_vmid1              : 8   ; /* [15..8]  */
        unsigned int    m3_vmid2              : 8   ; /* [23..16]  */
        unsigned int    reserved_0            : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL0_U;

/* Define the union U_SC_VMID_CTRL1_U */
/* usb vmidÅäÖÃ */
/* 0x20E4 */
typedef union tagScVmidCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    usbhost_echi_vmid     : 8   ; /* [7..0]  */
        unsigned int    usbhost_ochi_vmid     : 8   ; /* [15..8]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_VMID_CTRL1_U;

/* Define the union U_SC_I2C_M_VMID_U */
/* i2c_m vmidÅäÖÃ */
/* 0x20E8 */
typedef union tagScI2cMVmid
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c_m_vmid            : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_M_VMID_U;

/* Define the union U_SC_M3_REMAP_ADDR_EN_U */
/* m3µØÖ·Ó³ÉäÊ¹ÄÜ */
/* 0x2180 */
typedef union tagScM3RemapAddrEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    m3_remap_addr_en      : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_M3_REMAP_ADDR_EN_U;

/* Define the union U_SC_ITS_M3_INT_MUX_SEL_U */
/* ¶ÔÖÐ¶ÏËÍµ½ITS»¹ÊÇM3£¬×öMUXÑ¡Ôñ */
/* 0x21F0 */
typedef union tagScItsM3IntMuxSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    int_spi_mux_sel       : 1   ; /* [0]  */
        unsigned int    int_i2c2_mux_sel      : 1   ; /* [1]  */
        unsigned int    int_i2c1_mux_sel      : 1   ; /* [2]  */
        unsigned int    int_i2c0_mux_sel      : 1   ; /* [3]  */
        unsigned int    int_uart1_mux_sel     : 1   ; /* [4]  */
        unsigned int    int_uart0_mux_sel     : 1   ; /* [5]  */
        unsigned int    int_timer2_mux_sel    : 1   ; /* [6]  */
        unsigned int    int_timer1_mux_sel    : 1   ; /* [7]  */
        unsigned int    int_ext2_mux_sel      : 1   ; /* [8]  */
        unsigned int    int_ext1_mux_sel      : 1   ; /* [9]  */
        unsigned int    int_ext0_mux_sel      : 1   ; /* [10]  */
        unsigned int    reserved_0            : 21  ; /* [31..11]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_M3_INT_MUX_SEL_U;

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

/* Define the union U_SC_PM_CTRL0_U */
/* PW_STAT_CTRLµÄ¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2380 */
typedef union tagScPmCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pw_mode_ctrl          : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_CTRL0_U;

/* Define the union U_SC_PM_CTRL1_U */
/* PW_STAT_CTRLµÄ¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2384 */
typedef union tagScPmCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pwr_m3_ctrl_en        : 1   ; /* [0]  */
        unsigned int    pw_dn_4s_hard_invalid : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_CTRL1_U;

/* Define the union U_SC_PM_CTRL2_U */
/* PW_STAT_CTRLµÄ¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2388 */
typedef union tagScPmCtrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pwr_m3_ctrl           : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_CTRL2_U;

/* Define the union U_SC_PM_INT_U */
/* PW_STAT_CTRLÄÚÖÐ¶ÏÔ´¼Ä´æÆ÷ */
/* 0x238C */
typedef union tagScPmInt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    timeout_10ms          : 3   ; /* [2..0]  */
        unsigned int    pw_btn_pw_dn          : 1   ; /* [3]  */
        unsigned int    pw_btn_pw_up          : 1   ; /* [4]  */
        unsigned int    pw_btn_pw_dn_4s_soft  : 1   ; /* [5]  */
        unsigned int    pw_btn_pw_dn_4s_hard  : 1   ; /* [6]  */
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_INT_U;

/* Define the union U_SC_PM_INT_MASK_U */
/* PW_STAT_CTRLµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
/* 0x2390 */
typedef union tagScPmIntMask
{
    /* Define the struct bits */
    struct
    {
        unsigned int    timeout_10ms_int_mask : 1   ; /* [0]  */
        unsigned int    pw_btn_pw_dn_int_mask : 1   ; /* [1]  */
        unsigned int    pw_btn_pw_up_int_mask : 1   ; /* [2]  */
        unsigned int    pw_btn_pw_dn_4s_soft_int_mask : 1   ; /* [3]  */
        unsigned int    pw_btn_pw_dn_4s_hard_int_mask : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_INT_MASK_U;

/* Define the union U_SC_PM_CTRL3_U */
/* ¿ØÖÆ¹Ü½ÅPM_CTL */
/* 0x2394 */
typedef union tagScPmCtrl3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pad_pm_ctl_out        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_CTRL3_U;

/* Define the union U_SC_PM_ST_U */
/* ÉÏµç¸´Î»Ö¸Ê¾×´Ì¬ÐÅºÅ */
/* 0x2398 */
typedef union tagScPmSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pw_ctrl_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_ST_U;

/* Define the union U_SC_BISR_RE_REPAIR_EN_U */
/* BISRµÄRE REPAIRÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x2D80 */
typedef union tagScBisrReRepairEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    bisr_re_repair_en     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BISR_RE_REPAIR_EN_U;

/* Define the union U_SC_BISR_INT_U */
/* BISR_TOP_IDIEµÄÖÐ¶ÏÔ´¼Ä´æÆ÷ */
/* 0x2D90 */
typedef union tagScBisrInt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hard_repair_done      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BISR_INT_U;

/* Define the union U_SC_BISR_INT_MASK_U */
/* BISR_TOP_IDIEµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
/* 0x2DA0 */
typedef union tagScBisrIntMask
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hard_repair_done_int_mask : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BISR_INT_MASK_U;

/* Define the union U_SC_SMMU_MEM_CTRL0_U */
/* smmu mem¿ØÖÆ¼Ä´æÆ÷0 */
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

/* Define the union U_SC_SRAM_MEM_CTRL_U */
/* sram mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3020 */
typedef union tagScSramMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rashsd_sram      : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_MEM_CTRL_U;

/* Define the union U_SC_SFC_MEM_CTRL_U */
/* sfc mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3030 */
typedef union tagScSfcMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rft_sfc          : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_MEM_CTRL_U;

/* Define the union U_SC_SEC_MEM_CTRL_U */
/* sec mem¿ØÖÆ¼Ä´æÆ÷ */
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

/* Define the union U_SC_WDG_CLK_ST_U */
/* WDGÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5300 */
typedef union tagScWdgClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_wdg_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_CLK_ST_U;

/* Define the union U_SC_TIMER_CLK_ST_U */
/* TIMERÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5304 */
typedef union tagScTimerClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_timer_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_CLK_ST_U;

/* Define the union U_SC_UART_CLK_ST_U */
/* UARTÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5308 */
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

/* Define the union U_SC_ITS_CLK_ST_U */
/* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_IPCM_CLK_ST_U */
/* IPCMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5310 */
typedef union tagScIpcmClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_ipcm_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_CLK_ST_U;

/* Define the union U_SC_SRAM_CLK_ST_U */
/* SRAMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5314 */
typedef union tagScSramClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sram_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_CLK_ST_U;

/* Define the union U_SC_SFC_CLK_ST_U */
/* SFCÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5318 */
typedef union tagScSfcClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sfc_st            : 1   ; /* [0]  */
        unsigned int    clk_sfc_bus_st        : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_CLK_ST_U;

/* Define the union U_SC_I2CM_CLK_ST_U */
/* I2CMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x531C */
typedef union tagScI2cmClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_i2c_m_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_CLK_ST_U;

/* Define the union U_SC_I2C_CLK_ST_U */
/* I2CÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5320 */
typedef union tagScI2cClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_i2c0_st           : 1   ; /* [0]  */
        unsigned int    clk_i2c1_st           : 1   ; /* [1]  */
        unsigned int    clk_i2c2_st           : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_CLK_ST_U;

/* Define the union U_SC_GPIO_CLK_ST_U */
/* GPIOÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5324 */
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

/* Define the union U_SC_CPU_CLK_ST_U */
/* M3 CPUÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5328 */
typedef union tagScCpuClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_cpu_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_CLK_ST_U;

/* Define the union U_SC_USB_CLK_ST_U */
/* USBÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5334 */
typedef union tagScUsbClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_usb_ahb_st        : 1   ; /* [0]  */
        unsigned int    clk_usb_phy_core_st   : 1   ; /* [1]  */
        unsigned int    clk_usb_ohci_48m_st   : 1   ; /* [2]  */
        unsigned int    clk_usb_ohci_12m_st   : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_CLK_ST_U;

/* Define the union U_SC_SPI_CLK_ST_U */
/* SPIÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5344 */
typedef union tagScSpiClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_spi_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_CLK_ST_U;

/* Define the union U_SC_SEC_CLK_ST_U */
/* SECÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_WDG_RESET_ST_U */
/* WDG¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A00 */
typedef union tagScWdgResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    wdg_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_WDG_RESET_ST_U;

/* Define the union U_SC_TIMER_RESET_ST_U */
/* TIMER¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A04 */
typedef union tagScTimerResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    timer_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TIMER_RESET_ST_U;

/* Define the union U_SC_UART_RESET_ST_U */
/* UART¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A08 */
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

/* Define the union U_SC_ITS_RESET_ST_U */
/* ITS¸´Î»×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_IPCM_RESET_ST_U */
/* IPCM¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A10 */
typedef union tagScIpcmResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ipcm_srst_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_IPCM_RESET_ST_U;

/* Define the union U_SC_SRAM_RESET_ST_U */
/* SRAM¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A14 */
typedef union tagScSramResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_srst_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_RESET_ST_U;

/* Define the union U_SC_SFC_RESET_ST_U */
/* SFC¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A18 */
typedef union tagScSfcResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sfc_srst_st           : 1   ; /* [0]  */
        unsigned int    sfc_bus_srst_st       : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SFC_RESET_ST_U;

/* Define the union U_SC_I2CM_RESET_ST_U */
/* I2CM¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A1C */
typedef union tagScI2cmResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c_m_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2CM_RESET_ST_U;

/* Define the union U_SC_I2C_RESET_ST_U */
/* I2C¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A20 */
typedef union tagScI2cResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    i2c0_srst_st          : 1   ; /* [0]  */
        unsigned int    i2c1_srst_st          : 1   ; /* [1]  */
        unsigned int    i2c2_srst_st          : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_I2C_RESET_ST_U;

/* Define the union U_SC_GPIO_RESET_ST_U */
/* GPIO¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A24 */
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

/* Define the union U_SC_CPU_RESET_ST_U */
/* M3 CPU¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A28 */
typedef union tagScCpuResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    cpu_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_CPU_RESET_ST_U;

/* Define the union U_SC_USB_RESET_ST_U */
/* USB¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A44 */
typedef union tagScUsbResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    usbhost_utmi0_srst_st : 1   ; /* [0]  */
        unsigned int    usbhost_phy0_srst_st  : 1   ; /* [1]  */
        unsigned int    usbhost_ahb_srst_st   : 1   ; /* [2]  */
        unsigned int    usbhost_phy0_port_srst_st : 1   ; /* [3]  */
        unsigned int    usbhost_phy0_por_srst_st : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_USB_RESET_ST_U;

/* Define the union U_SC_SPI_RESET_ST_U */
/* SPI¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A4C */
typedef union tagScSpiResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    spi_srst_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SPI_RESET_ST_U;

/* Define the union U_SC_SEC_RESET_ST_U */
/* SEC¸´Î»×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_SEC_BUILD_RESET_ST_U */
/* SEC BUILD¸´Î»×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_PW_CTRL_RESET_ST_U */
/* PW_CTRL¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A60 */
typedef union tagScPwCtrlResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pw_ctrl_srst_st       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PW_CTRL_RESET_ST_U;

/* Define the union U_SC_MBIST_CPUI_ENABLE_U */
/* CPUÆô¶¯MBISTÑ¡ÔñÐÅºÅ£»Ð¾Æ¬µçÂ·²âÊÔÊ¹ÓÃ¡£ */
/* 0x5C00 */
typedef union tagScMbistCpuiEnable
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mbist_cpui_enable     : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MBIST_CPUI_ENABLE_U;

/* Define the union U_SC_MBIST_CPUI_RESET_N_U */
/* CPUÆô¶¯MBIST¸´Î»ÐÅºÅ */
/* 0x5C04 */
typedef union tagScMbistCpuiResetN
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mbist_cpui_reset_n    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MBIST_CPUI_RESET_N_U;

/* Define the union U_SC_MBIST_CPUI_WRITE_EN_U */
/* DFT MBISTÄ£Ê½ÏÂTMBµçÂ·CPU2TAP½Ó¿ÚÅäÖÃ¼Ä´æÆ÷£»Ð¾Æ¬µçÂ·²âÊÔÊ¹ÓÃ¡£ */
/* 0x5C0C */
typedef union tagScMbistCpuiWriteEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mbist_cpui_write_en   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MBIST_CPUI_WRITE_EN_U;

/* Define the union U_SC_SRAM_ST0_U */
/* sram×´Ì¬¼Ä´æÆ÷0 */
/* 0x6000 */
typedef union tagScSramSt0
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

} SC_SRAM_ST0_U;

/* Define the union U_SC_SRAM_ST3_U */
/* sram×´Ì¬¼Ä´æÆ÷3 */
/* 0x600C */
typedef union tagScSramSt3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_count_ecc_sglerr : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST3_U;

/* Define the union U_SC_SRAM_ST4_U */
/* sram×´Ì¬¼Ä´æÆ÷4 */
/* 0x6010 */
typedef union tagScSramSt4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_count_ecc_mulerr : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST4_U;

/* Define the union U_SC_SRAM_ST5_U */
/* sram×´Ì¬¼Ä´æÆ÷5 */
/* 0x6014 */
typedef union tagScSramSt5
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sram_dfx_dbg          : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SRAM_ST5_U;

/* Define the union U_SC_M3_STAT_U */
/*  */
/* 0x6100 */
typedef union tagScM3Stat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    m3_brchstat           : 4   ; /* [3..0]  */
        unsigned int    m3_halted             : 1   ; /* [4]  */
        unsigned int    m3_lockup             : 1   ; /* [5]  */
        unsigned int    m3_sleeping           : 1   ; /* [6]  */
        unsigned int    m3_currpri            : 8   ; /* [14..7]  */
        unsigned int    m3_wicenack           : 1   ; /* [15]  */
        unsigned int    m3_wakeup             : 1   ; /* [16]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_M3_STAT_U;

/* Define the union U_SC_PM_ST0_U */
/* PW_STAT_CTRLµÄÄÚ²¿×´Ì¬ */
/* 0x6380 */
typedef union tagScPmSt0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pw_curr_stat          : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_ST0_U;

/* Define the union U_SC_PM_ST1_U */
/* À´×Ô¹Ü½ÅµÄPWRGD×´Ì¬ */
/* 0x6384 */
typedef union tagScPmSt1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pwrgd                 : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_ST1_U;

/* Define the union U_SC_PM_ST2_U */
/* Êä³ö¸ø¹Ü½ÅµÄPWR_CTRL2~0 */
/* 0x6388 */
typedef union tagScPmSt2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pwr_ctrl              : 3   ; /* [2..0]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_ST2_U;

/* Define the union U_SC_PM_ST3_U */
/* MEÏà¹Ø¹Ü½Å×´Ì¬ */
/* 0x638C */
typedef union tagScPmSt3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pad_sm_alert_n_in     : 1   ; /* [0]  */
        unsigned int    pad_ext_int0_in       : 1   ; /* [1]  */
        unsigned int    pad_ext_int1_in       : 1   ; /* [2]  */
        unsigned int    pad_ext_int2_in       : 1   ; /* [3]  */
        unsigned int    pad_me_mst_in         : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PM_ST3_U;

/* Define the union U_SC_DBG_AUTH_CTRL_U */
/* °²È«µÈ¼¶ÊÚÈ¨¿ØÖÆ¼Ä´æÆ÷¡£
£¨×¢Òâ£º1£¬´Ë¼Ä´æÆ÷¿Õ¼ä±ØÐëÊÇ°²È«·ÃÎÊ²ÅÄÜ³É¹¦£»2£¬Ä¬ÈÏ¾ùÊÇ´ò¿ªµ÷ÊÔÄ£Ê½£¬ÓÐÂ©¶´£¬ÔÚÕý³£¹¤×÷Ê±Çë³õÊ¼»¯¹Ø±Õµ÷ÊÔ¹¦ÄÜ£© */
/* 0x6700 */
typedef union tagScDbgAuthCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rsa_djtag_sec_acc_en  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DBG_AUTH_CTRL_U;

/* Define the union U_SC_BOOT_CTRL_ST_U */
/* SC_BOOT_CTRL_ST±íÊ¾CPUÆô¶¯¿ØÖÆµÄPADºÍeFuseÐÅºÅ×´Ì¬ */
/* 0xE084 */
typedef union tagScBootCtrlSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    efuse_ns_forbid       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_BOOT_CTRL_ST_U;

/* Define the union U_SC_EFUSE_RD_ADDR_U */
/* ¶ÁDIEIDÊ±ËùÓÃµØÖ·£¨°²È«¼Ä´æÆ÷£© */
/* 0xE100 */
typedef union tagScEfuseRdAddr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    efuse_rd_addr         : 7   ; /* [6..0]  */
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_EFUSE_RD_ADDR_U;

#endif

#endif // __M3_SUB_REG_OFFSET_H__

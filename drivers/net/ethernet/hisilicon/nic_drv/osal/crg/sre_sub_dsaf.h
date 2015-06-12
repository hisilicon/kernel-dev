/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpaISWHqOHbuE99WnZz8Urp0KNb0WWl+qRa4EjbbH6IvvJ8uAZ7aN4Vd0NGho
uLjlYuOoFI7qbbSv+pGrmQZd2p+vvraTSUNhoxo1HN7RmTat5sOdsNViVvDbWAVqNe05Ke/b
cFMyGOT741d0G9dovxshQzNBQQ9w0z82rrCcfBOoJZPdqrEbnhKv3S6fNEkaug==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  °æÈ¨ËùÓÐ (C), 2011-2021, »ªÎª¼¼ÊõÓÐÏÞ¹«Ë¾

 ******************************************************************************
  ÎÄ ¼þ Ãû   : sre_sub_dsaf.h
  °æ ±¾ ºÅ   : ³õ¸å
  ×÷    Õß   : z00176027
  Éú³ÉÈÕÆÚ   : 2013Äê10ÔÂ17ÈÕ
  ×î½üÐÞ¸Ä   :
  ¹¦ÄÜÃèÊö   : p660 dsaf×ÓÏµÍ³ ¸´Î»¼°½â¸´Î»Ïà¹ØÍ·ÎÄ¼þ
  º¯ÊýÁÐ±í   :
  ÐÞ¸ÄÀúÊ·   :
  1.ÈÕ    ÆÚ   : 2013Äê10ÔÂ17ÈÕ
    ×÷    Õß   : z00176027
    ÐÞ¸ÄÄÚÈÝ   : ´´½¨ÎÄ¼þ

******************************************************************************/
#ifndef __C_SRE_SUB_DSAF_H__
#define __C_SRE_SUB_DSAF_H__

#include "hrdCommon.h"
#include "hrdOs.h"
#include "osal_api.h"
#include "hrd_crg_api.h"

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

    /* dsaf_subctrl Ä£¿é¼Ä´æÆ÷»ùµØÖ· */
#define DSAF_SUB_BASE                             (0xC0000000)
#define DSAF_SUB_BASE_SIZE                        (0x10000)

/******************************************************************************/
/*                      PHOSPHOR DSAF_SUB Registers' Definitions                            */
/******************************************************************************/

#define DSAF_SUB_SC_NT_SRAM_CLK_SEL_REG                (DSAF_SUB_BASE + 0x100)  /* Node TableÄ£¿éTCAM±ímemoryÊ±ÖÓ¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_CRG_CTRL0_REG              (DSAF_SUB_BASE + 0x180)  /* ÅäÖÃHILINK CRG */
#define DSAF_SUB_SC_HILINK3_CRG_CTRL1_REG              (DSAF_SUB_BASE + 0x184)  /* ÅäÖÃHILINK CRG */
#define DSAF_SUB_SC_HILINK3_CRG_CTRL2_REG              (DSAF_SUB_BASE + 0x188)  /* ÅäÖÃHILINK CRG */
#define DSAF_SUB_SC_HILINK3_CRG_CTRL3_REG              (DSAF_SUB_BASE + 0x18C)  /* ÅäÖÃHILINK CRG */
#define DSAF_SUB_SC_HILINK4_CRG_CTRL0_REG              (DSAF_SUB_BASE + 0x190)  /* ÅäÖÃHILINK CRG */
#define DSAF_SUB_SC_HILINK4_CRG_CTRL1_REG              (DSAF_SUB_BASE + 0x194)  /* ÅäÖÃHILINK CRG */
#define DSAF_SUB_SC_DSAF_CLK_EN_REG                    (DSAF_SUB_BASE + 0x300)  /* DSAFÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_DSAF_CLK_DIS_REG                   (DSAF_SUB_BASE + 0x304)  /* DSAFÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_NT_CLK_EN_REG                      (DSAF_SUB_BASE + 0x308)  /* NTÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_NT_CLK_DIS_REG                     (DSAF_SUB_BASE + 0x30C)  /* NTÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_XGE_CLK_EN_REG                     (DSAF_SUB_BASE + 0x310)  /* XGEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_XGE_CLK_DIS_REG                    (DSAF_SUB_BASE + 0x314)  /* XGEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_GE_CLK_EN_REG                      (DSAF_SUB_BASE + 0x318)  /* GEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_GE_CLK_DIS_REG                     (DSAF_SUB_BASE + 0x31C)  /* GEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_PPE_CLK_EN_REG                     (DSAF_SUB_BASE + 0x320)  /* PPEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_PPE_CLK_DIS_REG                    (DSAF_SUB_BASE + 0x324)  /* PPEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ROCEE_CLK_EN_REG                   (DSAF_SUB_BASE + 0x328)  /* ROCEEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ROCEE_CLK_DIS_REG                  (DSAF_SUB_BASE + 0x32C)  /* ROCEEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_CPU_CLK_EN_REG                     (DSAF_SUB_BASE + 0x330)  /* CPUÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_CPU_CLK_DIS_REG                    (DSAF_SUB_BASE + 0x334)  /* CPUÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SAS_CLK_EN_REG                     (DSAF_SUB_BASE + 0x338)  /* SASÊ±ÖÓÃÅ¿ØÐÅºÅ */
#define DSAF_SUB_SC_SAS_CLK_DIS_REG                    (DSAF_SUB_BASE + 0x33C)  /* SASÊ±ÖÓÃÅ¿ØÐÅºÅ */
#define DSAF_SUB_SC_ITS_CLK_EN_REG                     (DSAF_SUB_BASE + 0x340)  /* itsÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ITS_CLK_DIS_REG                    (DSAF_SUB_BASE + 0x344)  /* itsÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SRAM_CLK_EN_REG                    (DSAF_SUB_BASE + 0x348)  /* SRAMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SRAM_CLK_DIS_REG                   (DSAF_SUB_BASE + 0x34C)  /* SRAMÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_RCB_PPE_COM_CLK_EN_REG             (DSAF_SUB_BASE + 0x350)  /* RCB PPE¹«¹²Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_RCB_PPE_COM_CLK_DIS_REG            (DSAF_SUB_BASE + 0x354)  /* RCB PPE¹«¹²Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_TIMER_CLK_EN_REG                   (DSAF_SUB_BASE + 0x358)  /* TIMERÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_TIMER_CLK_DIS_REG                  (DSAF_SUB_BASE + 0x35C)  /* TIMERÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SLLC_CLK_EN_REG                    (DSAF_SUB_BASE + 0x360)  /* SLLC1Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SLLC_CLK_DIS_REG                   (DSAF_SUB_BASE + 0x364)  /* SLLC1Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define DSAF_SUB_SC_XBAR_RESET_REQ_REG                 (DSAF_SUB_BASE + 0xA00)  /* XBARÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_XBAR_RESET_DREQ_REG                (DSAF_SUB_BASE + 0xA04)  /* XBARÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_NT_RESET_REQ_REG                   (DSAF_SUB_BASE + 0xA08)  /* NTÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_NT_RESET_DREQ_REG                  (DSAF_SUB_BASE + 0xA0C)  /* NTÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_XGE_RESET_REQ_REG                  (DSAF_SUB_BASE + 0xA10)  /* XGEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_XGE_RESET_DREQ_REG                 (DSAF_SUB_BASE + 0xA14)  /* XGEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_GE_RESET_REQ0_REG                  (DSAF_SUB_BASE + 0xA18)  /* GEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_GE_RESET_DREQ0_REG                 (DSAF_SUB_BASE + 0xA1C)  /* GEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_GE_RESET_REQ1_REG                  (DSAF_SUB_BASE + 0xA20)  /* GEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷1 */
#define DSAF_SUB_SC_GE_RESET_DREQ1_REG                 (DSAF_SUB_BASE + 0xA24)  /* GEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷1 */
#define DSAF_SUB_SC_PPE_RESET_REQ_REG                  (DSAF_SUB_BASE + 0xA48)  /* PPEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_PPE_RESET_DREQ_REG                 (DSAF_SUB_BASE + 0xA4C)  /* PPEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ROCEE_RESET_REQ_REG                (DSAF_SUB_BASE + 0xA50)  /* ROCEEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ROCEE_RESET_DREQ_REG               (DSAF_SUB_BASE + 0xA54)  /* ROCEEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_CPU_RESET_REQ_REG                  (DSAF_SUB_BASE + 0xA58)  /* CPUÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_CPU_RESET_DREQ_REG                 (DSAF_SUB_BASE + 0xA5C)  /* CPUÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SAS_RESET_REQ_REG                  (DSAF_SUB_BASE + 0xA60)  /* SASÈí¸´Î»ÇëÇó */
#define DSAF_SUB_SC_SAS_RESET_DREQ_REG                 (DSAF_SUB_BASE + 0xA64)  /* SAS³·ÏúÈí¸´Î»ÇëÇó */
#define DSAF_SUB_SC_SLLC_TSVRX_RESET_REQ_REG           (DSAF_SUB_BASE + 0xA68)  /* SLLC TSVRXÍ¨µÀÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SLLC_TSVRX_RESET_DREQ_REG          (DSAF_SUB_BASE + 0xA6C)  /* SLLC TSVRXÍ¨µÀÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ITS_RESET_REQ_REG                  (DSAF_SUB_BASE + 0xA70)  /* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ITS_RESET_DREQ_REG                 (DSAF_SUB_BASE + 0xA74)  /* ITSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SRAM_RESET_REQ_REG                 (DSAF_SUB_BASE + 0xA80)  /* SRAMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SRAM_RESET_DREQ_REG                (DSAF_SUB_BASE + 0xA84)  /* SRAMÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_RCB_PPE_COM_RESET_REQ_REG          (DSAF_SUB_BASE + 0xA88)  /* RCB PPE¹«¹²Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_RCB_PPE_COM_RESET_DREQ_REG         (DSAF_SUB_BASE + 0xA8C)  /* RCB PPE¹«¹²Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_TIMER_RESET_REQ_REG                (DSAF_SUB_BASE + 0xA98)  /* TIMERÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_TIMER_RESET_DREQ_REG               (DSAF_SUB_BASE + 0xA9C)  /* TIMERÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SLLC_RESET_REQ_REG                 (DSAF_SUB_BASE + 0xAA0)  /* SLLC1Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SLLC_RESET_DREQ_REG                (DSAF_SUB_BASE + 0xAA4)  /* SLLC1Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_DISPATCH_DAW_EN_REG                (DSAF_SUB_BASE + 0x1000) /* dispatch daw enÅäÖÃ */
#define DSAF_SUB_SC_DISPATCH_DAW_ARRAY0_REG            (DSAF_SUB_BASE + 0x1004) /* dispatch dawÅäÖÃÕóÁÐ0 */
#define DSAF_SUB_SC_DISPATCH_DAW_ARRAY1_REG            (DSAF_SUB_BASE + 0x1008) /* dispatch dawÅäÖÃÕóÁÐ1 */
#define DSAF_SUB_SC_DISPATCH_DAW_ARRAY2_REG            (DSAF_SUB_BASE + 0x100C) /* dispatch dawÅäÖÃÕóÁÐ2 */
#define DSAF_SUB_SC_DISPATCH_DAW_ARRAY3_REG            (DSAF_SUB_BASE + 0x1010) /* dispatch dawÅäÖÃÕóÁÐ3 */
#define DSAF_SUB_SC_DISPATCH_DAW_ARRAY4_REG            (DSAF_SUB_BASE + 0x1014) /* dispatch dawÅäÖÃÕóÁÐ4 */
#define DSAF_SUB_SC_DISPATCH_DAW_ARRAY5_REG            (DSAF_SUB_BASE + 0x1018) /* dispatch dawÅäÖÃÕóÁÐ5 */
#define DSAF_SUB_SC_DISPATCH_DAW_ARRAY6_REG            (DSAF_SUB_BASE + 0x101C) /* dispatch dawÅäÖÃÕóÁÐ6 */
#define DSAF_SUB_SC_DISPATCH_DAW_ARRAY7_REG            (DSAF_SUB_BASE + 0x1020) /* dispatch dawÅäÖÃÕóÁÐ7 */
#define DSAF_SUB_SC_DISPATCH_RETRY_CONTROL_REG         (DSAF_SUB_BASE + 0x1030) /* dispatch retry¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_DISPATCH_INTMASK_REG               (DSAF_SUB_BASE + 0x1100) /* dispatchµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
#define DSAF_SUB_SC_DISPATCH_RAWINT_REG                (DSAF_SUB_BASE + 0x1104) /* dispatchµÄÔ­Ê¼ÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_DISPATCH_INTSTAT_REG               (DSAF_SUB_BASE + 0x1108) /* dispatchµÄÆÁ±ÎºóµÄÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_DISPATCH_INTCLR_REG                (DSAF_SUB_BASE + 0x110C) /* dispatchµÄÖÐ¶ÏÇå³ý¼Ä´æÆ÷ */
#define DSAF_SUB_SC_DISPATCH_ERRSTAT_REG               (DSAF_SUB_BASE + 0x1110) /* dispatchµÄERR×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_REMAP_CTRL_REG                     (DSAF_SUB_BASE + 0x1200) /* subsysµÄÆô¶¯Remap¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SRAM_CTRL0_REG                     (DSAF_SUB_BASE + 0x2030) /* sram¿ØÖÆ¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_M3_CTRL_REG                        (DSAF_SUB_BASE + 0x2040) /* M3¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_REMAP_SRAM_ADDR_CTRL_REG           (DSAF_SUB_BASE + 0x2050) /* M3 remapµ½sramµÄµØÖ·ÅäÖÃ */
#define DSAF_SUB_SC_LIGHT_MODULE_DETECT_EN_REG         (DSAF_SUB_BASE + 0x2060) /* ¹âÄ£¿éSDÐÅºÅ¼ì²âÊ¹ÄÜ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_TIMER_CLKEN_CTRL_REG               (DSAF_SUB_BASE + 0x2070) /* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_VMID_CTRL0_REG                     (DSAF_SUB_BASE + 0x20E0) /* M3 VMIDÅäÖÃ */
#define DSAF_SUB_SC_ROCEE_VMID_REG                     (DSAF_SUB_BASE + 0x20E4) /* ROCEE VMIDÅäÖÃ */
#define DSAF_SUB_SC_ITS_M3_INT_MUX_SEL_REG             (DSAF_SUB_BASE + 0x21F0) /* ¶ÔÖÐ¶ÏËÍµ½ITS»¹ÊÇM3£¬×öMUXÑ¡Ôñ */
#define DSAF_SUB_SC_FTE_MUX_SEL_REG                    (DSAF_SUB_BASE + 0x2200)
#define DSAF_SUB_SC_TCAM_MBIST_EN_REG                  (DSAF_SUB_BASE + 0x2300) /* DSAFÖÐTCAM MBISTÆô¶¯Ê¹ÄÜ¡£ */
#define DSAF_SUB_SC_HILINK2_LRSTB_MUX_CTRL_REG         (DSAF_SUB_BASE + 0x2340) /* HILINK2 lrstb[7:0]µÄMUXÑ¡Ôñ¿ØÖÆ */
#define DSAF_SUB_SC_HILINK2_MACRO_SS_REFCLK_REG        (DSAF_SUB_BASE + 0x2400) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_CS_REFCLK_DIRSEL_REG (DSAF_SUB_BASE + 0x2404) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_LIFECLK2DIG_SEL_REG  (DSAF_SUB_BASE + 0x2408) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_CORE_CLK_SELEXT_REG  (DSAF_SUB_BASE + 0x240C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_CORE_CLK_SEL_REG     (DSAF_SUB_BASE + 0x2410) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_CTRL_BUS_MODE_REG    (DSAF_SUB_BASE + 0x2414) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_MACROPWRDB_REG       (DSAF_SUB_BASE + 0x2418) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_GRSTB_REG            (DSAF_SUB_BASE + 0x241C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_BIT_SLIP_REG         (DSAF_SUB_BASE + 0x2420) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_LRSTB_REG            (DSAF_SUB_BASE + 0x2424) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_SS_REFCLK_REG        (DSAF_SUB_BASE + 0x2500) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_CS_REFCLK_DIRSEL_REG (DSAF_SUB_BASE + 0x2504) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_LIFECLK2DIG_SEL_REG  (DSAF_SUB_BASE + 0x2508) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_CORE_CLK_SELEXT_REG  (DSAF_SUB_BASE + 0x250C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_CORE_CLK_SEL_REG     (DSAF_SUB_BASE + 0x2510) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_CTRL_BUS_MODE_REG    (DSAF_SUB_BASE + 0x2514) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_MACROPWRDB_REG       (DSAF_SUB_BASE + 0x2518) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_GRSTB_REG            (DSAF_SUB_BASE + 0x251C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_BIT_SLIP_REG         (DSAF_SUB_BASE + 0x2520) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_LRSTB_REG            (DSAF_SUB_BASE + 0x2524) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_SS_REFCLK_REG        (DSAF_SUB_BASE + 0x2600) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_CS_REFCLK_DIRSEL_REG (DSAF_SUB_BASE + 0x2604) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_LIFECLK2DIG_SEL_REG  (DSAF_SUB_BASE + 0x2608) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_CORE_CLK_SELEXT_REG  (DSAF_SUB_BASE + 0x260C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_CORE_CLK_SEL_REG     (DSAF_SUB_BASE + 0x2610) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_CTRL_BUS_MODE_REG    (DSAF_SUB_BASE + 0x2614) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_MACROPWRDB_REG       (DSAF_SUB_BASE + 0x2618) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_GRSTB_REG            (DSAF_SUB_BASE + 0x261C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_BIT_SLIP_REG         (DSAF_SUB_BASE + 0x2620) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_LRSTB_REG            (DSAF_SUB_BASE + 0x2624) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_M3_I_ADDR_H_REG                    (DSAF_SUB_BASE + 0x2E00) /* M3Ö¸ÁîÍ¨µÀ¸ß32Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
#define DSAF_SUB_SC_M3_D_ADDR_H_REG                    (DSAF_SUB_BASE + 0x2E04) /* M3Êý¾ÝÍ¨µÀ¸ß32Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
#define DSAF_SUB_SC_M3_S_ADDR_H_REG                    (DSAF_SUB_BASE + 0x2E08) /* M3ÍâÉèÍ¨µÀ¸ß32Î»µØÖ·²¹³ä¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SMMU_MEM_CTRL0_REG                 (DSAF_SUB_BASE + 0x3000) /* smmu mem¿ØÖÆ¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_SMMU_MEM_CTRL1_REG                 (DSAF_SUB_BASE + 0x3004) /* smmu mem¿ØÖÆ¼Ä´æÆ÷1 */
#define DSAF_SUB_SC_SMMU_MEM_CTRL2_REG                 (DSAF_SUB_BASE + 0x3008) /* smmu mem¿ØÖÆ¼Ä´æÆ÷2 */
#define DSAF_SUB_SC_SLLC1_MEM_CTRL_REG                 (DSAF_SUB_BASE + 0x3010) /* sllc1 mem¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SRAM_MEM_CTRL_REG                  (DSAF_SUB_BASE + 0x3020) /* sram mem¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ROCEE_MEM_CTRL0_REG                (DSAF_SUB_BASE + 0x3030) /* rocee mem¿ØÖÆ¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_ROCEE_MEM_CTRL1_REG                (DSAF_SUB_BASE + 0x3034) /* rocee mem¿ØÖÆ¼Ä´æÆ÷1 */
#define DSAF_SUB_SC_XGE_MEM_CTRL0_REG                  (DSAF_SUB_BASE + 0x3040) /* xge mem¿ØÖÆ¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_XGE_MEM_CTRL1_REG                  (DSAF_SUB_BASE + 0x3044) /* xge mem¿ØÖÆ¼Ä´æÆ÷1 */
#define DSAF_SUB_SC_SAS_MEM_CTRL_REG                   (DSAF_SUB_BASE + 0x3050) /* sas mem¿ØÖÆ¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_GE_MEM_CTRL_REG                    (DSAF_SUB_BASE + 0x3060) /* ge mem¿ØÖÆ¼Ä´æÆ÷ */
#define DSAF_SUB_SC_XBAR_MEM_CTRL0_REG                 (DSAF_SUB_BASE + 0x3070) /* xbar mem¿ØÖÆ¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_XBAR_MEM_CTRL1_REG                 (DSAF_SUB_BASE + 0x3074) /* xbar mem¿ØÖÆ¼Ä´æÆ÷1 */
#define DSAF_SUB_SC_RCB_MEM_CTRL0_REG                  (DSAF_SUB_BASE + 0x3080) /* rcb mem¿ØÖÆ¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_RCB_MEM_CTRL1_REG                  (DSAF_SUB_BASE + 0x3084) /* rcb mem¿ØÖÆ¼Ä´æÆ÷1 */
#define DSAF_SUB_SC_DSAF_CLK_ST_REG                    (DSAF_SUB_BASE + 0x5300) /* DSAFÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_NT_CLK_ST_REG                      (DSAF_SUB_BASE + 0x5304) /* NTÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_XGE_CLK_ST_REG                     (DSAF_SUB_BASE + 0x5308) /* XGEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_GE_CLK_ST_REG                      (DSAF_SUB_BASE + 0x530C) /* GEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_PPE_CLK_ST_REG                     (DSAF_SUB_BASE + 0x5310) /* PPEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ROCEE_CLK_ST_REG                   (DSAF_SUB_BASE + 0x5314) /* ROCEEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_CPU_CLK_ST_REG                     (DSAF_SUB_BASE + 0x5318) /* CPUÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SAS_CLK_ST_REG                     (DSAF_SUB_BASE + 0x531C) /* SASÊ±ÖÓÃÅ¿ØÐÅºÅ×´Ì¬ */
#define DSAF_SUB_SC_ITS_CLK_ST_REG                     (DSAF_SUB_BASE + 0x5320) /* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SRAM_CLK_ST_REG                    (DSAF_SUB_BASE + 0x5324) /* SRAMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_RCB_PPE_COM_CLK_ST_REG             (DSAF_SUB_BASE + 0x5328) /* RCB PPE¹«¹²Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_TIMER_CLK_ST_REG                   (DSAF_SUB_BASE + 0x532C) /* TIMERÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SLLC_CLK_ST_REG                    (DSAF_SUB_BASE + 0x5330) /* SLLC1Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_XBAR_RESET_ST_REG                  (DSAF_SUB_BASE + 0x5A00) /* DSAFÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_NT_RESET_ST_REG                    (DSAF_SUB_BASE + 0x5A04) /* NTÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_XGE_RESET_ST_REG                   (DSAF_SUB_BASE + 0x5A08) /* XGEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_GE_RESET_ST0_REG                   (DSAF_SUB_BASE + 0x5A0C) /* GEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_GE_RESET_ST1_REG                   (DSAF_SUB_BASE + 0x5A10) /* GEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷1 */
#define DSAF_SUB_SC_PPE_RESET_ST_REG                   (DSAF_SUB_BASE + 0x5A24) /* PPEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ROCEE_RESET_ST_REG                 (DSAF_SUB_BASE + 0x5A28) /* ROCEEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_CPU_RESET_ST_REG                   (DSAF_SUB_BASE + 0x5A2C) /* CPUÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SAS_RESET_ST_REG                   (DSAF_SUB_BASE + 0x5A30) /* SASÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SLLC_TSVRX_RESET_ST_REG            (DSAF_SUB_BASE + 0x5A34) /* ÎªSLLC TSVRXÍ¨µÀÈí¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ITS_RESET_ST_REG                   (DSAF_SUB_BASE + 0x5A38) /* ITSÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SRAM_RESET_ST_REG                  (DSAF_SUB_BASE + 0x5A40) /* SRAMÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_RCB_PPE_COM_RESET_ST_REG           (DSAF_SUB_BASE + 0x5A44) /* RCB PPE¹«¹²Èí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_TIMER_RESET_ST_REG                 (DSAF_SUB_BASE + 0x5A4C) /* TIMER¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SLLC_RESET_ST_REG                  (DSAF_SUB_BASE + 0x5A50) /* SLLC1¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_SRAM_ST0_REG                       (DSAF_SUB_BASE + 0x6000) /* sram×´Ì¬¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_SRAM_ST1_REG                       (DSAF_SUB_BASE + 0x6004) /* sram×´Ì¬¼Ä´æÆ÷1 */
#define DSAF_SUB_SC_SRAM_ST2_REG                       (DSAF_SUB_BASE + 0x6008) /* sram×´Ì¬¼Ä´æÆ÷2 */
#define DSAF_SUB_SC_SRAM_ST3_REG                       (DSAF_SUB_BASE + 0x600C) /* sram×´Ì¬¼Ä´æÆ÷3 */
#define DSAF_SUB_SC_SRAM_ST4_REG                       (DSAF_SUB_BASE + 0x6010) /* sram×´Ì¬¼Ä´æÆ÷4 */
#define DSAF_SUB_SC_SRAM_ST5_REG                       (DSAF_SUB_BASE + 0x6014) /* sram×´Ì¬¼Ä´æÆ÷5 */
#define DSAF_SUB_SC_M3_STAT_REG                        (DSAF_SUB_BASE + 0x6100)
#define DSAF_SUB_SC_TCAM_MBIST_ST_REG                  (DSAF_SUB_BASE + 0x6300) /* DSAFÖÐTCAM MBIST²âÊÔ½á¹û×´Ì¬ */
#define DSAF_SUB_SC_GE_ST_REG                          (DSAF_SUB_BASE + 0x6380) /* DSAFÖÐGEµÄ¹¤×÷×´Ì¬Ö¸Ê¾ */
#define DSAF_SUB_SC_XGE_ST_REG                         (DSAF_SUB_BASE + 0x6390) /* DSAFÖÐXGEµÄ¹¤×÷×´Ì¬Ö¸Ê¾ */
#define DSAF_SUB_SC_HILINK2_MACRO_PLLOUTOFLOCK_REG     (DSAF_SUB_BASE + 0x6400) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_PRBS_ERR_REG         (DSAF_SUB_BASE + 0x6404) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK2_MACRO_LOS_REG              (DSAF_SUB_BASE + 0x6408) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_PLLOUTOFLOCK_REG     (DSAF_SUB_BASE + 0x6500) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_PRBS_ERR_REG         (DSAF_SUB_BASE + 0x6504) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK3_MACRO_LOS_REG              (DSAF_SUB_BASE + 0x6508) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_PLLOUTOFLOCK_REG     (DSAF_SUB_BASE + 0x6600) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_PRBS_ERR_REG         (DSAF_SUB_BASE + 0x6604) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_HILINK4_MACRO_LOS_REG              (DSAF_SUB_BASE + 0x6608) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define DSAF_SUB_SC_ECO_RSV0_REG                       (DSAF_SUB_BASE + 0x8000) /* ECO ¼Ä´æÆ÷0 */
#define DSAF_SUB_SC_ECO_RSV1_REG                       (DSAF_SUB_BASE + 0x8004) /* ECO ¼Ä´æÆ÷1 */
#define DSAF_SUB_SC_ECO_RSV2_REG                       (DSAF_SUB_BASE + 0x8008) /* ECO ¼Ä´æÆ÷2 */


#if(ENDNESS == ENDNESS_BIG)

/* Define the union U_SC_NT_SRAM_CLK_SEL_U */
/* Node TableÄ£¿éTCAM±ímemoryÊ±ÖÓ¿ØÖÆ¼Ä´æÆ÷ */
/* 0x100 */
typedef union tagScNtSramClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    nt_sram_clk_sel       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_SRAM_CLK_SEL_U;

/* Define the union U_SC_HILINK3_CRG_CTRL0_U */
/* ÅäÖÃHILINK CRG */
/* 0x180 */
typedef union tagScHilink3CrgCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink3_mode_cfg      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_CRG_CTRL0_U;

/* Define the union U_SC_HILINK3_CRG_CTRL1_U */
/* ÅäÖÃHILINK CRG */
/* 0x184 */
typedef union tagScHilink3CrgCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink3_ge_tx_div_sel : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_CRG_CTRL1_U;

/* Define the union U_SC_HILINK3_CRG_CTRL2_U */
/* ÅäÖÃHILINK CRG */
/* 0x188 */
typedef union tagScHilink3CrgCtrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    ge2_mclk_clk_sel      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_CRG_CTRL2_U;

/* Define the union U_SC_HILINK3_CRG_CTRL3_U */
/* ÅäÖÃHILINK CRG */
/* 0x18C */
typedef union tagScHilink3CrgCtrl3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    ge3_mclk_clk_sel      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_CRG_CTRL3_U;

/* Define the union U_SC_HILINK4_CRG_CTRL0_U */
/* ÅäÖÃHILINK CRG */
/* 0x190 */
typedef union tagScHilink4CrgCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink4_mode_cfg      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_CRG_CTRL0_U;

/* Define the union U_SC_HILINK4_CRG_CTRL1_U */
/* ÅäÖÃHILINK CRG */
/* 0x194 */
typedef union tagScHilink4CrgCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink4_ge_tx_div_sel : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_CRG_CTRL1_U;

/* Define the union U_SC_DSAF_CLK_EN_U */
/* DSAFÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x300 */
typedef union tagScDsafClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
        unsigned int    clk_xbar_com_enb      : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [17..13]  */
        unsigned int    clk_xbar_roce_enb     : 1   ; /* [12]  */
        unsigned int    reserved_2            : 5   ; /* [11..7]  */
        unsigned int    clk_xbar_ppe_enb      : 1   ; /* [6]  */
        unsigned int    clk_xbar_xge5_enb     : 1   ; /* [5]  */
        unsigned int    clk_xbar_xge4_enb     : 1   ; /* [4]  */
        unsigned int    clk_xbar_xge3_enb     : 1   ; /* [3]  */
        unsigned int    clk_xbar_xge2_enb     : 1   ; /* [2]  */
        unsigned int    clk_xbar_xge1_enb     : 1   ; /* [1]  */
        unsigned int    clk_xbar_xge0_enb     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DSAF_CLK_EN_U;

/* Define the union U_SC_DSAF_CLK_DIS_U */
/* DSAFÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x304 */
typedef union tagScDsafClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
        unsigned int    clk_xbar_com_dsb      : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [17..13]  */
        unsigned int    clk_xbar_roce_dsb     : 1   ; /* [12]  */
        unsigned int    reserved_2            : 5   ; /* [11..7]  */
        unsigned int    clk_xbar_ppe_dsb      : 1   ; /* [6]  */
        unsigned int    clk_xbar_xge5_dsb     : 1   ; /* [5]  */
        unsigned int    clk_xbar_xge4_dsb     : 1   ; /* [4]  */
        unsigned int    clk_xbar_xge3_dsb     : 1   ; /* [3]  */
        unsigned int    clk_xbar_xge2_dsb     : 1   ; /* [2]  */
        unsigned int    clk_xbar_xge1_dsb     : 1   ; /* [1]  */
        unsigned int    clk_xbar_xge0_dsb     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DSAF_CLK_DIS_U;

/* Define the union U_SC_NT_CLK_EN_U */
/* NTÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x308 */
typedef union tagScNtClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_nt_enb            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_CLK_EN_U;

/* Define the union U_SC_NT_CLK_DIS_U */
/* NTÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x30C */
typedef union tagScNtClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_nt_dsb            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_CLK_DIS_U;

/* Define the union U_SC_XGE_CLK_EN_U */
/* XGEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x310 */
typedef union tagScXgeClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
        unsigned int    clk_xge5_tx_enb       : 1   ; /* [12]  */
        unsigned int    clk_xge4_tx_enb       : 1   ; /* [11]  */
        unsigned int    clk_xge3_tx_enb       : 1   ; /* [10]  */
        unsigned int    clk_xge2_tx_enb       : 1   ; /* [9]  */
        unsigned int    clk_xge1_tx_enb       : 1   ; /* [8]  */
        unsigned int    clk_xge0_tx_enb       : 1   ; /* [7]  */
        unsigned int    clk_xge5_rx_enb       : 1   ; /* [6]  */
        unsigned int    clk_xge4_rx_enb       : 1   ; /* [5]  */
        unsigned int    clk_xge3_rx_enb       : 1   ; /* [4]  */
        unsigned int    clk_xge2_rx_enb       : 1   ; /* [3]  */
        unsigned int    clk_xge1_rx_enb       : 1   ; /* [2]  */
        unsigned int    clk_xge0_rx_enb       : 1   ; /* [1]  */
        unsigned int    clk_xge_cfg_enb       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_CLK_EN_U;

/* Define the union U_SC_XGE_CLK_DIS_U */
/* XGEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x314 */
typedef union tagScXgeClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
        unsigned int    clk_xge5_tx_dsb       : 1   ; /* [12]  */
        unsigned int    clk_xge4_tx_dsb       : 1   ; /* [11]  */
        unsigned int    clk_xge3_tx_dsb       : 1   ; /* [10]  */
        unsigned int    clk_xge2_tx_dsb       : 1   ; /* [9]  */
        unsigned int    clk_xge1_tx_dsb       : 1   ; /* [8]  */
        unsigned int    clk_xge0_tx_dsb       : 1   ; /* [7]  */
        unsigned int    clk_xge5_rx_dsb       : 1   ; /* [6]  */
        unsigned int    clk_xge4_rx_dsb       : 1   ; /* [5]  */
        unsigned int    clk_xge3_rx_dsb       : 1   ; /* [4]  */
        unsigned int    clk_xge2_rx_dsb       : 1   ; /* [3]  */
        unsigned int    clk_xge1_rx_dsb       : 1   ; /* [2]  */
        unsigned int    clk_xge0_rx_dsb       : 1   ; /* [1]  */
        unsigned int    clk_xge_cfg_dsb       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_CLK_DIS_U;

/* Define the union U_SC_GE_CLK_EN_U */
/* GEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x318 */
typedef union tagScGeClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gmac7_rx_enb      : 1   ; /* [31]  */
        unsigned int    clk_gmac6_rx_enb      : 1   ; /* [30]  */
        unsigned int    clk_gmac5_rx_enb      : 1   ; /* [29]  */
        unsigned int    clk_gmac4_rx_enb      : 1   ; /* [28]  */
        unsigned int    clk_gmac3_rx_enb      : 1   ; /* [27]  */
        unsigned int    clk_gmac2_rx_enb      : 1   ; /* [26]  */
        unsigned int    clk_gmac1_rx_enb      : 1   ; /* [25]  */
        unsigned int    clk_gmac0_rx_enb      : 1   ; /* [24]  */
        unsigned int    clk_gmac7_tx_enb      : 1   ; /* [23]  */
        unsigned int    clk_gmac6_tx_enb      : 1   ; /* [22]  */
        unsigned int    clk_gmac5_tx_enb      : 1   ; /* [21]  */
        unsigned int    clk_gmac4_tx_enb      : 1   ; /* [20]  */
        unsigned int    clk_gmac3_tx_enb      : 1   ; /* [19]  */
        unsigned int    clk_gmac2_tx_enb      : 1   ; /* [18]  */
        unsigned int    clk_gmac1_tx_enb      : 1   ; /* [17]  */
        unsigned int    clk_gmac0_tx_enb      : 1   ; /* [16]  */
        unsigned int    clk_gmac7_125m_enb    : 1   ; /* [15]  */
        unsigned int    clk_gmac6_125m_enb    : 1   ; /* [14]  */
        unsigned int    clk_gmac5_125m_enb    : 1   ; /* [13]  */
        unsigned int    clk_gmac4_125m_enb    : 1   ; /* [12]  */
        unsigned int    clk_gmac3_125m_enb    : 1   ; /* [11]  */
        unsigned int    clk_gmac2_125m_enb    : 1   ; /* [10]  */
        unsigned int    clk_gmac1_125m_enb    : 1   ; /* [9]  */
        unsigned int    clk_gmac0_125m_enb    : 1   ; /* [8]  */
        unsigned int    clk_gmac7_sys_enb     : 1   ; /* [7]  */
        unsigned int    clk_gmac6_sys_enb     : 1   ; /* [6]  */
        unsigned int    clk_gmac5_sys_enb     : 1   ; /* [5]  */
        unsigned int    clk_gmac4_sys_enb     : 1   ; /* [4]  */
        unsigned int    clk_gmac3_sys_enb     : 1   ; /* [3]  */
        unsigned int    clk_gmac2_sys_enb     : 1   ; /* [2]  */
        unsigned int    clk_gmac1_sys_enb     : 1   ; /* [1]  */
        unsigned int    clk_gmac0_sys_enb     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_CLK_EN_U;

/* Define the union U_SC_GE_CLK_DIS_U */
/* GEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x31C */
typedef union tagScGeClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gmac7_rx_dsb      : 1   ; /* [31]  */
        unsigned int    clk_gmac6_rx_dsb      : 1   ; /* [30]  */
        unsigned int    clk_gmac5_rx_dsb      : 1   ; /* [29]  */
        unsigned int    clk_gmac4_rx_dsb      : 1   ; /* [28]  */
        unsigned int    clk_gmac3_rx_dsb      : 1   ; /* [27]  */
        unsigned int    clk_gmac2_rx_dsb      : 1   ; /* [26]  */
        unsigned int    clk_gmac1_rx_dsb      : 1   ; /* [25]  */
        unsigned int    clk_gmac0_rx_dsb      : 1   ; /* [24]  */
        unsigned int    clk_gmac7_tx_dsb      : 1   ; /* [23]  */
        unsigned int    clk_gmac6_tx_dsb      : 1   ; /* [22]  */
        unsigned int    clk_gmac5_tx_dsb      : 1   ; /* [21]  */
        unsigned int    clk_gmac4_tx_dsb      : 1   ; /* [20]  */
        unsigned int    clk_gmac3_tx_dsb      : 1   ; /* [19]  */
        unsigned int    clk_gmac2_tx_dsb      : 1   ; /* [18]  */
        unsigned int    clk_gmac1_tx_dsb      : 1   ; /* [17]  */
        unsigned int    clk_gmac0_tx_dsb      : 1   ; /* [16]  */
        unsigned int    clk_gmac7_125m_dsb    : 1   ; /* [15]  */
        unsigned int    clk_gmac6_125m_dsb    : 1   ; /* [14]  */
        unsigned int    clk_gmac5_125m_dsb    : 1   ; /* [13]  */
        unsigned int    clk_gmac4_125m_dsb    : 1   ; /* [12]  */
        unsigned int    clk_gmac3_125m_dsb    : 1   ; /* [11]  */
        unsigned int    clk_gmac2_125m_dsb    : 1   ; /* [10]  */
        unsigned int    clk_gmac1_125m_dsb    : 1   ; /* [9]  */
        unsigned int    clk_gmac0_125m_dsb    : 1   ; /* [8]  */
        unsigned int    clk_gmac7_sys_dsb     : 1   ; /* [7]  */
        unsigned int    clk_gmac6_sys_dsb     : 1   ; /* [6]  */
        unsigned int    clk_gmac5_sys_dsb     : 1   ; /* [5]  */
        unsigned int    clk_gmac4_sys_dsb     : 1   ; /* [4]  */
        unsigned int    clk_gmac3_sys_dsb     : 1   ; /* [3]  */
        unsigned int    clk_gmac2_sys_dsb     : 1   ; /* [2]  */
        unsigned int    clk_gmac1_sys_dsb     : 1   ; /* [1]  */
        unsigned int    clk_gmac0_sys_dsb     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_CLK_DIS_U;

/* Define the union U_SC_PPE_CLK_EN_U */
/* PPEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x320 */
typedef union tagScPpeClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    clk_rcb_ppe7_enb      : 1   ; /* [7]  */
        unsigned int    clk_rcb_ppe6_enb      : 1   ; /* [6]  */
        unsigned int    clk_ppe5_enb          : 1   ; /* [5]  */
        unsigned int    clk_ppe4_enb          : 1   ; /* [4]  */
        unsigned int    clk_ppe3_enb          : 1   ; /* [3]  */
        unsigned int    clk_ppe2_enb          : 1   ; /* [2]  */
        unsigned int    clk_ppe1_enb          : 1   ; /* [1]  */
        unsigned int    clk_ppe0_enb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_CLK_EN_U;

/* Define the union U_SC_PPE_CLK_DIS_U */
/* PPEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x324 */
typedef union tagScPpeClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    clk_rcb_ppe7_dsb      : 1   ; /* [7]  */
        unsigned int    clk_rcb_ppe6_dsb      : 1   ; /* [6]  */
        unsigned int    clk_ppe5_dsb          : 1   ; /* [5]  */
        unsigned int    clk_ppe4_dsb          : 1   ; /* [4]  */
        unsigned int    clk_ppe3_dsb          : 1   ; /* [3]  */
        unsigned int    clk_ppe2_dsb          : 1   ; /* [2]  */
        unsigned int    clk_ppe1_dsb          : 1   ; /* [1]  */
        unsigned int    clk_ppe0_dsb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_CLK_DIS_U;

/* Define the union U_SC_ROCEE_CLK_EN_U */
/* ROCEEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x328 */
typedef union tagScRoceeClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
        unsigned int    clk_rocee5_enb        : 1   ; /* [5]  */
        unsigned int    clk_rocee4_enb        : 1   ; /* [4]  */
        unsigned int    clk_rocee3_enb        : 1   ; /* [3]  */
        unsigned int    clk_rocee2_enb        : 1   ; /* [2]  */
        unsigned int    clk_rocee1_enb        : 1   ; /* [1]  */
        unsigned int    clk_rocee0_enb        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_CLK_EN_U;

/* Define the union U_SC_ROCEE_CLK_DIS_U */
/* ROCEEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x32C */
typedef union tagScRoceeClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
        unsigned int    clk_rocee5_dsb        : 1   ; /* [5]  */
        unsigned int    clk_rocee4_dsb        : 1   ; /* [4]  */
        unsigned int    clk_rocee3_dsb        : 1   ; /* [3]  */
        unsigned int    clk_rocee2_dsb        : 1   ; /* [2]  */
        unsigned int    clk_rocee1_dsb        : 1   ; /* [1]  */
        unsigned int    clk_rocee0_dsb        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_CLK_DIS_U;

/* Define the union U_SC_CPU_CLK_EN_U */
/* CPUÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x330 */
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
/* CPUÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x334 */
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

/* Define the union U_SC_SAS_CLK_EN_U */
/* SASÊ±ÖÓÃÅ¿ØÐÅºÅ */
/* 0x338 */
typedef union tagScSasClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    clk_sas_ch7_tx_enb    : 1   ; /* [19]  */
        unsigned int    clk_sas_ch6_tx_enb    : 1   ; /* [18]  */
        unsigned int    clk_sas_ch5_tx_enb    : 1   ; /* [17]  */
        unsigned int    clk_sas_ch4_tx_enb    : 1   ; /* [16]  */
        unsigned int    clk_sas_ch3_tx_enb    : 1   ; /* [15]  */
        unsigned int    clk_sas_ch2_tx_enb    : 1   ; /* [14]  */
        unsigned int    clk_sas_ch1_tx_enb    : 1   ; /* [13]  */
        unsigned int    clk_sas_ch0_tx_enb    : 1   ; /* [12]  */
        unsigned int    clk_sas_ch7_rx_enb    : 1   ; /* [11]  */
        unsigned int    clk_sas_ch6_rx_enb    : 1   ; /* [10]  */
        unsigned int    clk_sas_ch5_rx_enb    : 1   ; /* [9]  */
        unsigned int    clk_sas_ch4_rx_enb    : 1   ; /* [8]  */
        unsigned int    clk_sas_ch3_rx_enb    : 1   ; /* [7]  */
        unsigned int    clk_sas_ch2_rx_enb    : 1   ; /* [6]  */
        unsigned int    clk_sas_ch1_rx_enb    : 1   ; /* [5]  */
        unsigned int    clk_sas_ch0_rx_enb    : 1   ; /* [4]  */
        unsigned int    clk_sas_oob_enb       : 1   ; /* [3]  */
        unsigned int    clk_sas_ahb_enb       : 1   ; /* [2]  */
        unsigned int    clk_sas_mem_enb       : 1   ; /* [1]  */
        unsigned int    clk_sas_enb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_CLK_EN_U;

/* Define the union U_SC_SAS_CLK_DIS_U */
/* SASÊ±ÖÓÃÅ¿ØÐÅºÅ */
/* 0x33C */
typedef union tagScSasClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    clk_sas_ch7_tx_dsb    : 1   ; /* [19]  */
        unsigned int    clk_sas_ch6_tx_dsb    : 1   ; /* [18]  */
        unsigned int    clk_sas_ch5_tx_dsb    : 1   ; /* [17]  */
        unsigned int    clk_sas_ch4_tx_dsb    : 1   ; /* [16]  */
        unsigned int    clk_sas_ch3_tx_dsb    : 1   ; /* [15]  */
        unsigned int    clk_sas_ch2_tx_dsb    : 1   ; /* [14]  */
        unsigned int    clk_sas_ch1_tx_dsb    : 1   ; /* [13]  */
        unsigned int    clk_sas_ch0_tx_dsb    : 1   ; /* [12]  */
        unsigned int    clk_sas_ch7_rx_dsb    : 1   ; /* [11]  */
        unsigned int    clk_sas_ch6_rx_dsb    : 1   ; /* [10]  */
        unsigned int    clk_sas_ch5_rx_dsb    : 1   ; /* [9]  */
        unsigned int    clk_sas_ch4_rx_dsb    : 1   ; /* [8]  */
        unsigned int    clk_sas_ch3_rx_dsb    : 1   ; /* [7]  */
        unsigned int    clk_sas_ch2_rx_dsb    : 1   ; /* [6]  */
        unsigned int    clk_sas_ch1_rx_dsb    : 1   ; /* [5]  */
        unsigned int    clk_sas_ch0_rx_dsb    : 1   ; /* [4]  */
        unsigned int    clk_sas_oob_dsb       : 1   ; /* [3]  */
        unsigned int    clk_sas_ahb_dsb       : 1   ; /* [2]  */
        unsigned int    clk_sas_mem_dsb       : 1   ; /* [1]  */
        unsigned int    clk_sas_dsb           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_CLK_DIS_U;

/* Define the union U_SC_ITS_CLK_EN_U */
/* itsÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x340 */
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
/* itsÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x344 */
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

/* Define the union U_SC_SRAM_CLK_EN_U */
/* SRAMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x348 */
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
/* 0x34C */
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

/* Define the union U_SC_RCB_PPE_COM_CLK_EN_U */
/* RCB PPE¹«¹²Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x350 */
typedef union tagScRcbPpeComClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rcb_ppe_com_enb   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_CLK_EN_U;

/* Define the union U_SC_RCB_PPE_COM_CLK_DIS_U */
/* RCB PPE¹«¹²Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x354 */
typedef union tagScRcbPpeComClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rcb_ppe_com_dsb   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_CLK_DIS_U;

/* Define the union U_SC_TIMER_CLK_EN_U */
/* TIMERÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x358 */
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
/* 0x35C */
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

/* Define the union U_SC_SLLC_CLK_EN_U */
/* SLLC1Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x360 */
typedef union tagScSllcClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_sllc_enb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_CLK_EN_U;

/* Define the union U_SC_SLLC_CLK_DIS_U */
/* SLLC1Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x364 */
typedef union tagScSllcClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_sllc_dsb          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_CLK_DIS_U;

/* Define the union U_SC_XBAR_RESET_REQ_U */
/* XBARÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA00 */
typedef union tagScXbarResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    xbar_srst_req         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XBAR_RESET_REQ_U;

/* Define the union U_SC_XBAR_RESET_DREQ_U */
/* XBARÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA04 */
typedef union tagScXbarResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    xbar_srst_dreq        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XBAR_RESET_DREQ_U;

/* Define the union U_SC_NT_RESET_REQ_U */
/* NTÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA08 */
typedef union tagScNtResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    nt_srst_req           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_RESET_REQ_U;

/* Define the union U_SC_NT_RESET_DREQ_U */
/* NTÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA0C */
typedef union tagScNtResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    nt_srst_dreq          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_RESET_DREQ_U;

/* Define the union U_SC_XGE_RESET_REQ_U */
/* XGEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA10 */
typedef union tagScXgeResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    xge5_fec_rx_srst_req  : 1   ; /* [30]  */
        unsigned int    xge4_fec_rx_srst_req  : 1   ; /* [29]  */
        unsigned int    xge3_fec_rx_srst_req  : 1   ; /* [28]  */
        unsigned int    xge2_fec_rx_srst_req  : 1   ; /* [27]  */
        unsigned int    xge1_fec_rx_srst_req  : 1   ; /* [26]  */
        unsigned int    xge0_fec_rx_srst_req  : 1   ; /* [25]  */
        unsigned int    xge5_fec_tx_srst_req  : 1   ; /* [24]  */
        unsigned int    xge4_fec_tx_srst_req  : 1   ; /* [23]  */
        unsigned int    xge3_fec_tx_srst_req  : 1   ; /* [22]  */
        unsigned int    xge2_fec_tx_srst_req  : 1   ; /* [21]  */
        unsigned int    xge1_fec_tx_srst_req  : 1   ; /* [20]  */
        unsigned int    xge0_fec_tx_srst_req  : 1   ; /* [19]  */
        unsigned int    xge5_core_rx_srst_req : 1   ; /* [18]  */
        unsigned int    xge4_core_rx_srst_req : 1   ; /* [17]  */
        unsigned int    xge3_core_rx_srst_req : 1   ; /* [16]  */
        unsigned int    xge2_core_rx_srst_req : 1   ; /* [15]  */
        unsigned int    xge1_core_rx_srst_req : 1   ; /* [14]  */
        unsigned int    xge0_core_rx_srst_req : 1   ; /* [13]  */
        unsigned int    xge5_core_tx_srst_req : 1   ; /* [12]  */
        unsigned int    xge4_core_tx_srst_req : 1   ; /* [11]  */
        unsigned int    xge3_core_tx_srst_req : 1   ; /* [10]  */
        unsigned int    xge2_core_tx_srst_req : 1   ; /* [9]  */
        unsigned int    xge1_core_tx_srst_req : 1   ; /* [8]  */
        unsigned int    xge0_core_tx_srst_req : 1   ; /* [7]  */
        unsigned int    xge5_cfg_mac_srst_req : 1   ; /* [6]  */
        unsigned int    xge4_cfg_mac_srst_req : 1   ; /* [5]  */
        unsigned int    xge3_cfg_mac_srst_req : 1   ; /* [4]  */
        unsigned int    xge2_cfg_mac_srst_req : 1   ; /* [3]  */
        unsigned int    xge1_cfg_mac_srst_req : 1   ; /* [2]  */
        unsigned int    xge0_cfg_mac_srst_req : 1   ; /* [1]  */
        unsigned int    xge_cfg_srst_req      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_RESET_REQ_U;

/* Define the union U_SC_XGE_RESET_DREQ_U */
/* XGEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA14 */
typedef union tagScXgeResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    xge5_fec_rx_srst_dreq : 1   ; /* [30]  */
        unsigned int    xge4_fec_rx_srst_dreq : 1   ; /* [29]  */
        unsigned int    xge3_fec_rx_srst_dreq : 1   ; /* [28]  */
        unsigned int    xge2_fec_rx_srst_dreq : 1   ; /* [27]  */
        unsigned int    xge1_fec_rx_srst_dreq : 1   ; /* [26]  */
        unsigned int    xge0_fec_rx_srst_dreq : 1   ; /* [25]  */
        unsigned int    xge5_fec_tx_srst_dreq : 1   ; /* [24]  */
        unsigned int    xge4_fec_tx_srst_dreq : 1   ; /* [23]  */
        unsigned int    xge3_fec_tx_srst_dreq : 1   ; /* [22]  */
        unsigned int    xge2_fec_tx_srst_dreq : 1   ; /* [21]  */
        unsigned int    xge1_fec_tx_srst_dreq : 1   ; /* [20]  */
        unsigned int    xge0_fec_tx_srst_dreq : 1   ; /* [19]  */
        unsigned int    xge5_core_rx_srst_dreq : 1   ; /* [18]  */
        unsigned int    xge4_core_rx_srst_dreq : 1   ; /* [17]  */
        unsigned int    xge3_core_rx_srst_dreq : 1   ; /* [16]  */
        unsigned int    xge2_core_rx_srst_dreq : 1   ; /* [15]  */
        unsigned int    xge1_core_rx_srst_dreq : 1   ; /* [14]  */
        unsigned int    xge0_core_rx_srst_dreq : 1   ; /* [13]  */
        unsigned int    xge5_core_tx_srst_dreq : 1   ; /* [12]  */
        unsigned int    xge4_core_tx_srst_dreq : 1   ; /* [11]  */
        unsigned int    xge3_core_tx_srst_dreq : 1   ; /* [10]  */
        unsigned int    xge2_core_tx_srst_dreq : 1   ; /* [9]  */
        unsigned int    xge1_core_tx_srst_dreq : 1   ; /* [8]  */
        unsigned int    xge0_core_tx_srst_dreq : 1   ; /* [7]  */
        unsigned int    xge5_cfg_mac_srst_dreq : 1   ; /* [6]  */
        unsigned int    xge4_cfg_mac_srst_dreq : 1   ; /* [5]  */
        unsigned int    xge3_cfg_mac_srst_dreq : 1   ; /* [4]  */
        unsigned int    xge2_cfg_mac_srst_dreq : 1   ; /* [3]  */
        unsigned int    xge1_cfg_mac_srst_dreq : 1   ; /* [2]  */
        unsigned int    xge0_cfg_mac_srst_dreq : 1   ; /* [1]  */
        unsigned int    xge_cfg_srst_dreq     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_RESET_DREQ_U;

/* Define the union U_SC_GE_RESET_REQ0_U */
/* GEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷0 */
/* 0xA18 */
typedef union tagScGeResetReq0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
        unsigned int    gmac5_eth_srst_req    : 1   ; /* [29]  */
        unsigned int    gmac4_eth_srst_req    : 1   ; /* [28]  */
        unsigned int    gmac3_eth_srst_req    : 1   ; /* [27]  */
        unsigned int    gmac2_eth_srst_req    : 1   ; /* [26]  */
        unsigned int    gmac1_eth_srst_req    : 1   ; /* [25]  */
        unsigned int    gmac0_eth_srst_req    : 1   ; /* [24]  */
        unsigned int    gmac5_cfg_srst_req    : 1   ; /* [23]  */
        unsigned int    gmac4_cfg_srst_req    : 1   ; /* [22]  */
        unsigned int    gmac3_cfg_srst_req    : 1   ; /* [21]  */
        unsigned int    gmac2_cfg_srst_req    : 1   ; /* [20]  */
        unsigned int    gmac1_cfg_srst_req    : 1   ; /* [19]  */
        unsigned int    gmac0_cfg_srst_req    : 1   ; /* [18]  */
        unsigned int    gmac5_rx_srst_req     : 1   ; /* [17]  */
        unsigned int    gmac4_rx_srst_req     : 1   ; /* [16]  */
        unsigned int    gmac3_rx_srst_req     : 1   ; /* [15]  */
        unsigned int    gmac2_rx_srst_req     : 1   ; /* [14]  */
        unsigned int    gmac1_rx_srst_req     : 1   ; /* [13]  */
        unsigned int    gmac0_rx_srst_req     : 1   ; /* [12]  */
        unsigned int    gmac5_tx_srst_req     : 1   ; /* [11]  */
        unsigned int    gmac4_tx_srst_req     : 1   ; /* [10]  */
        unsigned int    gmac3_tx_srst_req     : 1   ; /* [9]  */
        unsigned int    gmac2_tx_srst_req     : 1   ; /* [8]  */
        unsigned int    gmac1_tx_srst_req     : 1   ; /* [7]  */
        unsigned int    gmac0_tx_srst_req     : 1   ; /* [6]  */
        unsigned int    gmac5_sys_srst_req    : 1   ; /* [5]  */
        unsigned int    gmac4_sys_srst_req    : 1   ; /* [4]  */
        unsigned int    gmac3_sys_srst_req    : 1   ; /* [3]  */
        unsigned int    gmac2_sys_srst_req    : 1   ; /* [2]  */
        unsigned int    gmac1_sys_srst_req    : 1   ; /* [1]  */
        unsigned int    gmac0_sys_srst_req    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_REQ0_U;

/* Define the union U_SC_GE_RESET_DREQ0_U */
/* GEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷0 */
/* 0xA1C */
typedef union tagScGeResetDreq0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
        unsigned int    gmac5_eth_srst_dreq   : 1   ; /* [29]  */
        unsigned int    gmac4_eth_srst_dreq   : 1   ; /* [28]  */
        unsigned int    gmac3_eth_srst_dreq   : 1   ; /* [27]  */
        unsigned int    gmac2_eth_srst_dreq   : 1   ; /* [26]  */
        unsigned int    gmac1_eth_srst_dreq   : 1   ; /* [25]  */
        unsigned int    gmac0_eth_srst_dreq   : 1   ; /* [24]  */
        unsigned int    gmac5_cfg_srst_dreq   : 1   ; /* [23]  */
        unsigned int    gmac4_cfg_srst_dreq   : 1   ; /* [22]  */
        unsigned int    gmac3_cfg_srst_dreq   : 1   ; /* [21]  */
        unsigned int    gmac2_cfg_srst_dreq   : 1   ; /* [20]  */
        unsigned int    gmac1_cfg_srst_dreq   : 1   ; /* [19]  */
        unsigned int    gmac0_cfg_srst_dreq   : 1   ; /* [18]  */
        unsigned int    gmac5_rx_srst_dreq    : 1   ; /* [17]  */
        unsigned int    gmac4_rx_srst_dreq    : 1   ; /* [16]  */
        unsigned int    gmac3_rx_srst_dreq    : 1   ; /* [15]  */
        unsigned int    gmac2_rx_srst_dreq    : 1   ; /* [14]  */
        unsigned int    gmac1_rx_srst_dreq    : 1   ; /* [13]  */
        unsigned int    gmac0_rx_srst_dreq    : 1   ; /* [12]  */
        unsigned int    gmac5_tx_srst_dreq    : 1   ; /* [11]  */
        unsigned int    gmac4_tx_srst_dreq    : 1   ; /* [10]  */
        unsigned int    gmac3_tx_srst_dreq    : 1   ; /* [9]  */
        unsigned int    gmac2_tx_srst_dreq    : 1   ; /* [8]  */
        unsigned int    gmac1_tx_srst_dreq    : 1   ; /* [7]  */
        unsigned int    gmac0_tx_srst_dreq    : 1   ; /* [6]  */
        unsigned int    gmac5_sys_srst_dreq   : 1   ; /* [5]  */
        unsigned int    gmac4_sys_srst_dreq   : 1   ; /* [4]  */
        unsigned int    gmac3_sys_srst_dreq   : 1   ; /* [3]  */
        unsigned int    gmac2_sys_srst_dreq   : 1   ; /* [2]  */
        unsigned int    gmac1_sys_srst_dreq   : 1   ; /* [1]  */
        unsigned int    gmac0_sys_srst_dreq   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_DREQ0_U;

/* Define the union U_SC_GE_RESET_REQ1_U */
/* GEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷1 */
/* 0xA20 */
typedef union tagScGeResetReq1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
        unsigned int    gmac7_fifo_srst_req   : 1   ; /* [17]  */
        unsigned int    gmac6_fifo_srst_req   : 1   ; /* [16]  */
        unsigned int    gmac7_eth_srst_req    : 1   ; /* [15]  */
        unsigned int    gmac6_eth_srst_req    : 1   ; /* [14]  */
        unsigned int    gmac7_cfg_srst_req    : 1   ; /* [13]  */
        unsigned int    gmac6_cfg_srst_req    : 1   ; /* [12]  */
        unsigned int    gmac7_rx_srst_req     : 1   ; /* [11]  */
        unsigned int    gmac6_rx_srst_req     : 1   ; /* [10]  */
        unsigned int    gmac7_tx_srst_req     : 1   ; /* [9]  */
        unsigned int    gmac6_tx_srst_req     : 1   ; /* [8]  */
        unsigned int    gmac7_sys_srst_req    : 1   ; /* [7]  */
        unsigned int    gmac6_sys_srst_req    : 1   ; /* [6]  */
        unsigned int    gmac5_fifo_srst_req   : 1   ; /* [5]  */
        unsigned int    gmac4_fifo_srst_req   : 1   ; /* [4]  */
        unsigned int    gmac3_fifo_srst_req   : 1   ; /* [3]  */
        unsigned int    gmac2_fifo_srst_req   : 1   ; /* [2]  */
        unsigned int    gmac1_fifo_srst_req   : 1   ; /* [1]  */
        unsigned int    gmac0_fifo_srst_req   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_REQ1_U;

/* Define the union U_SC_GE_RESET_DREQ1_U */
/* GEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷1 */
/* 0xA24 */
typedef union tagScGeResetDreq1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
        unsigned int    gmac7_fifo_srst_dreq  : 1   ; /* [17]  */
        unsigned int    gmac6_fifo_srst_dreq  : 1   ; /* [16]  */
        unsigned int    gmac7_eth_srst_dreq   : 1   ; /* [15]  */
        unsigned int    gmac6_eth_srst_dreq   : 1   ; /* [14]  */
        unsigned int    gmac7_cfg_srst_dreq   : 1   ; /* [13]  */
        unsigned int    gmac6_cfg_srst_dreq   : 1   ; /* [12]  */
        unsigned int    gmac7_rx_srst_dreq    : 1   ; /* [11]  */
        unsigned int    gmac6_rx_srst_dreq    : 1   ; /* [10]  */
        unsigned int    gmac7_tx_srst_dreq    : 1   ; /* [9]  */
        unsigned int    gmac6_tx_srst_dreq    : 1   ; /* [8]  */
        unsigned int    gmac7_sys_srst_dreq   : 1   ; /* [7]  */
        unsigned int    gmac6_sys_srst_dreq   : 1   ; /* [6]  */
        unsigned int    gmac5_fifo_srst_dreq  : 1   ; /* [5]  */
        unsigned int    gmac4_fifo_srst_dreq  : 1   ; /* [4]  */
        unsigned int    gmac3_fifo_srst_dreq  : 1   ; /* [3]  */
        unsigned int    gmac2_fifo_srst_dreq  : 1   ; /* [2]  */
        unsigned int    gmac1_fifo_srst_dreq  : 1   ; /* [1]  */
        unsigned int    gmac0_fifo_srst_dreq  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_DREQ1_U;

/* Define the union U_SC_PPE_RESET_REQ_U */
/* PPEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA48 */
typedef union tagScPpeResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    rcb2_srst_req         : 1   ; /* [9]  */
        unsigned int    rcb1_srst_req         : 1   ; /* [8]  */
        unsigned int    ppe7_srst_req         : 1   ; /* [7]  */
        unsigned int    ppe6_srst_req         : 1   ; /* [6]  */
        unsigned int    ppe5_srst_req         : 1   ; /* [5]  */
        unsigned int    ppe4_srst_req         : 1   ; /* [4]  */
        unsigned int    ppe3_srst_req         : 1   ; /* [3]  */
        unsigned int    ppe2_srst_req         : 1   ; /* [2]  */
        unsigned int    ppe1_srst_req         : 1   ; /* [1]  */
        unsigned int    ppe0_srst_req         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_RESET_REQ_U;

/* Define the union U_SC_PPE_RESET_DREQ_U */
/* PPEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA4C */
typedef union tagScPpeResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    rcb2_srst_dreq        : 1   ; /* [9]  */
        unsigned int    rcb1_srst_dreq        : 1   ; /* [8]  */
        unsigned int    ppe7_srst_dreq        : 1   ; /* [7]  */
        unsigned int    ppe6_srst_dreq        : 1   ; /* [6]  */
        unsigned int    ppe5_srst_dreq        : 1   ; /* [5]  */
        unsigned int    ppe4_srst_dreq        : 1   ; /* [4]  */
        unsigned int    ppe3_srst_dreq        : 1   ; /* [3]  */
        unsigned int    ppe2_srst_dreq        : 1   ; /* [2]  */
        unsigned int    ppe1_srst_dreq        : 1   ; /* [1]  */
        unsigned int    ppe0_srst_dreq        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_RESET_DREQ_U;

/* Define the union U_SC_ROCEE_RESET_REQ_U */
/* ROCEEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA50 */
typedef union tagScRoceeResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rocee_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_RESET_REQ_U;

/* Define the union U_SC_ROCEE_RESET_DREQ_U */
/* ROCEEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA54 */
typedef union tagScRoceeResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rocee_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_RESET_DREQ_U;

/* Define the union U_SC_CPU_RESET_REQ_U */
/* CPUÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA58 */
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
/* CPUÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA5C */
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

/* Define the union U_SC_SAS_RESET_REQ_U */
/* SASÈí¸´Î»ÇëÇó */
/* 0xA60 */
typedef union tagScSasResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
        unsigned int    sas_ch7_tx_srst_req   : 1   ; /* [18]  */
        unsigned int    sas_ch6_tx_srst_req   : 1   ; /* [17]  */
        unsigned int    sas_ch5_tx_srst_req   : 1   ; /* [16]  */
        unsigned int    sas_ch4_tx_srst_req   : 1   ; /* [15]  */
        unsigned int    sas_ch3_tx_srst_req   : 1   ; /* [14]  */
        unsigned int    sas_ch2_tx_srst_req   : 1   ; /* [13]  */
        unsigned int    sas_ch1_tx_srst_req   : 1   ; /* [12]  */
        unsigned int    sas_ch0_tx_srst_req   : 1   ; /* [11]  */
        unsigned int    sas_ch7_rx_srst_req   : 1   ; /* [10]  */
        unsigned int    sas_ch6_rx_srst_req   : 1   ; /* [9]  */
        unsigned int    sas_ch5_rx_srst_req   : 1   ; /* [8]  */
        unsigned int    sas_ch4_rx_srst_req   : 1   ; /* [7]  */
        unsigned int    sas_ch3_rx_srst_req   : 1   ; /* [6]  */
        unsigned int    sas_ch2_rx_srst_req   : 1   ; /* [5]  */
        unsigned int    sas_ch1_rx_srst_req   : 1   ; /* [4]  */
        unsigned int    sas_ch0_rx_srst_req   : 1   ; /* [3]  */
        unsigned int    sas_ahb_srst_req      : 1   ; /* [2]  */
        unsigned int    sas_oob_srst_req      : 1   ; /* [1]  */
        unsigned int    sas_srst_req          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_RESET_REQ_U;

/* Define the union U_SC_SAS_RESET_DREQ_U */
/* SAS³·ÏúÈí¸´Î»ÇëÇó */
/* 0xA64 */
typedef union tagScSasResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
        unsigned int    sas_ch7_tx_srst_dreq  : 1   ; /* [18]  */
        unsigned int    sas_ch6_tx_srst_dreq  : 1   ; /* [17]  */
        unsigned int    sas_ch5_tx_srst_dreq  : 1   ; /* [16]  */
        unsigned int    sas_ch4_tx_srst_dreq  : 1   ; /* [15]  */
        unsigned int    sas_ch3_tx_srst_dreq  : 1   ; /* [14]  */
        unsigned int    sas_ch2_tx_srst_dreq  : 1   ; /* [13]  */
        unsigned int    sas_ch1_tx_srst_dreq  : 1   ; /* [12]  */
        unsigned int    sas_ch0_tx_srst_dreq  : 1   ; /* [11]  */
        unsigned int    sas_ch7_rx_srst_dreq  : 1   ; /* [10]  */
        unsigned int    sas_ch6_rx_srst_dreq  : 1   ; /* [9]  */
        unsigned int    sas_ch5_rx_srst_dreq  : 1   ; /* [8]  */
        unsigned int    sas_ch4_rx_srst_dreq  : 1   ; /* [7]  */
        unsigned int    sas_ch3_rx_srst_dreq  : 1   ; /* [6]  */
        unsigned int    sas_ch2_rx_srst_dreq  : 1   ; /* [5]  */
        unsigned int    sas_ch1_rx_srst_dreq  : 1   ; /* [4]  */
        unsigned int    sas_ch0_rx_srst_dreq  : 1   ; /* [3]  */
        unsigned int    sas_ahb_srst_dreq     : 1   ; /* [2]  */
        unsigned int    sas_oob_srst_dreq     : 1   ; /* [1]  */
        unsigned int    sas_srst_dreq         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_RESET_DREQ_U;

/* Define the union U_SC_SLLC_TSVRX_RESET_REQ_U */
/* SLLC TSVRXÍ¨µÀÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA68 */
typedef union tagScSllcTsvrxResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    sllc_tsvrx3_srst_req  : 1   ; /* [3]  */
        unsigned int    sllc_tsvrx2_srst_req  : 1   ; /* [2]  */
        unsigned int    sllc_tsvrx1_srst_req  : 1   ; /* [1]  */
        unsigned int    sllc_tsvrx0_srst_req  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_TSVRX_RESET_REQ_U;

/* Define the union U_SC_SLLC_TSVRX_RESET_DREQ_U */
/* SLLC TSVRXÍ¨µÀÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA6C */
typedef union tagScSllcTsvrxResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    sllc_tsvrx3_srst_dreq : 1   ; /* [3]  */
        unsigned int    sllc_tsvrx2_srst_dreq : 1   ; /* [2]  */
        unsigned int    sllc_tsvrx1_srst_dreq : 1   ; /* [1]  */
        unsigned int    sllc_tsvrx0_srst_dreq : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_TSVRX_RESET_DREQ_U;

/* Define the union U_SC_ITS_RESET_REQ_U */
/* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA70 */
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
/* 0xA74 */
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

/* Define the union U_SC_SRAM_RESET_REQ_U */
/* SRAMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA80 */
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
/* 0xA84 */
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

/* Define the union U_SC_RCB_PPE_COM_RESET_REQ_U */
/* RCB PPE¹«¹²Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA88 */
typedef union tagScRcbPpeComResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rcb_ppe_com_srst_req  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_RESET_REQ_U;

/* Define the union U_SC_RCB_PPE_COM_RESET_DREQ_U */
/* RCB PPE¹«¹²Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA8C */
typedef union tagScRcbPpeComResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rcb_ppe_com_srst_dreq : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_RESET_DREQ_U;

/* Define the union U_SC_TIMER_RESET_REQ_U */
/* TIMERÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA98 */
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
/* 0xA9C */
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

/* Define the union U_SC_SLLC_RESET_REQ_U */
/* SLLC1Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAA0 */
typedef union tagScSllcResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sllc_srst_req         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_RESET_REQ_U;

/* Define the union U_SC_SLLC_RESET_DREQ_U */
/* SLLC1Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAA4 */
typedef union tagScSllcResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sllc_srst_dreq        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_RESET_DREQ_U;

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

/* Define the union U_SC_M3_CTRL_U */
/* M3¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2040 */
typedef union tagScM3Ctrl
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

} SC_M3_CTRL_U;

/* Define the union U_SC_REMAP_SRAM_ADDR_CTRL_U */
/* M3 remapµ½sramµÄµØÖ·ÅäÖÃ */
/* 0x2050 */
typedef union tagScRemapSramAddrCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    remap_sram_addr       : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REMAP_SRAM_ADDR_CTRL_U;

/* Define the union U_SC_LIGHT_MODULE_DETECT_EN_U */
/* ¹âÄ£¿éSDÐÅºÅ¼ì²âÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x2060 */
typedef union tagScLightModuleDetectEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
        unsigned int    light_module_sd_5_en  : 1   ; /* [5]  */
        unsigned int    light_module_sd_4_en  : 1   ; /* [4]  */
        unsigned int    light_module_sd_3_en  : 1   ; /* [3]  */
        unsigned int    light_module_sd_2_en  : 1   ; /* [2]  */
        unsigned int    light_module_sd_1_en  : 1   ; /* [1]  */
        unsigned int    light_module_sd_0_en  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LIGHT_MODULE_DETECT_EN_U;

/* Define the union U_SC_TIMER_CLKEN_CTRL_U */
/* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2070 */
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

/* Define the union U_SC_VMID_CTRL0_U */
/* M3 VMIDÅäÖÃ */
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

/* Define the union U_SC_ROCEE_VMID_U */
/* ROCEE VMIDÅäÖÃ */
/* 0x20E4 */
typedef union tagScRoceeVmid
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    rocee_vmid            : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_VMID_U;

/* Define the union U_SC_ITS_M3_INT_MUX_SEL_U */
/* ¶ÔÖÐ¶ÏËÍµ½ITS»¹ÊÇM3£¬×öMUXÑ¡Ôñ */
/* 0x21F0 */
typedef union tagScItsM3IntMuxSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    int_m3_ring0_mux_sel  : 1   ; /* [4]  */
        unsigned int    int_m3_ring1_mux_sel  : 1   ; /* [3]  */
        unsigned int    int_tbl_da_mux_sel    : 1   ; /* [2]  */
        unsigned int    int_tbl_sa_mux_sel    : 1   ; /* [1]  */
        unsigned int    int_tbl_old_mux_sel   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_M3_INT_MUX_SEL_U;

/* Define the union U_SC_FTE_MUX_SEL_U */
/*  */
/* 0x2200 */
typedef union tagScFteMuxSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mux_sel_fte           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_FTE_MUX_SEL_U;

/* Define the union U_SC_TCAM_MBIST_EN_U */
/* DSAFÖÐTCAM MBISTÆô¶¯Ê¹ÄÜ¡£ */
/* 0x2300 */
typedef union tagScTcamMbistEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    tcam_mbist_en         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TCAM_MBIST_EN_U;

/* Define the union U_SC_HILINK2_LRSTB_MUX_CTRL_U */
/* HILINK2 lrstb[7:0]µÄMUXÑ¡Ôñ¿ØÖÆ */
/* 0x2340 */
typedef union tagScHilink2LrstbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink2_lrstb_mux_sel : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_LRSTB_MUX_CTRL_U;

/* Define the union U_SC_HILINK2_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2400 */
typedef union tagScHilink2MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    hilink2_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    hilink2_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink2_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink2_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink2_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink2_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink2_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink2_ss_refclk0_x2s : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK2_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2404 */
typedef union tagScHilink2MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    hilink2_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    hilink2_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink2_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink2_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink2_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink2_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink2_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink2_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink2_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink2_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK2_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2408 */
typedef union tagScHilink2MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink2_lifeclk2dig_sel : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK2_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x240C */
typedef union tagScHilink2MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink2_core_clk_selext : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK2_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2410 */
typedef union tagScHilink2MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink2_core_clk_sel  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK2_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2414 */
typedef union tagScHilink2MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink2_ctrl_bus_mode : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK2_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2418 */
typedef union tagScHilink2MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink2_macropwrdb    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK2_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x241C */
typedef union tagScHilink2MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink2_grstb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK2_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2420 */
typedef union tagScHilink2MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink2_bit_slip      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK2_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2424 */
typedef union tagScHilink2MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink2_lrstb         : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_LRSTB_U;

/* Define the union U_SC_HILINK3_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2500 */
typedef union tagScHilink3MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    hilink3_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    hilink3_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink3_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink3_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink3_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink3_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink3_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink3_ss_refclk0_x2s : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK3_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2504 */
typedef union tagScHilink3MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    hilink3_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    hilink3_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink3_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink3_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink3_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink3_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink3_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink3_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink3_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink3_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK3_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2508 */
typedef union tagScHilink3MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink3_lifeclk2dig_sel : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK3_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x250C */
typedef union tagScHilink3MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink3_core_clk_selext : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK3_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2510 */
typedef union tagScHilink3MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink3_core_clk_sel  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK3_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2514 */
typedef union tagScHilink3MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink3_ctrl_bus_mode : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK3_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2518 */
typedef union tagScHilink3MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink3_macropwrdb    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK3_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x251C */
typedef union tagScHilink3MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink3_grstb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK3_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2520 */
typedef union tagScHilink3MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink3_bit_slip      : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK3_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2524 */
typedef union tagScHilink3MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink3_lrstb         : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_LRSTB_U;

/* Define the union U_SC_HILINK4_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2600 */
typedef union tagScHilink4MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    hilink4_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    hilink4_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink4_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink4_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink4_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink4_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink4_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink4_ss_refclk0_x2s : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK4_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2604 */
typedef union tagScHilink4MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    hilink4_cs_refclk1_dirsel4 : 1   ; /* [9]  */
        unsigned int    hilink4_cs_refclk1_dirsel3 : 1   ; /* [8]  */
        unsigned int    hilink4_cs_refclk1_dirsel2 : 1   ; /* [7]  */
        unsigned int    hilink4_cs_refclk1_dirsel1 : 1   ; /* [6]  */
        unsigned int    hilink4_cs_refclk1_dirsel0 : 1   ; /* [5]  */
        unsigned int    hilink4_cs_refclk0_dirsel4 : 1   ; /* [4]  */
        unsigned int    hilink4_cs_refclk0_dirsel3 : 1   ; /* [3]  */
        unsigned int    hilink4_cs_refclk0_dirsel2 : 1   ; /* [2]  */
        unsigned int    hilink4_cs_refclk0_dirsel1 : 1   ; /* [1]  */
        unsigned int    hilink4_cs_refclk0_dirsel0 : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK4_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2608 */
typedef union tagScHilink4MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink4_lifeclk2dig_sel : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK4_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x260C */
typedef union tagScHilink4MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink4_core_clk_selext : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK4_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2610 */
typedef union tagScHilink4MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink4_core_clk_sel  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK4_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2614 */
typedef union tagScHilink4MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink4_ctrl_bus_mode : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK4_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2618 */
typedef union tagScHilink4MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink4_macropwrdb    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK4_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x261C */
typedef union tagScHilink4MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink4_grstb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK4_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2620 */
typedef union tagScHilink4MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink4_bit_slip      : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK4_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2624 */
typedef union tagScHilink4MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink4_lrstb         : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_LRSTB_U;

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

/* Define the union U_SC_SLLC1_MEM_CTRL_U */
/* sllc1 mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3010 */
typedef union tagScSllc1MemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    ctrl_rft_sllc1        : 10  ; /* [9..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC1_MEM_CTRL_U;

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

/* Define the union U_SC_ROCEE_MEM_CTRL0_U */
/* rocee mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3030 */
typedef union tagScRoceeMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_rocee        : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_MEM_CTRL0_U;

/* Define the union U_SC_ROCEE_MEM_CTRL1_U */
/* rocee mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3034 */
typedef union tagScRoceeMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    ctrl_rft_rocee        : 10  ; /* [9..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_MEM_CTRL1_U;

/* Define the union U_SC_XGE_MEM_CTRL0_U */
/* xge mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3040 */
typedef union tagScXgeMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_xge          : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_MEM_CTRL0_U;

/* Define the union U_SC_XGE_MEM_CTRL1_U */
/* xge mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3044 */
typedef union tagScXgeMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    ctrl_rft_xge          : 10  ; /* [9..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_MEM_CTRL1_U;

/* Define the union U_SC_SAS_MEM_CTRL_U */
/* sas mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3050 */
typedef union tagScSasMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_sas          : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_MEM_CTRL_U;

/* Define the union U_SC_GE_MEM_CTRL_U */
/* ge mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3060 */
typedef union tagScGeMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_ge           : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_MEM_CTRL_U;

/* Define the union U_SC_XBAR_MEM_CTRL0_U */
/* xbar mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3070 */
typedef union tagScXbarMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_xbar         : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XBAR_MEM_CTRL0_U;

/* Define the union U_SC_XBAR_MEM_CTRL1_U */
/* xbar mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3074 */
typedef union tagScXbarMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
        unsigned int    ctrl_rashde_xbar      : 7   ; /* [6..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XBAR_MEM_CTRL1_U;

/* Define the union U_SC_RCB_MEM_CTRL0_U */
/* rcb mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3080 */
typedef union tagScRcbMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_rcb          : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_MEM_CTRL0_U;

/* Define the union U_SC_RCB_MEM_CTRL1_U */
/* rcb mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3084 */
typedef union tagScRcbMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    ctrl_rft_rcb          : 10  ; /* [9..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_MEM_CTRL1_U;

/* Define the union U_SC_DSAF_CLK_ST_U */
/* DSAFÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5300 */
typedef union tagScDsafClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
        unsigned int    clk_xbar_com_st       : 1   ; /* [18]  */
        unsigned int    reserved_1            : 5   ; /* [17..13]  */
        unsigned int    clk_xbar_roce_st      : 1   ; /* [12]  */
        unsigned int    reserved_2            : 5   ; /* [11..7]  */
        unsigned int    clk_xbar_ppe_st       : 1   ; /* [6]  */
        unsigned int    clk_xbar_xge5_st      : 1   ; /* [5]  */
        unsigned int    clk_xbar_xge4_st      : 1   ; /* [4]  */
        unsigned int    clk_xbar_xge3_st      : 1   ; /* [3]  */
        unsigned int    clk_xbar_xge2_st      : 1   ; /* [2]  */
        unsigned int    clk_xbar_xge1_st      : 1   ; /* [1]  */
        unsigned int    clk_xbar_xge0_st      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DSAF_CLK_ST_U;

/* Define the union U_SC_NT_CLK_ST_U */
/* NTÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5304 */
typedef union tagScNtClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_nt_st             : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_CLK_ST_U;

/* Define the union U_SC_XGE_CLK_ST_U */
/* XGEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5308 */
typedef union tagScXgeClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
        unsigned int    clk_xge5_rx_st        : 1   ; /* [12]  */
        unsigned int    clk_xge4_rx_st        : 1   ; /* [11]  */
        unsigned int    clk_xge3_rx_st        : 1   ; /* [10]  */
        unsigned int    clk_xge2_rx_st        : 1   ; /* [9]  */
        unsigned int    clk_xge1_rx_st        : 1   ; /* [8]  */
        unsigned int    clk_xge0_rx_st        : 1   ; /* [7]  */
        unsigned int    clk_xge5_tx_st        : 1   ; /* [6]  */
        unsigned int    clk_xge4_tx_st        : 1   ; /* [5]  */
        unsigned int    clk_xge3_tx_st        : 1   ; /* [4]  */
        unsigned int    clk_xge2_tx_st        : 1   ; /* [3]  */
        unsigned int    clk_xge1_tx_st        : 1   ; /* [2]  */
        unsigned int    clk_xge0_tx_st        : 1   ; /* [1]  */
        unsigned int    clk_xge_cfg_st        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_CLK_ST_U;

/* Define the union U_SC_GE_CLK_ST_U */
/* GEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x530C */
typedef union tagScGeClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gmac7_rx_st       : 1   ; /* [31]  */
        unsigned int    clk_gmac6_rx_st       : 1   ; /* [30]  */
        unsigned int    clk_gmac5_rx_st       : 1   ; /* [29]  */
        unsigned int    clk_gmac4_rx_st       : 1   ; /* [28]  */
        unsigned int    clk_gmac3_rx_st       : 1   ; /* [27]  */
        unsigned int    clk_gmac2_rx_st       : 1   ; /* [26]  */
        unsigned int    clk_gmac1_rx_st       : 1   ; /* [25]  */
        unsigned int    clk_gmac0_rx_st       : 1   ; /* [24]  */
        unsigned int    clk_gmac7_tx_st       : 1   ; /* [23]  */
        unsigned int    clk_gmac6_tx_st       : 1   ; /* [22]  */
        unsigned int    clk_gmac5_tx_st       : 1   ; /* [21]  */
        unsigned int    clk_gmac4_tx_st       : 1   ; /* [20]  */
        unsigned int    clk_gmac3_tx_st       : 1   ; /* [19]  */
        unsigned int    clk_gmac2_tx_st       : 1   ; /* [18]  */
        unsigned int    clk_gmac1_tx_st       : 1   ; /* [17]  */
        unsigned int    clk_gmac0_tx_st       : 1   ; /* [16]  */
        unsigned int    clk_gmac7_125m_st     : 1   ; /* [15]  */
        unsigned int    clk_gmac6_125m_st     : 1   ; /* [14]  */
        unsigned int    clk_gmac5_125m_st     : 1   ; /* [13]  */
        unsigned int    clk_gmac4_125m_st     : 1   ; /* [12]  */
        unsigned int    clk_gmac3_125m_st     : 1   ; /* [11]  */
        unsigned int    clk_gmac2_125m_st     : 1   ; /* [10]  */
        unsigned int    clk_gmac1_125m_st     : 1   ; /* [9]  */
        unsigned int    clk_gmac0_125m_st     : 1   ; /* [8]  */
        unsigned int    clk_gmac7_sys_st      : 1   ; /* [7]  */
        unsigned int    clk_gmac6_sys_st      : 1   ; /* [6]  */
        unsigned int    clk_gmac5_sys_st      : 1   ; /* [5]  */
        unsigned int    clk_gmac4_sys_st      : 1   ; /* [4]  */
        unsigned int    clk_gmac3_sys_st      : 1   ; /* [3]  */
        unsigned int    clk_gmac2_sys_st      : 1   ; /* [2]  */
        unsigned int    clk_gmac1_sys_st      : 1   ; /* [1]  */
        unsigned int    clk_gmac0_sys_st      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_CLK_ST_U;

/* Define the union U_SC_PPE_CLK_ST_U */
/* PPEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5310 */
typedef union tagScPpeClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    clk_rcb_ppe7_st       : 1   ; /* [7]  */
        unsigned int    clk_rcb_ppe6_st       : 1   ; /* [6]  */
        unsigned int    clk_ppe5_st           : 1   ; /* [5]  */
        unsigned int    clk_ppe4_st           : 1   ; /* [4]  */
        unsigned int    clk_ppe3_st           : 1   ; /* [3]  */
        unsigned int    clk_ppe2_st           : 1   ; /* [2]  */
        unsigned int    clk_ppe1_st           : 1   ; /* [1]  */
        unsigned int    clk_ppe0_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_CLK_ST_U;

/* Define the union U_SC_ROCEE_CLK_ST_U */
/* ROCEEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5314 */
typedef union tagScRoceeClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
        unsigned int    clk_rocee5_st         : 1   ; /* [5]  */
        unsigned int    clk_rocee4_st         : 1   ; /* [4]  */
        unsigned int    clk_rocee3_st         : 1   ; /* [3]  */
        unsigned int    clk_rocee2_st         : 1   ; /* [2]  */
        unsigned int    clk_rocee1_st         : 1   ; /* [1]  */
        unsigned int    clk_rocee0_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_CLK_ST_U;

/* Define the union U_SC_CPU_CLK_ST_U */
/* CPUÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5318 */
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

/* Define the union U_SC_SAS_CLK_ST_U */
/* SASÊ±ÖÓÃÅ¿ØÐÅºÅ×´Ì¬ */
/* 0x531C */
typedef union tagScSasClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    clk_sas_ch7_tx_st     : 1   ; /* [19]  */
        unsigned int    clk_sas_ch6_tx_st     : 1   ; /* [18]  */
        unsigned int    clk_sas_ch5_tx_st     : 1   ; /* [17]  */
        unsigned int    clk_sas_ch4_tx_st     : 1   ; /* [16]  */
        unsigned int    clk_sas_ch3_tx_st     : 1   ; /* [15]  */
        unsigned int    clk_sas_ch2_tx_st     : 1   ; /* [14]  */
        unsigned int    clk_sas_ch1_tx_st     : 1   ; /* [13]  */
        unsigned int    clk_sas_ch0_tx_st     : 1   ; /* [12]  */
        unsigned int    clk_sas_ch7_rx_st     : 1   ; /* [11]  */
        unsigned int    clk_sas_ch6_rx_st     : 1   ; /* [10]  */
        unsigned int    clk_sas_ch5_rx_st     : 1   ; /* [9]  */
        unsigned int    clk_sas_ch4_rx_st     : 1   ; /* [8]  */
        unsigned int    clk_sas_ch3_rx_st     : 1   ; /* [7]  */
        unsigned int    clk_sas_ch2_rx_st     : 1   ; /* [6]  */
        unsigned int    clk_sas_ch1_rx_st     : 1   ; /* [5]  */
        unsigned int    clk_sas_ch0_rx_st     : 1   ; /* [4]  */
        unsigned int    clk_sas_oob_st        : 1   ; /* [3]  */
        unsigned int    clk_sas_ahb_st        : 1   ; /* [2]  */
        unsigned int    clk_sas_mem_st        : 1   ; /* [1]  */
        unsigned int    clk_sas_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_CLK_ST_U;

/* Define the union U_SC_ITS_CLK_ST_U */
/* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5320 */
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

/* Define the union U_SC_SRAM_CLK_ST_U */
/* SRAMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5324 */
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

/* Define the union U_SC_RCB_PPE_COM_CLK_ST_U */
/* RCB PPE¹«¹²Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5328 */
typedef union tagScRcbPpeComClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_rcb_ppe_com_st    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_CLK_ST_U;

/* Define the union U_SC_TIMER_CLK_ST_U */
/* TIMERÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x532C */
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

/* Define the union U_SC_SLLC_CLK_ST_U */
/* SLLC1Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5330 */
typedef union tagScSllcClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    clk_sllc_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_CLK_ST_U;

/* Define the union U_SC_XBAR_RESET_ST_U */
/* DSAFÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A00 */
typedef union tagScXbarResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    xbar_srst_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XBAR_RESET_ST_U;

/* Define the union U_SC_NT_RESET_ST_U */
/* NTÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A04 */
typedef union tagScNtResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    nt_srst_st            : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_RESET_ST_U;

/* Define the union U_SC_XGE_RESET_ST_U */
/* XGEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A08 */
typedef union tagScXgeResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 1   ; /* [31]  */
        unsigned int    xge5_fec_rx_srst_st   : 1   ; /* [30]  */
        unsigned int    xge4_fec_rx_srst_st   : 1   ; /* [29]  */
        unsigned int    xge3_fec_rx_srst_st   : 1   ; /* [28]  */
        unsigned int    xge2_fec_rx_srst_st   : 1   ; /* [27]  */
        unsigned int    xge1_fec_rx_srst_st   : 1   ; /* [26]  */
        unsigned int    xge0_fec_rx_srst_st   : 1   ; /* [25]  */
        unsigned int    xge5_fec_tx_srst_st   : 1   ; /* [24]  */
        unsigned int    xge4_fec_tx_srst_st   : 1   ; /* [23]  */
        unsigned int    xge3_fec_tx_srst_st   : 1   ; /* [22]  */
        unsigned int    xge2_fec_tx_srst_st   : 1   ; /* [21]  */
        unsigned int    xge1_fec_tx_srst_st   : 1   ; /* [20]  */
        unsigned int    xge0_fec_tx_srst_st   : 1   ; /* [19]  */
        unsigned int    xge5_core_rx_srst_st  : 1   ; /* [18]  */
        unsigned int    xge4_core_rx_srst_st  : 1   ; /* [17]  */
        unsigned int    xge3_core_rx_srst_st  : 1   ; /* [16]  */
        unsigned int    xge2_core_rx_srst_st  : 1   ; /* [15]  */
        unsigned int    xge1_core_rx_srst_st  : 1   ; /* [14]  */
        unsigned int    xge0_core_rx_srst_st  : 1   ; /* [13]  */
        unsigned int    xge5_core_tx_srst_st  : 1   ; /* [12]  */
        unsigned int    xge4_core_tx_srst_st  : 1   ; /* [11]  */
        unsigned int    xge3_core_tx_srst_st  : 1   ; /* [10]  */
        unsigned int    xge2_core_tx_srst_st  : 1   ; /* [9]  */
        unsigned int    xge1_core_tx_srst_st  : 1   ; /* [8]  */
        unsigned int    xge0_core_tx_srst_st  : 1   ; /* [7]  */
        unsigned int    xge5_cfg_mac_srst_st  : 1   ; /* [6]  */
        unsigned int    xge4_cfg_mac_srst_st  : 1   ; /* [5]  */
        unsigned int    xge3_cfg_mac_srst_st  : 1   ; /* [4]  */
        unsigned int    xge2_cfg_mac_srst_st  : 1   ; /* [3]  */
        unsigned int    xge1_cfg_mac_srst_st  : 1   ; /* [2]  */
        unsigned int    xge0_cfg_mac_srst_st  : 1   ; /* [1]  */
        unsigned int    xge_cfg_srst_st       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_RESET_ST_U;

/* Define the union U_SC_GE_RESET_ST0_U */
/* GEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷0 */
/* 0x5A0C */
typedef union tagScGeResetSt0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
        unsigned int    gmac5_eth_srst_st     : 1   ; /* [29]  */
        unsigned int    gmac4_eth_srst_st     : 1   ; /* [28]  */
        unsigned int    gmac3_eth_srst_st     : 1   ; /* [27]  */
        unsigned int    gmac2_eth_srst_st     : 1   ; /* [26]  */
        unsigned int    gmac1_eth_srst_st     : 1   ; /* [25]  */
        unsigned int    gmac0_eth_srst_st     : 1   ; /* [24]  */
        unsigned int    gmac5_cfg_srst_st     : 1   ; /* [23]  */
        unsigned int    gmac4_cfg_srst_st     : 1   ; /* [22]  */
        unsigned int    gmac3_cfg_srst_st     : 1   ; /* [21]  */
        unsigned int    gmac2_cfg_srst_st     : 1   ; /* [20]  */
        unsigned int    gmac1_cfg_srst_st     : 1   ; /* [19]  */
        unsigned int    gmac0_cfg_srst_st     : 1   ; /* [18]  */
        unsigned int    gmac5_rx_srst_st      : 1   ; /* [17]  */
        unsigned int    gmac4_rx_srst_st      : 1   ; /* [16]  */
        unsigned int    gmac3_rx_srst_st      : 1   ; /* [15]  */
        unsigned int    gmac2_rx_srst_st      : 1   ; /* [14]  */
        unsigned int    gmac1_rx_srst_st      : 1   ; /* [13]  */
        unsigned int    gmac0_rx_srst_st      : 1   ; /* [12]  */
        unsigned int    gmac5_tx_srst_st      : 1   ; /* [11]  */
        unsigned int    gmac4_tx_srst_st      : 1   ; /* [10]  */
        unsigned int    gmac3_tx_srst_st      : 1   ; /* [9]  */
        unsigned int    gmac2_tx_srst_st      : 1   ; /* [8]  */
        unsigned int    gmac1_tx_srst_st      : 1   ; /* [7]  */
        unsigned int    gmac0_tx_srst_st      : 1   ; /* [6]  */
        unsigned int    gmac5_sys_srst_st     : 1   ; /* [5]  */
        unsigned int    gmac4_sys_srst_st     : 1   ; /* [4]  */
        unsigned int    gmac3_sys_srst_st     : 1   ; /* [3]  */
        unsigned int    gmac2_sys_srst_st     : 1   ; /* [2]  */
        unsigned int    gmac1_sys_srst_st     : 1   ; /* [1]  */
        unsigned int    gmac0_sys_srst_st     : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_ST0_U;

/* Define the union U_SC_GE_RESET_ST1_U */
/* GEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷1 */
/* 0x5A10 */
typedef union tagScGeResetSt1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
        unsigned int    gmac7_fifo_srst_st    : 1   ; /* [17]  */
        unsigned int    gmac6_fifo_srst_st    : 1   ; /* [16]  */
        unsigned int    gmac7_eth_srst_st     : 1   ; /* [15]  */
        unsigned int    gmac6_eth_srst_st     : 1   ; /* [14]  */
        unsigned int    gmac7_cfg_srst_st     : 1   ; /* [13]  */
        unsigned int    gmac6_cfg_srst_st     : 1   ; /* [12]  */
        unsigned int    gmac7_rx_srst_st      : 1   ; /* [11]  */
        unsigned int    gmac6_rx_srst_st      : 1   ; /* [10]  */
        unsigned int    gmac7_tx_srst_st      : 1   ; /* [9]  */
        unsigned int    gmac6_tx_srst_st      : 1   ; /* [8]  */
        unsigned int    gmac7_sys_srst_st     : 1   ; /* [7]  */
        unsigned int    gmac6_sys_srst_st     : 1   ; /* [6]  */
        unsigned int    gmac5_fifo_srst_st    : 1   ; /* [5]  */
        unsigned int    gmac4_fifo_srst_st    : 1   ; /* [4]  */
        unsigned int    gmac3_fifo_srst_st    : 1   ; /* [3]  */
        unsigned int    gmac2_fifo_srst_st    : 1   ; /* [2]  */
        unsigned int    gmac1_fifo_srst_st    : 1   ; /* [1]  */
        unsigned int    gmac0_fifo_srst_st    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_ST1_U;

/* Define the union U_SC_PPE_RESET_ST_U */
/* PPEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A24 */
typedef union tagScPpeResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    rcb2_srst_st          : 1   ; /* [9]  */
        unsigned int    rcb1_srst_st          : 1   ; /* [8]  */
        unsigned int    ppe7_srst_st          : 1   ; /* [7]  */
        unsigned int    ppe6_srst_st          : 1   ; /* [6]  */
        unsigned int    ppe5_srst_st          : 1   ; /* [5]  */
        unsigned int    ppe4_srst_st          : 1   ; /* [4]  */
        unsigned int    ppe3_srst_st          : 1   ; /* [3]  */
        unsigned int    ppe2_srst_st          : 1   ; /* [2]  */
        unsigned int    ppe1_srst_st          : 1   ; /* [1]  */
        unsigned int    ppe0_srst_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_RESET_ST_U;

/* Define the union U_SC_ROCEE_RESET_ST_U */
/* ROCEEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A28 */
typedef union tagScRoceeResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rocee_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_RESET_ST_U;

/* Define the union U_SC_CPU_RESET_ST_U */
/* CPUÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A2C */
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

/* Define the union U_SC_SAS_RESET_ST_U */
/* SASÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A30 */
typedef union tagScSasResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
        unsigned int    sas_ch7_tx_srst_st    : 1   ; /* [18]  */
        unsigned int    sas_ch6_tx_srst_st    : 1   ; /* [17]  */
        unsigned int    sas_ch5_tx_srst_st    : 1   ; /* [16]  */
        unsigned int    sas_ch4_tx_srst_st    : 1   ; /* [15]  */
        unsigned int    sas_ch3_tx_srst_st    : 1   ; /* [14]  */
        unsigned int    sas_ch2_tx_srst_st    : 1   ; /* [13]  */
        unsigned int    sas_ch1_tx_srst_st    : 1   ; /* [12]  */
        unsigned int    sas_ch0_tx_srst_st    : 1   ; /* [11]  */
        unsigned int    sas_ch7_rx_srst_st    : 1   ; /* [10]  */
        unsigned int    sas_ch6_rx_srst_st    : 1   ; /* [9]  */
        unsigned int    sas_ch5_rx_srst_st    : 1   ; /* [8]  */
        unsigned int    sas_ch4_rx_srst_st    : 1   ; /* [7]  */
        unsigned int    sas_ch3_rx_srst_st    : 1   ; /* [6]  */
        unsigned int    sas_ch2_rx_srst_st    : 1   ; /* [5]  */
        unsigned int    sas_ch1_rx_srst_st    : 1   ; /* [4]  */
        unsigned int    sas_ch0_rx_srst_st    : 1   ; /* [3]  */
        unsigned int    sas_ahb_srst_st       : 1   ; /* [2]  */
        unsigned int    sas_oob_srst_st       : 1   ; /* [1]  */
        unsigned int    sas_srst_st           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_RESET_ST_U;

/* Define the union U_SC_SLLC_TSVRX_RESET_ST_U */
/* ÎªSLLC TSVRXÍ¨µÀÈí¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A34 */
typedef union tagScSllcTsvrxResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    sllc_tsvrx3_srst_st   : 1   ; /* [3]  */
        unsigned int    sllc_tsvrx2_srst_st   : 1   ; /* [2]  */
        unsigned int    sllc_tsvrx1_srst_st   : 1   ; /* [1]  */
        unsigned int    sllc_tsvrx0_srst_st   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_TSVRX_RESET_ST_U;

/* Define the union U_SC_ITS_RESET_ST_U */
/* ITSÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A38 */
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

/* Define the union U_SC_SRAM_RESET_ST_U */
/* SRAMÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A40 */
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

/* Define the union U_SC_RCB_PPE_COM_RESET_ST_U */
/* RCB PPE¹«¹²Èí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A44 */
typedef union tagScRcbPpeComResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    rcb_ppe_com_srst_st   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_RESET_ST_U;

/* Define the union U_SC_TIMER_RESET_ST_U */
/* TIMER¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A4C */
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

/* Define the union U_SC_SLLC_RESET_ST_U */
/* SLLC1¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A50 */
typedef union tagScSllcResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    sllc_srst_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_RESET_ST_U;

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

/* Define the union U_SC_TCAM_MBIST_ST_U */
/* DSAFÖÐTCAM MBIST²âÊÔ½á¹û×´Ì¬ */
/* 0x6300 */
typedef union tagScTcamMbistSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    tcam_mbist_err_mem    : 1   ; /* [2]  */
        unsigned int    tcam_mbist_err_cmp    : 1   ; /* [1]  */
        unsigned int    tcam_mbist_done       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TCAM_MBIST_ST_U;

/* Define the union U_SC_GE_ST_U */
/* DSAFÖÐGEµÄ¹¤×÷×´Ì¬Ö¸Ê¾ */
/* 0x6380 */
typedef union tagScGeSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    gmac_led_activity_7   : 1   ; /* [7]  */
        unsigned int    gmac_led_activity_6   : 1   ; /* [6]  */
        unsigned int    gmac_led_activity_5   : 1   ; /* [5]  */
        unsigned int    gmac_led_activity_4   : 1   ; /* [4]  */
        unsigned int    gmac_led_activity_3   : 1   ; /* [3]  */
        unsigned int    gmac_led_activity_2   : 1   ; /* [2]  */
        unsigned int    gmac_led_activity_1   : 1   ; /* [1]  */
        unsigned int    gmac_led_activity_0   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_ST_U;

/* Define the union U_SC_XGE_ST_U */
/* DSAFÖÐXGEµÄ¹¤×÷×´Ì¬Ö¸Ê¾ */
/* 0x6390 */
typedef union tagScXgeSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
        unsigned int    xge_app_rx_link_up_5  : 1   ; /* [5]  */
        unsigned int    xge_app_rx_link_up_4  : 1   ; /* [4]  */
        unsigned int    xge_app_rx_link_up_3  : 1   ; /* [3]  */
        unsigned int    xge_app_rx_link_up_2  : 1   ; /* [2]  */
        unsigned int    xge_app_rx_link_up_1  : 1   ; /* [1]  */
        unsigned int    xge_app_rx_link_up_0  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_ST_U;

/* Define the union U_SC_HILINK2_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6400 */
typedef union tagScHilink2MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink2_plloutoflock  : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK2_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6404 */
typedef union tagScHilink2MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink2_prbs_err      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK2_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6408 */
typedef union tagScHilink2MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink2_los           : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_LOS_U;

/* Define the union U_SC_HILINK3_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6500 */
typedef union tagScHilink3MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink3_plloutoflock  : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK3_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6504 */
typedef union tagScHilink3MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink3_prbs_err      : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK3_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6508 */
typedef union tagScHilink3MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink3_los           : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_LOS_U;

/* Define the union U_SC_HILINK4_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6600 */
typedef union tagScHilink4MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink4_plloutoflock  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK4_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6604 */
typedef union tagScHilink4MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink4_prbs_err      : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK4_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6608 */
typedef union tagScHilink4MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink4_los           : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_LOS_U;

#else

/* Define the union U_SC_NT_SRAM_CLK_SEL_U */
/* Node TableÄ£¿éTCAM±ímemoryÊ±ÖÓ¿ØÖÆ¼Ä´æÆ÷ */
/* 0x100 */
typedef union tagScNtSramClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nt_sram_clk_sel       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_SRAM_CLK_SEL_U;

/* Define the union U_SC_HILINK3_CRG_CTRL0_U */
/* ÅäÖÃHILINK CRG */
/* 0x180 */
typedef union tagScHilink3CrgCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_mode_cfg      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_CRG_CTRL0_U;

/* Define the union U_SC_HILINK3_CRG_CTRL1_U */
/* ÅäÖÃHILINK CRG */
/* 0x184 */
typedef union tagScHilink3CrgCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_ge_tx_div_sel : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_CRG_CTRL1_U;

/* Define the union U_SC_HILINK3_CRG_CTRL2_U */
/* ÅäÖÃHILINK CRG */
/* 0x188 */
typedef union tagScHilink3CrgCtrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ge2_mclk_clk_sel      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_CRG_CTRL2_U;

/* Define the union U_SC_HILINK3_CRG_CTRL3_U */
/* ÅäÖÃHILINK CRG */
/* 0x18C */
typedef union tagScHilink3CrgCtrl3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ge3_mclk_clk_sel      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_CRG_CTRL3_U;

/* Define the union U_SC_HILINK4_CRG_CTRL0_U */
/* ÅäÖÃHILINK CRG */
/* 0x190 */
typedef union tagScHilink4CrgCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_mode_cfg      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_CRG_CTRL0_U;

/* Define the union U_SC_HILINK4_CRG_CTRL1_U */
/* ÅäÖÃHILINK CRG */
/* 0x194 */
typedef union tagScHilink4CrgCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_ge_tx_div_sel : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_CRG_CTRL1_U;

/* Define the union U_SC_DSAF_CLK_EN_U */
/* DSAFÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x300 */
typedef union tagScDsafClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_xbar_xge0_enb     : 1   ; /* [0]  */
        unsigned int    clk_xbar_xge1_enb     : 1   ; /* [1]  */
        unsigned int    clk_xbar_xge2_enb     : 1   ; /* [2]  */
        unsigned int    clk_xbar_xge3_enb     : 1   ; /* [3]  */
        unsigned int    clk_xbar_xge4_enb     : 1   ; /* [4]  */
        unsigned int    clk_xbar_xge5_enb     : 1   ; /* [5]  */
        unsigned int    clk_xbar_ppe_enb      : 1   ; /* [6]  */
        unsigned int    reserved_2            : 5   ; /* [11..7]  */
        unsigned int    clk_xbar_roce_enb     : 1   ; /* [12]  */
        unsigned int    reserved_1            : 5   ; /* [17..13]  */
        unsigned int    clk_xbar_com_enb      : 1   ; /* [18]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DSAF_CLK_EN_U;

/* Define the union U_SC_DSAF_CLK_DIS_U */
/* DSAFÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x304 */
typedef union tagScDsafClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_xbar_xge0_dsb     : 1   ; /* [0]  */
        unsigned int    clk_xbar_xge1_dsb     : 1   ; /* [1]  */
        unsigned int    clk_xbar_xge2_dsb     : 1   ; /* [2]  */
        unsigned int    clk_xbar_xge3_dsb     : 1   ; /* [3]  */
        unsigned int    clk_xbar_xge4_dsb     : 1   ; /* [4]  */
        unsigned int    clk_xbar_xge5_dsb     : 1   ; /* [5]  */
        unsigned int    clk_xbar_ppe_dsb      : 1   ; /* [6]  */
        unsigned int    reserved_2            : 5   ; /* [11..7]  */
        unsigned int    clk_xbar_roce_dsb     : 1   ; /* [12]  */
        unsigned int    reserved_1            : 5   ; /* [17..13]  */
        unsigned int    clk_xbar_com_dsb      : 1   ; /* [18]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DSAF_CLK_DIS_U;

/* Define the union U_SC_NT_CLK_EN_U */
/* NTÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x308 */
typedef union tagScNtClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_nt_enb            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_CLK_EN_U;

/* Define the union U_SC_NT_CLK_DIS_U */
/* NTÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x30C */
typedef union tagScNtClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_nt_dsb            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_CLK_DIS_U;

/* Define the union U_SC_XGE_CLK_EN_U */
/* XGEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x310 */
typedef union tagScXgeClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_xge_cfg_enb       : 1   ; /* [0]  */
        unsigned int    clk_xge0_rx_enb       : 1   ; /* [1]  */
        unsigned int    clk_xge1_rx_enb       : 1   ; /* [2]  */
        unsigned int    clk_xge2_rx_enb       : 1   ; /* [3]  */
        unsigned int    clk_xge3_rx_enb       : 1   ; /* [4]  */
        unsigned int    clk_xge4_rx_enb       : 1   ; /* [5]  */
        unsigned int    clk_xge5_rx_enb       : 1   ; /* [6]  */
        unsigned int    clk_xge0_tx_enb       : 1   ; /* [7]  */
        unsigned int    clk_xge1_tx_enb       : 1   ; /* [8]  */
        unsigned int    clk_xge2_tx_enb       : 1   ; /* [9]  */
        unsigned int    clk_xge3_tx_enb       : 1   ; /* [10]  */
        unsigned int    clk_xge4_tx_enb       : 1   ; /* [11]  */
        unsigned int    clk_xge5_tx_enb       : 1   ; /* [12]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_CLK_EN_U;

/* Define the union U_SC_XGE_CLK_DIS_U */
/* XGEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x314 */
typedef union tagScXgeClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_xge_cfg_dsb       : 1   ; /* [0]  */
        unsigned int    clk_xge0_rx_dsb       : 1   ; /* [1]  */
        unsigned int    clk_xge1_rx_dsb       : 1   ; /* [2]  */
        unsigned int    clk_xge2_rx_dsb       : 1   ; /* [3]  */
        unsigned int    clk_xge3_rx_dsb       : 1   ; /* [4]  */
        unsigned int    clk_xge4_rx_dsb       : 1   ; /* [5]  */
        unsigned int    clk_xge5_rx_dsb       : 1   ; /* [6]  */
        unsigned int    clk_xge0_tx_dsb       : 1   ; /* [7]  */
        unsigned int    clk_xge1_tx_dsb       : 1   ; /* [8]  */
        unsigned int    clk_xge2_tx_dsb       : 1   ; /* [9]  */
        unsigned int    clk_xge3_tx_dsb       : 1   ; /* [10]  */
        unsigned int    clk_xge4_tx_dsb       : 1   ; /* [11]  */
        unsigned int    clk_xge5_tx_dsb       : 1   ; /* [12]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_CLK_DIS_U;

/* Define the union U_SC_GE_CLK_EN_U */
/* GEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x318 */
typedef union tagScGeClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gmac0_sys_enb     : 1   ; /* [0]  */
        unsigned int    clk_gmac1_sys_enb     : 1   ; /* [1]  */
        unsigned int    clk_gmac2_sys_enb     : 1   ; /* [2]  */
        unsigned int    clk_gmac3_sys_enb     : 1   ; /* [3]  */
        unsigned int    clk_gmac4_sys_enb     : 1   ; /* [4]  */
        unsigned int    clk_gmac5_sys_enb     : 1   ; /* [5]  */
        unsigned int    clk_gmac6_sys_enb     : 1   ; /* [6]  */
        unsigned int    clk_gmac7_sys_enb     : 1   ; /* [7]  */
        unsigned int    clk_gmac0_125m_enb    : 1   ; /* [8]  */
        unsigned int    clk_gmac1_125m_enb    : 1   ; /* [9]  */
        unsigned int    clk_gmac2_125m_enb    : 1   ; /* [10]  */
        unsigned int    clk_gmac3_125m_enb    : 1   ; /* [11]  */
        unsigned int    clk_gmac4_125m_enb    : 1   ; /* [12]  */
        unsigned int    clk_gmac5_125m_enb    : 1   ; /* [13]  */
        unsigned int    clk_gmac6_125m_enb    : 1   ; /* [14]  */
        unsigned int    clk_gmac7_125m_enb    : 1   ; /* [15]  */
        unsigned int    clk_gmac0_tx_enb      : 1   ; /* [16]  */
        unsigned int    clk_gmac1_tx_enb      : 1   ; /* [17]  */
        unsigned int    clk_gmac2_tx_enb      : 1   ; /* [18]  */
        unsigned int    clk_gmac3_tx_enb      : 1   ; /* [19]  */
        unsigned int    clk_gmac4_tx_enb      : 1   ; /* [20]  */
        unsigned int    clk_gmac5_tx_enb      : 1   ; /* [21]  */
        unsigned int    clk_gmac6_tx_enb      : 1   ; /* [22]  */
        unsigned int    clk_gmac7_tx_enb      : 1   ; /* [23]  */
        unsigned int    clk_gmac0_rx_enb      : 1   ; /* [24]  */
        unsigned int    clk_gmac1_rx_enb      : 1   ; /* [25]  */
        unsigned int    clk_gmac2_rx_enb      : 1   ; /* [26]  */
        unsigned int    clk_gmac3_rx_enb      : 1   ; /* [27]  */
        unsigned int    clk_gmac4_rx_enb      : 1   ; /* [28]  */
        unsigned int    clk_gmac5_rx_enb      : 1   ; /* [29]  */
        unsigned int    clk_gmac6_rx_enb      : 1   ; /* [30]  */
        unsigned int    clk_gmac7_rx_enb      : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_CLK_EN_U;

/* Define the union U_SC_GE_CLK_DIS_U */
/* GEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x31C */
typedef union tagScGeClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gmac0_sys_dsb     : 1   ; /* [0]  */
        unsigned int    clk_gmac1_sys_dsb     : 1   ; /* [1]  */
        unsigned int    clk_gmac2_sys_dsb     : 1   ; /* [2]  */
        unsigned int    clk_gmac3_sys_dsb     : 1   ; /* [3]  */
        unsigned int    clk_gmac4_sys_dsb     : 1   ; /* [4]  */
        unsigned int    clk_gmac5_sys_dsb     : 1   ; /* [5]  */
        unsigned int    clk_gmac6_sys_dsb     : 1   ; /* [6]  */
        unsigned int    clk_gmac7_sys_dsb     : 1   ; /* [7]  */
        unsigned int    clk_gmac0_125m_dsb    : 1   ; /* [8]  */
        unsigned int    clk_gmac1_125m_dsb    : 1   ; /* [9]  */
        unsigned int    clk_gmac2_125m_dsb    : 1   ; /* [10]  */
        unsigned int    clk_gmac3_125m_dsb    : 1   ; /* [11]  */
        unsigned int    clk_gmac4_125m_dsb    : 1   ; /* [12]  */
        unsigned int    clk_gmac5_125m_dsb    : 1   ; /* [13]  */
        unsigned int    clk_gmac6_125m_dsb    : 1   ; /* [14]  */
        unsigned int    clk_gmac7_125m_dsb    : 1   ; /* [15]  */
        unsigned int    clk_gmac0_tx_dsb      : 1   ; /* [16]  */
        unsigned int    clk_gmac1_tx_dsb      : 1   ; /* [17]  */
        unsigned int    clk_gmac2_tx_dsb      : 1   ; /* [18]  */
        unsigned int    clk_gmac3_tx_dsb      : 1   ; /* [19]  */
        unsigned int    clk_gmac4_tx_dsb      : 1   ; /* [20]  */
        unsigned int    clk_gmac5_tx_dsb      : 1   ; /* [21]  */
        unsigned int    clk_gmac6_tx_dsb      : 1   ; /* [22]  */
        unsigned int    clk_gmac7_tx_dsb      : 1   ; /* [23]  */
        unsigned int    clk_gmac0_rx_dsb      : 1   ; /* [24]  */
        unsigned int    clk_gmac1_rx_dsb      : 1   ; /* [25]  */
        unsigned int    clk_gmac2_rx_dsb      : 1   ; /* [26]  */
        unsigned int    clk_gmac3_rx_dsb      : 1   ; /* [27]  */
        unsigned int    clk_gmac4_rx_dsb      : 1   ; /* [28]  */
        unsigned int    clk_gmac5_rx_dsb      : 1   ; /* [29]  */
        unsigned int    clk_gmac6_rx_dsb      : 1   ; /* [30]  */
        unsigned int    clk_gmac7_rx_dsb      : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_CLK_DIS_U;

/* Define the union U_SC_PPE_CLK_EN_U */
/* PPEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x320 */
typedef union tagScPpeClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_ppe0_enb          : 1   ; /* [0]  */
        unsigned int    clk_ppe1_enb          : 1   ; /* [1]  */
        unsigned int    clk_ppe2_enb          : 1   ; /* [2]  */
        unsigned int    clk_ppe3_enb          : 1   ; /* [3]  */
        unsigned int    clk_ppe4_enb          : 1   ; /* [4]  */
        unsigned int    clk_ppe5_enb          : 1   ; /* [5]  */
        unsigned int    clk_rcb_ppe6_enb      : 1   ; /* [6]  */
        unsigned int    clk_rcb_ppe7_enb      : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_CLK_EN_U;

/* Define the union U_SC_PPE_CLK_DIS_U */
/* PPEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x324 */
typedef union tagScPpeClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_ppe0_dsb          : 1   ; /* [0]  */
        unsigned int    clk_ppe1_dsb          : 1   ; /* [1]  */
        unsigned int    clk_ppe2_dsb          : 1   ; /* [2]  */
        unsigned int    clk_ppe3_dsb          : 1   ; /* [3]  */
        unsigned int    clk_ppe4_dsb          : 1   ; /* [4]  */
        unsigned int    clk_ppe5_dsb          : 1   ; /* [5]  */
        unsigned int    clk_rcb_ppe6_dsb      : 1   ; /* [6]  */
        unsigned int    clk_rcb_ppe7_dsb      : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_CLK_DIS_U;

/* Define the union U_SC_ROCEE_CLK_EN_U */
/* ROCEEÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x328 */
typedef union tagScRoceeClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rocee0_enb        : 1   ; /* [0]  */
        unsigned int    clk_rocee1_enb        : 1   ; /* [1]  */
        unsigned int    clk_rocee2_enb        : 1   ; /* [2]  */
        unsigned int    clk_rocee3_enb        : 1   ; /* [3]  */
        unsigned int    clk_rocee4_enb        : 1   ; /* [4]  */
        unsigned int    clk_rocee5_enb        : 1   ; /* [5]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_CLK_EN_U;

/* Define the union U_SC_ROCEE_CLK_DIS_U */
/* ROCEEÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x32C */
typedef union tagScRoceeClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rocee0_dsb        : 1   ; /* [0]  */
        unsigned int    clk_rocee1_dsb        : 1   ; /* [1]  */
        unsigned int    clk_rocee2_dsb        : 1   ; /* [2]  */
        unsigned int    clk_rocee3_dsb        : 1   ; /* [3]  */
        unsigned int    clk_rocee4_dsb        : 1   ; /* [4]  */
        unsigned int    clk_rocee5_dsb        : 1   ; /* [5]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_CLK_DIS_U;

/* Define the union U_SC_CPU_CLK_EN_U */
/* CPUÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x330 */
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
/* CPUÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x334 */
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

/* Define the union U_SC_SAS_CLK_EN_U */
/* SASÊ±ÖÓÃÅ¿ØÐÅºÅ */
/* 0x338 */
typedef union tagScSasClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sas_enb           : 1   ; /* [0]  */
        unsigned int    clk_sas_mem_enb       : 1   ; /* [1]  */
        unsigned int    clk_sas_ahb_enb       : 1   ; /* [2]  */
        unsigned int    clk_sas_oob_enb       : 1   ; /* [3]  */
        unsigned int    clk_sas_ch0_rx_enb    : 1   ; /* [4]  */
        unsigned int    clk_sas_ch1_rx_enb    : 1   ; /* [5]  */
        unsigned int    clk_sas_ch2_rx_enb    : 1   ; /* [6]  */
        unsigned int    clk_sas_ch3_rx_enb    : 1   ; /* [7]  */
        unsigned int    clk_sas_ch4_rx_enb    : 1   ; /* [8]  */
        unsigned int    clk_sas_ch5_rx_enb    : 1   ; /* [9]  */
        unsigned int    clk_sas_ch6_rx_enb    : 1   ; /* [10]  */
        unsigned int    clk_sas_ch7_rx_enb    : 1   ; /* [11]  */
        unsigned int    clk_sas_ch0_tx_enb    : 1   ; /* [12]  */
        unsigned int    clk_sas_ch1_tx_enb    : 1   ; /* [13]  */
        unsigned int    clk_sas_ch2_tx_enb    : 1   ; /* [14]  */
        unsigned int    clk_sas_ch3_tx_enb    : 1   ; /* [15]  */
        unsigned int    clk_sas_ch4_tx_enb    : 1   ; /* [16]  */
        unsigned int    clk_sas_ch5_tx_enb    : 1   ; /* [17]  */
        unsigned int    clk_sas_ch6_tx_enb    : 1   ; /* [18]  */
        unsigned int    clk_sas_ch7_tx_enb    : 1   ; /* [19]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_CLK_EN_U;

/* Define the union U_SC_SAS_CLK_DIS_U */
/* SASÊ±ÖÓÃÅ¿ØÐÅºÅ */
/* 0x33C */
typedef union tagScSasClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sas_dsb           : 1   ; /* [0]  */
        unsigned int    clk_sas_mem_dsb       : 1   ; /* [1]  */
        unsigned int    clk_sas_ahb_dsb       : 1   ; /* [2]  */
        unsigned int    clk_sas_oob_dsb       : 1   ; /* [3]  */
        unsigned int    clk_sas_ch0_rx_dsb    : 1   ; /* [4]  */
        unsigned int    clk_sas_ch1_rx_dsb    : 1   ; /* [5]  */
        unsigned int    clk_sas_ch2_rx_dsb    : 1   ; /* [6]  */
        unsigned int    clk_sas_ch3_rx_dsb    : 1   ; /* [7]  */
        unsigned int    clk_sas_ch4_rx_dsb    : 1   ; /* [8]  */
        unsigned int    clk_sas_ch5_rx_dsb    : 1   ; /* [9]  */
        unsigned int    clk_sas_ch6_rx_dsb    : 1   ; /* [10]  */
        unsigned int    clk_sas_ch7_rx_dsb    : 1   ; /* [11]  */
        unsigned int    clk_sas_ch0_tx_dsb    : 1   ; /* [12]  */
        unsigned int    clk_sas_ch1_tx_dsb    : 1   ; /* [13]  */
        unsigned int    clk_sas_ch2_tx_dsb    : 1   ; /* [14]  */
        unsigned int    clk_sas_ch3_tx_dsb    : 1   ; /* [15]  */
        unsigned int    clk_sas_ch4_tx_dsb    : 1   ; /* [16]  */
        unsigned int    clk_sas_ch5_tx_dsb    : 1   ; /* [17]  */
        unsigned int    clk_sas_ch6_tx_dsb    : 1   ; /* [18]  */
        unsigned int    clk_sas_ch7_tx_dsb    : 1   ; /* [19]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_CLK_DIS_U;

/* Define the union U_SC_ITS_CLK_EN_U */
/* itsÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x340 */
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
/* itsÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x344 */
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

/* Define the union U_SC_SRAM_CLK_EN_U */
/* SRAMÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x348 */
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
/* 0x34C */
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

/* Define the union U_SC_RCB_PPE_COM_CLK_EN_U */
/* RCB PPE¹«¹²Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x350 */
typedef union tagScRcbPpeComClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rcb_ppe_com_enb   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_CLK_EN_U;

/* Define the union U_SC_RCB_PPE_COM_CLK_DIS_U */
/* RCB PPE¹«¹²Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x354 */
typedef union tagScRcbPpeComClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rcb_ppe_com_dsb   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_CLK_DIS_U;

/* Define the union U_SC_TIMER_CLK_EN_U */
/* TIMERÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x358 */
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
/* 0x35C */
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

/* Define the union U_SC_SLLC_CLK_EN_U */
/* SLLC1Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x360 */
typedef union tagScSllcClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sllc_enb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_CLK_EN_U;

/* Define the union U_SC_SLLC_CLK_DIS_U */
/* SLLC1Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x364 */
typedef union tagScSllcClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sllc_dsb          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_CLK_DIS_U;

/* Define the union U_SC_XBAR_RESET_REQ_U */
/* XBARÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA00 */
typedef union tagScXbarResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    xbar_srst_req         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XBAR_RESET_REQ_U;

/* Define the union U_SC_XBAR_RESET_DREQ_U */
/* XBARÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA04 */
typedef union tagScXbarResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    xbar_srst_dreq        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XBAR_RESET_DREQ_U;

/* Define the union U_SC_NT_RESET_REQ_U */
/* NTÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA08 */
typedef union tagScNtResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nt_srst_req           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_RESET_REQ_U;

/* Define the union U_SC_NT_RESET_DREQ_U */
/* NTÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA0C */
typedef union tagScNtResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nt_srst_dreq          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_RESET_DREQ_U;

/* Define the union U_SC_XGE_RESET_REQ_U */
/* XGEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA10 */
typedef union tagScXgeResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    xge_cfg_srst_req      : 1   ; /* [0]  */
        unsigned int    xge0_cfg_mac_srst_req : 1   ; /* [1]  */
        unsigned int    xge1_cfg_mac_srst_req : 1   ; /* [2]  */
        unsigned int    xge2_cfg_mac_srst_req : 1   ; /* [3]  */
        unsigned int    xge3_cfg_mac_srst_req : 1   ; /* [4]  */
        unsigned int    xge4_cfg_mac_srst_req : 1   ; /* [5]  */
        unsigned int    xge5_cfg_mac_srst_req : 1   ; /* [6]  */
        unsigned int    xge0_core_tx_srst_req : 1   ; /* [7]  */
        unsigned int    xge1_core_tx_srst_req : 1   ; /* [8]  */
        unsigned int    xge2_core_tx_srst_req : 1   ; /* [9]  */
        unsigned int    xge3_core_tx_srst_req : 1   ; /* [10]  */
        unsigned int    xge4_core_tx_srst_req : 1   ; /* [11]  */
        unsigned int    xge5_core_tx_srst_req : 1   ; /* [12]  */
        unsigned int    xge0_core_rx_srst_req : 1   ; /* [13]  */
        unsigned int    xge1_core_rx_srst_req : 1   ; /* [14]  */
        unsigned int    xge2_core_rx_srst_req : 1   ; /* [15]  */
        unsigned int    xge3_core_rx_srst_req : 1   ; /* [16]  */
        unsigned int    xge4_core_rx_srst_req : 1   ; /* [17]  */
        unsigned int    xge5_core_rx_srst_req : 1   ; /* [18]  */
        unsigned int    xge0_fec_tx_srst_req  : 1   ; /* [19]  */
        unsigned int    xge1_fec_tx_srst_req  : 1   ; /* [20]  */
        unsigned int    xge2_fec_tx_srst_req  : 1   ; /* [21]  */
        unsigned int    xge3_fec_tx_srst_req  : 1   ; /* [22]  */
        unsigned int    xge4_fec_tx_srst_req  : 1   ; /* [23]  */
        unsigned int    xge5_fec_tx_srst_req  : 1   ; /* [24]  */
        unsigned int    xge0_fec_rx_srst_req  : 1   ; /* [25]  */
        unsigned int    xge1_fec_rx_srst_req  : 1   ; /* [26]  */
        unsigned int    xge2_fec_rx_srst_req  : 1   ; /* [27]  */
        unsigned int    xge3_fec_rx_srst_req  : 1   ; /* [28]  */
        unsigned int    xge4_fec_rx_srst_req  : 1   ; /* [29]  */
        unsigned int    xge5_fec_rx_srst_req  : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_RESET_REQ_U;

/* Define the union U_SC_XGE_RESET_DREQ_U */
/* XGEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA14 */
typedef union tagScXgeResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    xge_cfg_srst_dreq     : 1   ; /* [0]  */
        unsigned int    xge0_cfg_mac_srst_dreq : 1   ; /* [1]  */
        unsigned int    xge1_cfg_mac_srst_dreq : 1   ; /* [2]  */
        unsigned int    xge2_cfg_mac_srst_dreq : 1   ; /* [3]  */
        unsigned int    xge3_cfg_mac_srst_dreq : 1   ; /* [4]  */
        unsigned int    xge4_cfg_mac_srst_dreq : 1   ; /* [5]  */
        unsigned int    xge5_cfg_mac_srst_dreq : 1   ; /* [6]  */
        unsigned int    xge0_core_tx_srst_dreq : 1   ; /* [7]  */
        unsigned int    xge1_core_tx_srst_dreq : 1   ; /* [8]  */
        unsigned int    xge2_core_tx_srst_dreq : 1   ; /* [9]  */
        unsigned int    xge3_core_tx_srst_dreq : 1   ; /* [10]  */
        unsigned int    xge4_core_tx_srst_dreq : 1   ; /* [11]  */
        unsigned int    xge5_core_tx_srst_dreq : 1   ; /* [12]  */
        unsigned int    xge0_core_rx_srst_dreq : 1   ; /* [13]  */
        unsigned int    xge1_core_rx_srst_dreq : 1   ; /* [14]  */
        unsigned int    xge2_core_rx_srst_dreq : 1   ; /* [15]  */
        unsigned int    xge3_core_rx_srst_dreq : 1   ; /* [16]  */
        unsigned int    xge4_core_rx_srst_dreq : 1   ; /* [17]  */
        unsigned int    xge5_core_rx_srst_dreq : 1   ; /* [18]  */
        unsigned int    xge0_fec_tx_srst_dreq : 1   ; /* [19]  */
        unsigned int    xge1_fec_tx_srst_dreq : 1   ; /* [20]  */
        unsigned int    xge2_fec_tx_srst_dreq : 1   ; /* [21]  */
        unsigned int    xge3_fec_tx_srst_dreq : 1   ; /* [22]  */
        unsigned int    xge4_fec_tx_srst_dreq : 1   ; /* [23]  */
        unsigned int    xge5_fec_tx_srst_dreq : 1   ; /* [24]  */
        unsigned int    xge0_fec_rx_srst_dreq : 1   ; /* [25]  */
        unsigned int    xge1_fec_rx_srst_dreq : 1   ; /* [26]  */
        unsigned int    xge2_fec_rx_srst_dreq : 1   ; /* [27]  */
        unsigned int    xge3_fec_rx_srst_dreq : 1   ; /* [28]  */
        unsigned int    xge4_fec_rx_srst_dreq : 1   ; /* [29]  */
        unsigned int    xge5_fec_rx_srst_dreq : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_RESET_DREQ_U;

/* Define the union U_SC_GE_RESET_REQ0_U */
/* GEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷0 */
/* 0xA18 */
typedef union tagScGeResetReq0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gmac0_sys_srst_req    : 1   ; /* [0]  */
        unsigned int    gmac1_sys_srst_req    : 1   ; /* [1]  */
        unsigned int    gmac2_sys_srst_req    : 1   ; /* [2]  */
        unsigned int    gmac3_sys_srst_req    : 1   ; /* [3]  */
        unsigned int    gmac4_sys_srst_req    : 1   ; /* [4]  */
        unsigned int    gmac5_sys_srst_req    : 1   ; /* [5]  */
        unsigned int    gmac0_tx_srst_req     : 1   ; /* [6]  */
        unsigned int    gmac1_tx_srst_req     : 1   ; /* [7]  */
        unsigned int    gmac2_tx_srst_req     : 1   ; /* [8]  */
        unsigned int    gmac3_tx_srst_req     : 1   ; /* [9]  */
        unsigned int    gmac4_tx_srst_req     : 1   ; /* [10]  */
        unsigned int    gmac5_tx_srst_req     : 1   ; /* [11]  */
        unsigned int    gmac0_rx_srst_req     : 1   ; /* [12]  */
        unsigned int    gmac1_rx_srst_req     : 1   ; /* [13]  */
        unsigned int    gmac2_rx_srst_req     : 1   ; /* [14]  */
        unsigned int    gmac3_rx_srst_req     : 1   ; /* [15]  */
        unsigned int    gmac4_rx_srst_req     : 1   ; /* [16]  */
        unsigned int    gmac5_rx_srst_req     : 1   ; /* [17]  */
        unsigned int    gmac0_cfg_srst_req    : 1   ; /* [18]  */
        unsigned int    gmac1_cfg_srst_req    : 1   ; /* [19]  */
        unsigned int    gmac2_cfg_srst_req    : 1   ; /* [20]  */
        unsigned int    gmac3_cfg_srst_req    : 1   ; /* [21]  */
        unsigned int    gmac4_cfg_srst_req    : 1   ; /* [22]  */
        unsigned int    gmac5_cfg_srst_req    : 1   ; /* [23]  */
        unsigned int    gmac0_eth_srst_req    : 1   ; /* [24]  */
        unsigned int    gmac1_eth_srst_req    : 1   ; /* [25]  */
        unsigned int    gmac2_eth_srst_req    : 1   ; /* [26]  */
        unsigned int    gmac3_eth_srst_req    : 1   ; /* [27]  */
        unsigned int    gmac4_eth_srst_req    : 1   ; /* [28]  */
        unsigned int    gmac5_eth_srst_req    : 1   ; /* [29]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_REQ0_U;

/* Define the union U_SC_GE_RESET_DREQ0_U */
/* GEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷0 */
/* 0xA1C */
typedef union tagScGeResetDreq0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gmac0_sys_srst_dreq   : 1   ; /* [0]  */
        unsigned int    gmac1_sys_srst_dreq   : 1   ; /* [1]  */
        unsigned int    gmac2_sys_srst_dreq   : 1   ; /* [2]  */
        unsigned int    gmac3_sys_srst_dreq   : 1   ; /* [3]  */
        unsigned int    gmac4_sys_srst_dreq   : 1   ; /* [4]  */
        unsigned int    gmac5_sys_srst_dreq   : 1   ; /* [5]  */
        unsigned int    gmac0_tx_srst_dreq    : 1   ; /* [6]  */
        unsigned int    gmac1_tx_srst_dreq    : 1   ; /* [7]  */
        unsigned int    gmac2_tx_srst_dreq    : 1   ; /* [8]  */
        unsigned int    gmac3_tx_srst_dreq    : 1   ; /* [9]  */
        unsigned int    gmac4_tx_srst_dreq    : 1   ; /* [10]  */
        unsigned int    gmac5_tx_srst_dreq    : 1   ; /* [11]  */
        unsigned int    gmac0_rx_srst_dreq    : 1   ; /* [12]  */
        unsigned int    gmac1_rx_srst_dreq    : 1   ; /* [13]  */
        unsigned int    gmac2_rx_srst_dreq    : 1   ; /* [14]  */
        unsigned int    gmac3_rx_srst_dreq    : 1   ; /* [15]  */
        unsigned int    gmac4_rx_srst_dreq    : 1   ; /* [16]  */
        unsigned int    gmac5_rx_srst_dreq    : 1   ; /* [17]  */
        unsigned int    gmac0_cfg_srst_dreq   : 1   ; /* [18]  */
        unsigned int    gmac1_cfg_srst_dreq   : 1   ; /* [19]  */
        unsigned int    gmac2_cfg_srst_dreq   : 1   ; /* [20]  */
        unsigned int    gmac3_cfg_srst_dreq   : 1   ; /* [21]  */
        unsigned int    gmac4_cfg_srst_dreq   : 1   ; /* [22]  */
        unsigned int    gmac5_cfg_srst_dreq   : 1   ; /* [23]  */
        unsigned int    gmac0_eth_srst_dreq   : 1   ; /* [24]  */
        unsigned int    gmac1_eth_srst_dreq   : 1   ; /* [25]  */
        unsigned int    gmac2_eth_srst_dreq   : 1   ; /* [26]  */
        unsigned int    gmac3_eth_srst_dreq   : 1   ; /* [27]  */
        unsigned int    gmac4_eth_srst_dreq   : 1   ; /* [28]  */
        unsigned int    gmac5_eth_srst_dreq   : 1   ; /* [29]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_DREQ0_U;

/* Define the union U_SC_GE_RESET_REQ1_U */
/* GEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷1 */
/* 0xA20 */
typedef union tagScGeResetReq1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gmac0_fifo_srst_req   : 1   ; /* [0]  */
        unsigned int    gmac1_fifo_srst_req   : 1   ; /* [1]  */
        unsigned int    gmac2_fifo_srst_req   : 1   ; /* [2]  */
        unsigned int    gmac3_fifo_srst_req   : 1   ; /* [3]  */
        unsigned int    gmac4_fifo_srst_req   : 1   ; /* [4]  */
        unsigned int    gmac5_fifo_srst_req   : 1   ; /* [5]  */
        unsigned int    gmac6_sys_srst_req    : 1   ; /* [6]  */
        unsigned int    gmac7_sys_srst_req    : 1   ; /* [7]  */
        unsigned int    gmac6_tx_srst_req     : 1   ; /* [8]  */
        unsigned int    gmac7_tx_srst_req     : 1   ; /* [9]  */
        unsigned int    gmac6_rx_srst_req     : 1   ; /* [10]  */
        unsigned int    gmac7_rx_srst_req     : 1   ; /* [11]  */
        unsigned int    gmac6_cfg_srst_req    : 1   ; /* [12]  */
        unsigned int    gmac7_cfg_srst_req    : 1   ; /* [13]  */
        unsigned int    gmac6_eth_srst_req    : 1   ; /* [14]  */
        unsigned int    gmac7_eth_srst_req    : 1   ; /* [15]  */
        unsigned int    gmac6_fifo_srst_req   : 1   ; /* [16]  */
        unsigned int    gmac7_fifo_srst_req   : 1   ; /* [17]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_REQ1_U;

/* Define the union U_SC_GE_RESET_DREQ1_U */
/* GEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷1 */
/* 0xA24 */
typedef union tagScGeResetDreq1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gmac0_fifo_srst_dreq  : 1   ; /* [0]  */
        unsigned int    gmac1_fifo_srst_dreq  : 1   ; /* [1]  */
        unsigned int    gmac2_fifo_srst_dreq  : 1   ; /* [2]  */
        unsigned int    gmac3_fifo_srst_dreq  : 1   ; /* [3]  */
        unsigned int    gmac4_fifo_srst_dreq  : 1   ; /* [4]  */
        unsigned int    gmac5_fifo_srst_dreq  : 1   ; /* [5]  */
        unsigned int    gmac6_sys_srst_dreq   : 1   ; /* [6]  */
        unsigned int    gmac7_sys_srst_dreq   : 1   ; /* [7]  */
        unsigned int    gmac6_tx_srst_dreq    : 1   ; /* [8]  */
        unsigned int    gmac7_tx_srst_dreq    : 1   ; /* [9]  */
        unsigned int    gmac6_rx_srst_dreq    : 1   ; /* [10]  */
        unsigned int    gmac7_rx_srst_dreq    : 1   ; /* [11]  */
        unsigned int    gmac6_cfg_srst_dreq   : 1   ; /* [12]  */
        unsigned int    gmac7_cfg_srst_dreq   : 1   ; /* [13]  */
        unsigned int    gmac6_eth_srst_dreq   : 1   ; /* [14]  */
        unsigned int    gmac7_eth_srst_dreq   : 1   ; /* [15]  */
        unsigned int    gmac6_fifo_srst_dreq  : 1   ; /* [16]  */
        unsigned int    gmac7_fifo_srst_dreq  : 1   ; /* [17]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_DREQ1_U;

/* Define the union U_SC_PPE_RESET_REQ_U */
/* PPEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA48 */
typedef union tagScPpeResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ppe0_srst_req         : 1   ; /* [0]  */
        unsigned int    ppe1_srst_req         : 1   ; /* [1]  */
        unsigned int    ppe2_srst_req         : 1   ; /* [2]  */
        unsigned int    ppe3_srst_req         : 1   ; /* [3]  */
        unsigned int    ppe4_srst_req         : 1   ; /* [4]  */
        unsigned int    ppe5_srst_req         : 1   ; /* [5]  */
        unsigned int    ppe6_srst_req         : 1   ; /* [6]  */
        unsigned int    ppe7_srst_req         : 1   ; /* [7]  */
        unsigned int    rcb1_srst_req         : 1   ; /* [8]  */
        unsigned int    rcb2_srst_req         : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_RESET_REQ_U;

/* Define the union U_SC_PPE_RESET_DREQ_U */
/* PPEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA4C */
typedef union tagScPpeResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ppe0_srst_dreq        : 1   ; /* [0]  */
        unsigned int    ppe1_srst_dreq        : 1   ; /* [1]  */
        unsigned int    ppe2_srst_dreq        : 1   ; /* [2]  */
        unsigned int    ppe3_srst_dreq        : 1   ; /* [3]  */
        unsigned int    ppe4_srst_dreq        : 1   ; /* [4]  */
        unsigned int    ppe5_srst_dreq        : 1   ; /* [5]  */
        unsigned int    ppe6_srst_dreq        : 1   ; /* [6]  */
        unsigned int    ppe7_srst_dreq        : 1   ; /* [7]  */
        unsigned int    rcb1_srst_dreq        : 1   ; /* [8]  */
        unsigned int    rcb2_srst_dreq        : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_RESET_DREQ_U;

/* Define the union U_SC_ROCEE_RESET_REQ_U */
/* ROCEEÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA50 */
typedef union tagScRoceeResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rocee_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_RESET_REQ_U;

/* Define the union U_SC_ROCEE_RESET_DREQ_U */
/* ROCEEÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA54 */
typedef union tagScRoceeResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rocee_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_RESET_DREQ_U;

/* Define the union U_SC_CPU_RESET_REQ_U */
/* CPUÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA58 */
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
/* CPUÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA5C */
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

/* Define the union U_SC_SAS_RESET_REQ_U */
/* SASÈí¸´Î»ÇëÇó */
/* 0xA60 */
typedef union tagScSasResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sas_srst_req          : 1   ; /* [0]  */
        unsigned int    sas_oob_srst_req      : 1   ; /* [1]  */
        unsigned int    sas_ahb_srst_req      : 1   ; /* [2]  */
        unsigned int    sas_ch0_rx_srst_req   : 1   ; /* [3]  */
        unsigned int    sas_ch1_rx_srst_req   : 1   ; /* [4]  */
        unsigned int    sas_ch2_rx_srst_req   : 1   ; /* [5]  */
        unsigned int    sas_ch3_rx_srst_req   : 1   ; /* [6]  */
        unsigned int    sas_ch4_rx_srst_req   : 1   ; /* [7]  */
        unsigned int    sas_ch5_rx_srst_req   : 1   ; /* [8]  */
        unsigned int    sas_ch6_rx_srst_req   : 1   ; /* [9]  */
        unsigned int    sas_ch7_rx_srst_req   : 1   ; /* [10]  */
        unsigned int    sas_ch0_tx_srst_req   : 1   ; /* [11]  */
        unsigned int    sas_ch1_tx_srst_req   : 1   ; /* [12]  */
        unsigned int    sas_ch2_tx_srst_req   : 1   ; /* [13]  */
        unsigned int    sas_ch3_tx_srst_req   : 1   ; /* [14]  */
        unsigned int    sas_ch4_tx_srst_req   : 1   ; /* [15]  */
        unsigned int    sas_ch5_tx_srst_req   : 1   ; /* [16]  */
        unsigned int    sas_ch6_tx_srst_req   : 1   ; /* [17]  */
        unsigned int    sas_ch7_tx_srst_req   : 1   ; /* [18]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_RESET_REQ_U;

/* Define the union U_SC_SAS_RESET_DREQ_U */
/* SAS³·ÏúÈí¸´Î»ÇëÇó */
/* 0xA64 */
typedef union tagScSasResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sas_srst_dreq         : 1   ; /* [0]  */
        unsigned int    sas_oob_srst_dreq     : 1   ; /* [1]  */
        unsigned int    sas_ahb_srst_dreq     : 1   ; /* [2]  */
        unsigned int    sas_ch0_rx_srst_dreq  : 1   ; /* [3]  */
        unsigned int    sas_ch1_rx_srst_dreq  : 1   ; /* [4]  */
        unsigned int    sas_ch2_rx_srst_dreq  : 1   ; /* [5]  */
        unsigned int    sas_ch3_rx_srst_dreq  : 1   ; /* [6]  */
        unsigned int    sas_ch4_rx_srst_dreq  : 1   ; /* [7]  */
        unsigned int    sas_ch5_rx_srst_dreq  : 1   ; /* [8]  */
        unsigned int    sas_ch6_rx_srst_dreq  : 1   ; /* [9]  */
        unsigned int    sas_ch7_rx_srst_dreq  : 1   ; /* [10]  */
        unsigned int    sas_ch0_tx_srst_dreq  : 1   ; /* [11]  */
        unsigned int    sas_ch1_tx_srst_dreq  : 1   ; /* [12]  */
        unsigned int    sas_ch2_tx_srst_dreq  : 1   ; /* [13]  */
        unsigned int    sas_ch3_tx_srst_dreq  : 1   ; /* [14]  */
        unsigned int    sas_ch4_tx_srst_dreq  : 1   ; /* [15]  */
        unsigned int    sas_ch5_tx_srst_dreq  : 1   ; /* [16]  */
        unsigned int    sas_ch6_tx_srst_dreq  : 1   ; /* [17]  */
        unsigned int    sas_ch7_tx_srst_dreq  : 1   ; /* [18]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_RESET_DREQ_U;

/* Define the union U_SC_SLLC_TSVRX_RESET_REQ_U */
/* SLLC TSVRXÍ¨µÀÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA68 */
typedef union tagScSllcTsvrxResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc_tsvrx0_srst_req  : 1   ; /* [0]  */
        unsigned int    sllc_tsvrx1_srst_req  : 1   ; /* [1]  */
        unsigned int    sllc_tsvrx2_srst_req  : 1   ; /* [2]  */
        unsigned int    sllc_tsvrx3_srst_req  : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_TSVRX_RESET_REQ_U;

/* Define the union U_SC_SLLC_TSVRX_RESET_DREQ_U */
/* SLLC TSVRXÍ¨µÀÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA6C */
typedef union tagScSllcTsvrxResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc_tsvrx0_srst_dreq : 1   ; /* [0]  */
        unsigned int    sllc_tsvrx1_srst_dreq : 1   ; /* [1]  */
        unsigned int    sllc_tsvrx2_srst_dreq : 1   ; /* [2]  */
        unsigned int    sllc_tsvrx3_srst_dreq : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_TSVRX_RESET_DREQ_U;

/* Define the union U_SC_ITS_RESET_REQ_U */
/* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA70 */
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
/* 0xA74 */
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

/* Define the union U_SC_SRAM_RESET_REQ_U */
/* SRAMÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA80 */
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
/* 0xA84 */
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

/* Define the union U_SC_RCB_PPE_COM_RESET_REQ_U */
/* RCB PPE¹«¹²Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA88 */
typedef union tagScRcbPpeComResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rcb_ppe_com_srst_req  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_RESET_REQ_U;

/* Define the union U_SC_RCB_PPE_COM_RESET_DREQ_U */
/* RCB PPE¹«¹²Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA8C */
typedef union tagScRcbPpeComResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rcb_ppe_com_srst_dreq : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_RESET_DREQ_U;

/* Define the union U_SC_TIMER_RESET_REQ_U */
/* TIMERÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA98 */
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
/* 0xA9C */
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

/* Define the union U_SC_SLLC_RESET_REQ_U */
/* SLLC1Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAA0 */
typedef union tagScSllcResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc_srst_req         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_RESET_REQ_U;

/* Define the union U_SC_SLLC_RESET_DREQ_U */
/* SLLC1Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAA4 */
typedef union tagScSllcResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc_srst_dreq        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_RESET_DREQ_U;

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

/* Define the union U_SC_M3_CTRL_U */
/* M3¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2040 */
typedef union tagScM3Ctrl
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

} SC_M3_CTRL_U;

/* Define the union U_SC_REMAP_SRAM_ADDR_CTRL_U */
/* M3 remapµ½sramµÄµØÖ·ÅäÖÃ */
/* 0x2050 */
typedef union tagScRemapSramAddrCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    remap_sram_addr       : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_REMAP_SRAM_ADDR_CTRL_U;

/* Define the union U_SC_LIGHT_MODULE_DETECT_EN_U */
/* ¹âÄ£¿éSDÐÅºÅ¼ì²âÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x2060 */
typedef union tagScLightModuleDetectEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    light_module_sd_0_en  : 1   ; /* [0]  */
        unsigned int    light_module_sd_1_en  : 1   ; /* [1]  */
        unsigned int    light_module_sd_2_en  : 1   ; /* [2]  */
        unsigned int    light_module_sd_3_en  : 1   ; /* [3]  */
        unsigned int    light_module_sd_4_en  : 1   ; /* [4]  */
        unsigned int    light_module_sd_5_en  : 1   ; /* [5]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_LIGHT_MODULE_DETECT_EN_U;

/* Define the union U_SC_TIMER_CLKEN_CTRL_U */
/* TIMERµÄclkenÉú³É¿ØÖÆ¼Ä´æÆ÷ */
/* 0x2070 */
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

/* Define the union U_SC_VMID_CTRL0_U */
/* M3 VMIDÅäÖÃ */
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

/* Define the union U_SC_ROCEE_VMID_U */
/* ROCEE VMIDÅäÖÃ */
/* 0x20E4 */
typedef union tagScRoceeVmid
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rocee_vmid            : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_VMID_U;

/* Define the union U_SC_ITS_M3_INT_MUX_SEL_U */
/* ¶ÔÖÐ¶ÏËÍµ½ITS»¹ÊÇM3£¬×öMUXÑ¡Ôñ */
/* 0x21F0 */
typedef union tagScItsM3IntMuxSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    int_tbl_old_mux_sel   : 1   ; /* [0]  */
        unsigned int    int_tbl_sa_mux_sel    : 1   ; /* [1]  */
        unsigned int    int_tbl_da_mux_sel    : 1   ; /* [2]  */
        unsigned int    int_m3_ring1_mux_sel  : 1   ; /* [3]  */
        unsigned int    int_m3_ring0_mux_sel  : 1   ; /* [4]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ITS_M3_INT_MUX_SEL_U;

/* Define the union U_SC_FTE_MUX_SEL_U */
/*  */
/* 0x2200 */
typedef union tagScFteMuxSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mux_sel_fte           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_FTE_MUX_SEL_U;

/* Define the union U_SC_TCAM_MBIST_EN_U */
/* DSAFÖÐTCAM MBISTÆô¶¯Ê¹ÄÜ¡£ */
/* 0x2300 */
typedef union tagScTcamMbistEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tcam_mbist_en         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TCAM_MBIST_EN_U;

/* Define the union U_SC_HILINK2_LRSTB_MUX_CTRL_U */
/* HILINK2 lrstb[7:0]µÄMUXÑ¡Ôñ¿ØÖÆ */
/* 0x2340 */
typedef union tagScHilink2LrstbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_lrstb_mux_sel : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_LRSTB_MUX_CTRL_U;

/* Define the union U_SC_HILINK2_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2400 */
typedef union tagScHilink2MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_ss_refclk0_x2s : 2   ; /* [1..0]  */
        unsigned int    hilink2_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink2_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink2_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink2_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink2_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink2_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink2_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK2_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2404 */
typedef union tagScHilink2MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
        unsigned int    hilink2_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink2_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink2_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink2_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink2_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink2_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink2_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink2_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink2_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK2_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2408 */
typedef union tagScHilink2MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_lifeclk2dig_sel : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK2_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x240C */
typedef union tagScHilink2MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_core_clk_selext : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK2_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2410 */
typedef union tagScHilink2MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_core_clk_sel  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK2_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2414 */
typedef union tagScHilink2MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_ctrl_bus_mode : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK2_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2418 */
typedef union tagScHilink2MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_macropwrdb    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK2_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x241C */
typedef union tagScHilink2MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_grstb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK2_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2420 */
typedef union tagScHilink2MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_bit_slip      : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK2_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2424 */
typedef union tagScHilink2MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_lrstb         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_LRSTB_U;

/* Define the union U_SC_HILINK3_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2500 */
typedef union tagScHilink3MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_ss_refclk0_x2s : 2   ; /* [1..0]  */
        unsigned int    hilink3_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink3_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink3_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink3_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink3_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink3_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink3_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK3_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2504 */
typedef union tagScHilink3MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
        unsigned int    hilink3_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink3_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink3_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink3_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink3_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink3_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink3_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink3_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink3_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK3_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2508 */
typedef union tagScHilink3MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_lifeclk2dig_sel : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK3_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x250C */
typedef union tagScHilink3MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_core_clk_selext : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK3_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2510 */
typedef union tagScHilink3MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_core_clk_sel  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK3_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2514 */
typedef union tagScHilink3MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_ctrl_bus_mode : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK3_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2518 */
typedef union tagScHilink3MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_macropwrdb    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK3_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x251C */
typedef union tagScHilink3MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_grstb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK3_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2520 */
typedef union tagScHilink3MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_bit_slip      : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK3_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2524 */
typedef union tagScHilink3MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_lrstb         : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_LRSTB_U;

/* Define the union U_SC_HILINK4_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2600 */
typedef union tagScHilink4MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_ss_refclk0_x2s : 2   ; /* [1..0]  */
        unsigned int    hilink4_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink4_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink4_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink4_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink4_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink4_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink4_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK4_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2604 */
typedef union tagScHilink4MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_cs_refclk0_dirsel0 : 1   ; /* [0]  */
        unsigned int    hilink4_cs_refclk0_dirsel1 : 1   ; /* [1]  */
        unsigned int    hilink4_cs_refclk0_dirsel2 : 1   ; /* [2]  */
        unsigned int    hilink4_cs_refclk0_dirsel3 : 1   ; /* [3]  */
        unsigned int    hilink4_cs_refclk0_dirsel4 : 1   ; /* [4]  */
        unsigned int    hilink4_cs_refclk1_dirsel0 : 1   ; /* [5]  */
        unsigned int    hilink4_cs_refclk1_dirsel1 : 1   ; /* [6]  */
        unsigned int    hilink4_cs_refclk1_dirsel2 : 1   ; /* [7]  */
        unsigned int    hilink4_cs_refclk1_dirsel3 : 1   ; /* [8]  */
        unsigned int    hilink4_cs_refclk1_dirsel4 : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK4_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2608 */
typedef union tagScHilink4MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_lifeclk2dig_sel : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK4_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x260C */
typedef union tagScHilink4MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_core_clk_selext : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK4_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2610 */
typedef union tagScHilink4MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_core_clk_sel  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK4_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2614 */
typedef union tagScHilink4MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_ctrl_bus_mode : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK4_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2618 */
typedef union tagScHilink4MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_macropwrdb    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK4_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x261C */
typedef union tagScHilink4MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_grstb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK4_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2620 */
typedef union tagScHilink4MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_bit_slip      : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK4_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2624 */
typedef union tagScHilink4MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_lrstb         : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_LRSTB_U;

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

/* Define the union U_SC_SLLC1_MEM_CTRL_U */
/* sllc1 mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3010 */
typedef union tagScSllc1MemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rft_sllc1        : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC1_MEM_CTRL_U;

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

/* Define the union U_SC_ROCEE_MEM_CTRL0_U */
/* rocee mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3030 */
typedef union tagScRoceeMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_rocee        : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_MEM_CTRL0_U;

/* Define the union U_SC_ROCEE_MEM_CTRL1_U */
/* rocee mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3034 */
typedef union tagScRoceeMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rft_rocee        : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_MEM_CTRL1_U;

/* Define the union U_SC_XGE_MEM_CTRL0_U */
/* xge mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3040 */
typedef union tagScXgeMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_xge          : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_MEM_CTRL0_U;

/* Define the union U_SC_XGE_MEM_CTRL1_U */
/* xge mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3044 */
typedef union tagScXgeMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rft_xge          : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_MEM_CTRL1_U;

/* Define the union U_SC_SAS_MEM_CTRL_U */
/* sas mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3050 */
typedef union tagScSasMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_sas          : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_MEM_CTRL_U;

/* Define the union U_SC_GE_MEM_CTRL_U */
/* ge mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3060 */
typedef union tagScGeMemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_ge           : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_MEM_CTRL_U;

/* Define the union U_SC_XBAR_MEM_CTRL0_U */
/* xbar mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3070 */
typedef union tagScXbarMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_xbar         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XBAR_MEM_CTRL0_U;

/* Define the union U_SC_XBAR_MEM_CTRL1_U */
/* xbar mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3074 */
typedef union tagScXbarMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rashde_xbar      : 7   ; /* [6..0]  */
        unsigned int    reserved_0            : 25  ; /* [31..7]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XBAR_MEM_CTRL1_U;

/* Define the union U_SC_RCB_MEM_CTRL0_U */
/* rcb mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3080 */
typedef union tagScRcbMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_rcb          : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_MEM_CTRL0_U;

/* Define the union U_SC_RCB_MEM_CTRL1_U */
/* rcb mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3084 */
typedef union tagScRcbMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rft_rcb          : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_MEM_CTRL1_U;

/* Define the union U_SC_DSAF_CLK_ST_U */
/* DSAFÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5300 */
typedef union tagScDsafClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_xbar_xge0_st      : 1   ; /* [0]  */
        unsigned int    clk_xbar_xge1_st      : 1   ; /* [1]  */
        unsigned int    clk_xbar_xge2_st      : 1   ; /* [2]  */
        unsigned int    clk_xbar_xge3_st      : 1   ; /* [3]  */
        unsigned int    clk_xbar_xge4_st      : 1   ; /* [4]  */
        unsigned int    clk_xbar_xge5_st      : 1   ; /* [5]  */
        unsigned int    clk_xbar_ppe_st       : 1   ; /* [6]  */
        unsigned int    reserved_2            : 5   ; /* [11..7]  */
        unsigned int    clk_xbar_roce_st      : 1   ; /* [12]  */
        unsigned int    reserved_1            : 5   ; /* [17..13]  */
        unsigned int    clk_xbar_com_st       : 1   ; /* [18]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DSAF_CLK_ST_U;

/* Define the union U_SC_NT_CLK_ST_U */
/* NTÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5304 */
typedef union tagScNtClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_nt_st             : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_CLK_ST_U;

/* Define the union U_SC_XGE_CLK_ST_U */
/* XGEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5308 */
typedef union tagScXgeClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_xge_cfg_st        : 1   ; /* [0]  */
        unsigned int    clk_xge0_tx_st        : 1   ; /* [1]  */
        unsigned int    clk_xge1_tx_st        : 1   ; /* [2]  */
        unsigned int    clk_xge2_tx_st        : 1   ; /* [3]  */
        unsigned int    clk_xge3_tx_st        : 1   ; /* [4]  */
        unsigned int    clk_xge4_tx_st        : 1   ; /* [5]  */
        unsigned int    clk_xge5_tx_st        : 1   ; /* [6]  */
        unsigned int    clk_xge0_rx_st        : 1   ; /* [7]  */
        unsigned int    clk_xge1_rx_st        : 1   ; /* [8]  */
        unsigned int    clk_xge2_rx_st        : 1   ; /* [9]  */
        unsigned int    clk_xge3_rx_st        : 1   ; /* [10]  */
        unsigned int    clk_xge4_rx_st        : 1   ; /* [11]  */
        unsigned int    clk_xge5_rx_st        : 1   ; /* [12]  */
        unsigned int    reserved_0            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_CLK_ST_U;

/* Define the union U_SC_GE_CLK_ST_U */
/* GEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x530C */
typedef union tagScGeClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_gmac0_sys_st      : 1   ; /* [0]  */
        unsigned int    clk_gmac1_sys_st      : 1   ; /* [1]  */
        unsigned int    clk_gmac2_sys_st      : 1   ; /* [2]  */
        unsigned int    clk_gmac3_sys_st      : 1   ; /* [3]  */
        unsigned int    clk_gmac4_sys_st      : 1   ; /* [4]  */
        unsigned int    clk_gmac5_sys_st      : 1   ; /* [5]  */
        unsigned int    clk_gmac6_sys_st      : 1   ; /* [6]  */
        unsigned int    clk_gmac7_sys_st      : 1   ; /* [7]  */
        unsigned int    clk_gmac0_125m_st     : 1   ; /* [8]  */
        unsigned int    clk_gmac1_125m_st     : 1   ; /* [9]  */
        unsigned int    clk_gmac2_125m_st     : 1   ; /* [10]  */
        unsigned int    clk_gmac3_125m_st     : 1   ; /* [11]  */
        unsigned int    clk_gmac4_125m_st     : 1   ; /* [12]  */
        unsigned int    clk_gmac5_125m_st     : 1   ; /* [13]  */
        unsigned int    clk_gmac6_125m_st     : 1   ; /* [14]  */
        unsigned int    clk_gmac7_125m_st     : 1   ; /* [15]  */
        unsigned int    clk_gmac0_tx_st       : 1   ; /* [16]  */
        unsigned int    clk_gmac1_tx_st       : 1   ; /* [17]  */
        unsigned int    clk_gmac2_tx_st       : 1   ; /* [18]  */
        unsigned int    clk_gmac3_tx_st       : 1   ; /* [19]  */
        unsigned int    clk_gmac4_tx_st       : 1   ; /* [20]  */
        unsigned int    clk_gmac5_tx_st       : 1   ; /* [21]  */
        unsigned int    clk_gmac6_tx_st       : 1   ; /* [22]  */
        unsigned int    clk_gmac7_tx_st       : 1   ; /* [23]  */
        unsigned int    clk_gmac0_rx_st       : 1   ; /* [24]  */
        unsigned int    clk_gmac1_rx_st       : 1   ; /* [25]  */
        unsigned int    clk_gmac2_rx_st       : 1   ; /* [26]  */
        unsigned int    clk_gmac3_rx_st       : 1   ; /* [27]  */
        unsigned int    clk_gmac4_rx_st       : 1   ; /* [28]  */
        unsigned int    clk_gmac5_rx_st       : 1   ; /* [29]  */
        unsigned int    clk_gmac6_rx_st       : 1   ; /* [30]  */
        unsigned int    clk_gmac7_rx_st       : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_CLK_ST_U;

/* Define the union U_SC_PPE_CLK_ST_U */
/* PPEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5310 */
typedef union tagScPpeClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_ppe0_st           : 1   ; /* [0]  */
        unsigned int    clk_ppe1_st           : 1   ; /* [1]  */
        unsigned int    clk_ppe2_st           : 1   ; /* [2]  */
        unsigned int    clk_ppe3_st           : 1   ; /* [3]  */
        unsigned int    clk_ppe4_st           : 1   ; /* [4]  */
        unsigned int    clk_ppe5_st           : 1   ; /* [5]  */
        unsigned int    clk_rcb_ppe6_st       : 1   ; /* [6]  */
        unsigned int    clk_rcb_ppe7_st       : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_CLK_ST_U;

/* Define the union U_SC_ROCEE_CLK_ST_U */
/* ROCEEÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5314 */
typedef union tagScRoceeClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rocee0_st         : 1   ; /* [0]  */
        unsigned int    clk_rocee1_st         : 1   ; /* [1]  */
        unsigned int    clk_rocee2_st         : 1   ; /* [2]  */
        unsigned int    clk_rocee3_st         : 1   ; /* [3]  */
        unsigned int    clk_rocee4_st         : 1   ; /* [4]  */
        unsigned int    clk_rocee5_st         : 1   ; /* [5]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_CLK_ST_U;

/* Define the union U_SC_CPU_CLK_ST_U */
/* CPUÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5318 */
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

/* Define the union U_SC_SAS_CLK_ST_U */
/* SASÊ±ÖÓÃÅ¿ØÐÅºÅ×´Ì¬ */
/* 0x531C */
typedef union tagScSasClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sas_st            : 1   ; /* [0]  */
        unsigned int    clk_sas_mem_st        : 1   ; /* [1]  */
        unsigned int    clk_sas_ahb_st        : 1   ; /* [2]  */
        unsigned int    clk_sas_oob_st        : 1   ; /* [3]  */
        unsigned int    clk_sas_ch0_rx_st     : 1   ; /* [4]  */
        unsigned int    clk_sas_ch1_rx_st     : 1   ; /* [5]  */
        unsigned int    clk_sas_ch2_rx_st     : 1   ; /* [6]  */
        unsigned int    clk_sas_ch3_rx_st     : 1   ; /* [7]  */
        unsigned int    clk_sas_ch4_rx_st     : 1   ; /* [8]  */
        unsigned int    clk_sas_ch5_rx_st     : 1   ; /* [9]  */
        unsigned int    clk_sas_ch6_rx_st     : 1   ; /* [10]  */
        unsigned int    clk_sas_ch7_rx_st     : 1   ; /* [11]  */
        unsigned int    clk_sas_ch0_tx_st     : 1   ; /* [12]  */
        unsigned int    clk_sas_ch1_tx_st     : 1   ; /* [13]  */
        unsigned int    clk_sas_ch2_tx_st     : 1   ; /* [14]  */
        unsigned int    clk_sas_ch3_tx_st     : 1   ; /* [15]  */
        unsigned int    clk_sas_ch4_tx_st     : 1   ; /* [16]  */
        unsigned int    clk_sas_ch5_tx_st     : 1   ; /* [17]  */
        unsigned int    clk_sas_ch6_tx_st     : 1   ; /* [18]  */
        unsigned int    clk_sas_ch7_tx_st     : 1   ; /* [19]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_CLK_ST_U;

/* Define the union U_SC_ITS_CLK_ST_U */
/* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5320 */
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

/* Define the union U_SC_SRAM_CLK_ST_U */
/* SRAMÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5324 */
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

/* Define the union U_SC_RCB_PPE_COM_CLK_ST_U */
/* RCB PPE¹«¹²Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5328 */
typedef union tagScRcbPpeComClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_rcb_ppe_com_st    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_CLK_ST_U;

/* Define the union U_SC_TIMER_CLK_ST_U */
/* TIMERÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x532C */
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

/* Define the union U_SC_SLLC_CLK_ST_U */
/* SLLC1Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5330 */
typedef union tagScSllcClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_sllc_st           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_CLK_ST_U;

/* Define the union U_SC_XBAR_RESET_ST_U */
/* DSAFÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A00 */
typedef union tagScXbarResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    xbar_srst_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XBAR_RESET_ST_U;

/* Define the union U_SC_NT_RESET_ST_U */
/* NTÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A04 */
typedef union tagScNtResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    nt_srst_st            : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_NT_RESET_ST_U;

/* Define the union U_SC_XGE_RESET_ST_U */
/* XGEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A08 */
typedef union tagScXgeResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    xge_cfg_srst_st       : 1   ; /* [0]  */
        unsigned int    xge0_cfg_mac_srst_st  : 1   ; /* [1]  */
        unsigned int    xge1_cfg_mac_srst_st  : 1   ; /* [2]  */
        unsigned int    xge2_cfg_mac_srst_st  : 1   ; /* [3]  */
        unsigned int    xge3_cfg_mac_srst_st  : 1   ; /* [4]  */
        unsigned int    xge4_cfg_mac_srst_st  : 1   ; /* [5]  */
        unsigned int    xge5_cfg_mac_srst_st  : 1   ; /* [6]  */
        unsigned int    xge0_core_tx_srst_st  : 1   ; /* [7]  */
        unsigned int    xge1_core_tx_srst_st  : 1   ; /* [8]  */
        unsigned int    xge2_core_tx_srst_st  : 1   ; /* [9]  */
        unsigned int    xge3_core_tx_srst_st  : 1   ; /* [10]  */
        unsigned int    xge4_core_tx_srst_st  : 1   ; /* [11]  */
        unsigned int    xge5_core_tx_srst_st  : 1   ; /* [12]  */
        unsigned int    xge0_core_rx_srst_st  : 1   ; /* [13]  */
        unsigned int    xge1_core_rx_srst_st  : 1   ; /* [14]  */
        unsigned int    xge2_core_rx_srst_st  : 1   ; /* [15]  */
        unsigned int    xge3_core_rx_srst_st  : 1   ; /* [16]  */
        unsigned int    xge4_core_rx_srst_st  : 1   ; /* [17]  */
        unsigned int    xge5_core_rx_srst_st  : 1   ; /* [18]  */
        unsigned int    xge0_fec_tx_srst_st   : 1   ; /* [19]  */
        unsigned int    xge1_fec_tx_srst_st   : 1   ; /* [20]  */
        unsigned int    xge2_fec_tx_srst_st   : 1   ; /* [21]  */
        unsigned int    xge3_fec_tx_srst_st   : 1   ; /* [22]  */
        unsigned int    xge4_fec_tx_srst_st   : 1   ; /* [23]  */
        unsigned int    xge5_fec_tx_srst_st   : 1   ; /* [24]  */
        unsigned int    xge0_fec_rx_srst_st   : 1   ; /* [25]  */
        unsigned int    xge1_fec_rx_srst_st   : 1   ; /* [26]  */
        unsigned int    xge2_fec_rx_srst_st   : 1   ; /* [27]  */
        unsigned int    xge3_fec_rx_srst_st   : 1   ; /* [28]  */
        unsigned int    xge4_fec_rx_srst_st   : 1   ; /* [29]  */
        unsigned int    xge5_fec_rx_srst_st   : 1   ; /* [30]  */
        unsigned int    reserved_0            : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_RESET_ST_U;

/* Define the union U_SC_GE_RESET_ST0_U */
/* GEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷0 */
/* 0x5A0C */
typedef union tagScGeResetSt0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gmac0_sys_srst_st     : 1   ; /* [0]  */
        unsigned int    gmac1_sys_srst_st     : 1   ; /* [1]  */
        unsigned int    gmac2_sys_srst_st     : 1   ; /* [2]  */
        unsigned int    gmac3_sys_srst_st     : 1   ; /* [3]  */
        unsigned int    gmac4_sys_srst_st     : 1   ; /* [4]  */
        unsigned int    gmac5_sys_srst_st     : 1   ; /* [5]  */
        unsigned int    gmac0_tx_srst_st      : 1   ; /* [6]  */
        unsigned int    gmac1_tx_srst_st      : 1   ; /* [7]  */
        unsigned int    gmac2_tx_srst_st      : 1   ; /* [8]  */
        unsigned int    gmac3_tx_srst_st      : 1   ; /* [9]  */
        unsigned int    gmac4_tx_srst_st      : 1   ; /* [10]  */
        unsigned int    gmac5_tx_srst_st      : 1   ; /* [11]  */
        unsigned int    gmac0_rx_srst_st      : 1   ; /* [12]  */
        unsigned int    gmac1_rx_srst_st      : 1   ; /* [13]  */
        unsigned int    gmac2_rx_srst_st      : 1   ; /* [14]  */
        unsigned int    gmac3_rx_srst_st      : 1   ; /* [15]  */
        unsigned int    gmac4_rx_srst_st      : 1   ; /* [16]  */
        unsigned int    gmac5_rx_srst_st      : 1   ; /* [17]  */
        unsigned int    gmac0_cfg_srst_st     : 1   ; /* [18]  */
        unsigned int    gmac1_cfg_srst_st     : 1   ; /* [19]  */
        unsigned int    gmac2_cfg_srst_st     : 1   ; /* [20]  */
        unsigned int    gmac3_cfg_srst_st     : 1   ; /* [21]  */
        unsigned int    gmac4_cfg_srst_st     : 1   ; /* [22]  */
        unsigned int    gmac5_cfg_srst_st     : 1   ; /* [23]  */
        unsigned int    gmac0_eth_srst_st     : 1   ; /* [24]  */
        unsigned int    gmac1_eth_srst_st     : 1   ; /* [25]  */
        unsigned int    gmac2_eth_srst_st     : 1   ; /* [26]  */
        unsigned int    gmac3_eth_srst_st     : 1   ; /* [27]  */
        unsigned int    gmac4_eth_srst_st     : 1   ; /* [28]  */
        unsigned int    gmac5_eth_srst_st     : 1   ; /* [29]  */
        unsigned int    reserved_0            : 2   ; /* [31..30]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_ST0_U;

/* Define the union U_SC_GE_RESET_ST1_U */
/* GEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷1 */
/* 0x5A10 */
typedef union tagScGeResetSt1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gmac0_fifo_srst_st    : 1   ; /* [0]  */
        unsigned int    gmac1_fifo_srst_st    : 1   ; /* [1]  */
        unsigned int    gmac2_fifo_srst_st    : 1   ; /* [2]  */
        unsigned int    gmac3_fifo_srst_st    : 1   ; /* [3]  */
        unsigned int    gmac4_fifo_srst_st    : 1   ; /* [4]  */
        unsigned int    gmac5_fifo_srst_st    : 1   ; /* [5]  */
        unsigned int    gmac6_sys_srst_st     : 1   ; /* [6]  */
        unsigned int    gmac7_sys_srst_st     : 1   ; /* [7]  */
        unsigned int    gmac6_tx_srst_st      : 1   ; /* [8]  */
        unsigned int    gmac7_tx_srst_st      : 1   ; /* [9]  */
        unsigned int    gmac6_rx_srst_st      : 1   ; /* [10]  */
        unsigned int    gmac7_rx_srst_st      : 1   ; /* [11]  */
        unsigned int    gmac6_cfg_srst_st     : 1   ; /* [12]  */
        unsigned int    gmac7_cfg_srst_st     : 1   ; /* [13]  */
        unsigned int    gmac6_eth_srst_st     : 1   ; /* [14]  */
        unsigned int    gmac7_eth_srst_st     : 1   ; /* [15]  */
        unsigned int    gmac6_fifo_srst_st    : 1   ; /* [16]  */
        unsigned int    gmac7_fifo_srst_st    : 1   ; /* [17]  */
        unsigned int    reserved_0            : 14  ; /* [31..18]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_RESET_ST1_U;

/* Define the union U_SC_PPE_RESET_ST_U */
/* PPEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A24 */
typedef union tagScPpeResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ppe0_srst_st          : 1   ; /* [0]  */
        unsigned int    ppe1_srst_st          : 1   ; /* [1]  */
        unsigned int    ppe2_srst_st          : 1   ; /* [2]  */
        unsigned int    ppe3_srst_st          : 1   ; /* [3]  */
        unsigned int    ppe4_srst_st          : 1   ; /* [4]  */
        unsigned int    ppe5_srst_st          : 1   ; /* [5]  */
        unsigned int    ppe6_srst_st          : 1   ; /* [6]  */
        unsigned int    ppe7_srst_st          : 1   ; /* [7]  */
        unsigned int    rcb1_srst_st          : 1   ; /* [8]  */
        unsigned int    rcb2_srst_st          : 1   ; /* [9]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PPE_RESET_ST_U;

/* Define the union U_SC_ROCEE_RESET_ST_U */
/* ROCEEÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A28 */
typedef union tagScRoceeResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rocee_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_ROCEE_RESET_ST_U;

/* Define the union U_SC_CPU_RESET_ST_U */
/* CPUÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A2C */
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

/* Define the union U_SC_SAS_RESET_ST_U */
/* SASÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A30 */
typedef union tagScSasResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sas_srst_st           : 1   ; /* [0]  */
        unsigned int    sas_oob_srst_st       : 1   ; /* [1]  */
        unsigned int    sas_ahb_srst_st       : 1   ; /* [2]  */
        unsigned int    sas_ch0_rx_srst_st    : 1   ; /* [3]  */
        unsigned int    sas_ch1_rx_srst_st    : 1   ; /* [4]  */
        unsigned int    sas_ch2_rx_srst_st    : 1   ; /* [5]  */
        unsigned int    sas_ch3_rx_srst_st    : 1   ; /* [6]  */
        unsigned int    sas_ch4_rx_srst_st    : 1   ; /* [7]  */
        unsigned int    sas_ch5_rx_srst_st    : 1   ; /* [8]  */
        unsigned int    sas_ch6_rx_srst_st    : 1   ; /* [9]  */
        unsigned int    sas_ch7_rx_srst_st    : 1   ; /* [10]  */
        unsigned int    sas_ch0_tx_srst_st    : 1   ; /* [11]  */
        unsigned int    sas_ch1_tx_srst_st    : 1   ; /* [12]  */
        unsigned int    sas_ch2_tx_srst_st    : 1   ; /* [13]  */
        unsigned int    sas_ch3_tx_srst_st    : 1   ; /* [14]  */
        unsigned int    sas_ch4_tx_srst_st    : 1   ; /* [15]  */
        unsigned int    sas_ch5_tx_srst_st    : 1   ; /* [16]  */
        unsigned int    sas_ch6_tx_srst_st    : 1   ; /* [17]  */
        unsigned int    sas_ch7_tx_srst_st    : 1   ; /* [18]  */
        unsigned int    reserved_0            : 13  ; /* [31..19]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SAS_RESET_ST_U;

/* Define the union U_SC_SLLC_TSVRX_RESET_ST_U */
/* ÎªSLLC TSVRXÍ¨µÀÈí¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A34 */
typedef union tagScSllcTsvrxResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc_tsvrx0_srst_st   : 1   ; /* [0]  */
        unsigned int    sllc_tsvrx1_srst_st   : 1   ; /* [1]  */
        unsigned int    sllc_tsvrx2_srst_st   : 1   ; /* [2]  */
        unsigned int    sllc_tsvrx3_srst_st   : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_TSVRX_RESET_ST_U;

/* Define the union U_SC_ITS_RESET_ST_U */
/* ITSÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A38 */
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

/* Define the union U_SC_SRAM_RESET_ST_U */
/* SRAMÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A40 */
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

/* Define the union U_SC_RCB_PPE_COM_RESET_ST_U */
/* RCB PPE¹«¹²Èí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A44 */
typedef union tagScRcbPpeComResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    rcb_ppe_com_srst_st   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_RCB_PPE_COM_RESET_ST_U;

/* Define the union U_SC_TIMER_RESET_ST_U */
/* TIMER¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A4C */
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

/* Define the union U_SC_SLLC_RESET_ST_U */
/* SLLC1¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A50 */
typedef union tagScSllcResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    sllc_srst_st          : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC_RESET_ST_U;

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

/* Define the union U_SC_TCAM_MBIST_ST_U */
/* DSAFÖÐTCAM MBIST²âÊÔ½á¹û×´Ì¬ */
/* 0x6300 */
typedef union tagScTcamMbistSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    tcam_mbist_done       : 1   ; /* [0]  */
        unsigned int    tcam_mbist_err_cmp    : 1   ; /* [1]  */
        unsigned int    tcam_mbist_err_mem    : 1   ; /* [2]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_TCAM_MBIST_ST_U;

/* Define the union U_SC_GE_ST_U */
/* DSAFÖÐGEµÄ¹¤×÷×´Ì¬Ö¸Ê¾ */
/* 0x6380 */
typedef union tagScGeSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    gmac_led_activity_0   : 1   ; /* [0]  */
        unsigned int    gmac_led_activity_1   : 1   ; /* [1]  */
        unsigned int    gmac_led_activity_2   : 1   ; /* [2]  */
        unsigned int    gmac_led_activity_3   : 1   ; /* [3]  */
        unsigned int    gmac_led_activity_4   : 1   ; /* [4]  */
        unsigned int    gmac_led_activity_5   : 1   ; /* [5]  */
        unsigned int    gmac_led_activity_6   : 1   ; /* [6]  */
        unsigned int    gmac_led_activity_7   : 1   ; /* [7]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_GE_ST_U;

/* Define the union U_SC_XGE_ST_U */
/* DSAFÖÐXGEµÄ¹¤×÷×´Ì¬Ö¸Ê¾ */
/* 0x6390 */
typedef union tagScXgeSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    xge_app_rx_link_up_0  : 1   ; /* [0]  */
        unsigned int    xge_app_rx_link_up_1  : 1   ; /* [1]  */
        unsigned int    xge_app_rx_link_up_2  : 1   ; /* [2]  */
        unsigned int    xge_app_rx_link_up_3  : 1   ; /* [3]  */
        unsigned int    xge_app_rx_link_up_4  : 1   ; /* [4]  */
        unsigned int    xge_app_rx_link_up_5  : 1   ; /* [5]  */
        unsigned int    reserved_0            : 26  ; /* [31..6]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_XGE_ST_U;

/* Define the union U_SC_HILINK2_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6400 */
typedef union tagScHilink2MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_plloutoflock  : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK2_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6404 */
typedef union tagScHilink2MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_prbs_err      : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK2_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6408 */
typedef union tagScHilink2MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_los           : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MACRO_LOS_U;

/* Define the union U_SC_HILINK3_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6500 */
typedef union tagScHilink3MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_plloutoflock  : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK3_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6504 */
typedef union tagScHilink3MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_prbs_err      : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK3_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6508 */
typedef union tagScHilink3MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink3_los           : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK3_MACRO_LOS_U;

/* Define the union U_SC_HILINK4_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6600 */
typedef union tagScHilink4MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_plloutoflock  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK4_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6604 */
typedef union tagScHilink4MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_prbs_err      : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK4_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6608 */
typedef union tagScHilink4MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink4_los           : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK4_MACRO_LOS_U;

#define ST_READ_CNT 3000
#define NDELAY_TIME 20
#define NIC_RESET_ERROR 1

#endif
#endif

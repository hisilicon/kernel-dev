/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpaISWHqOHbuE99WnZz8Urp0KNb0WWl+qRa4EjbbH6IvvJ8uAZ7aN4Vd0NGho
uLjlYnPNu0lv5wj+FqeUtcjpbejxHD/ZYUB/i8fzto2W9jInosiH8dQXLf1YUUx8YifVEH2L
5m3bevEyzNGqTd8Am3Ff2jPOmPo7kQwYeorRtn65PiBAlpdEFjeeQQB6R2Pmsg==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  °æÈ¨ËùÓÐ (C), 2011-2021, »ªÎª¼¼ÊõÓÐÏÞ¹«Ë¾

 ******************************************************************************
  ÎÄ ¼þ Ãû   : sre_sub_peri.h
  °æ ±¾ ºÅ   : ³õ¸å
  ×÷    Õß   : z00176027
  Éú³ÉÈÕÆÚ   : 2013Äê10ÔÂ17ÈÕ
  ×î½üÐÞ¸Ä   :
  ¹¦ÄÜÃèÊö   : p660 pcie ×ÓÏµÍ³¸´Î»¼°½â¸´Î»Ïà¹ØÍ·ÎÄ¼þ
  º¯ÊýÁÐ±í   :
  ÐÞ¸ÄÀúÊ·   :
  1.ÈÕ    ÆÚ   : 2013Äê10ÔÂ17ÈÕ
    ×÷    Õß   : z00176027
    ÐÞ¸ÄÄÚÈÝ   : ´´½¨ÎÄ¼þ

******************************************************************************/
#ifndef __C_SRE_SUB_PCIE_H__
#define __C_SRE_SUB_PCIE_H__


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


#define PCIE_SUB_BASE                             (0xB0000000)
#define PCIE_SUB_BASE_SIZE                        (0x10000)

/******************************************************************************/
/*                      PHOSPHOR PCIE_SUB Registers' Definitions              */
/******************************************************************************/

#define PCIE_SUB_SC_PCIE0_CLK_EN_REG                   (PCIE_SUB_BASE + 0x300)  /* PCIE0Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE0_CLK_DIS_REG                  (PCIE_SUB_BASE + 0x304)  /* PCIE0Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE1_CLK_EN_REG                   (PCIE_SUB_BASE + 0x308)  /* PCIE1Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE1_CLK_DIS_REG                  (PCIE_SUB_BASE + 0x30C)  /* PCIE1Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE2_CLK_EN_REG                   (PCIE_SUB_BASE + 0x310)  /* PCIE2Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE2_CLK_DIS_REG                  (PCIE_SUB_BASE + 0x314)  /* PCIE2Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SAS_CLK_EN_REG                     (PCIE_SUB_BASE + 0x318)  /* SASÊ±ÖÓÃÅ¿ØÐÅºÅ */
#define PCIE_SUB_SC_SAS_CLK_DIS_REG                    (PCIE_SUB_BASE + 0x31C)  /* SASÊ±ÖÓÃÅ¿ØÐÅºÅ */
#define PCIE_SUB_SC_PCIE3_CLK_EN_REG                   (PCIE_SUB_BASE + 0x320)  /* PCIE3Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE3_CLK_DIS_REG                  (PCIE_SUB_BASE + 0x324)  /* PCIE3Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PCIE_SUB_SC_ITS_CLK_EN_REG                     (PCIE_SUB_BASE + 0x328)  /* ITSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_ITS_CLK_DIS_REG                    (PCIE_SUB_BASE + 0x32C)  /* ITSÊ±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SLLC_CLK_EN_REG                    (PCIE_SUB_BASE + 0x360)  /* SLLC0Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SLLC_CLK_DIS_REG                   (PCIE_SUB_BASE + 0x364)  /* SLLC0Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE0_RESET_REQ_REG                (PCIE_SUB_BASE + 0xA00)  /* PCIE0Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE0_RESET_DREQ_REG               (PCIE_SUB_BASE + 0xA04)  /* PCIE0Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE1_RESET_REQ_REG                (PCIE_SUB_BASE + 0xA08)  /* PCIE1Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE1_RESET_DREQ_REG               (PCIE_SUB_BASE + 0xA0C)  /* PCIE1Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE2_RESET_REQ_REG                (PCIE_SUB_BASE + 0xA10)  /* PCIE2Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE2_RESET_DREQ_REG               (PCIE_SUB_BASE + 0xA14)  /* PCIE2Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SAS_RESET_REQ_REG                  (PCIE_SUB_BASE + 0xA18)  /* SASÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SAS_RESET_DREQ_REG                 (PCIE_SUB_BASE + 0xA1C)  /* SASÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_MCTP0_RESET_REQ_REG                (PCIE_SUB_BASE + 0xA20)  /* MCTP0Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
#define PCIE_SUB_SC_MCTP0_RESET_DREQ_REG               (PCIE_SUB_BASE + 0xA24)  /* MCTP0Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
#define PCIE_SUB_SC_MCTP1_RESET_REQ_REG                (PCIE_SUB_BASE + 0xA28)  /* MCTP1Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
#define PCIE_SUB_SC_MCTP1_RESET_DREQ_REG               (PCIE_SUB_BASE + 0xA2C)  /* MCTP1Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
#define PCIE_SUB_SC_MCTP2_RESET_REQ_REG                (PCIE_SUB_BASE + 0xA30)  /* MCTP2Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
#define PCIE_SUB_SC_MCTP2_RESET_DREQ_REG               (PCIE_SUB_BASE + 0xA34)  /* MCTP2Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
#define PCIE_SUB_SC_SLLC_TSVRX_RESET_REQ_REG           (PCIE_SUB_BASE + 0xA58)  /* SLLC TSVRXÍ¨µÀÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SLLC_TSVRX_RESET_DREQ_REG          (PCIE_SUB_BASE + 0xA5C)  /* SLLC TSVRXÍ¨µÀÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE_HILINK_PCS_RESET_REQ_REG      (PCIE_SUB_BASE + 0xA60)  /* PCIE HILINK PCSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE_HILINK_PCS_RESET_DREQ_REG     (PCIE_SUB_BASE + 0xA64)  /* PCIE HILINK PCSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE3_RESET_REQ_REG                (PCIE_SUB_BASE + 0xA68)  /* PCIE3Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE3_RESET_DREQ_REG               (PCIE_SUB_BASE + 0xA6C)  /* PCIE3Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_MCTP3_RESET_REQ_REG                (PCIE_SUB_BASE + 0xA70)  /* MCTP3Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_MCTP3_RESET_DREQ_REG               (PCIE_SUB_BASE + 0xA74)  /* MCTP3Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_ITS_RESET_REQ_REG                  (PCIE_SUB_BASE + 0xA80)  /* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_ITS_RESET_DREQ_REG                 (PCIE_SUB_BASE + 0xA84)  /* ITSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SLLC_RESET_REQ_REG                 (PCIE_SUB_BASE + 0xAA0)  /* SLLC0Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SLLC_RESET_DREQ_REG                (PCIE_SUB_BASE + 0xAA4)  /* SLLC0Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCS_LOCAL_RESET_REQ_REG            (PCIE_SUB_BASE + 0xAC0)  /* PCS LOCALÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCS_LOCAL_RESET_DREQ_REG           (PCIE_SUB_BASE + 0xAC4)  /* PCS LOCALÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_DISP_DAW_EN_REG                    (PCIE_SUB_BASE + 0x1000) /* dispatch daw enÅäÖÃ */
#define PCIE_SUB_SC_DISPATCH_DAW_ARRAY0_REG            (PCIE_SUB_BASE + 0x1004) /* dispatch dawÅäÖÃÕóÁÐ0 */
#define PCIE_SUB_SC_DISPATCH_DAW_ARRAY1_REG            (PCIE_SUB_BASE + 0x1008) /* dispatch dawÅäÖÃÕóÁÐ1 */
#define PCIE_SUB_SC_DISPATCH_DAW_ARRAY2_REG            (PCIE_SUB_BASE + 0x100C) /* dispatch dawÅäÖÃÕóÁÐ2 */
#define PCIE_SUB_SC_DISPATCH_DAW_ARRAY3_REG            (PCIE_SUB_BASE + 0x1010) /* dispatch dawÅäÖÃÕóÁÐ3 */
#define PCIE_SUB_SC_DISPATCH_DAW_ARRAY4_REG            (PCIE_SUB_BASE + 0x1014) /* dispatch dawÅäÖÃÕóÁÐ4 */
#define PCIE_SUB_SC_DISPATCH_DAW_ARRAY5_REG            (PCIE_SUB_BASE + 0x1018) /* dispatch dawÅäÖÃÕóÁÐ5 */
#define PCIE_SUB_SC_DISPATCH_DAW_ARRAY6_REG            (PCIE_SUB_BASE + 0x101C) /* dispatch dawÅäÖÃÕóÁÐ6 */
#define PCIE_SUB_SC_DISPATCH_DAW_ARRAY7_REG            (PCIE_SUB_BASE + 0x1020) /* dispatch dawÅäÖÃÕóÁÐ7 */
#define PCIE_SUB_SC_DISPATCH_RETRY_CONTROL_REG         (PCIE_SUB_BASE + 0x1030) /* dispatch retry¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_DISPATCH_INTMASK_REG               (PCIE_SUB_BASE + 0x1100) /* dispatchµÄÖÐ¶ÏÆÁ±Î¼Ä´æÆ÷ */
#define PCIE_SUB_SC_DISPATCH_RAWINT_REG                (PCIE_SUB_BASE + 0x1104) /* dispatchµÄÔ­Ê¼ÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_DISPATCH_INTSTAT_REG               (PCIE_SUB_BASE + 0x1108) /* dispatchµÄÆÁ±ÎºóµÄÖÐ¶Ï×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_DISPATCH_INTCLR_REG                (PCIE_SUB_BASE + 0x110C) /* dispatchµÄÖÐ¶ÏÇå³ý¼Ä´æÆ÷ */
#define PCIE_SUB_SC_DISPATCH_ERRSTAT_REG               (PCIE_SUB_BASE + 0x1110) /* dispatchµÄERR×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_REMAP_CTRL_REG                     (PCIE_SUB_BASE + 0x1200) /* subsysµÄÆô¶¯Remap¼Ä´æÆ÷ */
#define PCIE_SUB_SC_FTE_MUX_CTRL_REG                   (PCIE_SUB_BASE + 0x2200)
#define PCIE_SUB_SC_HILINK0_MUX_CTRL_REG               (PCIE_SUB_BASE + 0x2300) /* HILINK¸´ÓÃÑ¡Ôñ */
#define PCIE_SUB_SC_HILINK1_MUX_CTRL_REG               (PCIE_SUB_BASE + 0x2304) /* HILINK¸´ÓÃÑ¡Ôñ */
#define PCIE_SUB_SC_HILINK2_MUX_CTRL_REG               (PCIE_SUB_BASE + 0x2308) /* HILINK¸´ÓÃÑ¡Ôñ */
#define PCIE_SUB_SC_HILINK5_MUX_CTRL_REG               (PCIE_SUB_BASE + 0x2314) /* HILINK¸´ÓÃÑ¡Ôñ */
#define PCIE_SUB_SC_HILINK1_AHB_MUX_CTRL_REG           (PCIE_SUB_BASE + 0x2324) /* HILINK AHB¸´ÓÃÑ¡Ôñ */
#define PCIE_SUB_SC_HILINK2_AHB_MUX_CTRL_REG           (PCIE_SUB_BASE + 0x2328) /* HILINK AHB¸´ÓÃÑ¡Ôñ */
#define PCIE_SUB_SC_HILINK5_AHB_MUX_CTRL_REG           (PCIE_SUB_BASE + 0x2334) /* HILINK AHB¸´ÓÃÑ¡Ôñ */
#define PCIE_SUB_SC_HILINK5_LRSTB_MUX_CTRL_REG         (PCIE_SUB_BASE + 0x2340) /* HILINK5 lrstb[3:0]µÄMUXÑ¡Ôñ¿ØÖÆ */
#define PCIE_SUB_SC_HILINK6_LRSTB_MUX_CTRL_REG         (PCIE_SUB_BASE + 0x2344) /* HILINK6 lrstb[3:0]µÄMUXÑ¡Ôñ¿ØÖÆ */
#define PCIE_SUB_SC_HILINK0_MACRO_SS_REFCLK_REG        (PCIE_SUB_BASE + 0x2400) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_CS_REFCLK_DIRSEL_REG (PCIE_SUB_BASE + 0x2404) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_LIFECLK2DIG_SEL_REG  (PCIE_SUB_BASE + 0x2408) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_CORE_CLK_SELEXT_REG  (PCIE_SUB_BASE + 0x240C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_CORE_CLK_SEL_REG     (PCIE_SUB_BASE + 0x2410) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_CTRL_BUS_MODE_REG    (PCIE_SUB_BASE + 0x2414) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_MACROPWRDB_REG       (PCIE_SUB_BASE + 0x2418) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_GRSTB_REG            (PCIE_SUB_BASE + 0x241C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_BIT_SLIP_REG         (PCIE_SUB_BASE + 0x2420) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_LRSTB_REG            (PCIE_SUB_BASE + 0x2424) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_SS_REFCLK_REG        (PCIE_SUB_BASE + 0x2500) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_CS_REFCLK_DIRSEL_REG (PCIE_SUB_BASE + 0x2504) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_LIFECLK2DIG_SEL_REG  (PCIE_SUB_BASE + 0x2508) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_CORE_CLK_SELEXT_REG  (PCIE_SUB_BASE + 0x250C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_CORE_CLK_SEL_REG     (PCIE_SUB_BASE + 0x2510) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_CTRL_BUS_MODE_REG    (PCIE_SUB_BASE + 0x2514) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_MACROPWRDB_REG       (PCIE_SUB_BASE + 0x2518) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_GRSTB_REG            (PCIE_SUB_BASE + 0x251C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_BIT_SLIP_REG         (PCIE_SUB_BASE + 0x2520) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_LRSTB_REG            (PCIE_SUB_BASE + 0x2524) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_SS_REFCLK_REG        (PCIE_SUB_BASE + 0x2600) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_CS_REFCLK_DIRSEL_REG (PCIE_SUB_BASE + 0x2604) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_LIFECLK2DIG_SEL_REG  (PCIE_SUB_BASE + 0x2608) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_CORE_CLK_SELEXT_REG  (PCIE_SUB_BASE + 0x260C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_CORE_CLK_SEL_REG     (PCIE_SUB_BASE + 0x2610) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_CTRL_BUS_MODE_REG    (PCIE_SUB_BASE + 0x2614) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_MACROPWRDB_REG       (PCIE_SUB_BASE + 0x2618) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_GRSTB_REG            (PCIE_SUB_BASE + 0x261C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_BIT_SLIP_REG         (PCIE_SUB_BASE + 0x2620) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_LRSTB_REG            (PCIE_SUB_BASE + 0x2624) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_SS_REFCLK_REG        (PCIE_SUB_BASE + 0x2700) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_CS_REFCLK_DIRSEL_REG (PCIE_SUB_BASE + 0x2704) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_LIFECLK2DIG_SEL_REG  (PCIE_SUB_BASE + 0x2708) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_CORE_CLK_SELEXT_REG  (PCIE_SUB_BASE + 0x270C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_CORE_CLK_SEL_REG     (PCIE_SUB_BASE + 0x2710) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_CTRL_BUS_MODE_REG    (PCIE_SUB_BASE + 0x2714) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_MACROPWRDB_REG       (PCIE_SUB_BASE + 0x2718) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_GRSTB_REG            (PCIE_SUB_BASE + 0x271C) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_BIT_SLIP_REG         (PCIE_SUB_BASE + 0x2720) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_LRSTB_REG            (PCIE_SUB_BASE + 0x2724) /* HILINKÅäÖÃ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE0_CLKREQ_REG                   (PCIE_SUB_BASE + 0x2800)
#define PCIE_SUB_SC_PCIE0_AXI_MSTR_OOO_WR_CFG_REG      (PCIE_SUB_BASE + 0x2880)
#define PCIE_SUB_SC_PCIE0_AXI_MSTR_OOO_RD_CFG_REG      (PCIE_SUB_BASE + 0x2890)
#define PCIE_SUB_SC_PCIE1_CLKREQ_REG                   (PCIE_SUB_BASE + 0x2900)
#define PCIE_SUB_SC_PCIE1_AXI_MSTR_OOO_WR_CFG_REG      (PCIE_SUB_BASE + 0x2980)
#define PCIE_SUB_SC_PCIE1_AXI_MSTR_OOO_RD_CFG_REG      (PCIE_SUB_BASE + 0x2990)
#define PCIE_SUB_SC_PCIE2_CLKREQ_REG                   (PCIE_SUB_BASE + 0x2A00)
#define PCIE_SUB_SC_PCIE2_AXI_MSTR_OOO_WR_CFG_REG      (PCIE_SUB_BASE + 0x2A80)
#define PCIE_SUB_SC_PCIE2_AXI_MSTR_OOO_RD_CFG_REG      (PCIE_SUB_BASE + 0x2A90)
#define PCIE_SUB_SC_PCIE3_CLKREQ_REG                   (PCIE_SUB_BASE + 0x2B00)
#define PCIE_SUB_SC_SMMU_MEM_CTRL0_REG                 (PCIE_SUB_BASE + 0x3000) /* smmu mem¿ØÖÆ¼Ä´æÆ÷0 */
#define PCIE_SUB_SC_SMMU_MEM_CTRL1_REG                 (PCIE_SUB_BASE + 0x3004) /* smmu mem¿ØÖÆ¼Ä´æÆ÷1 */
#define PCIE_SUB_SC_SMMU_MEM_CTRL2_REG                 (PCIE_SUB_BASE + 0x3008) /* smmu mem¿ØÖÆ¼Ä´æÆ÷2 */
#define PCIE_SUB_SC_SLLC0_MEM_CTRL_REG                 (PCIE_SUB_BASE + 0x3010) /* sllc0 mem¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SAS_MEM_CTRL_REG                   (PCIE_SUB_BASE + 0x3030) /* sas mem¿ØÖÆ¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE_MEM_CTRL0_REG                 (PCIE_SUB_BASE + 0x3040) /* pcie mem¿ØÖÆ¼Ä´æÆ÷0 */
#define PCIE_SUB_SC_PCIE_MEM_CTRL1_REG                 (PCIE_SUB_BASE + 0x3044) /* pcie mem¿ØÖÆ¼Ä´æÆ÷1 */
#define PCIE_SUB_SC_PCIE_MEM_CTRL2_REG                 (PCIE_SUB_BASE + 0x3048) /* pcie mem¿ØÖÆ¼Ä´æÆ÷2 */
#define PCIE_SUB_SC_SKEW_COMMON_0_REG                  (PCIE_SUB_BASE + 0x3400)
#define PCIE_SUB_SC_SKEW_COMMON_1_REG                  (PCIE_SUB_BASE + 0x3404)
#define PCIE_SUB_SC_SKEW_COMMON_2_REG                  (PCIE_SUB_BASE + 0x3408)
#define PCIE_SUB_SC_SKEW_A_0_REG                       (PCIE_SUB_BASE + 0x3500)
#define PCIE_SUB_SC_SKEW_A_1_REG                       (PCIE_SUB_BASE + 0x3504)
#define PCIE_SUB_SC_SKEW_A_2_REG                       (PCIE_SUB_BASE + 0x3508)
#define PCIE_SUB_SC_SKEW_A_3_REG                       (PCIE_SUB_BASE + 0x350C)
#define PCIE_SUB_SC_SKEW_A_4_REG                       (PCIE_SUB_BASE + 0x3510)
#define PCIE_SUB_SC_SKEW_A_5_REG                       (PCIE_SUB_BASE + 0x3514)
#define PCIE_SUB_SC_SKEW_A_6_REG                       (PCIE_SUB_BASE + 0x3518)
#define PCIE_SUB_SC_SKEW_A_7_REG                       (PCIE_SUB_BASE + 0x351C)
#define PCIE_SUB_SC_SKEW_A_8_REG                       (PCIE_SUB_BASE + 0x3520)
#define PCIE_SUB_SC_SKEW_B_0_REG                       (PCIE_SUB_BASE + 0x3600)
#define PCIE_SUB_SC_SKEW_B_1_REG                       (PCIE_SUB_BASE + 0x3604)
#define PCIE_SUB_SC_SKEW_B_2_REG                       (PCIE_SUB_BASE + 0x3608)
#define PCIE_SUB_SC_SKEW_B_3_REG                       (PCIE_SUB_BASE + 0x360C)
#define PCIE_SUB_SC_SKEW_B_4_REG                       (PCIE_SUB_BASE + 0x3610)
#define PCIE_SUB_SC_SKEW_B_5_REG                       (PCIE_SUB_BASE + 0x3614)
#define PCIE_SUB_SC_SKEW_B_6_REG                       (PCIE_SUB_BASE + 0x3618)
#define PCIE_SUB_SC_SKEW_B_7_REG                       (PCIE_SUB_BASE + 0x361C)
#define PCIE_SUB_SC_SKEW_B_8_REG                       (PCIE_SUB_BASE + 0x3620)
#define PCIE_SUB_SC_PCIE0_CLK_ST_REG                   (PCIE_SUB_BASE + 0x5300) /* PCIE0Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE1_CLK_ST_REG                   (PCIE_SUB_BASE + 0x5304) /* PCIE1Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE2_CLK_ST_REG                   (PCIE_SUB_BASE + 0x5308) /* PCIE2Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SAS_CLK_ST_REG                     (PCIE_SUB_BASE + 0x530C) /* SASÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE3_CLK_ST_REG                   (PCIE_SUB_BASE + 0x5310) /* PCIE3Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_ITS_CLK_ST_REG                     (PCIE_SUB_BASE + 0x5314) /* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SLLC_CLK_ST_REG                    (PCIE_SUB_BASE + 0x5330) /* SLLC0Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE0_RESET_ST_REG                 (PCIE_SUB_BASE + 0x5A00) /* PCIE0¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE1_RESET_ST_REG                 (PCIE_SUB_BASE + 0x5A04) /* PCIE1¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE2_RESET_ST_REG                 (PCIE_SUB_BASE + 0x5A08) /* PCIE2¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SAS_RESET_ST_REG                   (PCIE_SUB_BASE + 0x5A0C) /* SAS¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_MCTP0_RESET_ST_REG                 (PCIE_SUB_BASE + 0x5A10) /* MCTP0¸´Î»×´Ì¬¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
#define PCIE_SUB_SC_MCTP1_RESET_ST_REG                 (PCIE_SUB_BASE + 0x5A14) /* MCTP1¸´Î»×´Ì¬¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
#define PCIE_SUB_SC_MCTP2_RESET_ST_REG                 (PCIE_SUB_BASE + 0x5A18) /* MCTP2¸´Î»×´Ì¬¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
#define PCIE_SUB_SC_SLLC_TSVRX_RESET_ST_REG            (PCIE_SUB_BASE + 0x5A2C) /* ÎªSLLC TSVRXÍ¨µÀÈí¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE_HILINK_PCS_RESET_ST_REG       (PCIE_SUB_BASE + 0x5A30) /* PCIE HILINK PCS¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE3_RESET_ST_REG                 (PCIE_SUB_BASE + 0x5A34) /* PCIE3¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_MCTP3_RESET_ST_REG                 (PCIE_SUB_BASE + 0x5A38) /* MCTP3¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_ITS_RESET_ST_REG                   (PCIE_SUB_BASE + 0x5A40) /* ITS¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_SLLC_RESET_ST_REG                  (PCIE_SUB_BASE + 0x5A50) /* SLLC0¸´Î»×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCS_LOCAL_RESET_ST_REG             (PCIE_SUB_BASE + 0x5A60) /* PCS LOCALÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_PLLOUTOFLOCK_REG     (PCIE_SUB_BASE + 0x6400) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_PRBS_ERR_REG         (PCIE_SUB_BASE + 0x6404) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK0_MACRO_LOS_REG              (PCIE_SUB_BASE + 0x6408) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_PLLOUTOFLOCK_REG     (PCIE_SUB_BASE + 0x6500) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_PRBS_ERR_REG         (PCIE_SUB_BASE + 0x6504) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK1_MACRO_LOS_REG              (PCIE_SUB_BASE + 0x6508) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_PLLOUTOFLOCK_REG     (PCIE_SUB_BASE + 0x6600) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_PRBS_ERR_REG         (PCIE_SUB_BASE + 0x6604) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK5_MACRO_LOS_REG              (PCIE_SUB_BASE + 0x6608) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_PLLOUTOFLOCK_REG     (PCIE_SUB_BASE + 0x6700) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_PRBS_ERR_REG         (PCIE_SUB_BASE + 0x6704) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_HILINK6_MACRO_LOS_REG              (PCIE_SUB_BASE + 0x6708) /* HILINK×´Ì¬¼Ä´æÆ÷ */
#define PCIE_SUB_SC_PCIE0_RXEQINPRO_STAT_REG           (PCIE_SUB_BASE + 0x6800)
#define PCIE_SUB_SC_PCIE0_LINKINT_RCVRY_STAT_REG       (PCIE_SUB_BASE + 0x6804)
#define PCIE_SUB_SC_PCIE0_SYS_STATE0_REG               (PCIE_SUB_BASE + 0x6808) /* pcie0×´Ì¬¼Ä´æÆ÷0 */
#define PCIE_SUB_SC_PCIE0_SYS_STATE1_REG               (PCIE_SUB_BASE + 0x680C) /* pcie0×´Ì¬¼Ä´æÆ÷1 */
#define PCIE_SUB_SC_PCIE0_SYS_STATE2_REG               (PCIE_SUB_BASE + 0x6810) /* pcie0×´Ì¬¼Ä´æÆ÷2 */
#define PCIE_SUB_SC_PCIE0_SYS_STATE3_REG               (PCIE_SUB_BASE + 0x6814) /* pcie0×´Ì¬¼Ä´æÆ÷3 */
#define PCIE_SUB_SC_PCIE0_SYS_STATE4_REG               (PCIE_SUB_BASE + 0x6818) /* pcie0×´Ì¬¼Ä´æÆ÷4 */
#define PCIE_SUB_SC_PCIE0_AXI_MSTR_OOO_WR_STS0_REG     (PCIE_SUB_BASE + 0x6880) /* Ð´·½Ïò¸÷¸öoutstanding transaction IDµÄ¿ÕÏÐ×´Ì¬ */
#define PCIE_SUB_SC_PCIE0_AXI_MSTR_OOO_WR_STS1_REG     (PCIE_SUB_BASE + 0x6884) /* OOOÐ´Í¨µÀ¹¤×÷×´Ì¬ */
#define PCIE_SUB_SC_PCIE0_AXI_MSTR_OOO_RD_STS0_REG     (PCIE_SUB_BASE + 0x6890) /* ¶Á·½Ïò¸÷¸öoutstanding transaction IDµÄ¿ÕÏÐ×´Ì¬ */
#define PCIE_SUB_SC_PCIE0_AXI_MSTR_OOO_RD_STS1_REG     (PCIE_SUB_BASE + 0x6894) /* OOO¶ÁÍ¨µÀ¹¤×÷×´Ì¬ */
#define PCIE_SUB_SC_PCIE0_DSIZE_BRG_ECC_ERR_REG        (PCIE_SUB_BASE + 0x68A0) /* PCIE¶ÔÓ¦DSIZE BRGµÄECC´íÎó×´Ì¬ */
#define PCIE_SUB_SC_PCIE0_PCIEPHY_CTRL_ERROR_REG       (PCIE_SUB_BASE + 0x68C0) /* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
#define PCIE_SUB_SC_PCIE1_RXEQINPRO_STAT_REG           (PCIE_SUB_BASE + 0x6900)
#define PCIE_SUB_SC_PCIE1_LINKINT_RCVRY_STAT_REG       (PCIE_SUB_BASE + 0x6904)
#define PCIE_SUB_SC_PCIE1_SYS_STATE0_REG               (PCIE_SUB_BASE + 0x6908) /* pcie1×´Ì¬¼Ä´æÆ÷0 */
#define PCIE_SUB_SC_PCIE1_SYS_STATE1_REG               (PCIE_SUB_BASE + 0x690C) /* pcie1×´Ì¬¼Ä´æÆ÷1 */
#define PCIE_SUB_SC_PCIE1_SYS_STATE2_REG               (PCIE_SUB_BASE + 0x6910) /* pcie1×´Ì¬¼Ä´æÆ÷2 */
#define PCIE_SUB_SC_PCIE1_SYS_STATE3_REG               (PCIE_SUB_BASE + 0x6914) /* pcie1×´Ì¬¼Ä´æÆ÷3 */
#define PCIE_SUB_SC_PCIE1_SYS_STATE4_REG               (PCIE_SUB_BASE + 0x6918) /* pcie1×´Ì¬¼Ä´æÆ÷4 */
#define PCIE_SUB_SC_PCIE1_AXI_MSTR_OOO_WR_STS0_REG     (PCIE_SUB_BASE + 0x6980) /* Ð´·½Ïò¸÷¸öoutstanding transaction IDµÄ¿ÕÏÐ×´Ì¬ */
#define PCIE_SUB_SC_PCIE1_AXI_MSTR_OOO_WR_STS1_REG     (PCIE_SUB_BASE + 0x6984) /* OOOÐ´Í¨µÀ¹¤×÷×´Ì¬ */
#define PCIE_SUB_SC_PCIE1_AXI_MSTR_OOO_RD_STS0_REG     (PCIE_SUB_BASE + 0x6990) /* ¶Á·½Ïò¸÷¸öoutstanding transaction IDµÄ¿ÕÏÐ×´Ì¬ */
#define PCIE_SUB_SC_PCIE1_AXI_MSTR_OOO_RD_STS1_REG     (PCIE_SUB_BASE + 0x6994) /* OOO¶ÁÍ¨µÀ¹¤×÷×´Ì¬ */
#define PCIE_SUB_SC_PCIE1_DSIZE_BRG_ECC_ERR_REG        (PCIE_SUB_BASE + 0x69A0) /* PCIE¶ÔÓ¦DSIZE BRGµÄECC´íÎó×´Ì¬ */
#define PCIE_SUB_SC_PCIE1_PCIEPHY_CTRL_ERROR_REG       (PCIE_SUB_BASE + 0x69C0) /* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
#define PCIE_SUB_SC_PCIE2_RXEQINPRO_STAT_REG           (PCIE_SUB_BASE + 0x6A00)
#define PCIE_SUB_SC_PCIE2_LINKINT_RCVRY_STAT_REG       (PCIE_SUB_BASE + 0x6A04)
#define PCIE_SUB_SC_PCIE2_SYS_STATE0_REG               (PCIE_SUB_BASE + 0x6A08) /* pcie2×´Ì¬¼Ä´æÆ÷0 */
#define PCIE_SUB_SC_PCIE2_SYS_STATE1_REG               (PCIE_SUB_BASE + 0x6A0C) /* pcie2×´Ì¬¼Ä´æÆ÷1 */
#define PCIE_SUB_SC_PCIE2_SYS_STATE2_REG               (PCIE_SUB_BASE + 0x6A10) /* pcie2×´Ì¬¼Ä´æÆ÷2 */
#define PCIE_SUB_SC_PCIE2_SYS_STATE3_REG               (PCIE_SUB_BASE + 0x6A14) /* pcie2×´Ì¬¼Ä´æÆ÷3 */
#define PCIE_SUB_SC_PCIE2_SYS_STATE4_REG               (PCIE_SUB_BASE + 0x6A18) /* pcie2×´Ì¬¼Ä´æÆ÷4 */
#define PCIE_SUB_SC_PCIE2_AXI_MSTR_OOO_WR_STS0_REG     (PCIE_SUB_BASE + 0x6A80) /* Ð´·½Ïò¸÷¸öoutstanding transaction IDµÄ¿ÕÏÐ×´Ì¬ */
#define PCIE_SUB_SC_PCIE2_AXI_MSTR_OOO_WR_STS1_REG     (PCIE_SUB_BASE + 0x6A84) /* OOOÐ´Í¨µÀ¹¤×÷×´Ì¬ */
#define PCIE_SUB_SC_PCIE2_AXI_MSTR_OOO_RD_STS0_REG     (PCIE_SUB_BASE + 0x6A90) /* ¶Á·½Ïò¸÷¸öoutstanding transaction IDµÄ¿ÕÏÐ×´Ì¬ */
#define PCIE_SUB_SC_PCIE2_AXI_MSTR_OOO_RD_STS1_REG     (PCIE_SUB_BASE + 0x6A94) /* OOO¶ÁÍ¨µÀ¹¤×÷×´Ì¬ */
#define PCIE_SUB_SC_PCIE2_DSIZE_BRG_ECC_ERR_REG        (PCIE_SUB_BASE + 0x6AA0) /* PCIE¶ÔÓ¦DSIZE BRGµÄECC´íÎó×´Ì¬ */
#define PCIE_SUB_SC_PCIE2_PCIEPHY_CTRL_ERROR_REG       (PCIE_SUB_BASE + 0x6AC0) /* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
#define PCIE_SUB_SC_PCIE3_SYS_STATE0_REG               (PCIE_SUB_BASE + 0x6B08) /* pcie3×´Ì¬¼Ä´æÆ÷0 */
#define PCIE_SUB_SC_PCIE3_SYS_STATE1_REG               (PCIE_SUB_BASE + 0x6B0C) /* pcie3×´Ì¬¼Ä´æÆ÷1 */
#define PCIE_SUB_SC_PCIE3_SYS_STATE2_REG               (PCIE_SUB_BASE + 0x6B10) /* pcie3×´Ì¬¼Ä´æÆ÷2 */
#define PCIE_SUB_SC_PCIE3_SYS_STATE3_REG               (PCIE_SUB_BASE + 0x6B14) /* pcie3×´Ì¬¼Ä´æÆ÷3 */
#define PCIE_SUB_SC_PCIE3_SYS_STATE4_REG               (PCIE_SUB_BASE + 0x6B18) /* pcie3×´Ì¬¼Ä´æÆ÷4 */
#define PCIE_SUB_SC_PCIE3_PCIEPHY_CTRL_ERROR_REG       (PCIE_SUB_BASE + 0x6BC0) /* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
#define PCIE_SUB_SC_SKEW_ST_0_REG                      (PCIE_SUB_BASE + 0x7400)
#define PCIE_SUB_SC_SKEW_ST_A_0_REG                    (PCIE_SUB_BASE + 0x7500)
#define PCIE_SUB_SC_SKEW_ST_A_1_REG                    (PCIE_SUB_BASE + 0x7504)
#define PCIE_SUB_SC_SKEW_ST_A_2_REG                    (PCIE_SUB_BASE + 0x7508)
#define PCIE_SUB_SC_SKEW_ST_A_3_REG                    (PCIE_SUB_BASE + 0x750C)
#define PCIE_SUB_SC_SKEW_ST_B_0_REG                    (PCIE_SUB_BASE + 0x7600)
#define PCIE_SUB_SC_SKEW_ST_B_1_REG                    (PCIE_SUB_BASE + 0x7604)
#define PCIE_SUB_SC_SKEW_ST_B_2_REG                    (PCIE_SUB_BASE + 0x7608)
#define PCIE_SUB_SC_SKEW_ST_B_3_REG                    (PCIE_SUB_BASE + 0x760C)
#define PCIE_SUB_SC_ECO_RSV0_REG                       (PCIE_SUB_BASE + 0x8000) /* ECO ¼Ä´æÆ÷0 */
#define PCIE_SUB_SC_ECO_RSV1_REG                       (PCIE_SUB_BASE + 0x8004) /* ECO ¼Ä´æÆ÷1 */
#define PCIE_SUB_SC_ECO_RSV2_REG                       (PCIE_SUB_BASE + 0x8008) /* ECO ¼Ä´æÆ÷2 */

#if(ENDNESS == ENDNESS_BIG)

/* Define the union U_SC_PCIE0_CLK_EN_U */
/* PCIE0Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x300 */
typedef union tagScPcie0ClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie0_pipe_enb    : 1   ; /* [1]  */
        unsigned int    clk_pcie0_enb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_CLK_EN_U;

/* Define the union U_SC_PCIE0_CLK_DIS_U */
/* PCIE0Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x304 */
typedef union tagScPcie0ClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie0_pipe_dsb    : 1   ; /* [1]  */
        unsigned int    clk_pcie0_dsb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_CLK_DIS_U;

/* Define the union U_SC_PCIE1_CLK_EN_U */
/* PCIE1Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x308 */
typedef union tagScPcie1ClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie1_pipe_enb    : 1   ; /* [1]  */
        unsigned int    clk_pcie1_enb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_CLK_EN_U;

/* Define the union U_SC_PCIE1_CLK_DIS_U */
/* PCIE1Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x30C */
typedef union tagScPcie1ClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie1_pipe_dsb    : 1   ; /* [1]  */
        unsigned int    clk_pcie1_dsb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_CLK_DIS_U;

/* Define the union U_SC_PCIE2_CLK_EN_U */
/* PCIE2Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x310 */
typedef union tagScPcie2ClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie2_pipe_enb    : 1   ; /* [1]  */
        unsigned int    clk_pcie2_enb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_CLK_EN_U;

/* Define the union U_SC_PCIE2_CLK_DIS_U */
/* PCIE2Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x314 */
typedef union tagScPcie2ClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie2_pipe_dsb    : 1   ; /* [1]  */
        unsigned int    clk_pcie2_dsb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_CLK_DIS_U;

/* Define the union U_SC_SAS_CLK_EN_U */
/* SASÊ±ÖÓÃÅ¿ØÐÅºÅ */
/* 0x318 */
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
/* 0x31C */
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

/* Define the union U_SC_PCIE3_CLK_EN_U */
/* PCIE3Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x320 */
typedef union tagScPcie3ClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie3_pipe_enb    : 1   ; /* [1]  */
        unsigned int    clk_pcie3_enb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_CLK_EN_U;

/* Define the union U_SC_PCIE3_CLK_DIS_U */
/* PCIE3Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x324 */
typedef union tagScPcie3ClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie3_pipe_dsb    : 1   ; /* [1]  */
        unsigned int    clk_pcie3_dsb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_CLK_DIS_U;

/* Define the union U_SC_ITS_CLK_EN_U */
/* ITSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x328 */
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
/* 0x32C */
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

/* Define the union U_SC_SLLC_CLK_EN_U */
/* SLLC0Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
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
/* SLLC0Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
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

/* Define the union U_SC_PCIE0_RESET_REQ_U */
/* PCIE0Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA00 */
typedef union tagScPcie0ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie0_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_RESET_REQ_U;

/* Define the union U_SC_PCIE0_RESET_DREQ_U */
/* PCIE0Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA04 */
typedef union tagScPcie0ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie0_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_RESET_DREQ_U;

/* Define the union U_SC_PCIE1_RESET_REQ_U */
/* PCIE1Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA08 */
typedef union tagScPcie1ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie1_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_RESET_REQ_U;

/* Define the union U_SC_PCIE1_RESET_DREQ_U */
/* PCIE1Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA0C */
typedef union tagScPcie1ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie1_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_RESET_DREQ_U;

/* Define the union U_SC_PCIE2_RESET_REQ_U */
/* PCIE2Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA10 */
typedef union tagScPcie2ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie2_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_RESET_REQ_U;

/* Define the union U_SC_PCIE2_RESET_DREQ_U */
/* PCIE2Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA14 */
typedef union tagScPcie2ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie2_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_RESET_DREQ_U;

/* Define the union U_SC_SAS_RESET_REQ_U */
/* SASÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA18 */
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
/* SASÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA1C */
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

/* Define the union U_SC_MCTP0_RESET_REQ_U */
/* MCTP0Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA20 */
typedef union tagScMctp0ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp0_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP0_RESET_REQ_U;

/* Define the union U_SC_MCTP0_RESET_DREQ_U */
/* MCTP0Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA24 */
typedef union tagScMctp0ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp0_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP0_RESET_DREQ_U;

/* Define the union U_SC_MCTP1_RESET_REQ_U */
/* MCTP1Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA28 */
typedef union tagScMctp1ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp1_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP1_RESET_REQ_U;

/* Define the union U_SC_MCTP1_RESET_DREQ_U */
/* MCTP1Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA2C */
typedef union tagScMctp1ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp1_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP1_RESET_DREQ_U;

/* Define the union U_SC_MCTP2_RESET_REQ_U */
/* MCTP2Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA30 */
typedef union tagScMctp2ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp2_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP2_RESET_REQ_U;

/* Define the union U_SC_MCTP2_RESET_DREQ_U */
/* MCTP2Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA34 */
typedef union tagScMctp2ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp2_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP2_RESET_DREQ_U;

/* Define the union U_SC_SLLC_TSVRX_RESET_REQ_U */
/* SLLC TSVRXÍ¨µÀÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA58 */
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
/* 0xA5C */
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

/* Define the union U_SC_PCIE_HILINK_PCS_RESET_REQ_U */
/* PCIE HILINK PCSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA60 */
typedef union tagScPcieHilinkPcsResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_hilink_pcs_lane7_srst_req : 1   ; /* [31]  */
        unsigned int    pcie3_hilink_pcs_lane6_srst_req : 1   ; /* [30]  */
        unsigned int    pcie3_hilink_pcs_lane5_srst_req : 1   ; /* [29]  */
        unsigned int    pcie3_hilink_pcs_lane4_srst_req : 1   ; /* [28]  */
        unsigned int    pcie3_hilink_pcs_lane3_srst_req : 1   ; /* [27]  */
        unsigned int    pcie3_hilink_pcs_lane2_srst_req : 1   ; /* [26]  */
        unsigned int    pcie3_hilink_pcs_lane1_srst_req : 1   ; /* [25]  */
        unsigned int    pcie3_hilink_pcs_lane0_srst_req : 1   ; /* [24]  */
        unsigned int    pcie2_hilink_pcs_lane7_srst_req : 1   ; /* [23]  */
        unsigned int    pcie2_hilink_pcs_lane6_srst_req : 1   ; /* [22]  */
        unsigned int    pcie2_hilink_pcs_lane5_srst_req : 1   ; /* [21]  */
        unsigned int    pcie2_hilink_pcs_lane4_srst_req : 1   ; /* [20]  */
        unsigned int    pcie2_hilink_pcs_lane3_srst_req : 1   ; /* [19]  */
        unsigned int    pcie2_hilink_pcs_lane2_srst_req : 1   ; /* [18]  */
        unsigned int    pcie2_hilink_pcs_lane1_srst_req : 1   ; /* [17]  */
        unsigned int    pcie2_hilink_pcs_lane0_srst_req : 1   ; /* [16]  */
        unsigned int    pcie1_hilink_pcs_lane7_srst_req : 1   ; /* [15]  */
        unsigned int    pcie1_hilink_pcs_lane6_srst_req : 1   ; /* [14]  */
        unsigned int    pcie1_hilink_pcs_lane5_srst_req : 1   ; /* [13]  */
        unsigned int    pcie1_hilink_pcs_lane4_srst_req : 1   ; /* [12]  */
        unsigned int    pcie1_hilink_pcs_lane3_srst_req : 1   ; /* [11]  */
        unsigned int    pcie1_hilink_pcs_lane2_srst_req : 1   ; /* [10]  */
        unsigned int    pcie1_hilink_pcs_lane1_srst_req : 1   ; /* [9]  */
        unsigned int    pcie1_hilink_pcs_lane0_srst_req : 1   ; /* [8]  */
        unsigned int    pcie0_hilink_pcs_lane7_srst_req : 1   ; /* [7]  */
        unsigned int    pcie0_hilink_pcs_lane6_srst_req : 1   ; /* [6]  */
        unsigned int    pcie0_hilink_pcs_lane5_srst_req : 1   ; /* [5]  */
        unsigned int    pcie0_hilink_pcs_lane4_srst_req : 1   ; /* [4]  */
        unsigned int    pcie0_hilink_pcs_lane3_srst_req : 1   ; /* [3]  */
        unsigned int    pcie0_hilink_pcs_lane2_srst_req : 1   ; /* [2]  */
        unsigned int    pcie0_hilink_pcs_lane1_srst_req : 1   ; /* [1]  */
        unsigned int    pcie0_hilink_pcs_lane0_srst_req : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_HILINK_PCS_RESET_REQ_U;

/* Define the union U_SC_PCIE_HILINK_PCS_RESET_DREQ_U */
/* PCIE HILINK PCSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA64 */
typedef union tagScPcieHilinkPcsResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_hilink_pcs_lane7_srst_dreq : 1   ; /* [31]  */
        unsigned int    pcie3_hilink_pcs_lane6_srst_dreq : 1   ; /* [30]  */
        unsigned int    pcie3_hilink_pcs_lane5_srst_dreq : 1   ; /* [29]  */
        unsigned int    pcie3_hilink_pcs_lane4_srst_dreq : 1   ; /* [28]  */
        unsigned int    pcie3_hilink_pcs_lane3_srst_dreq : 1   ; /* [27]  */
        unsigned int    pcie3_hilink_pcs_lane2_srst_dreq : 1   ; /* [26]  */
        unsigned int    pcie3_hilink_pcs_lane1_srst_dreq : 1   ; /* [25]  */
        unsigned int    pcie3_hilink_pcs_lane0_srst_dreq : 1   ; /* [24]  */
        unsigned int    pcie2_hilink_pcs_lane7_srst_dreq : 1   ; /* [23]  */
        unsigned int    pcie2_hilink_pcs_lane6_srst_dreq : 1   ; /* [22]  */
        unsigned int    pcie2_hilink_pcs_lane5_srst_dreq : 1   ; /* [21]  */
        unsigned int    pcie2_hilink_pcs_lane4_srst_dreq : 1   ; /* [20]  */
        unsigned int    pcie2_hilink_pcs_lane3_srst_dreq : 1   ; /* [19]  */
        unsigned int    pcie2_hilink_pcs_lane2_srst_dreq : 1   ; /* [18]  */
        unsigned int    pcie2_hilink_pcs_lane1_srst_dreq : 1   ; /* [17]  */
        unsigned int    pcie2_hilink_pcs_lane0_srst_dreq : 1   ; /* [16]  */
        unsigned int    pcie1_hilink_pcs_lane7_srst_dreq : 1   ; /* [15]  */
        unsigned int    pcie1_hilink_pcs_lane6_srst_dreq : 1   ; /* [14]  */
        unsigned int    pcie1_hilink_pcs_lane5_srst_dreq : 1   ; /* [13]  */
        unsigned int    pcie1_hilink_pcs_lane4_srst_dreq : 1   ; /* [12]  */
        unsigned int    pcie1_hilink_pcs_lane3_srst_dreq : 1   ; /* [11]  */
        unsigned int    pcie1_hilink_pcs_lane2_srst_dreq : 1   ; /* [10]  */
        unsigned int    pcie1_hilink_pcs_lane1_srst_dreq : 1   ; /* [9]  */
        unsigned int    pcie1_hilink_pcs_lane0_srst_dreq : 1   ; /* [8]  */
        unsigned int    pcie0_hilink_pcs_lane7_srst_dreq : 1   ; /* [7]  */
        unsigned int    pcie0_hilink_pcs_lane6_srst_dreq : 1   ; /* [6]  */
        unsigned int    pcie0_hilink_pcs_lane5_srst_dreq : 1   ; /* [5]  */
        unsigned int    pcie0_hilink_pcs_lane4_srst_dreq : 1   ; /* [4]  */
        unsigned int    pcie0_hilink_pcs_lane3_srst_dreq : 1   ; /* [3]  */
        unsigned int    pcie0_hilink_pcs_lane2_srst_dreq : 1   ; /* [2]  */
        unsigned int    pcie0_hilink_pcs_lane1_srst_dreq : 1   ; /* [1]  */
        unsigned int    pcie0_hilink_pcs_lane0_srst_dreq : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_HILINK_PCS_RESET_DREQ_U;

/* Define the union U_SC_PCIE3_RESET_REQ_U */
/* PCIE3Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA68 */
typedef union tagScPcie3ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie3_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_RESET_REQ_U;

/* Define the union U_SC_PCIE3_RESET_DREQ_U */
/* PCIE3Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA6C */
typedef union tagScPcie3ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie3_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_RESET_DREQ_U;

/* Define the union U_SC_MCTP3_RESET_REQ_U */
/* MCTP3Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA70 */
typedef union tagScMctp3ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp3_srst_req        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP3_RESET_REQ_U;

/* Define the union U_SC_MCTP3_RESET_DREQ_U */
/* MCTP3Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA74 */
typedef union tagScMctp3ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp3_srst_dreq       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP3_RESET_DREQ_U;

/* Define the union U_SC_ITS_RESET_REQ_U */
/* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA80 */
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
/* 0xA84 */
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

/* Define the union U_SC_SLLC_RESET_REQ_U */
/* SLLC0Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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
/* SLLC0Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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

/* Define the union U_SC_PCS_LOCAL_RESET_REQ_U */
/* PCS LOCALÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAC0 */
typedef union tagScPcsLocalResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    pcie3_pcs_local_srst_req : 1   ; /* [3]  */
        unsigned int    pcie2_pcs_local_srst_req : 1   ; /* [2]  */
        unsigned int    pcie1_pcs_local_srst_req : 1   ; /* [1]  */
        unsigned int    pcie0_pcs_local_srst_req : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_REQ_U;

/* Define the union U_SC_PCS_LOCAL_RESET_DREQ_U */
/* PCS LOCALÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAC4 */
typedef union tagScPcsLocalResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    pcie3_pcs_local_srst_dreq : 1   ; /* [3]  */
        unsigned int    pcie2_pcs_local_srst_dreq : 1   ; /* [2]  */
        unsigned int    pcie1_pcs_local_srst_dreq : 1   ; /* [1]  */
        unsigned int    pcie0_pcs_local_srst_dreq : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_DREQ_U;

/* Define the union U_SC_DISP_DAW_EN_U */
/* dispatch daw enÅäÖÃ */
/* 0x1000 */
typedef union tagScDispDawEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    dispatch_daw_en       : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISP_DAW_EN_U;

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

/* Define the union U_SC_FTE_MUX_CTRL_U */
/*  */
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

/* Define the union U_SC_HILINK0_MUX_CTRL_U */
/* HILINK¸´ÓÃÑ¡Ôñ */
/* 0x2300 */
typedef union tagScHilink0MuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink0_mux_sel       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MUX_CTRL_U;

/* Define the union U_SC_HILINK1_MUX_CTRL_U */
/* HILINK¸´ÓÃÑ¡Ôñ */
/* 0x2304 */
typedef union tagScHilink1MuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink1_mux_sel       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MUX_CTRL_U;

/* Define the union U_SC_HILINK2_MUX_CTRL_U */
/* HILINK¸´ÓÃÑ¡Ôñ */
/* 0x2308 */
typedef union tagScHilink2MuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink2_mux_sel       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MUX_CTRL_U;

/* Define the union U_SC_HILINK5_MUX_CTRL_U */
/* HILINK¸´ÓÃÑ¡Ôñ */
/* 0x2314 */
typedef union tagScHilink5MuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink5_mux_sel       : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MUX_CTRL_U;

/* Define the union U_SC_HILINK1_AHB_MUX_CTRL_U */
/* HILINK AHB¸´ÓÃÑ¡Ôñ */
/* 0x2324 */
typedef union tagScHilink1AhbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink1_ahb_mux_sel   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_AHB_MUX_CTRL_U;

/* Define the union U_SC_HILINK2_AHB_MUX_CTRL_U */
/* HILINK AHB¸´ÓÃÑ¡Ôñ */
/* 0x2328 */
typedef union tagScHilink2AhbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink2_ahb_mux_sel   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_AHB_MUX_CTRL_U;

/* Define the union U_SC_HILINK5_AHB_MUX_CTRL_U */
/* HILINK AHB¸´ÓÃÑ¡Ôñ */
/* 0x2334 */
typedef union tagScHilink5AhbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink5_ahb_mux_sel   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_AHB_MUX_CTRL_U;

/* Define the union U_SC_HILINK5_LRSTB_MUX_CTRL_U */
/* HILINK5 lrstb[3:0]µÄMUXÑ¡Ôñ¿ØÖÆ */
/* 0x2340 */
typedef union tagScHilink5LrstbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink5_lrstb_mux_sel : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_LRSTB_MUX_CTRL_U;

/* Define the union U_SC_HILINK6_LRSTB_MUX_CTRL_U */
/* HILINK6 lrstb[3:0]µÄMUXÑ¡Ôñ¿ØÖÆ */
/* 0x2344 */
typedef union tagScHilink6LrstbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink6_lrstb_mux_sel : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_LRSTB_MUX_CTRL_U;

/* Define the union U_SC_HILINK0_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2400 */
typedef union tagScHilink0MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    hilink0_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    hilink0_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink0_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink0_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink0_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink0_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink0_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink0_ss_refclk0_x2s : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK0_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2404 */
typedef union tagScHilink0MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    hilink0_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    hilink0_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink0_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink0_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink0_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink0_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink0_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink0_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink0_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink0_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK0_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2408 */
typedef union tagScHilink0MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink0_lifeclk2dig_sel : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK0_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x240C */
typedef union tagScHilink0MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink0_core_clk_selext : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK0_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2410 */
typedef union tagScHilink0MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink0_core_clk_sel  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK0_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2414 */
typedef union tagScHilink0MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink0_ctrl_bus_mode : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK0_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2418 */
typedef union tagScHilink0MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink0_macropwrdb    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK0_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x241C */
typedef union tagScHilink0MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink0_grstb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK0_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2420 */
typedef union tagScHilink0MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink0_bit_slip      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK0_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2424 */
typedef union tagScHilink0MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink0_lrstb         : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_LRSTB_U;

/* Define the union U_SC_HILINK1_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2500 */
typedef union tagScHilink1MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    hilink1_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    hilink1_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink1_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink1_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink1_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink1_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink1_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink1_ss_refclk0_x2s : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK1_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2504 */
typedef union tagScHilink1MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    hilink1_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    hilink1_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink1_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink1_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink1_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink1_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink1_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink1_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink1_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink1_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK1_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2508 */
typedef union tagScHilink1MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink1_lifeclk2dig_sel : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK1_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x250C */
typedef union tagScHilink1MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink1_core_clk_selext : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK1_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2510 */
typedef union tagScHilink1MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink1_core_clk_sel  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK1_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2514 */
typedef union tagScHilink1MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink1_ctrl_bus_mode : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK1_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2518 */
typedef union tagScHilink1MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink1_macropwrdb    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK1_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x251C */
typedef union tagScHilink1MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink1_grstb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK1_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2520 */
typedef union tagScHilink1MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink1_bit_slip      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK1_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2524 */
typedef union tagScHilink1MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink1_lrstb         : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_LRSTB_U;

/* Define the union U_SC_HILINK5_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2600 */
typedef union tagScHilink5MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    hilink5_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    hilink5_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink5_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink5_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink5_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink5_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink5_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink5_ss_refclk0_x2s : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK5_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2604 */
typedef union tagScHilink5MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    hilink5_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    hilink5_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink5_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink5_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink5_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink5_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink5_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink5_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink5_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink5_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK5_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2608 */
typedef union tagScHilink5MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink5_lifeclk2dig_sel : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK5_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x260C */
typedef union tagScHilink5MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink5_core_clk_selext : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK5_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2610 */
typedef union tagScHilink5MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink5_core_clk_sel  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK5_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2614 */
typedef union tagScHilink5MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink5_ctrl_bus_mode : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK5_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2618 */
typedef union tagScHilink5MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink5_macropwrdb    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK5_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x261C */
typedef union tagScHilink5MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink5_grstb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK5_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2620 */
typedef union tagScHilink5MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink5_bit_slip      : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK5_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2624 */
typedef union tagScHilink5MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink5_lrstb         : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_LRSTB_U;

/* Define the union U_SC_HILINK6_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2700 */
typedef union tagScHilink6MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    hilink6_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    hilink6_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink6_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink6_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink6_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink6_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink6_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink6_ss_refclk0_x2s : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK6_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2704 */
typedef union tagScHilink6MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
        unsigned int    hilink6_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    hilink6_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink6_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink6_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink6_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink6_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink6_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink6_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink6_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink6_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK6_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2708 */
typedef union tagScHilink6MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink6_lifeclk2dig_sel : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK6_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x270C */
typedef union tagScHilink6MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink6_core_clk_selext : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK6_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2710 */
typedef union tagScHilink6MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink6_core_clk_sel  : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK6_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2714 */
typedef union tagScHilink6MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink6_ctrl_bus_mode : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK6_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2718 */
typedef union tagScHilink6MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink6_macropwrdb    : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK6_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x271C */
typedef union tagScHilink6MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    hilink6_grstb         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK6_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2720 */
typedef union tagScHilink6MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink6_bit_slip      : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK6_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2724 */
typedef union tagScHilink6MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink6_lrstb         : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_LRSTB_U;

/* Define the union U_SC_PCIE0_CLKREQ_U */
/*  */
/* 0x2800 */
typedef union tagScPcie0Clkreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    pcie0_apb_cfg_sel     : 2   ; /* [2..1]  */
        unsigned int    pcie0_phy_clk_req_n   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_CLKREQ_U;

/* Define the union U_SC_PCIE0_AXI_MSTR_OOO_WR_CFG_U */
/*  */
/* 0x2880 */
typedef union tagScPcie0AxiMstrOooWrCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_2            : 15  ; /* [31..17]  */
        unsigned int    pcie0_cfg_violate_posted_req_order : 1   ; /* [16]  */
        unsigned int    reserved_1            : 3   ; /* [15..13]  */
        unsigned int    pcie0_ctrl_lat_stat_wr_en : 1   ; /* [12]  */
        unsigned int    pcie0_wr_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie0_cfg_max_wr_trans : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_AXI_MSTR_OOO_WR_CFG_U;

/* Define the union U_SC_PCIE0_AXI_MSTR_OOO_RD_CFG_U */
/*  */
/* 0x2890 */
typedef union tagScPcie0AxiMstrOooRdCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_1            : 19  ; /* [31..13]  */
        unsigned int    pcie0_ctrl_lat_stat_rd_en : 1   ; /* [12]  */
        unsigned int    pcie0_rd_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie0_cfg_max_rd_trans : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_AXI_MSTR_OOO_RD_CFG_U;

/* Define the union U_SC_PCIE1_CLKREQ_U */
/*  */
/* 0x2900 */
typedef union tagScPcie1Clkreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    pcie1_apb_cfg_sel     : 2   ; /* [3..2]  */
        unsigned int    pcie1vsemi_phy_clk_req_n : 1   ; /* [1]  */
        unsigned int    pcie1hilink_phy_clk_req_n : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_CLKREQ_U;

/* Define the union U_SC_PCIE1_AXI_MSTR_OOO_WR_CFG_U */
/*  */
/* 0x2980 */
typedef union tagScPcie1AxiMstrOooWrCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_2            : 15  ; /* [31..17]  */
        unsigned int    pcie1_cfg_violate_posted_req_order : 1   ; /* [16]  */
        unsigned int    reserved_1            : 3   ; /* [15..13]  */
        unsigned int    pcie1_ctrl_lat_stat_wr_en : 1   ; /* [12]  */
        unsigned int    pcie1_wr_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie1_cfg_max_wr_trans : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_AXI_MSTR_OOO_WR_CFG_U;

/* Define the union U_SC_PCIE1_AXI_MSTR_OOO_RD_CFG_U */
/*  */
/* 0x2990 */
typedef union tagScPcie1AxiMstrOooRdCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_1            : 19  ; /* [31..13]  */
        unsigned int    pcie1_ctrl_lat_stat_rd_en : 1   ; /* [12]  */
        unsigned int    pcie1_rd_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie1_cfg_max_rd_trans : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_AXI_MSTR_OOO_RD_CFG_U;

/* Define the union U_SC_PCIE2_CLKREQ_U */
/*  */
/* 0x2A00 */
typedef union tagScPcie2Clkreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    pcie2_apb_cfg_sel     : 2   ; /* [3..2]  */
        unsigned int    pcie2vsemi_phy_clk_req_n : 1   ; /* [1]  */
        unsigned int    pcie2hilink_phy_clk_req_n : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_CLKREQ_U;

/* Define the union U_SC_PCIE2_AXI_MSTR_OOO_WR_CFG_U */
/*  */
/* 0x2A80 */
typedef union tagScPcie2AxiMstrOooWrCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_2            : 15  ; /* [31..17]  */
        unsigned int    pcie2_cfg_violate_posted_req_order : 1   ; /* [16]  */
        unsigned int    reserved_1            : 3   ; /* [15..13]  */
        unsigned int    pcie2_ctrl_lat_stat_wr_en : 1   ; /* [12]  */
        unsigned int    pcie2_wr_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie2_cfg_max_wr_trans : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_AXI_MSTR_OOO_WR_CFG_U;

/* Define the union U_SC_PCIE2_AXI_MSTR_OOO_RD_CFG_U */
/*  */
/* 0x2A90 */
typedef union tagScPcie2AxiMstrOooRdCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_1            : 19  ; /* [31..13]  */
        unsigned int    pcie2_ctrl_lat_stat_rd_en : 1   ; /* [12]  */
        unsigned int    pcie2_rd_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie2_cfg_max_rd_trans : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_AXI_MSTR_OOO_RD_CFG_U;

/* Define the union U_SC_PCIE3_CLKREQ_U */
/*  */
/* 0x2B00 */
typedef union tagScPcie3Clkreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
        unsigned int    pcie3_apb_cfg_sel     : 2   ; /* [2..1]  */
        unsigned int    pcie3_phy_clk_req_n   : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_CLKREQ_U;

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

/* Define the union U_SC_SLLC0_MEM_CTRL_U */
/* sllc0 mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3010 */
typedef union tagScSllc0MemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    ctrl_rft_sllc0        : 10  ; /* [9..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC0_MEM_CTRL_U;

/* Define the union U_SC_SAS_MEM_CTRL_U */
/* sas mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3030 */
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

/* Define the union U_SC_PCIE_MEM_CTRL0_U */
/* pcie mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3040 */
typedef union tagScPcieMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
        unsigned int    ctrl_rft_pcie         : 10  ; /* [9..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_MEM_CTRL0_U;

/* Define the union U_SC_PCIE_MEM_CTRL1_U */
/* pcie mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3044 */
typedef union tagScPcieMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rashsd_pcie      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_MEM_CTRL1_U;

/* Define the union U_SC_PCIE_MEM_CTRL2_U */
/* pcie mem¿ØÖÆ¼Ä´æÆ÷2 */
/* 0x3048 */
typedef union tagScPcieMemCtrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    ctrl_rfs_pcie         : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_MEM_CTRL2_U;

/* Define the union U_SC_SKEW_COMMON_0_U */
/*  */
/* 0x3400 */
typedef union tagScSkewCommon0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    skew_en               : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_COMMON_0_U;

/* Define the union U_SC_SKEW_COMMON_1_U */
/*  */
/* 0x3404 */
typedef union tagScSkewCommon1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
        unsigned int    skew_addr_offset      : 5   ; /* [4..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_COMMON_1_U;

/* Define the union U_SC_SKEW_COMMON_2_U */
/*  */
/* 0x3408 */
typedef union tagScSkewCommon2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    skew_config_in        : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_COMMON_2_U;

/* Define the union U_SC_SKEW_A_0_U */
/*  */
/* 0x3500 */
typedef union tagScSkewA0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    skew_bypass_a         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_0_U;

/* Define the union U_SC_SKEW_A_1_U */
/*  */
/* 0x3504 */
typedef union tagScSkewA1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    skew_config_in_a      : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_1_U;

/* Define the union U_SC_SKEW_A_2_U */
/*  */
/* 0x3508 */
typedef union tagScSkewA2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    skew_in_delay_sel_a   : 2   ; /* [3..2]  */
        unsigned int    skew_out_delay_sel_a  : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_2_U;

/* Define the union U_SC_SKEW_A_3_U */
/*  */
/* 0x350C */
typedef union tagScSkewA3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    skew_sel_a_0          : 1   ; /* [1]  */
        unsigned int    skew_sel_a_1          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_3_U;

/* Define the union U_SC_SKEW_A_4_U */
/*  */
/* 0x3510 */
typedef union tagScSkewA4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    skew_update_en_a      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_4_U;

/* Define the union U_SC_SKEW_A_5_U */
/*  */
/* 0x3514 */
typedef union tagScSkewA5
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    skew_varible_set_a    : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_5_U;

/* Define the union U_SC_SKEW_A_7_U */
/*  */
/* 0x351C */
typedef union tagScSkewA7
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    skew_dcell_set_a_h    : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_7_U;

/* Define the union U_SC_SKEW_A_8_U */
/*  */
/* 0x3520 */
typedef union tagScSkewA8
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    skew_sel_osc_a        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_8_U;

/* Define the union U_SC_SKEW_B_0_U */
/*  */
/* 0x3600 */
typedef union tagScSkewB0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    skew_bypass_b         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_0_U;

/* Define the union U_SC_SKEW_B_1_U */
/*  */
/* 0x3604 */
typedef union tagScSkewB1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    skew_config_in_b      : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_1_U;

/* Define the union U_SC_SKEW_B_2_U */
/*  */
/* 0x3608 */
typedef union tagScSkewB2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    skew_in_delay_sel_b   : 2   ; /* [3..2]  */
        unsigned int    skew_out_delay_sel_b  : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_2_U;

/* Define the union U_SC_SKEW_B_3_U */
/*  */
/* 0x360C */
typedef union tagScSkewB3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    skew_sel_b_0          : 1   ; /* [1]  */
        unsigned int    skew_sel_b_1          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_3_U;

/* Define the union U_SC_SKEW_B_4_U */
/*  */
/* 0x3610 */
typedef union tagScSkewB4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    skew_update_en_b      : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_4_U;

/* Define the union U_SC_SKEW_B_5_U */
/*  */
/* 0x3614 */
typedef union tagScSkewB5
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    skew_varible_set_b    : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_5_U;

/* Define the union U_SC_SKEW_B_7_U */
/*  */
/* 0x361C */
typedef union tagScSkewB7
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    skew_dcell_set_b_h    : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_7_U;

/* Define the union U_SC_SKEW_B_8_U */
/*  */
/* 0x3620 */
typedef union tagScSkewB8
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    skew_sel_osc_b        : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_8_U;

/* Define the union U_SC_PCIE0_CLK_ST_U */
/* PCIE0Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5300 */
typedef union tagScPcie0ClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie0_pipe_st     : 1   ; /* [1]  */
        unsigned int    clk_pcie0_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_CLK_ST_U;

/* Define the union U_SC_PCIE1_CLK_ST_U */
/* PCIE1Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5304 */
typedef union tagScPcie1ClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie1_pipe_st     : 1   ; /* [1]  */
        unsigned int    clk_pcie1_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_CLK_ST_U;

/* Define the union U_SC_PCIE2_CLK_ST_U */
/* PCIE2Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5308 */
typedef union tagScPcie2ClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie2_pipe_st     : 1   ; /* [1]  */
        unsigned int    clk_pcie2_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_CLK_ST_U;

/* Define the union U_SC_SAS_CLK_ST_U */
/* SASÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x530C */
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

/* Define the union U_SC_PCIE3_CLK_ST_U */
/* PCIE3Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5310 */
typedef union tagScPcie3ClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    clk_pcie3_pipe_st     : 1   ; /* [1]  */
        unsigned int    clk_pcie3_st          : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_CLK_ST_U;

/* Define the union U_SC_ITS_CLK_ST_U */
/* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5314 */
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

/* Define the union U_SC_SLLC_CLK_ST_U */
/* SLLC0Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_PCIE0_RESET_ST_U */
/* PCIE0¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A00 */
typedef union tagScPcie0ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie0_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_RESET_ST_U;

/* Define the union U_SC_PCIE1_RESET_ST_U */
/* PCIE1¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A04 */
typedef union tagScPcie1ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie1_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_RESET_ST_U;

/* Define the union U_SC_PCIE2_RESET_ST_U */
/* PCIE2¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A08 */
typedef union tagScPcie2ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie2_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_RESET_ST_U;

/* Define the union U_SC_SAS_RESET_ST_U */
/* SAS¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A0C */
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

/* Define the union U_SC_MCTP0_RESET_ST_U */
/* MCTP0¸´Î»×´Ì¬¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0x5A10 */
typedef union tagScMctp0ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp0_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP0_RESET_ST_U;

/* Define the union U_SC_MCTP1_RESET_ST_U */
/* MCTP1¸´Î»×´Ì¬¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0x5A14 */
typedef union tagScMctp1ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp1_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP1_RESET_ST_U;

/* Define the union U_SC_MCTP2_RESET_ST_U */
/* MCTP2¸´Î»×´Ì¬¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0x5A18 */
typedef union tagScMctp2ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp2_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP2_RESET_ST_U;

/* Define the union U_SC_SLLC_TSVRX_RESET_ST_U */
/* ÎªSLLC TSVRXÍ¨µÀÈí¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A2C */
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

/* Define the union U_SC_PCIE_HILINK_PCS_RESET_ST_U */
/* PCIE HILINK PCS¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A30 */
typedef union tagScPcieHilinkPcsResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_hilink_pcs_lane7_srst_st : 1   ; /* [31]  */
        unsigned int    pcie3_hilink_pcs_lane6_srst_st : 1   ; /* [30]  */
        unsigned int    pcie3_hilink_pcs_lane5_srst_st : 1   ; /* [29]  */
        unsigned int    pcie3_hilink_pcs_lane4_srst_st : 1   ; /* [28]  */
        unsigned int    pcie3_hilink_pcs_lane3_srst_st : 1   ; /* [27]  */
        unsigned int    pcie3_hilink_pcs_lane2_srst_st : 1   ; /* [26]  */
        unsigned int    pcie3_hilink_pcs_lane1_srst_st : 1   ; /* [25]  */
        unsigned int    pcie3_hilink_pcs_lane0_srst_st : 1   ; /* [24]  */
        unsigned int    pcie2_hilink_pcs_lane7_srst_st : 1   ; /* [23]  */
        unsigned int    pcie2_hilink_pcs_lane6_srst_st : 1   ; /* [22]  */
        unsigned int    pcie2_hilink_pcs_lane5_srst_st : 1   ; /* [21]  */
        unsigned int    pcie2_hilink_pcs_lane4_srst_st : 1   ; /* [20]  */
        unsigned int    pcie2_hilink_pcs_lane3_srst_st : 1   ; /* [19]  */
        unsigned int    pcie2_hilink_pcs_lane2_srst_st : 1   ; /* [18]  */
        unsigned int    pcie2_hilink_pcs_lane1_srst_st : 1   ; /* [17]  */
        unsigned int    pcie2_hilink_pcs_lane0_srst_st : 1   ; /* [16]  */
        unsigned int    pcie1_hilink_pcs_lane7_srst_st : 1   ; /* [15]  */
        unsigned int    pcie1_hilink_pcs_lane6_srst_st : 1   ; /* [14]  */
        unsigned int    pcie1_hilink_pcs_lane5_srst_st : 1   ; /* [13]  */
        unsigned int    pcie1_hilink_pcs_lane4_srst_st : 1   ; /* [12]  */
        unsigned int    pcie1_hilink_pcs_lane3_srst_st : 1   ; /* [11]  */
        unsigned int    pcie1_hilink_pcs_lane2_srst_st : 1   ; /* [10]  */
        unsigned int    pcie1_hilink_pcs_lane1_srst_st : 1   ; /* [9]  */
        unsigned int    pcie1_hilink_pcs_lane0_srst_st : 1   ; /* [8]  */
        unsigned int    pcie0_hilink_pcs_lane7_srst_st : 1   ; /* [7]  */
        unsigned int    pcie0_hilink_pcs_lane6_srst_st : 1   ; /* [6]  */
        unsigned int    pcie0_hilink_pcs_lane5_srst_st : 1   ; /* [5]  */
        unsigned int    pcie0_hilink_pcs_lane4_srst_st : 1   ; /* [4]  */
        unsigned int    pcie0_hilink_pcs_lane3_srst_st : 1   ; /* [3]  */
        unsigned int    pcie0_hilink_pcs_lane2_srst_st : 1   ; /* [2]  */
        unsigned int    pcie0_hilink_pcs_lane1_srst_st : 1   ; /* [1]  */
        unsigned int    pcie0_hilink_pcs_lane0_srst_st : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_HILINK_PCS_RESET_ST_U;

/* Define the union U_SC_PCIE3_RESET_ST_U */
/* PCIE3¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A34 */
typedef union tagScPcie3ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie3_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_RESET_ST_U;

/* Define the union U_SC_MCTP3_RESET_ST_U */
/* MCTP3¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A38 */
typedef union tagScMctp3ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    mctp3_srst_st         : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP3_RESET_ST_U;

/* Define the union U_SC_ITS_RESET_ST_U */
/* ITS¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A40 */
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

/* Define the union U_SC_SLLC_RESET_ST_U */
/* SLLC0¸´Î»×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_PCS_LOCAL_RESET_ST_U */
/* PCS LOCALÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A60 */
typedef union tagScPcsLocalResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    pcie3_pcs_local_srst_st : 1   ; /* [3]  */
        unsigned int    pcie2_pcs_local_srst_st : 1   ; /* [2]  */
        unsigned int    pcie1_pcs_local_srst_st : 1   ; /* [1]  */
        unsigned int    pcie0_pcs_local_srst_st : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_ST_U;

/* Define the union U_SC_HILINK0_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6400 */
typedef union tagScHilink0MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink0_plloutoflock  : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK0_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6404 */
typedef union tagScHilink0MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink0_prbs_err      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK0_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6408 */
typedef union tagScHilink0MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink0_los           : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_LOS_U;

/* Define the union U_SC_HILINK1_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6500 */
typedef union tagScHilink1MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink1_plloutoflock  : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK1_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6504 */
typedef union tagScHilink1MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink1_prbs_err      : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK1_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6508 */
typedef union tagScHilink1MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    hilink1_los           : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_LOS_U;

/* Define the union U_SC_HILINK5_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6600 */
typedef union tagScHilink5MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink5_plloutoflock  : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK5_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6604 */
typedef union tagScHilink5MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink5_prbs_err      : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK5_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6608 */
typedef union tagScHilink5MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink5_los           : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_LOS_U;

/* Define the union U_SC_HILINK6_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6700 */
typedef union tagScHilink6MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    hilink6_plloutoflock  : 2   ; /* [1..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK6_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6704 */
typedef union tagScHilink6MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink6_prbs_err      : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK6_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6708 */
typedef union tagScHilink6MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    hilink6_los           : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_LOS_U;

/* Define the union U_SC_PCIE0_RXEQINPRO_STAT_U */
/*  */
/* 0x6800 */
typedef union tagScPcie0RxeqinproStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    pcie0_mac_phy_rxeqinprogress : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_RXEQINPRO_STAT_U;

/* Define the union U_SC_PCIE0_LINKINT_RCVRY_STAT_U */
/*  */
/* 0x6804 */
typedef union tagScPcie0LinkintRcvryStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    pcie0_xmlh_ltssm_state_rcvry_eq : 1   ; /* [1]  */
        unsigned int    pcie0_cfg_link_eq_req_int : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_LINKINT_RCVRY_STAT_U;

/* Define the union U_SC_PCIE0_SYS_STATE0_U */
/* pcie0×´Ì¬¼Ä´æÆ÷0 */
/* 0x6808 */
typedef union tagScPcie0SysState0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_radm_msg_req_id_low : 8   ; /* [31..24]  */
        unsigned int    pcie0_p2_exit_reg     : 1   ; /* [23]  */
        unsigned int    pcie0_ref_clk_req_n   : 1   ; /* [22]  */
        unsigned int    pcie0_pm_status       : 1   ; /* [21]  */
        unsigned int    pcie0_hp_msi          : 1   ; /* [20]  */
        unsigned int    pcie0_hp_int          : 1   ; /* [19]  */
        unsigned int    pcie0_hp_pme          : 1   ; /* [18]  */
        unsigned int    pcie0_cfg_eml_control : 1   ; /* [17]  */
        unsigned int    pcie0_wake            : 1   ; /* [16]  */
        unsigned int    pcie0_xmlh_link_up    : 1   ; /* [15]  */
        unsigned int    pcie0_cfg_aer_int_msg_num : 5   ; /* [14..10]  */
        unsigned int    pcie0_cfg_aer_rc_err_int : 1   ; /* [9]  */
        unsigned int    pcie0_pm_curnt_state  : 3   ; /* [8..6]  */
        unsigned int    pcie0_rdlh_link_up    : 1   ; /* [5]  */
        unsigned int    pcie0_cfg_rcb         : 1   ; /* [4]  */
        unsigned int    pcie0_cfg_mem_space_en : 1   ; /* [3]  */
        unsigned int    pcie0_pm_xtlh_block_tlp : 1   ; /* [2]  */
        unsigned int    pcie0_radmx_cmposer_lookup_err : 1   ; /* [1]  */
        unsigned int    pcie0_gm_cmposer_lookup_err : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_SYS_STATE0_U;

/* Define the union U_SC_PCIE0_SYS_STATE1_U */
/* pcie0×´Ì¬¼Ä´æÆ÷1 */
/* 0x680C */
typedef union tagScPcie0SysState1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_radm_msg_req_id_high : 8   ; /* [31..24]  */
        unsigned int    pcie0_cfg_sys_err_rc  : 1   ; /* [23]  */
        unsigned int    pcie0_cfg_aer_rc_err_msi : 1   ; /* [22]  */
        unsigned int    pcie0_link_req_rst_not_reg : 1   ; /* [21]  */
        unsigned int    pcie0_bridge_flush_not_reg : 1   ; /* [20]  */
        unsigned int    pcie0_cfg_pme_msi_reg : 1   ; /* [19]  */
        unsigned int    pcie0_radm_msg_unlock_reg : 1   ; /* [18]  */
        unsigned int    pcie0_radm_cpl_timeout_reg : 1   ; /* [17]  */
        unsigned int    pcie0_radm_pm_to_ack_reg : 1   ; /* [16]  */
        unsigned int    pcie0_radm_pm_to_pme_reg : 1   ; /* [15]  */
        unsigned int    pcie0_radm_fatal_err_reg : 1   ; /* [14]  */
        unsigned int    pcie0_radm_nonfatal_err_reg : 1   ; /* [13]  */
        unsigned int    pcie0_radm_correctabl_err_reg : 1   ; /* [12]  */
        unsigned int    pcie0_mac_phy_power_down : 2   ; /* [11..10]  */
        unsigned int    pcie0_pm_linkst_l2_exit : 1   ; /* [9]  */
        unsigned int    pcie0_pm_linkst_in_l2 : 1   ; /* [8]  */
        unsigned int    pcie0_pm_linkst_in_l1 : 1   ; /* [7]  */
        unsigned int    pcie0_app_parity_errs_reg : 3   ; /* [6..4]  */
        unsigned int    pcie0_axi_parity_errs_reg : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_SYS_STATE1_U;

/* Define the union U_SC_PCIE0_SYS_STATE4_U */
/* pcie0×´Ì¬¼Ä´æÆ÷4 */
/* 0x6818 */
typedef union tagScPcie0SysState4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
        unsigned int    pcie0_link_req_rst_not : 1   ; /* [28]  */
        unsigned int    pcie0_bridge_flush_not : 1   ; /* [27]  */
        unsigned int    pcie0_radm_vendor_msg_reg : 1   ; /* [26]  */
        unsigned int    pcie0_cfg_pme_int_reg : 1   ; /* [25]  */
        unsigned int    pcie0_radm_intd_reg   : 1   ; /* [24]  */
        unsigned int    pcie0_radm_intc_reg   : 1   ; /* [23]  */
        unsigned int    pcie0_radm_intb_reg   : 1   ; /* [22]  */
        unsigned int    pcie0_radm_inta_reg   : 1   ; /* [21]  */
        unsigned int    pcie0_mstr_bresp_int  : 1   ; /* [20]  */
        unsigned int    pcie0_mstr_rresp_int  : 1   ; /* [19]  */
        unsigned int    pcie0_retry_sram_addr : 10  ; /* [18..9]  */
        unsigned int    pcie0_slv_err_int     : 1   ; /* [8]  */
        unsigned int    pcie0_mac_phy_rate    : 2   ; /* [7..6]  */
        unsigned int    pcie0_ltssm_state     : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_SYS_STATE4_U;

/* Define the union U_SC_PCIE0_AXI_MSTR_OOO_WR_STS1_U */
/* OOOÐ´Í¨µÀ¹¤×÷×´Ì¬ */
/* 0x6884 */
typedef union tagScPcie0AxiMstrOooWrSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
        unsigned int    pcie0_curr_wr_port_sts : 1   ; /* [16]  */
        unsigned int    pcie0_curr_wr_latency : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_AXI_MSTR_OOO_WR_STS1_U;

/* Define the union U_SC_PCIE0_AXI_MSTR_OOO_RD_STS1_U */
/* OOO¶ÁÍ¨µÀ¹¤×÷×´Ì¬ */
/* 0x6894 */
typedef union tagScPcie0AxiMstrOooRdSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
        unsigned int    pcie0_curr_rd_port_sts : 1   ; /* [16]  */
        unsigned int    pcie0_curr_rd_latency : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_AXI_MSTR_OOO_RD_STS1_U;

/* Define the union U_SC_PCIE0_DSIZE_BRG_ECC_ERR_U */
/* PCIE¶ÔÓ¦DSIZE BRGµÄECC´íÎó×´Ì¬ */
/* 0x68A0 */
typedef union tagScPcie0DsizeBrgEccErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    pcie0_rob_ecc_err_multpl : 1   ; /* [1]  */
        unsigned int    pcie0_rob_ecc_err_detect : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_DSIZE_BRG_ECC_ERR_U;

/* Define the union U_SC_PCIE0_PCIEPHY_CTRL_ERROR_U */
/* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
/* 0x68C0 */
typedef union tagScPcie0PciephyCtrlError
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie0_pciephy_ctrl_error : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_PCIEPHY_CTRL_ERROR_U;

/* Define the union U_SC_PCIE1_RXEQINPRO_STAT_U */
/*  */
/* 0x6900 */
typedef union tagScPcie1RxeqinproStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    pcie1_mac_phy_rxeqinprogress : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_RXEQINPRO_STAT_U;

/* Define the union U_SC_PCIE1_LINKINT_RCVRY_STAT_U */
/*  */
/* 0x6904 */
typedef union tagScPcie1LinkintRcvryStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    pcie1_xmlh_ltssm_state_rcvry_eq : 1   ; /* [1]  */
        unsigned int    pcie1_cfg_link_eq_req_int : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_LINKINT_RCVRY_STAT_U;

/* Define the union U_SC_PCIE1_SYS_STATE0_U */
/* pcie1×´Ì¬¼Ä´æÆ÷0 */
/* 0x6908 */
typedef union tagScPcie1SysState0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_radm_msg_req_id_low : 8   ; /* [31..24]  */
        unsigned int    pcie1_p2_exit_reg     : 1   ; /* [23]  */
        unsigned int    pcie1_ref_clk_req_n   : 1   ; /* [22]  */
        unsigned int    pcie1_pm_status       : 1   ; /* [21]  */
        unsigned int    pcie1_hp_msi          : 1   ; /* [20]  */
        unsigned int    pcie1_hp_int          : 1   ; /* [19]  */
        unsigned int    pcie1_hp_pme          : 1   ; /* [18]  */
        unsigned int    pcie1_cfg_eml_control : 1   ; /* [17]  */
        unsigned int    pcie1_wake            : 1   ; /* [16]  */
        unsigned int    pcie1_xmlh_link_up    : 1   ; /* [15]  */
        unsigned int    pcie1_cfg_aer_int_msg_num : 5   ; /* [14..10]  */
        unsigned int    pcie1_cfg_aer_rc_err_int : 1   ; /* [9]  */
        unsigned int    pcie1_pm_curnt_state  : 3   ; /* [8..6]  */
        unsigned int    pcie1_rdlh_link_up    : 1   ; /* [5]  */
        unsigned int    pcie1_cfg_rcb         : 1   ; /* [4]  */
        unsigned int    pcie1_cfg_mem_space_en : 1   ; /* [3]  */
        unsigned int    pcie1_pm_xtlh_block_tlp : 1   ; /* [2]  */
        unsigned int    pcie1_radmx_cmposer_lookup_err : 1   ; /* [1]  */
        unsigned int    pcie1_gm_cmposer_lookup_err : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_SYS_STATE0_U;

/* Define the union U_SC_PCIE1_SYS_STATE1_U */
/* pcie1×´Ì¬¼Ä´æÆ÷1 */
/* 0x690C */
typedef union tagScPcie1SysState1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_radm_msg_req_id_high : 8   ; /* [31..24]  */
        unsigned int    pcie1_cfg_sys_err_rc  : 1   ; /* [23]  */
        unsigned int    pcie1_cfg_aer_rc_err_msi : 1   ; /* [22]  */
        unsigned int    pcie1_link_req_rst_not_reg : 1   ; /* [21]  */
        unsigned int    pcie1_bridge_flush_not_reg : 1   ; /* [20]  */
        unsigned int    pcie1_cfg_pme_msi_reg : 1   ; /* [19]  */
        unsigned int    pcie1_radm_msg_unlock_reg : 1   ; /* [18]  */
        unsigned int    pcie1_radm_cpl_timeout_reg : 1   ; /* [17]  */
        unsigned int    pcie1_radm_pm_to_ack_reg : 1   ; /* [16]  */
        unsigned int    pcie1_radm_pm_to_pme_reg : 1   ; /* [15]  */
        unsigned int    pcie1_radm_fatal_err_reg : 1   ; /* [14]  */
        unsigned int    pcie1_radm_nonfatal_err_reg : 1   ; /* [13]  */
        unsigned int    pcie1_radm_correctabl_err_reg : 1   ; /* [12]  */
        unsigned int    pcie1_mac_phy_power_down : 2   ; /* [11..10]  */
        unsigned int    pcie1_pm_linkst_l2_exit : 1   ; /* [9]  */
        unsigned int    pcie1_pm_linkst_in_l2 : 1   ; /* [8]  */
        unsigned int    pcie1_pm_linkst_in_l1 : 1   ; /* [7]  */
        unsigned int    pcie1_app_parity_errs_reg : 3   ; /* [6..4]  */
        unsigned int    pcie1_axi_parity_errs_reg : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_SYS_STATE1_U;

/* Define the union U_SC_PCIE1_SYS_STATE4_U */
/* pcie1×´Ì¬¼Ä´æÆ÷4 */
/* 0x6918 */
typedef union tagScPcie1SysState4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
        unsigned int    pcie1_link_req_rst_not : 1   ; /* [28]  */
        unsigned int    pcie1_bridge_flush_not : 1   ; /* [27]  */
        unsigned int    pcie1_radm_vendor_msg_reg : 1   ; /* [26]  */
        unsigned int    pcie1_cfg_pme_int_reg : 1   ; /* [25]  */
        unsigned int    pcie1_radm_intd_reg   : 1   ; /* [24]  */
        unsigned int    pcie1_radm_intc_reg   : 1   ; /* [23]  */
        unsigned int    pcie1_radm_intb_reg   : 1   ; /* [22]  */
        unsigned int    pcie1_radm_inta_reg   : 1   ; /* [21]  */
        unsigned int    pcie1_mstr_bresp_int  : 1   ; /* [20]  */
        unsigned int    pcie1_mstr_rresp_int  : 1   ; /* [19]  */
        unsigned int    pcie1_retry_sram_addr : 10  ; /* [18..9]  */
        unsigned int    pcie1_slv_err_int     : 1   ; /* [8]  */
        unsigned int    pcie1_mac_phy_rate    : 2   ; /* [7..6]  */
        unsigned int    pcie1_ltssm_state     : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_SYS_STATE4_U;

/* Define the union U_SC_PCIE1_AXI_MSTR_OOO_WR_STS1_U */
/* OOOÐ´Í¨µÀ¹¤×÷×´Ì¬ */
/* 0x6984 */
typedef union tagScPcie1AxiMstrOooWrSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
        unsigned int    pcie1_curr_wr_port_sts : 1   ; /* [16]  */
        unsigned int    pcie1_curr_wr_latency : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_AXI_MSTR_OOO_WR_STS1_U;

/* Define the union U_SC_PCIE1_AXI_MSTR_OOO_RD_STS1_U */
/* OOO¶ÁÍ¨µÀ¹¤×÷×´Ì¬ */
/* 0x6994 */
typedef union tagScPcie1AxiMstrOooRdSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
        unsigned int    pcie1_curr_rd_port_sts : 1   ; /* [16]  */
        unsigned int    pcie1_curr_rd_latency : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_AXI_MSTR_OOO_RD_STS1_U;

/* Define the union U_SC_PCIE1_DSIZE_BRG_ECC_ERR_U */
/* PCIE¶ÔÓ¦DSIZE BRGµÄECC´íÎó×´Ì¬ */
/* 0x69A0 */
typedef union tagScPcie1DsizeBrgEccErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    pcie1_rob_ecc_err_multpl : 1   ; /* [1]  */
        unsigned int    pcie1_rob_ecc_err_detect : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_DSIZE_BRG_ECC_ERR_U;

/* Define the union U_SC_PCIE1_PCIEPHY_CTRL_ERROR_U */
/* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
/* 0x69C0 */
typedef union tagScPcie1PciephyCtrlError
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie1_pciephy_ctrl_error : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_PCIEPHY_CTRL_ERROR_U;

/* Define the union U_SC_PCIE2_RXEQINPRO_STAT_U */
/*  */
/* 0x6A00 */
typedef union tagScPcie2RxeqinproStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
        unsigned int    pcie2_mac_phy_rxeqinprogress : 8   ; /* [7..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_RXEQINPRO_STAT_U;

/* Define the union U_SC_PCIE2_LINKINT_RCVRY_STAT_U */
/*  */
/* 0x6A04 */
typedef union tagScPcie2LinkintRcvryStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    pcie2_xmlh_ltssm_state_rcvry_eq : 1   ; /* [1]  */
        unsigned int    pcie2_cfg_link_eq_req_int : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_LINKINT_RCVRY_STAT_U;

/* Define the union U_SC_PCIE2_SYS_STATE0_U */
/* pcie2×´Ì¬¼Ä´æÆ÷0 */
/* 0x6A08 */
typedef union tagScPcie2SysState0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_radm_msg_req_id_low : 8   ; /* [31..24]  */
        unsigned int    pcie2_p2_exit_reg     : 1   ; /* [23]  */
        unsigned int    pcie2_ref_clk_req_n   : 1   ; /* [22]  */
        unsigned int    pcie2_pm_status       : 1   ; /* [21]  */
        unsigned int    pcie2_hp_msi          : 1   ; /* [20]  */
        unsigned int    pcie2_hp_int          : 1   ; /* [19]  */
        unsigned int    pcie2_hp_pme          : 1   ; /* [18]  */
        unsigned int    pcie2_cfg_eml_control : 1   ; /* [17]  */
        unsigned int    pcie2_wake            : 1   ; /* [16]  */
        unsigned int    pcie2_xmlh_link_up    : 1   ; /* [15]  */
        unsigned int    pcie2_cfg_aer_int_msg_num : 5   ; /* [14..10]  */
        unsigned int    pcie2_cfg_aer_rc_err_int : 1   ; /* [9]  */
        unsigned int    pcie2_pm_curnt_state  : 3   ; /* [8..6]  */
        unsigned int    pcie2_rdlh_link_up    : 1   ; /* [5]  */
        unsigned int    pcie2_cfg_rcb         : 1   ; /* [4]  */
        unsigned int    pcie2_cfg_mem_space_en : 1   ; /* [3]  */
        unsigned int    pcie2_pm_xtlh_block_tlp : 1   ; /* [2]  */
        unsigned int    pcie2_radmx_cmposer_lookup_err : 1   ; /* [1]  */
        unsigned int    pcie2_gm_cmposer_lookup_err : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_SYS_STATE0_U;

/* Define the union U_SC_PCIE2_SYS_STATE1_U */
/* pcie2×´Ì¬¼Ä´æÆ÷1 */
/* 0x6A0C */
typedef union tagScPcie2SysState1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_radm_msg_req_id_high : 8   ; /* [31..24]  */
        unsigned int    pcie2_cfg_sys_err_rc  : 1   ; /* [23]  */
        unsigned int    pcie2_cfg_aer_rc_err_msi : 1   ; /* [22]  */
        unsigned int    pcie2_link_req_rst_not_reg : 1   ; /* [21]  */
        unsigned int    pcie2_bridge_flush_not_reg : 1   ; /* [20]  */
        unsigned int    pcie2_cfg_pme_msi_reg : 1   ; /* [19]  */
        unsigned int    pcie2_radm_msg_unlock_reg : 1   ; /* [18]  */
        unsigned int    pcie2_radm_cpl_timeout_reg : 1   ; /* [17]  */
        unsigned int    pcie2_radm_pm_to_ack_reg : 1   ; /* [16]  */
        unsigned int    pcie2_radm_pm_to_pme_reg : 1   ; /* [15]  */
        unsigned int    pcie2_radm_fatal_err_reg : 1   ; /* [14]  */
        unsigned int    pcie2_radm_nonfatal_err_reg : 1   ; /* [13]  */
        unsigned int    pcie2_radm_correctabl_err_reg : 1   ; /* [12]  */
        unsigned int    pcie2_mac_phy_power_down : 2   ; /* [11..10]  */
        unsigned int    pcie2_pm_linkst_l2_exit : 1   ; /* [9]  */
        unsigned int    pcie2_pm_linkst_in_l2 : 1   ; /* [8]  */
        unsigned int    pcie2_pm_linkst_in_l1 : 1   ; /* [7]  */
        unsigned int    pcie2_app_parity_errs_reg : 3   ; /* [6..4]  */
        unsigned int    pcie2_axi_parity_errs_reg : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_SYS_STATE1_U;

/* Define the union U_SC_PCIE2_SYS_STATE4_U */
/* pcie2×´Ì¬¼Ä´æÆ÷4 */
/* 0x6A18 */
typedef union tagScPcie2SysState4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
        unsigned int    pcie2_link_req_rst_not : 1   ; /* [28]  */
        unsigned int    pcie2_bridge_flush_not : 1   ; /* [27]  */
        unsigned int    pcie2_radm_vendor_msg_reg : 1   ; /* [26]  */
        unsigned int    pcie2_cfg_pme_int_reg : 1   ; /* [25]  */
        unsigned int    pcie2_radm_intd_reg   : 1   ; /* [24]  */
        unsigned int    pcie2_radm_intc_reg   : 1   ; /* [23]  */
        unsigned int    pcie2_radm_intb_reg   : 1   ; /* [22]  */
        unsigned int    pcie2_radm_inta_reg   : 1   ; /* [21]  */
        unsigned int    pcie2_mstr_bresp_int  : 1   ; /* [20]  */
        unsigned int    pcie2_mstr_rresp_int  : 1   ; /* [19]  */
        unsigned int    pcie2_retry_sram_addr : 10  ; /* [18..9]  */
        unsigned int    pcie2_slv_err_int     : 1   ; /* [8]  */
        unsigned int    pcie2_mac_phy_rate    : 2   ; /* [7..6]  */
        unsigned int    pcie2_ltssm_state     : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_SYS_STATE4_U;

/* Define the union U_SC_PCIE2_AXI_MSTR_OOO_WR_STS1_U */
/* OOOÐ´Í¨µÀ¹¤×÷×´Ì¬ */
/* 0x6A84 */
typedef union tagScPcie2AxiMstrOooWrSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
        unsigned int    pcie2_curr_wr_port_sts : 1   ; /* [16]  */
        unsigned int    pcie2_curr_wr_latency : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_AXI_MSTR_OOO_WR_STS1_U;

/* Define the union U_SC_PCIE2_AXI_MSTR_OOO_RD_STS1_U */
/* OOO¶ÁÍ¨µÀ¹¤×÷×´Ì¬ */
/* 0x6A94 */
typedef union tagScPcie2AxiMstrOooRdSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
        unsigned int    pcie2_curr_rd_port_sts : 1   ; /* [16]  */
        unsigned int    pcie2_curr_rd_latency : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_AXI_MSTR_OOO_RD_STS1_U;

/* Define the union U_SC_PCIE2_DSIZE_BRG_ECC_ERR_U */
/* PCIE¶ÔÓ¦DSIZE BRGµÄECC´íÎó×´Ì¬ */
/* 0x6AA0 */
typedef union tagScPcie2DsizeBrgEccErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
        unsigned int    pcie2_rob_ecc_err_multpl : 1   ; /* [1]  */
        unsigned int    pcie2_rob_ecc_err_detect : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_DSIZE_BRG_ECC_ERR_U;

/* Define the union U_SC_PCIE2_PCIEPHY_CTRL_ERROR_U */
/* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
/* 0x6AC0 */
typedef union tagScPcie2PciephyCtrlError
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie2_pciephy_ctrl_error : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_PCIEPHY_CTRL_ERROR_U;

/* Define the union U_SC_PCIE3_SYS_STATE0_U */
/* pcie3×´Ì¬¼Ä´æÆ÷0 */
/* 0x6B08 */
typedef union tagScPcie3SysState0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_radm_msg_req_id_low : 8   ; /* [31..24]  */
        unsigned int    pcie3_p2_exit_reg     : 1   ; /* [23]  */
        unsigned int    pcie3_ref_clk_req_n   : 1   ; /* [22]  */
        unsigned int    pcie3_pm_status       : 1   ; /* [21]  */
        unsigned int    pcie3_hp_msi          : 1   ; /* [20]  */
        unsigned int    pcie3_hp_int          : 1   ; /* [19]  */
        unsigned int    pcie3_hp_pme          : 1   ; /* [18]  */
        unsigned int    pcie3_cfg_eml_control : 1   ; /* [17]  */
        unsigned int    pcie3_wake            : 1   ; /* [16]  */
        unsigned int    pcie3_xmlh_link_up    : 1   ; /* [15]  */
        unsigned int    pcie3_cfg_aer_int_msg_num : 5   ; /* [14..10]  */
        unsigned int    pcie3_cfg_aer_rc_err_int : 1   ; /* [9]  */
        unsigned int    pcie3_pm_curnt_state  : 3   ; /* [8..6]  */
        unsigned int    pcie3_rdlh_link_up    : 1   ; /* [5]  */
        unsigned int    pcie3_cfg_rcb         : 1   ; /* [4]  */
        unsigned int    pcie3_cfg_mem_space_en : 1   ; /* [3]  */
        unsigned int    pcie3_pm_xtlh_block_tlp : 1   ; /* [2]  */
        unsigned int    pcie3_radmx_cmposer_lookup_err : 1   ; /* [1]  */
        unsigned int    pcie3_gm_cmposer_lookup_err : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_SYS_STATE0_U;

/* Define the union U_SC_PCIE3_SYS_STATE1_U */
/* pcie3×´Ì¬¼Ä´æÆ÷1 */
/* 0x6B0C */
typedef union tagScPcie3SysState1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_radm_msg_req_id_high : 8   ; /* [31..24]  */
        unsigned int    pcie3_cfg_sys_err_rc  : 1   ; /* [23]  */
        unsigned int    pcie3_cfg_aer_rc_err_msi : 1   ; /* [22]  */
        unsigned int    pcie3_link_req_rst_not_reg : 1   ; /* [21]  */
        unsigned int    pcie3_bridge_flush_not_reg : 1   ; /* [20]  */
        unsigned int    pcie3_cfg_pme_msi_reg : 1   ; /* [19]  */
        unsigned int    pcie3_radm_msg_unlock_reg : 1   ; /* [18]  */
        unsigned int    pcie3_radm_cpl_timeout_reg : 1   ; /* [17]  */
        unsigned int    pcie3_radm_pm_to_ack_reg : 1   ; /* [16]  */
        unsigned int    pcie3_radm_pm_to_pme_reg : 1   ; /* [15]  */
        unsigned int    pcie3_radm_fatal_err_reg : 1   ; /* [14]  */
        unsigned int    pcie3_radm_nonfatal_err_reg : 1   ; /* [13]  */
        unsigned int    pcie3_radm_correctabl_err_reg : 1   ; /* [12]  */
        unsigned int    pcie3_mac_phy_power_down : 2   ; /* [11..10]  */
        unsigned int    pcie3_pm_linkst_l2_exit : 1   ; /* [9]  */
        unsigned int    pcie3_pm_linkst_in_l2 : 1   ; /* [8]  */
        unsigned int    pcie3_pm_linkst_in_l1 : 1   ; /* [7]  */
        unsigned int    pcie3_app_parity_errs_reg : 3   ; /* [6..4]  */
        unsigned int    pcie3_axi_parity_errs_reg : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_SYS_STATE1_U;

/* Define the union U_SC_PCIE3_SYS_STATE4_U */
/* pcie3×´Ì¬¼Ä´æÆ÷4 */
/* 0x6B18 */
typedef union tagScPcie3SysState4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
        unsigned int    pcie3_link_req_rst_not : 1   ; /* [28]  */
        unsigned int    pcie3_bridge_flush_not : 1   ; /* [27]  */
        unsigned int    pcie3_radm_vendor_msg_reg : 1   ; /* [26]  */
        unsigned int    pcie3_cfg_pme_int_reg : 1   ; /* [25]  */
        unsigned int    pcie3_radm_intd_reg   : 1   ; /* [24]  */
        unsigned int    pcie3_radm_intc_reg   : 1   ; /* [23]  */
        unsigned int    pcie3_radm_intb_reg   : 1   ; /* [22]  */
        unsigned int    pcie3_radm_inta_reg   : 1   ; /* [21]  */
        unsigned int    pcie3_mstr_bresp_int  : 1   ; /* [20]  */
        unsigned int    pcie3_mstr_rresp_int  : 1   ; /* [19]  */
        unsigned int    pcie3_retry_sram_addr : 10  ; /* [18..9]  */
        unsigned int    pcie3_slv_err_int     : 1   ; /* [8]  */
        unsigned int    pcie3_mac_phy_rate    : 2   ; /* [7..6]  */
        unsigned int    pcie3_ltssm_state     : 6   ; /* [5..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_SYS_STATE4_U;

/* Define the union U_SC_PCIE3_PCIEPHY_CTRL_ERROR_U */
/* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
/* 0x6BC0 */
typedef union tagScPcie3PciephyCtrlError
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    pcie3_pciephy_ctrl_error : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_PCIEPHY_CTRL_ERROR_U;

/* Define the union U_SC_SKEW_ST_A_0_U */
/*  */
/* 0x7500 */
typedef union tagScSkewStA0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    skew_lock_a           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_A_0_U;

/* Define the union U_SC_SKEW_ST_A_1_U */
/*  */
/* 0x7504 */
typedef union tagScSkewStA1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    skew_varible_out_a    : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_A_1_U;

/* Define the union U_SC_SKEW_ST_A_3_U */
/*  */
/* 0x750C */
typedef union tagScSkewStA3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    skew_dcell_out_a_h    : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_A_3_U;

/* Define the union U_SC_SKEW_ST_B_0_U */
/*  */
/* 0x7600 */
typedef union tagScSkewStB0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
        unsigned int    skew_lock_b           : 1   ; /* [0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_B_0_U;

/* Define the union U_SC_SKEW_ST_B_1_U */
/*  */
/* 0x7604 */
typedef union tagScSkewStB1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
        unsigned int    skew_varible_out_b    : 16  ; /* [15..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_B_1_U;

/* Define the union U_SC_SKEW_ST_B_3_U */
/*  */
/* 0x760C */
typedef union tagScSkewStB3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
        unsigned int    skew_dcell_out_b_h    : 4   ; /* [3..0]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_B_3_U;


#else

/* Define the union U_SC_PCIE0_CLK_EN_U */
/* PCIE0Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x300 */
typedef union tagScPcie0ClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie0_enb         : 1   ; /* [0]  */
        unsigned int    clk_pcie0_pipe_enb    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_CLK_EN_U;

/* Define the union U_SC_PCIE0_CLK_DIS_U */
/* PCIE0Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x304 */
typedef union tagScPcie0ClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie0_dsb         : 1   ; /* [0]  */
        unsigned int    clk_pcie0_pipe_dsb    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_CLK_DIS_U;

/* Define the union U_SC_PCIE1_CLK_EN_U */
/* PCIE1Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x308 */
typedef union tagScPcie1ClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie1_enb         : 1   ; /* [0]  */
        unsigned int    clk_pcie1_pipe_enb    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_CLK_EN_U;

/* Define the union U_SC_PCIE1_CLK_DIS_U */
/* PCIE1Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x30C */
typedef union tagScPcie1ClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie1_dsb         : 1   ; /* [0]  */
        unsigned int    clk_pcie1_pipe_dsb    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_CLK_DIS_U;

/* Define the union U_SC_PCIE2_CLK_EN_U */
/* PCIE2Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x310 */
typedef union tagScPcie2ClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie2_enb         : 1   ; /* [0]  */
        unsigned int    clk_pcie2_pipe_enb    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_CLK_EN_U;

/* Define the union U_SC_PCIE2_CLK_DIS_U */
/* PCIE2Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x314 */
typedef union tagScPcie2ClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie2_dsb         : 1   ; /* [0]  */
        unsigned int    clk_pcie2_pipe_dsb    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_CLK_DIS_U;

/* Define the union U_SC_SAS_CLK_EN_U */
/* SASÊ±ÖÓÃÅ¿ØÐÅºÅ */
/* 0x318 */
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
/* 0x31C */
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

/* Define the union U_SC_PCIE3_CLK_EN_U */
/* PCIE3Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x320 */
typedef union tagScPcie3ClkEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie3_enb         : 1   ; /* [0]  */
        unsigned int    clk_pcie3_pipe_enb    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_CLK_EN_U;

/* Define the union U_SC_PCIE3_CLK_DIS_U */
/* PCIE3Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
/* 0x324 */
typedef union tagScPcie3ClkDis
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie3_dsb         : 1   ; /* [0]  */
        unsigned int    clk_pcie3_pipe_dsb    : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_CLK_DIS_U;

/* Define the union U_SC_ITS_CLK_EN_U */
/* ITSÊ±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
/* 0x328 */
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
/* 0x32C */
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

/* Define the union U_SC_SLLC_CLK_EN_U */
/* SLLC0Ê±ÖÓÊ¹ÄÜ¼Ä´æÆ÷ */
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
/* SLLC0Ê±ÖÓ½ûÖ¹¼Ä´æÆ÷ */
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

/* Define the union U_SC_PCIE0_RESET_REQ_U */
/* PCIE0Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA00 */
typedef union tagScPcie0ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_RESET_REQ_U;

/* Define the union U_SC_PCIE0_RESET_DREQ_U */
/* PCIE0Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA04 */
typedef union tagScPcie0ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_RESET_DREQ_U;

/* Define the union U_SC_PCIE1_RESET_REQ_U */
/* PCIE1Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA08 */
typedef union tagScPcie1ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_RESET_REQ_U;

/* Define the union U_SC_PCIE1_RESET_DREQ_U */
/* PCIE1Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA0C */
typedef union tagScPcie1ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_RESET_DREQ_U;

/* Define the union U_SC_PCIE2_RESET_REQ_U */
/* PCIE2Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA10 */
typedef union tagScPcie2ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_RESET_REQ_U;

/* Define the union U_SC_PCIE2_RESET_DREQ_U */
/* PCIE2Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA14 */
typedef union tagScPcie2ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_RESET_DREQ_U;

/* Define the union U_SC_SAS_RESET_REQ_U */
/* SASÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA18 */
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
/* SASÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA1C */
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

/* Define the union U_SC_MCTP0_RESET_REQ_U */
/* MCTP0Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA20 */
typedef union tagScMctp0ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp0_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP0_RESET_REQ_U;

/* Define the union U_SC_MCTP0_RESET_DREQ_U */
/* MCTP0Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA24 */
typedef union tagScMctp0ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp0_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP0_RESET_DREQ_U;

/* Define the union U_SC_MCTP1_RESET_REQ_U */
/* MCTP1Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA28 */
typedef union tagScMctp1ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp1_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP1_RESET_REQ_U;

/* Define the union U_SC_MCTP1_RESET_DREQ_U */
/* MCTP1Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA2C */
typedef union tagScMctp1ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp1_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP1_RESET_DREQ_U;

/* Define the union U_SC_MCTP2_RESET_REQ_U */
/* MCTP2Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA30 */
typedef union tagScMctp2ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp2_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP2_RESET_REQ_U;

/* Define the union U_SC_MCTP2_RESET_DREQ_U */
/* MCTP2Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0xA34 */
typedef union tagScMctp2ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp2_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP2_RESET_DREQ_U;

/* Define the union U_SC_SLLC_TSVRX_RESET_REQ_U */
/* SLLC TSVRXÍ¨µÀÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA58 */
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
/* 0xA5C */
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

/* Define the union U_SC_PCIE_HILINK_PCS_RESET_REQ_U */
/* PCIE HILINK PCSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA60 */
typedef union tagScPcieHilinkPcsResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_hilink_pcs_lane0_srst_req : 1   ; /* [0]  */
        unsigned int    pcie0_hilink_pcs_lane1_srst_req : 1   ; /* [1]  */
        unsigned int    pcie0_hilink_pcs_lane2_srst_req : 1   ; /* [2]  */
        unsigned int    pcie0_hilink_pcs_lane3_srst_req : 1   ; /* [3]  */
        unsigned int    pcie0_hilink_pcs_lane4_srst_req : 1   ; /* [4]  */
        unsigned int    pcie0_hilink_pcs_lane5_srst_req : 1   ; /* [5]  */
        unsigned int    pcie0_hilink_pcs_lane6_srst_req : 1   ; /* [6]  */
        unsigned int    pcie0_hilink_pcs_lane7_srst_req : 1   ; /* [7]  */
        unsigned int    pcie1_hilink_pcs_lane0_srst_req : 1   ; /* [8]  */
        unsigned int    pcie1_hilink_pcs_lane1_srst_req : 1   ; /* [9]  */
        unsigned int    pcie1_hilink_pcs_lane2_srst_req : 1   ; /* [10]  */
        unsigned int    pcie1_hilink_pcs_lane3_srst_req : 1   ; /* [11]  */
        unsigned int    pcie1_hilink_pcs_lane4_srst_req : 1   ; /* [12]  */
        unsigned int    pcie1_hilink_pcs_lane5_srst_req : 1   ; /* [13]  */
        unsigned int    pcie1_hilink_pcs_lane6_srst_req : 1   ; /* [14]  */
        unsigned int    pcie1_hilink_pcs_lane7_srst_req : 1   ; /* [15]  */
        unsigned int    pcie2_hilink_pcs_lane0_srst_req : 1   ; /* [16]  */
        unsigned int    pcie2_hilink_pcs_lane1_srst_req : 1   ; /* [17]  */
        unsigned int    pcie2_hilink_pcs_lane2_srst_req : 1   ; /* [18]  */
        unsigned int    pcie2_hilink_pcs_lane3_srst_req : 1   ; /* [19]  */
        unsigned int    pcie2_hilink_pcs_lane4_srst_req : 1   ; /* [20]  */
        unsigned int    pcie2_hilink_pcs_lane5_srst_req : 1   ; /* [21]  */
        unsigned int    pcie2_hilink_pcs_lane6_srst_req : 1   ; /* [22]  */
        unsigned int    pcie2_hilink_pcs_lane7_srst_req : 1   ; /* [23]  */
        unsigned int    pcie3_hilink_pcs_lane0_srst_req : 1   ; /* [24]  */
        unsigned int    pcie3_hilink_pcs_lane1_srst_req : 1   ; /* [25]  */
        unsigned int    pcie3_hilink_pcs_lane2_srst_req : 1   ; /* [26]  */
        unsigned int    pcie3_hilink_pcs_lane3_srst_req : 1   ; /* [27]  */
        unsigned int    pcie3_hilink_pcs_lane4_srst_req : 1   ; /* [28]  */
        unsigned int    pcie3_hilink_pcs_lane5_srst_req : 1   ; /* [29]  */
        unsigned int    pcie3_hilink_pcs_lane6_srst_req : 1   ; /* [30]  */
        unsigned int    pcie3_hilink_pcs_lane7_srst_req : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_HILINK_PCS_RESET_REQ_U;

/* Define the union U_SC_PCIE_HILINK_PCS_RESET_DREQ_U */
/* PCIE HILINK PCSÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA64 */
typedef union tagScPcieHilinkPcsResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_hilink_pcs_lane0_srst_dreq : 1   ; /* [0]  */
        unsigned int    pcie0_hilink_pcs_lane1_srst_dreq : 1   ; /* [1]  */
        unsigned int    pcie0_hilink_pcs_lane2_srst_dreq : 1   ; /* [2]  */
        unsigned int    pcie0_hilink_pcs_lane3_srst_dreq : 1   ; /* [3]  */
        unsigned int    pcie0_hilink_pcs_lane4_srst_dreq : 1   ; /* [4]  */
        unsigned int    pcie0_hilink_pcs_lane5_srst_dreq : 1   ; /* [5]  */
        unsigned int    pcie0_hilink_pcs_lane6_srst_dreq : 1   ; /* [6]  */
        unsigned int    pcie0_hilink_pcs_lane7_srst_dreq : 1   ; /* [7]  */
        unsigned int    pcie1_hilink_pcs_lane0_srst_dreq : 1   ; /* [8]  */
        unsigned int    pcie1_hilink_pcs_lane1_srst_dreq : 1   ; /* [9]  */
        unsigned int    pcie1_hilink_pcs_lane2_srst_dreq : 1   ; /* [10]  */
        unsigned int    pcie1_hilink_pcs_lane3_srst_dreq : 1   ; /* [11]  */
        unsigned int    pcie1_hilink_pcs_lane4_srst_dreq : 1   ; /* [12]  */
        unsigned int    pcie1_hilink_pcs_lane5_srst_dreq : 1   ; /* [13]  */
        unsigned int    pcie1_hilink_pcs_lane6_srst_dreq : 1   ; /* [14]  */
        unsigned int    pcie1_hilink_pcs_lane7_srst_dreq : 1   ; /* [15]  */
        unsigned int    pcie2_hilink_pcs_lane0_srst_dreq : 1   ; /* [16]  */
        unsigned int    pcie2_hilink_pcs_lane1_srst_dreq : 1   ; /* [17]  */
        unsigned int    pcie2_hilink_pcs_lane2_srst_dreq : 1   ; /* [18]  */
        unsigned int    pcie2_hilink_pcs_lane3_srst_dreq : 1   ; /* [19]  */
        unsigned int    pcie2_hilink_pcs_lane4_srst_dreq : 1   ; /* [20]  */
        unsigned int    pcie2_hilink_pcs_lane5_srst_dreq : 1   ; /* [21]  */
        unsigned int    pcie2_hilink_pcs_lane6_srst_dreq : 1   ; /* [22]  */
        unsigned int    pcie2_hilink_pcs_lane7_srst_dreq : 1   ; /* [23]  */
        unsigned int    pcie3_hilink_pcs_lane0_srst_dreq : 1   ; /* [24]  */
        unsigned int    pcie3_hilink_pcs_lane1_srst_dreq : 1   ; /* [25]  */
        unsigned int    pcie3_hilink_pcs_lane2_srst_dreq : 1   ; /* [26]  */
        unsigned int    pcie3_hilink_pcs_lane3_srst_dreq : 1   ; /* [27]  */
        unsigned int    pcie3_hilink_pcs_lane4_srst_dreq : 1   ; /* [28]  */
        unsigned int    pcie3_hilink_pcs_lane5_srst_dreq : 1   ; /* [29]  */
        unsigned int    pcie3_hilink_pcs_lane6_srst_dreq : 1   ; /* [30]  */
        unsigned int    pcie3_hilink_pcs_lane7_srst_dreq : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_HILINK_PCS_RESET_DREQ_U;

/* Define the union U_SC_PCIE3_RESET_REQ_U */
/* PCIE3Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA68 */
typedef union tagScPcie3ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_RESET_REQ_U;

/* Define the union U_SC_PCIE3_RESET_DREQ_U */
/* PCIE3Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA6C */
typedef union tagScPcie3ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_RESET_DREQ_U;

/* Define the union U_SC_MCTP3_RESET_REQ_U */
/* MCTP3Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA70 */
typedef union tagScMctp3ResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp3_srst_req        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP3_RESET_REQ_U;

/* Define the union U_SC_MCTP3_RESET_DREQ_U */
/* MCTP3Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA74 */
typedef union tagScMctp3ResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp3_srst_dreq       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP3_RESET_DREQ_U;

/* Define the union U_SC_ITS_RESET_REQ_U */
/* ITSÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xA80 */
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
/* 0xA84 */
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

/* Define the union U_SC_SLLC_RESET_REQ_U */
/* SLLC0Èí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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
/* SLLC0Èí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
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

/* Define the union U_SC_PCS_LOCAL_RESET_REQ_U */
/* PCS LOCALÈí¸´Î»ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAC0 */
typedef union tagScPcsLocalResetReq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_pcs_local_srst_req : 1   ; /* [0]  */
        unsigned int    pcie1_pcs_local_srst_req : 1   ; /* [1]  */
        unsigned int    pcie2_pcs_local_srst_req : 1   ; /* [2]  */
        unsigned int    pcie3_pcs_local_srst_req : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_REQ_U;

/* Define the union U_SC_PCS_LOCAL_RESET_DREQ_U */
/* PCS LOCALÈí¸´Î»È¥ÇëÇó¿ØÖÆ¼Ä´æÆ÷ */
/* 0xAC4 */
typedef union tagScPcsLocalResetDreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_pcs_local_srst_dreq : 1   ; /* [0]  */
        unsigned int    pcie1_pcs_local_srst_dreq : 1   ; /* [1]  */
        unsigned int    pcie2_pcs_local_srst_dreq : 1   ; /* [2]  */
        unsigned int    pcie3_pcs_local_srst_dreq : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_DREQ_U;

/* Define the union U_SC_DISP_DAW_EN_U */
/* dispatch daw enÅäÖÃ */
/* 0x1000 */
typedef union tagScDispDawEn
{
    /* Define the struct bits */
    struct
    {
        unsigned int    dispatch_daw_en       : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_DISP_DAW_EN_U;

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

/* Define the union U_SC_FTE_MUX_CTRL_U */
/*  */
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

/* Define the union U_SC_HILINK0_MUX_CTRL_U */
/* HILINK¸´ÓÃÑ¡Ôñ */
/* 0x2300 */
typedef union tagScHilink0MuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_mux_sel       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MUX_CTRL_U;

/* Define the union U_SC_HILINK1_MUX_CTRL_U */
/* HILINK¸´ÓÃÑ¡Ôñ */
/* 0x2304 */
typedef union tagScHilink1MuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_mux_sel       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MUX_CTRL_U;

/* Define the union U_SC_HILINK2_MUX_CTRL_U */
/* HILINK¸´ÓÃÑ¡Ôñ */
/* 0x2308 */
typedef union tagScHilink2MuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_mux_sel       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_MUX_CTRL_U;

/* Define the union U_SC_HILINK5_MUX_CTRL_U */
/* HILINK¸´ÓÃÑ¡Ôñ */
/* 0x2314 */
typedef union tagScHilink5MuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_mux_sel       : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MUX_CTRL_U;

/* Define the union U_SC_HILINK1_AHB_MUX_CTRL_U */
/* HILINK AHB¸´ÓÃÑ¡Ôñ */
/* 0x2324 */
typedef union tagScHilink1AhbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_ahb_mux_sel   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_AHB_MUX_CTRL_U;

/* Define the union U_SC_HILINK2_AHB_MUX_CTRL_U */
/* HILINK AHB¸´ÓÃÑ¡Ôñ */
/* 0x2328 */
typedef union tagScHilink2AhbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink2_ahb_mux_sel   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK2_AHB_MUX_CTRL_U;

/* Define the union U_SC_HILINK5_AHB_MUX_CTRL_U */
/* HILINK AHB¸´ÓÃÑ¡Ôñ */
/* 0x2334 */
typedef union tagScHilink5AhbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_ahb_mux_sel   : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_AHB_MUX_CTRL_U;

/* Define the union U_SC_HILINK5_LRSTB_MUX_CTRL_U */
/* HILINK5 lrstb[3:0]µÄMUXÑ¡Ôñ¿ØÖÆ */
/* 0x2340 */
typedef union tagScHilink5LrstbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_lrstb_mux_sel : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_LRSTB_MUX_CTRL_U;

/* Define the union U_SC_HILINK6_LRSTB_MUX_CTRL_U */
/* HILINK6 lrstb[3:0]µÄMUXÑ¡Ôñ¿ØÖÆ */
/* 0x2344 */
typedef union tagScHilink6LrstbMuxCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_lrstb_mux_sel : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_LRSTB_MUX_CTRL_U;

/* Define the union U_SC_HILINK0_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2400 */
typedef union tagScHilink0MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_ss_refclk0_x2s : 2   ; /* [1..0]  */
        unsigned int    hilink0_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink0_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink0_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink0_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink0_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink0_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink0_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK0_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2404 */
typedef union tagScHilink0MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
        unsigned int    hilink0_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink0_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink0_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink0_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink0_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink0_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink0_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink0_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink0_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK0_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2408 */
typedef union tagScHilink0MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_lifeclk2dig_sel : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK0_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x240C */
typedef union tagScHilink0MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_core_clk_selext : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK0_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2410 */
typedef union tagScHilink0MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_core_clk_sel  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK0_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2414 */
typedef union tagScHilink0MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_ctrl_bus_mode : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK0_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2418 */
typedef union tagScHilink0MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_macropwrdb    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK0_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x241C */
typedef union tagScHilink0MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_grstb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK0_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2420 */
typedef union tagScHilink0MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_bit_slip      : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK0_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2424 */
typedef union tagScHilink0MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_lrstb         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_LRSTB_U;

/* Define the union U_SC_HILINK1_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2500 */
typedef union tagScHilink1MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_ss_refclk0_x2s : 2   ; /* [1..0]  */
        unsigned int    hilink1_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink1_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink1_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink1_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink1_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink1_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink1_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK1_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2504 */
typedef union tagScHilink1MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
        unsigned int    hilink1_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink1_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink1_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink1_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink1_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink1_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink1_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink1_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink1_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK1_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2508 */
typedef union tagScHilink1MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_lifeclk2dig_sel : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK1_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x250C */
typedef union tagScHilink1MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_core_clk_selext : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK1_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2510 */
typedef union tagScHilink1MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_core_clk_sel  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK1_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2514 */
typedef union tagScHilink1MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_ctrl_bus_mode : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK1_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2518 */
typedef union tagScHilink1MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_macropwrdb    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK1_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x251C */
typedef union tagScHilink1MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_grstb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK1_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2520 */
typedef union tagScHilink1MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_bit_slip      : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK1_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2524 */
typedef union tagScHilink1MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_lrstb         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_LRSTB_U;

/* Define the union U_SC_HILINK5_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2600 */
typedef union tagScHilink5MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_ss_refclk0_x2s : 2   ; /* [1..0]  */
        unsigned int    hilink5_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink5_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink5_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink5_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink5_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink5_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink5_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK5_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2604 */
typedef union tagScHilink5MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
        unsigned int    hilink5_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink5_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink5_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink5_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink5_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink5_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink5_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink5_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink5_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK5_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2608 */
typedef union tagScHilink5MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_lifeclk2dig_sel : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK5_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x260C */
typedef union tagScHilink5MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_core_clk_selext : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK5_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2610 */
typedef union tagScHilink5MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_core_clk_sel  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK5_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2614 */
typedef union tagScHilink5MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_ctrl_bus_mode : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK5_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2618 */
typedef union tagScHilink5MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_macropwrdb    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK5_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x261C */
typedef union tagScHilink5MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_grstb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK5_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2620 */
typedef union tagScHilink5MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_bit_slip      : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK5_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2624 */
typedef union tagScHilink5MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_lrstb         : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_LRSTB_U;

/* Define the union U_SC_HILINK6_MACRO_SS_REFCLK_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2700 */
typedef union tagScHilink6MacroSsRefclk
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_ss_refclk0_x2s : 2   ; /* [1..0]  */
        unsigned int    hilink6_ss_refclk0_x2n : 2   ; /* [3..2]  */
        unsigned int    hilink6_ss_refclk0_x2e : 2   ; /* [5..4]  */
        unsigned int    hilink6_ss_refclk0_x2w : 2   ; /* [7..6]  */
        unsigned int    hilink6_ss_refclk1_x2s : 2   ; /* [9..8]  */
        unsigned int    hilink6_ss_refclk1_x2n : 2   ; /* [11..10]  */
        unsigned int    hilink6_ss_refclk1_x2e : 2   ; /* [13..12]  */
        unsigned int    hilink6_ss_refclk1_x2w : 2   ; /* [15..14]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_SS_REFCLK_U;

/* Define the union U_SC_HILINK6_MACRO_CS_REFCLK_DIRSEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2704 */
typedef union tagScHilink6MacroCsRefclkDirsel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_cs_refclk0_dirsel0 : 2   ; /* [1..0]  */
        unsigned int    hilink6_cs_refclk0_dirsel1 : 2   ; /* [3..2]  */
        unsigned int    hilink6_cs_refclk0_dirsel2 : 2   ; /* [5..4]  */
        unsigned int    hilink6_cs_refclk0_dirsel3 : 2   ; /* [7..6]  */
        unsigned int    hilink6_cs_refclk0_dirsel4 : 2   ; /* [9..8]  */
        unsigned int    hilink6_cs_refclk1_dirsel0 : 2   ; /* [11..10]  */
        unsigned int    hilink6_cs_refclk1_dirsel1 : 2   ; /* [13..12]  */
        unsigned int    hilink6_cs_refclk1_dirsel2 : 2   ; /* [15..14]  */
        unsigned int    hilink6_cs_refclk1_dirsel3 : 2   ; /* [17..16]  */
        unsigned int    hilink6_cs_refclk1_dirsel4 : 2   ; /* [19..18]  */
        unsigned int    reserved_0            : 12  ; /* [31..20]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_CS_REFCLK_DIRSEL_U;

/* Define the union U_SC_HILINK6_MACRO_LIFECLK2DIG_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2708 */
typedef union tagScHilink6MacroLifeclk2digSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_lifeclk2dig_sel : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_LIFECLK2DIG_SEL_U;

/* Define the union U_SC_HILINK6_MACRO_CORE_CLK_SELEXT_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x270C */
typedef union tagScHilink6MacroCoreClkSelext
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_core_clk_selext : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_CORE_CLK_SELEXT_U;

/* Define the union U_SC_HILINK6_MACRO_CORE_CLK_SEL_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2710 */
typedef union tagScHilink6MacroCoreClkSel
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_core_clk_sel  : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_CORE_CLK_SEL_U;

/* Define the union U_SC_HILINK6_MACRO_CTRL_BUS_MODE_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2714 */
typedef union tagScHilink6MacroCtrlBusMode
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_ctrl_bus_mode : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_CTRL_BUS_MODE_U;

/* Define the union U_SC_HILINK6_MACRO_MACROPWRDB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2718 */
typedef union tagScHilink6MacroMacropwrdb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_macropwrdb    : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_MACROPWRDB_U;

/* Define the union U_SC_HILINK6_MACRO_GRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x271C */
typedef union tagScHilink6MacroGrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_grstb         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_GRSTB_U;

/* Define the union U_SC_HILINK6_MACRO_BIT_SLIP_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2720 */
typedef union tagScHilink6MacroBitSlip
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_bit_slip      : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_BIT_SLIP_U;

/* Define the union U_SC_HILINK6_MACRO_LRSTB_U */
/* HILINKÅäÖÃ¼Ä´æÆ÷ */
/* 0x2724 */
typedef union tagScHilink6MacroLrstb
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_lrstb         : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_LRSTB_U;

/* Define the union U_SC_PCIE0_CLKREQ_U */
/*  */
/* 0x2800 */
typedef union tagScPcie0Clkreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_phy_clk_req_n   : 1   ; /* [0]  */
        unsigned int    pcie0_apb_cfg_sel     : 2   ; /* [2..1]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_CLKREQ_U;

/* Define the union U_SC_PCIE0_AXI_MSTR_OOO_WR_CFG_U */
/*  */
/* 0x2880 */
typedef union tagScPcie0AxiMstrOooWrCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_cfg_max_wr_trans : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie0_wr_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    pcie0_ctrl_lat_stat_wr_en : 1   ; /* [12]  */
        unsigned int    reserved_1            : 3   ; /* [15..13]  */
        unsigned int    pcie0_cfg_violate_posted_req_order : 1   ; /* [16]  */
        unsigned int    reserved_2            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_AXI_MSTR_OOO_WR_CFG_U;

/* Define the union U_SC_PCIE0_AXI_MSTR_OOO_RD_CFG_U */
/*  */
/* 0x2890 */
typedef union tagScPcie0AxiMstrOooRdCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_cfg_max_rd_trans : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie0_rd_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    pcie0_ctrl_lat_stat_rd_en : 1   ; /* [12]  */
        unsigned int    reserved_1            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_AXI_MSTR_OOO_RD_CFG_U;

/* Define the union U_SC_PCIE1_CLKREQ_U */
/*  */
/* 0x2900 */
typedef union tagScPcie1Clkreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1hilink_phy_clk_req_n : 1   ; /* [0]  */
        unsigned int    pcie1vsemi_phy_clk_req_n : 1   ; /* [1]  */
        unsigned int    pcie1_apb_cfg_sel     : 2   ; /* [3..2]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_CLKREQ_U;

/* Define the union U_SC_PCIE1_AXI_MSTR_OOO_WR_CFG_U */
/*  */
/* 0x2980 */
typedef union tagScPcie1AxiMstrOooWrCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_cfg_max_wr_trans : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie1_wr_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    pcie1_ctrl_lat_stat_wr_en : 1   ; /* [12]  */
        unsigned int    reserved_1            : 3   ; /* [15..13]  */
        unsigned int    pcie1_cfg_violate_posted_req_order : 1   ; /* [16]  */
        unsigned int    reserved_2            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_AXI_MSTR_OOO_WR_CFG_U;

/* Define the union U_SC_PCIE1_AXI_MSTR_OOO_RD_CFG_U */
/*  */
/* 0x2990 */
typedef union tagScPcie1AxiMstrOooRdCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_cfg_max_rd_trans : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie1_rd_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    pcie1_ctrl_lat_stat_rd_en : 1   ; /* [12]  */
        unsigned int    reserved_1            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_AXI_MSTR_OOO_RD_CFG_U;

/* Define the union U_SC_PCIE2_CLKREQ_U */
/*  */
/* 0x2A00 */
typedef union tagScPcie2Clkreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2hilink_phy_clk_req_n : 1   ; /* [0]  */
        unsigned int    pcie2vsemi_phy_clk_req_n : 1   ; /* [1]  */
        unsigned int    pcie2_apb_cfg_sel     : 2   ; /* [3..2]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_CLKREQ_U;

/* Define the union U_SC_PCIE2_AXI_MSTR_OOO_WR_CFG_U */
/*  */
/* 0x2A80 */
typedef union tagScPcie2AxiMstrOooWrCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_cfg_max_wr_trans : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie2_wr_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    pcie2_ctrl_lat_stat_wr_en : 1   ; /* [12]  */
        unsigned int    reserved_1            : 3   ; /* [15..13]  */
        unsigned int    pcie2_cfg_violate_posted_req_order : 1   ; /* [16]  */
        unsigned int    reserved_2            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_AXI_MSTR_OOO_WR_CFG_U;

/* Define the union U_SC_PCIE2_AXI_MSTR_OOO_RD_CFG_U */
/*  */
/* 0x2A90 */
typedef union tagScPcie2AxiMstrOooRdCfg
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_cfg_max_rd_trans : 6   ; /* [5..0]  */
        unsigned int    reserved_0            : 2   ; /* [7..6]  */
        unsigned int    pcie2_rd_rate_limit   : 4   ; /* [11..8]  */
        unsigned int    pcie2_ctrl_lat_stat_rd_en : 1   ; /* [12]  */
        unsigned int    reserved_1            : 19  ; /* [31..13]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_AXI_MSTR_OOO_RD_CFG_U;

/* Define the union U_SC_PCIE3_CLKREQ_U */
/*  */
/* 0x2B00 */
typedef union tagScPcie3Clkreq
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_phy_clk_req_n   : 1   ; /* [0]  */
        unsigned int    pcie3_apb_cfg_sel     : 2   ; /* [2..1]  */
        unsigned int    reserved_0            : 29  ; /* [31..3]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_CLKREQ_U;

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

/* Define the union U_SC_SLLC0_MEM_CTRL_U */
/* sllc0 mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3010 */
typedef union tagScSllc0MemCtrl
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rft_sllc0        : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SLLC0_MEM_CTRL_U;

/* Define the union U_SC_SAS_MEM_CTRL_U */
/* sas mem¿ØÖÆ¼Ä´æÆ÷ */
/* 0x3030 */
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

/* Define the union U_SC_PCIE_MEM_CTRL0_U */
/* pcie mem¿ØÖÆ¼Ä´æÆ÷0 */
/* 0x3040 */
typedef union tagScPcieMemCtrl0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rft_pcie         : 10  ; /* [9..0]  */
        unsigned int    reserved_0            : 22  ; /* [31..10]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_MEM_CTRL0_U;

/* Define the union U_SC_PCIE_MEM_CTRL1_U */
/* pcie mem¿ØÖÆ¼Ä´æÆ÷1 */
/* 0x3044 */
typedef union tagScPcieMemCtrl1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rashsd_pcie      : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_MEM_CTRL1_U;

/* Define the union U_SC_PCIE_MEM_CTRL2_U */
/* pcie mem¿ØÖÆ¼Ä´æÆ÷2 */
/* 0x3048 */
typedef union tagScPcieMemCtrl2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    ctrl_rfs_pcie         : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_MEM_CTRL2_U;

/* Define the union U_SC_SKEW_COMMON_0_U */
/*  */
/* 0x3400 */
typedef union tagScSkewCommon0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_en               : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_COMMON_0_U;

/* Define the union U_SC_SKEW_COMMON_1_U */
/*  */
/* 0x3404 */
typedef union tagScSkewCommon1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_addr_offset      : 5   ; /* [4..0]  */
        unsigned int    reserved_0            : 27  ; /* [31..5]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_COMMON_1_U;

/* Define the union U_SC_SKEW_COMMON_2_U */
/*  */
/* 0x3408 */
typedef union tagScSkewCommon2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_config_in        : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_COMMON_2_U;

/* Define the union U_SC_SKEW_A_0_U */
/*  */
/* 0x3500 */
typedef union tagScSkewA0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_bypass_a         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_0_U;

/* Define the union U_SC_SKEW_A_1_U */
/*  */
/* 0x3504 */
typedef union tagScSkewA1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_config_in_a      : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_1_U;

/* Define the union U_SC_SKEW_A_2_U */
/*  */
/* 0x3508 */
typedef union tagScSkewA2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_out_delay_sel_a  : 2   ; /* [1..0]  */
        unsigned int    skew_in_delay_sel_a   : 2   ; /* [3..2]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_2_U;

/* Define the union U_SC_SKEW_A_3_U */
/*  */
/* 0x350C */
typedef union tagScSkewA3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_sel_a_1          : 1   ; /* [0]  */
        unsigned int    skew_sel_a_0          : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_3_U;

/* Define the union U_SC_SKEW_A_4_U */
/*  */
/* 0x3510 */
typedef union tagScSkewA4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_update_en_a      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_4_U;

/* Define the union U_SC_SKEW_A_5_U */
/*  */
/* 0x3514 */
typedef union tagScSkewA5
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_varible_set_a    : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_5_U;

/* Define the union U_SC_SKEW_A_7_U */
/*  */
/* 0x351C */
typedef union tagScSkewA7
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_dcell_set_a_h    : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_7_U;

/* Define the union U_SC_SKEW_A_8_U */
/*  */
/* 0x3520 */
typedef union tagScSkewA8
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_sel_osc_a        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_A_8_U;

/* Define the union U_SC_SKEW_B_0_U */
/*  */
/* 0x3600 */
typedef union tagScSkewB0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_bypass_b         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_0_U;

/* Define the union U_SC_SKEW_B_1_U */
/*  */
/* 0x3604 */
typedef union tagScSkewB1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_config_in_b      : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_1_U;

/* Define the union U_SC_SKEW_B_2_U */
/*  */
/* 0x3608 */
typedef union tagScSkewB2
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_out_delay_sel_b  : 2   ; /* [1..0]  */
        unsigned int    skew_in_delay_sel_b   : 2   ; /* [3..2]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_2_U;

/* Define the union U_SC_SKEW_B_3_U */
/*  */
/* 0x360C */
typedef union tagScSkewB3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_sel_b_1          : 1   ; /* [0]  */
        unsigned int    skew_sel_b_0          : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_3_U;

/* Define the union U_SC_SKEW_B_4_U */
/*  */
/* 0x3610 */
typedef union tagScSkewB4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_update_en_b      : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_4_U;

/* Define the union U_SC_SKEW_B_5_U */
/*  */
/* 0x3614 */
typedef union tagScSkewB5
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_varible_set_b    : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_5_U;

/* Define the union U_SC_SKEW_B_7_U */
/*  */
/* 0x361C */
typedef union tagScSkewB7
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_dcell_set_b_h    : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_7_U;

/* Define the union U_SC_SKEW_B_8_U */
/*  */
/* 0x3620 */
typedef union tagScSkewB8
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_sel_osc_b        : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_B_8_U;

/* Define the union U_SC_PCIE0_CLK_ST_U */
/* PCIE0Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5300 */
typedef union tagScPcie0ClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie0_st          : 1   ; /* [0]  */
        unsigned int    clk_pcie0_pipe_st     : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_CLK_ST_U;

/* Define the union U_SC_PCIE1_CLK_ST_U */
/* PCIE1Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5304 */
typedef union tagScPcie1ClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie1_st          : 1   ; /* [0]  */
        unsigned int    clk_pcie1_pipe_st     : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_CLK_ST_U;

/* Define the union U_SC_PCIE2_CLK_ST_U */
/* PCIE2Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5308 */
typedef union tagScPcie2ClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie2_st          : 1   ; /* [0]  */
        unsigned int    clk_pcie2_pipe_st     : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_CLK_ST_U;

/* Define the union U_SC_SAS_CLK_ST_U */
/* SASÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x530C */
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

/* Define the union U_SC_PCIE3_CLK_ST_U */
/* PCIE3Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5310 */
typedef union tagScPcie3ClkSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    clk_pcie3_st          : 1   ; /* [0]  */
        unsigned int    clk_pcie3_pipe_st     : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_CLK_ST_U;

/* Define the union U_SC_ITS_CLK_ST_U */
/* ITSÊ±ÖÓ×´Ì¬¼Ä´æÆ÷ */
/* 0x5314 */
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

/* Define the union U_SC_SLLC_CLK_ST_U */
/* SLLC0Ê±ÖÓ×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_PCIE0_RESET_ST_U */
/* PCIE0¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A00 */
typedef union tagScPcie0ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_RESET_ST_U;

/* Define the union U_SC_PCIE1_RESET_ST_U */
/* PCIE1¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A04 */
typedef union tagScPcie1ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_RESET_ST_U;

/* Define the union U_SC_PCIE2_RESET_ST_U */
/* PCIE2¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A08 */
typedef union tagScPcie2ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_RESET_ST_U;

/* Define the union U_SC_SAS_RESET_ST_U */
/* SAS¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A0C */
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

/* Define the union U_SC_MCTP0_RESET_ST_U */
/* MCTP0¸´Î»×´Ì¬¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0x5A10 */
typedef union tagScMctp0ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp0_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP0_RESET_ST_U;

/* Define the union U_SC_MCTP1_RESET_ST_U */
/* MCTP1¸´Î»×´Ì¬¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0x5A14 */
typedef union tagScMctp1ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp1_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP1_RESET_ST_U;

/* Define the union U_SC_MCTP2_RESET_ST_U */
/* MCTP2¸´Î»×´Ì¬¼Ä´æÆ÷£¨×¢Òâ£º±¾¼Ä´æÆ÷±£Áô²»ÓÃ£© */
/* 0x5A18 */
typedef union tagScMctp2ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp2_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP2_RESET_ST_U;

/* Define the union U_SC_SLLC_TSVRX_RESET_ST_U */
/* ÎªSLLC TSVRXÍ¨µÀÈí¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A2C */
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

/* Define the union U_SC_PCIE_HILINK_PCS_RESET_ST_U */
/* PCIE HILINK PCS¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A30 */
typedef union tagScPcieHilinkPcsResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_hilink_pcs_lane0_srst_st : 1   ; /* [0]  */
        unsigned int    pcie0_hilink_pcs_lane1_srst_st : 1   ; /* [1]  */
        unsigned int    pcie0_hilink_pcs_lane2_srst_st : 1   ; /* [2]  */
        unsigned int    pcie0_hilink_pcs_lane3_srst_st : 1   ; /* [3]  */
        unsigned int    pcie0_hilink_pcs_lane4_srst_st : 1   ; /* [4]  */
        unsigned int    pcie0_hilink_pcs_lane5_srst_st : 1   ; /* [5]  */
        unsigned int    pcie0_hilink_pcs_lane6_srst_st : 1   ; /* [6]  */
        unsigned int    pcie0_hilink_pcs_lane7_srst_st : 1   ; /* [7]  */
        unsigned int    pcie1_hilink_pcs_lane0_srst_st : 1   ; /* [8]  */
        unsigned int    pcie1_hilink_pcs_lane1_srst_st : 1   ; /* [9]  */
        unsigned int    pcie1_hilink_pcs_lane2_srst_st : 1   ; /* [10]  */
        unsigned int    pcie1_hilink_pcs_lane3_srst_st : 1   ; /* [11]  */
        unsigned int    pcie1_hilink_pcs_lane4_srst_st : 1   ; /* [12]  */
        unsigned int    pcie1_hilink_pcs_lane5_srst_st : 1   ; /* [13]  */
        unsigned int    pcie1_hilink_pcs_lane6_srst_st : 1   ; /* [14]  */
        unsigned int    pcie1_hilink_pcs_lane7_srst_st : 1   ; /* [15]  */
        unsigned int    pcie2_hilink_pcs_lane0_srst_st : 1   ; /* [16]  */
        unsigned int    pcie2_hilink_pcs_lane1_srst_st : 1   ; /* [17]  */
        unsigned int    pcie2_hilink_pcs_lane2_srst_st : 1   ; /* [18]  */
        unsigned int    pcie2_hilink_pcs_lane3_srst_st : 1   ; /* [19]  */
        unsigned int    pcie2_hilink_pcs_lane4_srst_st : 1   ; /* [20]  */
        unsigned int    pcie2_hilink_pcs_lane5_srst_st : 1   ; /* [21]  */
        unsigned int    pcie2_hilink_pcs_lane6_srst_st : 1   ; /* [22]  */
        unsigned int    pcie2_hilink_pcs_lane7_srst_st : 1   ; /* [23]  */
        unsigned int    pcie3_hilink_pcs_lane0_srst_st : 1   ; /* [24]  */
        unsigned int    pcie3_hilink_pcs_lane1_srst_st : 1   ; /* [25]  */
        unsigned int    pcie3_hilink_pcs_lane2_srst_st : 1   ; /* [26]  */
        unsigned int    pcie3_hilink_pcs_lane3_srst_st : 1   ; /* [27]  */
        unsigned int    pcie3_hilink_pcs_lane4_srst_st : 1   ; /* [28]  */
        unsigned int    pcie3_hilink_pcs_lane5_srst_st : 1   ; /* [29]  */
        unsigned int    pcie3_hilink_pcs_lane6_srst_st : 1   ; /* [30]  */
        unsigned int    pcie3_hilink_pcs_lane7_srst_st : 1   ; /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE_HILINK_PCS_RESET_ST_U;

/* Define the union U_SC_PCIE3_RESET_ST_U */
/* PCIE3¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A34 */
typedef union tagScPcie3ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_RESET_ST_U;

/* Define the union U_SC_MCTP3_RESET_ST_U */
/* MCTP3¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A38 */
typedef union tagScMctp3ResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    mctp3_srst_st         : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_MCTP3_RESET_ST_U;

/* Define the union U_SC_ITS_RESET_ST_U */
/* ITS¸´Î»×´Ì¬¼Ä´æÆ÷ */
/* 0x5A40 */
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

/* Define the union U_SC_SLLC_RESET_ST_U */
/* SLLC0¸´Î»×´Ì¬¼Ä´æÆ÷ */
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

/* Define the union U_SC_PCS_LOCAL_RESET_ST_U */
/* PCS LOCALÈí¸´Î»ÇëÇó×´Ì¬¼Ä´æÆ÷ */
/* 0x5A60 */
typedef union tagScPcsLocalResetSt
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_pcs_local_srst_st : 1   ; /* [0]  */
        unsigned int    pcie1_pcs_local_srst_st : 1   ; /* [1]  */
        unsigned int    pcie2_pcs_local_srst_st : 1   ; /* [2]  */
        unsigned int    pcie3_pcs_local_srst_st : 1   ; /* [3]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCS_LOCAL_RESET_ST_U;

/* Define the union U_SC_HILINK0_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6400 */
typedef union tagScHilink0MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_plloutoflock  : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK0_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6404 */
typedef union tagScHilink0MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_prbs_err      : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK0_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6408 */
typedef union tagScHilink0MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink0_los           : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK0_MACRO_LOS_U;

/* Define the union U_SC_HILINK1_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6500 */
typedef union tagScHilink1MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_plloutoflock  : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK1_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6504 */
typedef union tagScHilink1MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_prbs_err      : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK1_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6508 */
typedef union tagScHilink1MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink1_los           : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK1_MACRO_LOS_U;

/* Define the union U_SC_HILINK5_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6600 */
typedef union tagScHilink5MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_plloutoflock  : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK5_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6604 */
typedef union tagScHilink5MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_prbs_err      : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK5_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6608 */
typedef union tagScHilink5MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink5_los           : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK5_MACRO_LOS_U;

/* Define the union U_SC_HILINK6_MACRO_PLLOUTOFLOCK_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6700 */
typedef union tagScHilink6MacroPlloutoflock
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_plloutoflock  : 2   ; /* [1..0]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_PLLOUTOFLOCK_U;

/* Define the union U_SC_HILINK6_MACRO_PRBS_ERR_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6704 */
typedef union tagScHilink6MacroPrbsErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_prbs_err      : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_PRBS_ERR_U;

/* Define the union U_SC_HILINK6_MACRO_LOS_U */
/* HILINK×´Ì¬¼Ä´æÆ÷ */
/* 0x6708 */
typedef union tagScHilink6MacroLos
{
    /* Define the struct bits */
    struct
    {
        unsigned int    hilink6_los           : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_HILINK6_MACRO_LOS_U;

/* Define the union U_SC_PCIE0_RXEQINPRO_STAT_U */
/*  */
/* 0x6800 */
typedef union tagScPcie0RxeqinproStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_mac_phy_rxeqinprogress : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_RXEQINPRO_STAT_U;

/* Define the union U_SC_PCIE0_LINKINT_RCVRY_STAT_U */
/*  */
/* 0x6804 */
typedef union tagScPcie0LinkintRcvryStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_cfg_link_eq_req_int : 1   ; /* [0]  */
        unsigned int    pcie0_xmlh_ltssm_state_rcvry_eq : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_LINKINT_RCVRY_STAT_U;

/* Define the union U_SC_PCIE0_SYS_STATE0_U */
/* pcie0×´Ì¬¼Ä´æÆ÷0 */
/* 0x6808 */
typedef union tagScPcie0SysState0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_gm_cmposer_lookup_err : 1   ; /* [0]  */
        unsigned int    pcie0_radmx_cmposer_lookup_err : 1   ; /* [1]  */
        unsigned int    pcie0_pm_xtlh_block_tlp : 1   ; /* [2]  */
        unsigned int    pcie0_cfg_mem_space_en : 1   ; /* [3]  */
        unsigned int    pcie0_cfg_rcb         : 1   ; /* [4]  */
        unsigned int    pcie0_rdlh_link_up    : 1   ; /* [5]  */
        unsigned int    pcie0_pm_curnt_state  : 3   ; /* [8..6]  */
        unsigned int    pcie0_cfg_aer_rc_err_int : 1   ; /* [9]  */
        unsigned int    pcie0_cfg_aer_int_msg_num : 5   ; /* [14..10]  */
        unsigned int    pcie0_xmlh_link_up    : 1   ; /* [15]  */
        unsigned int    pcie0_wake            : 1   ; /* [16]  */
        unsigned int    pcie0_cfg_eml_control : 1   ; /* [17]  */
        unsigned int    pcie0_hp_pme          : 1   ; /* [18]  */
        unsigned int    pcie0_hp_int          : 1   ; /* [19]  */
        unsigned int    pcie0_hp_msi          : 1   ; /* [20]  */
        unsigned int    pcie0_pm_status       : 1   ; /* [21]  */
        unsigned int    pcie0_ref_clk_req_n   : 1   ; /* [22]  */
        unsigned int    pcie0_p2_exit_reg     : 1   ; /* [23]  */
        unsigned int    pcie0_radm_msg_req_id_low : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_SYS_STATE0_U;

/* Define the union U_SC_PCIE0_SYS_STATE1_U */
/* pcie0×´Ì¬¼Ä´æÆ÷1 */
/* 0x680C */
typedef union tagScPcie0SysState1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_axi_parity_errs_reg : 4   ; /* [3..0]  */
        unsigned int    pcie0_app_parity_errs_reg : 3   ; /* [6..4]  */
        unsigned int    pcie0_pm_linkst_in_l1 : 1   ; /* [7]  */
        unsigned int    pcie0_pm_linkst_in_l2 : 1   ; /* [8]  */
        unsigned int    pcie0_pm_linkst_l2_exit : 1   ; /* [9]  */
        unsigned int    pcie0_mac_phy_power_down : 2   ; /* [11..10]  */
        unsigned int    pcie0_radm_correctabl_err_reg : 1   ; /* [12]  */
        unsigned int    pcie0_radm_nonfatal_err_reg : 1   ; /* [13]  */
        unsigned int    pcie0_radm_fatal_err_reg : 1   ; /* [14]  */
        unsigned int    pcie0_radm_pm_to_pme_reg : 1   ; /* [15]  */
        unsigned int    pcie0_radm_pm_to_ack_reg : 1   ; /* [16]  */
        unsigned int    pcie0_radm_cpl_timeout_reg : 1   ; /* [17]  */
        unsigned int    pcie0_radm_msg_unlock_reg : 1   ; /* [18]  */
        unsigned int    pcie0_cfg_pme_msi_reg : 1   ; /* [19]  */
        unsigned int    pcie0_bridge_flush_not_reg : 1   ; /* [20]  */
        unsigned int    pcie0_link_req_rst_not_reg : 1   ; /* [21]  */
        unsigned int    pcie0_cfg_aer_rc_err_msi : 1   ; /* [22]  */
        unsigned int    pcie0_cfg_sys_err_rc  : 1   ; /* [23]  */
        unsigned int    pcie0_radm_msg_req_id_high : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_SYS_STATE1_U;

/* Define the union U_SC_PCIE0_SYS_STATE4_U */
/* pcie0×´Ì¬¼Ä´æÆ÷4 */
/* 0x6818 */
typedef union tagScPcie0SysState4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_ltssm_state     : 6   ; /* [5..0]  */
        unsigned int    pcie0_mac_phy_rate    : 2   ; /* [7..6]  */
        unsigned int    pcie0_slv_err_int     : 1   ; /* [8]  */
        unsigned int    pcie0_retry_sram_addr : 10  ; /* [18..9]  */
        unsigned int    pcie0_mstr_rresp_int  : 1   ; /* [19]  */
        unsigned int    pcie0_mstr_bresp_int  : 1   ; /* [20]  */
        unsigned int    pcie0_radm_inta_reg   : 1   ; /* [21]  */
        unsigned int    pcie0_radm_intb_reg   : 1   ; /* [22]  */
        unsigned int    pcie0_radm_intc_reg   : 1   ; /* [23]  */
        unsigned int    pcie0_radm_intd_reg   : 1   ; /* [24]  */
        unsigned int    pcie0_cfg_pme_int_reg : 1   ; /* [25]  */
        unsigned int    pcie0_radm_vendor_msg_reg : 1   ; /* [26]  */
        unsigned int    pcie0_bridge_flush_not : 1   ; /* [27]  */
        unsigned int    pcie0_link_req_rst_not : 1   ; /* [28]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_SYS_STATE4_U;

/* Define the union U_SC_PCIE0_AXI_MSTR_OOO_WR_STS1_U */
/* OOOÐ´Í¨µÀ¹¤×÷×´Ì¬ */
/* 0x6884 */
typedef union tagScPcie0AxiMstrOooWrSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_curr_wr_latency : 16  ; /* [15..0]  */
        unsigned int    pcie0_curr_wr_port_sts : 1   ; /* [16]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_AXI_MSTR_OOO_WR_STS1_U;

/* Define the union U_SC_PCIE0_AXI_MSTR_OOO_RD_STS1_U */
/* OOO¶ÁÍ¨µÀ¹¤×÷×´Ì¬ */
/* 0x6894 */
typedef union tagScPcie0AxiMstrOooRdSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_curr_rd_latency : 16  ; /* [15..0]  */
        unsigned int    pcie0_curr_rd_port_sts : 1   ; /* [16]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_AXI_MSTR_OOO_RD_STS1_U;

/* Define the union U_SC_PCIE0_DSIZE_BRG_ECC_ERR_U */
/* PCIE¶ÔÓ¦DSIZE BRGµÄECC´íÎó×´Ì¬ */
/* 0x68A0 */
typedef union tagScPcie0DsizeBrgEccErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_rob_ecc_err_detect : 1   ; /* [0]  */
        unsigned int    pcie0_rob_ecc_err_multpl : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_DSIZE_BRG_ECC_ERR_U;

/* Define the union U_SC_PCIE0_PCIEPHY_CTRL_ERROR_U */
/* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
/* 0x68C0 */
typedef union tagScPcie0PciephyCtrlError
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie0_pciephy_ctrl_error : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE0_PCIEPHY_CTRL_ERROR_U;

/* Define the union U_SC_PCIE1_RXEQINPRO_STAT_U */
/*  */
/* 0x6900 */
typedef union tagScPcie1RxeqinproStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_mac_phy_rxeqinprogress : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_RXEQINPRO_STAT_U;

/* Define the union U_SC_PCIE1_LINKINT_RCVRY_STAT_U */
/*  */
/* 0x6904 */
typedef union tagScPcie1LinkintRcvryStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_cfg_link_eq_req_int : 1   ; /* [0]  */
        unsigned int    pcie1_xmlh_ltssm_state_rcvry_eq : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_LINKINT_RCVRY_STAT_U;

/* Define the union U_SC_PCIE1_SYS_STATE0_U */
/* pcie1×´Ì¬¼Ä´æÆ÷0 */
/* 0x6908 */
typedef union tagScPcie1SysState0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_gm_cmposer_lookup_err : 1   ; /* [0]  */
        unsigned int    pcie1_radmx_cmposer_lookup_err : 1   ; /* [1]  */
        unsigned int    pcie1_pm_xtlh_block_tlp : 1   ; /* [2]  */
        unsigned int    pcie1_cfg_mem_space_en : 1   ; /* [3]  */
        unsigned int    pcie1_cfg_rcb         : 1   ; /* [4]  */
        unsigned int    pcie1_rdlh_link_up    : 1   ; /* [5]  */
        unsigned int    pcie1_pm_curnt_state  : 3   ; /* [8..6]  */
        unsigned int    pcie1_cfg_aer_rc_err_int : 1   ; /* [9]  */
        unsigned int    pcie1_cfg_aer_int_msg_num : 5   ; /* [14..10]  */
        unsigned int    pcie1_xmlh_link_up    : 1   ; /* [15]  */
        unsigned int    pcie1_wake            : 1   ; /* [16]  */
        unsigned int    pcie1_cfg_eml_control : 1   ; /* [17]  */
        unsigned int    pcie1_hp_pme          : 1   ; /* [18]  */
        unsigned int    pcie1_hp_int          : 1   ; /* [19]  */
        unsigned int    pcie1_hp_msi          : 1   ; /* [20]  */
        unsigned int    pcie1_pm_status       : 1   ; /* [21]  */
        unsigned int    pcie1_ref_clk_req_n   : 1   ; /* [22]  */
        unsigned int    pcie1_p2_exit_reg     : 1   ; /* [23]  */
        unsigned int    pcie1_radm_msg_req_id_low : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_SYS_STATE0_U;

/* Define the union U_SC_PCIE1_SYS_STATE1_U */
/* pcie1×´Ì¬¼Ä´æÆ÷1 */
/* 0x690C */
typedef union tagScPcie1SysState1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_axi_parity_errs_reg : 4   ; /* [3..0]  */
        unsigned int    pcie1_app_parity_errs_reg : 3   ; /* [6..4]  */
        unsigned int    pcie1_pm_linkst_in_l1 : 1   ; /* [7]  */
        unsigned int    pcie1_pm_linkst_in_l2 : 1   ; /* [8]  */
        unsigned int    pcie1_pm_linkst_l2_exit : 1   ; /* [9]  */
        unsigned int    pcie1_mac_phy_power_down : 2   ; /* [11..10]  */
        unsigned int    pcie1_radm_correctabl_err_reg : 1   ; /* [12]  */
        unsigned int    pcie1_radm_nonfatal_err_reg : 1   ; /* [13]  */
        unsigned int    pcie1_radm_fatal_err_reg : 1   ; /* [14]  */
        unsigned int    pcie1_radm_pm_to_pme_reg : 1   ; /* [15]  */
        unsigned int    pcie1_radm_pm_to_ack_reg : 1   ; /* [16]  */
        unsigned int    pcie1_radm_cpl_timeout_reg : 1   ; /* [17]  */
        unsigned int    pcie1_radm_msg_unlock_reg : 1   ; /* [18]  */
        unsigned int    pcie1_cfg_pme_msi_reg : 1   ; /* [19]  */
        unsigned int    pcie1_bridge_flush_not_reg : 1   ; /* [20]  */
        unsigned int    pcie1_link_req_rst_not_reg : 1   ; /* [21]  */
        unsigned int    pcie1_cfg_aer_rc_err_msi : 1   ; /* [22]  */
        unsigned int    pcie1_cfg_sys_err_rc  : 1   ; /* [23]  */
        unsigned int    pcie1_radm_msg_req_id_high : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_SYS_STATE1_U;

/* Define the union U_SC_PCIE1_SYS_STATE4_U */
/* pcie1×´Ì¬¼Ä´æÆ÷4 */
/* 0x6918 */
typedef union tagScPcie1SysState4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_ltssm_state     : 6   ; /* [5..0]  */
        unsigned int    pcie1_mac_phy_rate    : 2   ; /* [7..6]  */
        unsigned int    pcie1_slv_err_int     : 1   ; /* [8]  */
        unsigned int    pcie1_retry_sram_addr : 10  ; /* [18..9]  */
        unsigned int    pcie1_mstr_rresp_int  : 1   ; /* [19]  */
        unsigned int    pcie1_mstr_bresp_int  : 1   ; /* [20]  */
        unsigned int    pcie1_radm_inta_reg   : 1   ; /* [21]  */
        unsigned int    pcie1_radm_intb_reg   : 1   ; /* [22]  */
        unsigned int    pcie1_radm_intc_reg   : 1   ; /* [23]  */
        unsigned int    pcie1_radm_intd_reg   : 1   ; /* [24]  */
        unsigned int    pcie1_cfg_pme_int_reg : 1   ; /* [25]  */
        unsigned int    pcie1_radm_vendor_msg_reg : 1   ; /* [26]  */
        unsigned int    pcie1_bridge_flush_not : 1   ; /* [27]  */
        unsigned int    pcie1_link_req_rst_not : 1   ; /* [28]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_SYS_STATE4_U;

/* Define the union U_SC_PCIE1_AXI_MSTR_OOO_WR_STS1_U */
/* OOOÐ´Í¨µÀ¹¤×÷×´Ì¬ */
/* 0x6984 */
typedef union tagScPcie1AxiMstrOooWrSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_curr_wr_latency : 16  ; /* [15..0]  */
        unsigned int    pcie1_curr_wr_port_sts : 1   ; /* [16]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_AXI_MSTR_OOO_WR_STS1_U;

/* Define the union U_SC_PCIE1_AXI_MSTR_OOO_RD_STS1_U */
/* OOO¶ÁÍ¨µÀ¹¤×÷×´Ì¬ */
/* 0x6994 */
typedef union tagScPcie1AxiMstrOooRdSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_curr_rd_latency : 16  ; /* [15..0]  */
        unsigned int    pcie1_curr_rd_port_sts : 1   ; /* [16]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_AXI_MSTR_OOO_RD_STS1_U;

/* Define the union U_SC_PCIE1_DSIZE_BRG_ECC_ERR_U */
/* PCIE¶ÔÓ¦DSIZE BRGµÄECC´íÎó×´Ì¬ */
/* 0x69A0 */
typedef union tagScPcie1DsizeBrgEccErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_rob_ecc_err_detect : 1   ; /* [0]  */
        unsigned int    pcie1_rob_ecc_err_multpl : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_DSIZE_BRG_ECC_ERR_U;

/* Define the union U_SC_PCIE1_PCIEPHY_CTRL_ERROR_U */
/* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
/* 0x69C0 */
typedef union tagScPcie1PciephyCtrlError
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie1_pciephy_ctrl_error : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE1_PCIEPHY_CTRL_ERROR_U;

/* Define the union U_SC_PCIE2_RXEQINPRO_STAT_U */
/*  */
/* 0x6A00 */
typedef union tagScPcie2RxeqinproStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_mac_phy_rxeqinprogress : 8   ; /* [7..0]  */
        unsigned int    reserved_0            : 24  ; /* [31..8]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_RXEQINPRO_STAT_U;

/* Define the union U_SC_PCIE2_LINKINT_RCVRY_STAT_U */
/*  */
/* 0x6A04 */
typedef union tagScPcie2LinkintRcvryStat
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_cfg_link_eq_req_int : 1   ; /* [0]  */
        unsigned int    pcie2_xmlh_ltssm_state_rcvry_eq : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_LINKINT_RCVRY_STAT_U;

/* Define the union U_SC_PCIE2_SYS_STATE0_U */
/* pcie2×´Ì¬¼Ä´æÆ÷0 */
/* 0x6A08 */
typedef union tagScPcie2SysState0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_gm_cmposer_lookup_err : 1   ; /* [0]  */
        unsigned int    pcie2_radmx_cmposer_lookup_err : 1   ; /* [1]  */
        unsigned int    pcie2_pm_xtlh_block_tlp : 1   ; /* [2]  */
        unsigned int    pcie2_cfg_mem_space_en : 1   ; /* [3]  */
        unsigned int    pcie2_cfg_rcb         : 1   ; /* [4]  */
        unsigned int    pcie2_rdlh_link_up    : 1   ; /* [5]  */
        unsigned int    pcie2_pm_curnt_state  : 3   ; /* [8..6]  */
        unsigned int    pcie2_cfg_aer_rc_err_int : 1   ; /* [9]  */
        unsigned int    pcie2_cfg_aer_int_msg_num : 5   ; /* [14..10]  */
        unsigned int    pcie2_xmlh_link_up    : 1   ; /* [15]  */
        unsigned int    pcie2_wake            : 1   ; /* [16]  */
        unsigned int    pcie2_cfg_eml_control : 1   ; /* [17]  */
        unsigned int    pcie2_hp_pme          : 1   ; /* [18]  */
        unsigned int    pcie2_hp_int          : 1   ; /* [19]  */
        unsigned int    pcie2_hp_msi          : 1   ; /* [20]  */
        unsigned int    pcie2_pm_status       : 1   ; /* [21]  */
        unsigned int    pcie2_ref_clk_req_n   : 1   ; /* [22]  */
        unsigned int    pcie2_p2_exit_reg     : 1   ; /* [23]  */
        unsigned int    pcie2_radm_msg_req_id_low : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_SYS_STATE0_U;

/* Define the union U_SC_PCIE2_SYS_STATE1_U */
/* pcie2×´Ì¬¼Ä´æÆ÷1 */
/* 0x6A0C */
typedef union tagScPcie2SysState1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_axi_parity_errs_reg : 4   ; /* [3..0]  */
        unsigned int    pcie2_app_parity_errs_reg : 3   ; /* [6..4]  */
        unsigned int    pcie2_pm_linkst_in_l1 : 1   ; /* [7]  */
        unsigned int    pcie2_pm_linkst_in_l2 : 1   ; /* [8]  */
        unsigned int    pcie2_pm_linkst_l2_exit : 1   ; /* [9]  */
        unsigned int    pcie2_mac_phy_power_down : 2   ; /* [11..10]  */
        unsigned int    pcie2_radm_correctabl_err_reg : 1   ; /* [12]  */
        unsigned int    pcie2_radm_nonfatal_err_reg : 1   ; /* [13]  */
        unsigned int    pcie2_radm_fatal_err_reg : 1   ; /* [14]  */
        unsigned int    pcie2_radm_pm_to_pme_reg : 1   ; /* [15]  */
        unsigned int    pcie2_radm_pm_to_ack_reg : 1   ; /* [16]  */
        unsigned int    pcie2_radm_cpl_timeout_reg : 1   ; /* [17]  */
        unsigned int    pcie2_radm_msg_unlock_reg : 1   ; /* [18]  */
        unsigned int    pcie2_cfg_pme_msi_reg : 1   ; /* [19]  */
        unsigned int    pcie2_bridge_flush_not_reg : 1   ; /* [20]  */
        unsigned int    pcie2_link_req_rst_not_reg : 1   ; /* [21]  */
        unsigned int    pcie2_cfg_aer_rc_err_msi : 1   ; /* [22]  */
        unsigned int    pcie2_cfg_sys_err_rc  : 1   ; /* [23]  */
        unsigned int    pcie2_radm_msg_req_id_high : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_SYS_STATE1_U;

/* Define the union U_SC_PCIE2_SYS_STATE4_U */
/* pcie2×´Ì¬¼Ä´æÆ÷4 */
/* 0x6A18 */
typedef union tagScPcie2SysState4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_ltssm_state     : 6   ; /* [5..0]  */
        unsigned int    pcie2_mac_phy_rate    : 2   ; /* [7..6]  */
        unsigned int    pcie2_slv_err_int     : 1   ; /* [8]  */
        unsigned int    pcie2_retry_sram_addr : 10  ; /* [18..9]  */
        unsigned int    pcie2_mstr_rresp_int  : 1   ; /* [19]  */
        unsigned int    pcie2_mstr_bresp_int  : 1   ; /* [20]  */
        unsigned int    pcie2_radm_inta_reg   : 1   ; /* [21]  */
        unsigned int    pcie2_radm_intb_reg   : 1   ; /* [22]  */
        unsigned int    pcie2_radm_intc_reg   : 1   ; /* [23]  */
        unsigned int    pcie2_radm_intd_reg   : 1   ; /* [24]  */
        unsigned int    pcie2_cfg_pme_int_reg : 1   ; /* [25]  */
        unsigned int    pcie2_radm_vendor_msg_reg : 1   ; /* [26]  */
        unsigned int    pcie2_bridge_flush_not : 1   ; /* [27]  */
        unsigned int    pcie2_link_req_rst_not : 1   ; /* [28]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_SYS_STATE4_U;

/* Define the union U_SC_PCIE2_AXI_MSTR_OOO_WR_STS1_U */
/* OOOÐ´Í¨µÀ¹¤×÷×´Ì¬ */
/* 0x6A84 */
typedef union tagScPcie2AxiMstrOooWrSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_curr_wr_latency : 16  ; /* [15..0]  */
        unsigned int    pcie2_curr_wr_port_sts : 1   ; /* [16]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_AXI_MSTR_OOO_WR_STS1_U;

/* Define the union U_SC_PCIE2_AXI_MSTR_OOO_RD_STS1_U */
/* OOO¶ÁÍ¨µÀ¹¤×÷×´Ì¬ */
/* 0x6A94 */
typedef union tagScPcie2AxiMstrOooRdSts1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_curr_rd_latency : 16  ; /* [15..0]  */
        unsigned int    pcie2_curr_rd_port_sts : 1   ; /* [16]  */
        unsigned int    reserved_0            : 15  ; /* [31..17]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_AXI_MSTR_OOO_RD_STS1_U;

/* Define the union U_SC_PCIE2_DSIZE_BRG_ECC_ERR_U */
/* PCIE¶ÔÓ¦DSIZE BRGµÄECC´íÎó×´Ì¬ */
/* 0x6AA0 */
typedef union tagScPcie2DsizeBrgEccErr
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_rob_ecc_err_detect : 1   ; /* [0]  */
        unsigned int    pcie2_rob_ecc_err_multpl : 1   ; /* [1]  */
        unsigned int    reserved_0            : 30  ; /* [31..2]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_DSIZE_BRG_ECC_ERR_U;

/* Define the union U_SC_PCIE2_PCIEPHY_CTRL_ERROR_U */
/* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
/* 0x6AC0 */
typedef union tagScPcie2PciephyCtrlError
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie2_pciephy_ctrl_error : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE2_PCIEPHY_CTRL_ERROR_U;

/* Define the union U_SC_PCIE3_SYS_STATE0_U */
/* pcie3×´Ì¬¼Ä´æÆ÷0 */
/* 0x6B08 */
typedef union tagScPcie3SysState0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_gm_cmposer_lookup_err : 1   ; /* [0]  */
        unsigned int    pcie3_radmx_cmposer_lookup_err : 1   ; /* [1]  */
        unsigned int    pcie3_pm_xtlh_block_tlp : 1   ; /* [2]  */
        unsigned int    pcie3_cfg_mem_space_en : 1   ; /* [3]  */
        unsigned int    pcie3_cfg_rcb         : 1   ; /* [4]  */
        unsigned int    pcie3_rdlh_link_up    : 1   ; /* [5]  */
        unsigned int    pcie3_pm_curnt_state  : 3   ; /* [8..6]  */
        unsigned int    pcie3_cfg_aer_rc_err_int : 1   ; /* [9]  */
        unsigned int    pcie3_cfg_aer_int_msg_num : 5   ; /* [14..10]  */
        unsigned int    pcie3_xmlh_link_up    : 1   ; /* [15]  */
        unsigned int    pcie3_wake            : 1   ; /* [16]  */
        unsigned int    pcie3_cfg_eml_control : 1   ; /* [17]  */
        unsigned int    pcie3_hp_pme          : 1   ; /* [18]  */
        unsigned int    pcie3_hp_int          : 1   ; /* [19]  */
        unsigned int    pcie3_hp_msi          : 1   ; /* [20]  */
        unsigned int    pcie3_pm_status       : 1   ; /* [21]  */
        unsigned int    pcie3_ref_clk_req_n   : 1   ; /* [22]  */
        unsigned int    pcie3_p2_exit_reg     : 1   ; /* [23]  */
        unsigned int    pcie3_radm_msg_req_id_low : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_SYS_STATE0_U;

/* Define the union U_SC_PCIE3_SYS_STATE1_U */
/* pcie3×´Ì¬¼Ä´æÆ÷1 */
/* 0x6B0C */
typedef union tagScPcie3SysState1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_axi_parity_errs_reg : 4   ; /* [3..0]  */
        unsigned int    pcie3_app_parity_errs_reg : 3   ; /* [6..4]  */
        unsigned int    pcie3_pm_linkst_in_l1 : 1   ; /* [7]  */
        unsigned int    pcie3_pm_linkst_in_l2 : 1   ; /* [8]  */
        unsigned int    pcie3_pm_linkst_l2_exit : 1   ; /* [9]  */
        unsigned int    pcie3_mac_phy_power_down : 2   ; /* [11..10]  */
        unsigned int    pcie3_radm_correctabl_err_reg : 1   ; /* [12]  */
        unsigned int    pcie3_radm_nonfatal_err_reg : 1   ; /* [13]  */
        unsigned int    pcie3_radm_fatal_err_reg : 1   ; /* [14]  */
        unsigned int    pcie3_radm_pm_to_pme_reg : 1   ; /* [15]  */
        unsigned int    pcie3_radm_pm_to_ack_reg : 1   ; /* [16]  */
        unsigned int    pcie3_radm_cpl_timeout_reg : 1   ; /* [17]  */
        unsigned int    pcie3_radm_msg_unlock_reg : 1   ; /* [18]  */
        unsigned int    pcie3_cfg_pme_msi_reg : 1   ; /* [19]  */
        unsigned int    pcie3_bridge_flush_not_reg : 1   ; /* [20]  */
        unsigned int    pcie3_link_req_rst_not_reg : 1   ; /* [21]  */
        unsigned int    pcie3_cfg_aer_rc_err_msi : 1   ; /* [22]  */
        unsigned int    pcie3_cfg_sys_err_rc  : 1   ; /* [23]  */
        unsigned int    pcie3_radm_msg_req_id_high : 8   ; /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_SYS_STATE1_U;

/* Define the union U_SC_PCIE3_SYS_STATE4_U */
/* pcie3×´Ì¬¼Ä´æÆ÷4 */
/* 0x6B18 */
typedef union tagScPcie3SysState4
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_ltssm_state     : 6   ; /* [5..0]  */
        unsigned int    pcie3_mac_phy_rate    : 2   ; /* [7..6]  */
        unsigned int    pcie3_slv_err_int     : 1   ; /* [8]  */
        unsigned int    pcie3_retry_sram_addr : 10  ; /* [18..9]  */
        unsigned int    pcie3_mstr_rresp_int  : 1   ; /* [19]  */
        unsigned int    pcie3_mstr_bresp_int  : 1   ; /* [20]  */
        unsigned int    pcie3_radm_inta_reg   : 1   ; /* [21]  */
        unsigned int    pcie3_radm_intb_reg   : 1   ; /* [22]  */
        unsigned int    pcie3_radm_intc_reg   : 1   ; /* [23]  */
        unsigned int    pcie3_radm_intd_reg   : 1   ; /* [24]  */
        unsigned int    pcie3_cfg_pme_int_reg : 1   ; /* [25]  */
        unsigned int    pcie3_radm_vendor_msg_reg : 1   ; /* [26]  */
        unsigned int    pcie3_bridge_flush_not : 1   ; /* [27]  */
        unsigned int    pcie3_link_req_rst_not : 1   ; /* [28]  */
        unsigned int    reserved_0            : 3   ; /* [31..29]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_SYS_STATE4_U;

/* Define the union U_SC_PCIE3_PCIEPHY_CTRL_ERROR_U */
/* PCIE¶ÔÓ¦PCS¿ØÖÆÆ÷µÄ´íÎó×´Ì¬ */
/* 0x6BC0 */
typedef union tagScPcie3PciephyCtrlError
{
    /* Define the struct bits */
    struct
    {
        unsigned int    pcie3_pciephy_ctrl_error : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_PCIE3_PCIEPHY_CTRL_ERROR_U;

/* Define the union U_SC_SKEW_ST_A_0_U */
/*  */
/* 0x7500 */
typedef union tagScSkewStA0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_lock_a           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_A_0_U;

/* Define the union U_SC_SKEW_ST_A_1_U */
/*  */
/* 0x7504 */
typedef union tagScSkewStA1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_varible_out_a    : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_A_1_U;

/* Define the union U_SC_SKEW_ST_A_3_U */
/*  */
/* 0x750C */
typedef union tagScSkewStA3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_dcell_out_a_h    : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_A_3_U;

/* Define the union U_SC_SKEW_ST_B_0_U */
/*  */
/* 0x7600 */
typedef union tagScSkewStB0
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_lock_b           : 1   ; /* [0]  */
        unsigned int    reserved_0            : 31  ; /* [31..1]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_B_0_U;

/* Define the union U_SC_SKEW_ST_B_1_U */
/*  */
/* 0x7604 */
typedef union tagScSkewStB1
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_varible_out_b    : 16  ; /* [15..0]  */
        unsigned int    reserved_0            : 16  ; /* [31..16]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_B_1_U;

/* Define the union U_SC_SKEW_ST_B_3_U */
/*  */
/* 0x760C */
typedef union tagScSkewStB3
{
    /* Define the struct bits */
    struct
    {
        unsigned int    skew_dcell_out_b_h    : 4   ; /* [3..0]  */
        unsigned int    reserved_0            : 28  ; /* [31..4]  */
    } bits;

    /* Define an unsigned member */
    unsigned int    u32;

} SC_SKEW_ST_B_3_U;



#endif

#endif // __PCIE_SUB_REG_OFFSET_H__

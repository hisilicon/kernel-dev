/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfTU8sjoWxiS1/cgWX5CtFNa63k6Yc/3U44KeRvBDtKMGylcadZp63N7vuf8lP2xr4S9J
nCHJXblfnpl3Puz4wNQtlnvTNeUQknlm2fbSwoA4WFagChI2aokTHVZI71ZSeg==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  版权所有 (C), 2011-2021, 华为技术有限公司

 ******************************************************************************
  文 件 名   : sre_sub_peri.c
  版 本 号   : 初稿
  作    者   : z00176027
  生成日期   : 2013年10月17日
  最近修改   :
  功能描述   : P660 PERI 复位及去复位
  函数列表   :
  修改历史   :
  1.日    期   : 2013年10月17日
    作    者   : z00176027
    修改内容   : 创建文件

******************************************************************************/
#include "sre_sub_poe.h"

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
UINT64 sub_poe_base_addr = 0;
UINT64 sub_poe_pa_addr = 0;

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

 /* 定义Dsaf SUB的寄存器读写接口 */

#ifdef HRD_OS_LINUX
 inline UINT32 POE_SUB_READ_REG(UINT32 pRegBase, UINT32 ulRegIndex)
 {
     volatile UINT32 temp;

     temp = ioread32((void __iomem *)(sub_poe_base_addr + (pRegBase - PERI_SUB_BASE) + (ulRegIndex << 2)));

     return temp;
 }

#define POE_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
    (iowrite32(ulValue, (void __iomem *)(sub_poe_base_addr + (pRegBase - PERI_SUB_BASE) + (ulRegIndex << 2))))

#else
#define POE_SUB_READ_REG(pRegBase, ulRegIndex) \
                   OS_READ_REG(pRegBase, ulRegIndex)

#define POE_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
                   OS_WRITE_REG(pRegBase, ulRegIndex, ulValue)


#endif



/*****************************************************************************
 函 数 名  : HRD_Poe_SramSrst
 功能描述  : SRAM 复位及解复位
 输入参数  : UINT32 ulEn  0:复位 >=1:去复位
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月22日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Poe_SramSrst(UINT32 ulEn)
{
    SC_SRAM_RESET_REQ_U o_sc_sram_reset_req;
    SC_SRAM_RESET_DREQ_U o_sc_sram_reset_dreq;


    memset(&o_sc_sram_reset_req,0,sizeof(SC_SRAM_RESET_REQ_U));
    memset(&o_sc_sram_reset_dreq,0,sizeof(SC_SRAM_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_sram_reset_req.bits.sram_srst_req = 0x1;
        POE_SUB_WRITE_REG(POE_SUB_SC_SRAM_RESET_REQ_REG,0,o_sc_sram_reset_req.u32);
    }
    else
    {
         o_sc_sram_reset_dreq.bits.sram_srst_dreq = 0x1;
         POE_SUB_WRITE_REG(POE_SUB_SC_SRAM_RESET_DREQ_REG,0,o_sc_sram_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_Poe_PoeSrst
 功能描述  : POE 复位及解复位
 输入参数  : UINT32 ulEn  0:复位 >=1:去复位
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月22日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Poe_PoeSrst(UINT32 ulEn)
{
    SC_POE_RESET_REQ_U o_sc_poe_reset_req;
    SC_POE_RESET_DREQ_U o_sc_poe_reset_dreq;


    memset(&o_sc_poe_reset_req,0,sizeof(SC_POE_RESET_REQ_U));
    memset(&o_sc_poe_reset_dreq,0,sizeof(SC_POE_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_poe_reset_req.bits.poe_srst_req = 0x1;
        POE_SUB_WRITE_REG(POE_SUB_SC_POE_RESET_REQ_REG,0,o_sc_poe_reset_req.u32);
    }
    else
    {
         o_sc_poe_reset_dreq.bits.poe_srst_dreq = 0x1;
         POE_SUB_WRITE_REG(POE_SUB_SC_POE_RESET_DREQ_REG,0,o_sc_poe_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

#ifdef HRD_OS_LINUX

EXPORT_SYMBOL(HRD_Poe_SramSrst);
EXPORT_SYMBOL(HRD_Poe_PoeSrst);
#endif



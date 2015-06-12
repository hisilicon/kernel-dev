/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfYxkNfJL/4k4na/dcQjngzQtYCZVMyia1/q9S8py6PGPSHLoe78M+QZMpFXN8wClAkjU
1sxTExws2dI1kAsgfNdds/aQYHktDwwVQr2EFomFebketbcbzRgXYzopzk9E1A==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  版权所有 (C), 2011-2021, 华为技术有限公司

 ******************************************************************************
  文 件 名   : sre_sub_crg.c
  版 本 号   : 初稿
  作    者   : z00176027
  生成日期   : 2013年10月28日
  最近修改   :
  功能描述   : 本文将用于存放一些较复杂模块的综合，实际上就是将较复杂的复位
               综合成1个函数
  函数列表   :
  修改历史   :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 创建文件

******************************************************************************/

#include "hrdCommon.h"
#include "hrdOs.h"
#include "osal_api.h"
#include "hrd_crg_api.h"

#ifdef HRD_OS_LINUX
#include "iware_comm_kernel_api.h"
#endif



/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
#ifdef HRD_OS_LINUX
extern UINT32 HRD_SubPeriInit(void);
extern UINT32 HRD_SubPeriExit(void);
extern UINT32 HRD_SubAlgInit(void);
extern UINT32 HRD_SubAlgExit(void);
extern UINT32 HRD_SubDsafInit(void);
extern UINT32 HRD_SubDsafExit(void);
extern UINT32 HRD_SubM3Init(void);
extern UINT32 HRD_SubM3Exit(void);
extern UINT32 HRD_SubPcieInit(void);
extern UINT32 HRD_SubPcieExit(void);
#endif

#ifdef HRD_OS_SRE
extern void HRD_PoeSub_PllCtrl(u32 pll);
#endif


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

#if 0
/*****************************************************************************
 函 数 名  : HRD_Its_Srst
 功能描述  : ITS复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Its_Srst(UINT32 ulEn)
{
    HRD_Peri_ItsSrst(ulEn);
    HRD_Alg_ItsSrst(ulEn);
    HRD_M3_ItsSrst(ulEn);
    HRD_Dsaf_ItsSrst(ulEn);
    HRD_Pcie_ItsSrst(ulEn);

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_Peri_DmacSrst
 功能描述  : DMAC复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Peri_DmacSrst(UINT32 ulEn)
{
    UINT32 ulChannelId;
    UINT32 ulRet;

    for(ulChannelId = 0; ulChannelId < PV660_DMAC_NUM; ulChannelId++)
    {
        ulRet = HRD_Peri_DmacSrstByChannel(ulChannelId,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }

    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Peri_NandcSrst
 功能描述  : Nandc复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Peri_NandcSrst(UINT32 ulEn)
{
    UINT32 ulChannelId;
    UINT32 ulRet;

    for(ulChannelId = 0; ulChannelId < PV660_NANDC_NUM; ulChannelId++)
    {
        ulRet = HRD_Peri_NandcSrstByChannel(ulChannelId,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }

    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Peri_GpioSrst
 功能描述  : GPIO复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Peri_GpioSrst(UINT32 ulEn)
{
    UINT32 ulChannelId;
    UINT32 ulRet;

    for(ulChannelId = 0; ulChannelId < PV660_IO_NUM; ulChannelId++)
    {
        ulRet = HRD_Peri_GpioSrstByPort(ulChannelId,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }

    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_Peri_WdgSrst
 功能描述  : Wdg复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Peri_WdgSrst(UINT32 ulEn)
{
    UINT32 ulChannelId;
    UINT32 ulRet;

    for(ulChannelId = 0; ulChannelId < PV660_WDG_NUM; ulChannelId++)
    {
        ulRet = HRD_Peri_WdgSrstByNum(ulChannelId,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }

    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_Alg_HilinkSrst
 功能描述  : Hilink复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Alg_HilinkSrst(UINT32 ulEn)
{
    UINT32 ulHllc;
    UINT32 ulLane;
    UINT32 ulRet;

    for(ulHllc = 0; ulHllc < PV660_HLLC_NUM; ulHllc++)
    {
        for(ulLane = 0; ulLane < PV660_PCS_LANE_NUM; ulLane++)
        {
            ulRet = HRD_Alg_HilinkSrstByLane(ulHllc,ulLane,ulEn);
            if(ulRet != OS_SUCCESS)
            {
                return ulRet;
            }
        }


    }
    return OS_SUCCESS;
}




/*****************************************************************************
 函 数 名  : HRD_Alg_HllcSrst
 功能描述  : Hllc复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Alg_HllcSrst(UINT32 ulEn)
{
    UINT32 ulHllc;
    UINT32 ulRet;

    for(ulHllc = 0; ulHllc < PV660_HLLC_NUM; ulHllc++)
    {

        ulRet = HRD_Alg_HllcSrstByHllc(ulHllc,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_M3_UartSrst
 功能描述  : M3 uart复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_M3_UartSrst(UINT32 ulEn)
{
    UINT32 ulPort;
    UINT32 ulRet;

    for(ulPort = 0; ulPort < PV660_M3_UART_NUM; ulPort++)
    {

        ulRet = HRD_M3_UartSrstByPort(ulPort,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_M3_SfcSrst
 功能描述  : M3 sfc复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_M3_SfcSrst(UINT32 ulEn)
{
    UINT32 ulPort;
    UINT32 ulRet;

    for(ulPort = 0; ulPort < PV660_M3_SFC_NUM; ulPort++)
    {

        ulRet = HRD_M3_SfcSrstByPort(ulPort,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_M3_I2cSrst
 功能描述  : M3 i2c复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_M3_I2cSrst(UINT32 ulEn)
{
    UINT32 ulPort;
    UINT32 ulRet;

    for(ulPort = 0; ulPort < PV660_M3_I2C_NUM; ulPort++)
    {

        ulRet = HRD_M3_I2cSrstByNum(ulPort,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_M3_UsbSrst
 功能描述  : M3 usb复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_M3_UsbSrst(UINT32 ulEn)
{
    UINT32 ulPort;
    UINT32 ulRet;

    for(ulPort = 0; ulPort < PV660_M3_USB_NUM; ulPort++)
    {

        ulRet = HRD_M3_UsbSrstByPort(ulPort,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Dsaf_PpeSrst
 功能描述  : PPE复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Dsaf_PpeSrst(UINT32 ulEn)
{
    UINT32 ulPort;
    UINT32 ulRet;

    for(ulPort = 0; ulPort < PV660_XGE_NUM; ulPort++)
    {

        ulRet = HRD_Dsaf_PpeSrstByPort(ulPort,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_Dsaf_XgeSrst
 功能描述  : XGE复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Dsaf_XgeSrst(UINT32 ulEn)
{
    UINT32 ulPort = 0;
    UINT32 ulRet;

    for(ulPort=0;ulPort<PV660_XGE_NUM;ulPort++)
    {
        ulRet = HRD_Dsaf_XgeSrstByPort(ulPort,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Dsaf_GeSrst
 功能描述  : GE复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Dsaf_GeSrst(UINT32 ulEn)
{
    UINT32 ulPort = 0;
    UINT32 ulRet;
    for(ulPort=0;ulPort<PV660_GE_NUM;ulPort++)
    {
        ulRet = HRD_Dsaf_GeSrstByPort(ulPort,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Dsaf_SllcSrst
 功能描述  : Dsaf sllc复位及解复位
 输入参数  : UINT32 ulEn
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月28日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Dsaf_SllcSrst(UINT32 ulEn)
{
    UINT32 ulChannel;
    UINT32 ulRet;

    for(ulChannel = 0; ulChannel < PV660_DSAF_SLLC_NUM; ulChannel++)
    {

        ulRet = HRD_Dsaf_SllcSrstByChannel(ulChannel,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }
    }

    return OS_SUCCESS;
}



/*****************************************************************************
 函 数 名  : HRD_CRG_Link
 功能描述  : 仅用于复位链接
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月22日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_CRG_Link(void)
{
    return OS_SUCCESS;
}

#ifdef HRD_OS_SRE
void HRD_PoePllCtrl(u32 pll)
{
    HRD_PoeSub_PllCtrl(pll);
    return;
}
#endif
#endif

#ifdef HRD_OS_LINUX
/*****************************************************************************
 函 数 名  : HRD_Crg_LinuxInit
 功能描述  : CRG 寄存器初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月21日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Crg_LinuxInit(void)
{
    UINT32 ulRet;

#if 0
    ulRet = HRD_SubPeriInit();
    if (ulRet != OS_SUCCESS)
    {
        return OS_ERROR;
    }

    ulRet = HRD_SubAlgInit();
    if (ulRet != OS_SUCCESS)
    {
        return OS_ERROR;
    }
#endif

    ulRet = HRD_SubDsafInit();
    if (ulRet != OS_SUCCESS)
    {
        return OS_ERROR;
    }

#if 0
    ulRet = HRD_SubM3Init();
    if (ulRet != OS_SUCCESS)
    {
        return OS_ERROR;
    }
#endif


    ulRet = HRD_SubPcieInit();
    if (ulRet != OS_SUCCESS)
    {
        return OS_ERROR;
    }

    return ulRet;
}

/*****************************************************************************
 函 数 名  : HRD_Crg_LinuxExit
 功能描述  : CRG 寄存器退出
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年11月21日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Crg_LinuxExit(void)
{
    UINT32 ulRet;

#if 0
    ulRet = HRD_SubPeriExit();
    if (ulRet != OS_SUCCESS)
    {
        return OS_ERROR;
    }

    ulRet = HRD_SubAlgExit();
    if (ulRet != OS_SUCCESS)
    {
        return OS_ERROR;
    }
#endif
    ulRet = HRD_SubDsafExit();
    if (ulRet != OS_SUCCESS)
    {
        return OS_ERROR;
    }

#if 0
    ulRet = HRD_SubM3Exit();
    if (ulRet != OS_SUCCESS)
    {
        return OS_ERROR;
    }
#endif

    ulRet = HRD_SubPcieExit();
    if (ulRet != OS_SUCCESS)
    {
        return OS_ERROR;
    }

    return ulRet;
}


#endif



/******************************************************************************

                  版权所有 (C), 2011-2021, 华为技术有限公司

 ******************************************************************************
  文 件 名   : sre_crg_api.h
  版 本 号   : 初稿
  作    者   : z00176027
  生成日期   : 2013年10月17日
  最近修改   :
  功能描述   : 复位及解复位对外接口
  函数列表   :
  修改历史   :
  1.日    期   : 2013年10月17日
    作    者   : z00176027
    修改内容   : 创建文件

******************************************************************************/

#ifndef __C_SRE_CRG_API_H__
#define __C_SRE_CRG_API_H__


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
#define PV660_DMAC_NUM              2
#define PV660_NANDC_NUM             1
#define PV660_IO_NUM                5
#define PV660_TIMER_NUM             24
#define PV660_WDG_NUM               16

#define PV660_HLLC_NUM              1
#define PV660_PCS_LANE_NUM          8
#define PV660_HLLC_CHANNEL_NUM      16

#define PV660_M3_UART_NUM           2
#define PV660_M3_SFC_NUM            2
#define PV660_M3_I2C_NUM            8
#define PV660_M3_IO_NUM             5
#define PV660_M3_USB_NUM            5

#define PV660_XGE_NUM               (6+2)
#define PV660_GE_NUM                (6+2)
#define PV660_DSAF_SLLC_NUM         4

#define PV660_PCIE_NUM              4
#define PV660_PCIE_SLLC_NUM         4


/* ITS */
UINT32 HRD_Its_Srst(UINT32 ulEn); /* ITS 总接口 */

/*************** Dsaf子系统复位接口 **************/
UINT32 HRD_Dsaf_SllcSrstByChannel(UINT32 ulChannel,UINT32 ulEn);
UINT32 HRD_Dsaf_SllcSrst(UINT32 ulEn);
UINT32 HRD_Dsaf_ItsSrst(UINT32 ulEn);
UINT32 HRD_Dsaf_SasSrst(UINT32 ulEn);
UINT32 HRD_Dsaf_GeSrst(UINT32 ulEn);
UINT32 HRD_Dsaf_GeSrstByPort(UINT32 ulPort,UINT32 ulEn);
UINT32 HRD_Dsaf_XgeSrstByPort(UINT32 ulPort,UINT32 ulEn);
UINT32 HRD_Dsaf_XgeSrst(UINT32 ulEn);
UINT32 HRD_Dsaf_PpeSrstByPort(UINT32 ulPort,UINT32 ulEn);
UINT32 HRD_Dsaf_PpeSrst(UINT32 ulEn);
UINT32 HRD_Dsaf_XbarSrst(UINT32 ulDasfIndex,UINT32 ulEn);
UINT32 HRD_Dsaf_NtSrst(UINT32 ulDasfIndex,UINT32 ulEn);
UINT32 HRD_Dsaf_RoceSrst(UINT32 ulEn);

/*************** Pcie子系统复位接口 **************/
UINT32 HRD_Pcie_PcieSrst(UINT32 ulChannel, UINT32 ulEn);
UINT32 HRD_Pcie_SasSrst(UINT32 ulEn);
UINT32 HRD_Pcie_MctpSrst(UINT32 ulChannel, UINT32 ulEn);
UINT32 HRD_Pcie_SllcTsvRxSrstByChannel(UINT32 ulChannel,UINT32 ulEn);
UINT32 HRD_Pcie_HilinkSrstByLane(UINT32 ulPcie, UINT32 ulLane, UINT32 ulEn);
UINT32 HRD_Pcie_Sllc0Srst(UINT32 ulEn);
UINT32 HRD_Pcie_PcsLocalSrstByChannel(UINT32 ulEn,UINT32 ulChannel);
UINT32 HRD_Pcie_ItsSrst(UINT32 ulEn);



/*************** Alg子系统复位接口 **************/
UINT32 HRD_Alg_SecSrst(UINT32 ulEn);
UINT32 HRD_Alg_RdeSrst(UINT32 ulEn);
UINT32 HRD_Alg_PaSrst(UINT32 ulEn);
UINT32 HRD_Alg_DjtagSrst(UINT32 ulEn);
UINT32 HRD_Alg_RsaSrst(UINT32 ulEn);
UINT32 HRD_Alg_RngSrst(UINT32 ulEn);
UINT32 HRD_Alg_SecBulidSrst(UINT32 ulEn);
UINT32 HRD_Alg_PcsLocalSrst(UINT32 ulEn);
UINT32 HRD_Alg_HllcSrstByHllc(UINT32 ulHllc,  UINT32 ulEn);
UINT32 HRD_Alg_HllcSrst(UINT32 ulEn);
UINT32 HRD_Alg_HilinkSrstByLane(UINT32 ulHllc, UINT32 ulLane, UINT32 ulEn);
UINT32 HRD_Alg_HilinkSrst(UINT32 ulEn);
UINT32 HRD_Alg_ItsSrst(UINT32 ulEn);


/*************** Peri子系统复位接口 **************/
UINT32 HRD_Peri_DmacSrstByChannel(UINT32 ulChannelId, UINT32 ulEn);
UINT32 HRD_Peri_DmacSrst(UINT32 ulEn);
UINT32 HRD_Peri_LocalbusSrst(UINT32 ulEn);
UINT32 HRD_Peri_GpioSrstByPort(UINT32 ulIoPort, UINT32 ulEn);
UINT32 HRD_Peri_GpioSrst( UINT32 ulEn);
UINT32 HRD_Peri_MdioSrst(UINT32 ulEn);
UINT32 HRD_Peri_DbgSrst(UINT32 ulEn);
UINT32 HRD_Peri_WdgSrstByNum(UINT32 ulWdg, UINT32 ulEn);
UINT32 HRD_Peri_WdgSrst(UINT32 ulEn);
UINT32 HRD_Peri_NandcSrst(UINT32 ulEn);
UINT32 HRD_Peri_NandcSrstByChannel(UINT32 ulChannelId, UINT32 ulEn);
UINT32 HRD_Peri_GicSrst(UINT32 ulEn);
UINT32 HRD_Peri_ItsSrst(UINT32 ulEn);
UINT32 HRD_Peri_SramSrst(UINT32 ulEn);
UINT32 HRD_Peri_PoeSrst(UINT32 ulEn);


/*************** Poe子系统复位接口 **************/
UINT32 HRD_Poe_SramSrst(UINT32 ulEn);
UINT32 HRD_Poe_PoeSrst(UINT32 ulEn);


/* 低功耗M3 */
UINT32 HRD_M3_WdgSrst(UINT32 ulEn);
UINT32 HRD_M3_TimerSrst(UINT32 ulEn);
UINT32 HRD_M3_UartSrstByPort(UINT32 ulPort,UINT32 ulEn);
UINT32 HRD_M3_UartSrst(UINT32 ulEn);
UINT32 HRD_M3_IpcmSrst(UINT32 ulEn);
UINT32 HRD_M3_SramSrst(UINT32 ulEn);
UINT32 HRD_M3_SfcSrstByPort(UINT32 ulPort,UINT32 ulEn);
UINT32 HRD_M3_SfcSrst(UINT32 ulEn);
UINT32 HRD_M3_I2cmSrst(UINT32 ulEn);
UINT32 HRD_M3_I2cSrstByNum(UINT32 ulNum, UINT32 ulEn);
UINT32 HRD_M3_I2cSrst(UINT32 ulEn);
UINT32 HRD_M3_GpioSrstByPort(UINT32 ulPort,UINT32 ulEn);
UINT32 HRD_M3_GpioSrst(UINT32 ulEn);
UINT32 HRD_M3_CpuSrst(UINT32 ulEn);
UINT32 HRD_M3_UsbSrstByPort(UINT32 ulPort,UINT32 ulEn);
UINT32 HRD_M3_UsbSrst(UINT32 ulEn);
UINT32 HRD_M3_SpiSrst(UINT32 ulEn);
UINT32 HRD_M3_SecSrst(UINT32 ulEn);
UINT32 HRD_M3_SecBuildSrst(UINT32 ulEn);
UINT32 HRD_M3_ItsSrst(UINT32 ulEn);

#endif


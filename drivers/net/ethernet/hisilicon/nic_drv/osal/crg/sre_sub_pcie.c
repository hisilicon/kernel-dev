/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfW4xh9+AoobWjbe8OM0uIPaHJV/Yzj+mufBLqE6uQ5LI8YuNNIzu6ZB8jOk0H+yM0bhr
Lq0HC/lvpES6LFe6piN8ImxNVdjtbvUrVY0IFauqET5It4xE7+KbAAQfaCfxeA==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  版权所有 (C), 2011-2021, 华为技术有限公司

 ******************************************************************************
  文 件 名   : sre_sub_pcie.c
  版 本 号   : 初稿
  作    者   : z00176027
  生成日期   : 2013年10月17日
  最近修改   :
  功能描述   : P660 sas子系统 复位及去复位
  函数列表   :
  修改历史   :
  1.日    期   : 2013年10月17日
    作    者   : z00176027
    修改内容   : 创建文件

******************************************************************************/

#include "sre_sub_pcie.h"


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
UINT64 sub_pcie_base_addr = 0;
UINT64 sub_pcie_pa_addr = 0;

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
 inline UINT32 PCIE_SUB_READ_REG(UINT32 pRegBase, UINT32 ulRegIndex)
 {
     volatile UINT32 temp;

     temp = ioread32((void __iomem *)(sub_pcie_base_addr + (pRegBase - PCIE_SUB_BASE) + (ulRegIndex << 2)));

     return temp;
 }

#define PCIE_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
    (iowrite32(ulValue, (void __iomem *)(sub_pcie_base_addr + (pRegBase - PCIE_SUB_BASE) + (ulRegIndex << 2))))

#else
#define PCIE_SUB_READ_REG(pRegBase, ulRegIndex) \
               OS_READ_REG(pRegBase, ulRegIndex)

#define PCIE_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
               OS_WRITE_REG(pRegBase, ulRegIndex, ulValue)

#endif


/*****************************************************************************
函 数 名  : HRD_Pcie_PcieSrst
功能描述  : Pcie复位及解复位
输入参数  : UINT32 ulChannel
            UINT32 ulEn  0:复位 >=1:去复位
输出参数  : 无
返 回 值  :
调用函数  :
被调函数  :

修改历史      :
 1.日    期   : 2013年10月26日
   作    者   : z00176027
   修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Pcie_PcieSrst(UINT32 ulChannel, UINT32 ulEn)
{
    SC_PCIE0_RESET_REQ_U o_sc_pcie_reset_req;
    SC_PCIE0_RESET_DREQ_U o_sc_pcie_reset_dreq;

    if(ulChannel >= PV660_PCIE_NUM)
    {
        return OS_FAIL;
    }

    memset(&o_sc_pcie_reset_req,0,sizeof(SC_PCIE0_RESET_REQ_U));
    memset(&o_sc_pcie_reset_dreq,0,sizeof(SC_PCIE0_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_pcie_reset_req.bits.pcie0_srst_req = 1;
        if((PV660_PCIE_NUM - 1) == ulChannel)
        {
            PCIE_SUB_WRITE_REG((PCIE_SUB_SC_PCIE3_RESET_REQ_REG),0,o_sc_pcie_reset_req.u32);
        }
        else
        {
            PCIE_SUB_WRITE_REG((PCIE_SUB_SC_PCIE0_RESET_REQ_REG + (ulChannel << 3)),0,o_sc_pcie_reset_req.u32);
        }

    }
    else
    {
        o_sc_pcie_reset_dreq.bits.pcie0_srst_dreq = 1;
        if((PV660_PCIE_NUM - 1) == ulChannel)
        {
            PCIE_SUB_WRITE_REG((PCIE_SUB_SC_PCIE3_RESET_DREQ_REG),0,o_sc_pcie_reset_req.u32);
        }
        else
        {
            PCIE_SUB_WRITE_REG((PCIE_SUB_SC_PCIE0_RESET_DREQ_REG + (ulChannel << 3)),0,o_sc_pcie_reset_dreq.u32);
        }

    }

    return OS_SUCCESS;
}

/*****************************************************************************
函 数 名  : HRD_Pcie_SasSrst
功能描述  : Pcie Sas复位及解复位
输入参数  : UINT32 ulEn  0:复位 >=1:去复位
输出参数  : 无
返 回 值  :
调用函数  :
被调函数  :

修改历史      :
 1.日    期   : 2013年10月26日
   作    者   : z00176027
   修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Pcie_SasSrst(UINT32 ulEn)
{
    SC_SAS_RESET_REQ_U o_sc_sas_reset_req;
    SC_SAS_RESET_DREQ_U o_sc_sas_reset_dreq;

    memset(&o_sc_sas_reset_req,0,sizeof(SC_SAS_RESET_REQ_U));
    memset(&o_sc_sas_reset_dreq,0,sizeof(SC_SAS_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_sas_reset_req.u32 |= 0x7ffff;
        PCIE_SUB_WRITE_REG(PCIE_SUB_SC_SAS_RESET_REQ_REG,0,o_sc_sas_reset_req.u32);

    }
    else
    {
        o_sc_sas_reset_dreq.u32 |= 0x7ffff;
        PCIE_SUB_WRITE_REG(PCIE_SUB_SC_SAS_RESET_DREQ_REG,0,o_sc_sas_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
函 数 名  : HRD_Pcie_MctpSrst
功能描述  : Mctp复位及解复位
输入参数  : UINT32 ulChannel
            UINT32 ulEn  0:复位 >=1:去复位
输出参数  : 无
返 回 值  :
调用函数  :
被调函数  :

修改历史      :
 1.日    期   : 2013年10月26日
   作    者   : z00176027
   修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Pcie_MctpSrst(UINT32 ulChannel, UINT32 ulEn)
{
    SC_MCTP0_RESET_REQ_U o_sc_mctp_reset_req;
    SC_MCTP0_RESET_DREQ_U o_sc_mctp_reset_dreq;

    if(ulChannel >= PV660_PCIE_NUM)
    {
        return OS_FAIL;
    }

    memset(&o_sc_mctp_reset_req,0,sizeof(SC_MCTP0_RESET_REQ_U));
    memset(&o_sc_mctp_reset_dreq,0,sizeof(SC_MCTP0_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_mctp_reset_req.bits.mctp0_srst_req = 1;
        if((PV660_PCIE_NUM - 1) == ulChannel)
        {
            PCIE_SUB_WRITE_REG((PCIE_SUB_SC_MCTP3_RESET_REQ_REG),0,o_sc_mctp_reset_req.u32);
        }
        else
        {
            PCIE_SUB_WRITE_REG((PCIE_SUB_SC_MCTP0_RESET_REQ_REG + (ulChannel << 3)),0,o_sc_mctp_reset_req.u32);
        }

    }
    else
    {
        o_sc_mctp_reset_dreq.bits.mctp0_srst_dreq = 1;
        if((PV660_PCIE_NUM - 1) == ulChannel)
        {
            PCIE_SUB_WRITE_REG((PCIE_SUB_SC_MCTP3_RESET_DREQ_REG),0,o_sc_mctp_reset_req.u32);
        }
        else
        {
            PCIE_SUB_WRITE_REG((PCIE_SUB_SC_MCTP0_RESET_DREQ_REG + (ulChannel << 3)),0,o_sc_mctp_reset_dreq.u32);
        }

    }

    return OS_SUCCESS;
}


/*****************************************************************************
函 数 名  : HRD_Pcie_SllcSrstByChannel
功能描述  : Pcie sllc复位及解复位
输入参数  : UINT32 ulChannel  ulChannel
            UINT32 ulEn  0:复位 >=1:去复位
输出参数  : 无
返 回 值  :
调用函数  :
被调函数  :

修改历史      :
 1.日    期   : 2013年10月26日
   作    者   : z00176027
   修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Pcie_SllcTsvRxSrstByChannel(UINT32 ulChannel,UINT32 ulEn)
{
    SC_SLLC_TSVRX_RESET_REQ_U o_sc_sllc_tsvrx_reset_req;
    SC_SLLC_TSVRX_RESET_DREQ_U o_sc_sllc_tsvrx_reset_dreq;

    if(ulChannel >= PV660_PCIE_SLLC_NUM)
    {
        return OS_FAIL;
    }

    memset(&o_sc_sllc_tsvrx_reset_req,0,sizeof(SC_SLLC_TSVRX_RESET_REQ_U));
    memset(&o_sc_sllc_tsvrx_reset_dreq,0,sizeof(SC_SLLC_TSVRX_RESET_DREQ_U));


    if(0 == ulEn)
    {
        o_sc_sllc_tsvrx_reset_req.u32 |= (1 << ulChannel);
        PCIE_SUB_WRITE_REG(PCIE_SUB_SC_SLLC_TSVRX_RESET_REQ_REG,0,o_sc_sllc_tsvrx_reset_req.u32);
    }
    else
    {
        o_sc_sllc_tsvrx_reset_dreq.u32 |= (1 << ulChannel);
        PCIE_SUB_WRITE_REG(PCIE_SUB_SC_SLLC_TSVRX_RESET_DREQ_REG,0,o_sc_sllc_tsvrx_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
函 数 名  : HRD_Pcie_HilinkSrstByLane
功能描述  : PCIE Hilink pcs复位及解复位
输入参数  : UINT32 ulTimer  Timer
            UINT32 ulEn  0:复位 >=1:去复位
输出参数  : 无
返 回 值  :
调用函数  :
被调函数  :

修改历史      :
 1.日    期   : 2013年10月22日
   作    者   : z00176027
   修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Pcie_HilinkSrstByLane(UINT32 ulPcie, UINT32 ulLane, UINT32 ulEn)
{
    SC_PCIE_HILINK_PCS_RESET_REQ_U o_sc_hilink_pcs_reset_req;
    SC_PCIE_HILINK_PCS_RESET_DREQ_U o_sc_hilink_pcs_reset_dreq;

    memset(&o_sc_hilink_pcs_reset_req,0,sizeof(SC_PCIE_HILINK_PCS_RESET_REQ_U));
    memset(&o_sc_hilink_pcs_reset_dreq,0,sizeof(SC_PCIE_HILINK_PCS_RESET_DREQ_U));

    if(ulPcie >= (PV660_PCIE_NUM - 1))
    {
        return OS_FAIL;
    }

    if(ulLane >= PV660_PCS_LANE_NUM)
    {
        return OS_FAIL;
    }


    if(0 == ulEn)
    {
        o_sc_hilink_pcs_reset_req.u32 |= ((ulPcie << PV660_PCS_LANE_NUM) | (1 << ulLane));
        PCIE_SUB_WRITE_REG(PCIE_SUB_SC_PCIE_HILINK_PCS_RESET_REQ_REG,0,o_sc_hilink_pcs_reset_req.u32);
    }
    else
    {
        o_sc_hilink_pcs_reset_dreq.u32 |= ((ulPcie << PV660_PCS_LANE_NUM) | (1 << ulLane));
        PCIE_SUB_WRITE_REG(PCIE_SUB_SC_PCIE_HILINK_PCS_RESET_DREQ_REG,0,o_sc_hilink_pcs_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
函 数 名  : HRD_Pcie_Sllc0Srst
功能描述  : Sllc0复位及解复位
输入参数  : UINT32 ulEn  0:复位 >=1:去复位
输出参数  : 无
返 回 值  :
调用函数  :
被调函数  :

修改历史      :
 1.日    期   : 2013年10月25日
   作    者   : z00176027
   修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Pcie_Sllc0Srst(UINT32 ulEn)
{
    SC_SLLC_RESET_REQ_U o_sc_sllc_reset_req;
    SC_SLLC_RESET_DREQ_U o_sc_sllc_reset_dreq;

    memset(&o_sc_sllc_reset_req,0,sizeof(SC_SLLC_RESET_REQ_U));
    memset(&o_sc_sllc_reset_dreq,0,sizeof(SC_SLLC_RESET_DREQ_U));

    if(0 == ulEn)
    {
         o_sc_sllc_reset_req.bits.sllc_srst_req = 1;
         PCIE_SUB_WRITE_REG(PCIE_SUB_SC_SLLC_RESET_REQ_REG,0,o_sc_sllc_reset_req.u32);
    }
    else
    {
        o_sc_sllc_reset_dreq.bits.sllc_srst_dreq = 1;
        PCIE_SUB_WRITE_REG(PCIE_SUB_SC_SLLC_RESET_DREQ_REG,0,o_sc_sllc_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
函 数 名  : HRD_Pcie_PcsLocalSrst
功能描述  : Pca Local复位及解复位
输入参数  : UINT32 ulEn  0:复位 >=1:去复位
            UINT32 ulChannel:代表解复位哪个pcs.
输出参数  : 无
返 回 值  :
调用函数  :
被调函数  :

修改历史      :
 1.日    期   : 2013年10月25日
   作    者   : z00176027
   修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Pcie_PcsLocalSrstByChannel(UINT32 ulEn,UINT32 ulChannel)
{
    SC_PCS_LOCAL_RESET_REQ_U o_sc_pcs_local_reset_req;
    SC_PCS_LOCAL_RESET_DREQ_U o_sc_pcs_local_reset_dreq;

    memset(&o_sc_pcs_local_reset_req,0,sizeof(SC_PCS_LOCAL_RESET_REQ_U));
    memset(&o_sc_pcs_local_reset_dreq,0,sizeof(SC_PCS_LOCAL_RESET_DREQ_U));

    if(0 == ulEn)
    {
         o_sc_pcs_local_reset_req.u32 |=  0x1<<ulChannel;
         PCIE_SUB_WRITE_REG(PCIE_SUB_SC_PCS_LOCAL_RESET_REQ_REG,0,o_sc_pcs_local_reset_req.u32);
    }
    else
    {
        o_sc_pcs_local_reset_dreq.u32 |=  0x1<<ulChannel;
        PCIE_SUB_WRITE_REG(PCIE_SUB_SC_PCS_LOCAL_RESET_DREQ_REG,0,o_sc_pcs_local_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
函 数 名  : HRD_Pcie_ItsSrst
功能描述  : Its复位及解复位
输入参数  : UINT32 ulEn  0:复位 >=1:去复位
输出参数  : 无
返 回 值  :
调用函数  :
被调函数  :

修改历史      :
 1.日    期   : 2013年10月25日
   作    者   : z00176027
   修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Pcie_ItsSrst(UINT32 ulEn)
{
    SC_ITS_RESET_REQ_U o_sc_its_reset_req;
    SC_ITS_RESET_DREQ_U o_sc_its_reset_dreq;

    memset(&o_sc_its_reset_req,0,sizeof(SC_ITS_RESET_REQ_U));
    memset(&o_sc_its_reset_dreq,0,sizeof(SC_ITS_RESET_DREQ_U));

    if(0 == ulEn)
    {
         o_sc_its_reset_req.bits.its_srst_req = 1;
         PCIE_SUB_WRITE_REG(PCIE_SUB_SC_ITS_RESET_REQ_REG,0,o_sc_its_reset_req.u32);
    }
    else
    {
        o_sc_its_reset_dreq.bits.its_srst_dreq = 1;
        PCIE_SUB_WRITE_REG(PCIE_SUB_SC_ITS_RESET_DREQ_REG,0,o_sc_its_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

#ifdef HRD_OS_LINUX
/*****************************************************************************
 函 数 名  : HRD_SubPcieInit
 功能描述  : SUB PCIE CRG linux初始化
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
UINT32 HRD_SubPcieInit(void)
{
    sub_pcie_pa_addr = PCIE_SUB_BASE;

    /* SUB ALG申请io内存 */
    if (!request_mem_region(sub_pcie_pa_addr, PCIE_SUB_BASE_SIZE, "SUB PCIE Reg"))
    {
        printk("SUB PCIE region busy\n");
        return OS_ERROR;
    }

    sub_pcie_base_addr = (UINT64)ioremap(sub_pcie_pa_addr, PCIE_SUB_BASE_SIZE);
    if (!sub_pcie_base_addr)
    {
        release_mem_region(sub_pcie_pa_addr, PCIE_SUB_BASE_SIZE);
        printk("could not ioremap SUB PCIE registers\n");

        return OS_ERROR;
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_SubPcieExit
 功能描述  : SUB PCIE CRG 退出
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
void HRD_SubPcieExit(void)
{
    /* 释放POU资源 */
    if (sub_pcie_base_addr > 0)
    {
        release_mem_region(sub_pcie_pa_addr, PCIE_SUB_BASE_SIZE);
        iounmap((void *)sub_pcie_base_addr);
    }
}


UINT64 HRD_SubPcieGetBase(void)
{
	return sub_pcie_base_addr;
}
EXPORT_SYMBOL(HRD_Pcie_HilinkSrstByLane);
EXPORT_SYMBOL(HRD_Pcie_SllcTsvRxSrstByChannel);
EXPORT_SYMBOL(HRD_Pcie_ItsSrst);
EXPORT_SYMBOL(HRD_Pcie_PcieSrst);
EXPORT_SYMBOL(HRD_Pcie_MctpSrst);
EXPORT_SYMBOL(HRD_Pcie_Sllc0Srst);
EXPORT_SYMBOL(HRD_Pcie_PcsLocalSrstByChannel);
EXPORT_SYMBOL(HRD_SubPcieGetBase);


#endif




/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfWNsP9llaiogFaoAxHNmKiCAEC5FzmRgoPFMghjUbQW7GzrAkJmTWAqMrbEvpKBaUjC4
LiMB5kwyzW/jJ+FlpccKnqWF8E9LZg9GuAdEu+lKfTWzrVSZScRBv9x9szgyJQ==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  版权所有 (C), 2011-2021, 华为技术有限公司

 ******************************************************************************
  文 件 名   : sre_alg.c
  版 本 号   : 初稿
  作    者   : z00176027
  生成日期   : 2013年10月17日
  最近修改   :
  功能描述   : P660 ALG 复位及去复位
  函数列表   :
  修改历史   :
  1.日    期   : 2013年10月17日
    作    者   : z00176027
    修改内容   : 创建文件

******************************************************************************/

#include "sre_sub_alg.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
UINT32 HRD_Alg_SecBulidSrst(UINT32 ulEn);

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
UINT64 sub_alg_base_addr = 0;
UINT64 sub_alg_pa_addr = 0;
/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/* 定义ALG SUB的寄存器读写接口 */
#ifdef HRD_OS_LINUX
 inline UINT32 ALG_SUB_READ_REG(UINT32 pRegBase, UINT32 ulRegIndex)
 {
     volatile UINT32 temp;
 
     temp = ioread32((void __iomem *)(sub_alg_base_addr + (pRegBase - ALG_SUB_BASE) + (ulRegIndex << 2)));
 
     return temp;
 }
 
#define ALG_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
    (iowrite32(ulValue, (void __iomem *)(sub_alg_base_addr + (pRegBase - ALG_SUB_BASE) + (ulRegIndex << 2))))
 
#else
#define ALG_SUB_READ_REG(pRegBase, ulRegIndex) \
    OS_READ_REG(pRegBase, ulRegIndex)

 #define ALG_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
    OS_WRITE_REG(pRegBase, ulRegIndex, ulValue)

#endif


 



/*****************************************************************************
 函 数 名  : HRD_Alg_ItsSrst
 功能描述  : Its复位及解复位
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
UINT32 HRD_Alg_ItsSrst(UINT32 ulEn)
{
    SC_ITS_RESET_REQ_U o_sc_its_reset_req;
    SC_ITS_RESET_DREQ_U o_sc_its_reset_dreq;

    memset(&o_sc_its_reset_req,0,sizeof(SC_ITS_RESET_REQ_U));
    memset(&o_sc_its_reset_dreq,0,sizeof(SC_ITS_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_its_reset_req.bits.its_srst_req = 1;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_ITS_RESET_REQ_REG,0,o_sc_its_reset_req.u32);
    }    
    else
    {        
         o_sc_its_reset_dreq.bits.its_srst_dreq = 1;
         ALG_SUB_WRITE_REG(ALG_SUB_SC_ITS_RESET_DREQ_REG,0,o_sc_its_reset_dreq.u32);
    }           

    return OS_SUCCESS;
}

/*****************************************************************************
函 数 名  : HRD_Alg_HilinkSrstByLane
功能描述  : Hilink pcs复位及解复位
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
UINT32 HRD_Alg_HilinkSrstByLane(UINT32 ulHllc, UINT32 ulLane, UINT32 ulEn)
{
    SC_HILINK_PCS_RESET_REQ_U o_sc_hilink_pcs_reset_req;
    SC_HILINK_PCS_RESET_DREQ_U o_sc_hilink_pcs_reset_dreq;
   

    memset(&o_sc_hilink_pcs_reset_req,0,sizeof(SC_HILINK_PCS_RESET_REQ_U));
    memset(&o_sc_hilink_pcs_reset_dreq,0,sizeof(SC_HILINK_PCS_RESET_DREQ_U));

    if(ulHllc >= PV660_HLLC_NUM)
    {
        return OS_FAIL;
    }

    if(ulLane >= PV660_PCS_LANE_NUM)
    {
        return OS_FAIL;
    }


    if(0 == ulEn)
    {
        o_sc_hilink_pcs_reset_req.u32 |= ((ulHllc << PV660_PCS_LANE_NUM) | (1 << ulLane));
        ALG_SUB_WRITE_REG(ALG_SUB_SC_HILINK_PCS_RESET_REQ_REG,0,o_sc_hilink_pcs_reset_req.u32);
    }
    else
    {   
        o_sc_hilink_pcs_reset_dreq.u32 |= ((ulHllc << PV660_PCS_LANE_NUM) | (1 << ulLane));
        ALG_SUB_WRITE_REG(ALG_SUB_SC_HILINK_PCS_RESET_REQ_REG,0,o_sc_hilink_pcs_reset_dreq.u32);
    }           

    return OS_SUCCESS;
}


/*****************************************************************************
函 数 名  : HRD_Alg_HllcRxTxSrstByChannel
功能描述  : Hllc 发送接收复位及解复位
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
UINT32 HRD_Alg_HllcRxTxSrstByChannel(UINT32 ulHllc, UINT32 ulChannel, UINT32 ulEn)
{
    SC_HLLC_RXTX_RESET_REQ_U o_sc_hllc_rxtx_reset_req;
    SC_HLLC_RXTX_RESET_DREQ_U o_sc_hllc_rxtx_reset_dreq;
   
    if(ulChannel >= PV660_HLLC_CHANNEL_NUM)
    {
        return OS_FAIL;
    }
   
    memset(&o_sc_hllc_rxtx_reset_req,0,sizeof(SC_HLLC_RXTX_RESET_REQ_U));
    memset(&o_sc_hllc_rxtx_reset_dreq,0,sizeof(SC_HLLC_RXTX_RESET_DREQ_U));
    
    if(0 == ulEn)
    {
        o_sc_hllc_rxtx_reset_req.u32 |= (1 << ulChannel); /* rx */
        ALG_SUB_WRITE_REG(ALG_SUB_SC_HLLC_RXTX_RESET_REQ_REG,0,o_sc_hllc_rxtx_reset_req.u32);
    }    
    else
    {   
        o_sc_hllc_rxtx_reset_dreq.u32 |= (1 << ulChannel); /* rx */
        ALG_SUB_WRITE_REG(ALG_SUB_SC_HLLC_RXTX_RESET_DREQ_REG,0,o_sc_hllc_rxtx_reset_dreq.u32);
    }
    
    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Alg_PaSrst
 功能描述  : PA复位及解复位
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
UINT32 HRD_Alg_PaSrst(UINT32 ulEn)
{
    SC_PA_RESET_REQ_U o_sc_pa_reset_req;
    SC_PA_RESET_DREQ_U o_sc_pa_reset_dreq;

    memset(&o_sc_pa_reset_req,0,sizeof(SC_PA_RESET_REQ_U));
    memset(&o_sc_pa_reset_dreq,0,sizeof(SC_PA_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_pa_reset_req.bits.pa_srst_req = 1;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_PA_RESET_REQ_REG,0,o_sc_pa_reset_req.u32);
    }    
    else
    {        
         o_sc_pa_reset_dreq.bits.pa_srst_dreq = 1;
         ALG_SUB_WRITE_REG(ALG_SUB_SC_PA_RESET_DREQ_REG,0,o_sc_pa_reset_dreq.u32);
    }           

    return OS_SUCCESS;
}

/*****************************************************************************
函 数 名  : HRD_Alg_HllcSrst
功能描述  : hllc复位及解复位
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
UINT32 HRD_Alg_HllcSrstByHllc(UINT32 ulHllc,  UINT32 ulEn)
{
    SC_HLLC_RESET_REQ_U o_sc_hllc_reset_req;
    SC_HLLC_RESET_DREQ_U o_sc_hllc_reset_dreq;
    UINT32 ulRet = OS_SUCCESS;
    UINT32 ulChannel;
   

    memset(&o_sc_hllc_reset_req,0,sizeof(SC_HLLC_RESET_REQ_U));
    memset(&o_sc_hllc_reset_dreq,0,sizeof(SC_HLLC_RESET_DREQ_U));

    if(ulHllc > PV660_HLLC_NUM)
    {
        return OS_FAIL;
    }

    if(0 == ulEn)
    {
        o_sc_hllc_reset_req.u32 |= (1 << ulHllc);
        ALG_SUB_WRITE_REG(ALG_SUB_SC_HLLC_RESET_REQ_REG,0,o_sc_hllc_reset_req.u32);
    }    
    else
    {   
        o_sc_hllc_reset_dreq.u32 |= (1 << ulHllc);
        ALG_SUB_WRITE_REG(ALG_SUB_SC_HLLC_RESET_REQ_REG,0,o_sc_hllc_reset_dreq.u32);
    }

    for(ulChannel = 0; ulChannel < PV660_HLLC_CHANNEL_NUM; ulChannel++)
    {
        ulRet = HRD_Alg_HllcRxTxSrstByChannel(ulHllc,ulChannel,ulEn);
        if(ulRet != OS_SUCCESS)
        {
            return ulRet;
        }    
        
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Alg_DjtagSrst
 功能描述  : Djtag复位及解复位
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
UINT32 HRD_Alg_DjtagSrst(UINT32 ulEn)
{
    SC_DJTAG_RESET_REQ_U o_sc_djtag_reset_req;
    SC_DJTAG_RESET_DREQ_U o_sc_djtag_reset_dreq;

    memset(&o_sc_djtag_reset_req,0,sizeof(SC_DJTAG_RESET_REQ_U));
    memset(&o_sc_djtag_reset_dreq,0,sizeof(SC_DJTAG_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_djtag_reset_req.bits.djtag_srst_req = 1;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_DJTAG_RESET_REQ_REG,0,o_sc_djtag_reset_req.u32);
    }    
    else
    {        
         o_sc_djtag_reset_dreq.bits.djtag_srst_dreq = 1;
         ALG_SUB_WRITE_REG(ALG_SUB_SC_DJTAG_RESET_DREQ_REG,0,o_sc_djtag_reset_dreq.u32);
    }           

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_Alg_RsaSrst
 功能描述  : RSA复位及解复位
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
UINT32 HRD_Alg_RsaSrst(UINT32 ulEn)
{
    SC_RSA_RESET_REQ_U o_sc_rsa_reset_req;
    SC_RSA_RESET_DREQ_U o_sc_rsa_reset_dreq;

    memset(&o_sc_rsa_reset_req,0,sizeof(SC_RSA_RESET_REQ_U));
    memset(&o_sc_rsa_reset_dreq,0,sizeof(SC_RSA_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_rsa_reset_req.bits.rsa_srst_req = 1;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_RSA_RESET_REQ_REG,0,o_sc_rsa_reset_req.u32);
    }    
    else
    {        
         o_sc_rsa_reset_dreq.bits.rsa_srst_dreq = 1;
         ALG_SUB_WRITE_REG(ALG_SUB_SC_RSA_RESET_DREQ_REG,0,o_sc_rsa_reset_dreq.u32);
    }           

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Alg_RdeSrst
 功能描述  : Rde复位及解复位
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
UINT32 HRD_Alg_RdeSrst(UINT32 ulEn)
{
    SC_RDE_RESET_REQ_U o_sc_rde_reset_req;
    SC_RDE_RESET_DREQ_U o_sc_rde_reset_dreq;

    memset(&o_sc_rde_reset_req,0,sizeof(SC_RDE_RESET_REQ_U));
    memset(&o_sc_rde_reset_dreq,0,sizeof(SC_RDE_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_rde_reset_req.bits.rde_srst_req = 1;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_RDE_RESET_REQ_REG,0,o_sc_rde_reset_req.u32);
    }    
    else
    {        
         o_sc_rde_reset_dreq.bits.rde_srst_dreq = 1;
         ALG_SUB_WRITE_REG(ALG_SUB_SC_RDE_RESET_DREQ_REG,0,o_sc_rde_reset_dreq.u32);
    }           

    return OS_SUCCESS;
}



/*****************************************************************************
 函 数 名  : HRD_Alg_SecSrst
 功能描述  : Sec复位及解复位
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
UINT32 HRD_Alg_SecSrst(UINT32 ulEn)
{
    SC_SEC_RESET_REQ_U o_sc_sec_reset_req;
    SC_SEC_RESET_DREQ_U o_sc_sec_reset_dreq;

    memset(&o_sc_sec_reset_req,0,sizeof(SC_SEC_RESET_REQ_U));
    memset(&o_sc_sec_reset_dreq,0,sizeof(SC_SEC_RESET_DREQ_U));

    if(0 == ulEn)
    {
        HRD_Alg_SecBulidSrst(ulEn);        
        o_sc_sec_reset_req.bits.sec_srst_req = 1;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_SEC_RESET_REQ_REG,0,o_sc_sec_reset_req.u32);
    }    
    else
    {        
         o_sc_sec_reset_dreq.bits.sec_srst_dreq = 1;
         ALG_SUB_WRITE_REG(ALG_SUB_SC_SEC_RESET_DREQ_REG,0,o_sc_sec_reset_dreq.u32);         
         HRD_Alg_SecBulidSrst(ulEn);
    }              

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Alg_RngSrst
 功能描述  : Rng复位及解复位
 输入参数  : UINT32 ulEn  0:复位 >=1:去复位
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2013年10月24日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Alg_RngSrst(UINT32 ulEn)
{
    SC_RNG_RESET_REQ_U o_sc_rng_reset_req;
    SC_RNG_RESET_DREQ_U o_sc_rng_reset_dreq;

    memset(&o_sc_rng_reset_req,0,sizeof(SC_RNG_RESET_REQ_U));
    memset(&o_sc_rng_reset_dreq,0,sizeof(SC_RNG_RESET_DREQ_U));

    if(0 == ulEn)
    {
        
        o_sc_rng_reset_req.bits.rng_srst_req = 1;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_RNG_RESET_REQ_REG,0,o_sc_rng_reset_req.u32);
    }    
    else
    {        
         o_sc_rng_reset_dreq.bits.rng_srst_dreq = 1;
         ALG_SUB_WRITE_REG(ALG_SUB_SC_RNG_RESET_DREQ_REG,0,o_sc_rng_reset_dreq.u32);

         
    }           

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_Alg_SecBulidSrst
 功能描述  : Sec Build复位及解复位
 输入参数  : UINT32 ulEn  0:复位 >=1:去复位
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2013年10月24日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Alg_SecBulidSrst(UINT32 ulEn)
{
    SC_SEC_BUILD_RESET_REQ_U o_sc_sec_build_reset_req;
    SC_SEC_BUILD_RESET_DREQ_U o_sc_sec_build_reset_dreq;   

    memset(&o_sc_sec_build_reset_req,0,sizeof(SC_SEC_BUILD_RESET_REQ_U));
    memset(&o_sc_sec_build_reset_dreq,0,sizeof(SC_SEC_BUILD_RESET_DREQ_U));   

    if(0 == ulEn)
    {
        o_sc_sec_build_reset_req.u32 |= 0x1fffff;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_SEC_BUILD_RESET_REQ_REG,0,o_sc_sec_build_reset_req.u32);
    }    
    else
    {   
        o_sc_sec_build_reset_dreq.u32 |= 0x1fffff;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_SEC_BUILD_RESET_DREQ_REG,0,o_sc_sec_build_reset_dreq.u32);
    }           

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Alg_PcsLocalSrst
 功能描述  : Pcs Local复位及解复位
 输入参数  : UINT32 ulEn  0:复位 >=1:去复位
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2013年10月24日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Alg_PcsLocalSrst(UINT32 ulEn)
{
    SC_PCS_LOCAL_RESET_REQ_U o_sc_pcs_reset_req;
    SC_PCS_LOCAL_RESET_DREQ_U o_sc_pcs_reset_dreq;   

    memset(&o_sc_pcs_reset_req,0,sizeof(SC_PCS_LOCAL_RESET_REQ_U));
    memset(&o_sc_pcs_reset_dreq,0,sizeof(SC_PCS_LOCAL_RESET_DREQ_U));   

    if(0 == ulEn)
    {
        o_sc_pcs_reset_req.bits.pcs_local_srst_req = 1;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_PCS_LOCAL_RESET_REQ_REG,0,o_sc_pcs_reset_req.u32);
    }    
    else
    {   
        o_sc_pcs_reset_dreq.bits.pcs_local_srst_dreq |= 1;
        ALG_SUB_WRITE_REG(ALG_SUB_SC_PCS_LOCAL_RESET_DREQ_REG,0,o_sc_pcs_reset_dreq.u32);
    }           

    return OS_SUCCESS;
}


#ifdef HRD_OS_LINUX
/*****************************************************************************
 函 数 名  : HRD_SubAlgInit
 功能描述  : SUB ALG CRG linux初始化
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
UINT32 HRD_SubAlgInit(void)
{
    sub_alg_pa_addr = ALG_SUB_BASE;
    
    /* SUB ALG申请io内存 */
    if (!request_mem_region(sub_alg_pa_addr, ALG_SUB_BASE_SIZE, "SUB ALG Reg"))
    {
        
        printk("SUB ALG region busy\n");       

        return OS_ERROR;
    }

    sub_alg_base_addr = (UINT64)ioremap(sub_alg_pa_addr, ALG_SUB_BASE_SIZE);
    if (!sub_alg_base_addr)
    {
        release_mem_region(sub_alg_pa_addr, ALG_SUB_BASE_SIZE);        
        printk("could not ioremap SUB ALG registers\n");        

        return OS_ERROR;
    }
    
    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_SubAlgInit
 功能描述  : SUB ALG CRG 退出
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
void HRD_SubAlgExit(void)
{
    /* 释放ALG资源 */
    if (sub_alg_base_addr > 0)
    {
        release_mem_region(sub_alg_pa_addr, ALG_SUB_BASE_SIZE);
        iounmap((void *)sub_alg_base_addr);
    }

}


EXPORT_SYMBOL(HRD_Alg_HilinkSrstByLane);
EXPORT_SYMBOL(HRD_Alg_HllcSrstByHllc);
EXPORT_SYMBOL(HRD_Alg_ItsSrst);
EXPORT_SYMBOL(HRD_Alg_SecSrst);
EXPORT_SYMBOL(HRD_Alg_RdeSrst);
EXPORT_SYMBOL(HRD_Alg_PaSrst);
EXPORT_SYMBOL(HRD_Alg_DjtagSrst);
EXPORT_SYMBOL(HRD_Alg_RsaSrst);
EXPORT_SYMBOL(HRD_Alg_RngSrst);
EXPORT_SYMBOL(HRD_Alg_SecBulidSrst);
EXPORT_SYMBOL(HRD_Alg_PcsLocalSrst);


#endif



/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfWbHLEGW1BqTAOu6nfK9MrPwQtHAgOBDZwjzcnox9pwyTrueftCMCPRnNIWsz29m4HAF
pKsGF9knWU5kSCelvdI0Zwh9Jmy7/DdEEaqa753kHWuRjrXAQV04eIrH6+iu5A==*/
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


#include "sre_sub_peri.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
extern UINT32 SRE_WriteReg(void *addr,UINT32 value);
extern UINT32 SRE_ReadReg(UINT32 * addr);

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
UINT64 sub_peri_base_addr = 0;
UINT64 sub_peri_pa_addr = 0;

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
 inline UINT32 PERI_SUB_READ_REG(UINT32 pRegBase, UINT32 ulRegIndex)
 {
     volatile UINT32 temp;

     temp = ioread32((void __iomem *)(sub_peri_base_addr + (pRegBase - PERI_SUB_BASE) + (ulRegIndex << 2)));

     return temp;
 }

#define PERI_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
    (iowrite32(ulValue, (void __iomem *)(sub_peri_base_addr + (pRegBase - PERI_SUB_BASE) + (ulRegIndex << 2))))

#else
#define PERI_SUB_READ_REG(pRegBase, ulRegIndex) \
                   OS_READ_REG(pRegBase, ulRegIndex)

#define PERI_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
                   OS_WRITE_REG(pRegBase, ulRegIndex, ulValue)


#endif




/*****************************************************************************
 函 数 名  : HRD_Peri_DmacSrstByChannel
 功能描述  : DMAC复位及解复位
 输入参数  : UINT32 ulChannelId  DMAC通道
             UINT32 ulEn         0:复位 >=1:去复位
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年10月22日
    作    者   : z00176027
    修改内容   : 新生成函数

*****************************************************************************/
UINT32 HRD_Peri_DmacSrstByChannel(UINT32 ulChannelId, UINT32 ulEn)
{
    SC_DMAC_RESET_REQ_U o_sc_dmac_reset_req;
    SC_DMAC_RESET_DREQ_U o_sc_dmac_reset_dreq;

    if(ulChannelId >= PV660_DMAC_NUM)
    {
        return OS_FAIL;
    }

    memset(&o_sc_dmac_reset_req,0,sizeof(SC_DMAC_RESET_REQ_U));
    memset(&o_sc_dmac_reset_dreq,0,sizeof(SC_DMAC_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_dmac_reset_req.u32 |= (1 << ulChannelId);
        PERI_SUB_WRITE_REG(PERI_SUB_SC_DMAC_RESET_REQ_REG,0,o_sc_dmac_reset_req.u32);
    }
    else
    {
         o_sc_dmac_reset_dreq.u32 |= (1 << ulChannelId);
         PERI_SUB_WRITE_REG(PERI_SUB_SC_DMAC_RESET_DREQ_REG,0,o_sc_dmac_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_Peri_NandcSrstByChannel
 功能描述  : Nandc复位及解复位
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
UINT32 HRD_Peri_NandcSrstByChannel(UINT32 ulChannelId, UINT32 ulEn)
{
    SC_NANDC_RESET_REQ_U o_sc_nandc_reset_req;
    SC_NANDC_RESET_DREQ_U o_sc_nandc_reset_dreq;

    if(ulChannelId >= PV660_NANDC_NUM)
    {
        return OS_FAIL;
    }


    memset(&o_sc_nandc_reset_req,0,sizeof(SC_NANDC_RESET_REQ_U));
    memset(&o_sc_nandc_reset_dreq,0,sizeof(SC_NANDC_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_nandc_reset_req.u32 |= (1 << ulChannelId);
        PERI_SUB_WRITE_REG(PERI_SUB_SC_NANDC_RESET_REQ_REG,0,o_sc_nandc_reset_req.u32);
    }
    else
    {
         o_sc_nandc_reset_dreq.u32 |= (1 << ulChannelId);
         PERI_SUB_WRITE_REG(PERI_SUB_SC_NANDC_RESET_DREQ_REG,0,o_sc_nandc_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Peri_LocalbusSrst
 功能描述  : Localbus复位及解复位
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
UINT32 HRD_Peri_LocalbusSrst(UINT32 ulEn)
{
    SC_LOCALBUS_RESET_REQ_U o_sc_localbus_reset_req;
    SC_LOCALBUS_RESET_DREQ_U o_sc_localbus_reset_dreq;

    memset(&o_sc_localbus_reset_req,0,sizeof(SC_LOCALBUS_RESET_REQ_U));
    memset(&o_sc_localbus_reset_dreq,0,sizeof(SC_LOCALBUS_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_localbus_reset_req.bits.localbus_srst_req = 1;
        PERI_SUB_WRITE_REG(PERI_SUB_SC_LOCALBUS_RESET_REQ_REG,0,o_sc_localbus_reset_req.u32);
    }
    else
    {
         o_sc_localbus_reset_dreq.bits.localbus_srst_dreq = 1;
         PERI_SUB_WRITE_REG(PERI_SUB_SC_LOCALBUS_RESET_DREQ_REG,0,o_sc_localbus_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Peri_GpioSrstByPort
 功能描述  : GPIO复位及解复位
 输入参数  : UINT32 ulIoPort  IO端口号
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
UINT32 HRD_Peri_GpioSrstByPort(UINT32 ulIoPort, UINT32 ulEn)
{
    SC_GPIO_RESET_REQ_U o_sc_gpio_reset_req;
    SC_GPIO_RESET_DREQ_U o_sc_gpio_reset_dreq;

    memset(&o_sc_gpio_reset_req,0,sizeof(SC_GPIO_RESET_REQ_U));
    memset(&o_sc_gpio_reset_dreq,0,sizeof(SC_GPIO_RESET_DREQ_U));

    if(ulIoPort >= PV660_IO_NUM)
    {
        return OS_FAIL;
    }


    if(0 == ulEn)
    {
        o_sc_gpio_reset_req.u32 |= (1 << ulIoPort);
        PERI_SUB_WRITE_REG(PERI_SUB_SC_GPIO_RESET_REQ_REG,0,o_sc_gpio_reset_req.u32);
    }
    else
    {
         o_sc_gpio_reset_dreq.u32 |= (1 << ulIoPort);
         PERI_SUB_WRITE_REG(PERI_SUB_SC_GPIO_RESET_DREQ_REG,0,o_sc_gpio_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Peri_MdioSrst
 功能描述  : Mdio复位及解复位
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
UINT32 HRD_Peri_MdioSrst(UINT32 ulEn)
{
    SC_MDIO_RESET_REQ_U o_sc_mdio_reset_req;
    SC_MDIO_RESET_DREQ_U o_sc_mdio_reset_dreq;

    memset(&o_sc_mdio_reset_req,0,sizeof(SC_MDIO_RESET_REQ_U));
    memset(&o_sc_mdio_reset_dreq,0,sizeof(SC_MDIO_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_mdio_reset_req.bits.mdio_srst_req = 1;
        PERI_SUB_WRITE_REG(PERI_SUB_SC_MDIO_RESET_REQ_REG,0,o_sc_mdio_reset_req.u32);
    }
    else
    {
        o_sc_mdio_reset_dreq.bits.mdio_srst_dreq = 1;
        PERI_SUB_WRITE_REG(PERI_SUB_SC_MDIO_RESET_DREQ_REG,0,o_sc_mdio_reset_dreq.u32);
    }

    return OS_SUCCESS;
}



/*****************************************************************************
 函 数 名  : HRD_Peri_DbgSrst
 功能描述  : Dbg复位及解复位
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
UINT32 HRD_Peri_DbgSrst(UINT32 ulEn)
{
    SC_DBG_RESET_REQ_U o_sc_dbg_reset_req;
    SC_DBG_RESET_DREQ_U o_sc_dbg_reset_dreq;

    memset(&o_sc_dbg_reset_req,0,sizeof(SC_DBG_RESET_REQ_U));
    memset(&o_sc_dbg_reset_dreq,0,sizeof(SC_DBG_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_dbg_reset_req.bits.dbg_srst_req = 1;
        o_sc_dbg_reset_req.bits.cs_tracein_srst_req = 1;
        PERI_SUB_WRITE_REG(PERI_SUB_SC_DBG_RESET_REQ_REG,0,o_sc_dbg_reset_req.u32);
    }
    else
    {
        o_sc_dbg_reset_dreq.bits.dbg_srst_dreq = 1;
        o_sc_dbg_reset_dreq.bits.cs_tracein_srst_dreq = 1;
        PERI_SUB_WRITE_REG(PERI_SUB_SC_DBG_RESET_DREQ_REG,0,o_sc_dbg_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
函 数 名  : HRD_Peri_WdgSrstByNum
功能描述  : Wdg复位及解复位
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
UINT32 HRD_Peri_WdgSrstByNum(UINT32 ulWdg, UINT32 ulEn)
{
    SC_WATCHDOG_RESET_REQ_U o_sc_watchdog_reset_req;
    SC_WATCHDOG_RESET_DREQ_U o_sc_watchdog_reset_dreq;

    memset(&o_sc_watchdog_reset_req,0,sizeof(SC_WATCHDOG_RESET_REQ_U));
    memset(&o_sc_watchdog_reset_dreq,0,sizeof(SC_WATCHDOG_RESET_DREQ_U));

    if(ulWdg >= PV660_WDG_NUM)
    {
        return OS_FAIL;
    }


    if(0 == ulEn)
    {
        o_sc_watchdog_reset_req.u32 |= (1 << ulWdg);
        PERI_SUB_WRITE_REG(PERI_SUB_SC_WATCHDOG_RESET_REQ_REG,0,o_sc_watchdog_reset_req.u32);
    }
    else
    {
        o_sc_watchdog_reset_dreq.u32 |= (1 << ulWdg);
        PERI_SUB_WRITE_REG(PERI_SUB_SC_WATCHDOG_RESET_DREQ_REG,0,o_sc_watchdog_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_Peri_ItsSrst
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
UINT32 HRD_Peri_ItsSrst(UINT32 ulEn)
{
    SC_ITS_RESET_REQ_U o_sc_its_reset_req;
    SC_ITS_RESET_DREQ_U o_sc_its_reset_dreq;

    memset(&o_sc_its_reset_req,0,sizeof(SC_ITS_RESET_REQ_U));
    memset(&o_sc_its_reset_dreq,0,sizeof(SC_ITS_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_its_reset_req.bits.its_srst_req = 1;
        PERI_SUB_WRITE_REG(PERI_SUB_SC_ITS_RESET_REQ_REG,0,o_sc_its_reset_req.u32);
    }
    else
    {
         o_sc_its_reset_dreq.bits.its_srst_dreq = 1;
         PERI_SUB_WRITE_REG(PERI_SUB_SC_ITS_RESET_DREQ_REG,0,o_sc_its_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_Crg_GicSrst
 功能描述  : Gic复位及解复位
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
UINT32 HRD_Peri_GicSrst(UINT32 ulEn)
{
    SC_GIC_RESET_REQ_U o_sc_gic_reset_req;
    SC_GIC_RESET_DREQ_U o_sc_gic_reset_dreq;

    memset(&o_sc_gic_reset_req,0,sizeof(SC_GIC_RESET_REQ_U));
    memset(&o_sc_gic_reset_dreq,0,sizeof(SC_GIC_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_gic_reset_req.bits.gic_srst_req = 1;
        PERI_SUB_WRITE_REG(PERI_SUB_SC_GIC_RESET_REQ_REG,0,o_sc_gic_reset_req.u32);
    }
    else
    {
         o_sc_gic_reset_dreq.bits.gic_srst_dreq = 1;
         PERI_SUB_WRITE_REG(PERI_SUB_SC_GIC_RESET_DREQ_REG,0,o_sc_gic_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


#ifdef HRD_OS_SRE
void HRD_PoeSub_PllCtrl(u32 pll)
{
    SC_PLL_CLK_BYPASS1_U sc_pll_clk_bypass1;
    SC_SC_CLK_SEL_U      sc_clk_sel;

    sc_pll_clk_bypass1.u32 = SRE_ReadReg((UINT32 *)POE_SUB_SC_PLL_CLK_BYPASS1);
    sc_clk_sel.u32 = SRE_ReadReg((UINT32 *)POE_SUB_SC_CLK_SEL);

    /* 切换到PLL1的时钟 */
    if ( 1 == pll )
    {
        sc_pll_clk_bypass1.u32 |= (1<<1);
        sc_clk_sel.u32 |= (1<<0);

        SRE_WriteReg((void *)POE_SUB_SC_PLL_CLK_BYPASS1,sc_pll_clk_bypass1.u32);

        SRE_WriteReg((void *)POE_SUB_SC_CLK_SEL,sc_clk_sel.u32);
    }
    else    /* 使用PLL0的时钟 */
    {

        sc_pll_clk_bypass1.u32 |= (1<<0);
        sc_clk_sel.u32 &= ~(1<<0);

        SRE_WriteReg((void *)POE_SUB_SC_PLL_CLK_BYPASS1,sc_pll_clk_bypass1.u32);

        SRE_WriteReg((void *)POE_SUB_SC_CLK_SEL,sc_clk_sel.u32);
    }

    return;
}

void HRD_SwitchPoePll(void)
{
    static u8 value = 0;
    u8 pll = value++;
    pll = pll&0x1;
    HRD_PoeSub_PllCtrl(pll);
}
#endif

#ifdef HRD_OS_LINUX
/*****************************************************************************
 函 数 名  : HRD_SubPeriInit
 功能描述  : SUB PERI CRG linux初始化
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
UINT32 HRD_SubPeriInit(void)
{
    sub_peri_pa_addr = PERI_SUB_BASE;

    /* SUB ALG申请io内存 */
    if (!request_mem_region(sub_peri_pa_addr, PERI_SUB_BASE_SIZE, "SUB PERI0 Reg"))
    {
        printk("SUB PERI0 region busy\n");
        return OS_ERROR;
    }

    sub_peri_base_addr = (UINT64)ioremap(sub_peri_pa_addr, PERI_SUB_BASE_SIZE);
    if (!sub_peri_base_addr)
    {
        release_mem_region(sub_peri_pa_addr, PERI_SUB_BASE_SIZE);
        printk("could not ioremap SUB PERI0 registers\n");

        return OS_ERROR;
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_SubPeriExit
 功能描述  : SUB Peri CRG 退出
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
void HRD_SubPeriExit(void)
{
    /* 释放Peri资源 */
    if (sub_peri_base_addr > 0)
    {
        release_mem_region(sub_peri_pa_addr, PERI_SUB_BASE_SIZE);
        iounmap((void *)sub_peri_base_addr);
    }

}

EXPORT_SYMBOL(HRD_Peri_GicSrst);
EXPORT_SYMBOL(HRD_Peri_ItsSrst);
EXPORT_SYMBOL(HRD_Peri_NandcSrstByChannel);
EXPORT_SYMBOL(HRD_Peri_DmacSrstByChannel);
EXPORT_SYMBOL(HRD_Peri_LocalbusSrst);
EXPORT_SYMBOL(HRD_Peri_GpioSrstByPort);
EXPORT_SYMBOL(HRD_Peri_MdioSrst);
EXPORT_SYMBOL(HRD_Peri_DbgSrst);
EXPORT_SYMBOL(HRD_Peri_WdgSrstByNum);

#endif



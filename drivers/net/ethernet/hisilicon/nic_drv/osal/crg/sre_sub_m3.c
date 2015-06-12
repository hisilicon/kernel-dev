/*--------------------------------------------------------------------------------------------------------------------------*/
/*!!Warning: This is a key information asset of Huawei Tech Co.,Ltd                                                         */
/*CODEMARK:kOyQZYzjDpyGdBAEC2GaWuVy7vy/wDnq7gJfHBOj2pBXFF9pJtpDLt9sw5WJiMsUkN5d7jr7
aK5J3kmlnl+vpQIjFDOn3HlYdNfd+O2Mso0Wn8OrHjkJn4/rA05FKgT2JMPJkXZ1ZyV4aAJ6
v59wfQ/04NKwbYZfeMT/rIDokF+BbRw/dXS+/ENY9TrEh0Zi9tSRcZkmiGaPsyGivXNUNg7Z
2E0EnZ9V6T1bIuE6S+nSqpX3TIHOxQj8PZaFxyu/oaIzzgE1AUBdNISFeY0Yow==*/
/*--------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************

                  版权所有 (C), 2011-2021, 华为技术有限公司

 ******************************************************************************
  文 件 名   : sre_crs20.c
  版 本 号   : 初稿
  作    者   : z00176027
  生成日期   : 2013年10月17日
  最近修改   :
  功能描述   : P660 子系统20 复位及去复位
  函数列表   :
  修改历史   :
  1.日    期   : 2013年10月17日
    作    者   : z00176027
    修改内容   : 创建文件

******************************************************************************/

#include "sre_sub_m3.h"


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
UINT64 sub_m3_base_addr = 0;
UINT64 sub_m3_pa_addr = 0;


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
 inline UINT32 M3_SUB_READ_REG(UINT32 pRegBase, UINT32 ulRegIndex)
 {
     volatile UINT32 temp;

     temp = ioread32((void __iomem *)(sub_m3_base_addr + (pRegBase - M3_SUB_BASE) + (ulRegIndex << 2)));

     return temp;
 }

#define M3_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
    (iowrite32(ulValue, (void __iomem *)(sub_m3_base_addr + (pRegBase - M3_SUB_BASE) + (ulRegIndex << 2))))

#else
#define M3_SUB_READ_REG(pRegBase, ulRegIndex) \
           OS_READ_REG(pRegBase, ulRegIndex)

#define M3_SUB_WRITE_REG(pRegBase, ulRegIndex, ulValue) \
           OS_WRITE_REG(pRegBase, ulRegIndex, ulValue)

#endif






/*****************************************************************************
 函 数 名  : HRD_M3_WdgSrst
 功能描述  : Wdg复位及解复位
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
UINT32 HRD_M3_WdgSrst(UINT32 ulEn)
{
    SC_WDG_RESET_REQ_U o_sc_wdg_reset_req;
    SC_WDG_RESET_DREQ_U o_sc_wdg_reset_dreq;

    memset(&o_sc_wdg_reset_req,0,sizeof(SC_WDG_RESET_REQ_U));
    memset(&o_sc_wdg_reset_dreq,0,sizeof(SC_WDG_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_wdg_reset_req.bits.wdg_srst_req = 1;
        M3_SUB_WRITE_REG(M3_SUB_SC_WDG_RESET_REQ_REG,0,o_sc_wdg_reset_req.u32);
    }
    else
    {
         o_sc_wdg_reset_dreq.bits.wdg_srst_dreq = 1;
         M3_SUB_WRITE_REG(M3_SUB_SC_WDG_RESET_DREQ_REG,0,o_sc_wdg_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_M3_TimerSrst
 功能描述  : Timer复位及解复位
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
UINT32 HRD_M3_TimerSrst(UINT32 ulEn)
{
    SC_TIMER_RESET_REQ_U o_sc_timer_reset_req;
    SC_TIMER_RESET_DREQ_U o_sc_timer_reset_dreq;

    memset(&o_sc_timer_reset_req,0,sizeof(SC_TIMER_RESET_REQ_U));
    memset(&o_sc_timer_reset_dreq,0,sizeof(SC_TIMER_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_timer_reset_req.bits.timer_srst_req = 1;
        M3_SUB_WRITE_REG(M3_SUB_SC_TIMER_RESET_REQ_REG,0,o_sc_timer_reset_req.u32);
    }
    else
    {
         o_sc_timer_reset_dreq.bits.timer_srst_dreq = 1;
         M3_SUB_WRITE_REG(M3_SUB_SC_TIMER_RESET_DREQ_REG,0,o_sc_timer_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_M3_UartSrst
 功能描述  : M3 uart复位及解复位
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
UINT32 HRD_M3_UartSrstByPort(UINT32 ulPort,UINT32 ulEn)
{
    SC_UART_RESET_REQ_U o_sc_uart_reset_req;
    SC_UART_RESET_DREQ_U o_sc_uart_reset_dreq;

    if(ulPort >= PV660_M3_UART_NUM)
    {
        return OS_FAIL;
    }

    memset(&o_sc_uart_reset_req,0,sizeof(SC_UART_RESET_REQ_U));
    memset(&o_sc_uart_reset_dreq,0,sizeof(SC_UART_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_uart_reset_req.u32 |= (1 << ulPort);
        M3_SUB_WRITE_REG(M3_SUB_SC_UART_RESET_REQ_REG,0,o_sc_uart_reset_req.u32);
    }
    else
    {
         o_sc_uart_reset_dreq.u32 |= (1 << ulPort);
         M3_SUB_WRITE_REG(M3_SUB_SC_UART_RESET_DREQ_REG,0,o_sc_uart_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_M3_ItsSrst
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
UINT32 HRD_M3_ItsSrst(UINT32 ulEn)
{
    SC_ITS_RESET_REQ_U o_sc_its_reset_req;
    SC_ITS_RESET_DREQ_U o_sc_its_reset_dreq;

    memset(&o_sc_its_reset_req,0,sizeof(SC_ITS_RESET_REQ_U));
    memset(&o_sc_its_reset_dreq,0,sizeof(SC_ITS_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_its_reset_req.bits.its_srst_req = 1;
        M3_SUB_WRITE_REG(M3_SUB_SC_ITS_RESET_REQ_REG,0,o_sc_its_reset_req.u32);
    }
    else
    {
         o_sc_its_reset_dreq.bits.its_srst_dreq = 1;
         M3_SUB_WRITE_REG(M3_SUB_SC_ITS_RESET_DREQ_REG,0,o_sc_its_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_M3_IpcmSrst
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
UINT32 HRD_M3_IpcmSrst(UINT32 ulEn)
{
    SC_IPCM_RESET_REQ_U o_sc_ipcm_reset_req;
    SC_IPCM_RESET_DREQ_U o_sc_ipcm_reset_dreq;

    memset(&o_sc_ipcm_reset_req,0,sizeof(SC_IPCM_RESET_REQ_U));
    memset(&o_sc_ipcm_reset_dreq,0,sizeof(SC_IPCM_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_ipcm_reset_req.bits.ipcm_srst_req = 1;
        M3_SUB_WRITE_REG(M3_SUB_SC_IPCM_RESET_REQ_REG,0,o_sc_ipcm_reset_req.u32);
    }
    else
    {
         o_sc_ipcm_reset_dreq.bits.ipcm_srst_dreq = 1;
         M3_SUB_WRITE_REG(M3_SUB_SC_IPCM_RESET_DREQ_REG,0,o_sc_ipcm_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_M3_SramSrst
 功能描述  : M3 Sram复位及解复位
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
UINT32 HRD_M3_SramSrst(UINT32 ulEn)
{
    SC_SRAM_RESET_REQ_U o_sc_sram_reset_req;
    SC_SRAM_RESET_DREQ_U o_sc_sram_reset_dreq;

    memset(&o_sc_sram_reset_req,0,sizeof(SC_SRAM_RESET_REQ_U));
    memset(&o_sc_sram_reset_dreq,0,sizeof(SC_SRAM_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_sram_reset_req.bits.sram_srst_req = 1;
        M3_SUB_WRITE_REG(M3_SUB_SC_SRAM_RESET_REQ_REG,0,o_sc_sram_reset_req.u32);
    }
    else
    {
         o_sc_sram_reset_dreq.bits.sram_srst_dreq = 1;
         M3_SUB_WRITE_REG(M3_SUB_SC_SRAM_RESET_DREQ_REG,0,o_sc_sram_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_M3_SfcSrstByPort
 功能描述  : M3 SFC复位及解复位
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
UINT32 HRD_M3_SfcSrstByPort(UINT32 ulPort,UINT32 ulEn)
{
    SC_SFC_RESET_REQ_U o_sc_sfc_reset_req;
    SC_SFC_RESET_DREQ_U o_sc_sfc_reset_dreq;

    if(ulPort >= PV660_M3_SFC_NUM)
    {
        return OS_FAIL;
    }

    memset(&o_sc_sfc_reset_req,0,sizeof(SC_SFC_RESET_REQ_U));
    memset(&o_sc_sfc_reset_dreq,0,sizeof(SC_SFC_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_sfc_reset_req.u32 |= (1 << ulPort);
        M3_SUB_WRITE_REG(M3_SUB_SC_SFC_RESET_REQ_REG,0,o_sc_sfc_reset_req.u32);
    }
    else
    {
         o_sc_sfc_reset_dreq.u32 |= (1 << ulPort);
         M3_SUB_WRITE_REG(M3_SUB_SC_SFC_RESET_DREQ_REG,0,o_sc_sfc_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_M3_I2cmSrst
 功能描述  : I2cm复位及解复位
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
UINT32 HRD_M3_I2cmSrst(UINT32 ulEn)
{
    SC_I2CM_RESET_REQ_U o_sc_i2cm_reset_req;
    SC_I2CM_RESET_DREQ_U o_sc_i2cm_reset_dreq;

    memset(&o_sc_i2cm_reset_req,0,sizeof(SC_I2CM_RESET_REQ_U));
    memset(&o_sc_i2cm_reset_dreq,0,sizeof(SC_I2CM_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_i2cm_reset_req.bits.i2c_m_srst_req = 1;
        M3_SUB_WRITE_REG(M3_SUB_SC_I2CM_RESET_REQ_REG,0,o_sc_i2cm_reset_req.u32);
    }
    else
    {
         o_sc_i2cm_reset_dreq.bits.i2c_m_srst_dreq = 1;
         M3_SUB_WRITE_REG(M3_SUB_SC_I2CM_RESET_DREQ_REG,0,o_sc_i2cm_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
 函 数 名  : HRD_M3_I2cSrstByNum
 功能描述  : I2cm复位及解复位
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
UINT32 HRD_M3_I2cSrstByNum(UINT32 ulNum, UINT32 ulEn)
{
    SC_I2C_RESET_REQ_U o_sc_i2c_reset_req;
    SC_I2C_RESET_DREQ_U o_sc_i2c_reset_dreq;

    if(ulNum > PV660_M3_I2C_NUM)
    {
        return OS_FAIL;
    }

    memset(&o_sc_i2c_reset_req,0,sizeof(SC_I2C_RESET_REQ_U));
    memset(&o_sc_i2c_reset_dreq,0,sizeof(SC_I2C_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_i2c_reset_req.u32 |= (1 << ulNum);
        M3_SUB_WRITE_REG(M3_SUB_SC_I2C_RESET_REQ_REG,0,o_sc_i2c_reset_req.u32);
    }
    else
    {
         o_sc_i2c_reset_dreq.u32 |= (1 << ulNum);
         M3_SUB_WRITE_REG(M3_SUB_SC_I2C_RESET_DREQ_REG,0,o_sc_i2c_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_M3_GpioSrst
 功能描述  : M3 GPIO复位及解复位
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
UINT32 HRD_M3_GpioSrstByPort(UINT32 ulPort,UINT32 ulEn)
{
    SC_GPIO_RESET_REQ_U o_sc_gpio_reset_req;
    SC_GPIO_RESET_DREQ_U o_sc_gpio_reset_dreq;

    if(ulPort >= PV660_M3_IO_NUM)
    {
        return OS_FAIL;
    }

    memset(&o_sc_gpio_reset_req,0,sizeof(SC_GPIO_RESET_REQ_U));
    memset(&o_sc_gpio_reset_dreq,0,sizeof(SC_GPIO_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_gpio_reset_req.u32 |= (1 << ulPort);
        M3_SUB_WRITE_REG(M3_SUB_SC_GPIO_RESET_REQ_REG,0,o_sc_gpio_reset_req.u32);
    }
    else
    {
         o_sc_gpio_reset_dreq.u32 |= (1 << ulPort);
         M3_SUB_WRITE_REG(M3_SUB_SC_GPIO_RESET_DREQ_REG,0,o_sc_gpio_reset_dreq.u32);
    }

    return OS_SUCCESS;
}

/*****************************************************************************
函 数 名  : HRD_M3_CpuSrst
功能描述  : M3 CPU复位及解复位
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
UINT32 HRD_M3_CpuSrst(UINT32 ulEn)
{
    SC_CPU_RESET_REQ_U o_sc_cpu_reset_req;
    SC_CPU_RESET_DREQ_U o_sc_cpu_reset_dreq;

    memset(&o_sc_cpu_reset_req,0,sizeof(SC_CPU_RESET_REQ_U));
    memset(&o_sc_cpu_reset_dreq,0,sizeof(SC_CPU_RESET_DREQ_U));

    if(0 == ulEn)
    {
         o_sc_cpu_reset_req.bits.cpu_srst_req = 1;
         M3_SUB_WRITE_REG(M3_SUB_SC_CPU_RESET_REQ_REG,0,o_sc_cpu_reset_req.u32);
    }
    else
    {
          o_sc_cpu_reset_dreq.bits.cpu_srst_dreq = 1;
          M3_SUB_WRITE_REG(M3_SUB_SC_CPU_RESET_DREQ_REG,0,o_sc_cpu_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_M3_UsbSrst
 功能描述  : M3 USB复位及解复位
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
UINT32 HRD_M3_UsbSrstByPort(UINT32 ulPort,UINT32 ulEn)
{
    SC_USB_RESET_REQ_U o_sc_usb_reset_req;
    SC_USB_RESET_DREQ_U o_sc_usb_reset_dreq;

    if(ulPort >= PV660_M3_USB_NUM)
    {
        return OS_FAIL;
    }

    memset(&o_sc_usb_reset_req,0,sizeof(SC_USB_RESET_REQ_U));
    memset(&o_sc_usb_reset_dreq,0,sizeof(SC_USB_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_usb_reset_req.u32 |= (1 << ulPort);
        M3_SUB_WRITE_REG(M3_SUB_SC_GPIO_RESET_REQ_REG,0,o_sc_usb_reset_req.u32);
    }
    else
    {
         o_sc_usb_reset_dreq.u32 |= (1 << ulPort);
         M3_SUB_WRITE_REG(M3_SUB_SC_GPIO_RESET_DREQ_REG,0,o_sc_usb_reset_dreq.u32);
    }

    return OS_SUCCESS;
}



/*****************************************************************************
 函 数 名  : HRD_M3_SpiSrst
 功能描述  : M3 SPI复位及解复位
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
UINT32 HRD_M3_SpiSrst(UINT32 ulEn)
{
    SC_SPI_RESET_REQ_U o_sc_spi_reset_req;
    SC_SPI_RESET_DREQ_U o_sc_spi_reset_dreq;

    memset(&o_sc_spi_reset_req,0,sizeof(SC_SPI_RESET_REQ_U));
    memset(&o_sc_spi_reset_dreq,0,sizeof(SC_SPI_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_spi_reset_req.bits.spi_srst_req = 1;
        M3_SUB_WRITE_REG(M3_SUB_SC_SPI_RESET_REQ_REG,0,o_sc_spi_reset_req.u32);
    }
    else
    {
         o_sc_spi_reset_dreq.bits.spi_srst_dreq = 1;
         M3_SUB_WRITE_REG(M3_SUB_SC_SPI_RESET_DREQ_REG,0,o_sc_spi_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_M3_SecSrst
 功能描述  : M3 SEC复位及解复位
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
UINT32 HRD_M3_SecSrst(UINT32 ulEn)
{
    SC_SEC_RESET_REQ_U o_sc_sec_reset_req;
    SC_SEC_RESET_DREQ_U o_sc_sec_reset_dreq;

    memset(&o_sc_sec_reset_req,0,sizeof(SC_SEC_RESET_REQ_U));
    memset(&o_sc_sec_reset_dreq,0,sizeof(SC_SEC_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_sec_reset_req.bits.sec_srst_req = 1;
        M3_SUB_WRITE_REG(M3_SUB_SC_SEC_RESET_REQ_REG,0,o_sc_sec_reset_req.u32);
    }
    else
    {
         o_sc_sec_reset_dreq.bits.sec_srst_dreq = 1;
         M3_SUB_WRITE_REG(M3_SUB_SC_SEC_RESET_DREQ_REG,0,o_sc_sec_reset_dreq.u32);
    }

    return OS_SUCCESS;
}



/*****************************************************************************
 函 数 名  : HRD_M3_SecBuildSrst
 功能描述  : M3 SecBuild复位及解复位
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
UINT32 HRD_M3_SecBuildSrst(UINT32 ulEn)
{
    SC_SEC_BUILD_RESET_REQ_U o_sc_sec_build_reset_req;
    SC_SEC_BUILD_RESET_DREQ_U o_sc_sec_build_reset_dreq;

    memset(&o_sc_sec_build_reset_req,0,sizeof(SC_SEC_BUILD_RESET_REQ_U));
    memset(&o_sc_sec_build_reset_dreq,0,sizeof(SC_SEC_BUILD_RESET_DREQ_U));

    if(0 == ulEn)
    {
        o_sc_sec_build_reset_req.u32 |= 0x1FFFFF;
        M3_SUB_WRITE_REG(M3_SUB_SC_SEC_RESET_REQ_REG,0,o_sc_sec_build_reset_req.u32);
    }
    else
    {
         o_sc_sec_build_reset_dreq.u32 |= 0x1FFFFF;
         M3_SUB_WRITE_REG(M3_SUB_SC_SEC_RESET_DREQ_REG,0,o_sc_sec_build_reset_dreq.u32);
    }

    return OS_SUCCESS;
}


#ifdef HRD_OS_SRE
extern UINT32 SRE_UartWriteData(UINT8 ucByte);

void HRD_M3_SecureInit(void)
{
    UINT64 m3_subctrl_regbase =  M3_SUB_BASE;    //B109
    UINT32 ulCpuId = HRD_GetCpuId();

    if ( 0 != ulCpuId )
    {
        return;
    }

    SRE_UartWriteData('R');
    SRE_UartWriteData('e');
    SRE_UartWriteData('s');
    SRE_UartWriteData('e');
    SRE_UartWriteData('t');
    SRE_UartWriteData('l');
    SRE_UartWriteData('p');
    SRE_UartWriteData('m');
    SRE_UartWriteData('e');
    SRE_UartWriteData('!');

    /* 复位M3，使其进入复位状态 */
    HRD_WRITE_WORD(0xffffffff,(void *)(m3_subctrl_regbase+0xa50));


    /* 配置M3 remap到SRAM的地址 */
    HRD_WRITE_WORD(0xa4,(void *)(m3_subctrl_regbase+0x2198));   /* A4是SPI FLASH地址，EMU上M3不使用SPI FLASH启动和运行 */


    /* 配置M3 remap到SPI FLASH的地址 */
    HRD_WRITE_WORD(0xa2,(void *)(m3_subctrl_regbase+0x2194));   /* A2是SRAM地址，EMU上M3从SPI FLASH启动，实际这里remap到了SRAM */


    /* 配置M3 remap控制，配置为非安全启动，从SPI FLASH启动 */
    /*HRD_WRITE_WORD(0x0,(void *)(m3_subctrl_regbase+0x2098));  */


    /* 配置M3 remap到BOOTROM的地址 */
    /*HRD_WRITE_WORD(0xd1,(void *)(m3_subctrl_regbase+0x209c));  */


    /* 配置M3 撤离复位后，执行简易启动流程 */
    /*HRD_WRITE_WORD(0x1,(void *)(m3_subctrl_regbase+0x20b4));  */


    /* 解复位M3，使其重新启动 */
    HRD_WRITE_WORD(0xffffffff,(void *)(m3_subctrl_regbase+0xa54));

    return;
}
#endif

#ifdef HRD_OS_LINUX
/*****************************************************************************
 函 数 名  : HRD_SubM3Init
 功能描述  : SUB M3 CRG linux初始化
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
UINT32 HRD_SubM3Init(void)
{
    sub_m3_pa_addr = M3_SUB_BASE;

    /* SUB ALG申请io内存 */
    if (!request_mem_region(sub_m3_pa_addr, M3_SUB_BASE_SIZE, "SUB M3 Reg"))
    {

        printk("SUB M3 region busy\n");

        return OS_ERROR;
    }

    sub_m3_base_addr = (UINT64)ioremap(sub_m3_pa_addr, M3_SUB_BASE_SIZE);
    if (!sub_m3_base_addr)
    {
        release_mem_region(sub_m3_pa_addr, M3_SUB_BASE_SIZE);
        printk("could not ioremap SUB M3 registers\n");

        return OS_ERROR;
    }

    return OS_SUCCESS;
}


/*****************************************************************************
 函 数 名  : HRD_SubM3Exit
 功能描述  : SUB M3 CRG 退出
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
void HRD_SubM3Exit(void)
{
    /* 释放SubM3资源 */
    if (sub_m3_base_addr > 0)
    {
        release_mem_region(sub_m3_pa_addr, M3_SUB_BASE_SIZE);
        iounmap((void *)sub_m3_base_addr);
    }

}

EXPORT_SYMBOL(HRD_M3_WdgSrst);
EXPORT_SYMBOL(HRD_M3_TimerSrst);
EXPORT_SYMBOL(HRD_M3_UartSrstByPort);
EXPORT_SYMBOL(HRD_M3_IpcmSrst);
EXPORT_SYMBOL(HRD_M3_SramSrst);
EXPORT_SYMBOL(HRD_M3_SfcSrstByPort);
EXPORT_SYMBOL(HRD_M3_I2cmSrst);
EXPORT_SYMBOL(HRD_M3_I2cSrstByNum);
EXPORT_SYMBOL(HRD_M3_GpioSrstByPort);
EXPORT_SYMBOL(HRD_M3_CpuSrst);
EXPORT_SYMBOL(HRD_M3_UsbSrstByPort);
EXPORT_SYMBOL(HRD_M3_SpiSrst);
EXPORT_SYMBOL(HRD_M3_SecSrst);
EXPORT_SYMBOL(HRD_M3_SecBuildSrst);


#endif



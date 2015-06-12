/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : HRD_PCIE_KERNEL_API.h
  版 本 号   : 初稿
  作    者   : x68638
  生成日期   : 2013年8月22日
  最近修改   :
  功能描述   : PCIE模块的对外头文件，内核态使用
  函数列表   :
  修改历史   :
  1.日    期   : 2013年8月22日
    作    者   : x68638
    修改内容   : 创建文件

******************************************************************************/
#ifndef HRD_PCIE_KERNEL_API
#define HRD_PCIE_KERNEL_API

#include <linux/module.h>
#include <linux/types.h>
#include "hrd_module.h"
#include "iware_error.h"
#include "iware_serdes_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define PCIE_MAX_PORT_NUM               (4)
#define PCIE_MAX_OUTBOUND               (6)
#define PCIE_MAX_INBOUND                (4)

#define PCIE3_MAX_OUTBOUND               (16)
#define PCIE3_MAX_INBOUND                (16)


#define PCIE_LINK_LOOP_CNT          (0x1000)
#define PCIE_IATU_ADDR_MASK         (0xFFFFF000)
#define PCIE_1M_ALIGN_SHIRFT        (20)
#define PCIE_BDF_MASK               (0xF0000FFF)
#define PCIE_BUS_SHIRFT             (20)
#define PCIE_DEV_SHIRFT             (15)
#define PCIE_FUNC_SHIRFT            (12)

#define PCIE_DBI_CS2_ENABLE              (0x1)
#define PCIE_DBI_CS2_DISABLE             (0x0)

#define PCIE_DMA_MAX_BD_NUM             (0x8000)
#define PCIE_DMA_CHANLE_READ             (0x1)
#define PCIE_DMA_CHANLE_WRITE             (0x0)


/*******************************PCIE 错误信息定义*************************/
#define PCIE_ERROR_NO_BASE               HRD_ERRNO(HRD_MID_PCIE,ERRNO)
/* iatu配置结构体为空 */
#define PCIE_ERR_IATU_TABLE_NULL        HRD_ERRNO(HRD_MID_PCIE,0x0) //iatu表为空
#define PCIE_ERR_LINK_OVER_TIME         HRD_ERRNO(HRD_MID_PCIE,0x1) //等待link up超时
#define PCIE_ERR_UNIMPLEMENT_PCIE_TYPE  HRD_ERRNO(HRD_MID_PCIE,0x2) //未实现或未支持pcie类型
#define PCIE_ERR_ALREADY_INIT           HRD_ERRNO(HRD_MID_PCIE,0x3) //已经初始化了
#define PCIE_ERR_PARAM_INVALID          HRD_ERRNO(HRD_MID_PCIE,0x4) //无效参数
#define PCIE_ERR_MEM_OPT_OVER           HRD_ERRNO(HRD_MID_PCIE,0x5) //操作的地址范围超过窗口定义范围
#define PCIE_ERR_NOT_INIT               HRD_ERRNO(HRD_MID_PCIE,0x6) //pcie未初始化
#define PCIE_ERR_CFG_OPT_OVER           HRD_ERRNO(HRD_MID_PCIE,0x7) //PCIE 配置操作超过窗口范围
#define PCIE_ERR_DMA_READ_CHANLE_BUSY   HRD_ERRNO(HRD_MID_PCIE,0x8) //读通道正忙
#define PCIE_ERR_DMA_WRITE_CHANLE_BUSY   HRD_ERRNO(HRD_MID_PCIE,0x9) //写通道正忙
#define PCIE_ERR_DMAR_NO_RESORCE   HRD_ERRNO(HRD_MID_PCIE,0xa) //读通道没有资源
#define PCIE_ERR_DMAW_NO_RESORCE   HRD_ERRNO(HRD_MID_PCIE,0xb) //写通道没有资源
#define PCIE_ERR_DMA_OVER_MAX_RESORCE HRD_ERRNO(HRD_MID_PCIE,0xc) //dma超过最大资源限制
#define PCIE_ERR_NO_IATU_WINDOW             HRD_ERRNO(HRD_MID_PCIE,0xd) //未找到合适的窗口
#define PCIE_ERR_DMA_TRANSPORT_OVER_TIME         HRD_ERRNO(HRD_MID_PCIE,0xe)  //dma传输超时
#define PCIE_ERR_DMA_MEM_ALLOC_ERROR             HRD_ERRNO(HRD_MID_PCIE,0xf)  //内存分配失败
#define PCIE_ERR_DMA_ABORT                      HRD_ERRNO(HRD_MID_PCIE,0x10) //dma abort
#define PCIE_ERR_UNSUPPORT_BAR_TYPE             HRD_ERRNO(HRD_MID_PCIE,0x11) //不支持的bar类型

/*1.1.2.1 PCIE接口类型定义*/
enum pcie_port_type_e{
    PCIE_ROOT_COMPLEX,  /* RC模式 */
    PCIE_END_POINT, /* EP模式 */
    PCIE_NTB_TO_NTB,            /* NTB模式*/
    PCIE_NTB_TO_RP,
};

enum pcie_port_gen_e{
    PCIE_GEN1_0 = 1,    /*PCIE 1.0*/
    PCIE_GEN2_0 = 2,    /*PCIE 2.0*/
    PCIE_GEN3_0 = 3 /*PCIE 3.0*/
} ;

enum pcie_port_width_e {
    PCIE_WITDH_X1 = 0x1,
    PCIE_WITDH_X2 = 0x3,
    PCIE_WITDH_X4 = 0x7,
    PCIE_WITDH_X8 = 0xf,
    PCIE_WITDH_INVALID
} ;

struct pcie_port_info_s {
    enum pcie_port_type_e port_type;    /*记录PCIE接口类型，RC/EP/NTB*/
    enum pcie_port_width_e port_width;/*记录PCIE接口宽度，X1/X2/X4/X8*/
    enum pcie_port_gen_e port_gen;  /*记录PCIE接口速率,1.0/2.0/3.0*/
    int link_status;     /*记录PCIE接口的link状态*/
};

struct pcie_cfg_params_s
{
    u8 swing;     //摆幅
	struct ffe_data ffe;//preemphasis & deemphasis
	struct dfe_data dfe;
	struct ctle_data ctle;
};

enum pcie_error_type_e
{
    PCIE_CORRECTABLE_ERROR = 0,
    PCIE_NON_FATAL_ERROR,
    PCIE_FATAL_ERROR,
    PCIE_UNSUPPORTED_REQUEST_ERROR,
    PCIE_ALL_ERROR
};

union pcie_device_status_u
{
       struct
        {
            u16 correctable_error : 1;
            u16 non_fatal_error : 1;
            u16 fatal_error : 1;
            u16 unsupported_error : 1;
            u16 aux_power : 1;
            u16 transaction_pending : 1;
            u16 reserved_6_15 : 10;
       }bits;

    u16 value;
};


union pcie_uc_aer_status_u
{
      struct
        {
            u32    undefined   : 1   ; /* [0] undefined  */
            u32    reserved_1_3   : 3   ; /* reserved */
            u32    data_link_proto_error   : 1   ; /* Data Link Protocol Error Status */
            u32    reserved_5_11   : 7   ; /* reserved */
            u32    poisoned_tlp_status   : 1   ; /* Poisoned TLP Status */
            u32    flow_control_proto_error   : 1   ; /* Flow Control Protocol Error Status */
            u32    completion_time_out   : 1   ; /* Completion Timeout Status */
            u32    compler_abort_status   : 1   ; /* Completer Abort Status */
            u32    unexpect_completion_status   : 1   ; /* Unexpected Completion Status */
            u32    receiver_overflow_status   : 1   ; /*Receiver Overflow Status */
            u32    malformed_tlp_status   : 1   ; /* Malformed TLP Status*/
            u32    ecrc_error_status   : 1   ; /* ECRC Error Status */
            u32    unsupport_request_error_status   : 1   ; /* Unsupported Request Error Status */
            u32    reserved_21   : 1   ; /*  reserved */
            u32    uncorrectable_interal_error   : 1   ; /* Uncorrectable Internal Error Status */
            u32    reserved_23   : 1   ; /* reserved*/
            u32    atomicop_egress_blocked_status   : 1   ; /* AtomicOp Egress Blocked Status */
            u32    tlp_prefix_blocked_error_status   : 1   ; /* TLP Prefix Blocked Error Status */
            u32    reserved_26_31   : 1   ; /* reserved */
       }bits;

       u32 value;
};

union pcie_co_aer_status_u
{
       struct
        {
            u32    receiver_error_status   : 1   ; /* Receiver Error Status  */
            u32    reserved_1_5   : 5   ; /* Reserved */
            u32    bad_tlp_status   : 1   ; /* Bad TLP Status */
            u32    bad_dllp_status   : 1   ; /* Bad DLLP Status */
            u32    reply_num_rollover_status   : 1   ; /* REPLAY_NUM Rollover Status*/
            u32    reserved_9_11   : 3   ; /* Reserved */
            u32    reply_timer_timeout   : 1   ; /* Replay Timer Timeout Status */
            u32    advisory_nonfatal_error   : 1   ; /* Advisory Non-Fatal Error Status*/
            u32    corrected_internal_error   : 1   ; /*Corrected Internal Error Status*/
            u32    reserved_15_31   : 1   ; /* Reserved */
       }bits;
       u32 value;
};

struct pcie_aer_status_s
{
    union pcie_uc_aer_status_u uc_aer_status;
    union pcie_co_aer_status_u co_aer_status;
};



struct pcie_loop_test_result_s
{
   u32 tx_pkts_cnt; //发包个数统计
   u32 rx_pkts_cnt; //收包个数统计
   u32 error_pkts_cnt;  //错包计数
   u32 droped_pkts_cnt; //丢包计数
   union pcie_device_status_u device_status; //设备状态
   struct pcie_aer_status_s    pcie_aer_status; //AER错误状态
} ;

enum pcie_dma_channel_status_e
{
    PCIE_DMA_CS_RESERVED = 0,    /* DMA通道的当前状态 ; 0:保留,1:running；2:Halted;3:Stopped*/
    PCIE_DMA_CS_RUNNING = 1,
    PCIE_DMA_CS_HALTED = 2,
    PCIE_DMA_CS_STOPPED = 3
};

enum pcie_dma_int_type_e{
    PCIE_DMA_INT_TYPE_DONE=0,
    PCIE_DMA_INT_TYPE_ABORT,
    PCIE_DMA_INT_ALL,
    PCIE_DMA_INT_NONE
};

/*pcie 初始化模式*/
enum pcie_mode_e
{
    PCIE_EP_DEVICE = 0x0,
    LEGACY_PCIE_EP_DEVICE = 0x1,
    RP_OF_PCIE_RC = 0x4,
    PCIE_INVALID = 0x100
};

struct pcie_ntb_port_cfg /*NTB初始化数据结构*/
{
    u32 pcie_if;
    struct pcie_port_info_s port_info;
    u32 ntb_is_local;
    u64 iep_bar01; /*iep bar 01*/
    u64 iep_bar23;
    u64 iep_bar45;
    u64 iep_bar01_xlat; /*NTB_TO_NTB才起作用*/
    u64 iep_bar23_xlat; /*iep 地址转换寄存器23*/
    u64 iep_bar45_xlat;
    u64 iep_bar_lmt23; /*bar limit*/
    u64 iep_bar_lmt45; /*bar limit*/
    u64 eep_bar01;
    u64 eep_bar23;
    u64 eep_bar45;
    u64 eep_bar23_xlat;
    u64 eep_bar45_xlat;
    u64 eep_bar_lmt23; /*bar limit*/
    u64 eep_bar_lmt45; /*bar limit*/
};


struct pcie_ntb_cfg_s /*NTB初始化数据结构*/
{
    u64 iep_bar01_xlat; /*NTB_TO_NTB才起作用*/
    u64 iep_bar23_xlat; /*iep 地址转换寄存器23*/
    u64 iep_bar45_xlat;
    u64 eep_bar01;
    u64 eep_bar23;
    u64 eep_bar45;
    u64 eep_bar23_xlat;
    u64 eep_bar45_xlat;
    u64 eep_bar_lmt23; /*bar limit*/
    u64 eep_bar_lmt45; /*bar limit*/
};



/*****************************************************************************
 函 数 名  : pcie_mode_get
 功能描述  : 获取PCIE模式信息，包括接口类型、link状态、设备link宽度、link速
             度
 输入参数  : u32 pcie_if
             pcie_port_info *port_info
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月6日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_mode_get(u32 pcie_if, struct pcie_port_info_s *port_info);

/*****************************************************************************
 函 数 名  : pcie_port_ctrl
 功能描述  : 控制对PCIE端口进行使能或关闭
 输入参数  :
            u32 pcie_if : pcie控制器端口号
            u32 port_ctrl :　0，关闭端口；1：打开端口

 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月6日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_ctrl(u32 pcie_if, u32 port_ctrl);

/*****************************************************************************
 函 数 名  : pcie_link_speed_set
 功能描述  : 设置PCIE的link速率,GEN1.0,GEN2.0或GEN3.0
 输入参数  : u32 pcie_if
             pcie_port_gen_e speed
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月6日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_link_speed_set(u32 pcie_if,enum pcie_port_gen_e speed);

/*****************************************************************************
 函 数 名  : pcie_port_cfg_set
 功能描述  : 通过PCIE端口配置，设备PCIE端口的预加重、去加重、摆幅、均衡等功
             能
 输入参数  : u32 pcie_if
             pcie_cfg_params *cfg_params
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_cfg_set(u32 pcie_if, struct pcie_cfg_params_s *cfg_params);

/*****************************************************************************
 函 数 名  : pcie_port_cfg_get
 功能描述  : 获取某个PCIE端口的预加重、去加重、摆幅、均衡等的配置
 输入参数  : u32 pcie_if
             pcie_cfg_params *cfg_params
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_cfg_get(u32 pcie_if, struct pcie_cfg_params_s *cfg_params);

/*****************************************************************************
 函 数 名  : pcie_switch_clock
 功能描述  : 控制PCIE时钟的关闭和开启
 输入参数  : u32 pcie_if
             u32 clock_on
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2014年12月30日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_switch_clock(u32 pcie_if, u32 clock_on);

/*****************************************************************************
 函 数 名  : pcie_dma_cur_status_get
 功能描述  : 获取PCIE DMA引擎通道的当前运行状态
 输入参数  : u32 pcie_if
             u32 channel
             pcie_dma_channel_status_e *dma_channel_status
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern void pcie_dma_cur_status_get(u32 pcie_if, u32 channel, enum pcie_dma_channel_status_e *dma_channel_status);

/*****************************************************************************
 函 数 名  : pcie_dma_int_enable
 功能描述  : PCIE DMA通道的中断使能控制
 输入参数  : u32 pcie_if
             u32 channel
             pcie_dma_int_type_e int_type
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern void pcie_dma_int_enable(u32 pcie_if, u32 channel, enum pcie_dma_int_type_e int_type);

/*****************************************************************************
 函 数 名  : pcie_dma_int_mask
 功能描述  : PCIE DMA通道的中断屏蔽控制
 输入参数  : u32 pcie_if
             u32 channel
             pcie_dma_int_type_e int_type
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern void pcie_dma_int_mask(u32 pcie_if, u32 channel, enum pcie_dma_int_type_e int_type);

/*****************************************************************************
 函 数 名  : pcie_dma_tranfer_stop
 功能描述  : PCIE DMA通道的数据传输中止命令接口
 输入参数  : u32 pcie_if
             u32 channel
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern void pcie_dma_tranfer_stop(u32 pcie_if, u32 channel);


/*****************************************************************************
 函 数 名  : pcie_dma_int_status_get
 功能描述  : PCIE DMA中断原因获取
 输入参数  : u32 pcie_if
             u32 channel
             pcie_dma_int_type_e *dma_int_status
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern void pcie_dma_int_status_get(u32 pcie_if, u32 channel, int *dma_int_status);

/*****************************************************************************
 函 数 名  : pcie_dma_int_clear
 功能描述  : 清除某个DMA通道的中断原因寄存器
 输入参数  : u32 pcie_if
             u32 channel
             pcie_dma_int_type_e dma_int_type
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern void pcie_dma_int_clear(u32 pcie_if, u32 channel, enum pcie_dma_int_type_e dma_int_type);


/*****************************************************************************
 函 数 名  : pcie_dma_read
 功能描述  : PCIE DMA读数据接口
 输入参数  :
            u32 pcie_if : pcie控制器端口
            Void *source : 数据传输源地址（可为64位地址）
            Void *dest : 数据传输目的地址（可为64位地址）
            u32 transferSize : 传输数据的总长度
            u32 burstSize : 一次传输的数据长度

 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 PCIE DMA读数据接口，支持块传输和链式传输，transferSize大小为1B-4GB，根据burstSize
 大小，自动进行链式传输或者一次性的块传输
 此接口为异步接口
 source地址指针指向读数据的源地址，对于非NTB端口，此地址应该落在某个EP的Memroy BAR地址空间里；对于NTB
 模式，此地址应该落在PBAR23、PBAR45定义的地址空间里
 dest地址指针指向读数据的目的地址，此地址应为本地的内存地址空间

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_dma_read(u32 pcie_if,void *source, void *dest,u32 transfer_size, u32 burst_size);

/*****************************************************************************
 函 数 名  : pcie_dma_write
 功能描述  : PCIE DMA写数据接口
 输入参数  :
            u32 pcie_if : pcie控制器端口
            Void *source : 数据传输源地址（可为64位地址）
            Void *dest : 数据传输目的地址（可为64位地址）
            u32 transferSize : 传输数据的总长度
            u32 burstSize : 一次传输的数据长度

 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 PCIE DMA写数据接口，支持块传输和链式传输，transferSize大小为1B-4GB，根据burstSize
 大小，自动进行链式传输或者一次性的块传输
 此接口为异步接口
 source地址指针指向写数据的源地址，此地址应为本地的内存地址空间
 dest地址指针指向写数据的目的地址，对于非NTB端口，此地址应该落在某个EP的Memory BAR地址空间里；对于NTB
 端口，此地址应该落在PBAR23、PBAR45定义的地址空间里。
 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_dma_write(u32 pcie_if,void *source, void *dest,u32 transfer_size, u32 burst_size);

/*****************************************************************************
 函 数 名  : pcie_setup_ntb
 功能描述  : 对某个端口进行NTB的初始配置
 输入参数  : u32 pcie_if: pcie控制器端口号
             pcie_ntg_cfg *ntb_cfg: NTB配置结构体
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_setup_ntb(u32 pcie_if,struct pcie_ntb_cfg_s *ntb_cfg);

/*****************************************************************************
 函 数 名  : pcie_ntb_doorbell_send
 功能描述  : 实现NTB两侧系统之间的Doorbell中断发送功能
 输入参数  :
 输入参数    u32 pcie_if : pcie控制器端口号
             u32 doorbell : doorbell中断号，支持范围为0-31

 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_ntb_doorbell_send(u32 pcie_if,u32 doorbell);

/*****************************************************************************
 函 数 名  : pcie_loop_test_start
 功能描述  : 启动PCIE端口的环回测试功能
 输入参数  :
                u32 pcie_if : pcie控制器端口号
                u32 loop_type : 0:近端环回；1：远端环回

 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_loop_test_start(u32 pcie_if, u32 loop_type);

/*****************************************************************************
 函 数 名  : pcie_loop_test_stop
 功能描述  : 关闭PCIE端口的环回测试功能
 输入参数  : u32 pcie_if : pcie控制器端口号
             u32 loop_type : 0:近端环回；1：远端环回
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_loop_test_stop(u32 pcie_if, u32 loop_type);

/*****************************************************************************
 函 数 名  : pcie_loop_test_get
 功能描述  : 获取PCIE环回测试结果
 输入参数  : u32 pcie_if : pcie控制器端口号
             u32 loop_type : 0:近端环回；1：远端环回
 输出参数  : pcie_loop_test_result_s *test_result : 记录PCIE环回测试结果
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_loop_test_get(u32 pcie_if, u32 loop_type, struct pcie_loop_test_result_s *test_result);

/*****************************************************************************
 函 数 名  : pcie_loop_test
 功能描述  : PCIE环回测试，读写内存
 输入参数  : u32 pcie_if : pcie控制器端口号
             u32 test_bytes : 测试的字节数
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_loop_test(u32 pcie_if, u32 test_bytes);

/*****************************************************************************
 函 数 名  : pcie_port_reset
 功能描述  : PCIE端口复位，复位此PCIE端口下的所有设备，包括端口本身
 输入参数  : u32 pcie_if : pcie控制器端口号
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_reset(u32 pcie_if);

/*****************************************************************************
 函 数 名  : pcie_port_error_report_enable
 功能描述  : 使能PCIE端口的错误上报功能
 输入参数  : u32 pcie_if : pcie控制器端口号
             u32 bus ： 总线号
             u32 dev ： 设备号
             u32 func ： 功能号
             pcie_error_type_e pcie_error : PCIE错误类型，枚举型参数
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_error_report_enable(u32 pcie_if, u32 bus, u32 dev, u32 func,enum pcie_error_type_e pcie_error);

/*****************************************************************************
 函 数 名  : pcie_port_error_report_disable
 功能描述  : 禁止PCIE端口的错误上报功能
 输入参数  : u32 pcie_if : pcie控制器端口号
             u32 bus ： 总线号
             u32 dev ： 设备号
             u32 func ： 功能号
             pcie_error_type_e pcie_error : PCIE错误类型，枚举型参数
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_error_report_disable(u32 pcie_if, u32 bus, u32 dev, u32 func,enum pcie_error_type_e pcie_error);

/*****************************************************************************
 函 数 名  : pcie_device_error_status_get
 功能描述  : 获取PCIE设备的错误状态（非AER错误）
 输入参数  : u32 pcie_if : pcie控制器端口号
             u32 bus ： 总线号
             u32 dev ： 设备号
             u32 func ： 功能号
             u32 clear : 读后是否清0，为1表示读后将错误状态清零，否则不清零
 输出参数  : pcie_device_status_u *pcie_stat : PCIE错误状态结构体，记录PCIE的错误状态信息
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_device_error_status_get(u32 pcie_if, u32 bus, u32 dev, u32 func,u32 clear, \
union pcie_device_status_u *pcie_stat);

/*****************************************************************************
 函 数 名  : pcie_port_aer_cap_get
 功能描述  : 获取某个PCIE端口设备是否支持AER功能
 输入参数  : u32 pcie_if : pcie控制器端口号
             u32 bus ： 总线号
             u32 dev ： 设备号
             u32 func ： 功能号
 输出参数  : u32 *aer_cap : 是否支持AER功能，0不支持，1支持
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_aer_cap_get(u32 pcie_if, u32 bus, u32 dev, u32 func,u32 *aer_cap);

/*****************************************************************************
 函 数 名  : pcie_port_aer_status_get
 功能描述  : 获取某个PCIE端口设备所有AER错误信息
 输入参数  : u32 pcie_if : pcie控制器端口号
             u32 bus ： 总线号
             u32 dev ： 设备号
             u32 func ： 功能号
 输出参数  : pcie_uc_aer_status_u *pcie_aer_status : PCIE端口AER错误状态结构体，记录所有的AER错误状态信息
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_aer_status_get(u32 pcie_if, u32 bus, u32 dev, u32 func,union pcie_uc_aer_status_u *pcie_aer_status);

/*****************************************************************************
 函 数 名  : pcie_port_aer_status_clr
 功能描述  : 清除某个PCIE端口设备所有的AER错误状态
 输入参数  : u32 pcie_if : pcie控制器端口号
             u32 bus ： 总线号
             u32 dev ： 设备号
             u32 func ： 功能号
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_aer_status_clr(u32 pcie_if, u32 bus, u32 dev, u32 func);

/*****************************************************************************
 函 数 名  : pcie_port_aer_report_enable
 功能描述  : 使能PCIE端口的AER错误中断上报
 输入参数  : u32 pcie_if : pcie控制器端口号
             pcie_error_type_e pcie_aer_type ：AER错误类型
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_aer_report_enable(u32 pcie_if, enum pcie_error_type_e pcie_aer_type);


/*****************************************************************************
 函 数 名  : pcie_port_aer_report_disable
 功能描述  : 关闭PCIE端口的AER错误中断上报
 输入参数  : u32 pcie_if : pcie控制器端口号
             pcie_error_type_e pcie_aer_type：AER错误类型
 输出参数  : 无
 返 回 值  : int
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2013年8月8日
    作    者   : z00228490
    修改内容   : 新生成函数

*****************************************************************************/
extern int pcie_port_aer_report_disable(u32 pcie_if, enum pcie_error_type_e pcie_aer_type);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif

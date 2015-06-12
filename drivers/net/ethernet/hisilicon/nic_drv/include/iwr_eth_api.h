#ifndef _IWR_ETH_API_H_
#define _IWR_ETH_API_H_

/*调试网口*/
#define HIGMAC_DEVNAME_DEBUGGMAC                         "HiFE0" /*调试网口名*/

/*Canbus网口*/
#define HIGMAC_DEVNAME_CANGMAC                           "HiFE1" /*Canbus网口名*/

/*命令字定义*/
#define SIOCDEVDRVCMD                            0x89F4
#define ETH_MACLOOP_SET                          0x01 /*ETH_命令字,MAC环回设置,0:禁止 1:使能,无参数*/
#define ETH_MACLINELOOP_SET                      0x02 /*ETH_命令字,MAC线路环回设置,0:禁止 1:使能,无参数*/
#define ETH_STATICS_SHOW                         0x03 /*ETH_命令字,获取统计信息*/
#define ETH_SET_MAC_ADDR	                     0x05 /*ETH_命令字,设置物理地址,无参数*/
#define ETH_CHIP_RESET                           0x06 /*ETH_命令字,gmac重置,无参数*/
#define ETH_CFG_SET                              0x07 /*ETH_命令字,gmac配置设置*/
#define ETH_TX_ENABLE                            0x08 /*ETH_命令字,gmac发送通道使能,0:禁止 1:使能,无参数*/
#define ETH_RX_ENABLE                            0x09 /*ETH_命令字,gmac接受通道使能,0:禁止 1:使能,无参数*/
#define ETH_MACLOOP_TEST                         0x0A /*ETH_命令字,环回测试*/
#define ETH_PORT_MODE_SET                        0x0B /*ETH_命令字,端口模式设置*/

#define ETH_RCB_RX_CNT_SHOW                      0x0C /*ETH_命令字,显示RCB Rx方向统计信息*/
#define ETH_RCB_TX_CNT_SHOW                      0x0D /*ETH_命令字,显示RCB Tx方向统计信息*/
#define ETH_PPE_CNT_BY_PORT_SHOW                 0x0E /*ETH_命令字,显示PPE 按port统计信息*/
#define ETH_PPE_CNT_BY_QUE_SHOW                  0x0F /*ETH_命令字,显示PPE 按queue统计信息*/
#define ETH_RCB_RX_RING_SHOW                     0x10 /*ETH_命令字,显示RCB Rx ring BD信息*/
#define ETH_RCB_TX_RING_SHOW                     0x11 /*ETH_命令字,显示RCB Tx ring BD信息*/

/*网口速率设置枚举定义*/
typedef enum tagGmacPortMode
{
    GMAC_10M_MII = 0, /*10M MII*/
    GMAC_100M_MII, /*100M MII*/
    GMAC_1000M_GMII, /*1000M MII*/
    GMAC_10M_RGMII, /*10M RGMII*/
    GMAC_100M_RGMII, /*100M RGMII*/
    GMAC_1000M_RGMII, /*1000M RGMII*/
    GMAC_SERDES, /*Serdes mode*/
    GMAC_UNKNOW_MODE /*未知速率模式*/
}HIGMAC_MODE_ENUM;

/*----------------------------------------------*
 *调试网口和can网口接口            *
 *----------------------------------------------*/

/*获取统计信息结构体定义
相应命令字为HIGMAC_STATICS_GET*/
typedef struct stuInfra_ETH_STATICS
{
    U32 ulRecPktCnt;     /*收包统计值*/
    U32 ulSendPktCnt;    /*发包统计值*/
    U32 ulRecErrPktCnt;  /*收包错误统计值*/
    U32 ulSendErrPktCnt; /*发包错误统计值*/
}HIGMAC_ETH_STATICS_STRU;

/*端口模式配置结构体定义
相应命令字为HIGAMC_CFG_SET*/
typedef struct stuPORT_MODE_CFG
{
    HIGMAC_MODE_ENUM port_mode;       /*端口模式*/
    U32 ulDuplexMode;                 /*0半双工 1 全双工*/
    U32 an_enable;                    /*自协商使能*/
}HIGMAC_PORTMODE_CFG_STRU;

/*获取体检信息结构体定义
相应命令字为HIGAMC_RECORD_INFO*/
typedef struct stuRecord_reg
{
   U32 CF_INTRPT_STAT;          /*中断状态寄存器 0x434*/
   U32 PORT_MOD;                /*端口状态0x0040*/
   U32 BUS_ERR_ADDR;            /*出错总线地址 0x43C*/
   U32 TX_DROP_CNT;             /*发送方向因FIFO空而丢弃的包的统计计数器0x448*/
   U32 RX_OVER_FLOW_CNT;        /*FIFO满而丢弃的包的统计计数器0x44C*/

   U32 FIFO_CURR_STATUS;        /*FIFO当前状态 0x454*/
   U32 FIFO_HIS_STATUS;         /*FIFO历史状态寄存器 0x458*/
   U32 TX_CS_FAIL_CNT;          /*发送添加校验和失败计数器0x460*/
   U32 RX_BUFRQ_ERR_CNT;        /*接受方向申请缓存错误计数器0x58C*/
   U32 TX_BUFRL_ERR_CNT;        /*发送方向释放缓存错误计数器0x590*/

   U32 RX_WE_ERR_CNT;           /*接受方向写工作描述符错误计数器0x594*/
   U32 TX_UNDERRUN;             /*发送过程中发生内部错误而导致发送失败的次数统计0x150*/
   U32 TX_CRC_ERROR;            /*发送帧长正确CRC错误的帧数统计0x158*/
   U32 RX_LONG_ERRORS;          /*接收帧长大于允许的最大长度的帧数0x00C0*/
   U32 RX_VERY_LONG_ERR_CNT;    /*接收到超长帧统计0x0D0*/

   U32 RX_DATA_ERR;             /*接收数据错误帧统计0x0B8*/
   U32 RX_RUNT_ERR_CNT;         /*接收到帧长小于64byte大于等于12byte的帧数统计0x0D4*/
   U32 RX_SHORT_ERR_CNT;        /*小于96bit的帧数统计0x0D8*/
   U32 RX_OVERRUN_CNT;          /*FIFO溢出统计寄存器0x1EC*/
   U32 RX_FCS_ERRORS;           /*接收CRC检验错误的帧数统计0x0B0*/

   U32 RX_ALIGN_ERRORS;         /*数据非字节对齐错误帧统计0x0BC*/
   U32 TX_EXCESSIVE_LENGTH_DROP;/*超过设定的最大帧长导致发送失败次数统计0x14C*/
}HIGMAC_RECORD_REG_STRU;

/*获取网口临终遗言结构体定义
相应命令字为HIGAMC_LASTWORDS*/
typedef struct stuRecord_lastwords
{
   U32 CF_INTRPT_STAT;          /*中断状态寄存器 0x434*/
   U32 PORT_MOD;                /*端口状态0x0040*/
   U32 BUS_ERR_ADDR;            /*出错总线地址 0x43C*/
   U32 TX_DROP_CNT;             /*发送方向因FIFO空而丢弃的包的统计计数器0x448*/

   U32 RX_OVER_FLOW_CNT;        /*FIFO满而丢弃的包的统计计数器0x44C*/
   U32 FIFO_CURR_STATUS;        /*FIFO当前状态 0x454*/
   U32 FIFO_HIS_STATUS;         /*FIFO历史状态寄存器 0x458*/
   U32 TX_CS_FAIL_CNT;          /*发送添加校验和失败计数器0x460*/
   U32 RX_BUFRQ_ERR_CNT;        /*接受方向申请缓存错误计数器0x58C*/
   U32 TX_BUFRL_ERR_CNT;        /*发送方向释放缓存错误计数器0x590*/
   U32 RX_WE_ERR_CNT;           /*接受方向写工作描述符错误计数器0x594*/

   U32 TX_UNDERRUN;             /*发送过程中发生内部错误而导致发送失败的次数统计0x150*/
   U32 TX_CRC_ERROR;            /*发送帧长正确CRC错误的帧数统计0x158*/
   U32 RX_LONG_ERRORS;          /*接收帧长大于允许的最大长度的帧数0x00C0*/
   U32 RX_VERY_LONG_ERR_CNT;    /*接收到超长帧统计0x0D0*/
   U32 RX_DATA_ERR;             /*接收数据错误帧统计0x0B8*/
   U32 RX_RUNT_ERR_CNT;         /*接收到帧长小于64byte大于等于12byte的帧数统计0x0D4*/
   U32 RX_SHORT_ERR_CNT;        /*小于96bit的帧数统计0x0D8*/
   U32 RX_OVERRUN_CNT;          /*FIFO溢出统计寄存器0x1EC*/
   U32 RX_FCS_ERRORS;           /*接收CRC检验错误的帧数统计0x0B0*/
   U32 RX_ALIGN_ERRORS;         /*数据非字节对齐错误帧统计0x0BC*/
   U32 TX_EXCESSIVE_LENGTH_DROP;/*超过设定的最大帧长导致发送失败次数统计0x14C*/
}HIGMAC_LASTWORDS_REG_STRU;

/** 环回模式枚举定义 */
typedef enum tagLoopMode
{
    /* BEGIN: Modified by w00146306, 2010/2/27   问题单号:BT9D00310 */
    ETH_NO_LOOP    = 0,
    /* END:   Modified by w00146306, 2010/2/27 */
    ETH_MAC_SYS_LOOP,            /**< MAC环回      */
    ETH_MAC_LINE_LOOP,           /**< MAC线路环回  */
    ETH_PHY_SYS_LOOP,            /**< PHY环回      */
    ETH_PHY_LINE_LOOP,           /**< PHY线路环回  */
    ETH_UNKNOWN_LOOP_MODE        /**< 未知环回模式 */

} LOOP_MODE_E;

/*设置PAUSE帧配置*/
typedef struct eth_set_pause_cfg
{
   U32 pause_time;              /*pause帧时间*/
   U32 pause_gap;               /*pause帧间隙*/
   U32 pause_rsp_mode;          /*pause帧模式*/
   U32 pause_local_mac_h;       /*pause帧本地mac高位地址*/
   U32 pause_local_mac_l;       /*pause帧本地mac低位地址*/
   U32 pause_peer_mac_h;        /*pause帧对端mac高位地址*/
   U32 pause_peer_mac_l;        /*pause帧对端mac低位地址*/
}eth_set_pause_cfg_s;

/** link模式枚举定义 */
typedef enum link_mode
{
    ETH_NORMAL_MODE = 0,
    ETH_LINK_UP = 2,
    ETH_LINK_DOWN = 3,
} link_mode_e;


/*设置xge link 模式*/
typedef struct eth_set_link_cfg
{
   link_mode_e link_mode;       /*link 模式*/
   U32 rf_enable;               /*rf告警使能*/
   U32 lf_enalbe;               /*if告警使能*/
}eth_set_link_cfg_s;


/** lfrf插入模式枚举定义 */
typedef enum lfrf_mode
{
    ETH_LFRF_NORMAL_MODE = 0,
    ETH_INSERT_LF = 2,
    ETH_INSERT_RF = 3,
} lfrf_mode_e;


/*设置xge link 模式*/
typedef struct eth_set_lfrf_mode
{
   lfrf_mode_e lfrf_mode;       /*lfrf 模式*/
   U32 rf_tx_enable;            /*rf告警使能*/
   U32 unidir_enalbe;           /*单向ifrf告警使能*/
}eth_set_lfrf_mode_s;

/*ppe 中断掩码*/
typedef struct
{
    U32    qid_err_int_msk;          /*qid错误中断*/
    U32    pkt_sram_par_int_msk;     /*奇偶校验错误中断*/
}hrd_ppe_int_msk_s;

/*ppe tnl 中断掩码*/
typedef struct ppe_tnl_int_msk
{
        U32  rx_sram_par_int ;
        U32  tx_sram_par_int ;
        U32  tx_drop_int ;
        U32  rx_fifo_overflow_int ;
        U32  rx_drop_int ;
        U32  rx_no_buf_int ;
        U32  rx_pkt_int;
        U32  ddr_rw_int;
        U32  tx_bd_addr_fifo_int ;
        U32  tx_st_heat_int;
        U32  rx_st_heat_int;
        U32  pa_st_heat_int;
        U32  Reserved_20 ;
}hrd_ppe_tnl_int_msk_s;


/*xge 中断掩码*/
typedef struct xge_int_msk
{
        U32    ierr_u              ; /* [0] enables or disables the interrupt of the same name. 0=disable, 1=enable. */
        U32    reserved_3          ; /* [2..1]  */
        U32    ovf                 ; /* [3] enables or disables the interrupt of the same name. 0=disable, 1=enable. */
        U32    link_up             ; /* [4] enables or disables the interrupt of the same name. 0=disable, 1=enable. */
        U32    link_down           ; /* [5] enables or disables the interrupt of the same name. 0=disable, 1=enable. */
        U32    lf                  ; /* [6] enables or disables the interrupt of the same name. 0=disable, 1=enable. */
        U32    rf                  ; /* [7] enables or disables the interrupt of the same name. 0=disable, 1=enable. */
        U32    sd                  ; /* [8] enables or disables the interrupt of the same name. 0=disable, 1=enable.      <br><b>note: the field is obsolete, please don't use it again.</b> */
        U32    sf                  ; /* [9] enables or disables the interrupt of the same name. 0=disable, 1=enable.      <br><b>note: the field is obsolete, please don't use it again.</b> */
        U32    wis                 ; /* [10] enables or disables the interrupt of the same name. 0=disable, 1=enable.      <br><b>note: the field is obsolete, please don't use it again.</b> */
        U32    mib_ecc_uni         ; /* [11] enables or disables the interrupt of the same name. 0=disable, 1=enable.      <br><b>note: the field is obsolete, please don't use it again.</b> */
        U32    mib_ecc_mul         ; /* [12] enables or disables the interrupt of the same name. 0=disable, 1=enable.      <br><b>note: the field is obsolete, please don't use it again.</b> */
        U32    fec_ecc_uni         ; /* [13] enables or disables the interrupt of the same name. 0=disable, 1=enable.      <br><b>note: the field is obsolete, please don't use it again.</b> */
        U32    fec_ecc_mul         ; /* [14] enables or disables the interrupt of the same name. 0=disable, 1=enable.      <br><b>note: the field is obsolete, please don't use it again.</b> */
        U32    reserved_17         ; /* [31..15]  */
}hrd_xge_int_msk_s;


/* 网口错误码定义 */
#define HRD_NIC_FREE_BUF_ERROR              HRD_ERRNO(HRD_MID_NIC,0x1)
#define HRD_NIC_RawMUTEX_ERROR              HRD_ERRNO(HRD_MID_NIC,0x2)
#define HRD_NIC_PKT_LEN_ERROR               HRD_ERRNO(HRD_MID_NIC,0x3)
#define HRD_NIC_PKT_DROP_ERROR              HRD_ERRNO(HRD_MID_NIC,0x4)
#define HRD_NIC_PKT_L2_ERROR                HRD_ERRNO(HRD_MID_NIC,0x5)
#define HRD_NIC_INVLD_PKT_ERROR             HRD_ERRNO(HRD_MID_NIC,0x6)
#define HRD_NIC_INT_OPEN_CLOSE_ERR          HRD_ERRNO(HRD_MID_NIC,0x7)

#endif


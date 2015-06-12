
#ifndef _MCSS_MOD_API_H_
#define _MCSS_MOD_API_H_

#include "hrd_mod_common_api.h"


/**
 * 公共命令字
 */
typedef enum
{
    MCSS_CMD_GMAC_INIT = 0,             /**< 初始化 */
    MCSS_CMD_GMAC_SET_PORT,             /**< 设置GMAC端口，如速率、工作模式、自协商使能、自协商模式 */

    MCSS_CMD_GMAC_SET_MAC = 3,          /**< 设置GMAC参数, 命令2传不进去，待确认 */
    MCSS_CMD_GMAC_SET_PID,              /**< 设置进程ID */
    MCSS_CMD_GMAC_SET_TX_PORT,          /**< 设置发送端口 */
    MCSS_CMD_GMAC_RX_PKT,               /**< 接收报文 */
    MCSS_CMD_GMAC_SET_CORE_GRP_BIND,    /**< 设置VCPU与GROUP绑定关系 */
    MCSS_CMD_GMAC_SET_QUE_CNT,          /**< 设置POE队列收包个数达到多少个上报中断 */
    MCSS_CMD_GMAC_LOOP,                 /**< 设置GMAC内环 */
    MCSS_CMD_GMAC_GE_SHOW,              /**< 查询GE收发包信息，直接内核打印 */
    MCSS_CMD_GMAC_PPE_SHOW,             /**< 查询PPE收发包信息，直接内核打印 */
    MCSS_CMD_GMAC_POE_SHOW,             /**< 查询POE收发包信息，直接内核打印 */
    MCSS_CMD_GMAC_BMU_SHOW,             /**< 查询BMU收发包信息，直接内核打印 */
    MCSS_CMD_SRIO_READ,                 /**< SRIO read */
    MCSS_CMD_SRIO_WRITE,                /**< SRIO write */
    MCSS_CMD_SRIO_ALLOC_BDBUF,          /**< SRIO alloc pool for BD */
    MCSS_CMD_SRIO_SET_SPEED,            /**< SRIO link speed select */
    MCSS_CMD_SERDES2_SETRD,             /**< SERDES2 set round */
    MCSS_CMD_BMU_GET_POOL,              /**< get pool id and real size through expect size */
    MCSS_CMD_BMU_ALLOC_POOL,            /**< alloc pool through pool id */
    MCSS_CMD_BMU_FREE_POOL,             /**< free pool through addr and pool id */
    MCSS_CMD_PRINTK_BUILD_TIME,         /**< 打印编译时间 */
    MCSS_CMD_SET_PRINTK_LEVEL,          /**< 设置打印级别 */
    MCSS_CMD_GMAC_GET_PORT,             /**< 获取GMAC端口，如速率、工作模式、自协商使能、自协商模式 */
    MCSS_CMD_GMAC_GET_ANG_ST,           /**< 自协商模式链接状态 */

    /* Begin add by z00200342 2013-8-5 DTS2013080504754  */
    /* 1381与650不一样，650软件实现，1381配置硬件        */

    MCSS_CMD_GMAC_STREAM_TRANSMIT_ADD,  /**< 转发MAC地址添加接口 */
    MCSS_CMD_GMAC_STREAM_TRANSMIT_DEL,  /**< 转发MAC地址删除接口 */
    MCSS_CMD_GMAC_STREAM_TRANSMIT_QUERY,/**< 转发配置查询接口 */
    /* End add by z00200342 2013-8-5 DTS2013080504754 */

    /* Begin add by z00200342 2013-8-5 DTS2013080504885 */
    MCSS_CMD_BMU_COMM_INIT,             /**< BMU 硬件公共初始化 */
    MCSS_CMD_POE_COMM_INIT,             /**< POE 硬件公共初始化 */
    MCSS_CMD_GMAC_COMM_INIT,            /**< GMAC硬件公共初始化 */
    MCSS_CMD_SRIO_COMM_INIT,            /**< SRIO硬件公共初始化 */
    MCSS_CMD_TM_COMM_INIT,              /**< TM  硬件公共初始化 */
    /* End add by z00200342 2013-8-5 DTS2013080504885 */

    MCSS_CMD_GMAC_SHARE_UC_SHOW,        /**< 各个gmac公共的单播地址配置展示 */
    MCSS_CMD_GMAC_FILTER_SHOW,          /**< 过滤配置显示 */

    /* Begin add by z00200342 2013-9-6: 增加发送接收针对指定端口号和MAC的统计 */
    MCSS_CMD_GMAC_ENABLE_SOFT_CNT,      /**< 使能软件统计，指定端口号和MAC值 */
    MCSS_CMD_GMAC_GET_SOFT_CNT,         /**< 获取软件统计 */
    MCSS_CMD_GMAC_CLEAR_SOFT_CNT,       /**< 清除统计值 */
    /* End add by z00200342 2013-9-6 */

    MCSS_CMD_GMAC_STREAM_TRANSMIT_CFG,  /**< GSM流转发配置 */
    MCSS_CMD_INVALID                    /**< 无效的IOCTRL命令 */
} MCSS_CMD_TYPE_ENUM;

/**
 * 全双工，半双工枚举定义
 */
typedef enum
{
    MCSS_GMAC_HALF_DUPLEX_MODE = 0,    /**< 0: 半双工 */
    MCSS_GMAC_FULL_DUPLEX_MODE         /**< 1：全双工  */
} MCSS_DUPLEX_TYPE_E;

/**
 * 接口类型
 */
typedef enum {
    MCSS_GMAC_SPEED_10M_MII = 0,       /* reserve */
    MCSS_GMAC_SPEED_100M_MII,          /* reserve */
    MCSS_GMAC_SPEED_1000M_GMII,        /* reserve */
    MCSS_GMAC_SPEED_10M_RGMII,         /* reserve */
    MCSS_GMAC_SPEED_100M_RGMII,        /* reserve */
    MCSS_GMAC_SPEED_1000M_RGMII,       /* reserve */
    MCSS_GMAC_SPEED_10M_SGMII,
    MCSS_GMAC_SPEED_100M_SGMII,
    MCSS_GMAC_SPEED_1000M_SGMII,
    MCSS_GMAC_SPEED_10000M_SGMII       /* reserve */
} MCSS_PORT_MODE_E;

/**
 * 自协商使能
 */
typedef enum {
    MCSS_GMAC_AUTONEG_DISABLE = 0,      /* 自协商不使能 */
    MCSS_GMAC_AUTONEG_ENABLE            /* 自协商使能 */
} MCSS_AUTONEG_EN_E;

/**
 * 自协商模式
 */
typedef enum {
    MCSS_GMAC_AUTONEG_1000BASEX = 0,    /* 1000base-X */
    MCSS_GMAC_AUTONEG_SGMII_PHY,        /* SGMII phy模式 */
    MCSS_GMAC_AUTONEG_SGMII_MAC,        /* SGMII mac模式 */
    MCSS_GMAC_AUTONEG_RESERVE           /* 保留的不存在的模式 */
} MCSS_AUTONEG_MODE_E;

/**
 * 初始化配置信息
 */
typedef struct
{
    UINT32 port;
} MCSS_CMD_GMAC_INIT_ST;

/**
 * GE口初始化配置信息
 */
typedef struct
{
    UINT32 port;                       /* 端口号 */
    MCSS_DUPLEX_TYPE_E  duplex_type;   /* 双工模式 */
    MCSS_PORT_MODE_E    speed;         /* 速率 */
    MCSS_AUTONEG_EN_E   autoneg_en;    /* 自协商使能 */
    MCSS_AUTONEG_MODE_E autoneg_mode;  /* 自协商模式 */
} MCSS_CMD_GMAC_SET_PORT_ST;

/**
 * 自协商初始化配置信息
 */
typedef struct
{
    UINT32  port;                       /* 端口号 */
    UINT32  autoneg_st;                /* 自协商状态 */
} MCSS_CMD_GMAC_AUNEG_ST;

/**
 * mac地址
 */
typedef struct
{
    U8 mac[6];
} MCSS_TYPE_MAC_S;

/**
 * GE MAC类型
 */
typedef struct
{
    UINT32 pid;
    MCSS_TYPE_MAC_S mac;
} MCSS_CMD_GMAC_SET_MAC_ST;

/**
 * GSM配置结构体
 */
typedef struct
{
    UINT32 uwSwitchGrp;    /**< 转发报文Grp，需要为POE单独划分一个group作为转发端口 */
    UINT32 uwSwitchQos;    /**< 转发报文Qos*/
    UINT32 uwVmid;         /**< 硬件转发配置的VMID，多模场景时，必须和用于转发Group的VMID配置一致 */
}GE_TRANSMIT_CFG_S;

/**
 * GSM转发配置之已配置的mac地址信息结构体
 */
typedef struct
{
    UINT32 uwDesPortId;         /**< 目的端口号 */
    MCSS_TYPE_MAC_S ucCfgMac;   /**< 已经配置的MAC地址 */
}TRANSMIT_CFG_S;

/**
 * GSM转发配置查询结构体
 */
typedef struct
{
    UINT32 uwSwitchGrp;                /**<转发报文Grp */
    UINT32 uwSwitchQos;                /**<转发报文Qos */
    UINT32 uwVmid;                     /**<配置的VMID*/
    TRANSMIT_CFG_S stTransmitCfg[8];   /**< 已经配置的MAC地址 */
}GE_TRANSMIT_QUERY_S;

/**
 * GSM参数配置结构体
 */
typedef union
{
    struct
    {
        UINT32 uwPort;                 /**< 需配置的端口号 */
        GE_TRANSMIT_CFG_S stGeTranCfg; /**< GSM配置结构体 */
    } in;
} MCSS_CMD_GE_GSM_TRAN_CFG_PARAM;

/**
 * GSM转发MAC地址添加配置参数
 */
typedef union
{
    struct
    {
        UINT32 uwSrcPortId;
        UINT32 uwDesPortId;
        MCSS_TYPE_MAC_S stMac; /**< GSM 转发mac地址*/
    } in;
} MCSS_CMD_GE_GSM_TRAN_ADD_PARAM;

/**
 * GSM转发MAC地址删除配置参数
 */
typedef union
{
    struct
    {
        UINT32 uwSrcPortId;
        UINT32 uwDesPortId;
        MCSS_TYPE_MAC_S stMac; /**< 待删除的GSM 转发mac地址*/
    } in;
} MCSS_CMD_GE_GSM_TRAN_DEL_PARAM;


/**
 * GSM转发配置查询参数
 */
typedef union
{
    struct
    {
        UINT32 uwPortId;
    } in;
    struct
    {
        GE_TRANSMIT_QUERY_S stTranmitQuery; /**<GSM转发 配置参数  */
    } out;
} MCSS_CMD_GE_GSM_TRAN_QUERY_PARAM;

/**
 * GE收包
 */
#define MCSS_GMAC_RX_MAX_NUM 0x1000
typedef struct
{
    UINT32 uwNum;       /**< 接收的报文数 */
    struct tagInfo      /**< 接收报文信息结构体 数组 */
    {
        UINT32 uwPort;  /**< 端口号 */
        UINT32 uwPkt;   /**< 报文地址 */
        UINT32 uwLen;   /**< 长度 */
        UINT32 uwPool;  /**< 报文存储的pool号 */
        UINT32 uwBuf;   /**< 报文所属内存块的地址 */
    } stInfo[MCSS_GMAC_RX_MAX_NUM];
} MCSS_CMD_GMAC_RX_PKT_S;

/**
 * 接收方向多buffer回收
 */
#define MCSS_GMAC_RX_MAX_NUM 0x1000
typedef struct
{
    UINT32 uwNum;                        /**< 接收的报文数 */
    UINT32 auwBuf[MCSS_GMAC_RX_MAX_NUM];  /**< 报文所属内存块的地址 */
} MCSS_CMD_GMAC_RX_MULTI_FREE_S;


/**
 * VCPU与GROUP绑定配置结构体
 */
typedef struct
{
    UINT32 vcpuId;      /**< VCPUID */
    UINT32 groupMask;    /**< GROUP掩码 */
} MCSS_CMD_GMAC_SET_CORE_GRP_BIND_ST;

/**
 * GRP收包中断绑定配置结构体
 */
typedef struct
{
    UINT32 group;       /**< group号 */
    UINT32 cnt;         /**< group收包个数达到收包多少个上报中断 */
} MCSS_CMD_GMAC_SET_QUE_CNT_ST;

/**
 * 端口环回使能配置结构体
 */
typedef struct
{
    UINT32 port;        /**< 端口号 */
    UINT32 inLoopEn;    /**< 应用侧环回使能/不使能 */
    UINT32 lineLoopEn;  /**< 链路侧环回使能/不使能 */
} MCSS_CMD_GMAC_LOOP_ST;

/* @ingroup    srio
*srio读写数据结构体定义.
*相应命令字为SRIO_CMD_READ 和 SRIO_CMD_WRITE*/
typedef struct
{
    U32 ulPort;                       /* SRIO端口号 */
    U32 ulDeviceId;                   /* 对端设备ID */
    U32 ulOperAddr;                   /* 对端地址(物理地址) */
    U8 *pulDataBuffer;                /* 本地地址(虚拟地址) */
    U32 ulLength;                     /* 数据长度 */
} MCSS_SRIO_RW_ST;

/* @ingroup    bmu
*bmu申请pool结构体定义.
*相应命令字为MCSS_CMD_BMU_GET_POOL&MCSS_CMD_BMU_ALLOC_POOL&MCSS_CMD_BMU_FREE_POOL */
typedef union
{
    //输入参数
    struct
    {
        UINT32 poolid;
        void* addr;                   //地址,传递pool空间的起始地址
        UINT32 size;                  //期望大小
    }input;
    //输出参数
    struct
    {
        UINT32 poolid;
        void* addr;                   //地址,传递pool空间的起始地址
        UINT32 poolsize;              //该pool的实际大小
    }output;
}MCSS_BMU_POOL_ALLOC_U;

typedef union
{
    struct
    {
        UINT32 uwPrintkLevel;
    } in;
} MCSS_CMD_SET_PRINTK_LEVEL_PARAM;

//srio set speed enum
typedef enum enSrioSpeedEx
{
    MCSS_SRIO_1G25,
    MCSS_SRIO_2G5,
    MCSS_SRIO_3G125,
    MCSS_SRIO_5G,
    MCSS_SRIO_6G25
}EnSrioSpeedEx;

/* Begin add by z00200342 2013-8-5 DTS2013080504754  */
/**< 每个端口支持可配置的转发MAC地址的最大数量 */
#define MCSS_GMAC_PORT_STREAM_TRANSMIT_MAX_MAC_NUM  (0x8UL)

/**
 * GSM流转发结构体
 */
typedef struct
{
    UINT32 uwInPort;                /**< 源(输入)端口 */
    MCSS_TYPE_MAC_S stMac;          /**< 需转发的MAC地址 */
    UINT32 uwOutPort;               /**< 目的(转出端口) */
} MCSS_GMAC_STREAM_TRANSMIT_ST;

/**
 * 增加GSM流转发配置命令的参数
 */
typedef union
{
    MCSS_GMAC_STREAM_TRANSMIT_ST in;
} MCSS_CMD_GMAC_STREAM_TRANSMIT_ADD_PARAM;

/**
 * 删除GSM流转发配置命令的参数
 */
typedef union
{
    struct
    {
        UINT32 uwInPort;                /**< 源(输入)端口 */
        MCSS_TYPE_MAC_S stMac;          /**< 需转发的MAC地址 */
    } in;
} MCSS_CMD_GMAC_STREAM_TRANSMIT_DEL_PARAM;

/**
 * 查询GSM流转发配置命令的参数
 */
typedef union
{
    struct
    {
        UINT32 uwInPort;                /**< 源(输入)端口 */
    } in;   /**< 输入: 端口号 */
    struct
    {
        UINT32 uwMacNum;                /**< 已配置的转发MAC数 */
        MCSS_GMAC_STREAM_TRANSMIT_ST stInfo[MCSS_GMAC_PORT_STREAM_TRANSMIT_MAX_MAC_NUM];
    } out;  /**<输出: 该端口对应的转发MAC配置信息 */
} MCSS_CMD_GMAC_STREAM_TRANSMIT_QUERY_PARAM;

/* End add by z00200342 2013-8-5 DTS2013080504754 */

/* Begin add by z00200342 2013-8-5 DTS2013080504885 */

/**
 * BMU硬件公共初始化参数
 */
typedef union
{
    struct
    {
        UINT32 uwOverFlowSpaceAddr;     /**< 溢出空间起始地址 */
        UINT32 uwOverFlowSpaceSize;     /**< 溢出空间大小 */
    } in;
} MCSS_CMD_BMU_COMM_INIT_PARAM;

/**
 * POE硬件公共初始化
 */
typedef union
{
    struct
    {
        UINT32 uwOverFlowSpaceAddr;     /**< 溢出空间起始地址 */
        UINT32 uwOverFlowSpaceSize;     /**< 溢出空间大小 */
    } in;
} MCSS_CMD_POE_COMM_INIT_PARAM;

/**
 * GMAC硬件公共初始化
 */
typedef union
{
    struct
    {
        UINT32 uwOverFlowSpaceAddr;     /**< 溢出空间起始地址 */
        UINT32 uwOverFlowSpaceSize;     /**< 溢出空间大小 */
    } in;
} MCSS_CMD_GMAC_COMM_INIT_PARAM;

/**
 * SRIO硬件公共初始化
 */
typedef union
{
    struct
    {
        UINT32 uwOverFlowSpaceAddr;     /**< 溢出空间起始地址 */
        UINT32 uwOverFlowSpaceSize;     /**< 溢出空间大小 */
    } in;
} MCSS_CMD_SRIO_COMM_INIT_PARAM;

/**
 * TM硬件公共初始化
 */
typedef union
{
    struct
    {
        UINT32 uwOverFlowSpaceAddr;     /**< 溢出空间起始地址 */
        UINT32 uwOverFlowSpaceSize;     /**< 溢出空间大小 */
    } in;
} MCSS_CMD_TM_COMM_INIT_PARAM;

/* End add by z00200342 2013-8-5 DTS2013080504885 */

/* Begin add by z00200342 2013-9-6: 增加发送接收针对指定端口号和MAC的统计 */
/**
 * 使能软件统计，指定端口号和MAC值
 */
typedef union
{
    struct
    {
        UINT32 uwTxRx;          /**< 0: 发送方向 1:接收方向 */
        UINT32 uwEnable;        /**< 0: 不使能 1: 使能 */
        UINT32 uwPort;          /**< 统计的端口号 */
        MCSS_TYPE_MAC_S stMac;  /**< 指定需要统计的MAC */
    } in;
} MCSS_CMD_GMAC_ENABLE_SOFT_CNT_PARAM;

/**
 * 获取软件统计
 */
typedef union
{
    struct
    {
        UINT32 uwTxRx;          /**< 0: 发送方向 1:接收方向 */
    } in;
    union
    {
        struct
        {
            UINT32 uwEnable;        /**< 是否已使能软件统计 */
            UINT32 uwPort;          /**< 待统计的端口 */
            UINT32 uwTotalCnt;      /**< 总数统计 */
            UINT32 uwTotalOkCnt;    /**< 成功总数统计 */
            UINT32 uwMacCnt;        /**< 指定MAC总数统计 */
            UINT32 uwMacOkCnt;      /**< 指定MAC成功总数统计 */
            UINT32 auwResv[4];
            MCSS_TYPE_MAC_S stMac;  /**< 指定统计的MAC */
        } tx;
        struct
        {
            UINT32 uwEnable;        /**< 是否已使能软件统计 */
            UINT32 uwPort;          /**< 待统计的端口 */
            UINT32 uwTotalCnt;      /**< 总数统计 */
            UINT32 uwTotalOkCnt;    /**< 上送用户态总数统计 */
            UINT32 uwTotalTxCnt;    /**< 转发报文总数统计 */
            UINT32 uwTotalFailCnt;  /**< 失败的报文数统计 */
            UINT32 uwMacCnt;        /**< 指定MAC总数统计 */
            UINT32 uwMacOkCnt;      /**< 指定MAC上送用户态总数统计 */
            UINT32 uwMacTxCnt;      /**< 指定MAC转发总数统计 */
            UINT32 uwMacFailCnt;    /**< 指定MAC接收失败的总数统计 */
            MCSS_TYPE_MAC_S stMac;  /**< 指定统计的MAC */
            UINT32 uwBadBufAddrCnt; /**< 异常buf地址统计 */
            UINT32 uwLastBadBufAddr;/**< 最后一个异常buf地址 */
        } rx;
    } out;
} MCSS_CMD_GMAC_GET_SOFT_CNT_PARAM;

/* End add by z00200342 2013-9-6 */

/* BEGIN: Added by w00200395, 2013/12/10   问题单号:DTS2013112610434 */
typedef union
{
    struct
    {
        UINT32 uwNum;          /**< 需要释放的buffer数 */
        void*  pstRxMultFree;  /**< 接收释放buffer的索引 */
    } in;
} MCSS_CMD_ETH_MULTI_FREE_PARAM;
/* END:   Added by w00200395, 2013/12/10 */

/* SRE标准接口命令字/参数定义 */
#include "iware_comm_kernel_api.h"
#include "iware_pcie_kernel_api.h"



#endif /* _MCSS_MOD_API_H_ */


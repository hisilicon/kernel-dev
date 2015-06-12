/******************************************************************************

                  版权所有 (C), 2012-2016, 华为技术有限公司

 ******************************************************************************
  文 件 名   : hrd_log.h
  版 本 号   : 初稿
  作    者   : z00142220
  生成日期   : 2013年7月26日
  最近修改   :
  功能描述   : 日志公共机制头文件，定义公共的日志记录宏定义，接口原型等
  函数列表   :
  修改历史   :
  1.日    期   : 2013年7月26日
    作    者   : z00142220
    修改内容   : 创建文件

******************************************************************************/
#ifndef _HRD_LOG_H
#define _HRD_LOG_H

#include "osal_api.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/* 默认日志ID */
#define HRD_DEF_LOG_ID 0xffffffffUL

#if 0  /* HRD暂不支持日志记录，只用打印 */
/**
 * 本地日志种类标志
 */
typedef enum
{
    OS_LOG_TYPE_TEXT = 0,           /**< 文本类型   */
    OS_LOG_TYPE_BIN  = 1,           /**< 二进制类型 */
    OS_LOG_TYPE_BUTT
}OS_LOG_TYPE_E;


/**
 * 分区日志需要通过主核LogAgent上报
 */
typedef enum
{
    OS_LOG_REPORT_OFF = 0,          /**< 不允许上报 */
    OS_LOG_REPORT_ON  = 1,          /**< 允许上报   */
    OS_LOG_REPORT_BUTT
}OS_LOG_REPORT_E;

/**
 * 日志是否允许覆盖的标志，针对report过程有效
 */
typedef enum
{
    OS_LOG_OVERWRITE_OFF = 0,       /**< 不允许循环覆盖 */
    OS_LOG_OVERWRITE_ON  = 1,       /**< 允许循环覆盖   */
    OS_LOG_OVERWRITE_BUTT
}OS_LOG_OVERWRITE_E;


/**
* LOG过滤钩子函数指针，返回为OS_SUCCESS表示该日志被过滤不记录，否则记录
*/
typedef UINT32 (*LOG_FILTER_HOOK)(UINT32 uwPtNum,UINT32 uwLogId, UINT32 uwUserData);


/**
* LOG打印钩子函数指针
*/
typedef VOID (*LOG_PRINT_HOOK)( UINT32 uwPtNum, UINT32 uwTimeStamp,
                                UINT32 uwLogId, UINT32 uwUserData,
                                UINT32 len, VOID *pLogItem);


/**
* LOG分区属性
*/
typedef struct tagLogRegionAttr
{
    CHAR   *name;                   /**< 分区名称，最长为OS_LOG_NAME_LEN    */
    UINT32  startAddr;              /**< 分区起始虚拟地址，必须是32字节对齐 */
    UINT32  length;                 /**< 分区长度，不能导致地址翻转         */
    OS_LOG_TYPE_E      type;        /**< 分区LOG类型                        */
    OS_LOG_REPORT_E    reportflag;  /**< 分区LOG记录通过主核Agent上报标识   */
    OS_LOG_OVERWRITE_E overflag;    /**< 分区LOG覆盖标志，针对report过程    */
    LOG_FILTER_HOOK    pfnFilter;   /**< 分区纪录过滤函数                   */
    LOG_PRINT_HOOK     pfnPrint;    /**< 分区LOG打印函数, 二进制日志有效    */
}IWARE_LOG_REGION_ATTR_S;

/*****************************************************************************
 函 数 名  : IWARE_LogPtCreate
 功能描述  : 创建LOG分区
 输入参数  : IWARE_LOG_REGION_ATTR_S *pstRegionAttr--日志分区属性

 输出参数  : UINT32 *puwPtId--日志分区ID
 返 回 值  :
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   :
    作    者   :
    修改内容   : 新生成函数
*****************************************************************************/
extern UINT32 IWARE_LogPtCreate(IWARE_LOG_REGION_ATTR_S *pstRegionAttr, UINT32 *puwPtId);
#endif

/* 定义DBG调试信息是否打印的控制开关，由每个模块自己进行重定义，0表示关闭打印，1表示打开打印 */
/* 例如: #undef HRD_DBG_ENABLE                                                   */
/*       #define HRD_DBG_ENABLE   0                                              */
#define HRD_DBG_ENABLE IWARE_LOG_LEVEL_DEBUGGING

/* ERROR等级定义 */
typedef enum tagiWareLogLevel
{
    IWARE_LOG_LEVEL_DEBUGGING = 0,  /* 调试级别 */
    IWARE_LOG_LEVEL_INFORMATIONAL,  /* 信息级别 */
    IWARE_LOG_LEVEL_NOTIFICATION,   /* 正常级别 */
    IWARE_LOG_LEVEL_WARNING,        /* 警告级别 */
    IWARE_LOG_LEVEL_ERROR,          /* 错误级别 */
    IWARE_LOG_LEVEL_CRITICAL,       /* 严重级别 */
    IWARE_LOG_LEVEL_ALERT,          /* 告警级别 */
    IWARE_LOG_LEVEL_EMERGENCY,      /* 紧急级别 */
}IWARE_LOG_LEVEL_S;

#if 0  /* HRD暂不支持日志记录，只用打印 */
/*****************************************************************************
 函 数 名  : IWARE_LogOutPut
 功能描述  : 文本日志记录
 输入参数  : UINT32 uwPtId--日志分区，在SRE中使用日志需要先创建一个分区
             UINT32 uwLogId--日志ID，用户自己定义
             UINT32 uwModeId--模块ID
             UINT32 uwLevel--日志严重级别
             VOID *format--用户自定义的类型格式化字符串指针，决定输出格式
                            必须保证指向的字符串存放在一直会保留的内存区中
             UINT32 uwPara1~uwPara4--用户自定义的四个参数
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   :
    作    者   :
    修改内容   : 新生成函数
*****************************************************************************/
UINT32 IWARE_LogOutPut(UINT32 uwPtId, UINT32 uwLogId, UINT32 uwModeId, UINT32 uwLevel, char *format,
                      UINT32 uwPara1, UINT32 uwPara2, UINT32 uwPara3, UINT32 uwPara4);



/*****************************************************************************
 函 数 名  : IWARE_LogShow
 功能描述  : 文本日志记录
 输入参数  : UINT32 uwPtId--分区号
             UINT32 uwNum--需要显示的日志条数
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :
 修改历史      :
  1.日    期   :
    作    者   :
    修改内容   : 新生成函数
*****************************************************************************/
extern UINT32 IWARE_LogShow(UINT32 uwPtId, UINT32 uwNum);
#endif

/*****************************************************/
/* hrd_log: hrd平台日志记录宏                      */
/*****************************************************/
#if 1
 /*当前不支持日志，控制日志打印到屏幕 */
#define hrd_log(level, enable, log_id, format, para1, para2, para3, para4) \
    do  \
    {   \
        if (unlikely(level >= enable)) \
        {  \
            osal_printf("%s:%d " format, __func__, __LINE__, para1, para2, para3, para4);  \
        }  \
    }   \
    while (0)

#else
/* HRD暂不支持日志记录，只用打印 */
/****************************************************************************************/
/* 正式发布版本打印到系统日志或者上层回调接口， 假设同一个模块，日志存放到同一个分区中  */
/* 不同等级日志记录宏，各模块可自己定制修改HRD_THIS_MODULE_PT_ID和HRD_THIS_MODULE_ID    */
/* 例如: #undef HRD_THIS_MODULE_PT_ID                                                   */
/*       #define HRD_THIS_MODULE_PT_ID   0                                              */
/*       #undef HRD_THIS_MODULE_ID                                                      */
/*       #define HRD_THIS_MODULE_ID   0                                                 */
/****************************************************************************************/
 #define hrd_log(level, log_id, format, para1, para2, para3, para4) \
    do  \
    {   \
        IWARE_LogOutPut(HRD_THIS_MODULE_PT_ID, log_id, HRD_THIS_MODULE_ID, level, format, para1, para2, para3, para4);  \
    }   \
    while (0)
#endif

/*****************************************************/
/* 不同等级日志记录宏，各模块可自己定制修改HRD_NAME */
/* 例如: #undef HRD_NAME                            */
/*       #define HRD_NAME   "dsaf"                 */
/*****************************************************/
#if 0 /*TBD*/
#define hrd_log_dbg(log_id, format, para1, para2, para3, para4) \
    hrd_log(IWARE_LOG_LEVEL_DEBUGGING, HRD_DBG_ENABLE, log_id, "<" HRD_NAME "-DBG>" format, para1, para2, para3, para4)

/*lint -emacro(506, hrd_log_info, hrd_log_warn, hrd_log_err) */
#define hrd_log_info(log_id, format, para1, para2, para3, para4) \
    hrd_log(IWARE_LOG_LEVEL_INFORMATIONAL, 1, log_id, "<" HRD_NAME "-INFO>" format, para1, para2, para3, para4)

#define hrd_log_note(log_id, format, para1, para2, para3, para4) \
    hrd_log(IWARE_LOG_LEVEL_NOTIFICATION, 1, log_id,"<" HRD_NAME "-NOTE>" format, para1, para2, para3, para4)

#define hrd_log_warn(log_id, format, para1, para2, para3, para4) \
    hrd_log(IWARE_LOG_LEVEL_WARNING, 1, log_id, "<" HRD_NAME "-WARN>" format, para1, para2, para3, para4)

#define hrd_log_err(log_id, format, para1, para2, para3, para4) \
    hrd_log(IWARE_LOG_LEVEL_ERROR, 1, log_id,"<" HRD_NAME "-ERR>" format, para1, para2, para3, para4)

#define hrd_log_crit(log_id, format, para1, para2, para3, para4) \
    hrd_log(IWARE_LOG_LEVEL_CRITICAL,1, log_id,"<" HRD_NAME "-CRIT>" format, para1, para2, para3, para4)

#define hrd_log_alrt(log_id, format, para1, para2, para3, para4) \
    hrd_log(IWARE_LOG_LEVEL_ALERT, 1, log_id, "<" HRD_NAME "-ALRT>" format, para1, para2, para3, para4)

#define hrd_log_emrg(log_id, format, para1, para2, para3, para4) \
    hrd_log(IWARE_LOG_LEVEL_EMERGENCY, 1, log_id,"<" HRD_NAME "-EMRG>" format, para1, para2, para3, para4)
#endif

#define log_dbg(dev, fmt, arg...) dev_dbg(dev, fmt, ##arg)

#define log_info(dev, fmt, arg...) dev_info(dev, fmt, ##arg)

#define log_note(dev, fmt, arg...) dev_note(dev, fmt, ##arg)

#define log_warn(dev, fmt, arg...) dev_warn(dev, fmt, ##arg)

#define log_err(dev, fmt, arg...) dev_err(dev, fmt, ##arg)

#define log_crit(dev, fmt, arg...) dev_crit(dev, fmt, ##arg)

#define log_alrt(dev, fmt, arg...) dev_alrt(dev, fmt, ##arg)

#define log_emrg(dev, fmt, arg...) dev_emrg(dev, fmt, ##arg)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


#endif /* _HRD_LOG_H */




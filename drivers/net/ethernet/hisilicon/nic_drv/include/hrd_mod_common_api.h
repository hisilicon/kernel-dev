/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : hrd_mod_common_api.h
  版 本 号   : 初稿
  作    者   : z00200342
  生成日期   : 2013年4月11日
  最近修改   :
  功能描述   : 公共定义
  函数列表   :
  修改历史   :
  1.日    期   : 2013年4月11日
    作    者   : z00200342
    修改内容   : 创建文件
  2.日    期   : 2013年11月22日
    作    者   : l00176160
    修改内容   : DTS2013112208956
******************************************************************************/

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


#ifndef _MCSS_MOD_COMMON_API_H_
#define _MCSS_MOD_COMMON_API_H_


#define MCSS_DEVS_NUM 3              /**< 默认3个设备     */
#define MCSS_MAX_INSTANCE_NUM   3    /**< 默认3个 进程    */
#define MCSS_NAME "mcss"              /**< mcss设备名字     */
#define MC_MOD_VERSION "iware V100R005C00B106"      /**< MCSS KO版本号   */
#define MC_CACHE_LINE_SIZE 64UL



/* begin add by z00200342 2013/4/10: 标准SRE数据结构&接口定义 */
#define MCSS_EACH_MODULE_MAX_CMD_NUM  100   /* 每个模块最大可定义命令字数 */
typedef enum /* 定义各模块命令字的范围，此定义供SRE标准接口使用 */
{
    MCSS_CMD_BMU_START = 100,                          /**<  BMU模块命令字起始范围   */
    MCSS_CMD_BMU_END   = MCSS_CMD_BMU_START + 100,     /**<  BMU模块命令字终止范围   */
    MCSS_CMD_POE_START,                                /**<  POE模块命令字起始范围 201  */
    MCSS_CMD_POE_END   = MCSS_CMD_POE_START + 100,     /**<  POE模块命令字终止范围   */
    MCSS_CMD_PPE_START,                                /**<  PPE模块命令字起始范围 302  */
    MCSS_CMD_PPE_END   = MCSS_CMD_PPE_START + 100,     /**<  PPE模块命令字终止范围   */
    MCSS_CMD_TM_START,                                 /**<  TM模块命令字起始范围 403  */
    MCSS_CMD_TM_END   = MCSS_CMD_TM_START + 100,       /**<  TM模块命令字终止范围   */
    MCSS_CMD_ETH_START,                                /**<  ETH模块命令字起始范围 504  */
    MCSS_CMD_ETH_END   = MCSS_CMD_ETH_START + 100,     /**<  ETH模块命令字终止范围   */
    MCSS_CMD_SRIO_START,                               /**<  SRIO模块命令字起始范围 605  */
    MCSS_CMD_SRIO_END   = MCSS_CMD_SRIO_START + 100,   /**<  SRIO模块命令字终止范围   */
    MCSS_CMD_SEC_START,                                /**<  SEC模块命令字起始范围 706  */
    MCSS_CMD_SEC_END   = MCSS_CMD_SEC_START + 100,     /**<  SEC模块命令字终止范围   */
    MCSS_CMD_FABRIC_START,                                /**<  FABRIC模块命令字起始范围 807  */
    MCSS_CMD_FABRIC_END   = MCSS_CMD_FABRIC_START + 100,     /**<  FABRIC模块命令字终止范围   */
    MCSS_CMD_SERDES_START,                                /**<  SERDES模块命令字起始范围 908  */
    MCSS_CMD_SERDES_END   = MCSS_CMD_SERDES_START + 100,      /**<  SERDES模块命令字终止范围   */
    MCSS_CMD_BPSS_START,                                       /**<  BPSS模块命令字起始范围 1009    */
    MCSS_CMD_BPSS_END   = MCSS_CMD_BPSS_START + 100,           /**<  BPSS模块命令字终止范围   */
    MCSS_CMD_DMA_START,                                        /**<  DMA模块命令字起始范围  1110  */
    MCSS_CMD_DMA_END   = MCSS_CMD_DMA_START + 100              /**<  DMA模块命令字终止范围   */
} MCSS_CMD_MODULE_DEF_ENUM;

/* end add by z00200342 2013/4/10: 标准SRE数据结构&接口定义 */

/* P650单板类型 */
#define  MC_BOARD_TYPE_UBBP_Y  0      /**< 业务验证板  */
#define  MC_BOARD_TYPE_UBBP_T  1      /**< t 板   */
#define  MC_BOARD_TYPE_UBBP_V  2      /**< v 板   */

#define  MC_BOARD_TYPE MC_BOARD_TYPE_UBBP_T   /**< T板不初始化PHY/MDIO  */

#endif  /* _MCSS_MOD_COMMON_API_H_ */





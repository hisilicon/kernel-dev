#ifndef _IWARE_SERDES_KERNEL_API_H
#define _IWARE_SERDES_KERNEL_API_H

#include <linux/module.h>
#include <linux/types.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*ctle 均衡*/
struct ctle_data
{
	u8 ctlebst[3];
	u8 ctlecmband[3];
	u8 ctlermband[3];
	u8 ctleza[3];
	u8 ctlesqh[3];
	u8 ctleactgn[3];
	u8 ctlepassgn;
};

/*dfe 均衡*/
struct dfe_data
{
	u8 dfepwrdb;      //FixTAP下电
    u8 dfemvtappwrdb; //Float Tap下电
    u8 dfefxtap[5];   //DFE Fix Tap 增益
    u8 dfemvtap[6];   //DFE Floating Taps 幅值
};

/*ffe 预加重*/
struct ffe_data
{
	u8 pre1;
	u8 main;
	u8 post1;
};

/*****************************************************************************
 函 数 名  : serdes_ctle_get
 功能描述  : ctle 均衡参数获取 
 输入参数  : u8 macro,u8 lane
 输出参数  : struct ctle_data *p_ctle_data
 返 回 值  : 0为成功，非0为错误
*****************************************************************************/
extern int serdes_ctle_get(u8 macro,u8 lane,struct ctle_data *p_ctle_data);
/*****************************************************************************
 函 数 名  : serdes_dfe_get
 功能描述  : dfe 均衡参数获取 
 输入参数  : u8 macro,u8 lane
 输出参数  : struct dfe_data *p_dfe_data
 返 回 值  : 0为成功，非0为错误
*****************************************************************************/
extern int serdes_dfe_get(u8 macro,u8 lane,struct dfe_data *p_dfe_data);
/*****************************************************************************
 函 数 名  : serdes_ffe_set
 功能描述  : ffe 预加重参数设置 
 输入参数  : u8 macro,u8 lane,const struct ffe_data *p_ffe_data
 输出参数  : 
 返 回 值  : 0为成功，非0为错误
*****************************************************************************/
extern int serdes_ffe_set(u8 macro,u8 lane,const struct ffe_data *p_ffe_data);
/*****************************************************************************
 函 数 名  : serdes_ffe_get
 功能描述  : ffe 预加重参数获取
 
 输入参数  : u8 macro,u8 lane,const struct ffe_data *p_ffe_data
 输出参数  : 
 返 回 值  : 0为成功，非0为错误
*****************************************************************************/
extern int serdes_ffe_get(u8 macro,u8 lane,struct ffe_data *p_ffe_data);

/*****************************************************************************
 函 数 名  : serdes_swing_set
 功能描述  : ffe 摆幅参数设置
 
 输入参数  : u8 macro,u8 lane,u8 swing
 输出参数  : 
 返 回 值  : 0为成功，非0为错误
*****************************************************************************/
extern int serdes_swing_set(u8 macro,u8 lane,const u8 swing);
/*****************************************************************************
 函 数 名  : serdes_swing_set
 功能描述  : ffe 摆幅参数获取
 
 输入参数  : u8 macro,u8 lane
 输出参数  : u8 *p_swing
 返 回 值  : 0为成功，非0为错误
*****************************************************************************/
extern int serdes_swing_get(u8 macro,u8 lane,u8 *p_swing);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif


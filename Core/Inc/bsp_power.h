/**
 * @file    bsp_power.h
 * @brief   传感器电源控制接口
 *
 * 功能说明：
 * - MQ-4气体传感器：通过PB0(GAS_PWR_CTRL)控制PMOS高侧开关，控制5V-SW-GAS供电
 * - JSN-SR04T超声波雷达：通过PB1(RADAR_PWR_CTRL)控制PMOS高侧开关，控制5V-SW-RADAR供电
 *
 */
 
#ifndef BSP_POWER_H
#define BSP_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"


void BSP_Power_GasOn(void);
void BSP_Power_GasOff(void);

void BSP_Power_RadarOn(void);
void BSP_Power_RadarOff(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_POWER_H */


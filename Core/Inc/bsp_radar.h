/**
 * @file    bsp_radar.h
 * @brief   JSN-SR04T 超声波测距模块 BSP 接口
 */

#ifndef BSP_RADAR_H
#define BSP_RADAR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

// 错误码定义
#define RADAR_ERR_NONE           0x0000  // 成功
#define RADAR_ERR_TIMEOUT_ECHO   0x0001  // ECHO上升沿超时
#define RADAR_ERR_TIMEOUT_PULSE  0x0002  // ECHO高电平超时
#define RADAR_ERR_OUT_OF_RANGE   0x0003  // 距离超出有效范围
#define RADAR_ERR_INVALID        0xFFFF  // 测量失败（通用）

// 参数定义
#define RADAR_TRIG_PULSE_US      10u     // TRIG脉冲宽度（微秒）
#define RADAR_ECHO_TIMEOUT_MS    60u    // ECHO超时时间（毫秒）
#define RADAR_ECHO_START_TIMEOUT_MS 10u // ECHO上升沿等待超时（毫秒）
#define RADAR_SAMPLE_COUNT       5u      // 采样次数
#define RADAR_SAMPLE_INTERVAL_MS 100u    // 采样间隔（毫秒，JSN-SR04T建议≥60ms，使用100ms更稳定）
#define RADAR_MIN_DISTANCE_CM    20u     // 最小有效距离（cm）
#define RADAR_MAX_DISTANCE_CM    600u    // 最大有效距离（cm）
#define RADAR_POWER_STABLE_MS    30u     // 上电稳定时间（毫秒，预留）

/**
 * @brief  初始化雷达模块
 */
void BSP_Radar_Init(void);

/**
 * @brief  开启雷达模块电源
 */
void BSP_Radar_PowerOn(void);

/**
 * @brief  关闭雷达模块电源
 */
void BSP_Radar_PowerOff(void);

/**
 * @brief  执行单次测距
 * @retval 距离值（cm），如果失败返回 RADAR_ERR_INVALID (0xFFFF)
 */
uint16_t BSP_Radar_SingleMeasure(void);

/**
 * @brief  执行滤波测距（5次采样+中值+均值滤波）
 * @retval 距离值（cm），如果失败返回 RADAR_ERR_INVALID (0xFFFF)
 */
uint16_t BSP_Radar_Measure(void);

/**
 * @brief  检查距离值是否在有效范围内
 * @param  distance_cm: 距离值（cm）
 * @retval 1=有效，0=无效
 */
uint8_t BSP_Radar_IsDistanceValid(uint16_t distance_cm);

/**
 * @brief  雷达ECHO引脚EXTI回调（由HAL_GPIO_EXTI_Callback转发）
 * @param  GPIO_Pin: 触发中断的GPIO Pin
 */
void BSP_Radar_EXTI_Callback(uint16_t GPIO_Pin);

#ifdef __cplusplus
}
#endif

#endif /* BSP_RADAR_H */

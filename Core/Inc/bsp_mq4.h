/**
 * @file    bsp_mq4.h
 * @brief   MQ-4（MQ4）气体传感器采样与“默认浓度估算”输出接口。
 */

#ifndef BSP_MQ4_H
#define BSP_MQ4_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// 默认：估算ppm上限
#ifndef BSP_MQ4_PPM_MAX
#define BSP_MQ4_PPM_MAX (10000u)
#endif

// 默认采样参数
#ifndef BSP_MQ4_SAMPLE_COUNT
#define BSP_MQ4_SAMPLE_COUNT (5u)
#endif

#ifndef BSP_MQ4_SAMPLE_INTERVAL_MS
#define BSP_MQ4_SAMPLE_INTERVAL_MS (100u)
#endif

// MQ4 预热时间（毫秒），默认 30s 方便调试改为5s
#ifndef BSP_MQ4_PREHEAT_MS
#define BSP_MQ4_PREHEAT_MS (1000u)
#endif

void BSP_MQ4_Init(void);
void BSP_MQ4_PreheatOnce(void);

uint16_t BSP_MQ4_ReadAdcOnce(void);
uint16_t BSP_MQ4_ReadAdcAverage(uint8_t n, uint32_t interval_ms);

// ADC -> 传感器 AO 节点电压
uint16_t BSP_MQ4_AdcToAo_mV(uint16_t adc_raw);

// AO 节点电压 -> 传感器输出端电压
uint16_t BSP_MQ4_AoToSensor_mV(uint16_t ao_mv);

// ADC -> 传感器输出端电压（mV）
uint16_t BSP_MQ4_AdcToSensor_mV(uint16_t adc_raw);

// 基准校准相关函数
void BSP_MQ4_CalibrateBaseline(uint16_t sensor_mv);  // 校准基准值（以当前环境作为0ppm）
uint16_t BSP_MQ4_GetBaseline(void);                  // 获取当前基准值
uint8_t BSP_MQ4_IsCalibrated(void);                  // 检查是否已校准

// 传感器输出电压（mV） -> 估算ppm（相对于基准值）
uint16_t BSP_MQ4_Sensor_mV_ToPpmEst(uint16_t sensor_mv);

#ifdef __cplusplus
}
#endif

#endif /* BSP_MQ4_H */


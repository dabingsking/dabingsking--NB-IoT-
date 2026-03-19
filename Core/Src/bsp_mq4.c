/**
 * @file    bsp_mq4.c
 * @brief   MQ-4（MQ4）气体传感器采样与“默认浓度估算”实现。
 */

#include "bsp_mq4.h"
#include "main.h"
#include "bsp_power.h"

extern ADC_HandleTypeDef hadc1;

// MQ4基准校准值（室内正常环境的传感器电压，作为0ppm基准）
static uint16_t s_mq4_baseline_mv = 0;  // 0表示未校准
static uint8_t s_mq4_calibrated = 0;

void BSP_MQ4_Init(void)
{
  // 初始化时基准值未校准
  s_mq4_baseline_mv = 0;
  s_mq4_calibrated = 0;
}

void BSP_MQ4_PreheatOnce(void)
{
	
  static uint8_t preheated = 0;
  if (preheated) return;

  BSP_Power_GasOn();

  // 预热时间按默认 30s
  HAL_Delay(BSP_MQ4_PREHEAT_MS);
  preheated = 1;
}

uint16_t BSP_MQ4_ReadAdcOnce(void)
{
  uint16_t adc = 0;

  (void)HAL_ADC_Stop(&hadc1);
  if (HAL_ADC_Start(&hadc1) != HAL_OK) {
    return 0;
  }

  if (HAL_ADC_PollForConversion(&hadc1, 50) == HAL_OK) {
    adc = (uint16_t)HAL_ADC_GetValue(&hadc1);
  }

  (void)HAL_ADC_Stop(&hadc1);
  return adc;
}

uint16_t BSP_MQ4_ReadAdcAverage(uint8_t n, uint32_t interval_ms)
{
  if (n == 0) return 0;

  uint32_t sum = 0;
  for (uint8_t i = 0; i < n; i++) {
    sum += (uint32_t)BSP_MQ4_ReadAdcOnce();
    if (interval_ms != 0 && i + 1 < n) {
      HAL_Delay(interval_ms);
    }
  }
  return (uint16_t)(sum / n);
}

uint16_t BSP_MQ4_AdcToAo_mV(uint16_t adc_raw)
{
  // AO 节点电压 = adc/4095 * 3300
  return (uint16_t)(((uint32_t)adc_raw * 3300u) / 4095u);
}

uint16_t BSP_MQ4_AoToSensor_mV(uint16_t ao_mv)
{
  // 分压比 2/3，还原到传感器输出端：×1.5 = ×3/2
  return (uint16_t)(((uint32_t)ao_mv * 3u) / 2u);
}

uint16_t BSP_MQ4_AdcToSensor_mV(uint16_t adc_raw)
{
  return BSP_MQ4_AoToSensor_mV(BSP_MQ4_AdcToAo_mV(adc_raw));
}

/**
 * @brief  校准MQ4基准值
 * @param  sensor_mv: 当前传感器电压（mV）
 */
void BSP_MQ4_CalibrateBaseline(uint16_t sensor_mv)
{
  s_mq4_baseline_mv = sensor_mv;
  s_mq4_calibrated = 1;
}

/**
 * @brief  获取当前基准值
 * @retval 基准电压（mV），0表示未校准
 */
uint16_t BSP_MQ4_GetBaseline(void)
{
  return s_mq4_baseline_mv;
}

/**
 * @brief  检查是否已校准
 * @retval 1=已校准, 0=未校准
 */
uint8_t BSP_MQ4_IsCalibrated(void)
{
  return s_mq4_calibrated;
}

/**
 * @brief  传感器电压转换为相对ppm（相对于基准值）
 * @param  sensor_mv: 当前传感器电压（mV）
 * @retval 相对ppm值（相对于基准的增量）
 * @note   如果未校准，返回绝对ppm值（旧逻辑）
 *          如果已校准，返回相对于基准的ppm增量
 */
uint16_t BSP_MQ4_Sensor_mV_ToPpmEst(uint16_t sensor_mv)
{
  // 如果未校准，使用旧的绝对线性映射
  if (s_mq4_calibrated == 0 || s_mq4_baseline_mv == 0) {
    const uint32_t SENSOR_MV_MAX = 4950u;
    if (sensor_mv >= SENSOR_MV_MAX) return (uint16_t)BSP_MQ4_PPM_MAX;
    uint32_t ppm = ((uint32_t)sensor_mv * (uint32_t)BSP_MQ4_PPM_MAX) / SENSOR_MV_MAX;
    if (ppm > BSP_MQ4_PPM_MAX) ppm = BSP_MQ4_PPM_MAX;
    return (uint16_t)ppm;
  }
  
  // 已校准：计算相对于基准的增量
  // 如果当前值小于基准值，返回0（不应该出现，但保护性处理）
  if (sensor_mv < s_mq4_baseline_mv) {
    return 0;
  }
  
  // 计算电压增量（mV）
  uint32_t delta_mv = sensor_mv - s_mq4_baseline_mv;
  
  // 将电压增量转换为ppm
  uint32_t ppm = (delta_mv * 5u);
  
  // 限制最大值
  if (ppm > BSP_MQ4_PPM_MAX) {
    ppm = BSP_MQ4_PPM_MAX;
  }
  
  return (uint16_t)ppm;
}


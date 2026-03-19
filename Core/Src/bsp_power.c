/**
 * @file    bsp_power.c
 * @brief   传感器电源控制接口实现（5V供电完整实现）。
 *
 * 硬件说明：
 * - MQ-4气体传感器：通过PB0(GAS_PWR_CTRL)控制PMOS高侧开关，控制5V-SW-GAS供电
 * - JSN-SR04T超声波雷达：通过PB1(RADAR_PWR_CTRL)控制PMOS高侧开关，控制5V-SW-RADAR供电
 *
 * - GPIO高电平 → 光耦导通 → PMOS Gate拉低 → PMOS导通 → 传感器上电
 * - GPIO低电平 → 光耦关断 → PMOS Gate上拉 → PMOS截止 → 传感器断电
 *
 */

#include "bsp_power.h"

/**
 * @brief  开启MQ-4气体传感器电源
 * @note   控制PB0(GAS_PWR_CTRL)输出高电平，使PMOS导通，5V-SW-GAS上电
 * @note   高电平=上电，低电平=断电
 */
void BSP_Power_GasOn(void)
{
  // GPIO高电平 → PMOS导通 → 传感器上电
  HAL_GPIO_WritePin(GAS_PWR_CTRL_GPIO_Port, GAS_PWR_CTRL_Pin, GPIO_PIN_SET);
  
  // 等待电源稳定
  // HAL_Delay(1);
}

/**
 * @brief  关闭MQ-4气体传感器电源
 * @note   控制PB0(GAS_PWR_CTRL)输出低电平，使PMOS截止，5V-SW-GAS断电
 * @note   高电平=上电，低电平=断电
 */
void BSP_Power_GasOff(void)
{
  HAL_GPIO_WritePin(GAS_PWR_CTRL_GPIO_Port, GAS_PWR_CTRL_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  开启JSN-SR04T超声波雷达电源
 * @note   控制PB1(RADAR_PWR_CTRL)输出高电平，使PMOS导通，5V-SW-RADAR上电
 * @note   高电平=上电，低电平=断电
 */
void BSP_Power_RadarOn(void)
{
  // GPIO高电平 → PMOS导通 → 传感器上电
  HAL_GPIO_WritePin(RADAR_PWR_CTRL_GPIO_Port, RADAR_PWR_CTRL_Pin, GPIO_PIN_SET);
  
}

/**
 * @brief  关闭JSN-SR04T超声波雷达电源
 * @note   控制PB1(RADAR_PWR_CTRL)输出低电平，使PMOS截止，5V-SW-RADAR断电
 * @note   高电平=上电，低电平=断电
 */
void BSP_Power_RadarOff(void)
{
  // GPIO低电平 → PMOS截止 → 传感器断电
  HAL_GPIO_WritePin(RADAR_PWR_CTRL_GPIO_Port, RADAR_PWR_CTRL_Pin, GPIO_PIN_RESET);
  
}

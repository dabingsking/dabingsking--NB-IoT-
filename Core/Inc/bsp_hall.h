/**
 * @file    bsp_hall.h
 * @brief   霍尔传感器（井盖开合检测）
 *
 * 硬件约定：
 * - 传感器：A3144E 单极霍尔开关
 * - 引脚：PC5 (HALL_DO)，外接上拉到 3.3V
 * - 语义：高电平 = 井盖闭合（正常），低电平 = 井盖打开（触发）
 */

#ifndef BSP_HALL_H
#define BSP_HALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/** 井盖状态：1 = 闭合（正常），0 = 打开（触发） */
#define HALL_STATE_CLOSED  1u
#define HALL_STATE_OPEN    0u

/**
 * @brief 霍尔传感器初始化入口。
 */
void BSP_Hall_Init(void);

/**
 * @brief 读取霍尔传感器原始状态（无去抖）。
 * @retval HALL_STATE_CLOSED(1) 井盖闭合
 * @retval HALL_STATE_OPEN(0)   井盖打开
 */
uint8_t BSP_Hall_ReadState(void);

/**
 * @brief 读取霍尔传感器带去抖的稳定状态。
 * @retval HALL_STATE_CLOSED(1) 井盖闭合
 * @retval HALL_STATE_OPEN(0)   井盖打开
 */
uint8_t BSP_Hall_ReadStateDebounced(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_HALL_H */


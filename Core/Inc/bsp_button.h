/**
 * @file    bsp_button.h
 * @brief   USER 按键驱动接口（PC13，上拉输入，按下=低电平）。
 *
 * 功能说明：
 * - 提供原始按键状态读取（未去抖）。
 * - 在 `BSP_Button_Poll()` 中实现去抖与短按事件识别，适合主循环轮询使用。
 */

#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#include "main.h"
#include <stdint.h>

void BSP_Button_Init(void);
void BSP_Button_Poll(void);

// 立即读取（未去抖）：1=按下，0=松开
uint8_t BSP_Button_IsPressedRaw(void);

// 去抖后的稳定状态：1=按下，0=松开
uint8_t BSP_Button_IsPressedStable(void);

// 读取一次“短按”事件（<2s），读出即清除：1=有事件，0=无
uint8_t BSP_Button_GetShortPressEvent(void);

#endif /* BSP_BUTTON_H */


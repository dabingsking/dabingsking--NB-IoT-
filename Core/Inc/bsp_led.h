/**
 * @file    bsp_led.h
 * @brief   RGB LED (共阳极) 驱动接口：PC0/PC1/PC2 低电平点亮。
 *
 * 提供的功能：
 * - 初始化 LED GPIO（推挽输出，默认熄灭）。
 * - 控制单色/三色 LED 的亮灭，用于状态指示与调试。
 */

#ifndef BSP_LED_H
#define BSP_LED_H

#include "main.h"
#include <stdint.h>

void BSP_LED_Init(void);
void BSP_LED_AllOff(void);

void BSP_LED_SetRed(uint8_t on);
void BSP_LED_SetGreen(uint8_t on);
void BSP_LED_SetBlue(uint8_t on);

void BSP_LED_SetRGB(uint8_t r_on, uint8_t g_on, uint8_t b_on);

#endif /* BSP_LED_H */


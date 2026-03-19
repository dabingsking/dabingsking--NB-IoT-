/**
 * @file    bsp_led.c
 * @brief   RGB LED 驱动（PC0/PC1/PC2，共阳极，低电平点亮）。
 *
 * - 提供简单、直观的 LED 控制接口，用于状态指示与调试。
 */

#include "bsp_led.h"

static void led_write(GPIO_TypeDef *port, uint16_t pin, uint8_t on)
{
  // 共阳极：GPIO=0 点亮，GPIO=1 熄灭
  HAL_GPIO_WritePin(port, pin, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void BSP_LED_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOC_CLK_ENABLE();

  // 先默认熄灭
  HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = LED_R_Pin | LED_G_Pin | LED_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void BSP_LED_AllOff(void)
{
  BSP_LED_SetRGB(0, 0, 0);
}

void BSP_LED_SetRed(uint8_t on)
{
  led_write(LED_R_GPIO_Port, LED_R_Pin, on);
}

void BSP_LED_SetGreen(uint8_t on)
{
  led_write(LED_G_GPIO_Port, LED_G_Pin, on);
}

void BSP_LED_SetBlue(uint8_t on)
{
  led_write(LED_B_GPIO_Port, LED_B_Pin, on);
}

void BSP_LED_SetRGB(uint8_t r_on, uint8_t g_on, uint8_t b_on)
{
  BSP_LED_SetRed(r_on);
  BSP_LED_SetGreen(g_on);
  BSP_LED_SetBlue(b_on);
}


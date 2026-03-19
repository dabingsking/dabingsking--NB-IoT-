/**
 * @file    bsp_button.c
 * @brief   用户按键驱动（PC13，上拉输入，按下=低电平）。
 *
 * 功能概述：
 * - 提供原始按键读取、去抖后的稳定状态读取。
 * - 在轮询函数 `BSP_Button_Poll()` 中实现去抖与“短按事件”检测。
 * - 仅负责本地状态机，不直接耦合具体业务逻辑。
 */

#include "bsp_button.h"

#define BTN_DEBOUNCE_MS      (30u)
#define BTN_SHORT_MAX_MS     (2000u)

static uint8_t s_last_raw = 0;
static uint8_t s_stable = 0;
static uint32_t s_last_change_tick = 0;
static uint32_t s_press_start_tick = 0;
static uint8_t s_short_event = 0;

uint8_t BSP_Button_IsPressedRaw(void)
{
  return (HAL_GPIO_ReadPin(USER_BTN_GPIO_Port, USER_BTN_Pin) == GPIO_PIN_RESET) ? 1u : 0u;
}

uint8_t BSP_Button_IsPressedStable(void)
{
  return s_stable;
}

void BSP_Button_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitStruct.Pin = USER_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(USER_BTN_GPIO_Port, &GPIO_InitStruct);

  // 初始化去抖状态
  s_last_raw = BSP_Button_IsPressedRaw();
  s_stable = s_last_raw;
  s_last_change_tick = HAL_GetTick();
  s_press_start_tick = 0;
  s_short_event = 0;
}

void BSP_Button_Poll(void)
{
  uint32_t now = HAL_GetTick();
  uint8_t raw = BSP_Button_IsPressedRaw();

  if (raw != s_last_raw) {
    s_last_raw = raw;
    s_last_change_tick = now;
    return;
  }

  if ((now - s_last_change_tick) < BTN_DEBOUNCE_MS) {
    return;
  }

  if (raw == s_stable) {
    return;
  }

  // 稳定状态发生变化
  uint8_t prev = s_stable;
  s_stable = raw;

  if (prev == 0u && s_stable == 1u) {
    // 按下
    s_press_start_tick = now;
  } else if (prev == 1u && s_stable == 0u) {
    // 松开：判断是否短按
    uint32_t dur = (s_press_start_tick == 0u) ? 0u : (now - s_press_start_tick);
    if (dur > 0u && dur < BTN_SHORT_MAX_MS) {
      s_short_event = 1u;
    }
    s_press_start_tick = 0u;
  }
}

uint8_t BSP_Button_GetShortPressEvent(void)
{
  uint8_t e = s_short_event;
  s_short_event = 0u;
  return e;
}


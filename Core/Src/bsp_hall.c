/**
 * @file    bsp_hall.c
 * @brief   霍尔传感器（井盖开合检测）
 */

#include "bsp_hall.h"
#include "bsp_debug.h"

void BSP_Hall_Init(void)
{
}

uint8_t BSP_Hall_ReadState(void)
{
  GPIO_PinState state = HAL_GPIO_ReadPin(HALL_DO_GPIO_Port, HALL_DO_Pin);

  /* 高电平 = 闭合（正常），低电平 = 打开（触发） */
  return (state == GPIO_PIN_SET) ? HALL_STATE_CLOSED : HALL_STATE_OPEN;
}

uint8_t BSP_Hall_ReadStateDebounced(void)
{
  uint8_t count_closed = 0u;

  /* 连续采样 5 次，每次间隔 10ms */
  for (int i = 0; i < 5; i++) {
    if (BSP_Hall_ReadState() == HALL_STATE_CLOSED) {
      count_closed++;
    }

    if (i < 4) {
      HAL_Delay(10);
    }
  }

  uint8_t stable_state =
      (count_closed >= 3u) ? HALL_STATE_CLOSED : HALL_STATE_OPEN;

  /* 打印当前稳定状态 */
  BSP_Debug_Printf("[HALL] debounced state=%u (%s)\r\n",
                   (unsigned)stable_state,
                   (stable_state == HALL_STATE_CLOSED) ? "CLOSED" : "OPEN");

  return stable_state;
}


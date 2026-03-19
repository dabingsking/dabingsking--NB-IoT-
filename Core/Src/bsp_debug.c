/**
 * @file    bsp_debug.c
 * @brief   USART1 调试输出封装模块。
 *
 * - 统一调试日志出口（基于 USART1），避免在业务代码中直接操作 UART 句柄。
 * - 提供字符串发送、字节流发送、printf 风格格式化输出等常用接口。
 * - 可选支持 `printf` 重定向到 USART1（由 `BSP_DEBUG_ENABLE_PRINTF_RETARGET` 控制）。
 */

#include "bsp_debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart1;

void BSP_Debug_Init(void)
{
}

void BSP_Debug_SendString(const char *str)
{
  if (str == NULL) return;
  uint16_t len = (uint16_t)strlen(str);
  if (len == 0) return;
  (void)HAL_UART_Transmit(&huart1, (uint8_t *)str, len, HAL_MAX_DELAY);
}

void BSP_Debug_SendBytes(const uint8_t *data, uint16_t len)
{
  if (data == NULL || len == 0) return;
  (void)HAL_UART_Transmit(&huart1, (uint8_t *)data, len, HAL_MAX_DELAY);
}

void BSP_Debug_Printf(const char *fmt, ...)
{
  if (fmt == NULL) return;

  char buffer[256];
  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  if (n <= 0) return;
  if (n >= (int)sizeof(buffer)) n = (int)sizeof(buffer) - 1;
  (void)HAL_UART_Transmit(&huart1, (uint8_t *)buffer, (uint16_t)n, HAL_MAX_DELAY);
}

#if BSP_DEBUG_ENABLE_PRINTF_RETARGET
int __io_putchar(int ch)
{
  uint8_t c = (uint8_t)ch;
  (void)HAL_UART_Transmit(&huart1, &c, 1, HAL_MAX_DELAY);
  return ch;
}
#endif


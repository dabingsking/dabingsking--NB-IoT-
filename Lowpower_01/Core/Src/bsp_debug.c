#include "bsp_debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// 使用 main.c 中的 USART1 句柄作为调试串口
extern UART_HandleTypeDef huart1;

void BSP_Debug_Init(void)
{
  /* 目前无需额外初始化，仅占位：
   * - USART1 的时钟 / GPIO / 波特率 已由 MX_USART1_UART_Init() 完成
   * - 如需在将来增加接收中断或环形缓冲，可以在此处扩展
   */
}

void BSP_Debug_SendString(const char *str)
{
  // 空指针或空字符串直接返回，避免访问非法地址
  if (str == NULL) return;
  uint16_t len = (uint16_t)strlen(str);
  if (len == 0) return;
  (void)HAL_UART_Transmit(&huart1, (uint8_t *)str, len, HAL_MAX_DELAY);
}

void BSP_Debug_SendBytes(const uint8_t *data, uint16_t len)
{
  // data 为空或长度为 0 时不做任何发送
  if (data == NULL || len == 0) return;
  (void)HAL_UART_Transmit(&huart1, (uint8_t *)data, len, HAL_MAX_DELAY);
}

void BSP_Debug_Printf(const char *fmt, ...)
{
  // 防御式编程：格式字符串为空时直接返回
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
  // printf 重定向：逐字节输出到 USART1
  uint8_t c = (uint8_t)ch;
  (void)HAL_UART_Transmit(&huart1, &c, 1, HAL_MAX_DELAY);
  return ch;
}
#endif


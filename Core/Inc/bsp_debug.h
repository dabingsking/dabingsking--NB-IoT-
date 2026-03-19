/**
 * @file    bsp_debug.h
 * @brief   USART1 调试输出封装接口
 */

#ifndef BSP_DEBUG_H
#define BSP_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief 是否启用 printf 重定向到 USART1（通过 __io_putchar）
 */
#ifndef BSP_DEBUG_ENABLE_PRINTF_RETARGET
#define BSP_DEBUG_ENABLE_PRINTF_RETARGET 1
#endif

/**
 * @brief 调试模块初始化
 * @note  USART1 的硬件初始化在 MX_USART1_UART_Init() 中完成
 */
void BSP_Debug_Init(void);

/**
 * @brief 发送以 '\0' 结尾的字符串
 * @param  str: C 字符串指针（不包含结尾换行）
 */
void BSP_Debug_SendString(const char *str);

/**
 * @brief 发送原始字节流
 * @param  data: 数据缓冲区
 * @param  len : 字节数
 */
void BSP_Debug_SendBytes(const uint8_t *data, uint16_t len);

/**
 * @brief printf 风格的格式化输出到调试串口
 * @param  fmt: printf 风格格式字符串
 */
void BSP_Debug_Printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DEBUG_H */


#ifndef BSP_DEBUG_H
#define BSP_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief 调试串口(BSP_Debug)模块
 *
 * 设计目的：
 * - 统一通过 USART1 输出调试日志、十六进制数据等
 * - 封装 HAL_UART_Transmit，避免在业务代码中到处直接操作 UART 句柄
 *
 * 说明：
 * - 默认使用 main.c 中的全局句柄 huart1（115200, 8N1）
 * - 可通过 BSP_Debug_SendString / BSP_Debug_Printf 直接输出调试信息
 */

/**
 * @brief 是否启用 printf 重定向到 USART1（通过 __io_putchar）
 *
 * 设为 1：
 *   - 需要在 syscalls.c 中使用 newlib 的 printf
 *   - 代码体积会略有增加（引入格式化库）
 * 设为 0：
 *   - 不启用重定向，推荐直接使用 BSP_Debug_Printf()
 */
#ifndef BSP_DEBUG_ENABLE_PRINTF_RETARGET
#define BSP_DEBUG_ENABLE_PRINTF_RETARGET 1
#endif

/**
 * @brief 调试模块初始化（目前主要用于占位，保持接口统一）
 * @note  USART1 的硬件初始化在 MX_USART1_UART_Init() 中完成
 */
void BSP_Debug_Init(void);

/**
 * @brief 发送以 '\0' 结尾的字符串
 * @param  str: C 字符串指针（不包含结尾换行）
 * @note  内部使用阻塞式 HAL_UART_Transmit，开发阶段使用即可
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
 * @note  内部使用静态 256 字节缓冲区，长度超出会被截断
 */
void BSP_Debug_Printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* BSP_DEBUG_H */


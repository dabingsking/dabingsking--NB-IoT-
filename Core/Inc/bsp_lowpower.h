/**
 * @file    bsp_lowpower.h
 * @brief   低功耗管理模块接口（STOP2 进入/退出与唤醒标志管理）。
 *
 * 提供能力：
 * - 封装 HAL STOP2 进入/退出流程，统一处理 EXTI/RTC 唤醒配置与时钟恢复。
 * - 通过 RTC 备份寄存器在首次上电与 STOP2 唤醒之间进行区分。
 */

#ifndef BSP_LOWPOWER_H
#define BSP_LOWPOWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 唤醒原因枚举（由 LP_IdentifyWakeupReason 返回） */
typedef enum {
    WAKEUP_REASON_POWER_ON = 0,
    WAKEUP_REASON_RTC,
    WAKEUP_REASON_ACC,
    WAKEUP_REASON_HALL,
    WAKEUP_REASON_UNKNOWN,
} WakeupReason_t;

/**
 * @brief  配置 RTC WakeUp Timer
 * @param  seconds  唤醒周期（秒），最大 131072
 */
void LP_ConfigRtcWakeupSeconds(uint32_t seconds);

/**
 * @brief  STOP2 唤醒后识别唤醒原因
 * @retval WakeupReason_t
 * @note   必须在 LP_ExitStop2() 之后调用
 */
WakeupReason_t LP_IdentifyWakeupReason(void);

/**
 * @brief  STOP2 唤醒后外设重初始化回调（弱定义）
 * @note   用户在 main.c USER CODE 区域覆盖此函数，调用 MX_USART1_UART_Init 等
 */
void LP_PeriphReinit_Callback(void);

/**
 * @brief  进入STOP2低功耗模式
 * @retval None
 * @note   函数执行后会进入STOP2模式，程序在此处停止
 *         唤醒后会从函数返回，继续执行后续代码
 */
void LP_EnterStop2(void);

/**
 * @brief  退出STOP2模式后的恢复处理
 * @retval None
 * @note   STOP2唤醒后，系统时钟会恢复到MSI（4MHz），需要重新配置到80MHz
 *         同时需要重新初始化外设时钟
 */
void LP_ExitStop2(void);

/**
 * @brief  判断是否为STOP2唤醒（首次上电 vs STOP2唤醒）
 * @retval 1=STOP2唤醒, 0=首次上电
 * @note   使用RTC备份寄存器作为标志位
 */
uint8_t LP_IsStop2Wakeup(void);

/**
 * @brief  清除STOP2唤醒标志
 * @retval None
 */
void LP_ClearStop2WakeupFlag(void);

/**
 * @brief  设置STOP2唤醒标志
 * @retval None
 */
void LP_SetStop2WakeupFlag(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LOWPOWER_H */

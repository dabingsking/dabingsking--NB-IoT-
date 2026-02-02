#ifndef BSP_LOWPOWER_H
#define BSP_LOWPOWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief 低功耗管理模块
 * @note  提供STOP2模式进入/退出、唤醒源判断等功能
 */

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
 * @brief  设置STOP2唤醒标志（进入STOP2前调用）
 * @retval None
 */
void LP_SetStop2WakeupFlag(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LOWPOWER_H */

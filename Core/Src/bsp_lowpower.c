/**
 * @file    bsp_lowpower.c
 * @brief   低功耗管理模块：负责 STOP2 模式的进入/退出与唤醒标志管理。
 *
 * 功能概述：
 * - 提供 STOP2 进入接口 `LP_EnterStop2()`，封装 EXTI/RTC 唤醒配置与 DBGMCU 设置。
 * - 提供 STOP2 退出接口 `LP_ExitStop2()`，封装系统时钟恢复逻辑。
 * - 通过 RTC 备份寄存器记录“是否来自 STOP2 唤醒”，区分首次上电与唤醒流程。
 *
 */

#include "bsp_lowpower.h"
#include "main.h"
#include "stm32l4xx_hal_pwr_ex.h"  
#include "stm32l4xx_hal.h"  

extern void SystemClock_Config(void);

extern RTC_HandleTypeDef hrtc;

#define RTC_BKP_FLAG_STOP2_WAKEUP  RTC_BKP_DR0
#define STOP2_WAKEUP_FLAG_VALUE     (0x1234u)

/**
 * @brief  判断是否为STOP2唤醒
 * @retval 1=STOP2唤醒, 0=首次上电
 */
uint8_t LP_IsStop2Wakeup(void)
{
  HAL_PWR_EnableBkUpAccess();
  uint32_t flag = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_FLAG_STOP2_WAKEUP);
  HAL_PWR_DisableBkUpAccess();
  
  return (flag == STOP2_WAKEUP_FLAG_VALUE) ? 1 : 0;
}

/**
 * @brief  设置STOP2唤醒标志
 */
void LP_SetStop2WakeupFlag(void)
{
  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_FLAG_STOP2_WAKEUP, STOP2_WAKEUP_FLAG_VALUE);
  HAL_PWR_DisableBkUpAccess();
}

/**
 * @brief  清除STOP2唤醒标志
 */
void LP_ClearStop2WakeupFlag(void)
{
  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_FLAG_STOP2_WAKEUP, 0);
  HAL_PWR_DisableBkUpAccess();
}

/**
 * @brief  进入STOP2低功耗模式
 * @note   进入前准备：
 *         1. 设置STOP2唤醒标志（用于区分首次上电和唤醒）
 *         2. 配置EXTI为唤醒源
 *         3. 确保EXTI中断已使能（LIS3DH唤醒）
 *         4. 进入STOP2模式
 *         
 *         唤醒后：
 *         - STM32L4的STOP2唤醒不会导致系统复位！
 *         - 唤醒后会从HAL_PWREx_EnterSTOP2Mode()函数返回
 *         - 系统时钟恢复到MSI（4MHz）
 *         - 需要调用LP_ExitStop2()恢复时钟和外设
 *         - 程序继续执行，不会从main()重新开始
 */
void LP_EnterStop2(void)
{
  // 步骤1：设置STOP2唤醒标志（用于下次唤醒时判断）
  LP_SetStop2WakeupFlag();
  
  // 步骤2：清除所有唤醒标志
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  
  // 步骤3：确保EXTI中断已使能并配置正确
  // STM32L4的EXTI可以在STOP2模式下唤醒系统，但需要满足以下条件：
  // 1. EXTI中断必须使能（NVIC）
  // 2. EXTI线必须配置为中断模式（GPIO_MODE_IT_xxx）
  // 3. EXTI挂起标志必须清除（避免残留中断立即唤醒）
  
  // 清除EXTI挂起标志
  __HAL_GPIO_EXTI_CLEAR_IT(ACC_INT1_Pin);
  
  // 确保EXTI中断已使能，优先级为最高
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  
  // 步骤3.5：使能EXTI线作为STOP2唤醒源
  // STM32L4需要显式配置PWR_CR3寄存器的EWUP位来使能外部唤醒
  // 但是EXTI线（如PA8）不需要EWUP配置，只需要确保：
  // 1. EXTI中断模式已配置（GPIO_MODE_IT_xxx）
  // 2. EXTI中断已使能（NVIC）
  // 3. EXTI事件模式已使能（关键！）
  
  // 使能EXTI8的事件模式（不仅仅是中断模式）
  // STOP2唤醒需要EXTI事件，不仅仅是中断
  // 设置EXTI->EMR1寄存器的对应位
  EXTI->EMR1 |= (1 << 8);  
  
  // 步骤4：禁用调试模式
  // 在STOP2模式下，调试器会阻止系统进入低功耗，导致系统复位
  // 必须禁用调试模式才能正常进入STOP2
  HAL_DBGMCU_DisableDBGStopMode();  
  
  // 步骤5：进入STOP2模式
  HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
  
  // 步骤6：唤醒后执行（从STOP2返回后）
}

/**
 * @brief  退出STOP2模式后的恢复处理
 * @note   STOP2唤醒后的必要操作：
 *         1. 恢复系统时钟
 *         2. 重新初始化外设时钟（I2C、USART等）
 *         3. 清除STOP2唤醒标志
 */
void LP_ExitStop2(void)
{
    /* 步骤1：恢复系统时钟到 80MHz */
    SystemClock_Config();
    HAL_Delay(2);

    /* 步骤2：重新初始化外设（时钟恢复后才能正常工作） */
    extern void MX_USART1_UART_Init(void);
    extern void MX_USART2_UART_Init(void);
    extern void MX_I2C1_Init(void);
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    HAL_Delay(10);
    MX_I2C1_Init();
    HAL_Delay(10);

    /* 步骤3：清除 STOP2 唤醒标志 */
    LP_ClearStop2WakeupFlag();
}

/**
 * @brief  配置 RTC WakeUp Timer 周期
 * @param  seconds  唤醒周期（秒）
 * @note   使用 RTC_WAKEUPCLOCK_CK_SPRE_16BITS（1Hz），最大 131072 秒
 */
void LP_ConfigRtcWakeupSeconds(uint32_t seconds)
{
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
    HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, seconds - 1u,
                                RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
}

/**
 * @brief  识别 STOP2 唤醒原因
 * @retval WakeupReason_t
 */
WakeupReason_t LP_IdentifyWakeupReason(void)
{
    /* 优先检查 RTC WakeUp Timer 标志 */
    if (__HAL_RTC_WAKEUPTIMER_GET_FLAG(&hrtc, RTC_FLAG_WUTF) != 0) {
        __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
        __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();
        return WAKEUP_REASON_RTC;
    }

    /* 检查 LIS3DH INT1（PA8 = EXTI8） */
    if (__HAL_GPIO_EXTI_GET_IT(ACC_INT1_Pin) != 0) {
        __HAL_GPIO_EXTI_CLEAR_IT(ACC_INT1_Pin);
        return WAKEUP_REASON_ACC;
    }

    return WAKEUP_REASON_UNKNOWN;
}

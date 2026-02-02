#include "bsp_lowpower.h"
#include "main.h"
#include "stm32l4xx_hal_pwr_ex.h"  // 需要包含此头文件以使用HAL_PWREx_EnterSTOP2Mode
#include "stm32l4xx_hal.h"  // 用于DBGMCU配置

// 外部引用RTC句柄
extern RTC_HandleTypeDef hrtc;

// 备份寄存器标志位定义
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
 *         2. 确保EXTI中断已使能（LIS3DH唤醒）
 *         3. 进入STOP2模式
 *         
 *         唤醒后：
 *         - 系统时钟恢复到MSI（4MHz）
 *         - 需要调用LP_ExitStop2()恢复时钟和外设
 */
void LP_EnterStop2(void)
{
  // 步骤1：设置STOP2唤醒标志（用于下次唤醒时判断）
  LP_SetStop2WakeupFlag();
  
  // 步骤2：确保EXTI中断已使能（LIS3DH唤醒源）
  // 注意：EXTI中断在MX_GPIO_Init()中已配置，但STOP2唤醒后需要重新使能
  // 确保中断优先级为最高（0,0），以便能够从STOP2唤醒
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  
  // 步骤3：进入STOP2模式
  // 注意：执行此函数后，程序会在此处停止，等待唤醒
  // 唤醒后会从此函数返回，继续执行后续代码
  // 使用HAL_PWREx_EnterSTOP2Mode（不是HAL_PWR_EnterSTOP2Mode）
  HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
  
  // 步骤4：唤醒后执行（从STOP2返回后）
  // 注意：STOP2唤醒后，系统时钟会恢复到MSI（4MHz），需要重新配置
  // 但这里不直接恢复，而是由调用者决定何时调用LP_ExitStop2()
}

/**
 * @brief  退出STOP2模式后的恢复处理
 * @note   STOP2唤醒后的必要操作：
 *         1. 恢复系统时钟（从MSI 4MHz恢复到80MHz）
 *         2. 重新初始化外设时钟（I2C、USART等）
 *         3. 清除STOP2唤醒标志
 */
void LP_ExitStop2(void)
{
  // 步骤1：恢复系统时钟
  // STOP2唤醒后，系统时钟会恢复到MSI（4MHz），需要重新配置到80MHz
  // 注意：SystemClock_Config()会重新配置PLL到80MHz
  SystemClock_Config();
  
  // 步骤2：重新初始化外设时钟
  // STOP2期间外设时钟可能被关闭，需要重新使能
  // 注意：HAL库会自动处理部分外设，但I2C等可能需要重新初始化
  // 这里只恢复时钟，具体外设初始化由调用者决定
  
  // 步骤3：清除STOP2唤醒标志（可选，也可以保留到下次进入STOP2前）
  // LP_ClearStop2WakeupFlag();  // 注释掉，保留标志直到下次进入STOP2
}

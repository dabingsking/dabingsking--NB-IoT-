/**
 * @file    bsp_radar.c
 * @brief   JSN-SR04T 超声波测距模块 BSP 实现。
 *
 * 当前实现说明：
 * - 使用 TIM2 1MHz 自由运行 + EXTI 双边沿 捕获 ECHO 上升/下降沿，计算脉宽（µs）。
 * - 提供单次测距与 5 次采样 + 中值/均值滤波的高层接口（参见 `BSP_Radar_SingleMeasure` / `BSP_Radar_Measure`）
 *
 */

#include "bsp_radar.h"
#include "bsp_debug.h"
#include "bsp_power.h"
#include "stm32l4xx_hal_tim.h"

extern TIM_HandleTypeDef htim2;

typedef enum {
  RADAR_SM_IDLE = 0,
  RADAR_SM_WAIT_RISE,
  RADAR_SM_WAIT_FALL,
  RADAR_SM_DONE,
  RADAR_SM_ERROR
} radar_sm_t;

static volatile radar_sm_t s_radar_sm = RADAR_SM_IDLE;
static volatile uint32_t s_echo_rise_cnt = 0;
static volatile uint32_t s_echo_fall_cnt = 0;
static volatile uint32_t s_echo_pulse_us = 0;
static volatile uint8_t s_echo_ready = 0;

/**
 * @brief  初始化DWT计数器
 * @note   只需调用一次，在系统初始化时调用
 */
static void BSP_Radar_InitDWT(void)
{
  // 启用DWT计数器（如果未启用）
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;
}

/**
 * @brief  微秒级延时函数
 * @param  us: 延时时间（微秒）
 * @note   使用DWT计数器实现延时
 *         系统时钟80MHz时，1us = 80个时钟周期
 */
static void BSP_Radar_DelayUs(uint32_t us)
{
  static uint8_t dwt_initialized = 0;

  // 首次调用时初始化DWT
  if (!dwt_initialized) {
    BSP_Radar_InitDWT();
    dwt_initialized = 1;
  }

  // 使用DWT计数器实现延时
  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = us * (SystemCoreClock / 1000000u);

  // 等待指定周期数
  while ((DWT->CYCCNT - start) < cycles) {
    // 等待
  }
}

/**
 * @brief  计算脉宽（us），基于TIM2计数值差值（TIM2=1MHz）
 */
static uint32_t BSP_Radar_DiffUs(uint32_t start_cnt, uint32_t end_cnt)
{
  if (end_cnt >= start_cnt) {
    return end_cnt - start_cnt;
  }
  // TIM2为32位自由运行，处理溢出
  return (0xFFFFFFFFu - start_cnt) + end_cnt + 1u;
}

void BSP_Radar_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin != RADAR_ECHO_Pin) {
    return;
  }

  GPIO_PinState level = HAL_GPIO_ReadPin(RADAR_ECHO_GPIO_Port, RADAR_ECHO_Pin);
  uint32_t now_cnt = __HAL_TIM_GET_COUNTER(&htim2); // 1cnt = 1us

  if (s_radar_sm == RADAR_SM_WAIT_RISE) {
    if (level == GPIO_PIN_SET) {
      s_echo_rise_cnt = now_cnt;
      s_radar_sm = RADAR_SM_WAIT_FALL;
    } else {
    }
    return;
  }

  if (s_radar_sm == RADAR_SM_WAIT_FALL) {
    if (level == GPIO_PIN_RESET) {
      s_echo_fall_cnt = now_cnt;
      s_echo_pulse_us = BSP_Radar_DiffUs(s_echo_rise_cnt, s_echo_fall_cnt);
      s_echo_ready = 1;
      s_radar_sm = RADAR_SM_DONE;
    } else {
    }
    return;
  }

  // 如果状态机不在WAIT_RISE或WAIT_FALL状态，忽略中断
}

/**
 * @brief  计算距离（cm）
 * @param  echo_time_us: ECHO高电平持续时间（微秒）
 * @retval 距离值（cm）
 * @note   JSN-SR04T使用标准公式：距离(cm) = echo_time_us / 58
 *         物理原理：距离 = (时间 × 声速) / 2
 *         声速 = 340 m/s = 34000 cm/s
 *         距离(cm) = (echo_time_us / 1000000) × 34000 / 2
 *                  = echo_time_us × 34000 / 2000000
 *                  = echo_time_us / 58.8235
 *         工程简化：距离(cm) ≈ echo_time_us / 58
 */
static uint16_t BSP_Radar_CalculateDistance(uint32_t echo_time_us)
{
  if (echo_time_us == 0) {
    return RADAR_ERR_INVALID;
  }

  uint32_t distance_cm = echo_time_us / 58u;

  if (distance_cm > 0xFFFFu) {
    return RADAR_ERR_INVALID;
  }

  return (uint16_t)distance_cm;
}

void BSP_Radar_Init(void)
{
  HAL_GPIO_WritePin(RADAR_TRIG_GPIO_Port, RADAR_TRIG_Pin, GPIO_PIN_RESET);
}

void BSP_Radar_PowerOn(void)
{
  BSP_Power_RadarOn();
}

void BSP_Radar_PowerOff(void)
{
  BSP_Power_RadarOff();
}

uint8_t BSP_Radar_IsDistanceValid(uint16_t distance_cm)
{
  if (distance_cm == RADAR_ERR_INVALID) {
    return 0;
  }

  if (distance_cm < RADAR_MIN_DISTANCE_CM || distance_cm > RADAR_MAX_DISTANCE_CM) {
    return 0;
  }

  return 1;
}

uint16_t BSP_Radar_SingleMeasure(void)
{
  // 1. 确保ECHO为低电平（等待上次测量完成）
  // 等待ECHO变为低电平，超时60ms
  uint32_t timeout_count = 60u;
  for (uint32_t i = 0; i < timeout_count; i++) {
    GPIO_PinState echo = HAL_GPIO_ReadPin(RADAR_ECHO_GPIO_Port, RADAR_ECHO_Pin);
    if (echo == GPIO_PIN_RESET) {
      break;  // ECHO已为低电平
    }
    HAL_Delay(1);
  }

  // 如果等待60ms后ECHO还是高电平，输出警告但继续
  if (HAL_GPIO_ReadPin(RADAR_ECHO_GPIO_Port, RADAR_ECHO_Pin) == GPIO_PIN_SET) {
    BSP_Debug_Printf("[RADAR] SingleMeasure: ECHO still HIGH after 60ms wait, continuing...\r\n");
  }

  // 2. 清除ECHO引脚的中断挂起标志
  __HAL_GPIO_EXTI_CLEAR_IT(RADAR_ECHO_Pin);

  // 3. 准备状态机
  s_echo_ready = 0;
  s_echo_pulse_us = 0;
  s_radar_sm = RADAR_SM_WAIT_RISE;

  // 4. 输出TRIG触发脉冲（10us高电平，JSN-SR04T要求≥10us）
  HAL_GPIO_WritePin(RADAR_TRIG_GPIO_Port, RADAR_TRIG_Pin, GPIO_PIN_SET);
  BSP_Radar_DelayUs(20);
  HAL_GPIO_WritePin(RADAR_TRIG_GPIO_Port, RADAR_TRIG_Pin, GPIO_PIN_RESET);

  // 5. 等待模块完成8个40kHz脉冲发送
  BSP_Radar_DelayUs(1000);

  // 6. 等待EXTI捕获完成，并做超时保护
  uint32_t start_ms = HAL_GetTick();
  while (!s_echo_ready) {
    uint32_t elapsed_ms = HAL_GetTick() - start_ms;
    if (elapsed_ms > (RADAR_ECHO_TIMEOUT_MS + RADAR_ECHO_START_TIMEOUT_MS + 10u)) {
      // 超时：未捕获到脉宽
      s_radar_sm = RADAR_SM_ERROR;
      GPIO_PinState echo_state = HAL_GPIO_ReadPin(RADAR_ECHO_GPIO_Port, RADAR_ECHO_Pin);
      uint32_t exti_pending = __HAL_GPIO_EXTI_GET_IT(RADAR_ECHO_Pin);
      BSP_Debug_Printf("[RADAR] SingleMeasure: EXTI wait timeout, sm=%u, ECHO=%u, EXTI_pending=%lu\r\n",
                       (unsigned)s_radar_sm,
                       (unsigned)echo_state,
                       (unsigned long)exti_pending);
      break;
    }
  }

  uint32_t echo_time_us = s_echo_pulse_us;

  if (echo_time_us == 0) {
    BSP_Debug_Printf("[RADAR] SingleMeasure: timeout (no echo)\r\n");
    return RADAR_ERR_INVALID;
  }

  // 5. 计算距离
  uint16_t distance_cm = BSP_Radar_CalculateDistance(echo_time_us);

  if (!BSP_Radar_IsDistanceValid(distance_cm)) {
    return RADAR_ERR_INVALID;
  }

  return distance_cm;
}

/**
 * @brief  对数组进行冒泡排序
 * @param  arr: 数组指针
 * @param  len: 数组长度
 */
static void BSP_Radar_SortArray(uint16_t *arr, uint8_t len)
{
  for (uint8_t i = 0; i < len - 1; i++) {
    for (uint8_t j = 0; j < len - 1 - i; j++) {
      if (arr[j] > arr[j + 1]) {
        uint16_t temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

uint16_t BSP_Radar_Measure(void)
{
  uint16_t samples[RADAR_SAMPLE_COUNT];
  uint8_t valid_count = 0;

  for (uint8_t i = 0; i < RADAR_SAMPLE_COUNT; i++) {
    uint16_t distance = BSP_Radar_SingleMeasure();

    if (distance != RADAR_ERR_INVALID) {
      if (distance > RADAR_MAX_DISTANCE_CM || distance < RADAR_MIN_DISTANCE_CM) {
        // out of range, skip
      } else {
        samples[valid_count] = distance;
        valid_count++;
      }
    }

    // 采样间隔
    if (i < RADAR_SAMPLE_COUNT - 1) {
      HAL_Delay(RADAR_SAMPLE_INTERVAL_MS);
    }
  }

  if (valid_count < 3) {
    BSP_Debug_Printf("[RADAR] Measure: insufficient valid samples (%u/%u)\r\n",
                    valid_count, RADAR_SAMPLE_COUNT);
    return RADAR_ERR_INVALID;
  }

  // 对有效样本排序
  BSP_Radar_SortArray(samples, valid_count);

  uint16_t median = samples[valid_count / 2];
  uint16_t final_distance;

  if (valid_count >= 5) {
    // 剔除最大最小值后，对剩余点求均值
    uint32_t sum = 0;
    for (uint8_t i = 1; i < valid_count - 1; i++) {
      sum += samples[i];
    }
    final_distance = (uint16_t)(sum / (valid_count - 2));
  } else if (valid_count == 4) {
    // 剔除最大最小值后，对剩余2个求均值
    uint32_t sum = samples[1] + samples[2];
    final_distance = (uint16_t)(sum / 2u);
  } else if (valid_count == 3) {
    // 检测异常值
    uint16_t min_val = samples[0];
    uint16_t max_val = samples[2];
    uint16_t mid_val = samples[1];
    uint16_t range = max_val - min_val;

    // 如果最大值和最小值的差异超过30cm或超过中值的50%，认为存在异常值
    uint16_t threshold = (mid_val * 40u) / 100u;  // 中值的40%
    if (threshold < 30u) threshold = 30u;  // 最小阈值30cm

    if (range > threshold) {
      // 存在异常值
      uint16_t diff_min = mid_val - min_val;
      uint16_t diff_max = max_val - mid_val;

      if (diff_min > diff_max) {
        final_distance = (uint16_t)((mid_val + max_val) / 2u);
      } else {
        final_distance = (uint16_t)((min_val + mid_val) / 2u);
      }
    } else {
      // 没有明显异常值，使用中值
      final_distance = median;
    }
  } else {
    // 使用中值
    final_distance = median;
  }

  // 最终范围检查
  if (BSP_Radar_IsDistanceValid(final_distance)) {
    BSP_Debug_Printf("[RADAR] Measure: final=%u cm (valid=%u)\r\n", final_distance, valid_count);
    return final_distance;
  } else {
    BSP_Debug_Printf("[RADAR] Measure: final distance out of range (%u cm)\r\n", final_distance);
    return RADAR_ERR_INVALID;
  }
}

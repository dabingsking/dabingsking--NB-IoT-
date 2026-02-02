/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_debug.h"
#include "bsp_lis3dh.h"
#include "bsp_lowpower.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// LIS3DH唤醒标志：EXTI9_5中断服务程序设置，主循环检测并处理
volatile uint8_t g_wakeup_source_acc = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  
  // 判断是否为STOP2唤醒（在初始化串口之前判断，避免串口输出问题）
  uint8_t is_wakeup = LP_IsStop2Wakeup();
  
  // 初始化调试串口（仅在首次上电时初始化，STOP2唤醒后会在恢复代码中重新初始化）
  if (!is_wakeup) {
    BSP_Debug_Init();
    BSP_Debug_SendString("\r\n[BOOT] Lowpower_01 USART1 debug ready\r\n");
    BSP_Debug_Printf("[BOOT] SYSCLK=%lu Hz\r\n", HAL_RCC_GetSysClockFreq());
    BSP_Debug_Printf("[BOOT] USART2 baud=%lu\r\n", huart2.Init.BaudRate);
  }
  if (is_wakeup) {
    // STOP2唤醒：先恢复时钟（串口输出需要时钟稳定）
    LP_ExitStop2();
    
    // 等待时钟稳定（STOP2唤醒后时钟从MSI恢复到80MHz需要时间）
    // 增加延迟确保时钟完全稳定
    HAL_Delay(50);
    
    // 重新初始化串口（STOP2唤醒后串口需要重新初始化才能正常输出）
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    
    // 等待串口初始化完成并稳定
    HAL_Delay(20);
    
    // 重新初始化调试串口
    BSP_Debug_Init();
    
    // 立即发送一个测试字符，确认串口工作并刷新缓冲区
    uint8_t test_char = '\n';
    HAL_UART_Transmit(&huart1, &test_char, 1, 100);
    HAL_Delay(10);
    
    // 现在可以安全地输出调试信息
    // 使用HAL_UART_Transmit直接发送，确保立即输出
    const char *msg1 = "\r\n[BOOT] STOP2 wakeup detected, restoring system...\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)msg1, strlen(msg1), 100);
    HAL_Delay(10);
    LP_ClearStop2WakeupFlag();
    
    // 重新初始化外设（STOP2唤醒后需要重新初始化）
    // 注意：I2C在STOP2期间可能被关闭，需要重新初始化
    MX_I2C1_Init();
    
    // 重新配置GPIO和EXTI中断（STOP2唤醒后EXTI配置可能丢失）
    // 注意：STOP2唤醒后，GPIO配置可能保持，但EXTI中断需要重新使能
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
    
    // 关键：STOP2唤醒后，检查INT1引脚状态和INT1_SRC寄存器
    // 因为RAM在STOP2期间会丢失，g_wakeup_source_acc无法保存中断状态
    // 需要立即检查是否是LIS3DH中断唤醒
    GPIO_PinState int1_state = HAL_GPIO_ReadPin(ACC_INT1_GPIO_Port, ACC_INT1_Pin);
    uint8_t exti_pending = (__HAL_GPIO_EXTI_GET_IT(ACC_INT1_Pin) != 0) ? 1 : 0;
    
    // 使用直接HAL_UART_Transmit确保立即输出
    char int1_msg[64];
    int len = snprintf(int1_msg, sizeof(int1_msg), 
                      "[BOOT] System restored from STOP2, INT1 pin=%d, EXTI pending=%d\r\n",
                      (int1_state == GPIO_PIN_SET) ? 1 : 0, exti_pending);
    if (len > 0 && len < (int)sizeof(int1_msg)) {
      HAL_UART_Transmit(&huart1, (uint8_t *)int1_msg, len, 100);
      HAL_Delay(5);
    }
    
    // 如果INT1引脚为高电平，说明LIS3DH中断已触发（锁存模式）
    // 或者检查EXTI中断标志
    if (int1_state == GPIO_PIN_SET || exti_pending != 0) {
      // LIS3DH中断唤醒，设置标志
      g_wakeup_source_acc = 1;
      const char *lis3dh_msg = "[BOOT] LIS3DH interrupt wakeup detected!\r\n";
      HAL_UART_Transmit(&huart1, (uint8_t *)lis3dh_msg, strlen(lis3dh_msg), 100);
      HAL_Delay(5);
      // 清除EXTI中断标志
      __HAL_GPIO_EXTI_CLEAR_IT(ACC_INT1_Pin);
    } else {
      // 清除EXTI中断标志（防止残留）
      __HAL_GPIO_EXTI_CLEAR_IT(ACC_INT1_Pin);
      const char *no_int_msg = "[BOOT] No LIS3DH interrupt detected at wakeup\r\n";
      HAL_UART_Transmit(&huart1, (uint8_t *)no_int_msg, strlen(no_int_msg), 100);
      HAL_Delay(5);
    }
  } else {
    // 首次上电：完整初始化
    BSP_Debug_Printf("[BOOT] First power-on, full initialization\r\n");
  }

  if (LIS3DH_Init() == HAL_OK) {
    uint8_t whoami = 0;
    (void)LIS3DH_ReadWhoAmI(&whoami);
    // LIS3DH 初始化成功：打印器件ID，正常应为 0x33
    BSP_Debug_Printf("[LIS3DH] init OK, WHOAMI=0x%02X\r\n", whoami);
    
    // 初始化完成后，清除EXTI中断标志（防止初始化过程中的残留中断）
    // 注意：必须在LIS3DH初始化完成后清除，确保INT1引脚状态稳定
    __HAL_GPIO_EXTI_CLEAR_IT(ACC_INT1_Pin);
    BSP_Debug_Printf("[LIS3DH] EXTI flag cleared, interrupt ready\r\n");
  } else {
    uint8_t whoami = 0xFF;
    (void)LIS3DH_ReadWhoAmI(&whoami);
    // 初始化失败：一般是 I2C 地址/连线/供电问题（或 SDO=VCC 导致地址变为 0x19）
    BSP_Debug_Printf("[LIS3DH] init FAIL, WHOAMI=0x%02X\r\n", whoami);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // 检测LIS3DH运动检测中断唤醒（EXTI9_5）
    if (g_wakeup_source_acc != 0) {
      // 清除唤醒标志（只在主循环处理中断）
      g_wakeup_source_acc = 0;
      
      // 调试：确认中断已触发（使用直接HAL_UART_Transmit确保立即输出）
      const char *int_detected_msg = "[MAIN] LIS3DH interrupt detected!\r\n";
      HAL_UART_Transmit(&huart1, (uint8_t *)int_detected_msg, strlen(int_detected_msg), 100);
      HAL_Delay(5);

      // 小延迟确保I2C总线就绪（中断触发后I2C可能需要短暂恢复时间）
      HAL_Delay(5);

      // 读取INT1_SRC：在锁存模式下，读取此寄存器会清除中断标志
      // 读取前先检查中断源，确认是运动检测中断（IA位=1）
      uint8_t int1_src = 0;
      if (LIS3DH_ReadInt1Src(&int1_src) == HAL_OK) {
        // 检查IA位（bit 6），确认是运动检测中断
        if ((int1_src & 0x40) == 0) {
          // IA位为0，可能不是运动检测中断，记录警告
          BSP_Debug_Printf("[LIS3DH] WARNING: int1_src=0x%02X (IA=0, may not be motion interrupt)\r\n", int1_src);
        }
        float ax_g = 0.0f, ay_g = 0.0f, az_g = 0.0f;
        HAL_StatusTypeDef status_accel = LIS3DH_ReadAccelG(&ax_g, &ay_g, &az_g);
        if (status_accel == HAL_OK) {
          BSP_Debug_Printf("[LIS3DH] WAKEUP! ax=%.3fg ay=%.3fg az=%.3fg int1_src=0x%02X\r\n",
                          (double)ax_g, (double)ay_g, (double)az_g, int1_src);
        } else {
          BSP_Debug_Printf("[LIS3DH] WAKEUP! (read accel failed, status=%d) int1_src=0x%02X\r\n",
                          status_accel, int1_src);
        }
      } else {
        BSP_Debug_Printf("[LIS3DH] WAKEUP! (read int1_src failed)\r\n");
      }
    }
    
    // 周期性读取LIS3DH数据（心跳打印，用于调试和可视化）
    // 注意：如果系统正常进入STOP2，心跳打印应该停止
    // 如果心跳打印还在继续，说明系统没有进入STOP2模式
    static uint32_t last = 0;
    uint32_t now = HAL_GetTick();
    if ((now - last) >= 2000) {
      last = now;
      BSP_Debug_Printf("[HB] tick=%lu (if you see this, system is NOT in STOP2!)\r\n", now);

      // 读取加速度数据（g单位，便于可视化）
      float ax_g = 0.0f, ay_g = 0.0f, az_g = 0.0f;
      int16_t ax_raw = 0, ay_raw = 0, az_raw = 0;
      if (LIS3DH_ReadAccelRaw(&ax_raw, &ay_raw, &az_raw) == HAL_OK) {
        // 转换为g单位（不再在心跳中读取INT1_SRC，避免影响锁存中断电平）
        if (LIS3DH_ReadAccelG(&ax_g, &ay_g, &az_g) == HAL_OK) {
          // 打印原始值和g单位的加速度数据，便于调试分析
          BSP_Debug_Printf("[LIS3DH] raw: x=%d y=%d z=%d | g: ax=%.3fg ay=%.3fg az=%.3fg\r\n",
                          ax_raw, ay_raw, az_raw, (double)ax_g, (double)ay_g, (double)az_g);
          
          // 调试：检查INT1引脚电平和中断状态（不读取INT1_SRC，避免清除锁存中断）
          GPIO_PinState int1_pin_state = HAL_GPIO_ReadPin(ACC_INT1_GPIO_Port, ACC_INT1_Pin);
          uint8_t exti_pending = (__HAL_GPIO_EXTI_GET_IT(ACC_INT1_Pin) != 0) ? 1 : 0;
          BSP_Debug_Printf("[LIS3DH] INT1 pin=%d, EXTI pending=%d\r\n", 
                          (int1_pin_state == GPIO_PIN_SET) ? 1 : 0, exti_pending);
        } else {
          BSP_Debug_Printf("[LIS3DH] read raw OK but convert ERR\r\n");
        }
      } else {
        BSP_Debug_Printf("[LIS3DH] read ERR\r\n");
      }
    }
    
    // 完成所有处理后，进入STOP2低功耗模式
    // 注意：STOP2唤醒后，程序会从main()重新开始执行（系统复位）
    // 因此需要在main()开始处判断是否为STOP2唤醒，并恢复系统
    BSP_Debug_Printf("[MAIN] Entering STOP2 mode, waiting for wakeup...\r\n");
    BSP_Debug_Printf("[MAIN] Flushing UART before STOP2...\r\n");
    HAL_Delay(200);  // 增加延迟，确保串口输出完全完成
    
    // 进入STOP2模式（程序会在此处停止，等待唤醒）
    // 唤醒后系统会复位，程序从main()重新开始执行
    // 注意：如果看到心跳打印继续，说明没有进入STOP2
    BSP_Debug_Printf("[MAIN] About to enter STOP2 NOW...\r\n");
    HAL_Delay(50);  // 最后一次延迟
    
    LP_EnterStop2();
    
    // 注意：STOP2唤醒后不会执行到这里，而是从main()重新开始
    // 如果执行到这里，说明没有进入STOP2模式！
    BSP_Debug_Printf("[MAIN] ERROR: Returned from LP_EnterStop2()! STOP2 failed!\r\n");
    
    // 注意：STOP2唤醒后不会执行到这里，而是从main()重新开始
    // 因此不需要在这里处理唤醒后的恢复
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();

  /** Enable the SYSCFG APB clock
  */
  __HAL_RCC_CRS_CLK_ENABLE();

  /** Configures CRS
  */
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_LSE;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
  RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_RELOADVALUE_CALCULATE(48000000,32768);
  RCC_CRSInitStruct.ErrorLimitValue = 34;
  RCC_CRSInitStruct.HSI48CalibrationValue = 32;

  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10D19CE4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RADAR_TRIG_GPIO_Port, RADAR_TRIG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GAS_PWR_CTRL_Pin|RADAR_PWR_CTRL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : RADAR_TRIG_Pin */
  GPIO_InitStruct.Pin = RADAR_TRIG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RADAR_TRIG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RADAR_ECHO_Pin */
  GPIO_InitStruct.Pin = RADAR_ECHO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RADAR_ECHO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : HALL_DO_Pin */
  GPIO_InitStruct.Pin = HALL_DO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(HALL_DO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GAS_PWR_CTRL_Pin RADAR_PWR_CTRL_Pin */
  GPIO_InitStruct.Pin = GAS_PWR_CTRL_Pin|RADAR_PWR_CTRL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ACC_INT1_Pin */
  GPIO_InitStruct.Pin = ACC_INT1_Pin;
  // INT1 配置为高电平有效脉冲：使用上升沿触发中断
  // LIS3DH INT1 默认空闲为低电平，事件发生时输出高电平脉冲
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;           // 由LIS3DH驱动电平，MCU侧不再上拉
  HAL_GPIO_Init(ACC_INT1_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

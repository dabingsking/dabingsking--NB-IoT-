/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
// 需要使用TIM2做1MHz自由运行计时（雷达ECHO脉宽测量）
#include "stm32l4xx_hal_tim.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MQ_4_AO_Pin GPIO_PIN_0
#define MQ_4_AO_GPIO_Port GPIOA
#define MCU_TX_NB_Pin GPIO_PIN_2
#define MCU_TX_NB_GPIO_Port GPIOA
#define MCU_RX_NB_Pin GPIO_PIN_3
#define MCU_RX_NB_GPIO_Port GPIOA
#define RADAR_TRIG_Pin GPIO_PIN_6
#define RADAR_TRIG_GPIO_Port GPIOA
#define RADAR_ECHO_Pin GPIO_PIN_7
#define RADAR_ECHO_GPIO_Port GPIOA
#define HALL_DO_Pin GPIO_PIN_5
#define HALL_DO_GPIO_Port GPIOC
#define GAS_PWR_CTRL_Pin GPIO_PIN_0
#define GAS_PWR_CTRL_GPIO_Port GPIOB
#define RADAR_PWR_CTRL_Pin GPIO_PIN_1
#define RADAR_PWR_CTRL_GPIO_Port GPIOB
#define ACC_INT1_Pin GPIO_PIN_8
#define ACC_INT1_GPIO_Port GPIOA
#define ACC_INT1_EXTI_IRQn EXTI9_5_IRQn
#define I2C_SCL_Pin GPIO_PIN_8
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_9
#define I2C_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

// USER 按键（PC13，上拉输入，按下=低电平）
#define USER_BTN_Pin GPIO_PIN_13
#define USER_BTN_GPIO_Port GPIOC

// RGB LED（共阳极：低电平点亮）
#define LED_R_Pin GPIO_PIN_0
#define LED_R_GPIO_Port GPIOC
#define LED_G_Pin GPIO_PIN_1
#define LED_G_GPIO_Port GPIOC
#define LED_B_Pin GPIO_PIN_2
#define LED_B_GPIO_Port GPIOC

// LIS3DH唤醒标志
extern volatile uint8_t g_wakeup_source_acc;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

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

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

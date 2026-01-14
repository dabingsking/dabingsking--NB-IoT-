#ifndef __HALL_H
#define __HALL_H

#include "sys.h"
#include <stdio.h>

#define KEY1_GPIO_CLK	RCC_APB2Periph_GPIOB
#define KEY1_GPIO_PORT	GPIOB
#define KEY1_GPIO_PIN	GPIO_Pin_1

#define KEY1 			GPIO_ReadInputDataBit(KEY1_GPIO_PORT, KEY1_GPIO_PIN)

extern uint8_t is_magnet_present;

void key_init(void);
void key_process(void);
void hall_handle(void);

#endif

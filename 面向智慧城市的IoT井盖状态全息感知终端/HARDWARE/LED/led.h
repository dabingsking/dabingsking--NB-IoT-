#ifndef __LED_H
#define __LED_H	 

#include "sys.h"

#define LED1_GPIO_CLK	RCC_APB2Periph_GPIOC
#define LED1_GPIO_PORT	GPIOC
#define LED1_GPIO_PIN	GPIO_Pin_13

#define LED2_GPIO_CLK	RCC_APB2Periph_GPIOB
#define LED2_GPIO_PORT	GPIOB
#define LED2_GPIO_PIN	GPIO_Pin_5

#define LED3_GPIO_CLK	RCC_APB2Periph_GPIOB
#define LED3_GPIO_PORT	GPIOB
#define LED3_GPIO_PIN	GPIO_Pin_9

#define LED1(onoff) 	GPIO_WriteBit(LED1_GPIO_PORT, LED1_GPIO_PIN, onoff)
#define LED2(onoff) 	GPIO_WriteBit(LED2_GPIO_PORT, LED2_GPIO_PIN, onoff)
#define LED3(onoff) 	GPIO_WriteBit(LED3_GPIO_PORT, LED3_GPIO_PIN, onoff)

void led_init(void);//≥ı ºªØ

		 				    
#endif

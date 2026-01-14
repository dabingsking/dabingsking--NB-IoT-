#ifndef __HCSR04_H
#define __HCSR04_H

#include "sys.h"

#define HCSR04_TRIG_GPIO_CLK	RCC_APB2Periph_GPIOB
#define HCSR04_TRIG_GPIO_PORT	GPIOB
#define HCSR04_TRIG_GPIO_PIN	GPIO_Pin_3

#define HCSR04_ECHO_GPIO_CLK	RCC_APB2Periph_GPIOB
#define HCSR04_ECHO_GPIO_PORT	GPIOB
#define HCSR04_ECHO_GPIO_PIN	GPIO_Pin_4

#define HCSR04_ECHO_EXIT_PORT	GPIO_PortSourceGPIOB
#define HCSR04_ECHO_EXIT_PIN	GPIO_PinSource4
#define HCSR04_ECHO_EXIT_LINE	EXTI_Line4

#define HCSR04_TRIG(onoff) 	GPIO_WriteBit(HCSR04_TRIG_GPIO_PORT, HCSR04_TRIG_GPIO_PIN, onoff)
#define HCSR04_ECHO(onoff) 	GPIO_WriteBit(HCSR04_ECHO_GPIO_PORT, HCSR04_ECHO_GPIO_PIN, onoff)

extern uint32_t distance;

void hcsr04_init(void);
void hcsr04_handle(void);

#endif

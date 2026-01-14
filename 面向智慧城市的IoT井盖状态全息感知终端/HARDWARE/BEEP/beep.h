#ifndef __BEEP_H
#define __BEEP_H

#include "sys.h"

#define BEEP_GPIO_CLK	RCC_APB2Periph_GPIOB
#define BEEP_GPIO_PORT	GPIOB
#define BEEP_GPIO_PIN	GPIO_Pin_14

#define BEEP(onoff) 	GPIO_WriteBit(BEEP_GPIO_PORT, BEEP_GPIO_PIN, onoff)

void beep_init(void);//≥ı ºªØ

#endif

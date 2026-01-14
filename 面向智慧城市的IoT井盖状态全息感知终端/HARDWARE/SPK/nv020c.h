#ifndef __NV020C_H
#define __NV020C_H

#include "sys.h"
#include <stdio.h>

#define NV020C_SDA_GPIO_CLK		RCC_APB2Periph_GPIOB
#define NV020C_SDA_GPIO_PORT	GPIOB
#define NV020C_SDA_GPIO_PIN		GPIO_Pin_13

#define NV020C_BUSY_GPIO_CLK	RCC_APB2Periph_GPIOB
#define NV020C_BUSY_GPIO_PORT	GPIOB
#define NV020C_BUSY_GPIO_PIN	GPIO_Pin_12

#define NV020C_SDA(onoff)		GPIO_WriteBit(NV020C_SDA_GPIO_PORT, NV020C_SDA_GPIO_PIN, onoff)
#define NV020C_BUSY				GPIO_ReadInputDataBit(NV020C_BUSY_GPIO_PORT, NV020C_BUSY_GPIO_PIN)

void nv020c_init(void);
void nv020c_play_sound(uint8_t addr);
void nv020c_play(uint16_t num);

#endif

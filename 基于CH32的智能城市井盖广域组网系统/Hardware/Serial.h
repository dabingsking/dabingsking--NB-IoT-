#ifndef __SERIAL_H
#define __SERIAL_H
#include "debug.h"
#include <stdio.h>

void Buzzer_ON_Twice(void);


extern char Usart1_Rx_Data[400];
extern char Usart2_Rx_Data[400];
void DMA_Usart1_Init(uint32_t BaudRate);
void DMA_Usart1_Start(void);
void DMA_Usart2_Init(uint32_t BaudRate);
void DMA_Usart2_Start(void);
void Usart1_Send_Char(char _char);
void Usart1_Send_String(char *str);
void Usart2_Send_Char(char _char);
void Usart2_Send_String(char *str);
void EC01G_Printf(char *format, ...);


#endif


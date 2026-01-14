#ifndef __MySPI_H
#define __MySPI_H

//直接写SS函数
#define ICM42688_SS(BitValue){GPIO_WriteBit(GPIOA,GPIO_Pin_4,(BitAction)BitValue);}

void MySPI_Init(void);
void MySPI_Start(void);
void MySPI_Stop(void);
uint8_t MySPI_WriteReadByte(uint8_t ByteSend);

#endif


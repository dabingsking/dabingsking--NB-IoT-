#include "sys.h"

char Usart1_Rx_Data[400];									//串口1接收数据的数组
char Usart2_Rx_Data[400];									//串口2接收数据的数组

/*
参数  : 无
函数体: 蜂鸣器响两次函数，用于提示
返回值: 无
*/
void Buzzer_ON_Twice(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_12, (BitAction)SET);
	Delay_Ms(150);
	GPIO_WriteBit(GPIOB, GPIO_Pin_12, (BitAction)RESET);	
	Delay_Ms(50);
	GPIO_WriteBit(GPIOB, GPIO_Pin_12, (BitAction)SET);	
	Delay_Ms(100);
	GPIO_WriteBit(GPIOB, GPIO_Pin_12, (BitAction)RESET);
}

/*
参数  : 无
函数体: 串口1GPIO口初始化函数
返回值: 无
*/
void DMA_Usart1_GPIO_Init(void)
{
	RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
}

/*
参数  : BaudRate：波特率
函数体: 串口1和DMA初始化函数
返回值: 无
*/
void DMA_Usart1_Init(uint32_t BaudRate)
{
	//各时钟使能
	DMA_Usart1_GPIO_Init();
	RCC_PB2PeriphClockCmd(RCC_PB2Periph_USART1, ENABLE);
	RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA1, ENABLE);
	//USART1
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = BaudRate;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStruct);
	//DMA
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DATAR;
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&Usart1_Rx_Data;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_BufferSize = 400;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_Init(DMA1_Channel5, &DMA_InitStruct);
	//NVIC
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
	//各通道使能
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	USART_Cmd(USART1, ENABLE);
	DMA_Cmd(DMA1_Channel5, ENABLE);
}

/*
参数  : 无
函数体: 串口1使能函数
返回值: 无
*/
void DMA_Usart1_Start(void)
{
	DMA_Cmd(DMA1_Channel5, DISABLE);
	DMA_SetCurrDataCounter(DMA1_Channel5, 400);
	DMA_Cmd(DMA1_Channel5, ENABLE);
}

/*
参数  : 无
函数体: 串口2GPIO口初始化函数
返回值: 无
*/
void DMA_Usart2_GPIO_Init(void)
{
	RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/*
参数  : BaudRate：波特率
函数体: 串口2和DMA初始化函数
返回值: 无
*/
void DMA_Usart2_Init(uint32_t BaudRate)
{
	//各时钟使能
	DMA_Usart2_GPIO_Init();
	RCC_PB1PeriphClockCmd(RCC_PB1Periph_USART2, ENABLE);
	RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA1, ENABLE);
	//USART2
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = BaudRate;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &USART_InitStruct);
	//DMA
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DATAR;
	DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&Usart2_Rx_Data;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_BufferSize = 400;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStruct.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_Init(DMA1_Channel6, &DMA_InitStruct);
	//NVIC	
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);
	//各通道使能
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	USART_Cmd(USART2, ENABLE);
	DMA_Cmd(DMA1_Channel6, ENABLE);
}

/*
参数  : 无
函数体: 串口2使能函数
返回值: 无
*/
void DMA_Usart2_Start(void)
{
	DMA_Cmd(DMA1_Channel6, DISABLE);
	DMA_SetCurrDataCounter(DMA1_Channel6, 400);
	DMA_Cmd(DMA1_Channel6, ENABLE);
}

/*
参数  : 要发送的字符
函数体: 串口1发送一个字符的函数
返回值: 无
*/
void Usart1_Send_Char(char _char)
{
	USART_SendData(USART1, _char);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)==RESET);
}

/*
参数  : 要发送的字符串
函数体: 串口1字符串的函数
返回值: 无
*/
void Usart1_Send_String(char *str)
{
	while(*str!='\0')
	{
		Usart1_Send_Char(*str);
		str++;
	}
}

/*
参数  : 要发送的字符串
函数体: 串口1重写Printf函数
返回值: 无
*/
void EC01G_Printf(char *format, ...)
{
	char String[400];						//首先定义输出的字符串
	va_list arg;							//定义一个参数列表变量
	va_start(arg, format);					//从format开始接受参数表，并放在arg里面
	vsprintf(String, format, arg);			//打印可变参数
	va_end(arg);							//释放参数表
	Usart2_Send_String(String);				//发送string
}

/*
参数  : 要发送的字符
函数体: 串口2发送一个字符的函数
返回值: 无
*/
void Usart2_Send_Char(char _char)
{
	USART_SendData(USART2, _char);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE)==RESET);
}

/*
参数  : 要发送的字符串
函数体: 串口2字符串的函数
返回值: 无
*/
void Usart2_Send_String(char *str)
{
	while(*str!='\0')
	{
		Usart2_Send_Char(*str);
		str++;
	}
}

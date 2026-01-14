#include "sys.h"

/*
参数  : 无
函数体: SPI初始化函数
返回值: 无
*/
void MySPI_Init(void)
{
	// 初始化GPIO引脚
	RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOA|RCC_PB2Periph_SPI1,ENABLE);
	
	GPIO_InitTypeDef GPIOIntistructure;
	GPIOIntistructure.GPIO_Mode=GPIO_Mode_Out_PP;							// MOSI，CS，SCK都是输出引脚，CS没有复用
	GPIOIntistructure.GPIO_Pin=GPIO_Pin_4;
	GPIOIntistructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIOIntistructure);

	GPIOIntistructure.GPIO_Mode=GPIO_Mode_AF_PP;							// MOSI，CS，SCK都是输出引脚，MOSI，SCK是复用引脚
	GPIOIntistructure.GPIO_Pin=GPIO_Pin_5|GPIO_Pin_7;
	GPIOIntistructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIOIntistructure);
	
	GPIOIntistructure.GPIO_Mode=GPIO_Mode_IPU;								// MISO是输入引脚，所以选择上拉输入
	GPIOIntistructure.GPIO_Pin=GPIO_Pin_6;
	GPIOIntistructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIOIntistructure);
	// 这里只针对SPI口初始化,SPI的复位和停止复位
	RCC_PB2PeriphResetCmd(RCC_PB2Periph_SPI1,ENABLE);						// 复位SPI1
	RCC_PB2PeriphResetCmd(RCC_PB2Periph_SPI1,DISABLE);						// 停止复位SPI1
	// 初始化SPI
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;								// 指定当前设备为主机
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;		// 选择SPI裁剪模式，为双线全双工
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;							// 配置8为数据帧
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;						// 配置为高位先行
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_256;		// 波特率预分频器，256分频
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;								// 要选择模式0，SPI_CPOL=0,空闲时是低电平
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;								// 选择第一个边沿采样，即SPI_CPHA=0
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;									// 选择软件模拟NSS
	SPI_InitStructure.SPI_CRCPolynomial=7;  								// CRC校验，不使用填默认值7
	SPI_Init(SPI1,&SPI_InitStructure);
	// 使能SPI外设
	SPI_Cmd(SPI1,ENABLE);
	// 默认给SS输出高电平，默认不选中从机
	ICM42688_SS(1);
}

/*
参数  : 无
函数体: SPI启动函数
返回值: 无
*/
void MySPI_Start(void)
{
	ICM42688_SS(0);
}

/*
参数  : 无
函数体: SPI停止函数
返回值: 无
*/
void MySPI_Stop(void)
{
	ICM42688_SS(1);
}

/*
参数  : 要写的内容
函数体: SPI写字节函数
返回值: 无
*/
uint8_t MySPI_WriteReadByte(uint8_t ByteSend)
{
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)!=SET);
	SPI_I2S_SendData(SPI1,ByteSend);
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)!=SET);
	return SPI_I2S_ReceiveData(SPI1);
}


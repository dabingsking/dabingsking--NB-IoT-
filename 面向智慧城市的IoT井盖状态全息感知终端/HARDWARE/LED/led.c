#include "led.h"
#include "flag.h"

void led_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(LED1_GPIO_CLK | LED2_GPIO_CLK | LED3_GPIO_CLK, ENABLE);	 //使能端口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = LED1_GPIO_PIN;	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度为50MHz
	GPIO_Init(LED1_GPIO_PORT, &GPIO_InitStructure); //根据设定参数初始化
	
	GPIO_InitStructure.GPIO_Pin = LED2_GPIO_PIN;	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度为50MHz
	GPIO_Init(LED2_GPIO_PORT, &GPIO_InitStructure); //根据设定参数初始化
	
	GPIO_InitStructure.GPIO_Pin = LED3_GPIO_PIN;	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度为50MHz
	GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStructure); //根据设定参数初始化
	
	LED1(Bit_RESET);
	LED2(Bit_RESET);
	LED3(Bit_RESET);
}
 
//将JTAG引脚禁用，PA13/14/15 & PB3/4默认配置为JTAG功能,注意
//RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE); // 改变指定管脚的映射 GPIO_Remap_SWJ_Disable SWJ 完全禁用（JTAG+SW-DP）

void led_handle(void)
{
	if(flag_get(&sys_flag, SYS_LDR_FLAG))
	{
		LED2(Bit_SET); //当检测到周围光线较弱时，自动可以打开警示灯
	}
	else
	{
		LED2(Bit_RESET);
	}
}

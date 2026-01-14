#include "nv020c.h"
#include "delay.h"

void nv020c_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(NV020C_SDA_GPIO_CLK | NV020C_BUSY_GPIO_CLK, ENABLE);	 //使能端口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = NV020C_SDA_GPIO_PIN;	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度为50MHz
	GPIO_Init(NV020C_SDA_GPIO_PORT, &GPIO_InitStructure); //根据设定参数初始化
	
	GPIO_InitStructure.GPIO_Pin = NV020C_BUSY_GPIO_PIN;	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度为50MHz
	GPIO_Init(NV020C_BUSY_GPIO_PORT, &GPIO_InitStructure); //根据设定参数初始化
}

void nv020c_play_sound(uint8_t addr)
{
	uint8_t i;
	
	NV020C_SDA(0);
	delay_ms(5);
	
	for(i=0;i<8;i++)
	{
		NV020C_SDA(1);
		if(addr&1)
		{
			delay_ms(3);
			NV020C_SDA(0);
			delay_ms(1);
		}
		else
		{
			delay_ms(1);
			NV020C_SDA(0);
			delay_ms(3);
		}
		addr>>=1;
	}
	NV020C_SDA(1);
	while(!NV020C_BUSY); // 等待播放完毕
}

void nv020c_play(uint16_t num)
{
	uint8_t bai,shi,ge;
	if(num < 1000)
	{
		bai = num/100;
		shi = num%100/10;
		ge = num%10;
		if(bai != 0)
		{
			nv020c_play_sound(bai+0x0c);
			nv020c_play_sound(0x17);
		}
		
		if(bai == 0 && shi == 0)
		{
			
		}
		else
		{
			nv020c_play_sound(shi+0x0c);
			nv020c_play_sound(0x16);
		}
		
		nv020c_play_sound(ge+0x0c);
		nv020c_play_sound(0x18);
	}
}

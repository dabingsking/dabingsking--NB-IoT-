#include "hcsr04.h"
#include "beep.h"
#include "delay.h"
#include <stdio.h>

#define HCSR04_DIS_THR	200

uint32_t number=0;			//记录定时器中断的次数
uint32_t times=0;			//记录回响信号的持续时间
uint32_t distance;

void hcsr04_init(void)
{
	GPIO_InitTypeDef itd;
	EXTI_InitTypeDef itd1;
	NVIC_InitTypeDef itd2;
	TIM_TimeBaseInitTypeDef itd3;   
	NVIC_InitTypeDef itd4;
	
	//初始化GPIO口,Trig使用推挽输出,Echo使用浮空输入
    RCC_APB2PeriphClockCmd(HCSR04_TRIG_GPIO_CLK |HCSR04_ECHO_GPIO_CLK, ENABLE);        //使能GPIOC的外设时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
    itd.GPIO_Mode = GPIO_Mode_Out_PP;                             //选择推挽输出模式
    itd.GPIO_Pin = HCSR04_TRIG_GPIO_PIN;                                
    itd.GPIO_Speed = GPIO_Speed_50MHz;                            //默认选择50MHz
    GPIO_Init(HCSR04_TRIG_GPIO_PORT, &itd);
    
    itd.GPIO_Mode = GPIO_Mode_IPU;//GPIO_Mode_IN_FLOATING;                        //选择浮空输入模式
    itd.GPIO_Pin = HCSR04_ECHO_GPIO_PIN;                                   
    GPIO_Init(HCSR04_ECHO_GPIO_PORT, &itd);
    
    //AFIO映射中断引脚
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);         //使能AFIO的外设时针
    GPIO_EXTILineConfig(HCSR04_ECHO_EXIT_PORT, HCSR04_ECHO_EXIT_PIN);  //选择外部中断源和中断通道
    
    //EXTI中断配置    
    itd1.EXTI_Line = HCSR04_ECHO_EXIT_LINE;                                  //echo使用的端口4,因此选择4号中断线
    itd1.EXTI_LineCmd = ENABLE;
    itd1.EXTI_Mode = EXTI_Mode_Interrupt;
    itd1.EXTI_Trigger = EXTI_Trigger_Rising_Falling;              //上升沿和下降沿都触发中断
    EXTI_Init(&itd1);
    
    //NVIC分配外部中断的中断优先级
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);             //指定中断分组

    itd2.NVIC_IRQChannel = EXTI4_IRQn;                          //使用的端口4,因此选择这个参数
    itd2.NVIC_IRQChannelCmd = ENABLE;
    itd2.NVIC_IRQChannelPreemptionPriority = 2;                   //抢占优先级
    itd2.NVIC_IRQChannelSubPriority = 2;                          //响应优先级 
    NVIC_Init(&itd2);
    
    //配置定时器
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    itd3.TIM_ClockDivision=TIM_CKD_DIV1;                        //使用时钟分频1
    itd3.TIM_CounterMode=TIM_CounterMode_Up;                    //向上计数
    //72MHz/72/100=1000,每秒定时器计数1000个,因此每个计数为100us
    itd3.TIM_Period=72-1;                                       //预分频系数
    itd3.TIM_Prescaler=100-1;                                   //自动重装器
    itd3.TIM_RepetitionCounter=0;                               //该参数仅给高级定时器使用
    TIM_TimeBaseInit(TIM2,&itd3);
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);                    //使能中断输出信号
    TIM_InternalClockConfig(TIM2);                              //选择内部时钟    
    
    //NVIC分配定时器的中断优先级
    itd4.NVIC_IRQChannel=TIM2_IRQn;                             //指定Tim2的中断通道
    itd4.NVIC_IRQChannelCmd=ENABLE;
    itd4.NVIC_IRQChannelPreemptionPriority=2;                   //抢占优先级
    itd4.NVIC_IRQChannelSubPriority=1;                          //响应优先级
    NVIC_Init(&itd4);
	
	HCSR04_TRIG(Bit_RESET);
	HCSR04_ECHO(Bit_RESET);
}

//定时器中断函数
void TIM2_IRQHandler(void)
{
    if(SET==TIM_GetITStatus(TIM2,TIM_FLAG_Update))
	{
        number++; //每次中断将次数++
        TIM_ClearITPendingBit(TIM2,TIM_FLAG_Update);
    }
}

//外部中断函数
void EXTI4_IRQHandler(void)
{
	static uint8_t flag = 0;				//用于记录中断信号是上升沿还是下降沿
    if(SET == EXTI_GetITStatus(HCSR04_ECHO_EXIT_LINE))
	{
        if(flag == 0){
            //上升沿即回响电平开始,打开计数器
            number = 0;
			flag = 1;
            TIM_SetCounter(TIM2,0);
            TIM_Cmd(TIM2,ENABLE);
            
        }else{
            //下降沿即回响电平结束,统计高电平持续时长
            TIM_Cmd(TIM2,DISABLE);
            flag = 0;
            times = number*100+TIM_GetCounter(TIM2);  //得到回响的高电平持续的us
        }
        EXTI_ClearITPendingBit(HCSR04_ECHO_EXIT_LINE);
    }
}

void hcsr04_handle(void)
{
	uint8_t i;
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//开启串口接受中断
	distance = 0;
	times = 0;
	for(i=0; i<2; ++i){              //每次取2次测距数据,取平均值减少误差
		HCSR04_TRIG(Bit_SET);
		delay_us(20);                   //根据说明书,需要提供至少10us的高电平
		HCSR04_TRIG(Bit_RESET);
		delay_ms(65);                   //根据说明书,每个周期至少需要等待60ms
		distance+=(times/5.8);          //根据说明书提供的公式,获取单位为mm的距离
		
	}
	distance = distance/2; //因为发送遇到障碍物到接收，需要经历两倍距离
	distance = distance/10.0; //除以10得到cm
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
	if(distance >= HCSR04_DIS_THR) 
	{
		distance = HCSR04_DIS_THR;
	}
	else
	{
		distance = HCSR04_DIS_THR - distance;
	}
	//printf("distance = %d\r\n",distance);
	
	
}


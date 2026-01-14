#include "adc.h"
#include "flag.h"
#include "delay.h"

#define BAT_ADC_CHANNEL		4 //电池ADC采集通道
#define LDR_ADC_CHANNEL		5 //光敏ADC采集通道
#define YW_ADC_CHANNEL		7 //液位ADC采集通道

#define LDR_ADC_THR			2000 //光敏阈值，0-4095

#define YW_ADC_MIN			0 		//0V,空液位对应的ADC值
#define YW_ADC_MAX			1240    //1V,满液位对应的ADC值
#define YW_LVL_MIN			0.0f 	//0mm,最小液位高度
#define YW_LVL_MAX			90.0f   //90mm,最大液位高度

uint16_t ldr_value = 0; //光敏采集值
uint16_t yw_value = 0;  //液位采集值
	  		   																   
void  adc_init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE );	  //使能ADC1通道时钟
 
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	//模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

}				  
//获得ADC值
//ch:通道值
u16 get_adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_7Cycles5 );	//ADC1,ADC通道,采样时间为7.5周期	  			     
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能		 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	
	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

u16 get_adc_average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=get_adc(ch);
		delay_us(100);
	}
	return temp_val/times;
} 	 

#define NUM 10

uint16_t get_adc_filter(uint8_t ch) //中位值平均值滤波
{
    uint16_t temp_buf[NUM], max, min;
    uint32_t sum = 0;
    uint8_t t;

    for(t = 0; t < NUM; t++)
    {
        temp_buf[t] = get_adc(ch);
        sum += temp_buf[t];
        delay_us(10);
    }

    max = min = temp_buf[0];
    for(t = 1; t < NUM; t++)
    {
        if(temp_buf[t] > max) max = temp_buf[t]; 
        if(temp_buf[t] < min) min = temp_buf[t];
    }

    sum = sum - max - min; //去掉一个最大值一个最小值
    
    return sum / (NUM - 2);
}

/* 获取液位高度函数
函数参数：
adc_value: 当前读取的ADC值
adc_min:   空液位对应的ADC值
adc_max:   满液位对应的ADC值
level_min: 最小液位高度（单位：mm）
level_max: 最大液位高度（单位：mm）
clamp:     是否限制输出范围（1为限制，0为不限制）
*/
float get_liquid_level(int adc_value, 
                      int adc_min, 
                      int adc_max,
                      float level_min,
                      float level_max,
                      int clamp) 
{
    // 参数校验
    if (adc_min == adc_max) {
        return (level_min + level_max) / 2.0f;  // 如果量程为0，返回中间值
    }

    // 计算量程
    int adc_range = adc_max - adc_min;
    float level_range = level_max - level_min;

    // 处理超出量程的情况
    if (adc_value < adc_min) {
        return clamp ? level_min : (level_min + (adc_value - adc_min) / (float)adc_range * level_range);
    }
    if (adc_value > adc_max) {
        return clamp ? level_max : (level_max + (adc_value - adc_max) / (float)adc_range * level_range);
    }

    // 计算液位高度
    float normalized_adc = (adc_value - adc_min) / (float)adc_range;
    float liquid_level = level_min + normalized_adc * level_range;

    return liquid_level;
}

void adc_handle(void)
{
	float yw_level;
	
	ldr_value = get_adc_average(LDR_ADC_CHANNEL, 5);
	if(ldr_value > LDR_ADC_THR) flag_set(&sys_flag, SYS_LDR_FLAG); //光敏强度低于阈值
	else flag_clr(&sys_flag, SYS_LDR_FLAG);
	
	yw_value = get_adc_average(YW_ADC_CHANNEL, 5);
	yw_level = get_liquid_level(yw_value, YW_ADC_MIN, YW_ADC_MAX, YW_LVL_MIN, YW_LVL_MAX, 1);
	
	
}

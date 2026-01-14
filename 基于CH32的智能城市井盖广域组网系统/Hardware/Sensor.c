#include "sys.h"

uint16_t i=1;

/*
参数  : 无
函数体: 蜂鸣器初始化函数
返回值: 无
*/
void BEEP_Init(void)
{
	RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOB,ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

/*
参数  : 无
函数体: 蜂鸣器响的函数
返回值: 无
*/
void BEEP_Up(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

/*
参数  : 无
函数体: 蜂鸣器不响的函数
返回值: 无
*/
void BEEP_Down(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
}

/*
参数  : 无
函数体: 界面显示函数
返回值: 无
*/
void Dnterface_Display(void)
{
	if(Warning_BenDi==0)
	{   
		if(LCD_Warn_Flag==2) {LCD_Warn_Flag=0;LCD_Fill (0, 0, LCD_W, LCD_H, WHITE);} 
		else 
		{
			LCD_Printf(35, 12, DARKBLUE, WHITE, 24, "智能井盖警报系统");
			LCD_Printf(35, 48, DARKBLUE, WHITE, 24, "倾斜角度:%3.2f°" , Qingxiejiao);    
			LCD_Printf(35, 48+24, DARKBLUE, WHITE, 24, "MQ_4_Value:%3.2f%%" , MQ4);
			LCD_Printf(35, 48+24*2, DARKBLUE, WHITE, 24, "Battery_Value:%3d%%" , Battery_level);
			LCD_Printf(35, 48+24*3, DARKBLUE, WHITE, 24, "Water_State:%s" , (Water_Value * 10 / 4096.0 < 1.0f)>0?Warn:Safety);
			LCD_Printf(35, 48+24*4, DARKBLUE, WHITE, 24, "HALL_State:%s" , HALL_State>0?Warn:Safety);  
		}
	} 
	else if(Warning_BenDi==1)
	{
		if(LCD_Warn_Flag==1) {LCD_Warn_Flag=0;LCD_Fill (0, 0, LCD_W, LCD_H, BLACK);} 
		else  
		{
			LCD_Printf(35, 12, RED, BLACK, 24, "智能井盖警报系统");
			LCD_Printf(35, 48, RED, BLACK, 24, "倾斜角度:%3.2f°" , Qingxiejiao);    
			LCD_Printf(35, 48+24, RED, BLACK, 24, "MQ_4_Value:%3.2f%%" , MQ4);
			LCD_Printf(35, 48+24*2, RED, BLACK, 24, "Battery_Value:%3d%%" , Battery_level);
			LCD_Printf(35, 48+24*3, RED, BLACK, 24, "Water_State:%s" , (Water_Value * 10 / 4096.0 < 1.0f)>0?Warn:Safety);
			LCD_Printf(35, 48+24*4, RED, BLACK, 24, "HALL_State:%s" , HALL_State>0?Warn:Safety);  
		}
	}	
}

/*
参数  : 无
函数体: 界面显示函数
返回值: 无
*/
void Update_Display(void)
{
	if(Qingxiejiao<10)
	{
		LCD_ShowFloatNum(144,48,Qingxiejiao,1,2,DARKBLUE,WHITE,24);
		LCD_ShowChar(144+48,48,' ',DARKBLUE,WHITE,24,0);
	}
	else
	{
		LCD_ShowFloatNum(144,48,Qingxiejiao,2,2,DARKBLUE,WHITE,24);
	}
    
	if(MQ4<10)
	{		
		LCD_ShowFloatNum(168,48+24,MQ4,1,2,DARKBLUE,WHITE,24);
		LCD_ShowChar(168+48,48+24,'%',DARKBLUE,WHITE,24,0);
		LCD_ShowChar(168+60,48+24,' ',DARKBLUE,WHITE,24,0);
	}
	else
	{
		LCD_ShowFloatNum(168,48+24,MQ4,2,2,DARKBLUE,WHITE,24);
		LCD_ShowChar(168+60,48+24,'%',DARKBLUE,WHITE,24,0);
	}

	if((Water_Value * 10 / 4096.0 < 1.0f)>0)
	{
		LCD_ShowString(179,48+24*3,"Warn  ",DARKBLUE,WHITE,24);
	}
	else
	{
		LCD_ShowString(179,48+24*3,"Safety",DARKBLUE,WHITE,24);
	}
	
	if(HALL_State!=0)
	{
		LCD_ShowString(167,48+24*4,"Warn  ",DARKBLUE,WHITE,24);
	}
	else
	{
		LCD_ShowString(167,48+24*4,"Safety",DARKBLUE,WHITE,24);
	}

    LCD_ShowNum(203,48+48,Battery_level,3,DARKBLUE,WHITE,24);
}
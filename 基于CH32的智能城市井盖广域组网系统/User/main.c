#include "sys.h"

uint16_t Arverge_MQ4 = 0, Arverge_WATER = 0, Arverge_Bat = 0;  // 甲烷，水位，电池电量平均值
float MQ4 = 0;                                                 // 甲烷的具体值

uint8_t V_Bat_Sw = 1;                                          // 电池是否检测变量

uint16_t Rtc_Clock = 0;                                        // 闹钟时间

uint16_t Time_dly = 0;                                         // 定时器延时变量

uint8_t Usart1_IDLE_Flag = 0, key_flag = 0, EC_01G_GPS_GetFlag = 0, GPS_OK_Flag = 0, Send_OK_Flag = 0, ADC_Flag = 0;

uint8_t LCD_Warn_Flag=0;
uint8_t Last_Warning_BenDi = 0;
uint8_t Warning_BenDi = 0;              // 出现报警的标志位
uint8_t Warning_Send = 1;               // 报警时只发送异常一次
uint8_t Warning_Start=0;                // 报警正式上报
uint8_t Warning_Yun=1;                  // 云端控制的标志位
uint8_t BenDi_GO=1;                     // 辅助云端控制的标志位
uint8_t Fast_Recover_flag=0;	        // 快速恢复标志位
uint16_t BenDi_Go_flag_dly=0;           // 本地发送的延时标志位

uint8_t Usart2_last_Length=0,Usart2_Length=0;
uint8_t Usart1_last_Length=0,Usart1_Length=0;

uint8_t HALL_Flag=0;                    // 霍尔感应标志位
uint16_t HALL_Flag_dly=0;               // 霍尔消抖延时

int main (void) {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);        // 中断优先级分组
    SystemCoreClockUpdate();                                // 更新系统核心时钟频率变量
    Delay_Init();                                           // 延时初始化
    BEEP_Init();                                            // 蜂鸣器初始化
    Buzzer_ON_Twice();                                      // 上电提示音
    LCD_Init();                                             // LCD初始化
    LCD_Fill (0, 0, LCD_W, LCD_H, WHITE);                   // 将LCD填充为白色
    LCD_Printf(52, 0, DARKBLUE, WHITE, 24, "智能井盖警报系统");
    LCD_Printf(0, 72, DARKBLUE, WHITE, 32, "芯火征途,你好欸哎");
    LCD_Printf(48,104, DARKBLUE, WHITE,24,"2025,嵌入式大赛");
    DMA_Usart1_Init (115200);                               // 串口1初始化
    DMA_Usart2_Init (9600);                                 // 串口2初始化
    
    ALLADC_Init();                                          // 所有ADC的初始化
    while (ICM42688_Init() != 0) Delay_Ms (100);            // ICM-42866初始化

    EC_01G_Connect();                                       // 通过EC_01_G连接物联网平台
    if (strstr (Usart2_Rx_Data, successfully_Connected) != 0)
        Buzzer_ON_Twice();
    EC_01G_Topic_Init();                                    // EC_01_G订阅上报和下发主题

    ICM_GetAngle();                                         // 获取角度
    Get_ADC_value();                                        // 获取传感器的值

    LCD_Fill (0, 0, LCD_W, LCD_H, WHITE);	
    LCD_Printf(35, 12, DARKBLUE, WHITE, 24, "智能井盖警报系统");
    LCD_Printf(35, 48, DARKBLUE, WHITE, 24, "倾斜角度:%3.2f°" , Qingxiejiao);
    LCD_Printf(35, 48+24, DARKBLUE, WHITE, 24, "MQ_4_Value:%3.2f%%" , MQ4);
    LCD_Printf(35, 48+24*2, DARKBLUE, WHITE, 24, "Battery_Value:%3d%%" , Battery_level);
    LCD_Printf(35, 48+24*3, DARKBLUE, WHITE, 24, "Water_State:%s" , (Water_Value * 10 / 4096.0 < 1.0f)>0?Warn:Safety);
    LCD_Printf(35, 48+24*4, DARKBLUE, WHITE, 24, "HALL_State:%s" , HALL_State>0?Warn:Safety);
    LCD_Printf(10, 48+24*5+12, DARKBLUE, WHITE, 32, "芯火征途,你好欸哎");

    Sensor_GPIO_Init();
    while (1) 
    {
        // 获取欧拉角并计算得到倾斜角
        ICM_GetAngle();
        calculate_tilt_angle (Q_ANGLE_X, Q_ANGLE_Y);

        // 获取传感器的值
        Get_ADC_value();

        //更新数值
        Update_Display();
    }
}

void USART1_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void USART1_IRQHandler (void)                                                       // 优先级 0 0
{
    if (USART_GetITStatus (USART1, USART_IT_IDLE) == SET) {
        Usart1_IDLE_Flag = 1;
        USART_GetITStatus (USART1, USART_IT_IDLE);
        USART_ReceiveData (USART1);
    }
}

void USART2_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void USART2_IRQHandler (void)                                                       // 优先级 0 1
{
    if (USART_GetITStatus (USART2, USART_IT_IDLE) == SET) {
        EC_01G_IDLE_Flag = 1;
        Usart1_Send_String(Usart2_Rx_Data);
        USART_GetITStatus (USART2, USART_IT_IDLE);
        USART_ReceiveData (USART2);
    }
}

void TIM2_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void TIM2_IRQHandler (void)                                                         // 优先级 0 2
{
    if (TIM_GetITStatus (TIM2, TIM_IT_Update) != RESET) {
        // 延时变量自加
        Time_dly++;

        // 20s检测一次GPS获取状态
		if (GPS_OK_Flag && Time_dly % 2000 == 0) 
        {
			GPS_OK_Flag = 0;
			if(Warning_BenDi&&!BenDi_GO) 
			{
				EC_01G_Send_Data (Water_Value * 10 / 4096.0 < 1.0f, Battery_level, MQ4, Qingxiejiao, 1000, &Send_OK_Flag, 0);
			}
			else EC_01G_Send_Data (Water_Value * 10 / 4096.0 < 1.0f, Battery_level, MQ4, Qingxiejiao, 1000, &Send_OK_Flag, Warning_BenDi);
		}
		if (!GPS_OK_Flag) 
        {
			EC_01G_GPS_Get (&GPS_OK_Flag);
			// Delay_Ms (2);
		} 
        else if (EC_01G_IDLE_Flag) 
        {
			EC_01G_Key_Capture(&Warning_Yun);
			for (uint16_t i = 0; i < 400; i++) Usart2_Rx_Data[i] = 0;
			EC_01G_IDLE_Flag = 0;
			DMA_Usart2_Start();
		}

        // 永磁体触发式霍尔传感系统判断
        if (GPIO_ReadInputDataBit (GPIOA, GPIO_Pin_15) == SET) HALL_Flag = 1;
        else HALL_Flag = 0;
        // 如果井盖从井体上方拿开超过5S，判断为正式报警，未超过5S又复原了，则算作异常扰动，不必报警
        if(HALL_Flag==1)
        {
            if(++HALL_Flag_dly>=500)
            {
                HALL_State=1;
            }
        }
        else
        {
            HALL_State=0;
            HALL_Flag_dly=0;
        }
        

        Last_Warning_BenDi=Warning_BenDi;
        if (Battery_level <= 5 || MQ4 >= 80 || Qingxiejiao >= 30 || Water_Value * 10 / 4096.0 < 1.0f || HALL_State == 1) 
        {
            Warning_BenDi = 1;
            if (Warning_Send == 1) 
            {
                EC_01G_Send_Data (Water_Value * 10 / 4096.0 < 1.0f, Battery_level, MQ4, Qingxiejiao, 1000, &Send_OK_Flag, Warning_BenDi);
                Warning_Send = 0;
                Fast_Recover_flag=1;
            }
        }
        else 
        {
            Warning_BenDi = 0;
            Warning_Send = 1;
        }

        // 5秒阈值判断是否意外扰动报警
		if(Fast_Recover_flag==1) BenDi_Go_flag_dly++;
		if(BenDi_Go_flag_dly>=500)
		{
			BenDi_Go_flag_dly=510;
			if(Warning_BenDi==0)
			{
				EC_01G_Send_Data (Water_Value * 10 / 4096.0 < 1.0f, Battery_level, MQ4, Qingxiejiao, 1000, &Send_OK_Flag, Warning_BenDi);
				Fast_Recover_flag=0;
			}
		}
        if(Fast_Recover_flag==0) BenDi_Go_flag_dly=0;
		
        // 云端控制蜂鸣器关闭
		if(Warning_Yun)
		{
			if(Warning_BenDi&&BenDi_GO) 
			{
				BEEP_Up();
			}
			else 
			{
				if(!Warning_BenDi) BenDi_GO=1;
				BEEP_Down();
			}
		}
		else if(!Warning_Yun)
		{
			BEEP_Down();
			Warning_Yun=1;
			BenDi_GO=0;
		}
        /*Warn_Show*/        
        if(Last_Warning_BenDi!=Warning_BenDi)
        {
            if(Warning_BenDi) LCD_Warn_Flag=1;
            else LCD_Warn_Flag=2;  
        }
        TIM_ClearITPendingBit (TIM2, TIM_IT_Update);
    }
}

/*EC-01G接收数据*/
		// if(EC_01G_IDLE_Flag)
		// {
		// 	Usart2_last_Length=Usart2_Length;
		// 	Usart2_Length=400-DMA_GetCurrDataCounter(DMA1_Channel6);
		// 	if(Usart2_last_Length>Usart2_Length)
		// 	{
		// 		for(;Usart2_last_Length>Usart2_Length;Usart2_last_Length--) Usart2_Rx_Data[Usart2_last_Length-1]=0;
		// 	}
		// 	Usart1_Send_String(Usart2_Rx_Data);
		// 	DMA_Usart2_Start();
		// 	EC_01G_IDLE_Flag=0;
		// }
/*STM32接收数据*/
		// if(Usart1_IDLE_Flag)
		// {
		// 	Usart1_last_Length=Usart1_Length;
		// 	Usart1_Length=400-DMA_GetCurrDataCounter(DMA1_Channel5);
		// 	if(Usart1_last_Length>Usart1_Length)
		// 	{
		// 		for(;Usart1_last_Length>Usart1_Length;Usart1_last_Length--) Usart1_Rx_Data[Usart1_last_Length-1]=0;
		// 	}
		// 	Usart2_Send_String(Usart1_Rx_Data);
		// 	DMA_Usart1_Start();
		// 	Usart1_IDLE_Flag=0;
		// }

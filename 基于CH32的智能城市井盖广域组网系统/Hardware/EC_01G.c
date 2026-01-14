#include "sys.h"

char *ON="ON";												// ON
char *OFF="OFF";											// OFF
char *OK_Ack="OK";											// OK应答
char *successfully_Open_Channel="+ECMTOPEN: 0,0";			// 成功打开连接通道
char *successfully_Connected="+ECMTCONN: 0,0,0";			// 成功连接
char *successfully_Disconnected="+ECMTDISC: 0,0";			// 成功断开连接
uint8_t EC_01G_IDLE_Flag=0;									// EC_01_G空闲标志位
char GPS_Time[10]={0};										// GPS时间
char GPS_Text[1]={0};										// 发送数据是否正确
char GPS_Loaction[2][15]={{0},{0}};							// GPS经纬度
char GPS_Speed[5]={0};										// GPS移动速度
char GPS_Date[10]={0};										// GPS其他数据
uint32_t Time,Date;
float N,E;
uint8_t HALL_State=0;										// 霍尔传感器的状态
uint8_t Key_Capture=0;

EC01G_Time_t* Time_Data;

/*
参数  : 无
函数体:	EC_01_G复位引脚初始化1
返回值:	无
*/
void EC_01G_Reset_GPIO_Init(void)
{
	RCC_PB2PeriphClockCmd(RCC_PB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);	
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}
	
/*
参数  : 无
函数体:	EC_01_G复位函数
返回值:	无
*/
void EC_01G_Reset(void)
{
	//复位引脚初始化
	EC_01G_Reset_GPIO_Init();

	//复位
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	Delay_Ms(20);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);

	//判断有没有发送来数据，有就清除
	while(EC_01G_IDLE_Flag==0) 
	{
		EC_01G_IDLE_Flag=0;
		DMA_Usart2_Start();	
	}
}

/*
参数  : cmd			：发送的数据
		Wait_Times  ：超时等待值
		over_Time	：发送完数据等待的安全时间，防止发送太快，物联网平台不能接收
函数体:	EC_01_G发送数据函数
返回值:	无
*/
uint8_t EC_Send_Cmd(char *cmd, uint16_t Wait_Times, char *Ack, uint16_t over_Time)
{
	Usart2_Send_String(cmd);
	if(EC_01G_IDLE_Flag==1)
	{		
		for(;strstr(Usart2_Rx_Data, Ack)==NULL;Wait_Times--)
		{
			Delay_Ms(1);
			if(EC_01G_IDLE_Flag) 
			{
				EC_01G_IDLE_Flag=0;
				DMA_Usart2_Start();
			}
			if(Wait_Times==0) break;
		}
	}
	Delay_Ms(over_Time);
	EC_01G_IDLE_Flag=0;
	DMA_Usart2_Start();
	
	return !!(Wait_Times);
}

/*
参数  : 无
函数体:	EC_01_G连接物联网平台的函数
返回值:	无
*/
void EC_01G_Connect(void)
{
	Time_Data=(EC01G_Time_t*)malloc(sizeof(EC01G_Time_t));

	EC_Send_Cmd("AT\r\n", 5000, OK_Ack, 500);
	EC_Send_Cmd("AT+CFUN=1\r\n", 5000, OK_Ack, 500);
	EC_Send_Cmd("AT+CREG=1\r\n", 5000, OK_Ack, 500);
	EC_Send_Cmd("AT+GPSRD=5\r\n", 5000, OK_Ack, 1000);
	EC_Send_Cmd("AT+GPS=1\r\n", 5000, OK_Ack, 1000);
	EC_Send_Cmd("AT+ECMTCFG =\"cloud\",0,4,1\r\n", 5000, OK_Ack, 1000);
/*1*/
	EC_Send_Cmd("AT+ECMTOPEN=0,\"sh-1-mqtt.iot-api.com\",1883\r\n", 5000, successfully_Open_Channel, 1000);
	EC_Send_Cmd("AT+ECMTCONN=0,\" \",\"yurx8avs8kt0tu1v\",\"Jm43QH0J4M\"\r\n", 5000, successfully_Connected, 1000);
/*2*/
	// EC_Send_Cmd("AT+ECMTOPEN=0,\"sh-1-mqtt.iot-api.com\",1883\r\n", 5000, successfully_Open_Channel, 1000);
	// EC_Send_Cmd("AT+ECMTCONN=0,\" \",\"gv0kdm6c9bpdfbjf\",\"Jm43QH0J4M\"\r\n", 5000, successfully_Connected, 1000);
}

/*
参数  : 无
函数体:	EC_01_G订阅上报和下发主题函数
返回值:	无
*/
void EC_01G_Topic_Init(void)
{
	EC_Send_Cmd("AT+ECMTSUB=0,1,\"attributes\",1\r\n", 5000, OK_Ack, 1500);
	EC_Send_Cmd("AT+ECMTSUB=0,1,\"attributes/response\",1\r\n", 5000, OK_Ack, 1500);
	EC_Send_Cmd("AT+ECMTSUB=0,1,\"attributes/push\",1\r\n", 5000, OK_Ack, 1500);
}

/*
参数  : WaterState：水位状态
		VBAT	  ：电池电量
		Gas		  ：甲烷浓度
		Angle	  ：倾斜角
		Wait_Times：超时等待
		Send_OK_Flag：发送成功标志位
		Key		  ：本地蜂鸣器报警标志位
函数体:	EC_01_G上报数据函数
返回值:	无
*/
void EC_01G_Send_Data(int WaterState,uint8_t VBAT, float Gas, float Angle, uint16_t Wait_Times, uint8_t *Send_OK_Flag,uint8_t Key)
{
	EC_01G_IDLE_Flag=0;
	DMA_Usart2_Start();
	EC_01G_GPS_Handle();
	EC01G_Printf("AT+ECMTPUB=0,1,1,0,\"attributes\",\"{\"GPSTime\":\"%04d年%02d月%02d日%02d:%02d:%02d\",\"location\":{\"lat\":%3.6f,\"lng\":%3.6f},\"GPSState\":\"%s\",\"WaterState\":\"%s\",\"Gas\":%3.2f,\"degrees\":%3.2f,\"elec\":%d,\"key\":%d}\"\r\n",
		 2000+Time_Data->year, Time_Data->month, Time_Data->day, Time_Data->hour, Time_Data->minute, Time_Data->second, N, E, HALL_State?ON:OFF, WaterState?ON:OFF, Gas, Angle, VBAT, Key);
	for(;strstr(Usart2_Rx_Data, "\"result\":")==NULL;Wait_Times--)
	{
		Delay_Ms(1);
		if(EC_01G_IDLE_Flag) 
		{
			EC_01G_IDLE_Flag=0;
			DMA_Usart2_Start();
		}
		if(Wait_Times==0) break;
	}
	if(strstr(Usart2_Rx_Data, "\"errcode\":403")!=NULL||Wait_Times==0) *Send_OK_Flag=false;
	else *Send_OK_Flag=true;
}

/*
参数  : 云端下发属性标志位
函数体:	GPS云端控制蜂鸣器函数
返回值:	无
*/
void EC_01G_Key_Capture(uint8_t *Warning_Yun)
{
	char *i=strstr(Usart2_Rx_Data, "+ECMTRECV: 0,0,\"attributes/push\",{\"key\":");
	if(i!=NULL)
	{
		if(strstr(i, "\"key\":1")!=NULL)	
		{
			*Warning_Yun= true;
			Delay_Ms(5);
		}
		if(strstr(i, "\"key\":0")!=NULL) 	
		{
			*Warning_Yun= false;
			Delay_Ms(5);
		}

	}
}

/*
参数  : GPS数据是否正确的标志位
函数体:	获取GPS数据的函数
返回值:	无
*/
void EC_01G_GPS_Get(uint8_t *GPS_State)
{
	if(EC_01G_IDLE_Flag)
	{
		EC_01G_IDLE_Flag=0;
		DMA_Usart2_Start();
		EC_01G_Key_Capture(&Warning_Yun);	
		char *i=strstr(Usart2_Rx_Data, "$GNRMC");
		if(i!=NULL)
		{
			EC_01G_GPS_DataFree();
			uint8_t cnt=0,length=0;
			while(*i!='\n')
			{
				if(*i==',') {length=0;cnt++;}				
				i++;
				if(*i!=',')
				{
					switch (cnt)
					{
						case 1 : GPS_Time[length]=*i; length++; break;			//GPS_Time
						case 2 : GPS_Text[length]=*i; length++; break;			//GPS_Text
						case 3 : GPS_Loaction[0][length]=*i; length++; break;	//GPS_Loaction
						case 5 : GPS_Loaction[1][length]=*i; length++; break;	//GPS_Loaction
						case 7 : GPS_Speed[length]=*i; length++; break;			//GPS_Speed
						case 9 : GPS_Date[length]=*i; length++; break;			//GPS_Date
						default: break;
					}
				}
			}
			if(*GPS_Text=='A') *GPS_State=true;
			else if(*GPS_Text=='V') *GPS_State=false;
		}
		for(uint16_t i=0;i<100;i++)
		{
			Usart2_Rx_Data[i]=0;
		}
	}	
}

/*
参数  : 无
函数体:	EC_01G，GPS数据释放函数：把GPS的数组中的数据释放
返回值:	无
*/
void EC_01G_GPS_DataFree(void)
{
	for(char i=0,length=0;i<6;length=0,i++)
	switch(i)  
	{
		case 0:while(GPS_Time[length]!='\0') {GPS_Time[length]=0;length++;}break;
		case 1:while(GPS_Text[length]!='\0') {GPS_Text[length]=0;length++;}break;
		case 2:while(GPS_Loaction[0][length]!='\0') {GPS_Loaction[0][length]=0;length++;}break;
		case 3:while(GPS_Loaction[1][length]!='\0') {GPS_Loaction[1][length]=0;length++;}break;
		case 4:while(GPS_Speed[length]!='\0') {GPS_Speed[length]=0;length++;}break;
		case 5:while(GPS_Date[length]!='\0') {GPS_Date[length]=0;length++;}break;
	}
}

/*
参数  : 无
函数体:	EC_01G，GPS数据处理函数
返回值:	无
*/
void EC_01G_GPS_Handle(void)
{
	//通过atoi函数解析GPS_Data，将字符串类型转换为整形
	Date=atoi(GPS_Date);

	//获取GPS发送数据的时间
	Time=atoi(GPS_Time)+80000;
	if(Time/10000>=24) Time-=240000;

	//解析年月日
	Time_Data->year=Date%100;
	Time_Data->month=Date/100%100;
	Time_Data->day=Date/10000%100;

	//解析时分秒
	Time_Data->hour=Time/10000%100;
	Time_Data->minute=Time/100%100;
	Time_Data->second=Time%100;

	//解析经纬度
	N=atof(GPS_Loaction[0]);
	E=atof(GPS_Loaction[1]);
	N/=100.0;
	N=(uint32_t)N+(N-(uint32_t)N)/0.6;
	E/=100.0;
	E=(uint32_t)E+(E-(uint32_t)E)/0.6;
}



#include "ec01g.h"	
#include "delay.h"
#include "hall.h"
#include "beep.h"
#include "gps.h"
#include "timer.h" 
#include "icm42688.h"
#include "hcsr04.h"
#include "stdio.h"	
#include <string.h>

#define MAX_RETRIES 3
#define TIMEOUT 4000 // 超时时间（单位：1毫秒）

volatile u8 usart2_res = 0,usart2_flag = 0;

char *ec_str1 = "AT+ECMTPUB=0,0,0,0,\"/sys/k0iv3mk6iEM/Cover/thing/event/property/post\",\"{\"method\":\"thing.service.property.set\",\"id\":\"259061280\",\"params\":{";
char *ec_str2 = "},\"version\":\"1.0.0\"}\"\r\n\0";
	
void ec01g_init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能USART2，GPIOA时钟

	//USART2_TX   GPIOA.2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.2

	//USART2_RX	  GPIOA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.3  

	//Usart3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART2, &USART_InitStructure); //初始化串口3
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
	USART_Cmd(USART2, ENABLE);                    //使能串口3
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				 //RST-->PC9 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC9
	GPIO_SetBits(GPIOC,GPIO_Pin_13);						 //PC13 输出高

}

void ec01g_rst(void)
{
	EC_RST = 0;
	delay_ms(500);
	EC_RST = 1;
	delay_ms(500);
}

// AT指令列表
const char *at_commands[] = {
    "AT\r\n",// 测试AT指令
    "AT+CFUN=1\r\n",  //关闭飞行模式
    "AT+ECMTCFG=\"cloud\",0,2,1\r\n", //配置平台为阿里物联网
    "AT+ECMTCFG=\"aliauth\",0 ,\"k0iv3mk6iEM\",\"Cover\",\"1ca4350cb7b8752792ab71716c66e216\"\r\n",    //分别写入刚才我们生成测试设备的设备证书
	"AT+ECMTOPEN=0, \"k0iv3mk6iEM.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883\r\n",//建立 tcp
	"AT+ECMTCONN=0,\"12345\"\r\n",//创建 mqtt
	"AT+GPS=1\r\n", //开启GPS
	"AT+LOCATION=1\r\n", //获取定位数据
	"AT+GPSRD=1\r\n",  //设置NMEA输出时间间隔
	"AT+ECLEDMODE=1\r\n",  //开启指示灯模式
    // 添加其他必要的AT指令
};

void usart_send_string(USART_TypeDef* USARTx, const char* str) 
{
    while (*str) 
	{
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
        USART_SendData(USARTx, *str++);
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
    }
}

int usart_receive_string(char* buffer, int bufferSize)
{
    int received = 0;
	char c;
		
    uint32_t start = get_tick_count();
    while ((get_tick_count() - start) < TIMEOUT) 
	{
        if (usart2_flag == 1) 
		{
			usart2_flag = 0;
            c = usart2_res;
            buffer[received++] = c;
            //if (received >= bufferSize - 1 || c == 'K') 
			if(received > 1)
			{
				if ((received >= bufferSize - 1) || ((buffer[received-2] == 'O') && (buffer[received-1] == 'K'))) 
				{
					break;
				}
			}
        }
    }
    buffer[received] = '\0'; // 确保字符串以null终止
    return received; // 返回接收到的字符数
}

void ec01g_handle(void)
{
	char response[128];
	int i;

	for (i = 0; i < sizeof(at_commands) / sizeof(at_commands[0]); ++i) 
	{
        int retries = 0;
        while (retries < MAX_RETRIES) 
		{
            usart_send_string(USART2, at_commands[i]);
            if (usart_receive_string(response, sizeof(response)) > 0) 
			{
                if (strstr(response, "OK")) 
				{
					usart_send_string(USART1, "Ec-01f AT Init Ok!\r\n");					
					if(i >= 3) delay_ms(100);
                    break; // 收到“OK”，继续下一条指令
                }
				
            }
			memset(response,0,sizeof(response));
			usart2_flag = 0;
			usart_send_string(USART1, at_commands[i]);
            retries++;
        }
        if (retries == MAX_RETRIES) {
            // 处理错误...
			usart_send_string(USART1, "Ec-01f AT Init Fail!\r\n");
			break;
        }
	}
	printf("i = %d, p = %d\r\n",i, sizeof(at_commands) / sizeof(at_commands[0]));
	if(i == (sizeof(at_commands) / sizeof(at_commands[0])))
	{		
		BEEP(Bit_RESET);delay_ms(100);
		BEEP(Bit_SET);delay_ms(100);
		BEEP(Bit_RESET);delay_ms(100);
		BEEP(Bit_SET);delay_ms(100);
		BEEP(Bit_RESET);delay_ms(100);
	}
}

void USART2_IRQHandler(void) //串口2中断服务程序
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收中断
	{
		usart2_flag = 1;
		usart2_res = USART_ReceiveData(USART2); //读取接收到的数据
		if ( (usart2_res == '\n') || (usart2_rx_count >= (USART2_MAX_LEN - 1)) ) 
		{
            usart2_rx_buf[usart2_rx_count] = '\0';
            usart2_rx_count = 0;
			// 检查是否以$GNRMC开头
			if ( ((usart2_rx_buf[1] == 'G') && (usart2_rx_buf[2] == 'N') && (usart2_rx_buf[3] == 'R') &&
				(usart2_rx_buf[4] == 'M') && (usart2_rx_buf[5] == 'C')) ||
				 ((usart2_rx_buf[1] == 'B') && (usart2_rx_buf[2] == 'D') && (usart2_rx_buf[3] == 'G') &&
				(usart2_rx_buf[4] == 'S') && (usart2_rx_buf[5] == 'V')) ||
				 ((usart2_rx_buf[1] == 'G') && (usart2_rx_buf[2] == 'N') && (usart2_rx_buf[3] == 'V') &&
				(usart2_rx_buf[4] == 'T') && (usart2_rx_buf[5] == 'G'))	) {
				usart2_rx_complete = 1;
				//USART_SendData(USART1, usart2_rx_buf[0]);
			}
            
        } 
		else 
		{
            usart2_rx_buf[usart2_rx_count++] = usart2_res;
        }	 		
		//USART_SendData(USART1, usart2_res);
	} 
} 

void nb_data_send(void)
{
	char buffer[500]; // 假设缓冲区足够大
	int length = 0;

	length += sprintf(buffer + length, "%s",ec_str1);

	length += sprintf(buffer + length, "%s","\"CoverState\":");
	length += sprintf(buffer + length, "%d,",is_magnet_present);
	
	length += sprintf(buffer + length, "%s","\"TiltValue\":");
	length += sprintf(buffer + length, "%d,",cover_tilt_value);
	
	length += sprintf(buffer + length, "%s","\"TiltValue\":");
	length += sprintf(buffer + length, "%d,",cover_tilt_value);
	
	length += sprintf(buffer + length, "%s","\"Height\":");
	length += sprintf(buffer + length, "%d,",distance);
	
	length += sprintf(buffer + length, "%s","\"GeoLocation\":{");
	length += sprintf(buffer + length, "%s","\"Longitude\":");
	length += sprintf(buffer + length, "%.2f,", reported_longitude);
	length += sprintf(buffer + length, "%s","\"Latitude\":");
	length += sprintf(buffer + length, "%.2f}", reported_latitude);
	
	length += sprintf(buffer + length, "%s",ec_str2);
	
	#if(USART1_DEBUG==0)
	//usart_send_string(USART1, buffer); // 输出
	#endif
	usart_send_string(USART2, buffer); // 输出
}

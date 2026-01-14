#include "gps.h"
#include "flag.h"
#include "delay.h"
#include "usart.h"	 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

volatile char usart2_rx_buf[USART2_MAX_LEN] = {0,};
volatile uint16_t usart2_rx_count = 0, usart2_rx_complete = 0, usart2_rx_timeout = 0;
GPS_INFO GPS;  //GPS信息结构体
char send_data[200];//发送内容
uint8_t send_len= 0;//数据长度

GPS_Data current_gps;
float reported_latitude = 0;     // 纬度（十进制度数）
float reported_longitude = 0;    // 经度（十进制度数）

static uint8_t GetComma(uint8_t num,char* str);
static int Get_Int_Number(char *s);
static double Get_Double_Number(char *s);
static float Get_Float_Number(char *s);
static void UTC2BTC(DATE_TIME *GPS);

void my_memset(char *buf)
{
	uint8_t i;
	for(i = 0; i < sizeof(buf);i++)
		*(buf+i) = 0;
}

//void USART2_IRQHandler(void)                	
//{
//	u8 res;

//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
//	{
//		res = USART_ReceiveData(USART2);
//		if (res == '\n' || usart2_rx_count >= USART2_MAX_LEN - 1) 
//		{
//            usart2_rx_buf[usart2_rx_count] = '\0';
//            usart2_rx_count = 0;
//            usart2_rx_complete = 1;
//        } 
//		else 
//		{
//            usart2_rx_buf[usart2_rx_count++] = res;
//        }	 
//	} 
//} 

//串口发送一个字节
void gps_send_char(u8 data)
{
	while((USART2->SR&0X40)==0); 
	USART2->DR = data;
}

//====================================================================//
// 语法格式: static int Str_To_Int(char *buf)
// 实现功能： 把一个字符串转化成整数
// 参    数：字符串
// 返 回 值：转化后整数值
//====================================================================//
static int Str_To_Int(char *buf)
{
    int rev = 0;
    int dat;
    char *str = buf;
    while(*str != '\0')
    {
        switch(*str)
        {
        case '0':
            dat = 0;
            break;
        case '1':
            dat = 1;
            break;
        case '2':
            dat = 2;
            break;
        case '3':
            dat = 3;
            break;
        case '4':
            dat = 4;
            break;
        case '5':
            dat = 5;
            break;
        case '6':
            dat = 6;
            break;
        case '7':
            dat = 7;
            break;
        case '8':
            dat = 8;
            break;
        case '9':
            dat = 9;
            break;
        }

        rev = rev * 10 + dat;
        str ++;
    }

    return rev;
}

//====================================================================//
// 语法格式: static int Get_Int_Number(char *s)
// 实现功能：把给定字符串第一个逗号之前的字符转化成整型
// 参    数：字符串
// 返 回 值：转化后整数值
//====================================================================//
static int Get_Int_Number(char *s)
{
    char buf[10];
    uint8_t i;
    int rev;
    i=GetComma(1, s);
    i = i - 1;
    strncpy(buf, s, i);
    buf[i] = 0;
    rev=Str_To_Int(buf);
    return rev;
}

//====================================================================//
// 语法格式: static float Str_To_Float(char *buf)
// 实现功能： 把一个字符串转化成浮点数
// 参    数：字符串
// 返 回 值：转化后单精度值
//====================================================================//
static float Str_To_Float(char *buf)
{
    float rev = 0;
    float dat;
    int integer = 1;
    char *str = buf;
    int i;
    while(*str != '\0')
    {
        switch(*str)
        {
        case '0':
            dat = 0;
            break;
        case '1':
            dat = 1;
            break;
        case '2':
            dat = 2;
            break;
        case '3':
            dat = 3;
            break;
        case '4':
            dat = 4;
            break;
        case '5':
            dat = 5;
            break;
        case '6':
            dat = 6;
            break;
        case '7':
            dat = 7;
            break;
        case '8':
            dat = 8;
            break;
        case '9':
            dat = 9;
            break;
        case '.':
            dat = '.';
            break;
        }
        if(dat == '.')
        {
            integer = 0;
            i = 1;
            str ++;
            continue;
        }
        if( integer == 1 )
        {
            rev = rev * 10 + dat;
        }
        else
        {
            rev = rev + dat / (10 * i);
            i = i * 10 ;
        }
        str ++;
    }
    return rev;

}

//====================================================================//
// 语法格式: static float Get_Float_Number(char *s)
// 实现功能： 把给定字符串第一个逗号之前的字符转化成单精度型
// 参    数：字符串
// 返 回 值：转化后单精度值
//====================================================================//
static float Get_Float_Number(char *s)
{
    char buf[10];
    uint8_t i;
    float rev;
    i=GetComma(1, s);
    i = i - 1;
    strncpy(buf, s, i);
    buf[i] = 0;
    rev=Str_To_Float(buf);
    return rev;
}

//====================================================================//
// 语法格式: static double Str_To_Double(char *buf)
// 实现功能： 把一个字符串转化成浮点数
// 参    数：字符串
// 返 回 值：转化后双精度值
//====================================================================//
static double Str_To_Double(char *buf)
{
    double rev = 0;
    double dat;
    int integer = 1;
    char *str = buf;
    int i;
    while(*str != '\0')
    {
        switch(*str)
        {
        case '0':
            dat = 0;
            break;
        case '1':
            dat = 1;
            break;
        case '2':
            dat = 2;
            break;
        case '3':
            dat = 3;
            break;
        case '4':
            dat = 4;
            break;
        case '5':
            dat = 5;
            break;
        case '6':
            dat = 6;
            break;
        case '7':
            dat = 7;
            break;
        case '8':
            dat = 8;
            break;
        case '9':
            dat = 9;
            break;
        case '.':
            dat = '.';
            break;
        }
        if(dat == '.')
        {
            integer = 0;
            i = 1;
            str ++;
            continue;
        }
        if( integer == 1 )
        {
            rev = rev * 10 + dat;
        }
        else
        {
            rev = rev + dat / (10 * i);
            i = i * 10 ;
        }
        str ++;
    }
    return rev;
}

//====================================================================//
// 语法格式: static double Get_Double_Number(char *s)
// 实现功能：把给定字符串第一个逗号之前的字符转化成双精度型
// 参    数：字符串
// 返 回 值：转化后双精度值
//====================================================================//
static double Get_Double_Number(char *s)
{
    char buf[10];
    uint8_t i;
    double rev;
    i=GetComma(1, s);
    i = i - 1;
    strncpy(buf, s, i);
    buf[i] = 0;
    rev=Str_To_Double(buf);
    return rev;
}

//====================================================================//
// 语法格式：static uint8_t GetComma(uint8_t num,char *str)
// 实现功能：计算字符串中各个逗号的位置
// 参    数：查找的逗号是第几个的个数，需要查找的字符串
// 返 回 值：0
//====================================================================//
static uint8_t GetComma(uint8_t num,char *str)
{
    uint8_t i,j = 0;
    int len=strlen(str);

    for(i = 0; i < len; i ++)
    {
        if(str[i] == ',')
            j++;
        if(j == num)
            return i + 1;
    }

    return 0;
}

//====================================================================//
//	语法格式：	Int_To_Str(int x,char *Str)
//	实现功能：	转化整型值为字符串形式
//	参数：		x: 转化的整数
//				Str:转化后的字符串
//	返回值：	无
//====================================================================//
void Int_To_Str(int x,char *Str)
{
    int t;
    char *Ptr,Buf[5];
    int i = 0;
    Ptr = Str;
    if(x < 10)		// 当整数小于10时,转化为"0x"的格式  
    {
        *Ptr ++ = '0';
        *Ptr ++ = x + 0x30;
    }
    else
    {
        while(x > 0)
        {
            t = x % 10;
            x = x / 10;
            Buf[i++] = t + 0x30;	// 通过计算把数字转化成ASCII码形式
        }
        i -- ;
        for(; i >= 0; i --) 		// 将得到的字符串倒序
        {
            *(Ptr++) = Buf[i];
        }
    }
    *Ptr = '\0';
}

// 示例：解析GPRMC语句
// $GPRMC,083559.00,A,2232.8994,N,11355.4673,E,0.004,,240324,,,D*49
void parse_gps_data(char* nmea_data, GPS_Data* gps) 
{
    char* token;
    uint8_t field_index = 0;
    
    // 校验语句前缀
    if(strstr(nmea_data, "$GPRMC") == NULL && 
       strstr(nmea_data, "$BDGSV") == NULL && 
       strstr(nmea_data, "$GNRMC") == NULL&& 
       strstr(nmea_data, "$GNVTG") == NULL&& 
       strstr(nmea_data, "$GPGGA") == NULL) {
        gps->is_valid = 0;
        return;
    }

    // 分割字段
    token = strtok(nmea_data, ",");
    while(token != NULL) {
        switch(field_index) {
            // GPRMC字段索引
            case 2:  // 状态指示（A=有效）
                gps->is_valid = (token[0] == 'A') ? 1 : 0;
                break;
                
            case 3:  // 纬度 ddmm.mmmm
                if(strlen(token) > 0) {
                    // 转换示例：2232.8994 -> 22°32.8994'
                    float lat = atof(token);
                    gps->latitude = (int)(lat/100) + fmod(lat,100)/60;
                }
                break;
                
            case 4:  // 纬度方向
                gps->lat_dir = token[0];
                break;
                
            case 5:  // 经度 dddmm.mmmm
                if(strlen(token) > 0) {
                    float lon = atof(token);
                    gps->longitude = (int)(lon/100) + fmod(lon,100)/60;
                }
                break;
                
            case 6:  // 经度方向
                gps->lon_dir = token[0];
                break;
        }
        token = strtok(NULL, ",");
        field_index++;
    }
}

void gps_handle(void)
{	
	char buffer[500]; // 假设缓冲区足够大
	int length = 0;
	if (usart2_rx_complete)   //如果接收完一行
	{
		parse_gps_data((char *)usart2_rx_buf, &current_gps);
    
		if(current_gps.is_valid) {
			reported_latitude = current_gps.latitude;     // 纬度（十进制度数）
			reported_longitude = current_gps.longitude;    // 经度（十进制度数）
			printf("Latitude: %.6f %c\n", current_gps.latitude, current_gps.lat_dir);
			printf("Longitude: %.6f %c\n", current_gps.longitude, current_gps.lon_dir);		
			length += sprintf(buffer + length, "%s","Your family member fell down!");
			length += sprintf(buffer + length, "Latitude: %.6f %c\n", current_gps.latitude, current_gps.lat_dir);
			length += sprintf(buffer + length, "Longitude: %.6f %c\n", current_gps.longitude, current_gps.lon_dir);
			length += sprintf(buffer + length, "%s","\0");
			#if(GPS_USART1_DEBUG==1)
			UART_PutStr (USART1, (uint8_t *)buffer);
			#endif	
		}			
		usart2_rx_complete  = 0;
	}
}

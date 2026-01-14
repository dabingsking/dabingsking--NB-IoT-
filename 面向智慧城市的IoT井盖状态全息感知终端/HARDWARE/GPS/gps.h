#ifndef __GPS_H
#define __GPS_H

#include "sys.h"
#include <stdio.h>

#define USART2_MAX_LEN	250

typedef struct
{
	int year;  
	int month; 
	int  day;
	int hour;
	int minute;
	int second;
}DATE_TIME;

typedef struct
{
	double  latitude;  //经度
	double  longitude; //纬度
	int     latitude_Degree;	//度
	int		latitude_Cent;		//分
	int   	latitude_Second;    //秒
	int     longitude_Degree;	//度
	int		longitude_Cent;		//分
	int   	longitude_Second;   //秒
	float 	speed;      //速度
	float 	direction;  //航向
	float 	height_ground;    //水平面高度
	float 	height_sea;       //海拔高度
	int     satellite;
	uint8_t NS;
	uint8_t EW;
	DATE_TIME D;
}GPS_INFO;

// 经纬度结构体定义
typedef struct {
    float latitude;     // 纬度（十进制度数）
    float longitude;    // 经度（十进制度数）
    char lat_dir;       // 纬度方向 N/S
    char lon_dir;       // 经度方向 E/W
    uint8_t is_valid;   // 数据有效性标志
} GPS_Data;

extern volatile char usart2_rx_buf[USART2_MAX_LEN];
extern volatile uint16_t usart2_rx_count, usart2_rx_complete, usart2_rx_timeout;
extern GPS_INFO GPS;  //GPS信息结构体
extern GPS_Data current_gps;
extern float reported_latitude;     // 纬度（十进制度数）
extern float reported_longitude;    // 经度（十进制度数）

void gps_handle(void);

#endif

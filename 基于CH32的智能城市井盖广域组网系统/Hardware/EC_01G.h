#ifndef __EC_01G_H
#define __EC_01G_H
#include "debug.h"
extern char *OK_Ack;
extern char *successfully_Open_Channel;
extern char *successfully_Connected;
extern char *successfully_Disconnected;			
extern uint8_t EC_01G_IDLE_Flag;
extern uint8_t Key_Capture;
extern uint8_t HALL_State;

typedef struct{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}EC01G_Time_t;

uint8_t EC_Send_Cmd(char *cmd, uint16_t Wait_Times, char *Ack, uint16_t over_Time);
void EC_01G_Reset(void);
void EC_01G_Connect(void);
void EC_01G_Topic_Init(void);
void EC_01G_Send_Data(int WaterState,uint8_t VBAT, float Gas, float Angle, uint16_t Wait_Times, uint8_t *Send_OK_Flag,uint8_t Key);
void EC_01G_Key_Capture(uint8_t *Warning_flag);
void EC_01G_GPS_Get(uint8_t *GPS_State);
void EC_01G_GPS_Handle(void);
void EC_01G_GPS_DataFree(void);

#endif

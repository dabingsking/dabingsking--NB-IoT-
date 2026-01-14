#ifndef __SYS_H
#define __SYS_H

#include "debug.h"
#include "TFT_LCD_Init.h"
#include "TFT_LCD.h"
#include "EC_01G.h"
#include "Serial.h"
#include "MySPI.h"
#include "ICM42688.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "Angle.h"
#include "Sensor.h"
#include "string.h"
#include "ADC.h"
#include "Timer.h"
#include <stdint.h>
#include <math.h>
#include <time.h>
#include "MyRTC.h"

extern uint16_t Arverge_Bat;
extern float MQ4;

extern uint16_t Arverge_MQ4,Arverge_WATER;

extern uint8_t V_Bat_Sw;

extern uint16_t Rtc_Clock;

extern uint8_t LCD_Warn_Flag;
extern uint8_t Warning_BenDi;
extern uint8_t Last_Warning_BenDi;
extern uint8_t Warning_Yun;


#endif

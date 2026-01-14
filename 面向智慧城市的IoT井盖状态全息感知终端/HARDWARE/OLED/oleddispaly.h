#ifndef _OLEDDISPLAY_H
#define _OLEDDISPLAY_H

#include  "sys.h"

typedef struct{
	uint16_t rst_flag;	
	uint16_t temp;
}sys_data_t;

typedef union{
	uint16_t config_buff[sizeof(sys_data_t)];
	sys_data_t sys_data;
}config_data_u;

extern config_data_u g_config_data;

extern float current_temp,set_temp;
extern uint8_t start_flag; 
extern uint8_t mode;

void power_up_config(void);
void display_init(void);
void oled_handle(void);

#endif

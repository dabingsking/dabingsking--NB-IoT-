#ifndef __TIMER_H
#define __TIMER_H

#include "sys.h"

extern uint8_t flag_1ms,flag_50ms,flag_500ms,flag_1s,flag_5s;

void tim3_init(u16 arr,u16 psc);
uint32_t get_tick_count(void); 
 
#endif

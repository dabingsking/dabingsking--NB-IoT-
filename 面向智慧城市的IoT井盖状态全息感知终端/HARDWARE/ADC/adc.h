#ifndef __ADC_H
#define __ADC_H	

#include "sys.h"

void adc_init(void);
u16  get_adc(u8 ch); 
u16 get_adc_average(u8 ch,u8 times); 
u16 get_adc_filter(u8 ch);
void adc_handle(void);

#endif 

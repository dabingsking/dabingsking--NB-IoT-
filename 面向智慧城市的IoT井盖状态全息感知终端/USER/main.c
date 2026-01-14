#include "delay.h"
#include "usart.h"	
#include "timer.h" 
#include "led.h"
#include "gps.h"
#include "hall.h"
#include "beep.h"
#include "hcsr04.h"
#include "ec01g.h"
#include "flag.h"
#include "icm42688.h"
#include "nv020c.h"
#include "malloc.h"	 
#include "oled.h"
#include "oleddispaly.h"



int main(void)
{
	delay_init();	    	//延时函数初始化	 
	delay_ms(500);	
	NVIC_Configuration(); 	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart1_init(115200);
	tim3_init(999,71);
	mem_init();				//初始化内存池 	
	key_init();
	beep_init();
	nv020c_init();	
	while(bsp_Icm42688Init() != 0)
	{
		delay_ms(100);
	}
	ec01g_init(9600);
	ec01g_rst();
	ec01g_handle();
	flag_init();	
	oled_init();
	hcsr04_init();
	delay_ms(1000);
	while(1)
	{	
		if(flag_1ms)
		{
			gps_handle();
			flag_1ms = 0;
		}
		if(flag_50ms)
		{								
			imu_handle();
			flag_50ms = 0;																				
		}
		if(flag_500ms)
		{				
			hall_handle();			
			oled_handle();			
			//nb_data_send();
			flag_500ms = 0;
		}
		
		if(flag_1s)
		{	
			nb_data_send();
			hcsr04_handle();
			flag_1s = 0;
		}
	}
}


#include "hall.h"
#include "flag.h"
#include "beep.h"
#include "delay.h"
#include "timer.h"
#include "nv020c.h"
#include "icm42688.h"
#include <stdint.h>
#include <stdbool.h>

#define HALL_USART1_DEBUG	0

uint8_t is_magnet_present = 0;
uint8_t key_value = 0xff;

void key_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(KEY1_GPIO_CLK, ENABLE);	 //使能端口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN;	//端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度为50MHz
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure); //根据设定参数初始化
}

int read_hall_sensor(void) 
{
    return KEY1;
}

/* 参数配置 */
#define DEBOUNCE_TIME_MS   50    // 去抖动时间（毫秒）
#define LONG_ABSENT_TIME_MS 2000 // 长时缺席判定阈值（毫秒）

typedef enum {
    MAGNET_PRESENT = 0,    // 磁铁存在
    MAGNET_ABSENT = 1,      // 磁铁短暂离开
    MAGNET_LONG_ABSENT = 2  // 磁铁长时间离开
} MagnetState;

MagnetState check_magnet_state(void) 
{
    /* 状态存储器（使用static保证数据持久性） */
    static int last_raw = -1;              // 上次传感器原始值
    static uint32_t debounce_start = 0;    // 抖动开始时间
    static MagnetState stable_state;       // 稳定状态
    static uint32_t absent_timestamp = 0;  // 开始缺席时间戳
    static bool initialized = false;        // 初始化标志

    /* 获取当前系统状态 */
    const int current_raw = read_hall_sensor();
    const uint32_t now = get_tick_count();

    /* 初始化模块状态 */
    if (!initialized) {
        last_raw = current_raw;
        stable_state = current_raw ? MAGNET_ABSENT : MAGNET_PRESENT;
        absent_timestamp = (stable_state == MAGNET_ABSENT) ? now : 0;
        initialized = true;
        return stable_state;
    }

    /* 状态变化检测 */
    if (current_raw != last_raw) {
        debounce_start = now;     // 重置抖动计时器
        last_raw = current_raw;   // 更新最后检测值
    }

    /* 去抖动状态机 */
    if ((now - debounce_start) >= DEBOUNCE_TIME_MS) {
        const MagnetState new_state = current_raw ? MAGNET_ABSENT : MAGNET_PRESENT;
        
        /* 检测到有效状态变化 */
        if (stable_state != new_state) {
            stable_state = new_state;
            absent_timestamp = (stable_state == MAGNET_ABSENT) ? now : 0;
        }
    }

    /* 长时缺席检测 */
    if ((stable_state == MAGNET_ABSENT) && 
        (absent_timestamp > 0) && 
        ((now - absent_timestamp) >= LONG_ABSENT_TIME_MS)) {
        return MAGNET_LONG_ABSENT;
    }

    return stable_state;
}

void hall_handle(void)
{
	MagnetState state = check_magnet_state();
    
    switch(state) 
	{
        case MAGNET_PRESENT:
			is_magnet_present = 0;			
			#if(HALL_USART1_DEBUG==1)
			printf("magnet is present!\r\n");
			#endif		
            // 处理磁铁存在逻辑
            break;
        case MAGNET_ABSENT:
			#if(HALL_USART1_DEBUG==1)
			printf("magnet is absent!\r\n");
			#endif
            // 处理磁铁短暂离开逻辑
            break;
        case MAGNET_LONG_ABSENT:
			is_magnet_present = 1;
			#if(HALL_USART1_DEBUG==1)
			printf("magnet is long absent!\r\n");
			#endif
            // 处理磁铁长时离开逻辑
            break;
    }
	
	if(is_magnet_present)
	{
		BEEP(Bit_SET); 	
		if(flag_5s)
		{
			flag_5s = 0;
			nv020c_play_sound(0x0a);
		}
	}
	else
	{
		BEEP(Bit_RESET);  
	}
}

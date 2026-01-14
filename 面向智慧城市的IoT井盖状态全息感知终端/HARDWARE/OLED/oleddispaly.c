#include "oleddispaly.h"
#include "oled.h" 
#include "hall.h" 
#include "hcsr04.h"
#include "icm42688.h"
#include "stdio.h"
#include "flag.h" 
#include "delay.h"  

#define APP_USART1_DEBUG	0

void display_distance(uint16_t distance) 
{
    uint8_t hundreds = distance / 100;
    uint8_t tens = (distance % 100) / 10;
    uint8_t units = distance % 10;
    int x_pos = 80;  // 初始显示位置（原百位位置）

    // 显示固定前缀
    oled_show_string(0, line5, (uint8_t *)"Distance:", 8);

    // 动态数字显示
    if (hundreds != 0) {
        // 显示三位数（包含所有数字）
        oled_show_char(x_pos, line5, hundreds + '0', 8);
        x_pos += 8;
        oled_show_char(x_pos, line5, tens + '0', 8);
        x_pos += 8;
    } else if (tens != 0) {
        // 显示两位数（跳过百位）
        oled_show_char(x_pos, line5, tens + '0', 8);
        x_pos += 8;
    }
    // 始终显示个位数
    oled_show_char(x_pos, line5, units + '0', 8);
    x_pos += 8;

    // 动态单位显示
    oled_show_char(x_pos, line5, 'C', 8);
    x_pos += 8;
    oled_show_char(x_pos, line5, 'M', 8);
	x_pos += 8;
    oled_show_char(x_pos, line5, ' ', 8);
	x_pos += 8;
    oled_show_char(x_pos, line5, ' ', 8);
}

void display_angle(uint16_t angle) 
{
    uint8_t hundreds = angle / 100;
    uint8_t tens = (angle % 100) / 10;
    uint8_t units = angle % 10;
    int x_pos = 54;  // 初始显示位置（原百位位置）

    // 显示固定前缀
    oled_show_string(0, line7, (uint8_t *)"Angle: ", 8);

    // 动态数字显示
    if (hundreds != 0) {
        // 显示三位数（包含所有数字）
        oled_show_char(x_pos, line7, hundreds + '0', 8);
        x_pos += 8;
        oled_show_char(x_pos, line7, tens + '0', 8);
        x_pos += 8;
    } else if (tens != 0) {
        // 显示两位数（跳过百位）
        oled_show_char(x_pos, line7, tens + '0', 8);
        x_pos += 8;
    }
    // 始终显示个位数
    oled_show_char(x_pos, line7, units + '0', 8);
    x_pos += 8;

    // 动态单位显示
    oled_show_char(x_pos, line7, 96+' ', 8);
    x_pos += 8;
    oled_show_char(x_pos, line7, ' ', 8);
	x_pos += 8;
    oled_show_char(x_pos, line7, ' ', 8);
}

void oled_handle(void)
{
	oled_show_string(0, line1,(uint8_t *)"Manhole cover", 8);
	if(is_magnet_present)
	{
		oled_show_string(0, line3,(uint8_t *)"Magnet:Absent ", 8);
	}
	else
	{
		oled_show_string(0, line3,(uint8_t *)"Magnet:Present", 8);  
	}
	
	display_distance(distance); 
	display_angle(cover_tilt_value);
}

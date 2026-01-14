#ifndef __OLED_H
#define __OLED_H	

#include "sys.h"
#include "stdlib.h"	   

#define OLED_MODE 	  0
#define SIZE 		  8
#define XLevelL		  0x00
#define XLevelH		  0x10
#define Max_Column	  128
#define Max_Row		  64
#define	Brightness	  0xFF 
#define X_WIDTH 	  128
#define Y_WIDTH 	  64	   

/* OLED屏幕有7行 */
#define line1 0
#define line2 1
#define line3 2
#define line4 3
#define line5 4
#define line6 5
#define line7 6
#define line8 7
/* OLED屏幕有128的点,也就是16列（16*8） */
#define byte(data)  data*8

#define OLED_SCL_GPIO		GPIOB
#define OLED_SCL_GPIO_PIN	GPIO_Pin_6
#define OLED_SCL_GPIO_CLK	RCC_APB2Periph_GPIOB
#define OLED_SDA_GPIO		GPIOB
#define OLED_SDA_GPIO_PIN	GPIO_Pin_5
#define OLED_SDA_GPIO_CLK	RCC_APB2Periph_GPIOB

//-----------------OLED IIC端口定义----------------  					   
#define OLED_SCL_LOW() 	GPIO_ResetBits(OLED_SCL_GPIO, OLED_SCL_GPIO_PIN)//SCL
#define OLED_SCL_HIGH() GPIO_SetBits(OLED_SCL_GPIO, OLED_SCL_GPIO_PIN)

#define OLED_SDA_LOW() 	GPIO_ResetBits(OLED_SDA_GPIO, OLED_SDA_GPIO_PIN)//SDA
#define OLED_SDA_HIGH() GPIO_SetBits(OLED_SDA_GPIO, OLED_SDA_GPIO_PIN)
		     
#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

#define OLED_DEBUG	0

//OLED控制用函数
void oled_display_on(void);
void oled_clear(void);
void oled_clear_blue(void);
void oled_clear_line(u8 line);
void oled_on(void);
void oled_show_char(u8 x,u8 y,u8 chr,u8 Char_Size);
void inverse_oled_show_char(u8 x,u8 y,u8 chr,u8 Char_Size);
void inverse_oled_show_string(u8 x,u8 y,u8 *chr,u8 Char_Size);
void oled_show_num(u8 x,u8 y,u32 num,u8 len,u8 size);
void inverse_oled_show_num(u8 x,u8 y,u32 num,u8 len,u8 size);
void oled_show_string(u8 x,u8 y,u8 *chr,u8 Char_Size);
void oled_show_chinese(u8 x,u8 y,u8 num);
void oled_show_chinese_string(u8 x,u8 y,u8 num);
void oled_draw_bmp(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);
void oled_init(void);

#endif  
	 


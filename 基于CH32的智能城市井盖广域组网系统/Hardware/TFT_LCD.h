#ifndef __TFT_LCD_H
#define __TFT_LCD_H		
#include "debug.h"                  // Device header


/****************************显示函数****************************/

/*LCD打印输出函数*/
void LCD_Printf(int16_t X, int16_t Y, uint16_t fc, uint16_t bc, uint8_t sizey, char *format, ...);
/*LCD显示字符函数*/
void LCD_ShowChar(uint16_t x,uint16_t y,char num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode);
/*LCD显示字符串函数*/
void LCD_ShowString(uint16_t x,uint16_t y,const char *String,uint16_t fc,uint16_t bc,uint8_t sizey);
/*LCD显示无符号整数函数*/
void LCD_ShowNum(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey);
/*LCD显示有符号整数函数*/
void LCD_ShowSignedNum(uint16_t x,uint16_t y,int32_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey);
/*LCD显示无符号小数函数*/
void LCD_ShowFloatNum(uint16_t x,uint16_t y,double num,uint8_t IntLength,uint8_t FraLength, uint16_t fc,uint16_t bc,uint8_t sizey);
/*LCD显示有符号小数函数*/
void LCD_ShowSignedFloatNum(uint16_t x,uint16_t y,double num,uint8_t IntLength,uint8_t FraLength, uint16_t fc,uint16_t bc,uint8_t sizey);
/*LCD显示自定义图像函数*/
void LCD_ShowImage(uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, uint16_t fc, uint16_t bc, const uint8_t *Image);
/*LCD显示图片函数*/
void LCD_ShowPicture(uint16_t x,uint16_t y,uint16_t length,uint16_t width,const uint8_t pic[]);
/*LCD求幂函数*/
uint32_t mypow(uint8_t m,uint8_t n);

/****************************绘图函数****************************/

/*LCD区域填充颜色函数*/
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);
/*LCD指定画点函数*/
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
/*LCD指定画线函数*/
void LCD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);
/*LCD指定画矩形函数*/
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
/*LCD指定画圆形函数*/
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color);

/****************************功能函数****************************/
/*LCD休眠函数*/
void LCD_Sleep(void);
/*LCD唤醒函数*/
void LCD_WKUP(void);


//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

#endif






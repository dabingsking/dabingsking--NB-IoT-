#ifndef __TFT_LCD_INIT_H
#define __TFT_LCD_INIT_H
#include "debug.h"

#ifndef  TFT_LCD_SIZE 
#define  TFT_LCD_SIZE 201/*选择LCD尺寸（1.47英寸和2.01英寸）*/
#endif

#define USE_HORIZONTAL 3  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#if TFT_LCD_SIZE==147
    #if USE_HORIZONTAL==0||USE_HORIZONTAL==1
        #define LCD_W 172	
        #define LCD_H 320

    #else
        #define LCD_W 320
        #define LCD_H 172
    #endif
#elif TFT_LCD_SIZE==201
    #if USE_HORIZONTAL==0||USE_HORIZONTAL==1
        #define LCD_W 240	
        #define LCD_H 296

    #else
        #define LCD_W 296
        #define LCD_H 240
    #endif
#endif

/*-----------------LCD端口定义(链嵌派)----------------*/
#define LCD_BLK_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_4)//BLK
#define LCD_BLK_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_4)

#define LCD_RES_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_5)//RES
#define LCD_RES_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_5)

#define LCD_DC_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_6)//DC
#define LCD_DC_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_6)

#define LCD_MOSI_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_7)//SDA=MOSI
#define LCD_MOSI_Set() GPIO_SetBits(GPIOB,GPIO_Pin_7)

#define LCD_SCLK_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_8)//SCL=SCLK
#define LCD_SCLK_Set() GPIO_SetBits(GPIOB,GPIO_Pin_8)
		     
#define LCD_CS_Clr()   GPIO_ResetBits(GPIOB,GPIO_Pin_9)//CS
#define LCD_CS_Set()   GPIO_SetBits(GPIOB,GPIO_Pin_9)



void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(uint8_t dat);//模拟SPI时序
void LCD_WR_DATA8(uint8_t dat);//写入一个字节
void LCD_WR_DATA(uint16_t dat);//写入两个字节
void LCD_WR_REG(uint8_t dat);//写入一个指令
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//设置坐标函数
void LCD_Init(void);//LCD初始化
#endif





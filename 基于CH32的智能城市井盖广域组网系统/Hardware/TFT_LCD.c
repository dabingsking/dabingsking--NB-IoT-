#include "TFT_LCD.h"
#include "TFT_LCD_Init.h"
#include "TFT_LCD_Font.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


/******************************************************************************
      函数说明：LCD打印输出函数
      入口数据：x,y显示坐标
                fc 字的颜色
                bc 字的背景色
                sizey 字号
				format 指定要显示的格式化字符串
				... 格式化字符串参数列表
      返回值：  无
******************************************************************************/
void LCD_Printf(int16_t X, int16_t Y, uint16_t fc, uint16_t bc, uint8_t sizey, char *format, ...)
{
	char String[256];						//定义字符数组
	va_list arg;							//定义可变参数列表数据类型的变量arg
	va_start(arg, format);					//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);							//结束变量arg
	LCD_ShowString(X, Y, String, fc, bc, sizey); //重定向到LCD显示字符串函数(字符串函数默认不叠加模式)
}

/******************************************************************************
      函数说明：LCD显示字符函数
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChar(uint16_t x,uint16_t y,char num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode)
{
	uint8_t temp,sizex,t,m=0;
	uint16_t i,TypefaceNum;//一个字符所占字节大小
	uint16_t x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{
		if(sizey==24)temp=ascii_2412[num][i];		 //调用12x24字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}

/******************************************************************************
      函数说明：显示字符串(支持中英文混打)
      入口数据：x,y显示坐标
                num 要显示小数
                String 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
				mode: 默认非叠加模式(故形参上没写)
      返回值：  无
	参考来自:江协科技OLED中英文混写代码
******************************************************************************/
void LCD_ShowString(uint16_t x,uint16_t y,const char *String,uint16_t fc,uint16_t bc,uint8_t sizey)
{
	uint16_t i = 0;
	char SingleChar[5];
	uint8_t CharLength = 0;
	uint16_t XOffset = 0;
	uint16_t pIndex;
	uint8_t sizex=sizey/2;
	while (String[i] != '\0')	//遍历字符串
	{
/***********************************************(n)ByteChar extraction***********************************************/
		/*此段代码的目的是，提取UTF8字符串中的一个字符，转存到SingleChar子字符串中*/
		/*判断UTF8编码第一个字节的标志位*/
		if ((String[i] & 0x80) == 0x00)			//第一个字节为0xxxxxxx
		{
			CharLength = 1;						//字符为1字节
			SingleChar[0] = String[i ++];		//将第一个字节写入SingleChar第0个位置，随后i指向下一个字节
			SingleChar[1] = '\0';				//为SingleChar添加字符串结束标志位
		}
		else if ((String[i] & 0xE0) == 0xC0)	//第一个字节为110xxxxx
		{
			CharLength = 2;						//字符为2字节
			SingleChar[0] = String[i ++];		//将第一个字节写入SingleChar第0个位置，随后i指向下一个字节
			if (String[i] == '\0') {break;}		//意外情况，跳出循环，结束显示
			SingleChar[1] = String[i ++];		//将第二个字节写入SingleChar第1个位置，随后i指向下一个字节
			SingleChar[2] = '\0';				//为SingleChar添加字符串结束标志位
		}
		else if ((String[i] & 0xF0) == 0xE0)	//第一个字节为1110xxxx
		{
			CharLength = 3;						//字符为3字节
			SingleChar[0] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[1] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[2] = String[i ++];
			SingleChar[3] = '\0';
		}
		else if ((String[i] & 0xF8) == 0xF0)	//第一个字节为11110xxx
		{
			CharLength = 4;						//字符为4字节
			SingleChar[0] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[1] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[2] = String[i ++];
			if (String[i] == '\0') {break;}
			SingleChar[3] = String[i ++];
			SingleChar[4] = '\0';
		}
		else
		{
			i ++;			//意外情况，i指向下一个字节，忽略此字节，继续判断下一个字节
			continue;
		}
/***********************************************(n)ByteChar Display***********************************************/
		/*单字节字符*/
		if (CharLength == 1)
		{
			LCD_ShowChar(x+XOffset,y, SingleChar[0], fc, bc, sizey, 0);//默认不叠加模式
			XOffset += sizex;
		}
		/*多字节字符*/
		else
		{
			switch (sizey)
			{
				/*size==24*/
				case 24:
						for (pIndex = 0; strcmp(tfont24[pIndex].Index, "") != 0; pIndex ++)
						{
							if (strcmp(tfont24[pIndex].Index, SingleChar) == 0) break;
						}
						LCD_ShowImage(x+XOffset, y, sizey, sizey, fc, bc, tfont24[pIndex].Msk);
						XOffset += sizey;
					break;
				/*size==32*/
				case 32:
						for (pIndex = 0; strcmp(tfont32[pIndex].Index, "") != 0; pIndex ++)
						{
							if (strcmp(tfont32[pIndex].Index, SingleChar) == 0) break;
						}
						LCD_ShowImage(x+XOffset, y, sizey, sizey, fc, bc, tfont32[pIndex].Msk);
						XOffset += sizey;
					break;
				default: break;
			}
		}
	}
}

/******************************************************************************
      函数说明：LCD显示无符号整数函数
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowNum(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey)
{         
	uint8_t sizex=sizey/2;	
	for (uint8_t i=0;i<len;i++)			//遍历数字的每一位								
	{
		LCD_ShowChar(x+i*sizex, y, num/mypow(10, (len-1)-i)%10+0x30, fc, bc, sizey, 0);
	}
} 


/******************************************************************************
      函数说明：LCD显示有符号整数函数
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowSignedNum(uint16_t x,uint16_t y,int32_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey)
{         	
	uint8_t sizex=sizey/2;
	if (num>=0)						//数字大于等于0
	{
		LCD_ShowChar(x, y, '+', fc, bc, sizey, 0);	//显示+号
	}
	else									//数字小于0
	{
		LCD_ShowChar(x, y, '-', fc, bc, sizey, 0);	//显示+号
		num=-num;					//Number1等于Number取负
	}

	x+=sizex;
	for(uint8_t i=0;i<len;i++)			//遍历数字的每一位								
	{
		LCD_ShowChar(x+i*sizex, y, num/mypow(10, (len-1)-i)%10+0x30, fc, bc, sizey, 0);
	}
} 

/******************************************************************************
      函数说明：LCD显示无符号小数函数
      入口数据：x,y显示坐标
                num 要显示小数变量
                IntLength 整数部分要显示的位数
                FraLength 小数部分要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowFloatNum(uint16_t x,uint16_t y,double num,uint8_t IntLength,uint8_t FraLength, uint16_t fc,uint16_t bc,uint8_t sizey)
{    
	uint32_t PowNum, IntNum, FraNum;
	uint8_t sizex=sizey/2;	
	
/*提取整数部分和小数部分*/
	//整数部分
	IntNum=(uint32_t)num;
	//小数部分			
	PowNum=mypow(10, FraLength);
	FraNum=(uint32_t)((num-IntNum)*PowNum);
	
	/*显示整数部分*/
	LCD_ShowNum(x, y, IntNum, IntLength, fc, bc, sizey);
	/*显示小数点*/
	LCD_ShowChar(x+IntLength*sizex, y, '.', fc, bc, sizey, 0);
	/*显示小数部分*/
	LCD_ShowNum(x+(IntLength+1)*sizex, y, FraNum, FraLength, fc, bc, sizey);
}

/******************************************************************************
      函数说明：LCD显示有符号小数函数
      入口数据：x,y显示坐标
                num 要显示小数变量
                IntLength 整数部分要显示的位数
                FraLength 小数部分要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowSignedFloatNum(uint16_t x,uint16_t y,double num,uint8_t IntLength,uint8_t FraLength, uint16_t fc,uint16_t bc,uint8_t sizey)
{    
	uint32_t PowNum, IntNum, FraNum;
	uint8_t sizex=sizey/2;	
/*判断正负*/
	if (num>=0)						//数字大于等于0
	{
		LCD_ShowChar(x, y, '+', fc, bc, sizey, 0);	//显示+号
	}
	else									//数字小于0
	{
		LCD_ShowChar(x, y, '-', fc, bc, sizey, 0);	//显示-号
		num=-num;					//Number取负
	}
	
/*提取整数部分和小数部分*/
	//整数部分
	IntNum=(uint32_t)num;
	//小数部分			
	PowNum=mypow(10, FraLength);
	FraNum=(uint32_t)((num-IntNum)*PowNum);

	x+=sizex;
	/*显示整数部分*/
	LCD_ShowNum(x, y, IntNum, IntLength, fc, bc, sizey);
	/*显示小数点*/
	LCD_ShowChar(x+IntLength*sizex, y, '.', fc, bc, sizey, 0);
	/*显示小数部分*/
	LCD_ShowNum(x+(IntLength+1)*sizex, y, FraNum, FraLength, fc, bc, sizey);
}

/******************************************************************************
      函数说明：LCD显示自定义图像函数
      入口数据：x,y显示坐标
                Width 图像宽度
                Height 图像高度
                fc 字的颜色
                bc 字的背景色
                *image 图像数组指针
      返回值：  无
******************************************************************************/
void LCD_ShowImage(uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, uint16_t fc, uint16_t bc, const uint8_t *Image)
{
	uint16_t i=0;
	uint8_t pagex=0,j=0;
	uint8_t k=0,Last_Page_Flag=0;
	pagex=Width/8;
	/*判断最后一页是否够一个字节*/
	if(Width%8!=0) {Last_Page_Flag=1;pagex++;}
	/*画第n排n列*/
	for(i=0;i<Height;i++)
	{
		/*画第一排n列*/
		for(j=0;j<pagex;j++,Image++)
		{
			/*横向画一个字节*/
			for(k=0;(Last_Page_Flag&&j==(pagex-1))?k<Width%8:k<8;k++)
			{
				if(*Image&(0x01<<k))
				{
					LCD_DrawPoint(x+k+j*8, y+i, fc);
				}
				else LCD_DrawPoint(x+k+j*8, y+i, bc);
			}
		}
	}
}

/******************************************************************************
      函数说明：LCD显示图片函数
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组    
      返回值：  无
******************************************************************************/
void LCD_ShowPicture(uint16_t x,uint16_t y,uint16_t length,uint16_t width,const uint8_t pic[])
{
	uint16_t i,j;
	uint32_t k=0;
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	for(i=0;i<length;i++)
	{
		for(j=0;j<width;j++)
		{
			LCD_WR_DATA8(pic[k*2]);
			LCD_WR_DATA8(pic[k*2+1]);
			k++;
		}
	}			
}

/******************************************************************************
      函数说明：LCD求幂函数
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
uint32_t mypow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;
	return result;
}

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
								color       要填充的颜色
      返回值：  无
******************************************************************************/
void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color)
{          
	uint16_t i,j; 
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//设置显示范围
	for(i=ysta;i<yend;i++)
	{													   	 	
		for(j=xsta;j<xend;j++)
		{
			LCD_WR_DATA(color);
		}
	} 					  	    
}

/******************************************************************************
      函数说明：LCD指定画点函数
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_Address_Set(x,y,x,y);//设置光标位置 
	LCD_WR_DATA(color);
}

/******************************************************************************
      函数说明：LCD指定画线函数
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   线的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向 
	else if (delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

/******************************************************************************
      函数说明：LCD指定画矩形函数
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   矩形的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}

/******************************************************************************
      函数说明：LCD指定画圆形函数
      入口数据：x0,y0   圆心坐标
                r       半径
                color   圆的颜色
      返回值：  无
******************************************************************************/
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1                
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//判断要画的点是否过远
		{
			b--;
		}
	}
}

/******************************************************************************
      函数说明：LCD休眠函数
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_Sleep(void)
{
	LCD_BLK_Set();
    LCD_WR_REG(0x28);
    LCD_WR_REG(0x10);
    Delay_Ms(120);
}

/******************************************************************************
      函数说明：LCD唤醒函数
      入口数据：无
      返回值：  无
******************************************************************************/
void LCD_WKUP(void)
{
    LCD_WR_REG(0x11);
    Delay_Ms(120);
    LCD_WR_REG(0x29);
	LCD_BLK_Clr();
}
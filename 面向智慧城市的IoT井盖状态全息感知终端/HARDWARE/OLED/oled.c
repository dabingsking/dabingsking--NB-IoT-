#include "delay.h"  	
#include "oled.h"
#include "stdlib.h"
#include "oledfont.h"  	 

//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 			   

void iic_start()
{
	OLED_SCL_HIGH();
	OLED_SDA_HIGH();
	OLED_SDA_LOW();
	OLED_SCL_LOW();
}

void iic_stop()
{
	OLED_SCL_HIGH();
	OLED_SDA_LOW();
	OLED_SDA_HIGH();
	
}

void iic_wait_ack()
{
	OLED_SCL_HIGH();
	OLED_SCL_LOW();
}

void iic_write_byte(unsigned char IIC_Byte)
{
	unsigned char i;
	unsigned char m,da;
	da=IIC_Byte;
	OLED_SCL_LOW();
	for(i=0;i<8;i++)		
	{
		m=da;
		m=m&0x80;
		if(m==0x80)
		{
			OLED_SDA_HIGH();
		}
		else 
			OLED_SDA_LOW();
		da=da<<1;
		OLED_SCL_HIGH();
		OLED_SCL_LOW();
	}
}

void iic_write_cmd(unsigned char IIC_Command)
{
	iic_start();
	iic_write_byte(0x78);       //Slave address,SA0=0
	iic_wait_ack();	
	iic_write_byte(0x00);		//write command
	iic_wait_ack();	
	iic_write_byte(IIC_Command); 
	iic_wait_ack();	
	iic_stop();
}

void iic_write_data(unsigned char IIC_Data)
{
	iic_start();
	iic_write_byte(0x78);			//D/C#=0; R/W#=0
	iic_wait_ack();	
	iic_write_byte(0x40);			//write data
	iic_wait_ack();	
	iic_write_byte(IIC_Data);
	iic_wait_ack();	
	iic_stop();
}

void oled_wr_byte(unsigned dat,unsigned cmd)
{
	if(cmd)
	{
		iic_write_data(dat);
	}
	else 
	{
		iic_write_cmd(dat);	
	}
}


/********************************************
// fill_Picture
********************************************/
void fill_picture(unsigned char fill_Data)
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		oled_wr_byte(0xb0+m,0);		//page0-page1
		oled_wr_byte(0x00,0);		 //low column start address
		oled_wr_byte(0x10,0);		 //high column start address
		for(n=0;n<128;n++)
		{
			oled_wr_byte(fill_Data,1);
		}
	}
}

//坐标设置
void oled_set_pos(unsigned char x, unsigned char y) 
{ 
	oled_wr_byte(0xb0+y,OLED_CMD);
	oled_wr_byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	oled_wr_byte((x&0x0f),OLED_CMD); 
}   	  
//开启OLED显示    
void oled_display_on(void)
{
	oled_wr_byte(0X8D,OLED_CMD);  //SET DCDC命令
	oled_wr_byte(0X14,OLED_CMD);  //DCDC ON
	oled_wr_byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示     
void oled_display_off(void)
{
	oled_wr_byte(0X8D,OLED_CMD);  //SET DCDC命令
	oled_wr_byte(0X10,OLED_CMD);  //DCDC OFF
	oled_wr_byte(0XAE,OLED_CMD);  //DISPLAY OFF
}		   			 
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!	  
void oled_clear(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		oled_wr_byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		oled_wr_byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		oled_wr_byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)
			oled_wr_byte(0,OLED_DATA); 
	} //更新显示
}

//清蓝屏函数	  
void oled_clear_blue(void)  
{  
	u8 i,n;		    
	for(i=2;i<8;i++)  
	{  
		oled_wr_byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		oled_wr_byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		oled_wr_byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)oled_wr_byte(0,OLED_DATA); 
	} //更新显示
}
//清行函数
void oled_clear_line(u8 line)
{
	u8 n;
	oled_wr_byte (0xb0+line,OLED_CMD);    //设置页地址（0~7）
	oled_wr_byte (0x00,OLED_CMD);      //设置显示位置—列低地址
	oled_wr_byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
	for(n=0;n<128;n++)oled_wr_byte(0,OLED_DATA); 
}

void oled_on(void)  
{  
	u8 i,n;		    
	for(i=0;i<8;i++)  
	{  
		oled_wr_byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		oled_wr_byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		oled_wr_byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)oled_wr_byte(1,OLED_DATA); 
	} //更新显示
}

//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示				 
//size:选择字体 8/6
void oled_show_char(u8 x,u8 y,u8 chr,u8 Char_Size)
{      	
	unsigned char c=0,i=0;	
	c=chr-' ';//得到偏移后的值			
	if(x>Max_Column-1)
	{
		x=0;
		y=y+2;
	}
	if(Char_Size ==8)//字体为8号
	{
		oled_set_pos(x,y);	
		for(i=0;i<8;i++)
			oled_wr_byte(F8X16[c*16+i],OLED_DATA);
		oled_set_pos(x,y+1);
		for(i=0;i<8;i++)
			oled_wr_byte(F8X16[c*16+i+8],OLED_DATA);
	}
	else 
	{	 ////字体为6号
		oled_set_pos(x,y);
		for(i=0;i<6;i++)
			oled_wr_byte(F6x8[c][i],OLED_DATA);
	}
}

//在指定位置反向显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示				 
//size:选择字体 8/6
void inverse_oled_show_char(u8 x,u8 y,u8 chr,u8 Char_Size)
{      	
	unsigned char c=0,i=0;	
	c=chr-' ';//得到偏移后的值			
	if(x>Max_Column-1)
	{
		x=0;
		y=y+2;
	}
	if(Char_Size ==8)//字体为8号
	{
		oled_set_pos(x,y);	
		for(i=0;i<8;i++)
			oled_wr_byte(~F8X16[c*16+i],OLED_DATA);
		oled_set_pos(x,y+1);
		for(i=0;i<8;i++)
			oled_wr_byte(~F8X16[c*16+i+8],OLED_DATA);
	}
	else 
	{	 ////字体为6号
		oled_set_pos(x,y);
		for(i=0;i<6;i++)
			oled_wr_byte(~F6x8[c][i],OLED_DATA);
	}
}

//m^n函数
u32 oled_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)
		result*=m;    
	return result;
}

//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~4294967295);	 		  
void oled_show_num(u8 x,u8 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				oled_show_char(x+size*t,y,' ',size);
				continue;
			}
			else 
				enshow=1; 
		}
	 	oled_show_char(x+size*t,y,temp+'0',size); 
	}
}

//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~4294967295);	 		  
void inverse_oled_show_num(u8 x,u8 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				inverse_oled_show_char(x+size*t,y,' ',size);
				continue;
			}
			else 
				enshow=1; 
		}
	 	inverse_oled_show_char(x+size*t,y,temp+'0',size); 
	}
}

//显示一个字符号串
void oled_show_string(u8 x,u8 y,u8 *chr,u8 Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{	
		oled_show_char(x,y,chr[j],Char_Size);
		x+=Char_Size;//字间距
		if(x>120) 
		{
			x=0;y+=2;
		}
		j++;
	}
}

//反向显示一个字符号串
void inverse_oled_show_string(u8 x,u8 y,u8 *chr,u8 Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{	
		inverse_oled_show_char(x,y,chr[j],Char_Size);
		x+=Char_Size;//字间距
		if(x>120) 
		{
			x=0;y+=2;
		}
		j++;
	}
}

//显示汉字
//x,y :起点坐标	 
//data[][16] 汉字取模
//num:二维数组行数	16x16
void oled_show_chinese(u8 x,u8 y,u8 num)
{      			    
	u8 t;
	oled_set_pos(x,y);//设置坐标
	for(t=0;t<16;t++)
	{
		oled_wr_byte(CN16CHAR[2*num][t],OLED_DATA);
	}	
	oled_set_pos(x,y+1);//设置坐标
	for(t=0;t<16;t++)
	{	
		oled_wr_byte(CN16CHAR[2*num+1][t],OLED_DATA);
	}					
}

//显示汉字
//x,y :起点坐标	 
//data[][16] 汉字取模
//num:二维数组行数	7x7
void oled_show_chinese_7x7(u8 x,u8 y,const char data[][16],u8 num)
{      			    
	u8 t;
	oled_set_pos(x,y);//设置坐标
	for(t=0;t<7;t++)
	{
		oled_wr_byte(data[2*num][t],OLED_DATA);
	}	
	oled_set_pos(x,y+1);//设置坐标
	for(t=0;t<7;t++)
	{	
		oled_wr_byte(data[2*num+1][t],OLED_DATA);
	}	
}

//显示一串汉字
//x,y :起点坐标	 
//data[][16] 汉字取模
//num:二维数组行数	16x16
void oled_show_chinese_string(u8 x,u8 y,u8 num)
{      			    
	u8 i;
	for(i=0;i<num;i++)
		oled_show_chinese(x+i*16,y,i);
}

/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void oled_draw_bmp(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
	unsigned int j=0;
	unsigned char x,y;

	if(y1%8==0) 
		y=y1/8;      
	else 
		y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		oled_set_pos(x0,y);
		for(x=x0;x<x1;x++)
		{      
			oled_wr_byte(BMP[j++],OLED_DATA);	    	
		}
	}
} 
					    
void oled_init(void)
{ 
 	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(OLED_SCL_GPIO_CLK | OLED_SDA_GPIO_CLK, ENABLE);//使能B端口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = OLED_SCL_GPIO_PIN;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	//速度10MHz
 	GPIO_Init(OLED_SCL_GPIO, &GPIO_InitStructure);	  	//初始化GPIO
	
	GPIO_InitStructure.GPIO_Pin = OLED_SDA_GPIO_PIN;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;	//速度10MHz
 	GPIO_Init(OLED_SDA_GPIO, &GPIO_InitStructure);	  	//初始化GPIO
	
 	GPIO_SetBits(OLED_SCL_GPIO, OLED_SCL_GPIO_PIN);	
	GPIO_SetBits(OLED_SDA_GPIO, OLED_SDA_GPIO_PIN);	
	
	delay_ms(100);
	oled_wr_byte(0xAE,OLED_CMD);//--display off
	oled_wr_byte(0x00,OLED_CMD);//---set low column address
	oled_wr_byte(0x10,OLED_CMD);//---set high column address
	oled_wr_byte(0x40,OLED_CMD);//--set start line address  
	oled_wr_byte(0xB0,OLED_CMD);//--set page address
	oled_wr_byte(0x81,OLED_CMD); // contract control
	oled_wr_byte(0xFF,OLED_CMD);//--128   
	oled_wr_byte(0xA1,OLED_CMD);//set segment remap 
	oled_wr_byte(0xA6,OLED_CMD);//--normal / reverse
	oled_wr_byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	oled_wr_byte(0x3F,OLED_CMD);//--1/32 duty
	oled_wr_byte(0xC8,OLED_CMD);//Com scan direction
	oled_wr_byte(0xD3,OLED_CMD);//-set display offset
	oled_wr_byte(0x00,OLED_CMD);//
	
	oled_wr_byte(0xD5,OLED_CMD);//set osc division
	oled_wr_byte(0x80,OLED_CMD);//
	
	oled_wr_byte(0xD8,OLED_CMD);//set area color mode off
	oled_wr_byte(0x05,OLED_CMD);//
	
	oled_wr_byte(0xD9,OLED_CMD);//Set Pre-Charge Period
	oled_wr_byte(0xF1,OLED_CMD);//
	
	oled_wr_byte(0xDA,OLED_CMD);//set com pin configuartion
	oled_wr_byte(0x12,OLED_CMD);//
	
	oled_wr_byte(0xDB,OLED_CMD);//set Vcomh
	oled_wr_byte(0x30,OLED_CMD);//
	
	oled_wr_byte(0x8D,OLED_CMD);//set charge pump enable
	oled_wr_byte(0x14,OLED_CMD);//
	
	oled_wr_byte(0xAF,OLED_CMD);//--turn on oled panel
	
	oled_display_on();
	oled_clear();
}  



#include "sys.h"
#include <stdlib.h>

#define USART_DEBUG			0

static float AccSensitivity		=	0.244f;						// 加速度的最小分辨率
static float GyroSensitivity	=	32.8f;						// 陀螺仪的最小分辨率

ICM42688RawData_t *Test_AccData;								// 加速度的数据结构体
ICM42688RawData_t *Test_GyroData;								// 陀螺仪的数据结构体

float Qingxiejiao=0;

/*
参数  : pBuffer为写入的数组，len为要读取的数据个数
函数体: 使用SPI读写多个数据（字节）
返回值: 无
*/
static void Icm_SPI_ReadWriteNByte(uint8_t* pBuffer,uint8_t len)
{
	uint8_t i=0;
	for(i=0;i<len;i++)
	{
		*pBuffer=MySPI_WriteReadByte(*pBuffer);
		pBuffer++;
	}
}

/*
参数  : Reg寄存器的地址
函数体: 使用SPI读取单个寄存器的值
返回值: 返回对应寄存器的值
*/
static uint8_t Icm42688_Read_Reg(uint8_t Reg)
{
	uint8_t Regval=0;
	MySPI_Start();												// SPI启动
	Reg |= 0x80;												// 使用SPI读取寄存器时要注意:最高位为读写位
	Icm_SPI_ReadWriteNByte(&Reg,1);								// 写入要度的寄存器的地址
	Icm_SPI_ReadWriteNByte(&Regval,1);							// 读取寄存器的值
	MySPI_Stop();												// SPI停止
	return Regval;
}

/*
参数  : Reg寄存器的地址
函数体: 使用SPI连续读取多个寄存器的值
返回值: 返回对应寄存器的值
*/
static void Icm42688_Read_Regs(uint8_t reg,uint8_t* buf,uint16_t len)
{
	reg|=0x80;													// 使用SPI读取寄存器时要注意:最高位为读写位
	MySPI_Start();												// SPI启动
	Icm_SPI_ReadWriteNByte(&reg,1);								// 写入要度的寄存器的地址
	Icm_SPI_ReadWriteNByte(buf,len);							// 读取寄存器数据
	MySPI_Stop();												// SPI停止
}

/*
参数  : Reg要写入的寄存器的地址，Value要写入的数据
函数体: 使用SPI向单个寄存器写数据
返回值: 返回对应寄存器的值
*/
static uint8_t Icm42688_Write_Reg(uint8_t Reg,uint8_t Value)
{
	MySPI_Start();												// SPI启动
	Icm_SPI_ReadWriteNByte(&Reg,1);								// 写入要度的寄存器的地址
	Icm_SPI_ReadWriteNByte(&Value,1);							// 读取寄存器数据
	MySPI_Stop();												// SPI停止
	return 0;
}

/*
参数  : Ascale为要选择的满量程
函数体: 选择加速度计的满量程范围(单位：g)
返回值: 加速度计的精度
*/
float Icm42688GetAres(uint8_t Ascale)
{
	switch(Ascale)
	{
		// 16为AD采集范围为-32768-+32768
		case AFS_2G:
			AccSensitivity=2000/32768.0f;
		break;
		case AFS_4G:
			AccSensitivity=4000/32768.0f;
		break;
		case AFS_8G:
			AccSensitivity=8000/32768.0f;
		break;
		case AFS_16G:
			AccSensitivity=16000/32768.0f;
		break;		
	}
	return AccSensitivity;
}

/*
参数  : Gscale要选择的满量程
函数体: 选择陀螺仪的满量程范围(单位：°/SEC)
返回值: 陀螺仪的精度
*/
float Icm42688GetGres(uint8_t Gscale)
{
	switch(Gscale)
	{
		case GFS_15_125DPS:
			GyroSensitivity=15.125f/32768.0f;
		break;
		case GFS_31_25DPS:
			GyroSensitivity=31.25f/32768.0f;
		break;
		case GFS_62_5DPS:
			GyroSensitivity=62.5f/32768.0f;
		break;
		case GFS_125DPS:
			GyroSensitivity=125.0/32768.0f;
		break;
		case GFS_250DPS:
			GyroSensitivity=250.0f/32768.0f;
		break;
		case GFS_500DPS:
			GyroSensitivity=500.0f/32768.0f;
		break;
		case GFS_1000DPS:
			GyroSensitivity=1000.0f/32768.0f;
		break;
		case GFS_2000DPS:
			GyroSensitivity=2000.0f/32768.0f;
		break;		
	}
	return GyroSensitivity;
}

/*
参数  : 无
函数体: 配置ICM42688的寄存器
返回值: 是否读取到ID号以及是否配置成功	0成功，1不成功
*/
int8_t Icm42688RegCfg(void)
{
	uint8_t Reg_Val=0;
	Reg_Val=Icm42688_Read_Reg(ICM42688_WHO_AM_I);						// 读取ICM42688_WHO_AM_I寄存器，获取设备ID号
	Icm42688_Write_Reg(ICM42688_REG_BANK_SEL,0);						// 选择Bank0为寄存器组
	Icm42688_Write_Reg(ICM42688_DEVICE_CONFIG,0x01);					// 软复位传感器
	Delay_Ms(100);
	//为指针分配内存
	Test_AccData=(ICM42688RawData_t *)malloc(sizeof(ICM42688RawData_t));
	Test_GyroData=(ICM42688RawData_t *)malloc(sizeof(ICM42688RawData_t));
	if(Reg_Val==ICM42688_ID)
	{
		Icm42688_Write_Reg(ICM42688_REG_BANK_SEL,1);					// 设置BANK1区寄存器
		Icm42688_Write_Reg(ICM42688_INTF_CONFIG4,0x02);					// 设置为4线SPI通信
		
		Icm42688_Write_Reg(ICM42688_REG_BANK_SEL,0);					// 设置BANK0区寄存器
		Icm42688_Write_Reg(ICM42688_FIFO_CONFIG,0x40);					// FIFO模式，0x40为流至 FIFO 模式，数据会不断地流入 FIFO 中，无论 FIFO 是否已满。如果 FIFO 满了还继续写入数据，可能会导致数据覆盖。
		
		Reg_Val = Icm42688_Read_Reg(ICM42688_INT_SOURCE0);				// 用于配置不同类型的中断是否路由至 INT1 引脚，方便系统对特定事件作出响应。
		Icm42688_Write_Reg(ICM42688_INT_SOURCE0,0x00);					// 设置UI AGC 准备好中断路由至 INT1
    	Icm42688_Write_Reg(ICM42688_FIFO_CONFIG2,0x00); 				// FIFO_WM低8位 FIFO 数据量达到或超过FIFO_WM 大小时生成中断  
    	Icm42688_Write_Reg(ICM42688_FIFO_CONFIG3,0x02); 				// FIFO_WM高4位  在选择此中断源前，需将本寄存器设置为非零值
		Icm42688_Write_Reg(ICM42688_INT_SOURCE0,Reg_Val);
		Icm42688_Write_Reg(ICM42688_FIFO_CONFIG1,0x63);					// 0110 0011 允许 FIFO 部分读取，并从上次读取点继续 FIFO 水印阈值触发模式 允许陀螺仪,加速度计数据包进入 FIFO
	
		Icm42688_Write_Reg(ICM42688_REG_BANK_SEL,0x00);
		Icm42688_Write_Reg(ICM42688_INT_CONFIG,0x36);					// 0011 0110 INT2中断 锁存模式，推挽输出，低电平有效，INT1中断和INT2一样

		Icm42688_Write_Reg(ICM42688_REG_BANK_SEL,0x00);
		Reg_Val = Icm42688_Read_Reg(ICM42688_INT_SOURCE0);				// 读取中断资源
		Reg_Val |= (1 << 2);											// 0000 0100	此值设置了FIFO 阈值中断路由至 INT1
		Icm42688_Write_Reg(ICM42688_INT_SOURCE0,Reg_Val);				// 给中断源0寄存器写入0x04,使能FIFO溢出中断到INT1
		//设置加速度计的量程和输出速率
		AccSensitivity=Icm42688GetAres(AFS_8G);							// 设置加速度计的最小精度
		Icm42688_Write_Reg(ICM42688_REG_BANK_SEL,0x00);					// 选择寄存器区域0
		Reg_Val=Icm42688_Read_Reg(ICM42688_ACCEL_CONFIG0);				// 先读取加速度配置寄存器0的值
		Reg_Val|=(AFS_8G<<5);											// 加速度计的量程设置位8G	5-7位设置量程
		Reg_Val|=(AODR_50Hz);											// 加速度计设置输出速率为50Hz	0-3位设置输出速率
    	Icm42688_Write_Reg(ICM42688_ACCEL_CONFIG0,Reg_Val);				// 将设置好的Reg_Val赋值给ICM42688_ACCEL_CONFIG0寄存器
		//设置陀螺仪的量程和输出速率
		GyroSensitivity=Icm42688GetGres(GFS_1000DPS);					// 设置陀螺仪的最小精度
		Icm42688_Write_Reg(ICM42688_REG_BANK_SEL,0x00);					// 选择寄存器区域0
		Reg_Val=Icm42688_Read_Reg(ICM42688_GYRO_CONFIG0);				// 先读取陀螺仪配置寄存器0的值
		Reg_Val|=(GFS_1000DPS<<5);   									// 陀螺仪量程设置位±1000dps	5-7位设置量程
		Reg_Val|=(GODR_50Hz);     										// 陀螺仪输出速率设置为50HZ	0-3位设置输出速率
		Icm42688_Write_Reg(ICM42688_GYRO_CONFIG0,Reg_Val);				// 将设置好的Reg_Val赋值ICM42688_GYRO_CONFIG0寄存器
		
		Icm42688_Write_Reg(ICM42688_REG_BANK_SEL,0x00);					// 选择寄存器区域0
		Reg_Val=Icm42688_Read_Reg(ICM42688_PWR_MGMT0); 					// 读取PWR—MGMT0当前寄存器的值(page72)
		Reg_Val&=~(1 << 5);												// 使能温度测量
		Reg_Val|=((3) << 2);											// 设置GYRO_MODE  0:关闭 1:待机 2:预留 3:低噪声
		Reg_Val|=(3);													// 设置ACCEL_MODE 0:关闭 1:关闭 2:低功耗 3:低噪声
		Icm42688_Write_Reg(ICM42688_PWR_MGMT0,Reg_Val);					// 将配置好的Reg_Val赋值给ICM42688_PWR_MGMT0寄存器
		Delay_Ms(1); 													// 操作完PWR—MGMT0寄存器后 200us内不能有任何读写寄存器的操作
		
		return 0;														// 读取到ID号 	 配置寄存器成功返回0
	}
	return -1;															// 未读取到ID号 配置寄存器失败返回-1
}

/*
参数  : 无
函数体: ICM42688初始化
返回值: 0: 初始化成功  其他值: 初始化失败
*/
int8_t ICM42688_Init(void)
{
	MySPI_Init();
	Delay_Ms(5);
	return (Icm42688RegCfg());
}

/*
参数  : 接收温度数据的数
函数体: 读取ICM42688内部的温度
返回值: 无
*/
void ICMGetTemp(int16_t *pTemp)
{
	uint8_t buffer[2]={0};
	Icm42688_Read_Regs(ICM42688_TEMP_DATA1,buffer,2);					// 温度数据的高字节,连续测量温度
	*pTemp=(int16_t)(((int16_t)((buffer[0]<<8)|buffer[1]))/132.48+25);
}

/*
参数  : 接收加速度计数据的结构体
函数体: 读取ICM42688加速度计的值
返回值: 无
*/
void ICMGetAccelerometer(ICM42688RawData_t* AccData)
{
	uint8_t buffer[6]={0};
	Icm42688_Read_Regs(ICM42688_ACCEL_DATA_X1,buffer,6);				// 第一位是加速度计 X 轴数据的高字节,连续读六次
	//读取三轴的加速度
	AccData->x=((uint16_t)buffer[0]<<8)|buffer[1];
	AccData->y=((uint16_t)buffer[2]<<8)|buffer[3];
	AccData->z=((uint16_t)buffer[4]<<8)|buffer[5];
	//对原始数据进行处理
	AccData->x=(int16_t)(AccData->x*AccSensitivity);					// 把三轴的原始测量数据和加速度计的最小精度相乘，
	AccData->y=(int16_t)(AccData->y*AccSensitivity);					// 从而得到三轴方向上实际的加速度值。
	AccData->z=(int16_t)(AccData->z*AccSensitivity);
}

/*
参数  : 接收陀螺仪数据的结构体
函数体: 读取ICM42688陀螺仪的值
返回值: 无
*/
void ICMGetGyroscope(ICM42688RawData_t* GroyData)
{
	uint8_t buffer[6]={0};
	Icm42688_Read_Regs(ICM42688_GYRO_DATA_X1,buffer,6);
	//读取三轴陀螺仪
	GroyData->x=((uint16_t)buffer[0]<<8)|buffer[1];
	GroyData->y=((uint16_t)buffer[2]<<8)|buffer[3];
	GroyData->z=((uint16_t)buffer[4]<<8)|buffer[5];
	//对原始数据进行处理
	GroyData->x=(int16_t)(GroyData->x*GyroSensitivity);				// 把三轴的原始测量数据和加速度计的最小精度相乘，
	GroyData->y=(int16_t)(GroyData->y*GyroSensitivity);				// 从而得到三轴方向上实际的加速度值。
	GroyData->z=(int16_t)(GroyData->z*GyroSensitivity);
}

/*
参数  : 接收加速度计数据的结构体，接收陀螺仪数据的结构体
函数体: 读取ICM42688加速度陀螺仪的值
返回值: 无
*/
void ICMGetRawData(ICM42688RawData_t* AccData,ICM42688RawData_t* GroyData)
{
	uint8_t buffer[12] = {0};
	Icm42688_Read_Regs(ICM42688_ACCEL_DATA_X1,buffer,12);
	// 获取原始数据
	AccData->x=((uint16_t)buffer[0]<<8)|buffer[1];
	AccData->y=((uint16_t)buffer[2]<<8)|buffer[3];
	AccData->z=((uint16_t)buffer[4]<<8)|buffer[5];
	GroyData->x=((uint16_t)buffer[6]<<8)|buffer[7];
	GroyData->y=((uint16_t)buffer[8]<<8)|buffer[9];
	GroyData->z=((uint16_t)buffer[10]<<8)|buffer[11];
	// 处理原始数据
	AccData->x=(int16_t)(AccData->x*AccSensitivity);					// 把三轴的原始测量数据和加速度计的最小精度相乘，
	AccData->y=(int16_t)(AccData->y*AccSensitivity);					// 从而得到三轴方向上实际的加速度值。
	AccData->z=(int16_t)(AccData->z*AccSensitivity);
	GroyData->x=(int16_t)(GroyData->x*GyroSensitivity);					// 把三轴的原始测量数据和加速度计的最小精度相乘，
	GroyData->y=(int16_t)(GroyData->y*GyroSensitivity);					// 从而得到三轴方向上实际的加速度值。
	GroyData->z=(int16_t)(GroyData->z*GyroSensitivity);

	AccData->x=-AccData->x;
	AccData->y=-AccData->y;
	AccData->z=-AccData->z;
	GroyData->x=-GroyData->x;
	GroyData->y=-GroyData->y;

}

/*
参数  : 无
函数体: 读取获取欧拉角
返回值: 无
*/
void ICM_GetAngle(void)
{
	ICMGetRawData(Test_AccData,Test_GyroData);
	IMUupdate((float)Test_GyroData->x*500/57.3f/65536, (float)Test_GyroData->y*500/57.3f/65536, (float)Test_GyroData->z*500/57.3f/65536,
						(float)Test_AccData->x*9.8f/16384, (float)Test_AccData->y*9.8f/16384, (float)Test_AccData->z*9.8f/16384);
	
}

/*
参数  : pitch：俯仰角
		roll ：横滚角
函数体: 计算获取获取倾斜角
返回值: 无
*/
void calculate_tilt_angle(float pitch, float roll)
{
	if(pitch<0) pitch=-pitch;
	if(roll<0) roll=-roll;
    // 小角度近似：tilt ≈ sqrt(pitch? + roll?)
    float pitch_sq = pitch * pitch;
    float roll_sq = roll * roll;
    float sum_sq = pitch_sq + roll_sq;

	Qingxiejiao=sqrt(sum_sq);
	// if(Qingxiejiao>=0&&Qingxiejiao<=90)
	// {
	// 	Qingxiejiao=180-Qingxiejiao;
	// }
	// else if(Qingxiejiao>90&&Qingxiejiao<=180)
	// {
	// 	Qingxiejiao=-(Qingxiejiao-180);
	// }
}





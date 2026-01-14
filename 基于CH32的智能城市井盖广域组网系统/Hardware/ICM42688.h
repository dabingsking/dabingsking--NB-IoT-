#ifndef __ICM42688_H
#define __ICM42688_H

#include <stdint.h>

//ICM42688 REG
//BANK0
#define ICM42688_DEVICE_CONFIG					0x11						// 设备配置,SPI模式选择和软件复位配置
#define	ICM42688_DRIVE_CONFIG					0x13						// 驱动配置，设置输出引脚14的压摆率，压摆率指的是信号(电压信号)变化的最大速率
#define	ICM42688_INT_CONFIG						0x14						// 中断配置，有两个中断，INT1，INT2
#define	ICM42688_FIFO_CONFIG					0x16						// FIFO 配置,队列工作模式配置
#define	ICM42688_TEMP_DATA1						0x1D						// 温度数据的高字节，测量ICM自身的温度，用于性能评估和热管理
#define	ICM42688_TEMP_DATA0						0x1E						// 温度数据的低字节
#define	ICM42688_ACCEL_DATA_X1					0x1F						// 加速度计 X 轴数据的高字节（15 至 8 位）
#define	ICM42688_ACCEL_DATA_X0					0x20						// 加速度计 X 轴数据的低字节（7 至 0 位）
#define	ICM42688_ACCEL_DATA_Y1					0x21						// 加速度计 Y 轴数据的高字节（15 至 8 位）
#define	ICM42688_ACCEL_DATA_Y0					0x22						// 加速度计 Y 轴数据的低字节（7 至 0 位）
#define	ICM42688_ACCEL_DATA_Z1					0x23						// 加速度计 Z 轴数据的高字节（15 至 8 位）
#define	ICM42688_ACCEL_DATA_Z0					0x24						// 加速度计 Z 轴数据的低字节（7 至 0 位）
#define	ICM42688_GYRO_DATA_X1					0x25						// 陀螺仪 X 轴数据的高字节（15 至 8 位）
#define	ICM42688_GYRO_DATA_X0					0x26						// 陀螺仪 X 轴数据的低字节（7 至 0 位）
#define	ICM42688_GYRO_DATA_Y1					0x27						// 陀螺仪 Y 轴数据的高字节（15 至 8 位）
#define	ICM42688_GYRO_DATA_Y0					0x28						// 陀螺仪 Y 轴数据的低字节（7 至 0 位）
#define	ICM42688_GYRO_DATA_Z1					0x29						// 陀螺仪 Z 轴数据的高字节（15 至 8 位）
#define	ICM42688_GYRO_DATA_Z0					0x2A						// 陀螺仪 Z 轴数据的低字节（7 至 0 位）
#define	ICM42688_TMST_FSYNCH					0x2B						// 同步脉冲时间戳高字节.存储从 FSYNC 信号上升沿到最近一次输出数据速率（ODR）时刻的时间差的高字节，直到用户接口（UI）从状态寄存器中读取到 FSYNC 标签。
#define	ICM42688_TMST_FSYNCL					0x2C						// 同步脉冲时间戳低字节.存储从 FSYNC 信号上升沿到最近一次输出数据速率（ODR）时刻的时间差的低字节，直到用户接口（UI）从状态寄存器中读取到 FSYNC 标签。
#define	ICM42688_INT_STATUS						0x2D						// 中断状态寄存器
#define	ICM42688_FIFO_COUNTH					0x2E						// FIFO 计数高字节寄存器（FIFO_COUNTH）
#define	ICM42688_FIFO_COUNTL					0x2F						// FIFO 计数低字节寄存器（FIFO_COUNTL）
#define	ICM42688_FIFO_DATA						0x30						// FIFO 数据寄存器（FIFO_DATA）
#define	ICM42688_APEX_DATA0						0x31						// APEX 数据 0 寄存器
#define	ICM42688_APEX_DATA1						0x32						// APEX 数据 1 寄存器
#define	ICM42688_APEX_DATA2						0x33						// APEX 数据 2 寄存器
#define	ICM42688_APEX_DATA3						0x34						// APEX 数据 3 寄存器
#define	ICM42688_APEX_DATA4						0x35						// APEX 数据 4 寄存器
#define	ICM42688_APEX_DATA5						0x36						// APEX 数据 5 寄存器
#define	ICM42688_INT_STATUS2					0x37						// 中断状态寄存器 2
#define	ICM42688_INT_STATUS3					0x38						// 中断状态寄存器 3	
#define	ICM42688_SIGNAL_PATH_RESET				0x4B						// 信号路径复位寄存器
#define	ICM42688_INTF_CONFIG0					0x4C						// 接口配置寄存器 0
#define	ICM42688_INTF_CONFIG1					0x4D						// 接口配置寄存器 1
#define	ICM42688_PWR_MGMT0						0x4E						// 电源管理 0 寄存器
#define	ICM42688_GYRO_CONFIG0					0x4F						// 陀螺仪配置寄存器 0
#define	ICM42688_ACCEL_CONFIG0					0x50						// 加速度计配置寄存器 0
#define	ICM42688_GYRO_CONFIG1					0x51						// 陀螺仪配置寄存器 1
#define	ICM42688_GYRO_ACCEL_CONFIG0				0x52						// 陀螺仪与加速度计配置寄存器 0
#define	ICM42688_ACCEL_CONFIG1					0x53						// 加速度计配置寄存器 1
#define	ICM42688_TMST_CONFIG					0x54						// 时间戳配置寄存器
#define	ICM42688_APEX_CONFIG0					0x56						// APEX 配置寄存器 0
#define	ICM42688_SMD_CONFIG						0x57						// 重大运动检测配置寄存器
#define	ICM42688_FIFO_CONFIG1					0x5F						// FIFO 配置寄存器 1
#define	ICM42688_FIFO_CONFIG2					0x60						// FIFO 配置寄存器 2
#define	ICM42688_FIFO_CONFIG3					0x61						// FIFO 配置寄存器 3
#define	ICM42688_FSYNC_CONFIG					0x62						// FSYNC 配置寄存器
#define	ICM42688_INT_CONFIG0					0x63						// 中断配置寄存器 0
#define	ICM42688_INT_CONFIG1					0x64						// 中断配置寄存器 1
#define	ICM42688_INT_SOURCE0					0x65						// 中断源寄存器 0
#define	ICM42688_INT_SOURCE1					0x66						// 中断源寄存器 1
#define	ICM42688_INT_SOURCE3					0x68						// 中断源寄存器 3
#define	ICM42688_INT_SOURCE4					0x69						// 中断源寄存器 4
#define	ICM42688_FIFO_LOST_PKT0					0x6C						// FIFO 丢失数据包寄存器 0
#define	ICM42688_FIFO_LOST_PKT1					0x6D						// FIFO 丢失数据包寄存器 1
#define	ICM42688_SELF_TEST_CONFIG				0x70						// 自检配置寄存器
#define	ICM42688_WHO_AM_I						0x75						// 设备标识寄存器
#define ICM42688_REG_BANK_SEL					0x76						// 寄存器组选择寄存器

//BANK1
#define ICM42688_SENSOR_CONFIG0					0x03						// 传感器配置寄存器 0
#define ICM42688_GYRO_CONFIG_STATIC2			0x0B						// 陀螺仪静态配置寄存器 2
#define ICM42688_GYRO_CONFIG_STATIC3			0x0C						// 陀螺仪静态配置寄存器 3
#define ICM42688_GYRO_CONFIG_STATIC4			0x0D						// 陀螺仪静态配置寄存器 4
#define ICM42688_GYRO_CONFIG_STATIC5			0x0E						// 陀螺仪静态配置寄存器 5
#define ICM42688_GYRO_CONFIG_STATIC6			0x0F						// 陀螺仪静态配置寄存器 6
#define ICM42688_GYRO_CONFIG_STATIC7			0x10						// 陀螺仪静态配置寄存器 7
#define ICM42688_GYRO_CONFIG_STATIC8			0x11						// 陀螺仪静态配置寄存器 8
#define ICM42688_GYRO_CONFIG_STATIC9			0x12						// 陀螺仪静态配置寄存器 9
#define ICM42688_GYRO_CONFIG_STATIC10			0x13						// 陀螺仪静态配置寄存器 10
#define ICM42688_XG_ST_DATA						0x5F						// X 轴陀螺仪自测试数据寄存器
#define ICM42688_YG_ST_DATA						0x60						// Y 轴陀螺仪自测试数据寄存器
#define ICM42688_ZG_ST_DATA						0x61						// Z 轴陀螺仪自测试数据寄存器
#define ICM42688_TMSTVAL0						0x62						// 时间戳值寄存器 0
#define ICM42688_TMSTVAL1						0x63						// 时间戳值寄存器 1
#define ICM42688_TMSTVAL2						0x64						// 时间戳值寄存器 2
#define ICM42688_INTF_CONFIG4					0x7A						// 接口配置寄存器 4
#define ICM42688_INTF_CONFIG5					0x7B						// 接口配置寄存器 5
#define ICM42688_INTF_CONFIG6					0x7C						// 接口配置寄存器 6

//BANK2
#define ICM42688_ACCEL_CONFIG_STATIC2			0x03						// 加速度计静态配置寄存器 2
#define ICM42688_ACCEL_CONFIG_STATIC3			0x04						// 加速度计静态配置寄存器 3
#define ICM42688_ACCEL_CONFIG_STATIC4			0x05						// 加速度计静态配置寄存器 4
#define ICM42688_XA_ST_DATA						0x3B						// X 轴加速度计自测试数据寄存器
#define ICM42688_YA_ST_DATA						0x3C						// Y 轴加速度计自测试数据寄存器
#define ICM42688_ZA_ST_DATA						0x3D						// Z 轴加速度计自测试数据寄存器

//BANK4
#define ICM42688_APEX_CONFIG1					0x40						// APEX 配置寄存器 1
#define ICM42688_APEX_CONFIG2					0x41						// APEX 配置寄存器 2
#define ICM42688_APEX_CONFIG3					0x42						// APEX 配置寄存器 3
#define ICM42688_APEX_CONFIG4					0x43						// APEX 配置寄存器 4
#define ICM42688_APEX_CONFIG5					0x44						// APEX 配置寄存器 5
#define ICM42688_APEX_CONFIG6					0x45						// APEX 配置寄存器 6
#define ICM42688_APEX_CONFIG7					0x46						// APEX 配置寄存器 7
#define ICM42688_APEX_CONFIG8					0x47						// APEX 配置寄存器 8
#define ICM42688_APEX_CONFIG9					0x48						// APEX 配置寄存器 9
#define ICM42688_ACCEL_WOM_X_THR				0x4A						// 加速度计 X 轴运动唤醒阈值寄存器
#define ICM42688_ACCEL_WOM_Y_THR				0x4B						// 加速度计 Y 轴运动唤醒阈值寄存器
#define ICM42688_ACCEL_WOM_Z_THR				0x4C						// 加速度计 Z 轴运动唤醒阈值寄存器
#define ICM42688_INT_SOURCE6					0x4D						// 中断源寄存器 6
#define ICM42688_INT_SOURCE7					0x4E						// 中断源寄存器 7
#define ICM42688_INT_SOURCE8					0x4F						// 中断源寄存器 8
#define ICM42688_INT_SOURCE9					0x50						// 中断源寄存器 9
#define ICM42688_INT_SOURCE10					0x51						// 中断源寄存器 10
#define ICM42688_OFFSET_USER0					0x77						// 用户零偏寄存器 0
#define ICM42688_OFFSET_USER1					0x78						// 用户零偏寄存器 1
#define ICM42688_OFFSET_USER2					0x79						// 用户零偏寄存器 2
#define ICM42688_OFFSET_USER3					0x7A						// 用户零偏寄存器 3
#define ICM42688_OFFSET_USER4					0x7B						// 用户零偏寄存器 4
#define ICM42688_OFFSET_USER5					0x7C						// 用户零偏寄存器 5
#define ICM42688_OFFSET_USER6					0x7D						// 用户零偏寄存器 6
#define ICM42688_OFFSET_USER7					0x7E						// 用户零偏寄存器 7
#define ICM42688_OFFSET_USER8					0x7F						// 用户零偏寄存器 8

#define AFS_2G									0x03						// 加速度计的量程选择范围2G-16G
#define AFS_4G									0x02
#define AFS_8G									0x01
#define AFS_16G									0x00

#define GFS_15_125DPS 							0x07						// 陀螺仪量程选择范围15.125-2000 °/SEG
#define GFS_31_25DPS  							0x06
#define GFS_62_5DPS   							0x05
#define GFS_125DPS    							0x04
#define GFS_250DPS    							0x03
#define GFS_500DPS    							0x02
#define GFS_1000DPS   							0x01
#define GFS_2000DPS   							0x00 						

#define AODR_8000Hz								0x03						// 加速度计用户接口输出的输出数据速率
#define AODR_4000Hz								0x04
#define AODR_2000Hz								0x05
#define AODR_1000Hz								0x06
#define AODR_200Hz								0x07
#define AODR_100Hz								0x08
#define AODR_50Hz								0x09
#define AODR_25Hz								0x0A
#define AODR_12_5Hz								0x0B
#define AODR_6_25Hz								0x0C
#define AODR_3_125Hz							0x0D
#define AODR_1_5625Hz							0x0E
#define AODR_500Hz								0x0F

#define GODR_8000Hz								0x03						// 陀螺仪用户接口输出的输出数据速率
#define GODR_4000Hz								0x04
#define GODR_2000Hz								0x05
#define GODR_1000Hz								0x06
#define GODR_200Hz								0x07
#define GODR_100Hz								0x08
#define GODR_50Hz								0x09
#define GODR_25Hz								0x0A
#define GODR_12_5Hz								0x0B
#define GODR_500Hz								0x0F

#define ICM42688_ID	             				0x47						// 设备的ID



typedef struct
{
	int16_t x;															// x 轴方向上直接输出的 16 位有符号原始数值
	int16_t y;															// y 轴方向上直接输出的 16 位有符号原始数值
	int16_t z;															// z 轴方向上直接输出的 16 位有符号原始数值
}	ICM42688RawData_t;

int8_t Icm42688RegCfg(void);


extern ICM42688RawData_t *Test_AccData;									// 加速度的是数据结构体
extern ICM42688RawData_t *Test_GyroData;								// 陀螺仪的数据结构体
extern float Qingxiejiao;


int8_t ICM42688_Init(void);
void ICMGetTemp(int16_t *pTemp);
void ICMGetAccelerometer(ICM42688RawData_t* AccData);
void ICMGetGyroscope(ICM42688RawData_t* GroyData);
void ICMGetRawData(ICM42688RawData_t* AccData,ICM42688RawData_t* GroyData);
void ICM_GetAngle(void);
void calculate_tilt_angle(float pitch, float roll);

#endif

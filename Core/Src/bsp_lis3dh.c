/**
 * @file    bsp_lis3dh.c
 * @brief   LIS3DH 三轴加速度计驱动（I2C 接口，支持运动检测/门限中断）。
 *
 * 功能概述：
 * - 初始化 LIS3DH：量程、输出数据率、高通滤波、中断阈值与持续时间等。
 * - 提供原始加速度数据读取（Raw）与 g 单位转换接口。
 * - 提供 INT1 源读取函数，用于中断锁存清除与调试。
 *
 */

#include "bsp_lis3dh.h"
#include <math.h>

extern I2C_HandleTypeDef hi2c1;

// 寄存器地址定义（LIS3DH 数据手册）
#define LIS3DH_REG_CTRL1      (0x20u)
#define LIS3DH_REG_CTRL2      (0x21u)  // CTRL_REG2：高通滤波器配置
#define LIS3DH_REG_CTRL3      (0x22u)
#define LIS3DH_REG_CTRL4      (0x23u)
#define LIS3DH_REG_CTRL5      (0x24u)
#define LIS3DH_REG_CTRL6      (0x25u)  // CTRL_REG6：控制INT1/INT2极性
#define LIS3DH_REG_INT1_CFG   (0x30u)
#define LIS3DH_REG_INT1_SRC   (0x31u)
#define LIS3DH_REG_INT1_THS   (0x32u)  // INT1中断阈值寄存器
#define LIS3DH_REG_INT1_DURATION (0x33u)  // INT1中断持续时间寄存器（去抖）
#define LIS3DH_REG_REFERENCE  (0x26u)  // 高通滤波器参考寄存器（读取可复位HPF）
#define LIS3DH_REG_OUT_X_L    (0x28u)

// 错误重试配置
#define LIS3DH_RETRY_COUNT    (3u)     // 最多重试3次
#define LIS3DH_RETRY_DELAY_MS (10u)    // 每次重试间隔10ms

static inline uint16_t lis3dh_dev_addr(void)
{
  // HAL API 需要传入“8-bit 地址”（7-bit 左移 1 位）
  return (uint16_t)(LIS3DH_I2C_ADDR_7BIT << 1);
}

/**
 * @brief  写入LIS3DH寄存器（带重试机制）
 * @param  reg: 寄存器地址
 * @param  val: 要写入的值
 * @retval HAL_StatusTypeDef: HAL_OK=成功, HAL_ERROR=失败
 * @note   最多重试3次，每次间隔10ms，提高通信可靠性
 */
static HAL_StatusTypeDef lis3dh_write_u8(uint8_t reg, uint8_t val)
{
  HAL_StatusTypeDef status;
  uint8_t retry = LIS3DH_RETRY_COUNT;
  
  // 写入 1 字节寄存器，失败时重试
  do {
    status = HAL_I2C_Mem_Write(&hi2c1, lis3dh_dev_addr(), reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 100);
    if (status == HAL_OK) {
      return HAL_OK;
    }
    // 失败时重试
    if (retry > 0) {
      HAL_Delay(LIS3DH_RETRY_DELAY_MS);
      retry--;
    }
  } while (retry > 0);
  
  return HAL_ERROR;
}

/**
 * @brief  读取LIS3DH寄存器（带重试机制）
 * @param  reg: 寄存器地址
 * @param  buf: 数据缓冲区
 * @param  len: 读取长度
 * @retval HAL_StatusTypeDef: HAL_OK=成功, HAL_ERROR=失败
 * @note   最多重试3次，每次间隔10ms，提高通信可靠性（特别是唤醒后）
 */
static HAL_StatusTypeDef lis3dh_read(uint8_t reg, uint8_t *buf, uint16_t len)
{
  if (buf == NULL || len == 0) return HAL_ERROR;
  
  HAL_StatusTypeDef status;
  uint8_t retry = LIS3DH_RETRY_COUNT;
  
  // 连续读取 len 字节，失败时重试
  do {
    status = HAL_I2C_Mem_Read(&hi2c1, lis3dh_dev_addr(), reg, I2C_MEMADD_SIZE_8BIT, buf, len, 100);
    if (status == HAL_OK) {
      return HAL_OK;
    }
    // 失败时重试
    if (retry > 0) {
      HAL_Delay(LIS3DH_RETRY_DELAY_MS);
      retry--;
    }
  } while (retry > 0);
  
  return HAL_ERROR;
}

HAL_StatusTypeDef LIS3DH_ReadWhoAmI(uint8_t *whoami)
{
  if (whoami == NULL) return HAL_ERROR;
  // 读取 WHOAMI
  return lis3dh_read(LIS3DH_REG_WHOAMI, whoami, 1);
}

/**
 * @brief  LIS3DH初始化函数
 * @retval HAL_StatusTypeDef: HAL_OK=成功, HAL_ERROR=失败
 *
 * 寄存器配置说明：
 *   CTRL_REG4 = 0x98: BDU=1, FS=±4g, HR=1（高分辨率12-bit）
 *   CTRL_REG1 = 0x37: ODR=25Hz, LPen=0, XYZ使能
 *     - 25Hz 采样间隔40ms，足够捕捉冲击事件
 *   CTRL_REG2 = 0x09: FDS=1（HPF到输出寄存器）, HP_IA1=1（HPF到INT1）
 *     - 高通滤波消除重力直流分量，防止静止时INT1常高
 *     - 配置后必须读 REFERENCE 寄存器复位HPF内部状态
 *   INT1_THS  = 0x10: 16 LSB × 16mg/LSB = 256mg 阈值
 *   INT1_DURATION = 0x01: 1个采样周期 = 40ms（25Hz）
 *     - 原值0x02@10Hz=200ms，太长导致冲击事件无法触发
 *   INT1_CFG  = 0x3F: OR逻辑，6方向全使能
 *   CTRL_REG5 = 0x40: LIR_INT1=1（锁存，读INT1_SRC清除）
 *   CTRL_REG3 = 0x40: I1_IA1=1（IA1事件映射到INT1引脚）
 *   CTRL_REG6 = 0x00: INT1高电平有效
 */
HAL_StatusTypeDef LIS3DH_Init(void)
{
  HAL_StatusTypeDef status;
  uint8_t whoami = 0;

  // 步骤1：验证器件ID
  if (LIS3DH_ReadWhoAmI(&whoami) != HAL_OK) return HAL_ERROR;
  if (whoami != LIS3DH_WHOAMI_VALUE) return HAL_ERROR;

  // 步骤2：掉电模式，配置期间不产生中断
  status = lis3dh_write_u8(LIS3DH_REG_CTRL1, 0x07u);  // ODR=0 + XYZ使能
  if (status != HAL_OK) return status;

  // 步骤3：CTRL_REG4 = 0x98: BDU=1 + FS=±4g + HR=1
  status = lis3dh_write_u8(LIS3DH_REG_CTRL4, 0x98u);
  if (status != HAL_OK) return status;

  // 步骤4：CTRL_REG2 = 0x00: 关闭HPF，6D检测使用原始加速度
  status = lis3dh_write_u8(LIS3DH_REG_CTRL2, 0x00u);
  if (status != HAL_OK) return status;

  // 步骤5：INT1_THS = 0x21: 33×16mg = 528mg ≈ 0.5g
  // 6D模式下：某轴加速度超过此阈值则认为重力矢量指向该面
  // 正常关盖 az≈1g > 0.5g → ZH面激活
  // 开盖翻转后水平轴>0.5g → 切换到新的面 → 触发中断
  status = lis3dh_write_u8(LIS3DH_REG_INT1_THS, 0x21u);
  if (status != HAL_OK) return status;

  // 步骤6：INT1_DURATION = 0x03: 3×40ms = 120ms 去抖
  // 防止轻微晃动误触发，要求姿态稳定保持120ms才确认
  status = lis3dh_write_u8(LIS3DH_REG_INT1_DURATION, 0x03u);
  if (status != HAL_OK) return status;

  // 步骤7：INT1_CFG = 0xEA: 6D位置识别模式
  // 0xEA = 1110_1010
  // bit7(AOI)=1 + bit6(6D)=1 → 6D方向检测模式
  // bit5(ZHIE)=1 bit4(ZLIE)=0 → 检测ZH（Z轴朝上）
  // bit3(YHIE)=1 bit2(YLIE)=0 → 检测YH（Y轴正方向）
  // bit1(XHIE)=1 bit0(XLIE)=0 → 检测XH（X轴正方向）
  // 6D模式下任意面切换时INT1_SRC.IA=1，INT1引脚产生上升沿
  // 正常关盖：az≈1g → ZH面激活；开盖倾斜后水平轴>0.5g → 面切换 → 触发
  status = lis3dh_write_u8(LIS3DH_REG_INT1_CFG, 0xEAu);
  if (status != HAL_OK) return status;

  // 步骤8：CTRL_REG5 = 0x40: LIR_INT1=1（锁存，读INT1_SRC清除）
  status = lis3dh_write_u8(LIS3DH_REG_CTRL5, 0x40u);
  if (status != HAL_OK) return status;

  // 步骤9：CTRL_REG3 = 0x40: I1_IA1=1（IA1事件映射到INT1引脚）
  status = lis3dh_write_u8(LIS3DH_REG_CTRL3, 0x40u);
  if (status != HAL_OK) return status;

  // 步骤10：CTRL_REG6 = 0x00: INT1高电平有效
  status = lis3dh_write_u8(LIS3DH_REG_CTRL6, 0x00u);
  if (status != HAL_OK) return status;

  // 步骤11：启动ODR = 25Hz（CTRL_REG1 = 0x37）
  // 最后写，确保所有中断配置就绪后再开始采样
  status = lis3dh_write_u8(LIS3DH_REG_CTRL1, 0x37u);
  if (status != HAL_OK) return status;

  // 步骤12：等待2个采样周期（25Hz → 80ms），让6D状态机稳定
  HAL_Delay(80);

  // 步骤13：读取INT1_SRC清除上电期间产生的初始状态切换中断
  // 上电时6D状态机从"未知"切换到"ZH面"会产生一次中断，必须清除
  // 否则进入STOP2前INT1已为高，无法产生上升沿唤醒
  {
    uint8_t src_dummy = 0;
    (void)lis3dh_read(LIS3DH_REG_INT1_SRC, &src_dummy, 1);
  }

  return HAL_OK;
}

/**
 * @brief  读取加速度原始值（16位有符号整数）
 * @param  x: X轴加速度原始值输出（-32768~32767）
 * @param  y: Y轴加速度原始值输出（-32768~32767）
 * @param  z: Z轴加速度原始值输出（-32768~32767）
 * @retval HAL_StatusTypeDef: HAL_OK=成功, HAL_ERROR=失败
 * @note   原始值需根据量程换算为实际加速度
 *         使用LIS3DH_ReadAccelG()可直接获得g单位的值
 */
HAL_StatusTypeDef LIS3DH_ReadAccelRaw(int16_t *x, int16_t *y, int16_t *z)
{
  if (x == NULL || y == NULL || z == NULL) return HAL_ERROR;

  // 连读 X/Y/Z 六个字节：设置 bit7 开启地址自增
  uint8_t buf[6] = {0};
  HAL_StatusTypeDef st = lis3dh_read((uint8_t)(LIS3DH_REG_OUT_X_L | 0x80u), buf, sizeof(buf));
  if (st != HAL_OK) return st;

  // LIS3DH 输出为 little-endian
  *x = (int16_t)((uint16_t)buf[1] << 8 | buf[0]);
  *y = (int16_t)((uint16_t)buf[3] << 8 | buf[2]);
  *z = (int16_t)((uint16_t)buf[5] << 8 | buf[4]);
  return HAL_OK;
}

/**
 * @brief  读取加速度值（g单位，浮点数）
 * @note   此函数内部调用LIS3DH_ReadAccelRaw()获取原始值后进行转换
 * @param  x_g: X轴加速度输出（g单位）
 * @param  y_g: Y轴加速度输出（g单位）
 * @param  z_g: Z轴加速度输出（g单位）
 * @retval HAL_StatusTypeDef: HAL_OK=成功, HAL_ERROR=失败
 * @note   根据±4g量程、高分辨率模式进行转换：
 *         - CTRL4=0x98：高分辨率(12-bit)，±4g 量程
 *         - 数据为左对齐格式：有效 12 位在高位，需要先右移4位得到raw12
 *         - 灵敏度：±4g 高分辨率模式下 1 LSB ≈ 2 mg = 0.002 g
 */
HAL_StatusTypeDef LIS3DH_ReadAccelG(float *x_g, float *y_g, float *z_g)
{
  if (x_g == NULL || y_g == NULL || z_g == NULL) return HAL_ERROR;
  
  int16_t x_raw = 0, y_raw = 0, z_raw = 0;
  HAL_StatusTypeDef status = LIS3DH_ReadAccelRaw(&x_raw, &y_raw, &z_raw);
  if (status != HAL_OK) return status;

  // 高分辨率模式下的数据为 12-bit 左对齐：有效位在[15:4]
  // 先右移4位得到 12-bit 有效数据，再按灵敏度 2 mg/LSB 转换为 g 单位
  const float LSB_TO_G = 0.002f;  // ±4g 高分辨率模式：1 LSB ≈ 2 mg = 0.002 g

  int16_t x12 = (int16_t)(x_raw >> 4);
  int16_t y12 = (int16_t)(y_raw >> 4);
  int16_t z12 = (int16_t)(z_raw >> 4);

  *x_g = (float)x12 * LSB_TO_G;
  *y_g = (float)y12 * LSB_TO_G;
  *z_g = (float)z12 * LSB_TO_G;

  return HAL_OK;
}

/**
 * @brief  读取INT1中断源寄存器
 * @param  src: 中断源寄存器值输出
 * @retval HAL_StatusTypeDef: HAL_OK=成功, HAL_ERROR=失败
 * @note   若开启锁存（CTRL_REG5.LIR_INT1=1），读取此寄存器可清除中断标志
 */
HAL_StatusTypeDef LIS3DH_ReadInt1Src(uint8_t *src)
{
  if (src == NULL) return HAL_ERROR;
  // 读取 INT1_SRC：若开启锁存，可同时用于"读取并清除"中断源
  return lis3dh_read(LIS3DH_REG_INT1_SRC, src, 1);
}

/**
 * @brief  计算俯仰角和横滚角，判断井盖是否开启
 * @param  tilt  输出结构体（pitch_deg, roll_deg, cover_open）
 * @retval HAL_OK / HAL_ERROR
 * @note   基于静态重力分量，适用于缓慢姿态变化（井盖开合）
 *         Z轴朝上为正常安装方向（az≈+1g时 pitch=roll≈0）
 *         开盖判断：|pitch| > 45° 或 |roll| > 45°
 */
HAL_StatusTypeDef LIS3DH_CalcTiltAngle(LIS3DH_Tilt_t *tilt)
{
  if (tilt == NULL) return HAL_ERROR;

  float ax = 0.f, ay = 0.f, az = 0.f;
  if (LIS3DH_ReadAccelG(&ax, &ay, &az) != HAL_OK) return HAL_ERROR;

  // pitch：绕Y轴，ax / sqrt(ay²+az²)
  // roll ：绕X轴，-ay / az
  tilt->pitch_deg = atan2f(ax, sqrtf(ay * ay + az * az)) * (180.0f / 3.14159265f);
  tilt->roll_deg  = atan2f(-ay, az) * (180.0f / 3.14159265f);

  float abs_pitch = (tilt->pitch_deg < 0.f) ? -tilt->pitch_deg : tilt->pitch_deg;
  float abs_roll  = (tilt->roll_deg  < 0.f) ? -tilt->roll_deg  : tilt->roll_deg;
  tilt->cover_open = ((abs_pitch > 45.0f) || (abs_roll > 45.0f)) ? 1u : 0u;

  return HAL_OK;
}


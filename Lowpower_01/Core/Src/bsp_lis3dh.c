#include "bsp_lis3dh.h"

// I2C 句柄在 main.c（CubeMX 生成）中定义，这里通过 extern 引用
extern I2C_HandleTypeDef hi2c1;

// 寄存器地址定义（LIS3DH 数据手册）
#define LIS3DH_REG_CTRL1      (0x20u)
#define LIS3DH_REG_CTRL3      (0x22u)
#define LIS3DH_REG_CTRL4      (0x23u)
#define LIS3DH_REG_CTRL5      (0x24u)
#define LIS3DH_REG_CTRL6      (0x25u)  // CTRL_REG6：控制INT1/INT2极性
#define LIS3DH_REG_INT1_CFG   (0x30u)
#define LIS3DH_REG_INT1_SRC   (0x31u)
#define LIS3DH_REG_INT1_THS   (0x32u)  // INT1中断阈值寄存器
#define LIS3DH_REG_INT1_DURATION (0x33u)  // INT1中断持续时间寄存器（去抖）
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
  
  // 通过 I2C Memory Write 写入 1 字节寄存器，失败时重试
  do {
    status = HAL_I2C_Mem_Write(&hi2c1, lis3dh_dev_addr(), reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 100);
    if (status == HAL_OK) {
      return HAL_OK;
    }
    // 失败时等待后重试
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
  
  // 通过 I2C Memory Read 连续读取 len 字节，失败时重试
  do {
    status = HAL_I2C_Mem_Read(&hi2c1, lis3dh_dev_addr(), reg, I2C_MEMADD_SIZE_8BIT, buf, len, 100);
    if (status == HAL_OK) {
      return HAL_OK;
    }
    // 失败时等待后重试（唤醒后I2C可能需要时间恢复）
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
  // 读取 WHOAMI（0x0F），正常应返回 0x33
  return lis3dh_read(LIS3DH_REG_WHOAMI, whoami, 1);
}

/**
 * @brief  LIS3DH初始化函数
 * @retval HAL_StatusTypeDef: HAL_OK=成功, HAL_ERROR=失败
 * @note   按照ARCHITECTURE.md第14章配置顺序：
 *         1. 检查器件ID
 *         2. 配置量程（CTRL_REG4）
 *         3. 配置数据输出率与低功耗模式（CTRL_REG1）
 *         4. 配置中断阈值（INT1_THS）
 *         5. 配置中断持续时间（INT1_DURATION）
 *         6. 配置中断逻辑（INT1_CFG）
 *         7. 使能INT1锁存（CTRL_REG5）
 *         8. 映射中断源到INT1（CTRL_REG3）
 */
HAL_StatusTypeDef LIS3DH_Init(void)
{
  HAL_StatusTypeDef status;
  uint8_t whoami = 0;
  
  // 步骤1：读ID，先确保器件在线且地址正确
  if (LIS3DH_ReadWhoAmI(&whoami) != HAL_OK) return HAL_ERROR;
  if (whoami != LIS3DH_WHOAMI_VALUE) return HAL_ERROR;

  // 步骤2：配置量程和分辨率（CTRL_REG4）
  // 选择与GitHub示例一致的高分辨率工作模式：
  // - BDU=1：块数据更新，避免读到"半更新"的数据
  // - FS=01：±4g 量程
  // - HR=1：高分辨率模式（12-bit，有效分辨率更高）
  // 0x98 = 0b1001_1000 -> BDU=1 + FS=01(±4g) + HR=1
  status = lis3dh_write_u8(LIS3DH_REG_CTRL4, 0x98u);
  if (status != HAL_OK) return status;

  // 步骤3：配置数据输出率与轴使能（CTRL_REG1）
  // 使用普通/高分辨率模式（不使能低功耗），与多数示例工程保持一致：
  // - ODR=10Hz：输出数据率 10Hz
  // - LPen=0 ：普通/高分辨率模式
  // - Zen=Yen=Xen=1：三轴全部使能
  // 0x27 = 0b0010_0111 -> ODR=10Hz + 普通模式 + XYZ 轴使能
  status = lis3dh_write_u8(LIS3DH_REG_CTRL1, 0x27u);
  if (status != HAL_OK) return status;

  // 步骤4：配置中断阈值（INT1_THS = 0x32）
  // 阈值 = 0x03 = 3 LSB = 48 mg（±4g量程，LSB=16 mg）
  // 进一步降低阈值以提高敏感度：确保轻微碰撞也能触发中断
  // 如果误触发太多，可以提高到 0x05 (80mg) 或 0x08 (128mg)
  status = lis3dh_write_u8(LIS3DH_REG_INT1_THS, 0x03u);
  if (status != HAL_OK) return status;

  // 步骤5：配置中断持续时间（INT1_DURATION = 0x33）
  // 持续时间 = 0x02 = 2个采样周期 = 2 / 10Hz = 0.2秒
  // 增加持续时间以提高可靠性：确保碰撞事件能被可靠捕获，减少漏检
  status = lis3dh_write_u8(LIS3DH_REG_INT1_DURATION, 0x02u);
  if (status != HAL_OK) return status;

  // 步骤6：配置中断逻辑（INT1_CFG = 0x30）
  // OR逻辑 + 全轴/方向使能（文档推荐 0x3F）
  // 任意轴任意方向超过阈值即触发，适合"震动/冲击"检测
  status = lis3dh_write_u8(LIS3DH_REG_INT1_CFG, 0x3Fu);
  if (status != HAL_OK) return status;

  // 步骤7：配置INT1锁存（CTRL_REG5 = 0x24）
  // 针对碰撞检测优化：使用锁存模式以提高可靠性
  // - LIR_INT1=1：INT1 输出锁存，直到读取 INT1_SRC 寄存器才清除
  // 这样可以确保中断信号保持，即使主循环处理延迟也不会丢失
  // 0x40 = 0b0100_0000 -> LIR_INT1=1（锁存模式）
  status = lis3dh_write_u8(LIS3DH_REG_CTRL5, 0x40u);
  if (status != HAL_OK) return status;

  // 步骤8：映射中断源到INT1（CTRL_REG3 = 0x22）
  // I1_IA1=1：将"运动检测中断(IA1)"映射到INT1引脚
  status = lis3dh_write_u8(LIS3DH_REG_CTRL3, 0x40u);
  if (status != HAL_OK) return status;

  // 步骤9：配置INT1极性（CTRL_REG6 = 0x25）
  // I_H_LACTIVE(bit1)：0=高电平/上升沿有效，1=低电平/下降沿有效
  // 为简化与EXTI的配合，采用"高电平/上升沿有效"的典型配置：
  // - INT1 空闲为低电平，事件发生时输出高电平脉冲
  // 与GPIO的上升沿中断(GPIO_MODE_IT_RISING)自然匹配
  // 0x00 = 0b0000_0000 -> I_H_LACTIVE=0（高电平/上升沿有效）
  status = lis3dh_write_u8(LIS3DH_REG_CTRL6, 0x00u);
  if (status != HAL_OK) return status;

  // 步骤10：清除初始化过程中可能产生的残留中断标志
  // 在锁存模式下，读取INT1_SRC寄存器可以清除中断标志
  // 这样可以避免初始化完成后立即触发一次误中断
  uint8_t int1_src_dummy = 0;
  (void)lis3dh_read(LIS3DH_REG_INT1_SRC, &int1_src_dummy, 1);

  return HAL_OK;
}

/**
 * @brief  读取加速度原始值（16位有符号整数）
 * @param  x: X轴加速度原始值输出（-32768~32767）
 * @param  y: Y轴加速度原始值输出（-32768~32767）
 * @param  z: Z轴加速度原始值输出（-32768~32767）
 * @retval HAL_StatusTypeDef: HAL_OK=成功, HAL_ERROR=失败
 * @note   原始值需根据量程换算为实际加速度（g单位）
 *         使用LIS3DH_ReadAccelG()可直接获得g单位的值
 */
HAL_StatusTypeDef LIS3DH_ReadAccelRaw(int16_t *x, int16_t *y, int16_t *z)
{
  if (x == NULL || y == NULL || z == NULL) return HAL_ERROR;

  // 连读 X/Y/Z 六个字节：设置 bit7 开启地址自增（Auto-increment）
  uint8_t buf[6] = {0};
  HAL_StatusTypeDef st = lis3dh_read((uint8_t)(LIS3DH_REG_OUT_X_L | 0x80u), buf, sizeof(buf));
  if (st != HAL_OK) return st;

  // LIS3DH 输出为 little-endian：OUT_X_L/OUT_X_H 等
  *x = (int16_t)((uint16_t)buf[1] << 8 | buf[0]);
  *y = (int16_t)((uint16_t)buf[3] << 8 | buf[2]);
  *z = (int16_t)((uint16_t)buf[5] << 8 | buf[4]);
  return HAL_OK;
}

/**
 * @brief  读取加速度值（g单位，浮点数）
 * @note   此函数内部调用LIS3DH_ReadAccelRaw()获取原始值后进行单位转换
 * @param  x_g: X轴加速度输出（g单位）
 * @param  y_g: Y轴加速度输出（g单位）
 * @param  z_g: Z轴加速度输出（g单位）
 * @retval HAL_StatusTypeDef: HAL_OK=成功, HAL_ERROR=失败
 * @note   根据±4g量程、高分辨率模式进行单位转换：
 *         - CTRL4=0x98：高分辨率(12-bit)，±4g 量程
 *         - 数据为左对齐格式：有效 12 位在高位，需要先右移4位得到raw12
 *         - 灵敏度：±4g 高分辨率模式下 1 LSB ≈ 2 mg = 0.002 g
 *         公式：实际加速度(g) = raw12 × 0.002
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
 *         建议在EXTI中断服务程序或主循环中读取，用于清除中断
 */
HAL_StatusTypeDef LIS3DH_ReadInt1Src(uint8_t *src)
{
  if (src == NULL) return HAL_ERROR;
  // 读取 INT1_SRC（0x31）：若开启锁存，可同时用于"读取并清除"中断源
  return lis3dh_read(LIS3DH_REG_INT1_SRC, src, 1);
}


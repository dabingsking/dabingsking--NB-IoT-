#ifndef BSP_LIS3DH_H
#define BSP_LIS3DH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * LIS3DH 默认参数（见 Doc/ARCHITECTURE.md）：
 * - I2C 7-bit 地址：0x18（当 SDO=GND）
 * - WHO_AM_I 寄存器：0x0F，读出应为 0x33
 */

#define LIS3DH_I2C_ADDR_7BIT  (0x18u)
#define LIS3DH_REG_WHOAMI     (0x0Fu)
#define LIS3DH_WHOAMI_VALUE   (0x33u)

// 寄存器地址定义
#define LIS3DH_REG_INT1_THS      (0x32u)  // INT1中断阈值寄存器
#define LIS3DH_REG_INT1_DURATION (0x33u)  // INT1中断持续时间寄存器（去抖）

HAL_StatusTypeDef LIS3DH_Init(void);
HAL_StatusTypeDef LIS3DH_ReadWhoAmI(uint8_t *whoami);
HAL_StatusTypeDef LIS3DH_ReadAccelRaw(int16_t *x, int16_t *y, int16_t *z);
HAL_StatusTypeDef LIS3DH_ReadAccelG(float *x_g, float *y_g, float *z_g);
HAL_StatusTypeDef LIS3DH_ReadInt1Src(uint8_t *src);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LIS3DH_H */


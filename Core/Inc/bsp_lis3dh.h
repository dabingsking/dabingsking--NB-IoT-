/**
 * @file    bsp_lis3dh.h
 * @brief   LIS3DH 三轴加速度计 BSP 接口（I2C 模式，支持运动检测）。
 *
 * 默认参数（详见 `Doc/ARCHITECTURE.md` 第 14 章）：
 * - I2C 7-bit 地址：0x18（当 SDO=GND）。
 * - WHO_AM_I 寄存器：0x0F，读出应为 0x33。
 */

#ifndef BSP_LIS3DH_H
#define BSP_LIS3DH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define LIS3DH_I2C_ADDR_7BIT  (0x18u)
#define LIS3DH_REG_WHOAMI     (0x0Fu)
#define LIS3DH_WHOAMI_VALUE   (0x33u)

/* 倾斜角度结构体 */
typedef struct {
    float pitch_deg;   /* 俯仰角（绕Y轴），单位：度，范围 -90~+90  */
    float roll_deg;    /* 横滚角（绕X轴），单位：度，范围 -180~+180 */
    uint8_t cover_open; /* 1=开盖（|pitch|>45° 或 |roll|>45°），0=正常 */
} LIS3DH_Tilt_t;

HAL_StatusTypeDef LIS3DH_Init(void);
HAL_StatusTypeDef LIS3DH_ReadWhoAmI(uint8_t *whoami);
HAL_StatusTypeDef LIS3DH_ReadAccelRaw(int16_t *x, int16_t *y, int16_t *z);
HAL_StatusTypeDef LIS3DH_ReadAccelG(float *x_g, float *y_g, float *z_g);
HAL_StatusTypeDef LIS3DH_ReadInt1Src(uint8_t *src);

/**
 * @brief  计算俯仰角和横滚角
 * @param  tilt  输出结构体
 * @retval HAL_OK / HAL_ERROR
 * @note   基于静态重力分量，适用于缓慢姿态变化（井盖开合）
 *         Z轴朝上为正常安装方向（az≈+1g时 pitch=roll=0）
 */
HAL_StatusTypeDef LIS3DH_CalcTiltAngle(LIS3DH_Tilt_t *tilt);

#ifdef __cplusplus
}
#endif

#endif /* BSP_LIS3DH_H */


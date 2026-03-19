/**
 * @file    app_main.h
 * @brief   应用层主状态机接口
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "bsp_lowpower.h"

/* 应用状态枚举 */
typedef enum {
    APP_STATE_INIT = 0,        /* 首次上电初始化完成后的第一次睡前准备 */
    APP_STATE_WAKEUP_RESTORE,  /* STOP2 唤醒后恢复时钟和外设 */
    APP_STATE_COLLECT_DATA,    /* 传感器采集 */
    APP_STATE_CHECK_ANOMALY,   /* 异常判断 */
    APP_STATE_NB_IOT_COMM,     /* NB-IoT 上报（占位） */
    APP_STATE_PRE_SLEEP,       /* 睡眠准备 */
    APP_STATE_SLEEP,           /* 进入 STOP2 */
} APP_State_t;

/* 传感器采集结果 */
typedef struct {
    uint16_t water_cm;    /* 水位距离（cm），0xFFFF=无效 */
    uint16_t gas_ppm;     /* 气体浓度（ppm估算） */
    uint8_t  hall_state;  /* 霍尔状态：1=闭合（正常），0=打开（异常） */
    uint8_t  acc_alarm;   /* 加速度报警：0=正常，1=异常 */
    uint8_t  anomaly;     /* 综合异常标志 */
} SensorData_t;

/* 异常判断阈值 */
#define APP_GAS_THRESHOLD_PPM    1000u   /* 气体报警阈值 */
#define APP_WATER_THRESHOLD_CM   50u     /* 水位报警阈值（低于此值报警） */
#define APP_RTC_WAKEUP_PERIOD_S  5u  /* RTC 唤醒周期：12小时 为方便调试改为5s*/

/* 应用层入口 */
void App_Run(void);

/* 运行时设置 RTC 唤醒周期（秒），下次进入 STOP2 时生效 */
void App_SetWakeupPeriod(uint32_t seconds);

#ifdef __cplusplus
}
#endif

#endif /* APP_MAIN_H */

/**
 * @file    app_main.c
 * @brief   应用层主状态机
 *
 * 工作模式：
 *   - 平时 STOP2 待机
 *   - RTC 定时唤醒 → 全量采集 → 判断 → 上报 → 回睡
 *   - LIS3DH/霍尔中断唤醒 → 轻量采集 → 判断 → 异常则上报 → 回睡
 */

#include "app_main.h"
#include "bsp_lowpower.h"
#include "bsp_debug.h"
#include "bsp_led.h"
#include "bsp_button.h"
#include "bsp_hall.h"
#include "bsp_lis3dh.h"
#include "bsp_radar.h"
#include "bsp_mq4.h"
#include "bsp_power.h"
#include "main.h"
#include <string.h>
#include "service_nb.h"

/* ------------------------------------------------------------------ */
/* 私有变量                                                             */
/* ------------------------------------------------------------------ */
static APP_State_t    s_state         = APP_STATE_INIT;
static WakeupReason_t s_wakeup_reason = WAKEUP_REASON_POWER_ON;
static SensorData_t   s_sensor        = {0};
static uint8_t        s_no_stop2      = 0;  /* 调试：禁止 STOP2 */
static uint8_t        s_btn_init_held = 0;  /* 上电时按键是否按住 */
static uint32_t       s_wakeup_period_s = APP_RTC_WAKEUP_PERIOD_S; /* 运行时可调唤醒周期 */

/* ------------------------------------------------------------------ */
/* 私有函数                                                             */
/* ------------------------------------------------------------------ */

static void state_init(void)
{
    /* 配置 RTC 唤醒定时器 */
    if (LP_ConfigRtcWakeupSeconds(s_wakeup_period_s) != HAL_OK) {
        BSP_Debug_Printf("[APP] RTC wakeup timer init failed\r\n");
    }

    /* 上电安全模式：按键按住则禁止 STOP2 */
    HAL_Delay(50);
    if (BSP_Button_IsPressedRaw()) {
        s_no_stop2      = 1;
        s_btn_init_held = 1;
        BSP_LED_SetRGB(0, 1, 0);
        HAL_Delay(150);
    }

    BSP_LED_SetRGB(0, 0, 1);
    BSP_Debug_Printf("[APP] Init done, RTC wakeup=%lu s\r\n", s_wakeup_period_s);

    s_state = APP_STATE_PRE_SLEEP;
}

static void state_wakeup_restore(void)
{
    /* LP_ExitStop2 已完成：时钟恢复 + UART/I2C 重初始化 */
    LP_ExitStop2();

    /* 重新初始化调试串口上层 */
    BSP_Debug_Init();

    /* 识别唤醒原因 */
    s_wakeup_reason = LP_IdentifyWakeupReason();
    BSP_Debug_Printf("[APP] Wakeup reason=%d\r\n", (int)s_wakeup_reason);

    /* 无论何种唤醒原因，始终读取 INT1_SRC 清除 LIS3DH 锁存
     * 若 INT1 引脚保持高电平（锁存未清除），下次进入 STOP2 会立即被唤醒 */
    {
        uint8_t src = 0;
        LIS3DH_ReadInt1Src(&src);
        if (src & 0x40u) {  /* IA=1 表示确实有中断事件 */
            BSP_Debug_Printf("[APP] LIS3DH INT1_SRC=0x%02X\r\n", src);
        }
    }

    /* 重新使能 EXTI */
    __HAL_GPIO_EXTI_CLEAR_IT(ACC_INT1_Pin);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    BSP_LED_SetRGB(0, 0, 1);
    s_state = APP_STATE_COLLECT_DATA;
}

static void state_collect_data(void)
{
    BSP_LED_SetRGB(0, 1, 0);
    memset(&s_sensor, 0, sizeof(s_sensor));
    s_sensor.water_cm = 0xFFFFu;

    /* 轻量采集（所有唤醒类型） */
    s_sensor.hall_state = BSP_Hall_ReadStateDebounced();

    {
        LIS3DH_Tilt_t tilt = {0};
        if (LIS3DH_CalcTiltAngle(&tilt) == HAL_OK) {
            s_sensor.acc_alarm = tilt.cover_open;
            BSP_Debug_Printf("[LIS3DH] pitch=%.1f roll=%.1f cover=%u\r\n",
                             tilt.pitch_deg, tilt.roll_deg, tilt.cover_open);
        }
    }

    /* 全量采集（RTC 周期唤醒） */
    if (s_wakeup_reason == WAKEUP_REASON_RTC ||
        s_wakeup_reason == WAKEUP_REASON_POWER_ON) {

        /* 雷达测距：上电后等500ms让模块稳定 */
        BSP_Power_RadarOn();
        HAL_Delay(500);
        s_sensor.water_cm = BSP_Radar_Measure();
        BSP_Power_RadarOff();

        /* MQ4 气体采集（阻塞预热30s） */
        BSP_Power_GasOn();
        HAL_Delay(BSP_MQ4_PREHEAT_MS);
        uint16_t adc = BSP_MQ4_ReadAdcAverage(BSP_MQ4_SAMPLE_COUNT,
                                               BSP_MQ4_SAMPLE_INTERVAL_MS);
        uint16_t sensor_mv = BSP_MQ4_AdcToSensor_mV(adc);
        s_sensor.gas_ppm = BSP_MQ4_Sensor_mV_ToPpmEst(sensor_mv);
        BSP_Debug_Printf("[MQ4] adc=%u sensor_mv=%u gas_ppm=%u\r\n",
                         adc, sensor_mv, s_sensor.gas_ppm);
        BSP_Power_GasOff();
    }

    BSP_Debug_Printf("[APP] Collect: hall=%u acc_alarm=%u water=%u gas=%u\r\n",
                     s_sensor.hall_state, s_sensor.acc_alarm,
                     s_sensor.water_cm, s_sensor.gas_ppm);

    s_state = APP_STATE_CHECK_ANOMALY;
}

static void state_check_anomaly(void)
{
    uint8_t anomaly = 0;

    if (s_sensor.gas_ppm   > APP_GAS_THRESHOLD_PPM)   anomaly = 1u;
    if (s_sensor.water_cm  < APP_WATER_THRESHOLD_CM)   anomaly = 1u;
    if (s_sensor.hall_state == HALL_STATE_OPEN)         anomaly = 1u;
    if (s_sensor.acc_alarm  != 0u)                     anomaly = 1u;

    s_sensor.anomaly = anomaly;

    /* RTC 周期唤醒始终上报（心跳包） */
    uint8_t need_report = anomaly;
    if (s_wakeup_reason == WAKEUP_REASON_RTC) need_report = 1u;

    BSP_Debug_Printf("[APP] Anomaly=%u need_report=%u\r\n", anomaly, need_report);

    s_state = need_report ? APP_STATE_NB_IOT_COMM : APP_STATE_PRE_SLEEP;
}

static void state_nb_iot_comm(void)
{
    BSP_LED_SetRGB(0, 1, 1);
    NB_Status_t ret = NB_ReportData(&s_sensor);
    if (ret != NB_OK) {
        BSP_Debug_Printf("[APP] NB report failed: %d, retry 1/2\r\n", (int)ret);
        HAL_Delay(2000);
        ret = NB_ReportData(&s_sensor);
    }
    if (ret != NB_OK) {
        BSP_Debug_Printf("[APP] NB report failed: %d, retry 2/2\r\n", (int)ret);
        HAL_Delay(2000);
        ret = NB_ReportData(&s_sensor);
    }
    if (ret != NB_OK) {
        BSP_Debug_Printf("[APP] NB report failed after 3 attempts: %d\r\n", (int)ret);
    }
    s_state = APP_STATE_PRE_SLEEP;
}

static void state_pre_sleep(void)
{
    if (s_no_stop2) {
        /* 调试模式：蓝灯常亮，持续采集并打印传感器数据 */
        BSP_LED_SetRGB(0, 0, 1);

        /* 霍尔 + 加速度 */
        uint8_t hall = BSP_Hall_ReadStateDebounced();
        LIS3DH_Tilt_t tilt = {0};
        LIS3DH_CalcTiltAngle(&tilt);

        BSP_Debug_Printf("[DEBUG] hall=%u acc_cover=%u pitch=%.1f roll=%.1f\r\n",
                         hall, tilt.cover_open, tilt.pitch_deg, tilt.roll_deg);

        /* 雷达 */
        BSP_Power_RadarOn();
        HAL_Delay(500);
        uint16_t water = BSP_Radar_Measure();
        BSP_Power_RadarOff();
        BSP_Debug_Printf("[DEBUG] water=%u cm\r\n", water);

        /* MQ4 */
        BSP_Power_GasOn();
        HAL_Delay(500);  /* 调试模式不等30s预热，快速采样看趋势 */
        uint16_t adc = BSP_MQ4_ReadAdcAverage(3, 100);
        uint16_t sensor_mv = BSP_MQ4_AdcToSensor_mV(adc);
        uint16_t gas_ppm = BSP_MQ4_Sensor_mV_ToPpmEst(sensor_mv);
        BSP_Power_GasOff();
        BSP_Debug_Printf("[DEBUG] adc=%u sensor_mv=%u gas_ppm=%u\r\n",
                         adc, sensor_mv, gas_ppm);

        HAL_Delay(3000);  /* 每3秒打印一次 */
        return;
    }

    /* 关闭传感器电源 */
    BSP_Power_GasOff();
    BSP_Power_RadarOff();
    BSP_LED_AllOff();

    /* 确保 EXTI 唤醒源就绪 */
    __HAL_GPIO_EXTI_CLEAR_IT(ACC_INT1_Pin);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    /* 清除 PWR 唤醒标志 */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /* 重新设置 RTC 唤醒定时器，确保进入 STOP2 前从 0 开始计数 */
    if (LP_ConfigRtcWakeupSeconds(s_wakeup_period_s) != HAL_OK) {
        BSP_Debug_Printf("[APP] RTC wakeup timer config failed, skip STOP2\r\n");
        HAL_Delay(500);
        s_state = APP_STATE_PRE_SLEEP;
        return;
    }

    BSP_Debug_Printf("[APP] Entering STOP2, wakeup in %lu s\r\n", s_wakeup_period_s);

    s_state = APP_STATE_SLEEP;
}

static void state_sleep(void)
{
    /* 进入 STOP2，唤醒后从此处返回 */
    LP_EnterStop2();

    /* === STOP2 唤醒后执行到这里 === */
    s_state = APP_STATE_WAKEUP_RESTORE;
}

/* ------------------------------------------------------------------ */
/* 公开入口                                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief  应用层主循环，在 main() 的 while(1) 中调用
 */
void App_Run(void)
{
    /* 首次进入：完成上电初始化后的配置 */
    state_init();

    while (1) {
        /* 按键轮询 */
        BSP_Button_Poll();
        if (BSP_Button_GetShortPressEvent()) {
            if (s_btn_init_held) {
                s_btn_init_held = 0;
            } else {
                s_no_stop2 = !s_no_stop2;
            }
            BSP_LED_SetRGB(1, 1, 1);
            HAL_Delay(80);
            BSP_LED_SetRGB(0, 0, 1);
        }

        switch (s_state) {
        case APP_STATE_INIT:           state_init();           break;
        case APP_STATE_WAKEUP_RESTORE: state_wakeup_restore(); break;
        case APP_STATE_COLLECT_DATA:   state_collect_data();   break;
        case APP_STATE_CHECK_ANOMALY:  state_check_anomaly();  break;
        case APP_STATE_NB_IOT_COMM:    state_nb_iot_comm();    break;
        case APP_STATE_PRE_SLEEP:      state_pre_sleep();      break;
        case APP_STATE_SLEEP:          state_sleep();          break;
        default:                       s_state = APP_STATE_PRE_SLEEP; break;
        }
    }
}

void App_SetWakeupPeriod(uint32_t seconds)
{
    if (seconds < 1u) seconds = 1u;
    s_wakeup_period_s = seconds;
}

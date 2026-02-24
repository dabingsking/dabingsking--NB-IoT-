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

/* ------------------------------------------------------------------ */
/* 私有变量                                                             */
/* ------------------------------------------------------------------ */
static APP_State_t    s_state         = APP_STATE_INIT;
static WakeupReason_t s_wakeup_reason = WAKEUP_REASON_POWER_ON;
static SensorData_t   s_sensor        = {0};
static uint8_t        s_no_stop2      = 0;  /* 调试：禁止 STOP2 */
static uint8_t        s_btn_init_held = 0;  /* 上电时按键是否按住 */

/* ------------------------------------------------------------------ */
/* 私有函数                                                             */
/* ------------------------------------------------------------------ */

static void state_init(void)
{
    /* 配置 RTC 唤醒定时器 */
    LP_ConfigRtcWakeupSeconds(APP_RTC_WAKEUP_PERIOD_S);

    /* 上电安全模式：按键按住则禁止 STOP2 */
    HAL_Delay(50);
    if (BSP_Button_IsPressedRaw()) {
        s_no_stop2      = 1;
        s_btn_init_held = 1;
        BSP_LED_SetRGB(0, 1, 0);
        HAL_Delay(150);
    }

    BSP_LED_SetRGB(0, 0, 1);
    BSP_Debug_Printf("[APP] Init done, RTC wakeup=%lu s\r\n",
                     APP_RTC_WAKEUP_PERIOD_S);

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

    /* 若为 ACC 唤醒，读取 INT1_SRC 清除锁存 */
    if (s_wakeup_reason == WAKEUP_REASON_ACC) {
        uint8_t src = 0;
        LIS3DH_ReadInt1Src(&src);
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
        float ax = 0.f, ay = 0.f, az = 0.f;
        if (LIS3DH_ReadAccelG(&ax, &ay, &az) == HAL_OK) {
            float mag2 = ax * ax + ay * ay + az * az;
            /* 合加速度偏离 1g 超过 0.5g 视为异常 */
            float diff = mag2 - 1.0f;
            s_sensor.acc_alarm = (diff > 0.25f || diff < -0.25f) ? 1u : 0u;
        }
    }

    /* 全量采集（RTC 周期唤醒） */
    if (s_wakeup_reason == WAKEUP_REASON_RTC ||
        s_wakeup_reason == WAKEUP_REASON_POWER_ON) {

        /* 雷达测距 */
        BSP_Power_RadarOn();
        HAL_Delay(50);
        s_sensor.water_cm = BSP_Radar_Measure();
        BSP_Power_RadarOff();

        /* MQ4 气体采集（阻塞预热） */
        BSP_Power_GasOn();
        HAL_Delay(BSP_MQ4_PREHEAT_MS);
        uint16_t adc = BSP_MQ4_ReadAdcAverage(BSP_MQ4_SAMPLE_COUNT,
                                               BSP_MQ4_SAMPLE_INTERVAL_MS);
        s_sensor.gas_ppm = BSP_MQ4_Sensor_mV_ToPpmEst(
                               BSP_MQ4_AdcToSensor_mV(adc));
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
    if (s_sensor.hall_state != 0u)                     anomaly = 1u;
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
    /* TODO: 实现 EC-01G AT + MQTT 上报 */
    BSP_Debug_Printf("[APP] NB-IoT report placeholder\r\n");
    s_state = APP_STATE_PRE_SLEEP;
}

static void state_pre_sleep(void)
{
    if (s_no_stop2) {
        /* 调试模式：蓝灯待机，不进入 STOP2 */
        BSP_LED_SetRGB(0, 0, 1);
        HAL_Delay(500);
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

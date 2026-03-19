/**
 * @file    service_nb.c
 * @brief   NB-IoT 服务层 - ThingsCloud MQTT 上报实现
 *
 * ThingsCloud MQTT 协议：
 *   服务器：bj-2-mqtt.iot-api.com:1883
 *   认证：clientId="thingscloud", username=<AccessToken>, password=<ProjectKey>
 *   发布主题：attributes
 *   订阅主题：attributes/push (接收响应)
 *   JSON格式：{"temperature":31.6} 或 {"level":85.5,"concentration":12.3}
 */

#include "service_nb.h"
#include "bsp_ec01g.h"
#include "bsp_debug.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* ThingsCloud 接入参数（从流程文档）                                    */
/* ------------------------------------------------------------------ */
#define THINGSCLOUD_HOST   "bj-2-mqtt.iot-api.com"
#define THINGSCLOUD_PORT   1883u
#define THINGSCLOUD_TOKEN  "saax9ry2ytlahvmp"  /* AccessToken */
#define THINGSCLOUD_KEY    "HpXvAsqbPE"        /* ProjectKey */

/* ------------------------------------------------------------------ */
/* 公开函数                                                             */
/* ------------------------------------------------------------------ */

NB_Status_t NB_ReportData(const SensorData_t *data)
{
    EC01G_Status_t ec_ret;

    /* ---- Step 1: 初始化（复位 + AT 验证） ---- */
    ec_ret = BSP_EC01G_Init();
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] EC01G init failed: %d\r\n", (int)ec_ret);
        return NB_ERR_NETWORK;
    }

    /* ---- Step 2: 检查 SIM 卡 ---- */
    ec_ret = BSP_EC01G_CheckSIM();
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] SIM check failed\r\n");
        return NB_ERR_NETWORK;
    }

    /* ---- Step 2.5: 等待网络注册完成 (+CREG: 6) ---- */
    /* EC01G 必须收到 +CREG: 6（NB-IoT 附着成功）后才能建立 TCP 连接，
     * 否则 AT+ECMTOPEN 返回 +CME ERROR: 3（操作不允许）。
     * WaitResponse 不清缓冲，直接轮询已有数据及后续 URC。 */
    ec_ret = BSP_EC01G_WaitResponse("+CREG: 6", 60000);
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] Network registration timeout (+CREG: 6 not seen)\r\n");
        return NB_ERR_NETWORK;
    }
    BSP_Debug_Printf("[NB] Network registered\r\n");

    /* ---- Step 3: 建立 MQTT TCP 连接 ---- */
    ec_ret = BSP_EC01G_MQTTOpen(THINGSCLOUD_HOST, THINGSCLOUD_PORT);
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] MQTT open failed\r\n");
        return NB_ERR_CONNECT;
    }

    /* ---- Step 4: MQTT 认证连接 ---- */
    ec_ret = BSP_EC01G_MQTTConnect("thingscloud",
                                    THINGSCLOUD_TOKEN,
                                    THINGSCLOUD_KEY);
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] MQTT connect failed\r\n");
        BSP_EC01G_MQTTDisconnect();
        return NB_ERR_CONNECT;
    }

    /* ---- Step 5: 订阅响应主题 ---- */
    ec_ret = BSP_EC01G_MQTTSubscribe("attributes/push", 0);
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] MQTT subscribe failed\r\n");
        BSP_EC01G_MQTTDisconnect();
        return NB_ERR_SEND;
    }

    /* ---- Step 6: 组装 JSON 并发布到 attributes 主题 ---- */
    /* water_cm == 0xFFFF 表示雷达无效，上报 -1 让云平台识别 */
    char json_buf[256];
    if (data->water_cm == 0xFFFFu) {
        snprintf(json_buf, sizeof(json_buf),
                 "{\"water_level\":-1,\"gas_ppm\":%u,\"hall_state\":%u,"
                 "\"acc_alarm\":%u,\"anomaly\":%u}",
                 (unsigned)data->gas_ppm,
                 (unsigned)data->hall_state,
                 (unsigned)data->acc_alarm,
                 (unsigned)data->anomaly);
    } else {
        snprintf(json_buf, sizeof(json_buf),
                 "{\"water_level\":%u,\"gas_ppm\":%u,\"hall_state\":%u,"
                 "\"acc_alarm\":%u,\"anomaly\":%u}",
                 (unsigned)data->water_cm,
                 (unsigned)data->gas_ppm,
                 (unsigned)data->hall_state,
                 (unsigned)data->acc_alarm,
                 (unsigned)data->anomaly);
    }

    ec_ret = BSP_EC01G_MQTTPublish("attributes", json_buf);
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] MQTT publish failed\r\n");
        BSP_EC01G_MQTTDisconnect();
        return NB_ERR_SEND;
    }

    /* ---- Step 7: 断开 MQTT 连接 ---- */
    BSP_EC01G_MQTTDisconnect();

    BSP_Debug_Printf("[NB] ThingsCloud report OK: %s\r\n", json_buf);
    return NB_OK;
}

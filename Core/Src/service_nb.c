/**
 * @file    service_nb.c
 * @brief   NB-IoT 服务层 - 巴法云 TCP 9501 上报实现
 *
 * 巴法云 TCP 协议：
 *   鉴权：uid#topic#\r\n          → 服务器回复 cmd=1&res=1\r\n
 *   发布：uid#topic#msg#\r\n      → 服务器可能回复 cmd=2&res=1\r\n
 */

#include "service_nb.h"
#include "bsp_ec01g.h"
#include "bsp_debug.h"
#include <stdio.h>
#include <string.h>
#include "stm32l4xx_hal.h"

/* ------------------------------------------------------------------ */
/* 巴法云接入参数                                                        */
/* ------------------------------------------------------------------ */
#define BEMFA_UID    "dff51e3cf58147c687884a86b88b72ea"
#define BEMFA_TOPIC  "AKRPL60J0004"
#define BEMFA_HOST   "bemfa.com"
#define BEMFA_PORT   9501u

/* ------------------------------------------------------------------ */
/* 公开函数                                                             */
/* ------------------------------------------------------------------ */

NB_Status_t NB_ReportData(const SensorData_t *data)
{
    EC01G_Status_t ec_ret;

    /* ---- Step 1: AT 通信验证 ---- */
    ec_ret = BSP_EC01G_Init();
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] EC01G init failed: %d\r\n", (int)ec_ret);
        return NB_ERR_NETWORK;
    }

    /* ---- Step 2: 网络注册检查 ---- */
    ec_ret = BSP_EC01G_CheckNetwork();
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] Network not registered\r\n");
        return NB_ERR_NETWORK;
    }

    /* ---- Step 3: TCP 连接 ---- */
    ec_ret = BSP_EC01G_TCPOpen(BEMFA_HOST, BEMFA_PORT);
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] TCP connect failed: %d\r\n", (int)ec_ret);
        return NB_ERR_CONNECT;
    }

    /* ---- Step 4: 鉴权（订阅） ---- */
    char auth_msg[96];
    snprintf(auth_msg, sizeof(auth_msg), BEMFA_UID "#" BEMFA_TOPIC "#\r\n");
    ec_ret = BSP_EC01G_TCPSend((uint8_t *)auth_msg, (uint16_t)strlen(auth_msg));
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] Auth send failed\r\n");
        BSP_EC01G_TCPClose();
        return NB_ERR_SEND;
    }
    HAL_Delay(1000);  /* 等待服务器鉴权回复 */

    /* ---- Step 5: 组装 JSON 并发布 ---- */
    char json_buf[160];
    snprintf(json_buf, sizeof(json_buf),
             "{\"gas\":%u,\"water\":%u,\"hall\":%u,"
             "\"acc_alarm\":%u,\"anomaly\":%u}",
             (unsigned)data->gas_ppm,
             (unsigned)data->water_cm,
             (unsigned)data->hall_state,
             (unsigned)data->acc_alarm,
             (unsigned)data->anomaly);

    char pub_msg[300];
    snprintf(pub_msg, sizeof(pub_msg),
             BEMFA_UID "#" BEMFA_TOPIC "#%s#\r\n", json_buf);

    ec_ret = BSP_EC01G_TCPSend((uint8_t *)pub_msg, (uint16_t)strlen(pub_msg));
    if (ec_ret != EC01G_OK) {
        BSP_Debug_Printf("[NB] Data send failed\r\n");
        BSP_EC01G_TCPClose();
        return NB_ERR_SEND;
    }
    HAL_Delay(500);  /* 等待发送完成 */

    /* ---- Step 6: 关闭 TCP ---- */
    BSP_EC01G_TCPClose();

    /* ---- Step 7: PSM ---- */
    BSP_EC01G_EnablePSM();

    BSP_Debug_Printf("[NB] Report OK: %s\r\n", json_buf);
    return NB_OK;
}

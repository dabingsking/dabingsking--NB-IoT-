/**
 * @file    bsp_ec01g.c
 * @brief   EC-01G NB-IoT 模块 AT 指令封装（ThingsCloud MQTT）
 *
 * 流程：复位 → AT → CPIN? → CGATT? → CSQ
 *       → ECMTOPEN → ECMTCONN → ECMTSUB/ECMTPUB → ECMTDISC
 *
 * RX 机制：
 *   USART2_IRQHandler → BSP_EC01G_UART_RxCallback(byte)
 *   → 填入 g_rx_buf[]，SendCmd() 轮询 g_rx_buf 匹配期望字符串
 */

#include "bsp_ec01g.h"
#include "bsp_debug.h"
#include <string.h>
#include <stdio.h>
#include "stm32l4xx_hal.h"

/* ------------------------------------------------------------------ */
/* 私有常量                                                             */
/* ------------------------------------------------------------------ */
#define EC01G_RX_BUF_SIZE   512u
#define EC01G_TX_TIMEOUT_MS 1000u

/* ------------------------------------------------------------------ */
/* 私有变量                                                             */
/* ------------------------------------------------------------------ */
static volatile char     g_rx_buf[EC01G_RX_BUF_SIZE];
static volatile uint16_t g_rx_len = 0;

extern UART_HandleTypeDef huart2;

/* ------------------------------------------------------------------ */
/* 私有函数                                                             */
/* ------------------------------------------------------------------ */

static void ClearRxBuf(void)
{
    g_rx_len = 0;
    memset((void *)g_rx_buf, 0, sizeof(g_rx_buf));
}

/**
 * @brief 发送 AT 指令，等待 expect 字符串出现或超时
 */
static EC01G_Status_t SendCmd(const char *cmd, const char *expect,
                               uint32_t timeout_ms)
{
    ClearRxBuf();
    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, (uint16_t)strlen(cmd),
                      EC01G_TX_TIMEOUT_MS);

    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms) {
        if (strstr((const char *)g_rx_buf, expect) != NULL) {
            return EC01G_OK;
        }
    }
    BSP_Debug_Printf("[EC01G] Timeout waiting '%s', got: %s\r\n",
                     expect, (const char *)g_rx_buf);
    return EC01G_ERR_TIMEOUT;
}

/* ------------------------------------------------------------------ */
/* 公开函数                                                             */
/* ------------------------------------------------------------------ */

void BSP_EC01G_UART_RxCallback(uint8_t byte)
{
    if (g_rx_len < EC01G_RX_BUF_SIZE - 2u) {
        g_rx_buf[g_rx_len++] = (char)byte;
        g_rx_buf[g_rx_len]   = '\0';
    }
}

EC01G_Status_t BSP_EC01G_WaitResponse(const char *expect, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms) {
        if (strstr((const char *)g_rx_buf, expect) != NULL) {
            return EC01G_OK;
        }
    }
    BSP_Debug_Printf("[EC01G] WaitResponse timeout, expect '%s', got: [%s]\r\n",
                     expect, (const char *)g_rx_buf);
    return EC01G_ERR_TIMEOUT;
}

EC01G_Status_t BSP_EC01G_Init(void)
{
    /* 使能 USART2 RXNE 中断 */
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);

    /* 复位模块：AT+ECRST */
    if (SendCmd("AT+ECRST\r\n", "OK", 3000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] Reset failed\r\n");
        return EC01G_ERR_TIMEOUT;
    }
    HAL_Delay(2000);  /* 等待模块重启 */

    /* 验证 AT 通信：AT */
    if (SendCmd("AT\r\n", "OK", 2000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] AT test failed\r\n");
        return EC01G_ERR_TIMEOUT;
    }

    BSP_Debug_Printf("[EC01G] Init OK\r\n");
    return EC01G_OK;
}

EC01G_Status_t BSP_EC01G_CheckSIM(void)
{
    /* AT+CPIN? 检查 SIM 卡状态 */
    if (SendCmd("AT+CPIN?\r\n", "READY", 3000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] SIM card not ready\r\n");
        return EC01G_ERR_NETWORK;
    }

    BSP_Debug_Printf("[EC01G] SIM card OK\r\n");
    return EC01G_OK;
}


EC01G_Status_t BSP_EC01G_MQTTOpen(const char *host, uint16_t port)
{
    /* AT+ECMTOPEN=0,"bj-2-mqtt.iot-api.com",1883 */
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+ECMTOPEN=0,\"%s\",%u\r\n", host, port);

    if (SendCmd(cmd, "OK", 10000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] MQTT open cmd failed\r\n");
        return EC01G_ERR_MQTT;
    }

    /* +ECMTOPEN: 0,0 是异步 URC，MQTTConnect 发送前等待确认。 */
    return EC01G_OK;
}

EC01G_Status_t BSP_EC01G_MQTTConnect(const char *client_id,
                                      const char *username,
                                      const char *password)
{
    /* 等待 +ECMTOPEN: 0,0 URC，确认 TCP 层建立后再发 CONN。
     * 超时说明连接未建立，直接返回错误，不继续发 ECMTCONN。 */
    if (BSP_EC01G_WaitResponse("+ECMTOPEN: 0,0", 30000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] +ECMTOPEN URC timeout, TCP not ready\r\n");
        return EC01G_ERR_MQTT;
    }

    /* 清缓冲，防止前面积累的数据导致 +ECMTCONN URC 匹配时溢出截断 */
    ClearRxBuf();

    /* AT+ECMTCONN=0,"<client_id>","<username>","<password>" */
    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "AT+ECMTCONN=0,\"%s\",\"%s\",\"%s\"\r\n",
             client_id, username, password);

    if (SendCmd(cmd, "OK", 8000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] MQTT connect cmd failed\r\n");
        return EC01G_ERR_MQTT;
    }

    /* 等待 URC 确认 MQTT 握手成功：+ECMTCONN: 0,0,0（必须等） */
    if (BSP_EC01G_WaitResponse("+ECMTCONN: 0,0,0", 15000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] MQTT auth timeout\r\n");
        return EC01G_ERR_MQTT;
    }

    BSP_Debug_Printf("[EC01G] MQTT connected\r\n");
    return EC01G_OK;
}

EC01G_Status_t BSP_EC01G_MQTTSubscribe(const char *topic, uint8_t qos)
{
    /* AT+ECMTSUB=0,1,"attributes/push",0 */
    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "AT+ECMTSUB=0,1,\"%s\",%u\r\n", topic, qos);

    if (SendCmd(cmd, "OK", 5000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] MQTT subscribe cmd failed\r\n");
        return EC01G_ERR_MQTT;
    }

    /* 等待 URC 确认订阅成功：+ECMTSUB: 0,1,0
     * 实际格式：+ECMTSUB: <client_idx>,<msgid>,<result>,<granted_qos>
     * msgid=1（与发送命令中的固定值一致），result=0 表示成功 */
    if (BSP_EC01G_WaitResponse("+ECMTSUB: 0,1,0", 5000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] MQTT subscribe timeout\r\n");
        return EC01G_ERR_MQTT;
    }

    return EC01G_OK;
}

EC01G_Status_t BSP_EC01G_MQTTPublish(const char *topic, const char *payload)
{
    /* AT+ECMTPUB=0,1,0,0,"attributes","{"temperature":31.6}" */
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "AT+ECMTPUB=0,1,0,0,\"%s\",\"%s\"\r\n", topic, payload);

    if (SendCmd(cmd, "OK", 5000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] MQTT publish cmd failed\r\n");
        return EC01G_ERR_MQTT;
    }

    /* 等待 URC 确认发布成功：+ECMTPUB: 0,<msg_id>,<qos>（msg_id 随发布次数变化，仅匹配前缀） */
    if (BSP_EC01G_WaitResponse("+ECMTPUB: 0,", 5000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] MQTT publish timeout\r\n");
        return EC01G_ERR_MQTT;
    }

    return EC01G_OK;
}

EC01G_Status_t BSP_EC01G_MQTTDisconnect(void)
{
    /* AT+ECMTDISC=0 断开 MQTT 连接 */
    SendCmd("AT+ECMTDISC=0\r\n", "OK", 3000);

    /* 等待 URC 确认：+ECMTDISC: 0,0 */
    BSP_EC01G_WaitResponse("+ECMTDISC: 0,0", 3000);

    return EC01G_OK;
}

/**
 * @file    bsp_ec01g.h
 * @brief   EC-01G NB-IoT 模块 AT 指令封装（ThingsCloud MQTT）
 */
#ifndef BSP_EC01G_H
#define BSP_EC01G_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    EC01G_OK = 0,
    EC01G_ERR_TIMEOUT,   /* AT 指令超时无响应 */
    EC01G_ERR_NETWORK,   /* 网络未注册 */
    EC01G_ERR_MQTT,      /* MQTT 操作失败 */
} EC01G_Status_t;

/* 初始化：复位模块，验证 AT 通信，使能 USART2 RXNE 中断 */
EC01G_Status_t BSP_EC01G_Init(void);

/* 检查 SIM 卡状态：AT+CPIN? */
EC01G_Status_t BSP_EC01G_CheckSIM(void);

/* 建立 MQTT TCP 连接：AT+ECMTOPEN */
EC01G_Status_t BSP_EC01G_MQTTOpen(const char *host, uint16_t port);

/* MQTT 连接认证：AT+ECMTCONN */
EC01G_Status_t BSP_EC01G_MQTTConnect(const char *client_id,
                                      const char *username,
                                      const char *password);

/* 订阅主题：AT+ECMTSUB */
EC01G_Status_t BSP_EC01G_MQTTSubscribe(const char *topic, uint8_t qos);

/* 发布消息：AT+ECMTPUB */
EC01G_Status_t BSP_EC01G_MQTTPublish(const char *topic, const char *payload);

/* 断开 MQTT 连接：AT+ECMTDISC */
EC01G_Status_t BSP_EC01G_MQTTDisconnect(void);

/* 等待 RX 缓冲出现 expect 字符串（不清缓冲） */
EC01G_Status_t BSP_EC01G_WaitResponse(const char *expect, uint32_t timeout_ms);

/* 由 USART2_IRQHandler 调用，将接收到的字节填入内部缓冲 */
void BSP_EC01G_UART_RxCallback(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* BSP_EC01G_H */

/**
 * @file    bsp_ec01g.h
 * @brief   EC-01G NB-IoT 模块 AT 指令封装（TCP socket 操作）
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
    EC01G_ERR_SOCKET,    /* socket 操作失败 */
} EC01G_Status_t;

/* 初始化：验证 AT 通信正常，使能 USART2 RXNE 中断 */
EC01G_Status_t BSP_EC01G_Init(void);

/* 轮询 AT+CEREG? 直到网络注册成功，超时 60s */
EC01G_Status_t BSP_EC01G_CheckNetwork(void);

/* 创建 TCP socket 并连接到指定主机端口 */
EC01G_Status_t BSP_EC01G_TCPOpen(const char *host, uint16_t port);

/* 将 data 字节数组 hex 编码后通过 AT+NSOSD 发送 */
EC01G_Status_t BSP_EC01G_TCPSend(const uint8_t *data, uint16_t len);

/* 关闭 TCP socket */
EC01G_Status_t BSP_EC01G_TCPClose(void);

/* 配置 PSM 模式（AT+CPSMS），通信完成后调用 */
EC01G_Status_t BSP_EC01G_EnablePSM(void);

/* 由 USART2_IRQHandler 调用，将接收到的字节填入内部缓冲 */
void BSP_EC01G_UART_RxCallback(uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* BSP_EC01G_H */

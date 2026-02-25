/**
 * @file    bsp_ec01g.c
 * @brief   EC-01G NB-IoT 模块 AT 指令封装（HAL USART2）
 *
 * RX 机制：
 *   USART2_IRQHandler → BSP_EC01G_UART_RxCallback(byte)
 *   → 填入 g_rx_buf[]，SendCmd() 轮询 g_rx_buf 匹配期望字符串
 *
 * 注意事项：
 *   每次 STOP2 唤醒后需重新调用 BSP_EC01G_Init() 重开 RXNE 中断位。
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
static int8_t            g_socket_id = -1;

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

/**
 * @brief 将字节数组转为大写 hex 字符串
 *        out 需至少 len*2+1 字节
 */
static void BytesToHex(const uint8_t *in, uint16_t len, char *out)
{
    static const char hex[] = "0123456789ABCDEF";
    for (uint16_t i = 0; i < len; i++) {
        out[i * 2]     = hex[(in[i] >> 4) & 0x0Fu];
        out[i * 2 + 1] = hex[in[i] & 0x0Fu];
    }
    out[len * 2] = '\0';
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

EC01G_Status_t BSP_EC01G_Init(void)
{
    g_socket_id = -1;
    ClearRxBuf();

    /* 使能 RXNE 中断（HAL_UART_Init 不会自动开启，NVIC 已在 MspInit 里使能） */
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);

    /* 最多重试 3 次，确认 AT 通信正常 */
    for (int i = 0; i < 3; i++) {
        if (SendCmd("AT\r\n", "OK", 2000) == EC01G_OK) {
            BSP_Debug_Printf("[EC01G] Init OK\r\n");
            return EC01G_OK;
        }
        HAL_Delay(500);
    }
    BSP_Debug_Printf("[EC01G] Init FAILED (no AT response)\r\n");
    return EC01G_ERR_TIMEOUT;
}

EC01G_Status_t BSP_EC01G_CheckNetwork(void)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 60000u) {
        ClearRxBuf();
        HAL_UART_Transmit(&huart2, (uint8_t *)"AT+CEREG?\r\n", 11,
                          EC01G_TX_TIMEOUT_MS);
        HAL_Delay(1000);
        /* 注册状态：,1 = 本地注册，,5 = 漫游注册 */
        if (strstr((const char *)g_rx_buf, ",1") != NULL ||
            strstr((const char *)g_rx_buf, ",5") != NULL) {
            BSP_Debug_Printf("[EC01G] Network registered\r\n");
            return EC01G_OK;
        }
        BSP_Debug_Printf("[EC01G] CEREG resp: %s\r\n", (const char *)g_rx_buf);
    }
    BSP_Debug_Printf("[EC01G] Network registration timeout\r\n");
    return EC01G_ERR_NETWORK;
}

EC01G_Status_t BSP_EC01G_TCPOpen(const char *host, uint16_t port)
{
    char cmd[128];

    /* 创建 TCP stream socket */
    if (SendCmd("AT+NSOCR=\"STREAM\",6,0,1\r\n", "OK", 5000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] NSOCR failed\r\n");
        return EC01G_ERR_SOCKET;
    }
    g_socket_id = 0;  /* EC01G 首次建 socket 返回 0 */

    /* 连接到服务器 */
    snprintf(cmd, sizeof(cmd), "AT+NSOCO=0,\"%s\",%u\r\n", host, port);
    if (SendCmd(cmd, "OK", 10000) != EC01G_OK) {
        BSP_Debug_Printf("[EC01G] NSOCO failed\r\n");
        g_socket_id = -1;
        return EC01G_ERR_SOCKET;
    }

    BSP_Debug_Printf("[EC01G] TCP connected to %s:%u\r\n", host, port);
    return EC01G_OK;
}

EC01G_Status_t BSP_EC01G_TCPSend(const uint8_t *data, uint16_t len)
{
    if (g_socket_id < 0) return EC01G_ERR_SOCKET;
    if (len == 0 || len > 256u) return EC01G_ERR_SOCKET;

    /*
     * 构建完整 AT 命令：AT+NSOSD=0,<len>,<HEX>\r\n
     * 最大：前缀20 + 256*2 hex + 2 \r\n = 534 字节，用 600 缓冲安全
     */
    static char full_cmd[600];
    int prefix_len = snprintf(full_cmd, sizeof(full_cmd),
                              "AT+NSOSD=%d,%u,", g_socket_id, (unsigned)len);
    if (prefix_len < 0 || (prefix_len + len * 2 + 3) >= (int)sizeof(full_cmd)) {
        return EC01G_ERR_SOCKET;
    }
    BytesToHex(data, len, full_cmd + prefix_len);
    int total = prefix_len + len * 2;
    full_cmd[total++] = '\r';
    full_cmd[total++] = '\n';
    full_cmd[total]   = '\0';

    return SendCmd(full_cmd, "OK", 5000);
}

EC01G_Status_t BSP_EC01G_TCPClose(void)
{
    if (g_socket_id < 0) return EC01G_OK;
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+NSOCL=%d\r\n", g_socket_id);
    g_socket_id = -1;
    /* 关闭失败不上报错误，避免影响主流程 */
    SendCmd(cmd, "OK", 3000);
    return EC01G_OK;
}

EC01G_Status_t BSP_EC01G_EnablePSM(void)
{
    /* TAU=00000001 (2s)，Active Time=00000000 (0s) → 立即进 PSM */
    return SendCmd("AT+CPSMS=1,,,\"00000001\",\"00000000\"\r\n", "OK", 3000);
}

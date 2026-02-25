# EC01G 接入巴法云 TCP 实现计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 在 STM32L431 + EC-01G 项目中新增通过 TCP 9501 上报传感器数据到巴法云的功能。

**Architecture:** 三层：`bsp_ec01g`（AT 指令 + TCP socket）→ `service_nb`（巴法云协议 + JSON）→ `app_main`（状态机集成）。USART2 中断逐字节填充全局 RX 缓冲，`SendCmd` 轮询该缓冲等待期望响应。

**Tech Stack:** STM32L431RCT6, HAL库, USART2 (9600 baud, PA2/PA3), EC-01G NB-IoT 模块, 巴法云 TCP 9501

---

## 预备知识

### 巴法云 TCP 9501 协议
连接后两条消息完成一次上报：
```
鉴权：dff51e3cf58147c687884a86b88b72ea#AKRPL60J0004#\r\n
      → 服务器回复：cmd=1&res=1\r\n

发布：dff51e3cf58147c687884a86b88b72ea#AKRPL60J0004#<json>#\r\n
      → 服务器可能回复：cmd=2&res=1\r\n
```

### EC01G TCP AT 指令序列
```
AT\r\n                                     → OK
AT+CEREG?\r\n                              → +CEREG: 0,1  或 +CEREG: 0,5 表示已注册
AT+NSOCR="STREAM",6,0,1\r\n               → 0\r\nOK  (返回 socket_id=0)
AT+NSOCO=0,"bemfa.com",9501\r\n            → OK
AT+NSOSD=0,<len>,<HEX数据>\r\n             → OK
AT+NSOCL=0\r\n                             → OK
AT+CPSMS=1,,,"00000001","00000000"\r\n     → OK
```

### USART2 中断架构
- `HAL_UART_MspInit` 已使能 USART2 NVIC（优先级2），但 RXNE 中断位未开
- `BSP_EC01G_Init()` 调用 `__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE)` 开启
- `USART2_IRQHandler` 中先读字节填缓冲，再调 `HAL_UART_IRQHandler`（处理错误标志）
- STOP2 唤醒后 `LP_PeriphReinit_Callback` 重跑 `MX_USART2_UART_Init`，RXNE 位被清，下次通信前再调 `BSP_EC01G_Init()` 重开

---

## Task 1：创建 bsp_ec01g.h

**Files:**
- Create: `Core/Inc/bsp_ec01g.h`

**Step 1: 新建头文件**

```c
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
```

**Step 2: 编译确认无报错**

在 CLion 中点 Build 或运行：
```bash
cmake --build build/ -- -j4 2>&1 | head -30
```
预期：编译通过（此时 .h 未被引用，无链接问题）。

**Step 3: Commit**

```bash
git add Core/Inc/bsp_ec01g.h
git commit -m "feat(bsp): add bsp_ec01g.h header"
```

---

## Task 2：创建 bsp_ec01g.c

**Files:**
- Create: `Core/Src/bsp_ec01g.c`

**Step 1: 新建源文件**

```c
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
static char              g_rx_buf[EC01G_RX_BUF_SIZE];
static volatile uint16_t g_rx_len = 0;
static int8_t            g_socket_id = -1;

extern UART_HandleTypeDef huart2;

/* ------------------------------------------------------------------ */
/* 私有函数                                                             */
/* ------------------------------------------------------------------ */

static void ClearRxBuf(void)
{
    g_rx_len = 0;
    memset(g_rx_buf, 0, sizeof(g_rx_buf));
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
        if (strstr(g_rx_buf, expect) != NULL) {
            return EC01G_OK;
        }
    }
    BSP_Debug_Printf("[EC01G] Timeout waiting '%s', got: %s\r\n",
                     expect, g_rx_buf);
    return EC01G_ERR_TIMEOUT;
}

/**
 * @brief 将字节数组转为大写 hex 字符串（不含终止符以外需调用者保证 out 够大）
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
    if (g_rx_len < EC01G_RX_BUF_SIZE - 1u) {
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
        if (strstr(g_rx_buf, ",1") != NULL ||
            strstr(g_rx_buf, ",5") != NULL) {
            BSP_Debug_Printf("[EC01G] Network registered\r\n");
            return EC01G_OK;
        }
        BSP_Debug_Printf("[EC01G] CEREG resp: %s\r\n", g_rx_buf);
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
    if (prefix_len < 0 || (prefix_len + len * 2 + 2) >= (int)sizeof(full_cmd)) {
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
```

**Step 2: 编译确认**

```bash
cmake --build build/ -- -j4 2>&1 | grep -E "error:|warning:"
```
此时 bsp_ec01g.c 未加入 CMakeLists，编译不到它，无报错。下一步加。

**Step 3: Commit**

```bash
git add Core/Src/bsp_ec01g.c
git commit -m "feat(bsp): add bsp_ec01g.c - AT command layer for EC-01G"
```

---

## Task 3：修改 CMakeLists.txt，加入新源文件

**Files:**
- Modify: `CMakeLists.txt`

**Step 1: 在 add_executable 源文件列表中添加**

找到 `CMakeLists.txt` 第 38-48 行的 `add_executable` 块，在末尾（`bsp_hall.c` 后）加入两行：

```cmake
add_executable(${CMAKE_PROJECT_NAME}
        Core/Src/app_main.c
        Core/Src/bsp_debug.c
        Core/Src/bsp_power.c
        Core/Src/bsp_mq4.c
        Core/Src/bsp_lis3dh.c
        Core/Src/bsp_lowpower.c
        Core/Src/bsp_led.c
        Core/Src/bsp_button.c
        Core/Src/bsp_radar.c
        Core/Src/bsp_hall.c
        Core/Src/bsp_ec01g.c
        Core/Src/service_nb.c)
```

**Step 2: 编译（此时 service_nb.c 还不存在，会报错）**

预期报错：`Core/Src/service_nb.c: No such file`
这是正常的，说明 CMake 已识别新文件，继续下一个 Task。

**Step 3: Commit**

```bash
git add CMakeLists.txt
git commit -m "build: add bsp_ec01g.c and service_nb.c to CMakeLists"
```

---

## Task 4：修改 stm32l4xx_it.c，接入 RX 回调

**Files:**
- Modify: `Core/Src/stm32l4xx_it.c`

**Step 1: 在 USER CODE BEGIN Includes 区域加 include**

找到第 24-25 行：
```c
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */
```
改为：
```c
/* USER CODE BEGIN Includes */
#include "bsp_ec01g.h"
/* USER CODE END Includes */
```

**Step 2: 修改 USART2_IRQHandler**

找到第 253-262 行的 `USART2_IRQHandler`：

```c
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}
```

修改为：

```c
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
  /* 先读字节填 EC01G 缓冲，再让 HAL 处理错误标志 */
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE)) {
      uint8_t byte = (uint8_t)(huart2.Instance->RDR & 0xFFu);
      BSP_EC01G_UART_RxCallback(byte);
  }
  /* 清除溢出错误，防止 ORE 引起中断死循环 */
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE)) {
      __HAL_UART_CLEAR_OREFLAG(&huart2);
  }
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}
```

**Step 3: 编译确认（service_nb.c 还不存在，先忽略该错误）**

预期：`bsp_ec01g.h` 引用无报错，USART2_IRQHandler 编译通过。

**Step 4: Commit**

```bash
git add Core/Src/stm32l4xx_it.c
git commit -m "feat(it): wire USART2 RXNE to BSP_EC01G_UART_RxCallback"
```

---

## Task 5：创建 service_nb.h

**Files:**
- Create: `Core/Inc/service_nb.h`

**Step 1: 新建头文件**

```c
/**
 * @file    service_nb.h
 * @brief   NB-IoT 服务层 - 巴法云 TCP 上报接口
 */
#ifndef SERVICE_NB_H
#define SERVICE_NB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_main.h"   /* SensorData_t */

typedef enum {
    NB_OK = 0,
    NB_ERR_NETWORK,   /* 网络未注册 */
    NB_ERR_CONNECT,   /* TCP 连接失败 */
    NB_ERR_SEND,      /* 数据发送失败 */
} NB_Status_t;

/**
 * @brief  完整执行一次巴法云上报：
 *         Init → CheckNetwork → TCPOpen → Auth → Publish → Close → PSM
 * @param  data  传感器采集结果指针
 * @return NB_OK 或对应错误码
 */
NB_Status_t NB_ReportData(const SensorData_t *data);

#ifdef __cplusplus
}
#endif

#endif /* SERVICE_NB_H */
```

**Step 2: Commit**

```bash
git add Core/Inc/service_nb.h
git commit -m "feat(service): add service_nb.h header"
```

---

## Task 6：创建 service_nb.c

**Files:**
- Create: `Core/Src/service_nb.c`

**Step 1: 新建源文件**

```c
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
```

**Step 2: 编译确认**

```bash
cmake --build build/ -- -j4 2>&1 | grep -E "error:|warning:"
```
预期：编译通过，无 error。

**Step 3: Commit**

```bash
git add Core/Src/service_nb.c Core/Inc/service_nb.h
git commit -m "feat(service): add service_nb.c - bemfa TCP report implementation"
```

---

## Task 7：修改 app_main.c，集成 NB_ReportData

**Files:**
- Modify: `Core/Src/app_main.c`

**Step 1: 在文件顶部 include 区域添加**

找到第 22 行 `#include <string.h>`，在其后加一行：

```c
#include "service_nb.h"
```

**Step 2: 替换 state_nb_iot_comm 函数体**

找到第 166-172 行的 `state_nb_iot_comm`：

```c
static void state_nb_iot_comm(void)
{
    BSP_LED_SetRGB(0, 1, 1);
    /* TODO: 实现 EC-01G AT + MQTT 上报 */
    BSP_Debug_Printf("[APP] NB-IoT report placeholder\r\n");
    s_state = APP_STATE_PRE_SLEEP;
}
```

替换为：

```c
static void state_nb_iot_comm(void)
{
    BSP_LED_SetRGB(0, 1, 1);
    NB_Status_t ret = NB_ReportData(&s_sensor);
    if (ret != NB_OK) {
        BSP_Debug_Printf("[APP] NB report failed: %d, will retry next wakeup\r\n",
                         (int)ret);
    }
    s_state = APP_STATE_PRE_SLEEP;
}
```

**Step 3: 编译确认**

```bash
cmake --build build/ -- -j4 2>&1 | grep -E "error:|warning:"
```
预期：编译通过，无 error。

**Step 4: Commit**

```bash
git add Core/Src/app_main.c
git commit -m "feat(app): integrate NB_ReportData into NB_IOT_COMM state"
```

---

## Task 8：硬件联调验证

**验证环境：** 调试模式（上电时按住按键，禁止 STOP2），用串口工具（115200 baud）监听 USART1 输出。

### 8.1 验证 AT 通信

**方法：** 在 `app_main.c` 的 `state_init()` 末尾临时加一行：

```c
/* 临时调试：上电后立即测试 EC01G AT 通信 */
BSP_Debug_Printf("[TEST] EC01G init=%d\r\n", (int)BSP_EC01G_Init());
```

**预期串口输出：**
```
[EC01G] Init OK
[TEST] EC01G init=0
```

如果输出 `init=1`（TIMEOUT），检查：
1. EC01G 是否已上电
2. USART2 波特率是否匹配 EC01G（默认9600，与 main.c 一致）
3. TX/RX 线是否接对（PA2=TX, PA3=RX）

### 8.2 验证网络注册

**方法：** 在调试代码中追加：

```c
BSP_Debug_Printf("[TEST] CEREG=%d\r\n", (int)BSP_EC01G_CheckNetwork());
```

**预期串口输出：**
```
[EC01G] Network registered
[TEST] CEREG=0
```

如果超时（60s 内未注册），检查：
1. SIM 卡是否插好
2. 信号强度：`AT+CSQ` 返回值，`99,99` 表示无信号

### 8.3 验证完整上报流程

**方法：** 移除临时调试代码，触发一次完整的 `NB_IOT_COMM` 状态：
- 在 `state_check_anomaly()` 中临时将 `need_report` 强制为 1
- 或等待 RTC 30s 超时触发

**预期串口输出（完整成功路径）：**
```
[EC01G] Init OK
[EC01G] Network registered
[EC01G] TCP connected to bemfa.com:9501
[NB] Report OK: {"gas":0,"water":65535,"hall":1,"acc_alarm":0,"anomaly":0}
[APP] NB-IoT comm done
```

**巴法云验证：** 登录巴法云控制台，查看主题 `AKRPL60J0004` 最新消息是否更新。

### 8.4 常见问题排查

| 现象 | 可能原因 | 排查方法 |
|---|---|---|
| `NSOCR failed` | SIM 未激活 / 未注册网络就建 socket | 确认 CheckNetwork 先通过 |
| `NSOCO failed` | DNS 解析失败 / 网络不通 | 用 `AT+NPING="8.8.8.8"` 测网络 |
| 巴法云收不到数据 | 鉴权消息格式错误 | 检查 UID 和 TOPIC 字符串是否正确 |
| `TCPSend` 超时 | NSOSD hex 编码错误 | 在 debug 中打印 full_cmd 内容确认格式 |
| 每次上电后第一次 Init 失败 | EC01G 上电后需等待约 3s 初始化 | 在 `state_init()` 末尾加 `HAL_Delay(3000)` |

---

## Task 9：清理临时调试代码并最终提交

**Step 1:** 移除 Task 8 中所有临时加入的调试行

**Step 2:** 最终编译确认

```bash
cmake --build build/ -- -j4 2>&1 | grep -E "error:|warning:"
```

**Step 3:** 最终提交

```bash
git add -A
git commit -m "feat: complete EC01G bemfa TCP integration - all sensors reported via NB-IoT"
```

---

## 附录：已知局限与后续优化（V2）

| 问题 | 当前状态 | V2 方案 |
|---|---|---|
| EC01G 上电等待 | 首次通信可能超时 | 加 3s 上电延时，或检测模块就绪标志 |
| NSODR 未解析 | 服务器下行数据在缓冲中被忽略 | 解析 `+NSODR:` URC，实现下行指令接收 |
| socket_id 固定为 0 | 仅支持单连接 | 从 NSOCR 响应中动态解析 socket_id |
| PSM TAU 值 | 当前设为 2s TAU | 根据运营商调整，12h 周期匹配 RTC 唤醒 |
| 硬件断电 | EC01G 常供电，PSM 有微安待机 | GPIO 控制 PMOS 彻底断电，V2 硬件改版 |

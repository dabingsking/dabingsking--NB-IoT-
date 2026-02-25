# EC01G 接入巴法云 TCP 方案设计文档

**日期**：2026-02-25
**作者**：设计讨论确认
**状态**：已批准，待实现

---

## 一、背景与目标

在现有低功耗井盖监控终端（STM32L431RCT6 + EC-01G NB-IoT）基础上，新增通过巴法云 TCP 长连接上报传感器数据的功能。

### 1.1 接入参数

| 参数项 | 值 |
|---|---|
| 服务器地址 | `bemfa.com` |
| 端口 | `9501`（TCP 透传） |
| Client UID | `dff51e3cf58147c687884a86b88b72ea` |
| 主题 (Topic) | `AKRPL60J0004` |
| 设备名称 | `MyNB01` |

### 1.2 选型决策

| 决策项 | 选择 | 理由 |
|---|---|---|
| 通信协议 | TCP 9501，非 MQTT 8333 | EC01G 原生 TCP AT 指令支持，无需 STM32 侧实现 MQTT 协议栈；低功耗场景连接时间更短 |
| 模块低功耗 | PSM 模式 | EC01G 走外部电源，未经单片机控制；PSM 保持网络注册，唤醒更快；第二版优化为硬件断电方案 |
| JSON timestamp | 去掉 | STM32 无网络对时能力，由巴法云服务器打时间戳 |
| 数据格式 | JSON | 包含全部传感器字段 |

---

## 二、软件架构

### 2.1 分层结构

```
┌─────────────────────────────────────┐
│  app_main.c  (NB_IOT_COMM 状态)     │  应用层
│  NB_ReportData(&g_sensor_data)      │
└───────────────┬─────────────────────┘
                │
┌───────────────▼─────────────────────┐
│  service_nb.c/.h                    │  服务层
│  - 组装 JSON payload                │
│  - 巴法云 TCP 协议封装               │
│  - 连接→鉴权→发布→断开 完整流程      │
└───────────────┬─────────────────────┘
                │
┌───────────────▼─────────────────────┐
│  bsp_ec01g.c/.h                     │  BSP 层
│  - AT 指令发送 / 应答接收            │
│  - TCP socket 操作                  │
│  - PSM 配置                         │
└───────────────┬─────────────────────┘
                │
         HAL USART2 (PA2/PA3)
```

### 2.2 新增 / 修改文件清单

| 文件 | 类型 | 说明 |
|---|---|---|
| `Core/Inc/bsp_ec01g.h` | 新增 | EC01G BSP 层头文件 |
| `Core/Src/bsp_ec01g.c` | 新增 | EC01G AT 指令封装实现 |
| `Core/Inc/service_nb.h` | 新增 | NB-IoT 服务层头文件 |
| `Core/Src/service_nb.c` | 新增 | 巴法云上报业务逻辑实现 |
| `Core/Src/app_main.c` | 修改 | 填充 `NB_IOT_COMM` 状态调用 |
| `CMakeLists.txt` | 修改 | 加入两个新源文件 |

---

## 三、bsp_ec01g 层设计

### 3.1 对外接口

```c
typedef enum {
    EC01G_OK = 0,
    EC01G_ERR_TIMEOUT,   // AT 指令超时无响应
    EC01G_ERR_NETWORK,   // 网络未注册
    EC01G_ERR_SOCKET,    // socket 操作失败
} EC01G_Status_t;

// 验证 AT 通信正常（UART2 由 main.c 已初始化）
EC01G_Status_t BSP_EC01G_Init(void);

// 轮询 AT+CEREG? 直到注册成功或超时（60s）
EC01G_Status_t BSP_EC01G_CheckNetwork(void);

// AT+NSOCR 创建 socket，AT+NSOCO 连接，socket_id 存内部静态变量
EC01G_Status_t BSP_EC01G_TCPOpen(const char *host, uint16_t port);

// hex 编码数据后通过 AT+NSOSD 发送
EC01G_Status_t BSP_EC01G_TCPSend(const uint8_t *data, uint16_t len);

// AT+NSOCL 关闭 socket
EC01G_Status_t BSP_EC01G_TCPClose(void);

// AT+CPSMS=1 配置 PSM，通信完成后调用
EC01G_Status_t BSP_EC01G_EnablePSM(void);
```

### 3.2 内部机制

- **RX 缓冲区**：`static char g_ec01g_rx_buf[512]`，由 `USART2_IRQHandler` 逐字节填充
- **SendCmd**：发送 AT 指令后轮询 `g_ec01g_rx_buf`，在指定超时内找到期望字符串则返回 `EC01G_OK`
- **BytesToHex**：将字节数组转为 hex 字符串，供 `AT+NSOSD` 使用

### 3.3 AT 指令序列

| 步骤 | 指令 | 期望响应 | 超时 |
|---|---|---|---|
| 1 | `AT\r\n` | `OK` | 2000ms |
| 2 | `AT+CEREG?\r\n` | `,1` 或 `,5` | 60000ms（循环重试） |
| 3 | `AT+NSOCR="STREAM",6,0,1\r\n` | `OK` | 5000ms |
| 4 | `AT+NSOCO=0,"bemfa.com",9501\r\n` | `OK` | 10000ms |
| 5 | `AT+NSOSD=0,<len>,<hex鉴权串>\r\n` | `OK` | 5000ms |
| 6 | 等待服务器下行 | `cmd=1&res=1` | 5000ms |
| 7 | `AT+NSOSD=0,<len>,<hex数据串>\r\n` | `OK` | 5000ms |
| 8 | `AT+NSOCL=0\r\n` | `OK` | 3000ms |
| 9 | `AT+CPSMS=1,,,"00000001","00000000"\r\n` | `OK` | 3000ms |

---

## 四、service_nb 层设计

### 4.1 对外接口

```c
typedef enum {
    NB_OK = 0,
    NB_ERR_NETWORK,   // 网络未注册
    NB_ERR_CONNECT,   // TCP 连接失败
    NB_ERR_SEND,      // 数据发送失败
} NB_Status_t;

// 唯一对外接口：完整执行一次上报流程
NB_Status_t NB_ReportData(const SensorData_t *data);
```

### 4.2 巴法云 TCP 9501 协议

```
TCP 连接建立后：

鉴权（订阅）：uid#topic#\r\n
              dff51e3cf58147c687884a86b88b72ea#AKRPL60J0004#\r\n
              → 服务器回复：cmd=1&res=1\r\n

发布数据：    uid#topic#msg#\r\n
              dff51e3cf58147c687884a86b88b72ea#AKRPL60J0004#<json>#\r\n
              → 服务器回复：cmd=2&res=1\r\n（或无回复）
```

### 4.3 NB_ReportData 内部流程

```
1. BSP_EC01G_Init()            验证 AT 通信
        ↓ 失败 → 返回 NB_ERR_NETWORK
2. BSP_EC01G_CheckNetwork()    确认已注册网络（最多等 60s）
        ↓ 失败 → 返回 NB_ERR_NETWORK
3. BSP_EC01G_TCPOpen("bemfa.com", 9501)
        ↓ 失败 → 返回 NB_ERR_CONNECT
4. 发送鉴权消息，等待 cmd=1&res=1
        ↓ 失败 → TCPClose，返回 NB_ERR_CONNECT
5. 组装 JSON，发送发布消息
        ↓ 失败 → TCPClose，返回 NB_ERR_SEND
6. BSP_EC01G_TCPClose()
7. BSP_EC01G_EnablePSM()
8. 返回 NB_OK
```

### 4.4 JSON 格式

```json
{
    "gas": 500,
    "water": 32,
    "hall": 1,
    "acc_alarm": 0,
    "anomaly": 1
}
```

组装代码：
```c
snprintf(json_buf, sizeof(json_buf),
    "{\"gas\":%lu,\"water\":%lu,\"hall\":%d,\"acc_alarm\":%d,\"anomaly\":%d}",
    data->gas_ppm, data->water_cm,
    data->hall_state, data->acc_alarm, data->anomaly);
```

---

## 五、app_main.c 集成

`NB_IOT_COMM` 状态修改：

```c
case APP_STATE_NB_IOT_COMM:
    NB_Status_t ret = NB_ReportData(&g_sensor_data);
    if (ret != NB_OK) {
        BSP_Debug_Printf("NB report failed: %d\r\n", ret);
        // 失败不阻塞，直接进睡眠，下次唤醒重试
    }
    app_state = APP_STATE_PRE_SLEEP;
    break;
```

---

## 六、后续优化方向（V2）

| 优化项 | 说明 |
|---|---|
| 硬件断电控制 | 通过单片机 GPIO 控制 EC01G 电源，完全断电省电最优 |
| 网络注册失败重试策略 | 指数退避，避免长时间等待耗电 |
| NSOSD 响应解析 | 目前只检查 OK，后续可解析 `+NSOSD:` 确认发送字节数 |
| 服务器下行指令 | 监听巴法云下发的控制指令（如远程重置） |

---

## 七、硬件备注

- EC01G 电源由外部电路控制，未经单片机 GPIO，V1 阶段不做硬件断电
- USART2 (PA2/PA3) 已在 `main.c` 中由 HAL 初始化，bsp_ec01g 直接使用
- EC01G RST 引脚当前未定义，V1 不做硬件复位

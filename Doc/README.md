# 基于 NB-IoT 的低功耗城市下水道监控终端


> **固件工程名**：Lowpower_01
> **最后更新**：2026-03-19

---

## 目录

1. [项目简介](#1-项目简介)
2. [硬件平台](#2-硬件平台)
3. [软件架构](#3-软件架构)
4. [目录结构](#4-目录结构)
5. [云平台接入（ThingsCloud）](#5-云平台接入thingscloud)
6. [快速上手](#6-快速上手)
7. [调试模式](#7-调试模式)
8. [数据上报格式](#8-数据上报格式)
9. [关键阈值与告警策略](#9-关键阈值与告警策略)
10. [文档索引](#10-文档索引)

---

## 1. 项目简介

本项目面向城市排水管网无人值守场景，设计了一款基于 **STM32L431RCT6 + NB-IoT（EC-01G）** 的低功耗远程监控终端。终端部署于井下，可实时监测：

- **沼气/可燃气浓度**（MQ-4 传感器）
- **管道水位**（JSN-SR04T 超声波测距）
- **井盖开合状态**（霍尔传感器）
- **井盖异动/碰撞**（LIS3DH 三轴加速度计）

采集数据通过 NB-IoT 模块上报至 **ThingsCloud** 物联网平台，支持异常告警与周期心跳上报。整个系统以 **"采集–判断–按需上报"** 为核心策略，绝大多数时间处于 **STOP2 超低功耗模式**，实现电池供电场景下的长期续航。

---

## 2. 硬件平台

### 2.1 核心模块

| 模块 | 型号 / 规格 | 说明 |
|:---|:---|:---|
| 主控 MCU | STM32L431RCT6 | Cortex-M4，80 MHz，支持 STOP2 |
| NB-IoT 通信 | EC-01G | 支持 PSM/eDRX，AT 指令控制 |
| 气体传感器 | MQ-4 | 甲烷/沼气检测，模拟量输出，需 30 s 预热 |
| 超声波测距 | JSN-SR04T | 防水型，量程 25 cm – 4.5 m，用于水位监测 |
| 加速度计 | LIS3DH | I2C 接口，支持运动唤醒中断（EXTI） |
| 霍尔传感器 | 开关型 | 检测井盖开合状态 |
| 电源系统 | 2 × 18650 锂电池（7.4 V） | PMOS 分时控制 MQ-4 / 超声波模块供电 |

### 2.2 引脚分配

| 引脚 | 功能标识 | 外设 | 说明 |
|:---|:---|:---|:---|
| PA0 | ADC1_IN5 | MQ-4 | 气体浓度模拟量输入 |
| PA2 | USART2_TX | EC-01G | AT 指令发送 |
| PA3 | USART2_RX | EC-01G | AT 指令接收 |
| PA6 | RADAR_TRIG | JSN-SR04T | 超声波触发脉冲（GPIO_Output） |
| PA7 | RADAR_ECHO | JSN-SR04T | 回波接收（GPIO_Input，EXTI） |
| PA8 | ACC_INT1 | LIS3DH | 运动唤醒中断（EXTI9_5，最高优先级） |
| PB0 | GAS_PWR_CTRL | MQ-4 电源 | PMOS 控制（低电平开启） |
| PB1 | RADAR_PWR_CTRL | JSN-SR04T 电源 | PMOS 控制（低电平开启） |
| PC5 | HALL_IN | 霍尔传感器 | 井盖状态（GPIO_Input） |

---

## 3. 软件架构

系统采用三层架构，严格按层调用，禁止跨层直接访问：

```
┌────────────────────────────────────────┐
│         应用层 (App)                   │
│   app_main.c  ← 主状态机 / 业务逻辑   │
├────────────────────────────────────────┤
│         服务层 (Service)               │
│   service_nb.c ← NB-IoT / MQTT 封装   │
├────────────────────────────────────────┤
│         驱动层 (BSP)                   │
│   bsp_ec01g   bsp_mq4    bsp_radar     │
│   bsp_hall    bsp_lis3dh bsp_power     │
│   bsp_led     bsp_button bsp_debug     │
│   bsp_lowpower                         │
├────────────────────────────────────────┤
│         STM32 HAL / LL 库              │
└────────────────────────────────────────┘
```

### 3.1 主状态机

```
上电
  │
  ▼
[INIT] ──► 配置 RTC 唤醒定时器，检测上电按键
  │
  ▼
[PRE_SLEEP] ──► 关闭传感器电源，使能 EXTI，进入 STOP2
  │
  ▼
[SLEEP] ◄────────────────────────────────┐
  │ 唤醒（RTC 定时 / LIS3DH 中断）      │
  ▼                                      │
[WAKEUP_RESTORE] ── 恢复时钟/外设，识别唤醒原因
  │                                      │
  ▼                                      │
[COLLECT_DATA]                           │
  │ 霍尔 + LIS3DH（轻量，每次均采集）   │
  │ 雷达 + MQ-4（全量，仅 RTC 唤醒时）  │
  ▼                                      │
[CHECK_ANOMALY]                          │
  │ 无异常且非 RTC 心跳 ──────────────► │
  │ 有异常 或 RTC 心跳                  │
  ▼                                      │
[NB_IOT_COMM] ── 三次重试上报 ──────────┘
```

### 3.2 唤醒源

| 唤醒源 | 触发条件 | 采集类型 |
|:---|:---|:---|
| RTC WakeUp Timer | 默认每 **12 小时**（可通过 `App_SetWakeupPeriod()` 调整） | **全量采集**（含 MQ-4、超声波） |
| EXTI PA8（LIS3DH） | 加速度超过阈值（井盖异动/碰撞） | **轻量采集**（霍尔 + 加速度） |

---

## 4. 目录结构

```
Lowpower_01/
├── Core/
│   ├── Inc/
│   │   ├── app_main.h          # 应用层头文件（状态枚举、数据结构、接口）
│   │   ├── service_nb.h        # NB-IoT 服务层头文件
│   │   ├── bsp_ec01g.h         # EC-01G AT 指令驱动
│   │   ├── bsp_mq4.h           # MQ-4 气体传感器驱动
│   │   ├── bsp_radar.h         # JSN-SR04T 超声波驱动
│   │   ├── bsp_hall.h          # 霍尔传感器驱动
│   │   ├── bsp_lis3dh.h        # LIS3DH 加速度计驱动
│   │   ├── bsp_power.h         # 外设电源控制
│   │   ├── bsp_lowpower.h      # STOP2 低功耗管理
│   │   ├── bsp_led.h           # LED 指示灯
│   │   ├── bsp_button.h        # 用户按键
│   │   └── bsp_debug.h         # 调试串口输出
│   └── Src/
│       ├── main.c              # HAL 初始化入口，调用 App_Run()
│       ├── app_main.c          # 应用层主状态机（核心业务逻辑）
│       ├── service_nb.c        # NB-IoT / MQTT 上报服务
│       ├── bsp_ec01g.c         # EC-01G AT 指令驱动实现
│       ├── bsp_mq4.c           # MQ-4 采集与估算
│       ├── bsp_radar.c         # 超声波测距（中值滤波）
│       ├── bsp_hall.c          # 霍尔状态读取（去抖）
│       ├── bsp_lis3dh.c        # LIS3DH I2C 驱动 + 倾角计算
│       ├── bsp_power.c         # PMOS 电源控制
│       ├── bsp_lowpower.c      # STOP2 进入/退出/唤醒识别
│       ├── bsp_led.c
│       ├── bsp_button.c
│       └── bsp_debug.c
├── Doc/
│   ├── README.md               # ← 本文件
│   ├── ARCHITECTURE.md         # 软件设计说明书（详细架构）
│   ├── DEV_PLAN_SENSORS_AND_ARCH.md  # 传感器优先开发计划
│   ├── MQTT指令集.md           # EC-01G MQTT AT 指令参考
│   ├── NB_MQTT_connect_and_garbled_text_plan_2026-02-28.md
│   ├── 基于NB-IoT的低功耗城市下水道监控终端 软件开发技术文档.md
│   ├── 大纲.md
│   ├── 指令集.md
│   ├── 流程.md
│   ├── 论文1.md
│   ├── 论文1.1.md
│   ├── Serial port data/       # 串口抓包原始数据
│   └── References/             # 参考文献 PDF
└── CMakeLists.txt
```

---

## 5. 云平台接入（ThingsCloud）

本项目当前使用 **ThingsCloud** 物联网平台（MQTT 协议）进行数据上报。

### 5.1 接入参数

> ⚠️ 以下为占位符，请替换为您自己在 ThingsCloud 控制台创建设备后获得的实际参数。

| 参数项 | 占位符 | 说明 |
|:---|:---|:---|
| MQTT Broker | `<YOUR_MQTT_HOST>` | 例如 `bj-2-mqtt.iot-api.com` |
| MQTT Port | `<YOUR_MQTT_PORT>` | 默认 `1883` |
| Client ID | `thingscloud` | 固定值（ThingsCloud 协议要求） |
| AccessToken（用户名） | `<YOUR_ACCESS_TOKEN>` | 设备 AccessToken，在 ThingsCloud 控制台获取 |
| ProjectKey（密码） | `<YOUR_PROJECT_KEY>` | 项目 Key，在 ThingsCloud 控制台获取 |
| 发布主题 | `attributes` | 上报传感器属性数据 |
| 订阅主题 | `attributes/push` | 接收云平台下行响应 |

### 5.2 配置位置

上述参数集中定义在 `Core/Src/service_nb.c` 顶部的宏：

```c
#define THINGSCLOUD_HOST   "<YOUR_MQTT_HOST>"
#define THINGSCLOUD_PORT   <YOUR_MQTT_PORT>
#define THINGSCLOUD_TOKEN  "<YOUR_ACCESS_TOKEN>"   /* MQTT Username */
#define THINGSCLOUD_KEY    "<YOUR_PROJECT_KEY>"    /* MQTT Password */
```

修改上述宏后重新编译即可。

### 5.3 通信流程

```
1. BSP_EC01G_Init()         → 复位模块，验证 AT 通信正常
2. BSP_EC01G_CheckSIM()     → 检查 SIM 卡状态
3. 等待 +CREG: 6            → 确认 NB-IoT 网络注册成功（最长 60 s）
4. BSP_EC01G_MQTTOpen()     → 建立 TCP 连接至 Broker
5. BSP_EC01G_MQTTConnect()  → MQTT 认证登录
6. BSP_EC01G_MQTTSubscribe()→ 订阅 attributes/push
7. BSP_EC01G_MQTTPublish()  → 发布 JSON 数据至 attributes
8. BSP_EC01G_MQTTDisconnect()→ 断开连接，准备回睡眠
```

---

## 6. 快速上手

### 6.1 开发环境

| 工具 | 版本要求 |
|:---|:---|
| CLion | 任意近期版本，需安装 STM32 插件 |
| CMake | ≥ 3.22 |
| arm-none-eabi-gcc | ≥ 10.x |
| STM32CubeMX | ≥ 6.x（引脚/时钟配置参考用） |
| ST-Link / OpenOCD | 用于烧录与调试 |

### 6.2 编译与烧录

```bash
# 在项目根目录执行
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../arm-none-eabi.cmake
make -j4

# 使用 ST-Link 烧录
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg \
        -c "program Lowpower_01.elf verify reset exit"
```

> 也可直接在 CLion 中点击 **Run / Debug** 配置一键编译烧录。

### 6.3 修改接入参数

1. 打开 `Core/Src/service_nb.c`。
2. 将 `THINGSCLOUD_HOST`、`THINGSCLOUD_TOKEN`、`THINGSCLOUD_KEY` 替换为实际值。
3. 重新编译烧录。

---

## 7. 调试模式

### 7.1 进入调试模式

**上电时按住用户按键**，LED 亮绿灯约 150 ms 后变蓝，即进入**调试模式**（禁止 STOP2 休眠）。

- 调试模式下系统持续运行，每 **3 秒**通过调试串口打印一次所有传感器数据。
- MQ-4 不执行 30 s 预热，快速采样便于趋势观察。

### 7.2 运行时切换

在正常运行期间，**短按**用户按键可临时开启/关闭调试模式（LED 闪白后恢复蓝灯指示）。

### 7.3 调试串口

| 参数 | 值 |
|:---|:---|
| 波特率 | 115200 |
| 数据位 | 8 |
| 停止位 | 1 |
| 校验位 | 无 |

典型日志输出示例：

```
[APP] Init done, RTC wakeup=43200 s
[APP] Wakeup reason=1
[LIS3DH] pitch=1.2 roll=0.8 cover=0
[MQ4] adc=1024 sensor_mv=825 gas_ppm=412
[APP] Collect: hall=1 acc_alarm=0 water=135 gas=412
[APP] Anomaly=0 need_report=1
[NB] Network registered
[NB] ThingsCloud report OK: {"water_level":135,"gas_ppm":412,"hall_state":1,"acc_alarm":0,"anomaly":0}
[APP] Entering STOP2, wakeup in 43200 s
```

### 7.4 LED 状态指示

| LED 颜色 | 含义 |
|:---|:---|
| 蓝色 | 系统正常运行 / 初始化完成 |
| 绿色 | 传感器采集中 |
| 青色（蓝+绿） | NB-IoT 通信中 |
| 白色闪烁 | 按键响应 |
| 熄灭 | 进入 STOP2 休眠 |

---

## 8. 数据上报格式

上报至 ThingsCloud `attributes` 主题的 JSON 格式：

```json
{
    "water_level": 135,
    "gas_ppm": 412,
    "hall_state": 1,
    "acc_alarm": 0,
    "anomaly": 0
}
```

| 字段 | 类型 | 说明 |
|:---|:---|:---|
| `water_level` | 整数（cm）/ `-1` | 水位距传感器距离（cm）；雷达无效时上报 `-1` |
| `gas_ppm` | 整数（PPM） | 可燃气体估算浓度（线性映射，未精确标定） |
| `hall_state` | 0 / 1 | 霍尔传感器状态：`1` = 井盖闭合，`0` = 井盖开启 |
| `acc_alarm` | 0 / 1 | 加速度异动告警：`1` = 检测到倾斜/碰撞事件 |
| `anomaly` | 0 / 1 | 综合异常标志：任一子告警触发时置 `1` |

---

## 9. 关键阈值与告警策略

### 9.1 异常判断阈值

| 监测项 | 告警条件 | 对应字段 |
|:---|:---|:---|
| 可燃气浓度 | `gas_ppm > 1000 PPM` | `anomaly = 1` |
| 水位 | `water_level < 50 cm` | `anomaly = 1` |
| 井盖状态 | `hall_state == 0`（开启） | `anomaly = 1` |
| 异动告警 | `acc_alarm == 1` | `anomaly = 1` |

> 阈值常量定义在 `Core/Inc/app_main.h`，可按需调整：
> ```c
> #define APP_GAS_THRESHOLD_PPM    1000u
> #define APP_WATER_THRESHOLD_CM   50u
> ```

### 9.2 上报策略

| 场景 | 上报行为 |
|:---|:---|
| RTC 定时唤醒（12 h） | **强制上报**（心跳包），无论是否异常 |
| LIS3DH 中断唤醒 + 无异常 | **不上报**，直接回睡眠（节省功耗） |
| LIS3DH 中断唤醒 + 有异常 | **立即上报** |
| 上报失败 | 最多重试 **3 次**，退避间隔 2 s；全部失败后回睡眠 |

---

## 10. 文档索引

本项目 `Doc/` 目录下包含以下参考文档（均保留，不删除）：

| 文档名 | 说明 |
|:---|:---|
| **README.md**（本文件） | 项目快速入门总览 |
| **ARCHITECTURE.md** | 软件设计说明书（详细架构、外设配置、状态机设计） |
| **DEV_PLAN_SENSORS_AND_ARCH.md** | 传感器 BSP 驱动优先开发计划 |
| **MQTT指令集.md** | EC-01G NB-IoT 模组 MQTT AT 指令参考手册 |
| **NB_MQTT_connect_and_garbled_text_plan_2026-02-28.md** | NB-IoT 连接问题排查与乱码修复方案 |
| **基于NB-IoT的低功耗城市下水道监控终端 软件开发技术文档.md** | 软件开发技术文档（硬件参数清单、实现方案） |
| **大纲.md** | 毕业论文大纲 |
| **指令集.md** | AT 指令使用备忘 |
| **流程.md** | 系统工作流程图 |
| **论文1.md / 论文1.1.md** | 毕业论文草稿 |
| **Serial port data/** | 串口通信原始抓包数据 |
| **References/** | 参考文献 PDF（井盖监测相关论文） |

---

*本文档由 Claude Sonnet 4.6 辅助生成，如有更新请同步修改本文件。*

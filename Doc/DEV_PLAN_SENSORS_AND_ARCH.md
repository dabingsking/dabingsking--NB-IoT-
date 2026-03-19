## 低功耗下水道终端开发计划（传感器优先，自底向上）

### 0. 总体路线

- **阶段 1：传感器 BSP 底层驱动打磨**
  - 目标：MQ-4、JSN-SR04T、霍尔、LIS3DH 均具备 **稳定、可复用的 BSP 接口**，能在当前硬件条件下可靠读数。
  - 要求：只做“能稳定读数据 + 必要硬件时序/电源控制 + 基本滤波/错误码”，不在 BSP 写高层业务逻辑。

- **阶段 2：各传感器独立 APP Demo**
  - 目标：为每个传感器写一个简单的“小应用测试函数”，只依赖 BSP，方便单独联调。
  - 要求：每个 demo 能在串口上连续输出调试信息，便于判断驱动是否稳定。

- **阶段 3：整体架构与目录重构（APP + BSP + HAL 三层）**
  - 目标：在底层稳定的前提下，整理目录结构和 `main.c`，实现 APP 主状态机，最终形成干净的三层架构。
  - 参考文档：`Doc/APP_BSP_HAL_ARCHITECTURE.md`

---

## 1. 阶段 1：传感器 BSP 底层驱动完善

### 1.1 MQ-4 气体传感器（`bsp_mq4`）

**目标**：在不追求精确标定的前提下，提供一套能稳定输出“估算浓度”的接口，并明确错误行为。

**需要具备的接口（示例）**

- `void BSP_MQ4_Init(void);`
- `void BSP_MQ4_PreheatOnce(void);`  
  - 首次调用阻塞预热（默认 30 s，可通过宏配置）；  
  - 后续调用立即返回。
- `uint16_t BSP_MQ4_ReadAdcOnce(void);`  
  - 单次采样，返回原始 ADC 值（0–4095），超时/失败时返回 0 或特定错误码。
- `uint16_t BSP_MQ4_ReadAdcAverage(uint8_t n, uint32_t interval_ms);`  
  - 多次采样取平均，提高稳定性。
- `uint16_t BSP_MQ4_AdcToAo_mV(uint16_t adc_raw);`  
  - ADC → AO 节点电压（mV）。
- `uint16_t BSP_MQ4_AoToSensor_mV(uint16_t ao_mv);`  
  - AO 节点电压 → 传感器实际输出电压（考虑分压 2/3）。
- `uint16_t BSP_MQ4_Sensor_mV_ToPpmEst(uint16_t sensor_mv);`  
  - 电压 → 估算 PPM（占位的线性映射，未校准，用于“有数可看”）。

**注意点**

- 当前电源控制为占位实现（5V 直供），后续由 `BSP_Power_GasOn/Off()` 接入 MOS 开关后再调整预热/断电逻辑。
- 明确错误约定：例如 ADC 一直为 0 时打印告警，但不死循环。

---

### 1.2 JSN-SR04T 超声波雷达（`bsp_radar`）

**目标**：封装“单次测距 + 5 次滤波测距”，让上层只关心“距离 cm 或错误码”。

**建议接口**

- 初始化：
  - `void BSP_Radar_Init(void);`  
    - 配置 TRIG/ECHO GPIO 与 EXTI，中断优先级，确保 TIM2 已由 HAL 层初始化并以 1 MHz 运行。
- 单次测距（底层原始接口）：
  - `static uint32_t BSP_Radar_SingleMeasureRawUs(void);`  
    - 触发一次测量，使用 EXTI + TIM2 记录 ECHO 脉宽（µs）；  
    - 超时或信号异常返回 0 或特殊错误码。
- 高层测距（滤波接口）：
  - `uint16_t BSP_Radar_Measure(void);`  
    - 连续测量 5 次；  
    - 进行中值 + 均值滤波；  
    - 返回距离（cm）；无效/超范围时返回 `0xFFFF`。
- EXTI 回调：
  - `void BSP_Radar_EXTI_Callback(uint16_t GPIO_Pin);`  
    - 在 `HAL_GPIO_EXTI_Callback()` 中转发 ECHO 事件，由 BSP 内部状态机处理。

**注意点**

- 硬件前提（接线、去耦、电源开关极性）在 `Doc/ARCHITECTURE.md` 已有记录，驱动实现中要对“无回波/毛刺”做防御（例如上升沿/下降沿状态机 + 超时保护）。
- 对异常测量值（过大、过小、不稳定）要统一转为 `0xFFFF`，避免 APP 层误将乱值当作真实水位。

---

### 1.3 霍尔传感器（预期 `bsp_hall`）

**目标**：提供一个“稳定的 0/1 井盖状态”，并通过简单去抖避免误触发。

**建议接口**

- `uint8_t BSP_Hall_ReadState(void);`  
  - 直接读取 GPIO 电平：1 = 闭合，0 = 打开（按当前电路逻辑）。
- `uint8_t BSP_Hall_ReadStateDebounced(void);`  
  - 5 次采样 + 多数表决，采样间隔 5–10 ms，总耗时约 20–50 ms；  
  - 返回去抖后的稳定状态（1 = 闭合，0 = 打开）。

**注意点**

- 去抖策略保持简单可靠，不记录历史，也不在 BSP 里做“状态变化事件”逻辑，这部分留给后续 APP 层。

---

### 1.4 LIS3DH 加速度计（`bsp_lis3dh`）

**目标**：保持现有较成熟实现，提供“初始化 + 读加速度 + 读中断源”的稳定接口，后续 APP 层以轮询和简单阈值判断为主。

**接口方向（已有为主）**

- `HAL_StatusTypeDef LIS3DH_Init(void);`
- `HAL_StatusTypeDef LIS3DH_ReadAccelRaw(int16_t *x, int16_t *y, int16_t *z);`
- `HAL_StatusTypeDef LIS3DH_ReadAccelG(float *ax_g, float *ay_g, float *az_g);`
- `HAL_StatusTypeDef LIS3DH_ReadInt1Src(uint8_t *int1_src);`

**注意点**

- 偏向于使用“RTC 唤醒 + 轮询加速度”的方案，中断唤醒部分已在文档中标记为存在时序问题，暂不作为主路径。

---

### 1.5 电源控制（`bsp_power`）

**目标**：为 MQ-4 与 JSN-SR04T 提供统一的上/下电接口，当前可继续占位实现，待硬件极性实测后填充。

**接口**

- `void BSP_Power_GasOn(void);`
- `void BSP_Power_GasOff(void);`
- `void BSP_Power_RadarOn(void);`
- `void BSP_Power_RadarOff(void);`

**注意点**

- 确认好 MOS 开关极性后，只在 `bsp_power.c` 内部调整具体 GPIO 电平；  
- 其他 BSP 模块（MQ-4、雷达）只能调用 `BSP_Power_XXX()`，不直接写 GPIO。

---

## 2. 阶段 2：各传感器独立 APP Demo

在 BSP 稳定后，为每个传感器写一个独立的小 demo，放在 APP 层（后续可迁移到 `Core/App` 目录）。

### 2.1 MQ-4 Demo

**目标**：验证预热逻辑、ADC 读数和 PPM 估算是否合理。

**示例接口**

- `void App_MQ4_Demo(void);`

**逻辑大致为**

- 调用 `BSP_MQ4_Init()`；
- 在循环中：
  - `BSP_MQ4_PreheatOnce();`
  - `adc = BSP_MQ4_ReadAdcAverage(...);`
  - `mv = BSP_MQ4_AdcToSensor_mV(adc);`
  - `ppm = BSP_MQ4_Sensor_mV_ToPpmEst(mv);`
  - `BSP_Debug_Printf()` 打印原始值与估算值；
  - 适当延时（例如 1 s）。

### 2.2 JSN-SR04T Demo

**目标**：验证单次测距与 5 次滤波的稳定性。

**示例接口**

- `void App_Radar_Demo(void);`

**逻辑大致为**

- 调用 `BSP_Radar_Init();`
- 在循环中：
  - `distance = BSP_Radar_Measure();`
  - 若为有效值，打印距离；否则打印“测距失败/超时”；
  - 延时 0.5–1 s。

### 2.3 霍尔 Demo

**目标**：验证去抖后状态是否稳定，记录简单状态变化。

**示例接口**

- `void App_Hall_Demo(void);`

**逻辑大致为**

- 定期调用 `BSP_Hall_ReadStateDebounced();`
- 如果与上一次状态不同，则打印“井盖开/关变化”；
- 可用 LED 做简单指示（例如打开=亮，关闭=灭）。

### 2.4 LIS3DH Demo

**目标**：验证轮询读取加速度数据是否稳定，检测简单“震动事件”。

**示例接口**

- `void App_Lis3dh_Demo(void);`

**逻辑大致为**

- 周期性调用 `LIS3DH_ReadAccelG()` 打印 ax/ay/az；
- 可加一个粗略软件阈值（例如 |a| > 某个 g）打印“摇晃/撞击”提示。

---

## 3. 阶段 3：整体架构与目录重构

在上述 BSP 与各自 Demo 都运行稳定后，再进行整体整理。

### 3.1 目录结构调整（草案）

- `Core/Bsp/`
  - 所有 `bsp_*.c` / `bsp_*.h`。
- `Core/App/`
  - `app_main.c/.h`（主状态机）。  
  - 各个传感器 Demo 或子模块：`app_mq4_demo.c`、`app_radar_demo.c` 等（必要时）。
- `Core/Src/`
  - `main.c`、`stm32l4xx_it.c`、`syscalls.c`、`sysmem.c` 等启动/中断/系统文件。

（实际迁移时根据 IDE/CMake 配置适当微调。）

### 3.2 main.c 精简与 APP 主状态机接入

- 保留：
  - HAL 初始化：`HAL_Init()` + `SystemClock_Config()`；
  - HAL/CubeMX 外设初始化：`MX_*_Init()`；
  - BSP 初始化：`BSP_Debug_Init()`、`BSP_LED_Init()`、`BSP_Button_Init()`、`LIS3DH_Init()` 等。
- 新增 APP 层入口：
  - `App_Init();`
  - 主循环改为：`for (;;) { App_RunOnce(); }`
- 将目前 main 中的：
  - MQ-4 输出逻辑；
  - 雷达测距逻辑；
  - 按键调试模式切换；
  - STOP2 进入/退出调度；
  逐步迁移到 APP 层的主状态机和/或各自子模块中。

### 3.3 与 `APP_BSP_HAL_ARCHITECTURE.md` 对齐

- 以 `Doc/APP_BSP_HAL_ARCHITECTURE.md` 为“分层规范”，确保：
  - APP 层不直接使用 HAL 接口；
  - BSP 层不包含业务策略，只封装板级细节；
  - 所有新接口命名遵循 `BSP_` / `App_` 前缀约定。

---

## 4. 时间和优先级建议（可根据实际进度调整）

- 阶段 1：BSP 驱动打磨  
  - MQ-4 / JSN-SR04T / 霍尔 / LIS3DH / 电源控制  
  - 建议：1–2 周（视硬件调试情况而定）。

- 阶段 2：各传感器 Demo  
  - 每个 Demo 写完后立即联调，保留为长期回归测试入口。  
  - 建议：1 周左右。

- 阶段 3：重构架构与 main  
  - 目录调整、主状态机引入、逻辑迁移与回归测试。  
  - 建议：1–2 周。

（以上为粗略估计，可根据毕设节奏灵活压缩或拆分。）

---

本计划文档主要给“后续开发顺序和边界”定个调子：**先把每个传感器底层打磨好，再用小 APP 验证，最后一次性收拢成干净的三层架构**。实际实现细节以代码和其他设计文档（如 `ARCHITECTURE.md`、`APP_BSP_HAL_ARCHITECTURE.md`）为准。


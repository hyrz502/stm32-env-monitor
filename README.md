# STM32 智能环境监控终端

基于 FreeRTOS 的多任务环境监控终端，使用 STM32F103 + DHT11 + OLED 实现 温湿度采集、光照检测、本地显示、串口远程控制、超限报警 功能。

## 硬件清单

| 模块 | 型号 | 接口 | 用途 |
|------|------|------|------|
| 主控 | STM32F103C8T6 | — | Cortex-M3，72MHz |
| 温湿度传感器 | DHT11 | GPIO（单总线） | 温度+湿度采集 |
| 显示屏 | 0.96寸 OLED | I2C | 本地数据显示 |
| 光敏传感器 | 光敏电阻 | ADC | 光照强度采集 |
| LED | — | GPIO | 报警指示 |
| 按键 x2 | — | GPIO + EXTI | 页面切换 / 配置调节 |
| 串口 | CH340/USB-TTL | UART1 | 上位机通信 |

## 功能列表

- **数据采集**：每1秒（可配置1-60秒）采集温度、湿度、光照，保留最近10条历史记录
- **本地显示**：OLED双页面切换
  - 数据页：实时温度/湿度/光照/运行时间
  - 配置页：采样间隔/报警阈值/自动报警开关
- **串口远程控制**：自定义二进制帧协议，支持6条命令
- **超限报警**：温度超过阈值时LED闪烁 + 串口自动上报
- **按键控制**：短按切换配置项，长按进入/退出配置页，短按调节参数

## 系统架构

### 任务划分

| 任务 | 优先级 | 栈大小 | 职责 |
|------|--------|--------|------|
| CollectTask | Normal | 512 words | 周期采集温湿度/光照，维护历史数据，设置报警事件标志 |
| ShowTask | Low | 1024 words | OLED双页面刷新显示，本地时间计时 |
| SerialTask | Low | 512 words | 串口DMA接收 + 帧协议解析 + 命令执行 |
| AlarmTask | Normal1 | 512 words | 事件组多条件等待，控制LED报警闪烁 |
| KeyTask | Low | 512 words | 按键消抖 + 长短按检测 + 页面/配置操作 |

### FreeRTOS 通信机制（6种）

| 机制 | 对象 | 使用场景 |
|------|------|---------|
| 任务通知（Task Notify） | CollectTask / KeyTask | 软件定时器回调通知CollectTask执行采集；EXTI中断通知KeyTask处理按键 |
| 队列（Queue） | LogQueue | CollectTask向队列推送历史数据，SerialTask请求时取出发送 |
| 二值信号量（Binary Sem） | SerialBinarySem / ADCBinarySem | 串口DMA空闲中断唤醒SerialTask；ADC DMA完成中断唤醒CollectTask |
| 互斥锁（Mutex） | myMutex01 | 保护Temp/Humi/light/Threshold/Collect_Period等共享变量，防止多任务读写冲突 |
| 事件组（Event Group） | AlarmEvent | AlarmTask等待 BIT_TEMP_OVER + BIT_AUTOALERM_ON 两个条件同时满足才触发报警 |
| 软件定时器（Timer） | CollectTimer | 周期定时触发采集，回调中发送任务通知 |

### 中断设计

| 中断 | 回调函数 | 动作 |
|------|---------|------|
| UART空闲中断 | HAL_UARTEx_RxEventCallback | DMA接收完成，释放SerialBinarySem唤醒SerialTask |
| GPIO外部中断 | HAL_GPIO_EXTI_Callback | 按键触发，发送任务通知唤醒KeyTask |
| ADC转换完成 | HAL_ADC_ConvCpltCallback | DMA搬运完成，释放ADCBinarySem唤醒CollectTask |

## 串口通信协议

### 帧格式

```
[0xAA] [LEN] [CMD] [DATA...] [XOR] [0x55]
```

| 字段 | 说明 |
|------|------|
| 0xAA | 帧头 |
| LEN | CMD + DATA + XOR 的总字节数 |
| CMD | 命令码 |
| DATA | 参数（0~N字节） |
| XOR | 异或校验（从CMD到DATA逐字节异或） |
| 0x55 | 帧尾 |

### 命令表

| CMD | 功能 | DATA参数 | 设备回复示例 |
|-----|------|---------|-------------|
| 0x01 | 请求当前数据 | 无 | `Temp:25℃ Light:3300mv Threshold:30℃ Interval:1s` |
| 0x02 | 设置采样间隔 | 1~60（秒） | `OK` / `ERR` |
| 0x03 | 设置报警阈值 | 1~127（℃） | `OK` / `ERR` |
| 0x04 | 控制LED | 0=关 / 1=开 / 2=闪烁 | — |
| 0x05 | 请求历史数据 | 无 | 10条历史记录 |
| 0x06 | 自动报警开关 | 1=开 / 0=关 | `AutoAlarm ON` / `AutoAlarm OFF` |

### 报警自动上报

当自动报警开启（CMD=0x06, DATA=1）且温度超过阈值时，设备自动发送：

```
[ALARM] Temp:35C Threshold:30C
```

温度恢复正常后发送：

```
[CLEAR] Temp:25C Threshold:30C
```

## 引脚分配

| 引脚 | 外设 | 用途 |
|------|------|------|
| PA9 / PA10 | USART1 | 串口通信（TX/RX） |
| PB6 / PB7 | I2C1 | OLED显示屏 |
| PA0 | ADC1_CH0 | 光敏传感器 |
| PB0 | GPIO Output | LED |
| PB1 | GPIO Output | LED（如需） |
| PA11 | GPIO Output | DHT11 数据线 |
| PA12 / PA15 | GPIO Input + EXTI | 按键1 / 按键2 |

> 引脚可能因CubeMX配置不同而变化，以 .ioc 文件为准。

## 开发环境

- **IDE**：Keil MDK-ARM 5
- **库**：STM32 HAL Library + CMSIS-RTOS2 (FreeRTOS)
- **配置工具**：STM32CubeMX
- **调试工具**：串口助手 / 逻辑分析仪

## 目录结构

```
├── Core/
│   ├── Inc/               # 头文件（main, usart, adc, i2c, tim, gpio, dma, freertos）
│   └── Src/               # 源文件
│       ├── main.c         # 主函数
│       ├── freertos.c     # FreeRTOS任务+IPC+协议解析（核心）
│       ├── usart.c        # UART初始化
│       ├── adc.c          # ADC初始化
│       ├── i2c.c          # I2C初始化
│       ├── tim.c          # 定时器初始化
│       ├── gpio.c         # GPIO初始化
│       └── dma.c          # DMA初始化
├── Hardware/              # 自定义驱动
│   ├── OLED.c / OLED.h    # OLED显示屏驱动
│   ├── DHT11.c / DHT11.h  # DHT11温湿度传感器驱动
│   └── Task.h             # 任务相关宏定义
├── 1 Environmental Monitoring Terminal.ioc  # CubeMX工程文件
└── README.md
```

## 使用方法

1. 用STM32CubeMX打开 `.ioc` 文件，重新生成工程（可选）
2. 用Keil MDK打开工程，编译下载
3. 串口助手连接（波特率115200，8N1）
4. 发送命令帧控制设备，例如：

```
设置采样间隔为5秒：AA 03 02 05 04 55
设置报警阈值为35℃：AA 03 03 23 20 55
请求当前数据：     AA 03 01 00 01 55
开启自动报警：     AA 03 06 01 07 55
```

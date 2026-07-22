# Smart-Bracelet 基于STM32的智能手环系统

## 简介

基于 **STM32F103C8T6** 微控制器和 **FreeRTOS** 实时操作系统的智能手环系统，集成心率监测、计步、计时、闹钟等功能，支持通过 ESP8266 WiFi 模块进行数据上报。

## 硬件平台

| 模块 | 型号/接口 | 说明 |
|------|-----------|------|
| MCU | STM32F103C8T6 | Cortex-M3, 72MHz, 64KB Flash, 20KB RAM |
| OLED显示屏 | SSD1306 128x64 | I2C1，显示界面交互 |
| 六轴传感器 | MPU6500 | I2C2，加速度计+陀螺仪，用于计步 |
| 心率传感器 | PulseSensor | ADC1 (PA1)，光电脉搏波检测 |
| WiFi模块 | ESP8266 | USART1，AT指令TCP通信，数据上报 |
| 蜂鸣器 | 有源蜂鸣器 | PB0 (TIM3_CH3 PWM)，闹钟提醒 |
| RTC | 内部RTC | 实时时钟，时间/日期/闹钟管理 |
| 按键 | 4个独立按键 | PA12(下) PA5(上) PA4(模式) PA15(电源) |
| 调试串口 | USART2 | 调试输出 |

## 软件架构

基于 **FreeRTOS (CMSIS-RTOS V2)** 多任务架构，通过页面切换实现多界面交互：

```
┌─────────────────────────────────────────────┐
│              FreeRTOS Scheduler              │
├──────────┬──────────────────────────────────┤
│ 优先级   │ 任务                              │
├──────────┼──────────────────────────────────┤
│ AboveNml │ keyTask        按键扫描与页面切换  │
│ Normal   │ mainMenuTask   主界面(时间显示)    │
│ Normal   │ timeSetTask    时间设置            │
│ Normal   │ alarmSetTask   闹钟设置            │
│ Normal   │ secondTask     秒表计时            │
│ Normal   │ alarmDispTask  闹钟提醒(蜂鸣+图片) │
│ Normal   │ heartTask      心率测量+波形显示   │
│ Normal   │ stepTask       计步数据展示        │
│ Low      │ walkTask       加速度采集(计步算法) │
│ Normal   │ infoMenuTask   系统信息(内存/任务)  │
└──────────┴──────────────────────────────────┘
```

**同步机制**：
- **Mutex** (`smaMutexHandle`)：OLED屏幕互斥访问
- **Mutex** (`UsartMutexHandle`)：串口通信互斥访问
- **Queue** (`queueKeyHandle`)：按键事件传递
- **Binary Semaphore** (`binarySemHandle`)：闹钟中断触发
- **Timer** (`tmrSingle/tmrCycleHandler`)：单次/周期定时器

## 页面功能

| 页面 | 名称 | 功能 | 操作方式 |
|------|------|------|----------|
| 0 | 空屏 | 关闭显示 | 按KEYPWR进入 |
| 1 | 主界面 | 实时时间显示 | 自动刷新 |
| 2 | 时间设置 | 设置年月日时分秒+星期 | 上/下键调整，模式键切换位置，确认键保存 |
| 3 | 闹钟设置 | 设置闹钟时间 | 上/下键调整，模式键切换位置，确认键保存 |
| 4 | 秒表 | 毫秒级计时 | 模式键启停，确认键归零 |
| 5 | 心率 | 心率测量+BPM显示+波形绘制 | 模式键启停 |
| 6 | 计步 | 步数/距离/卡路里显示 | 模式键启停WiFi上报 |
| 7 | 系统信息 | CPU型号/任务数/堆内存 | 上键发送任务列表，下键运行时统计，模式键动态内存管理 |

**按键说明**：
- **KEYDWN (PA12)**：数值减小 / 上翻
- **KEYUP (PA5)**：数值增大 / 下翻
- **KEYMODE (PA4)**：切换设置位置 / 启停功能
- **KEYPWR (PA15)**：页面切换 / 确认 / 返回

## 项目结构

```
Smart-Bracelet/
├── Core/
│   ├── Inc/
│   │   ├── main.h              # 主头文件，外设与模块统一引用
│   │   ├── oled.h              # OLED显示驱动
│   │   ├── oledfont.h          # 字模数据(ASC12/ASC16/HZ12/HZ16/BMP)
│   │   ├── mpu6500.h           # MPU6500六轴传感器驱动
│   │   ├── pulse_sensor.h      # 心率算法(基于阈值检测)
│   │   ├── appstep.h           # 计步算法(动态阈值峰值检测)
│   │   ├── appheart.h          # 心率应用层接口
│   │   ├── key.h               # 4键扫描驱动(消抖+长按)
│   │   ├── FreeRTOSConfig.h    # FreeRTOS配置
│   │   └── adc.h / i2c.h / rtc.h / tim.h / usart.h / gpio.h
│   └── Src/
│       ├── main.c              # 系统初始化，外设配置
│       ├── freertos.c          # 所有FreeRTOS任务实现(1224行)
│       ├── oled.c              # OLED驱动实现
│       ├── oledfont.c          # 字模数据
│       ├── mpu6500.c           # MPU6500驱动实现
│       ├── pulse_sensor.c      # 心率检测算法
│       ├── appstep.c           # 计步算法(动态阈值+波谷波峰检测)
│       ├── appheart.c          # 心率应用层
│       ├── key.c               # 按键扫描实现
│       ├── adc.c / i2c.c / rtc.c / tim.c / usart.c / gpio.c
│       └── stm32f1xx_hal_msp.c / stm32f1xx_it.c / system_stm32f1xx.c
├── Drivers/
│   ├── CMSIS/                  # ARM CMSIS库
│   └── STM32F1xx_HAL_Driver/   # STM32 HAL库
├── Middlewares/
│   └── Third_Party/            # FreeRTOS源码
├── MDK-ARM/
│   ├── Smart-Bracelet.uvprojx  # Keil工程文件
│   └── startup_stm32f103xb.s  # 启动文件
└── Smart-Bracelet.ioc          # STM32CubeMX工程配置
```

## 关键算法

### 心率检测

采用光电容积脉搏波 (PPG) 原理：
1. ADC以500Hz采样脉搏信号
2. 动态阈值检测心跳波峰
3. 计算IBI(心跳间隔)并滑动平均滤波
4. 输出BPM(次/分钟)，同步绘制128点波形

### 计步算法

基于加速度计动态阈值峰值检测：
1. 计算三轴加速度合量
2. 环形缓冲区动态调整阈值
3. 检测波峰波谷组合判定步伐
4. 根据默认步长(0.6m)和体重(65kg)计算距离/卡路里

## WiFi数据上报

通过USART1发送AT指令控制ESP8266，TCP Server模式：
- 周期上报计步数据：步数、距离、卡路里
- 按需发送FreeRTOS任务列表和运行时统计
- 命令格式：`AT+CIPSEND=0,<长度>\r\n` + 数据内容

## 构建环境

- **IDE**: Keil MDK-ARM v5
- **芯片包**: STM32F1xx DFP
- **代码生成**: STM32CubeMX
- **RTOS**: FreeRTOS (CMSIS-RTOS V2)

## 编译与烧录

1. 用 Keil MDK-ARM 打开 `MDK-ARM/Smart-Bracelet.uvprojx`
2. 编译工程 (Build)
3. 通过 ST-Link 连接开发板
4. Download 烧录固件

## 注意事项

- 心率传感器需正确连接至ADC1通道(PA1)
- MPU6500通过I2C2通信，确认AD0引脚电平选择正确地址(0x68/0x69)
- ESP8266模块通过USART1连接，波特率需匹配
- FreeRTOS堆空间配置为10KB (`configTOTAL_HEAP_SIZE`)
<img width="291" height="170" alt="image" src="https://github.com/user-attachments/assets/edbc46bb-d0e5-4be2-8f9d-6e9cf13cd6dd" /><img width="291" height="170" alt="image" src="https://github.com/user-attachments/assets/69683270-4ecd-4e26-b160-dc9be93e1181" />

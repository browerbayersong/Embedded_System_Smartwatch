# 嵌入式系统智能手表 - STM32 固件 + Android BLE 上位机

> 基于 **STM32F103C8T6** 的多功能智能手表固件 + **Android** 蓝牙上位机应用。
> 手表端集成 OLED 显示、6 轴运动传感器、HC-05 蓝牙通信与计步算法；手机端提供设备连接、数据采集与时间同步。

---

## 🧭 项目导航

| 模块 | 说明 | 位置 |
|------|------|------|
| **⭐ 主固件** | Phase 0-5 功能整合版（推荐烧录此版本） | `Phase3_Bluetooth/` |
| Phase 0 | HC-05 蓝牙模块 AT 配置工具（首次使用需配置蓝牙参数） | `Phase0_HC05Setting/` |
| Phase 1 | OLED 显示驱动（SSD1306 128×64，I2C1） | `Phase1_OLED/` |
| Phase 2 | MPU6050 6 轴传感器（加速度 + 陀螺仪 + 姿态角，I2C2） | `Phase2_MPU6050/` |
| Phase 3 | 蓝牙通信（USART2 + DMA，115200 baud，HC-05 STATE 引脚检测） | `Phase3_Bluetooth/` |
| **Android APP** | 手机端 BLE 上位机（扫描连接、数据显示、时间同步） | `Android_app/` |

> **提示**：Phase 1/2 的功能代码已完整合并到 `Phase3_Bluetooth/` 中，只需烧录此一份固件即可使用全部功能。

---

## 🔧 硬件清单

| 器件 | 型号 | 接口 | 地址 |
|------|------|------|------|
| 主控 | STM32F103C8T6 | - | - |
| OLED 显示屏 | SSD1306 128×64（黄蓝双色） | **I2C1** | `0x3C` (7-bit) / `0x78` (写) / `0x79` (读) |
| 运动传感器 | MPU6050（3 轴加速度 + 3 轴陀螺仪） | **I2C2** | `0x68` (7-bit) / `0xD0` (写) / `0xD1` (读) |
| 蓝牙模块 | HC-05（经典蓝牙 SPP） | **USART2 + PB0（STATE）** | - |
| I2C 引脚 | OLED: PB6(SCL)/PB7(SDA) · MPU6050: PB10(SCL)/PB11(SDA) | - | - |
| USART2 引脚 | PA2(TX) · PA3(RX) | - | - |

---

## ✨ 固件功能特性

### OLED 显示（Phase 1）
- 16×24 大字时钟显示（表盘页面核心视觉）
- 6×8 / 8×16 双字体系统
- 像素级绘图（画点、线、圆、矩形、位图）
- 5 页菜单自动切换（每 3 秒翻一页）
  1. **表盘** — 大字时钟 + 日期 + 快速姿态角
  2. **传感器** — 加速度 (m/s²) + 陀螺仪 (deg/s) + 温度 (°C)
  3. **蓝牙** — 连接状态、波特率、硬件引脚
  4. **设备信息** — MCU 型号、I2C 总线分配
  5. **活动** — 步数统计 + 距离(米) + 消耗卡路里

### MPU6050（Phase 2）
- 加速度量程 ±2g（16384 LSB/g）→ 物理单位 m/s²
- 陀螺仪量程 ±250°/s（131 LSB/°/s）→ 物理单位 deg/s
- 姿态角计算（Pitch/Roll）
- 温度读取（°C）
- I2C 通信错误检测与自动重连（每 3 秒重试一次）

### HC-05 蓝牙（Phase 3）
- **115200 baud** DMA 传输（接收用环形缓冲 + DMA Idle 中断）
- 帧协议：`[0xAA][CMD][DATA...][CHK][0x55]`（XOR 校验）
- 支持命令：`0x01` 传感器数据上报 / `0x02` 时间同步 / `0x03` 计步数据
- HC-05 STATE 引脚硬件检测连接状态（非软件猜测）

### 软 RTC（从 SmartWatch2 合并）
- TIM2 1Hz 中断递增时/分/秒
- 蓝牙接收到时间同步命令后自动更新系统时间

### 计步器（Phase 5，从 SmartWatch2 合并）
- 加速度向量幅值 + 低通滤波 + 动态阈值
- 200 ms 最小步间间隔（防抖）
- 步数 + 距离（米） + 卡路里估算

### I2C 总线恢复（从 SmartWatch2 合并）
- 启动时通过 SCL/SDA 模拟时钟脉冲解除 I2C 从机死锁
- 在 OLED 与 MPU6050 初始化前执行

---

## 📱 Android 上位机功能

| 功能 | 说明 |
|------|------|
| **BLE 设备扫描** | 搜索附近蓝牙低功耗设备，显示名称与 MAC 地址 |
| **设备连接管理** | 一键连接/断开，实时显示连接状态 |
| **传感器数据** | 解析 6 轴（加速度 + 陀螺仪）数据，动态更新 |
| **时间同步** | 向手表发送当前时间，保持设备时钟一致 |
| **日志系统** | 记录所有通信过程，支持查看与导出 |
| **主题切换** | 深色 / 浅色主题，Material Design 3 |

APK 下载：`Android_app/app/release/app-release.apk`（已签名，安装即用）

---

## 📂 项目结构

```
Embedded_System_Smartwatch/
├── Phase0_HC05Setting/          # HC-05 蓝牙模块配置（首次使用）
│   ├── Core/Src/hc05_at.c       # AT 指令发送/解析固件
│   └── hc05_at_host/            # PC 端 Web 配置工具（Python Flask）
│
├── Phase1_OLED/                 # OLED 显示驱动（功能已合并到 Phase3）
├── Phase2_MPU6050/              # MPU6050 传感器（功能已合并到 Phase3）
│
├── Phase3_Bluetooth/            # ⭐ 主项目（整合 Phase1-5 全部功能）
│   ├── Core/
│   │   ├── Inc/
│   │   │   ├── oled.h           # OLED 驱动 API（16x24 大字、绘图）
│   │   │   ├── mpu6050.h        # MPU6050 传感器 API
│   │   │   ├── bluetooth.h      # HC-05 蓝牙协议 API
│   │   │   ├── smartwatch_ui.h  # 5 页面 UI 框架
│   │   │   ├── pedometer.h      # 计步器算法 API
│   │   │   ├── soft_rtc.h       # 软 RTC（TIM2 1Hz）
│   │   │   └── i2c_drv.h        # I2C 总线恢复工具
│   │   └── Src/
│   │       ├── main.c           # 主循环（传感器+UI+蓝牙+RTC+计步）
│   │       ├── oled.c           # SSD1306 完整驱动
│   │       ├── mpu6050.c        # 6 轴传感器数据读取
│   │       ├── bluetooth.c      # 帧协议 + DMA 环形缓冲
│   │       ├── smartwatch_ui.c  # 5 页面渲染
│   │       ├── pedometer.c      # 步数/距离/卡路里算法
│   │       ├── soft_rtc.c       # 时/分/秒变量
│   │       ├── i2c_drv.c        # I2C 总线恢复
│   │       └── stm32f1xx_it.c   # TIM2 1Hz + USART2 DMA 中断
│   └── Drivers/                 # STM32 HAL 库
│
└── Android_app/                 # Android BLE 上位机
    ├── app/src/main/java/com/example/smartwatch/
    │   ├── MainActivity.kt      # 主界面（主页/连接/设置 Tab）
    │   ├── BleManager.kt        # BLE 扫描/连接/读写管理
    │   ├── BtProtocol.kt        # 与手表通信的帧协议解析
    │   └── ui/theme/            # Material Design 3 主题
    └── app/release/app-release.apk  ← 直接安装即可使用
```

---

## 🔌 通信协议

**手表端 ↔ 手机端** 约定的帧格式：

```
┌──────┬─────┬─────────────────┬──────┬──────┐
│ STX  │ CMD │      DATA       │ CHK  │ ETX  │
│ 0xAA │     │                 │ XOR  │ 0x55 │
└──────┴─────┴─────────────────┴──────┴──────┘
```

| 字段 | 字节数 | 说明 |
|------|--------|------|
| STX | 1 | 帧头固定 `0xAA` |
| CMD | 1 | `0x01`=传感器数据；`0x02`=时间同步；`0x03`=计步数据 |
| DATA | N | 载荷（传感器为 6 × 4 字节小端 float） |
| CHK | 1 | CMD + DATA 全部字节做 XOR 校验 |
| ETX | 1 | 帧尾固定 `0x55` |

> 协议实现：
> - 手表端：`Phase3_Bluetooth/Core/Src/bluetooth.c`
> - 手机端：`Android_app/app/src/main/java/com/example/smartwatch/BtProtocol.kt`

---

## 🚀 快速上手

### 1️⃣ 配置 HC-05（首次使用）

HC-05 默认波特率为 38400，需改为 **115200** 以匹配固件。

使用 `Phase0_HC05Setting/`：
- 方式 A：烧录固件 → USB 串口接 PC → 串口终端发送 `AT+UART=115200,0,0`
- 方式 B：运行 `hc05_at_host/hc05_at_host.py` → 浏览器打开 Web 界面 → 图形化配置波特率、名称、密码

### 2️⃣ 烧录主固件

1. 在 Keil MDK 或 STM32CubeIDE 中打开 `Phase3_Bluetooth/`
2. 编译 → 通过 ST-Link 或 USB-TTL 下载到 STM32F103C8T6
3. OLED 依次显示 5 个页面（每 3 秒切换）即为正常启动

### 3️⃣ 安装 Android APP

1. 手机安装 `Android_app/app/release/app-release.apk`
2. 授予**蓝牙**和**定位**权限
3. 扫描连接名为 `HC-05` 的设备（或你自定义的名称）
4. 主页面实时显示手表端的传感器数据与计步信息

---

## 📋 使用流程

| 步骤 | 操作 |
|------|------|
| 1 | 上电 → OLED 显示启动页面 → I2C1/OLED、I2C2/MPU6050 依次初始化 |
| 2 | 手表自动循环 5 个页面（表盘→传感器→蓝牙→设备信息→活动） |
| 3 | 打开手机 APP → 扫描 → 连接 HC-05 |
| 4 | 手机端可查看传感器数据、同步时间、导出日志 |
| 5 | 手表检测到运动自动更新步数与距离 |

---

## ⚠️ 注意事项

- HC-05 蓝牙波特率必须设为 **115200**（固件默认值），否则蓝牙通信乱码
- 首次安装 Android APP 必须授予「蓝牙扫描」「蓝牙连接」「精确定位」三项权限
- 建议保持手机与手表距离在 5 米以内以获得稳定通信
- MPU6050 WHO_AM_I 检测失败时会每 3 秒自动重试初始化
- 上电时 I2C 总线会先执行恢复序列，避免从机处于死锁状态

---

## 📄 许可证

本项目用于学习与课程实践，请在遵守相关法律法规的前提下使用。
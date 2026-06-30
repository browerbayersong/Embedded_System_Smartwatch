# Phase4 对接说明给陈斯焕

## 贡献人：宋柏粤

我已完成的部分：

1. `Phase4_FreeRTOS/Core/Inc/menu.h`
2. `Phase4_FreeRTOS/Core/Src/menu.c`
3. `Phase5_Advanced/Core/Inc/pedometer.h`
4. `Phase5_Advanced/Core/Src/pedometer.c`
5. `Phase3_Bluetooth/Core/Inc/smartwatch_ui.h` 数据结构扩展

---

## 我完成的功能

### 菜单交互

- 支持 5 个屏幕：
  - `Watch Face`
  - `Sensor`
  - `Activity`
  - `Settings`
  - `Device`
- 旋转编码器左右切换页面
- 短按进入/确认
- 长按返回主表盘
- `Settings` 页面支持亮度值调节
- `Activity` 页面显示步数、距离、卡路里

### 计步算法

- 实现步数检测逻辑
- IIR 低通滤波
- 动态阈值更新
- 峰值检测 + 最小步距 200ms
- 计算距离与卡路里
- 提供 reset 接口

### 数据对接

已扩展 `SmartWatchData_t`：
- `step_count`
- `distance_m`
- `calories`

这样 `watch_data` 可以直接传给 UI、蓝牙和显示逻辑。

---

## 你在 Phase4 需要做的对接点

### 1. FreeRTOS 任务框架

建议任务划分：
- `taskSensor`
- `taskMenu`
- `taskDisplay`
- `taskBluetooth`
- `taskTimeKeep`

### 2. 编码器输入

你需要把硬件读数传给菜单模块：
- `TIM3` 编码器模式
- `PA6` / `PA7`
- 读取增量：`int16_t delta = (int16_t)TIM3->CNT; TIM3->CNT = 0;`
- 传给：`Menu_ProcessRotation(&menu_context, delta);`

### 3. 按键输入

- `PB10` EXTI 外部中断
- 短按：`Menu_ProcessButton(&menu_context, false);`
- 长按：`Menu_ProcessButton(&menu_context, true);`

### 4. I2C 互斥

`OLED` 和 `MPU6050` 共用 I2C，必须用互斥锁保护。建议：
- `mutexI2C`
- `xSemaphoreTake(mutexI2C, pdMS_TO_TICKS(100))`
- `xSemaphoreGive(mutexI2C)`

### 5. 数据更新

`taskSensor` 需要：
- 读取 MPU6050 数据
- 更新 `watch_data.accel`/`gyro`/`angle`
- 调用 `Pedometer_Update()`
- 把步数数据写回 `watch_data`

---

## 我的接口建议

### 菜单接口

```c
MenuContext_t menu_context;
Menu_Init(&menu_context);
Menu_ProcessRotation(&menu_context, delta);
Menu_ProcessButton(&menu_context, long_press_flag);
Menu_Render(&watch_data, &menu_context);
```

### 计步接口

```c
PedometerState_t pedometer_state;
Pedometer_Init(&pedometer_state);
Pedometer_Update(&pedometer_state, &watch_data.accel, xTaskGetTickCount() * portTICK_PERIOD_MS);
watch_data.step_count = Pedometer_GetStepCount(&pedometer_state);
watch_data.distance_m = Pedometer_GetDistanceMeters(&pedometer_state);
watch_data.calories = Pedometer_GetCalories(&pedometer_state);
```

---

## 你可以直接调用我的内容

- `Menu_Init()`
- `Menu_ProcessRotation()`
- `Menu_ProcessButton()`
- `Menu_Render()`
- `Pedometer_Init()`
- `Pedometer_Update()`
- `Pedometer_GetStepCount()`
- `Pedometer_GetDistanceMeters()`
- `Pedometer_GetCalories()`

---

## 我希望你实现的内容

- `Phase4_FreeRTOS` 的 RTOS 工程
- `taskSensor` 读取 MPU6050 并写回 `watch_data`
- `taskMenu` 读取编码器输入并调用菜单接口
- `taskDisplay` 负责刷新 OLED
- `taskBluetooth` 继续蓝牙帧处理
- I2C 互斥

---

## 说明总结

我负责：
- 交互/菜单逻辑
- 计步算法
- UI 数据扩展
- Activity 页面显示

你负责：
- RTOS 任务
- 编码器与按键硬件对接
- I2C 互斥与任务调度
- OLED 刷新集成

如果你愿意，我可以继续把 `Phase4_FreeRTOS` 做成完整的 `main.c` 伪实现，让你直接接入。
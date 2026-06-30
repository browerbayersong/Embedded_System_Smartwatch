# 嵌入式系统智能手表 - Android BLE 上位机

> 基于 Android + Bluetooth Low Energy 的智能手表上位机应用，用于与搭载传感器的手表端进行蓝牙通信、数据采集与时间同步。

---

## 📱 功能特性

| 功能 | 说明 |
|------|------|
| **BLE 设备扫描** | 自动搜索附近的蓝牙低功耗设备并显示名称与 MAC 地址 |
| **设备连接管理** | 一键连接/断开，实时显示连接状态 |
| **传感器数据接收** | 解析手表端发送的 6 轴（加速度 + 陀螺仪）传感器数据 |
| **时间同步** | 向手表发送当前时间指令，保持设备时钟一致 |
| **日志系统** | 自动记录所有通信过程，支持查看与导出 |
| **深色/浅色主题** | 可切换界面配色，适应不同使用环境 |

---

## ⬇️ 下载 APK

### 最新版本

| 版本 | 类型 | 下载 | 说明 |
|------|------|------|------|
| v1.0 | Release | [点击下载](https://github.com/browerbayersong/Embedded_System_Smartwatch/releases/latest/download/app-release.apk) | 推荐安装，体积更小 |
| v1.0 | Debug | [点击下载](https://github.com/browerbayersong/Embedded_System_Smartwatch/releases/latest/download/app-debug.apk) | 调试版本，包含详细日志 |

> **提示**：如果 Releases 中还没有文件，请先参考下方「构建 APK」的说明自行编译，或者等待作者在 GitHub Releases 页面发布。

### 所有历史版本

👉 访问 [GitHub Releases 页面](https://github.com/browerbayersong/Embedded_System_Smartwatch/releases)

### 安装步骤

1. **下载** `.apk` 文件到手机（浏览器/微信/QQ 传文件都可以）
2. 在手机文件管理器中**点击该 APK 文件**
3. 如果提示「未知来源」，允许安装即可（系统设置 → 允许此应用安装未知来源）
4. 安装完成后，打开应用，授予**蓝牙**和**定位**权限
5. 点击「扫描设备」，找到手表后点击「连接」

---

## 🏗️ 项目结构

```
Embedded_System_Smartwatch/
├── Android_app/                      # Android 上位机工程（主项目）
│   ├── app/
│   │   ├── src/main/java/
│   │   │   └── com/example/smartwatch/
│   │   │       ├── MainActivity.kt   # 主界面（主页/连接/设置 三个 Tab）
│   │   │       ├── BleManager.kt     # BLE 扫描、连接、读写管理
│   │   │       ├── BtProtocol.kt     # 与手表通信的协议解析与打包
│   │   │       └── ui/theme/         # Compose 主题 (深色/浅色/颜色切换)
│   │   ├── build.gradle.kts          # 应用模块构建配置（含签名）
│   │   └── build/outputs/apk/        # 构建后 APK 输出目录
│   ├── build.gradle.kts              # 项目根构建配置
│   ├── settings.gradle.kts           # Gradle 项目设置
│   ├── gradle/libs.versions.toml     # 依赖版本管理
│   ├── gradle.properties              # Gradle 参数
│   └── gradlew / gradlew.bat         # Gradle Wrapper（Win/Mac/Linux）
├── .gitignore                        # Git 忽略规则
└── README.md                         # 本文件
```

---

## 🔌 通信协议

本应用与手表端约定的帧格式：

```
┌──────┬─────┬──────┬─────────────────┬──────┬──────┐
│ STX  │ CMD │ LEN  │      DATA       │ CHK  │ ETX  │
│ 0x02 │     │      │                 │ XOR  │ 0x03 │
└──────┴─────┴──────┴─────────────────┴──────┴──────┘
```

| 字段 | 字节数 | 说明 |
|------|--------|------|
| STX | 1 | 帧头固定 `0x02` |
| CMD | 1 | `0x01` = 传感器数据；`0x02` = 时间同步；`0x03` = 时间同步响应 |
| LEN | 1 | DATA 字段字节数 |
| DATA | N | 载荷（传感器为 6 × 4 字节小端 float） |
| CHK | 1 | CMD + LEN + DATA 全部字节做 XOR 校验 |
| ETX | 1 | 帧尾固定 `0x03` |

协议实现在 `Android_app/app/src/main/java/com/example/smartwatch/BtProtocol.kt` 中。

---

## 🛠️ 构建 APK（开发者）

### 环境要求

| 项目 | 版本 |
|------|------|
| Android Studio | 2024.x 或更高 |
| Gradle | 9.4.1（项目自带 Wrapper） |
| Android Gradle Plugin | 9.2.1 |
| Kotlin | 2.3.0（通过 Compose Plugin） |
| JDK | 17 或更高（Android Studio 自带即可） |
| Min SDK | 34 (Android 14) |
| Target SDK | 36 (Android 14+) |

### 方式一：在 Android Studio 中构建

1. 用 Android Studio 打开 `Android_app/` 目录
2. 等待 Gradle Sync 完成
3. 顶部菜单 → **Build** → **Build Bundle(s) / APK(s)** → **Build APK(s)**
4. 构建成功后在 `Android_app/app/build/outputs/apk/` 目录下找到 `app-debug.apk`

**打包 Release 版（有签名）**：
1. 顶部菜单 → **Build** → **Generate Signed Bundle / APK**
2. 选择 **APK** → 新建或选择 keystore
3. 选择 **release** → Finish
4. 输出在 `Android_app/app/release/app-release.apk`

### 方式二：命令行构建

```bash
cd Android_app

# Windows (PowerShell / CMD)
gradlew.bat assembleDebug     # Debug 版
gradlew.bat assembleRelease   # Release 版

# macOS / Linux
./gradlew assembleDebug
./gradlew assembleRelease
```

APK 输出路径：
- Debug: `Android_app/app/build/outputs/apk/debug/app-debug.apk`
- Release: `Android_app/app/build/outputs/apk/release/app-release.apk`

---

## 🚀 发布到 GitHub Releases

1. 在 GitHub 仓库页面点击 **Releases** → **Draft a new release**
2. Tag version: `v1.0`，Release title: `v1.0 - 首个正式版`
3. 把 `app-release.apk` 和 `app-debug.apk` **拖到文件上传区**
4. 勾选 **Set as the latest release** → **Publish release**
5. 发布完成后，上方 README 中的下载链接会自动生效

---

## 📋 使用流程

1. 打开 APP → 授予蓝牙与定位权限
2. 切换到「**连接**」Tab → 点击「**扫描设备**」
3. 列表中找到手表 → 点击「**连接**」
4. 连接成功后回到「**主页**」Tab：
   - 查看传感器数据（ax/ay/az, gx/gy/gz）
   - 点击「**同步时间到设备**」
5. 「**设置**」Tab 可切换主题/查看日志/导出日志文件

---

## ⚠️ 注意事项

- 手表端固件必须使用**相同的帧协议**（STX=0x02, ETX=0x03, XOR 校验, 小端 float）
- 首次安装必须授予「蓝牙扫描」「蓝牙连接」「精确定位」三项权限
- 手机蓝牙需开启，且位置服务处于开启状态
- 建议保持与设备距离在 5 米以内，以获得稳定的通信效果

---

## 📄 许可证

本项目用于学习与课程实践，请在遵守相关法律法规的前提下使用。
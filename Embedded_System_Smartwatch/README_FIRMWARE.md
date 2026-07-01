# 智能手表固件统一构建说明

## 1. 统一入口

现在项目根目录下已经添加了一个总 CMake 工程，入口为：

- [CMakeLists.txt](CMakeLists.txt)
- [src/main.c](src/main.c)

你可以通过 `SMARTWATCH_PHASE` 选择要编译的 Phase：

- `0` = Phase0_HC05Setting
- `1` = Phase1_OLED
- `2` = Phase2_MPU6050
- `3` = Phase3_Bluetooth

## 2. 构建命令

在项目根目录执行：

```bash
cmake -S . -B build -DSMARTWATCH_PHASE=3
cmake --build build -j
```

这会生成：

- `build/smartwatch_firmware.elf`
- `build/smartwatch_firmware.hex`
- `build/smartwatch_firmware.bin`

## 3. 用 ST-Link 下载到 STM32

如果你本机已经安装了 STM32CubeProgrammer 或 ST-Link 驱动，可以执行：

```bash
STM32_Programmer_CLI -c port=SWD -d build/smartwatch_firmware.bin 0x08000000 -v -rst
```

如果你的环境里没有 `STM32_Programmer_CLI`，可以改用 CubeProgrammer 的图形界面进行烧录。

## 4. 说明

- 这样做之后，不需要每个 Phase 单独维护一套独立的工程入口。
- 你仍然可以保留原来的 Phase 目录，后续继续在各自代码里开发。
- 如果之后要把 Phase4 FreeRTOS 也接进来，只需要再增加一个 `SMARTWATCH_PHASE=4` 分支即可。

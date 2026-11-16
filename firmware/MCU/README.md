# MCU 固件构建说明

## 目录

| 工程 | 说明 |
|------|------|
| `Application/` | 主固件：SCPI 解析 + USB CDC + FreeRTOS |
| `SCPI_Test/` | SCPI 解析移植验证工程 |
| `MCU_Test_Hardware/min_system_test/` | 最小系统测试 |
| `MCU_Test_Hardware/analog_hardware_test/` | 模拟板硬件测试 |
| `Clock_Generator_Config/MCU_Project/` | CDCI6214 时钟发生器配置 |

## 构建前必须补齐：STM32 USB Device Library

本仓库**不包含** `Middlewares/ST/STM32_USB_Device_Library`。

原因：该库适用 ST 的 **SLA0044（Ultimate Liberty）**，其中

- 第 4 条：只能与 ST 制造的 MCU/MPU 配合使用；
- 第 5 条：不得以任何方式使其受任何开源许可（GPL、EPL、Apache、BSD、MIT 等）约束。

本项目固件以 **GPL-3.0-only** 发布，二者不兼容。若把该库随仓库分发，会同时违反 SLA0044 与 GPL-3.0。因此它由使用者自行获取，并已加入 `.gitignore`。

### 获取方式

在本仓库根目录执行：

```powershell
# 方式 A：本机已有 STM32CubeF4 包
.\scripts\fetch_st_usb.ps1 -CubeRoot C:\STM32Cube\STM32Cube_FW_F4_V1.28.0

# 方式 B：从 ST 官方 GitHub 稀疏克隆
.\scripts\fetch_st_usb.ps1 -Download
```

脚本会把库复制到：

- `firmware/MCU/Application/Middlewares/ST/`
- `firmware/MCU/SCPI_Test/Middlewares/ST/`

`ArbWave30.uvprojx` 与 `SCPI_Test.uvprojx` 的包含路径已按上述位置配置，补齐后可直接用 Keil 打开。

### 后续工作

计划迁移到 **TinyUSB**（MIT），以消除上述许可冲突，并摆脱对 Keil 的依赖（配合迁移到 GCC 工具链）。迁移完成后，本文件与 `scripts/fetch_st_usb.ps1` 一并删除。

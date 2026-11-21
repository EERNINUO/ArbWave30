<!-- Assisted-by: DeepSeek - 文档框架 -->

# Third-Party Notices

本仓库包含部分第三方组件。它们不属于本项目许可证范围，各自适用其原始许可证；许可证原文见各组件目录下的 `LICENSE` 文件。

## 随仓库分发

| 组件 | 位置 | 许可证 |
|------|------|--------|
| STM32F4xx HAL Driver | `firmware/MCU/*/Drivers/STM32F4xx_HAL_Driver/` | BSD-3-Clause |
| CMSIS Core / Device | `firmware/MCU/*/Drivers/CMSIS/` | Apache-2.0 |
| FreeRTOS Kernel | `firmware/MCU/*/Middlewares/Third_Party/FreeRTOS/` | MIT |
| scpi-parser | `firmware/MCU/*/Middlewares/Third_Party/scpi-parser/` | BSD-2-Clause |

> `firmware/MCU/*/Drivers/CMSIS/DSP` 与 `CMSIS/NN` 当前未参与编译，后续清理。

## 不随仓库分发

| 内容 | 权利人 | 原因 | 获取方式 |
|------|--------|------|----------|
| Gowin DDS / rPLL / OSC / FIFO IP 生成物 | 高云半导体 | “All rights reserved”，未授予再分发权 | 用仓库内 `.ipc` 在 Gowin IP Generator 中重新生成，见 `firmware/FPGA/README.md` |
| STM32 USB Device Library | STMicroelectronics | SLA0044 第 4/5 条，与 GPL-3.0 不兼容 | 运行 `scripts/fetch_st_usb.ps1`，见 `firmware/MCU/README.md` |

## 说明

- 第三方组件的版权与许可归各自权利人所有，本项目仅按原许可条款使用。
- 本项目不因包含上述组件而对其主张任何权利。

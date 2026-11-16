# 许可证（Licensing）

本项目按目录分层授权，不同部分适用不同许可证。根目录 `LICENSE` 为固件的 GPL-3.0 全文；**当子目录下另有 `LICENSE` 时，以子目录为准**。

## 分层

| 路径 | 内容 | 许可证 | SPDX |
|------|------|--------|------|
| `firmware/` | MCU 固件、FPGA 逻辑、测试脚本 | GNU GPL v3.0 only | `GPL-3.0-only` |
| `hardware/` | 原理图、PCB、BOM、封装库 | CERN Open Hardware Licence v2 – Strongly Reciprocal | `CERN-OHL-S-2.0` |
| `docs/` | 设计文档、笔记、用户手册、图片 | CC BY-SA 4.0 | `CC-BY-SA-4.0` |
| `scripts/` | 自动化测试脚本 | GNU GPL v3.0 only | `GPL-3.0-only` |

许可证全文见根目录 `LICENSE`、`hardware/LICENSE`、`docs/LICENSE`。

## 第三方内容

本仓库包含部分第三方组件（ST HAL、CMSIS、FreeRTOS、scpi-parser 等），它们**不属于**上述许可证范围，各自适用原始许可证，清单与出处见 [NOTICE.md](NOTICE.md)。

以下第三方内容**不随本仓库分发**，需自行从原厂获取：

- 器件数据手册、应用笔记、IBIS 模型、评估板资料（版权归 ADI / TI / IVI Foundation）；
- 高云 Gowin IP 生成物（DDS / rPLL / OSC / FIFO，版权归高云半导体），仓库仅保留 `.ipc` 配置，用 Gowin IP Generator 重新生成，见 [firmware/FPGA/README.md](firmware/FPGA/README.md)；
- ST STM32 USB Device Library（SLA0044，与 GPL-3.0 不兼容），用 `scripts/fetch_st_usb.ps1` 获取，见 [firmware/MCU/README.md](firmware/MCU/README.md)。

## 商标

`ArbWave30` 名称与标志未授予任何许可。基于本设计制作的衍生产品可以说明 “based on ArbWave30”，但不得使用 `ArbWave30` 作为产品名或商标。

## 商业授权

如需将本项目的固件或硬件设计用于闭源商业产品，或需要保修、支持、定制，可联系作者讨论商业授权。

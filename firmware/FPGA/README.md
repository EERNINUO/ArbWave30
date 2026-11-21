<!-- Assisted-by: DeepSeek - 文档框架 -->

# FPGA 工程构建说明

## 工具链

- Gowin EDA V1.9.12.01（或兼容版本）
- 器件：GW2AR-LV18EQ144C8/I7（GW2AR-18，Device Version C）
- 仿真：ModelSim（`sim/` 下为 testbench，仿真缓存不入库）

## 目录

| 工程 | 说明 |
|------|------|
| `Application/` | 主工程：DDS 波形生成 + SPI 从机 + DAC 接口 |
| `FPGA_Test_Hardware/FPGA_Clock_Test/` | 时钟 / PLL 测试 |
| `FPGA_Test_Hardware/FPGA_Hardware_Test/` | 硬件测试（内部 OSC） |

## 构建前必须重新生成 IP

本仓库**不包含**高云 IP 的生成物（`dds_ii.v`、`fifo_hs.v`、`gowin_rpll.v`、`gowin_osc.v` 及 `temp/`）。

原因：这些文件由 Gowin IP Generator 生成，版权归高云半导体（“All rights reserved”），未授予再分发权。仓库只保留 `.ipc` 配置文件，它是唯一的重新生成依据：

| IP | 配置文件 | 生成到 |
|----|----------|--------|
| DDS II | `Application/src/wave_generation/dds_ii/dds_ii.ipc` | 同目录 |
| FIFO HS | `Application/src/dac_interface/fifo_hs/fifo_hs.ipc` | 同目录 |
| rPLL | `Application/src/sys_ctrl/gowin_rpll/gowin_rpll.ipc` | 同目录 |
| rPLL（时钟测试） | `FPGA_Test_Hardware/FPGA_Clock_Test/src/gowin_rpll/gowin_rpll.ipc` | 同目录 |
| OSC（硬件测试） | `FPGA_Test_Hardware/FPGA_Hardware_Test/src/gowin_osc/gowin_osc.ipc` | 同目录 |

### 生成步骤

1. 打开 Gowin EDA，进入 **Tools → IP Core Generator**；
2. 选择 **Open**，打开上表对应的 `.ipc` 文件；
3. 确认配置后点击 **Generate**，输出目录保持为同目录（生成物会写入 `temp/`）；
4. 回到工程，`Application/ArbWave30.gprj` 中引用的 `src/.../xxx.v` 即可被解析。

`temp/`、IP 生成的 `*.v`、`*.mod` 均已加入 `.gitignore`，不要提交。

## 约束文件

- 物理约束：`Application/src/ArbWave30.cst`
- 时序约束：`Application/src/ArbWave30.sdc`

## 烧录

综合 → 布局布线 → 生成比特流，通过 JTAG 烧录到外部 SPI Flash。用户侧操作步骤见 `docs/用户手册/`（待补充）。

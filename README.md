<div align="center">

<!-- Assisted-by: DeepSeek - 文档框架 -->

# ArbWave30
30MHz 任意波形发生器 | FPGA + 高速DAC | SCPI 可控 

<p align="center">
  <a href="https://github.com/EERNINUO/ArbWave30" style="margin: 2px;">
    <img alt="仓库状态" src="https://img.shields.io/badge/status-MVP 设计中-blue">
  </a>
  <a href="https://github.com/EERNINUO/ArbWave30" style="margin: 2px;">
    <img alt="GitHub 仓库星标" src="https://img.shields.io/github/stars/EERNINUO/ArbWave30">
  </a>
  <br/>
  <a href="https://raw.githubusercontent.com/EERNINUO/ArbWave30/main/LICENSE" style="margin: 2px;">
    <img src="https://img.shields.io/badge/License-GPL--3.0-red" alt="许可证">
  </a>
  <a href="https://raw.githubusercontent.com/EERNINUO/ArbWave30/main/LICENSE" style="margin: 2px;">
    <img src="https://img.shields.io/badge/License-CC--BY--SA--4.0-green" alt="许可证">
  </a>
    <a href="https://raw.githubusercontent.com/EERNINUO/ArbWave30/main/LICENSE" style="margin: 2px;">
    <img src="https://img.shields.io/badge/License-CERN--OHL--S--2.0-blue" alt="许可证">
  </a>
  <!-- <a href="https://github.com/EERNINUO/ArbWave30/releases" style="margin: 2px;">
    <img src="https://img.shields.io/github/v/release/EERNINUO/ArbWave30?color=blueviolet&include_prereleases" alt="发布版本">
  </a> -->
</p>
</div>

---

> **⚠️ 注意**：你当前查看的是 **develop** 开发分支，内容可能不稳定。
> 推荐使用稳定发布版本：[查看 Releases](https://github.com/EERNINUO/ArbWave30/tree/releases)


## ✨ 特性

### 硬件指标（V1.0）
- 波形：正弦、方波、三角波、锯齿波、阶梯波、直流
- 正弦波最高 **30MHz**，方波最高 **5MHz**
- 幅值范围（高阻）：20Vpp（DC\~10MHz），14Vpp（10\~20MHz），10Vpp（20\~30MHz）
- 幅值分辨率：2mV（典型），偏移范围 ±5V
- 总谐波失真（THD）：< -50dBc @10MHz 满幅
- 相位噪声：-100dBc/Hz @10kHz offset（30MHz 载波）(目标值)
- 输出阻抗：50Ω
- 参考时钟：内部 TCXO（±1ppm）或外部 10MHz 输入

### 控制与接口
- USB 2.0（USBTMC 或 CDC 类），支持 SCPI-1999 命令集
- SMA 输出，SMA 触发 I/O，SMA 外部时钟输入
- 电源输入：24V DC，功耗 <20W

### 软件与扩展路线
- V1.0：单通道 + 基础波形 + 核心 SCPI
- V2.0：双通道 + 调制 + 扫频
- V3.0：Linux GUI + 任意波（16kpts）

---

## 🚦 项目状态

> **当前阶段：MVP 硬件验证完成，FPGA 代码开发中**

### ✅ 已完成
- [x] PCB 设计与打样（4层板，信号完整性设计）
- [x] 整板焊接（除重构中的模拟滤波器外）
- [x] 模拟前端调试通过 ✅ 已输出 **差分正弦波**（见下方截图）
- [x] DAC 上电参数配置正常（SPI 接口通信稳定）
- [x] 正弦 DDS 核（IP 核实现，48位相位累加器）

### 🏗️ 开发中（预计 8 月底完成）
- [ ] 其他波形 DDS：三角波 / 锯齿波 / 方波 / 噪声（公式生成，无需 LUT）
- [ ] 幅度控制模块
- [ ] 相位控制模块
- [ ] 偏置控制模块
- [ ] SPI Slave 通信接口与寄存器组
- [ ] 顶层模块集成与系统联调
- [ ] 7阶椭圆滤波器重构与焊接（当前正在优化元件选型）
- [ ] 差分转单端电路调试

### 📋 待办（MVP 之后）
- [ ] 模拟板完整版设计（加入 硬件幅度控制、直流偏置、外部触发、外部时钟输入）
- [ ] 输出性能标定（THD、SFDR、相位噪声）
- [ ] SCPI 协议栈实现
- [ ] 上位机控制软件

### 📸 当前进展

**差分正弦波输出（50MHz 示波器实测）：**

![1MHz 差分正弦波](/docs/pic/1MHz_差分正弦波.BMP)
![100kHz 差分正弦波](/docs/pic/100kHz_差分正弦波.BMP)

> *注：当前使用 50MHz 带宽示波器，更高频率波形与精确 THD 测量待返校后用更高带宽仪器进行。*

---

## 🧱 仓库结构
ArbWave30/  
├── docs/ # 设计文档  
│    ├── pic/ # 文档图片  
│    ├── adr/ # 决策记录  
│    ├── 硬件设计文档/ # 硬件设计文档  
│    └──  LICENSE # CC BY-SA 4.0  
├── hardware/ # 原理图、PCB源文件 (🚧 开发中)   
│    ├── analog_board/ # 模拟板 PCB设计  
│    ├── control_board/ # 控制板 PCB设计  
│    └── LICENSE # CERN-OHL-S-2.0   
├── firmware/ (🚧 开发中)   
│    ├── fpga/ # FPGA 设计源代码（Verilog）  
│    │    ├── Application/ # 项目代码
│    │    └── FPGA_Test_Hardware/ # FPGA 硬件测试代码
│    │         ├── FPGA_Clock_Test/ # 时钟测试代码
│    │         └── FPGA_Hardware_Test/ # FPGA 硬件测试代码
│    └── mcu/ # MCU 固件源代码（C/C++）  
│         ├── Application/ # 项目代码
│         └── MCU_Test_Hardware/ # MCU 硬件测试代码
│              └── min_system_test/ # MCU 最小系统测试代码
├── simulink/ # LTspice 仿真文件  
├── datasheets/ # 组件数据手册，参考设计  
├── scripts/ # 自动化测试脚本（PyVISA）(🚧 开发中)  
├── LICENSE # GPL-3.0  
└── README.md  

---

## 🚀 快速开始

> 项目处于 **最小可行产品（MVP）设计阶段**，正在编写 FPGA 代码，待 MVP 验证完成后，将提供：
> - 打样 PCB 的 Gerber 文件及 BOM
> - FPGA 比特流烧录方法（通过 JTAG  加载）
> - 固件编译与升级步骤

当前你可以浏览 `docs/` 中的设计文档，了解详细的技术规格和设计思路。

---

## 📡 SCPI 命令示例 

```python
# 实际使用时，请将资源字符串替换为你的设备地址（可通过 pyvisa 的 ResourceManager().list_resources() 获取）
import pyvisa
rm = pyvisa.ResourceManager()
inst = rm.open_resource('USB0::0x1234::0x4321::AWG001::INSTR')
print(inst.query('*IDN?'))
inst.write('SOURce1:FUNCtion SIN')
inst.write('SOURce1:FREQuency 10e6')
inst.write('SOURce1:VOLTage 5')
inst.write('OUTPut1 ON')
```

完整命令集参见 [docs/SCPI_reference.md](docs/SCPI_reference.md)(🚧 开发中)。

## 🧪 自动化测试
测试脚本位于 `scripts/`（编写中），计划使用 PyVISA 与设备通信。待硬件就绪后，运行 `python scripts/test_waveform.py` 可进行基本功能测试。  

## 📄 文档
设计文档、测试报告和用户手册位于 `docs/`，持续更新中。

## 📅 版本路线图
| 阶段 | 功能 | 描述 |
|------|------|------|
| V1.0 | 单通道，基础波形（正弦、方波、三角、锯齿、阶梯波）| 支持频率、幅值、偏置、相位连续可调；支持SCPI命令（少量核心指令）|
| V2.0 | 双通道 | 增加第二路输出，支持频率/相位同步，相位差可调 |
|      | AM调制 |内/外部调制源，调制深度0\~100% |
| V3.0 | 嵌入式GUI | 基于Linux + LVGL，通过LCD触摸屏直接控制
|      | 任意波 | 支持用户自定义波形表，存储深度≥1Mpts |

## 📜 许可证
Copyright © 2026 EERNINUO

本项目采用**多许可证策略**，不同部分适用不同许可证：

| 组成部分 | 许可证 |
|----------|--------|
| 固件 (MCU/FPGA 代码) | [GNU General Public License v3.0](LICENSE) |
| 硬件设计文件 (原理图、PCB、BOM) | [CERN Open Hardware Licence Version 2 - Strongly Reciprocal](hardware/LICENSE) |
| 文档、图片、README | [Creative Commons Attribution-ShareAlike 4.0 International](docs/LICENSE) |

这意味着：
- 你可以自由使用、修改、分享固件代码，但任何衍生代码也必须以 GPL-3.0 开源。
- 你可以基于硬件设计文件制作、分发物理设备，但必须同时开源你对设计文件的修改，并保留署名。
- 你可以复制和修改文档，但必须同样以 CC BY-SA 4.0 分享，并注明原作者。

**商标声明**：`ArbWave30` 名称及标志未经授权不得用于商业产品。

## 🤖 AI 辅助声明

本项目文档框架由 GitHub Copilot / DeepSeek 辅助生成。
作者已逐一审查并测试，但如有任何问题欢迎提交 Issue。

## ⚠️ 免责声明
注意：本项目为个人学习与开源作品，不保证商业级可靠性。使用前请仔细阅读免责声明。

## 📫 联系方式
项目主页: https://github.com/EERNINUO/ArbWave30

## ⭐ Star History
如果您觉得这个项目有帮助，请给一个 Star ⭐，谢谢！



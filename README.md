<div align="center">

# ArbWave30
30MHz 任意波形发生器 | FPGA + 高速DAC | SCPI 可控 

<p align="center">
  <a href="https://github.com/EERNINUO/ArbWave30" style="margin: 2px;">
    <img alt="GitHub 仓库星标" src="https://img.shields.io/github/stars/EERNINUO/ArbWave30">
  </a>
  <a href="https://github.com/EERNINUO/ArbWave30" style="margin: 2px;">
    <img alt="GitHub 仓库分支" src="https://img.shields.io/github/forks/EERNINUO/ArbWave30">
  </a>
  <a href="https://raw.githubusercontent.com/EERNINUO/ArbWave30/main/LICENSE" style="margin: 2px;">
    <img src="https://img.shields.io/github/license/EERNINUO/ArbWave30" alt="许可证">
  </a>
  <a href="https://github.com/EERNINUO/ArbWave30/releases" style="margin: 2px;">
    <img src="https://img.shields.io/github/v/release/EERNINUO/ArbWave30?color=blueviolet&include_prereleases" alt="发布版本">
  </a>
</p>

</div>

---

## ✨ 特性

### 硬件指标（V1.0）
- 波形：正弦、方波、三角波、锯齿波、阶梯波、直流
- 正弦波最高 **30MHz**，方波最高 **5MHz**（上升沿 <50ns）
- 幅值范围（高阻）：20Vpp（DC\~10MHz），14Vpp（10\~20MHz），10Vpp（20\~30MHz）
- 幅值分辨率：2mV（典型），偏移范围 ±5V
- 总谐波失真（THD）：< -50dBc @10MHz 满幅
- 相位噪声：-100dBc/Hz @10kHz offset（30MHz 载波）(目标值)
- 输出阻抗：50Ω / 高阻（继电器切换）
- 参考时钟：内部 TCXO（±1ppm）或外部 10MHz 输入

### 控制与接口
- USB 2.0（USBTMC 或 CDC 类），支持 SCPI-1999 命令集
- SMA 输出，SMA 触发 I/O，SMA 外部时钟输入
- 电源输入：24V DC，功耗 <20W

### 软件与扩展路线
- V1.0：单通道 + 基础波形 + 核心 SCPI
- V2.0：双通道 + AM 调制 + 频率扫描
- V3.0：Linux GUI + 任意波（1Mpts）

---

## 🧱 仓库结构
ArbWave30/  
├── docs/ # 设计文档  
│    └──  LICENSE # CC BY-SA 4.0
├── hardware/ # 原理图、PCB源文件 (🚧 开发中)  
├── firmware/ (🚧 开发中)   
│    ├── LICENSE # CERN-OHL-S-2.0  
│    ├── fpga/ # FPGA 设计源代码（Verilog）  
│    └── mcu/ # MCU 固件源代码（C/C++）  
├── datasheets/ # 组件数据手册，参考设计  
├── scripts/ # 自动化测试脚本（PyVISA）(🚧 开发中)  
├── LICENSE # GPL-3.0  
└── README.md  

---

## 🚀 快速开始

> 项目处于 **V1.0 草案阶段**，硬件尚未打样，待硬件验证完成后，将提供：
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
|      | AM调制 |内/外部调制源，调制深度0~100% |
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

## ⚠️ 免责声明
注意：本项目为个人学习与开源作品，不保证商业级可靠性。使用前请仔细阅读免责声明。

## 📫 联系方式
项目主页: https://github.com/EERNINUO/ArbWave30

## ⭐ Star History
如果您觉得这个项目有帮助，请给一个 Star ⭐，谢谢！



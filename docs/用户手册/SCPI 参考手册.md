<!------------------------------------------------------------------------
SPDX-License-Identifier: CC-BY-SA-4.0
Copyright (C) 2026 [EERNINUO]

This work is licensed under the Creative Commons Attribution-ShareAlike 4.0 International License. 
To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/4.0/.
------------------------------------------------------------------------->

<!-- Assisted-by: DeepSeek - 文档框架 -->

# ArbWave30 可编程仪器标准命令（SCPI）参考手册

| 文档编号 | ArbWave30-UM-002 | 版本 | V1.0 |
| --- | --- | --- | --- |
| 项目名称 | ArbWave30 | 日期 | 2026-08-17 |
| 编写人 | EERNINUO | 状态 | 初稿 |

## 1. 概述
本文档提供了 ArbWave30 任意波形发生器的远程控制命令集。该仪器遵循 **SCPI（Standard Commands for Programmable Instruments）1999.0 标准** 和 **IEEE 488.2-1992** 规范。

**通信接口**：
- **物理层**：USB 2.0（CDC 类虚拟串口）
- **波特率**：115200 bps （该参数对虚拟串口无实质影响）
- **数据位**：8，**停止位**：1，**校验位**：无
- **结束符**：`\n` (换行，十六进制 0x0A)

---

## 2. 语法约定

在阅读本手册时，请注意以下符号约定：

| 符号 | 含义 | 示例 |
| :--- | :--- | :--- |
| `[ ]` | 可选关键字或参数，可省略。 | `OUTPut[:STATe]` 表示 `OUTPut:STATe` 可缩写为 `OUTPut` |
| `{ }` | 必选参数组，从其中选择一个。 | `VOLTage {<amplitude>|MIN|MAX}` |
| `|` | 逻辑“或”分隔符。 | `ON|OFF` |
| `#` | 通道占位符。 | `SOURce#` 代表 `SOURce1` 或 `SOURce2` |
| `< >` | 用户必须替换的实际数值。 | `<frequency>` |
| `( )` | 命令的短格式（助记符），SCPI 引擎同时接受长格式和短格式。 | `FREQuency` (短格式: `FREQ`) |

**查询命令**：在命令末尾添加问号 `?` 可查询当前设定值，如 `FREQuency?`。

---

## 3. IEEE 488.2 通用命令

### 3.1 `*IDN?` —— 查询设备标识
**功能**：返回设备制造商、型号、固件版本和序列号。
**格式**：`*IDN?`
**响应**：`<厂商>,<型号>,<固件版本>,<序列号>`
**示例**：
> `*IDN?` -> `EERNINUO,ArbWave30,V1.0.0,SN123456`
> \# 注意：实际返回值可能因设备和固件版本而异。

### 3.2 `*RST` —— 设备复位
**功能**：将设备参数重置为出厂默认值。
**格式**：`*RST`
**默认参数**：
- 频率：0 Hz
- 波形：正弦波
- 幅值：0 Vpp
- 偏置：0 V
- 输出状态：关闭 (OFF)

### 3.3 `*CLS` —— 清除错误队列
**功能**：清空系统错误/事件队列。
**格式**：`*CLS`

## 4. `SOURce` 子系统（波形生成参数）

该子系统用于配置信号发生器核心的 DDS 参数。

### 4.1 频率设置 `[SOURce#:]FREQuency`

**描述**：设置或查询指定通道的输出频率。

**命令格式**：
`[SOURce#:]FREQuency {<frequency>|MIN|MAX|DEFAULT}`
`[SOURce#:]FREQuency? [MIN|MAX]`

**参数说明**：
- `<frequency>`：浮点数，单位 Hz。
- `MIN`：最小值，0 Hz (0 Hz)。
- `MAX`：最大值，30,000,000 Hz (30 MHz)。

**示例**：
- `SOURce1:FREQuency 1000000` -> 设置通道 1 频率为 1 MHz。
- `SOUR:FREQ MAX` -> 设置当前选中通道频率为 30 MHz（短格式）。
- `FREQuency?` -> 返回 `1000000.000000`。

### 4.2 波形类型 `[SOURce#:]FUNCtion`

**描述**：设置或查询波形形状。

**命令格式**：
`[SOURce#:]FUNCtion {SINusoid|SQUare|TRIangle|SAWtooth|DC}`
`[SOURce#:]FUNCtion?`

**参数说明**：
- `SINusoid` (短格式: `SIN`)：正弦波。
- `SQUare` (短格式: `SQU`)：方波。
- `TRIangle` (短格式: `TRI`)：三角波。
- `SAWtooth` (短格式: `SAW`)：锯齿波（上升沿）。

**示例**：
- `SOURce2:FUNCtion SQU` -> 设置通道 2 输出方波。

### 4.3 幅度设置 `[SOURce#:]VOLTage`

**描述**：设置或查询输出幅度（峰峰值 Vpp）。

**命令格式**：
`[SOURce#:]VOLTage {<amplitude>|MIN|MAX|DEFAULT}`
`[SOURce#:]VOLTage? [MIN|MAX]`

**参数说明**：
- `<amplitude>`：浮点数，单位 Vpp（高阻负载下）。在 50Ω 负载下，实际输出电压为此值的一半。
- `MIN`：~0 Vpp。
- `MAX`：~20 Vpp。

**示例**：
- `SOURce1:VOLTage 2.5` -> 设置通道 1 输出 2.5 Vpp。

---

## 5. `OUTPut` 子系统（物理输出控制）

该子系统控制信号发生器前端的物理继电器和阻抗匹配电路。**双通道独立控制**，使用 `OUTPut#` 区分。

### 5.1 输出开关 `OUTPut#:STATe`

**描述**：打开或关闭指定通道的信号输出继电器。

**命令格式**：
`OUTPut#:STATe {ON|OFF|1|0}`
`OUTPut#:STATe?`

**参数说明**：
- `ON` 或 `1`：闭合继电器，输出信号。
- `OFF` 或 `0`：断开继电器，输出端对地短路（静音）。

**示例**：
- `OUTPut1:STATe ON` -> 打开通道 1。
- `OUTPut2:STATe OFF` -> 关闭通道 2。
- `OUTPut:STATe?` -> 注意：不带通道号时，查询返回当前选中通道（默认为通道 1）状态，建议始终指定通道号。

### 5.2 输出阻抗 `OUTPut#:IMPedance`

**描述**：设置屏幕显示的负载阻抗值与幅度/偏置值，不影响物理输出阻抗，仅影响计算系数。

**命令格式**：
`OUTPut#:IMPedance {50|HIGH}`

**参数说明**：
- `50`：匹配 50Ω 系统，显示的幅度为实际负载上的电压（即设定值的一半）。
- `HIGH`（或 `INF`）：高阻模式，显示的幅度为开路电压（设定值等于实际输出）。

---

## 6. 系统与状态 `SYSTem`

### 6.1 错误队列查询 `SYSTem:ERRor?`

**描述**：读取并清除错误队列中最旧的一条错误信息。

**命令格式**：
`SYSTem:ERRor?`

**返回格式**：
`<错误码>,"<错误描述>"`

- 无错误时返回：`0,"No error"`

---

## 7. 错误代码列表（常见）

| 错误码 | 说明 | 解决方案 |
| :--- | :--- | :--- |
| 0 | 无错误 | - |
| -102 | 语法错误 | 检查命令拼写，如 `FREQ` 误写为 `FRQ` |
| -113 | 未定义的命令头 | 该命令不支持，检查手册 |
| -222 | 参数数据超出范围 | 频率值超过 30MHz 或小于 1mHz |
| -224 | 非法参数值 | 波形类型或布尔值错误，如 `SQURE` |

---
## 8. 编程示例（Python + PyVISA）
由于设备使用 USB CDC 接口，因此可以通过 PyVISA 和 PySerial 两种方式进行通信。以下示例展示了如何使用 PyVISA 控制 ArbWave30。

### 方式一：PySerial （最简单、最可靠）

```python
import serial
import time

# 1. 配置串口参数（波特率在 CDC 下通常被忽略，但为了兼容性保留）
# Windows 示例: 'COM3'，Linux 示例: '/dev/ttyUSB0' 或 '/dev/ttyACM0'
ser = serial.Serial(
    port='COM3',       # 替换为你的实际端口
    baudrate=115200,   # CDC 下这个值不影响通信速度，只要两端一致即可
    timeout=2,         # 读超时（秒），防止 readline 卡死
    write_timeout=2
)

# 2. 发送 SCPI 命令（注意：务必添加换行符 `\n`）
ser.write(b'*IDN?\n')

# 3. 读取响应（SCPI 标准以换行符 `\n` 结束）
response = ser.readline().decode('utf-8').strip()
print(f"Device Info: {response}")

# 4. 执行一连串操作示例
ser.write(b'*RST\n')             # 复位
time.sleep(0.1)                  # 给 MCU 一点处理时间（可选）
ser.write(b'SOUR1:FREQ 1000\n')  # 设置 1kHz
ser.write(b'SOUR1:FUNC SIN\n')   # 正弦波
ser.write(b'OUTP1:STAT ON\n')    # 打开输出

# 5. 查询频率
ser.write(b'OUTP1:FREQ?\n')
freq = ser.readline().decode('utf-8').strip()
print(f"Current Frequency: {freq} Hz")

# 关闭串口
ser.close()
```

### 方式二：PyVISA （功能更强大）

```python
import pyvisa

# 1. 指定纯 Python 后端（无需安装 NI-VISA 驱动）
rm = pyvisa.ResourceManager('@py')

# 2. 列出所有资源，找到你的 COM 口（通常是 'ASRL3::INSTR' 类似格式）
# print(rm.list_resources())

# 3. 打开串口资源，并配置终止符
inst = rm.open_resource('ASRL3::INSTR')  # 请替换为实际的 ASRL 端口号
inst.baud_rate = 115200
inst.read_termination = '\n'   # 非常重要：库默认是 '\n'，但最好显式声明
inst.write_termination = '\n'  # 发送时自动追加换行符

# 4. 现在你可以像用 USBTMC 仪器一样使用 query
print(inst.query('*IDN?'))
inst.write('SOUR:FREQ 1000')
inst.write('SOUR:STAT ON')

# 5. 查询
freq = inst.query('SOUR:FREQ?')
print(f"Freq: {freq}")

inst.close()
```

## 6. 版本记录

| 版本 | 日期 | 修改内容 | 修改人 |
|------|------|----------|--------|
| V1.0 | 2026-08-17 | 初始架构 | EERNINUO |
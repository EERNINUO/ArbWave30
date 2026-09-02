# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 [EERNINUO]
#
# [This file is part of ArbWave30.]
#
# ArbWave30 is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
# ...

# 测试单端滤波器响应
# 测试设备：Siglent SDS824HD
# 该脚本仅供参考，不同示波器的指令集可能不同，请根据实际情况修改。

import time
import math
import pyvisa
from matplotlib import pyplot as plt

# 选择你的设备地址
OSCILLOSCOPE = "USB0::0xF4EC::0x1017::SDS08A0CA01306::INSTR"
ARBWAVE30 = "ASRL18::INSTR"

freq_list = []  # 测试频率列表，单位Hz
vpp_list = []  # 测试结果列表，单位Vpp
db_list = []  # 测试结果列表，单位dB

if OSCILLOSCOPE == "" or ARBWAVE30 == "":
    for device in pyvisa.ResourceManager().list_resources():
        print(device)
    print("请选择你的设备地址")
else:
    rm = pyvisa.ResourceManager()
    oscilloscope = rm.open_resource(OSCILLOSCOPE)
    arbwave30 = rm.open_resource(ARBWAVE30)

    arbwave30.write("*IDN?")
    print(arbwave30.read())

    # 配置基础参数
    arbwave30.write("SOURce1:FREQuency 10")
    arbwave30.write("SOURce1:VOLTage 10")
    arbwave30.write("OUTPut1:STATe ON")

    # 配置示波器
    oscilloscope.write("*RST") 

    oscilloscope.write(":MEAS ON") # 开启测量
    oscilloscope.write(":MEAS:ADV:CLE") # 清除测量项

    # 配置模拟通道
        # 打开通道1，关闭通道2
    oscilloscope.write(":CHAN1:SWIT ON")
    oscilloscope.write(":CHAN2:SWIT OFF")
        #设置耦合
    oscilloscope.write(":CHAN1:COUP DC")
        # 配置探头倍数
    oscilloscope.write(":CHAN1:PROB DEF")
        # 设置垂直标尺
    oscilloscope.write(":CHAN1:SCAL 1") 
        # 设置垂直位置
    oscilloscope.write(":CHAN1:OFFS 0")
        # 设置水平标尺
    oscilloscope.write(":TIM:SCAL 5E-2")
        # 设置水平位置
    oscilloscope.write(":TIM:DEL 0")
        # 配置触发
    oscilloscope.write(":TRIG:MODE AUTO")
    oscilloscope.write(":TRIG:RUN")
    oscilloscope.write(":TRIG:EDGE:SOUR CHAN1")
    oscilloscope.write(":TRIG:EDGE:LEV 0.1")
    oscilloscope.write(":TRIG:EDGE:SLOP RIS")

    # 配置测量项
    oscilloscope.write(":MEAS ON")
    oscilloscope.write(":MEAS:MODE ADV")
    oscilloscope.write(":MEAS:ADV:CLE")

    oscilloscope.write(":MEAS:ADV:P1:TYPE PKPK")
    oscilloscope.write(":MEAS:ADV:P1:SOUR1 C1")
    oscilloscope.write(":MEAS:ADV:P1 ON")

    time.sleep(5)  # 等待示波器稳定

    for exponent in range(1, 8):  # 指数项
        timescale = 0.5 / (10 ** exponent)
        oscilloscope.write(":TIM:SCAL " + str(timescale))
        for i in range(1, 10):  # 系数项
            if exponent != 7 or i < 7:  
                freq = i * 10 ** exponent
                arbwave30.write("SOURce1:FREQuency " + str(freq))
                time.sleep(0.2)  # 等待信号稳定
                oscilloscope.write("MEAS:ADV:STAT:RES")  
                time.sleep(2)
                oscilloscope.write(":MEAS:ADV:P1:STAT? MEAN")
                time.sleep(0.2)
                vpp = float(oscilloscope.read())
                freq_list.append(freq)
                vpp_list.append(vpp)
                db_list.append(20 * math.log10(vpp / 12))  # 转换为dB
    print("测试完成，测试结果如下：")
    print("频率列表：", freq_list)
    print("Vpp列表：", vpp_list)
    print("dB列表：", db_list)

    fig = plt.figure()
    ax1 = fig.subplots()

    x = freq_list
    y = db_list

    ax1.semilogx(x, y, label="diff to single Response")

    ax1.set_xlabel("Frequency (Hz)")
    ax1.set_ylabel("Amplitude (dB)")
    ax1.set_title("diff to single Response")
    ax1.grid(True)

    plt.show()

    arbwave30.close()
    oscilloscope.close()
    rm.close()


/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 [EERNINUO]
 *
 * [This file is part of ArbWave30.]
 *
 * ArbWave30 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * ...
 */

#ifndef ANALOG_BOARD_DRIVE_H
#define ANALOG_BOARD_DRIVE_H

#include <stdbool.h>
#include "stm32f4xx.h"
#include "main.h"

// 模拟板控制引脚定义
#define SPI_CS_Port SPI1_CS_GPIO_Port
#define SPI_CS_Pin SPI1_CS_Pin

#define ANALOG_BOARD_RST_Port FPGA_NRST_GPIO_Port
#define ANALOG_BOARD_RST_Pin FPGA_NRST_Pin

#define INTERRUPT_Port FPGA_INT_GPIO_Port
#define INTERRUPT_Pin FPGA_INT_Pin

// 模拟板系统参数
#define ANALOG_BOARD_FREQ 150000000 // 模拟板时钟频率，单位 Hz
#define FREQ_CTRL_WORD_LENGTH 48 // 频率控制字长度，单位 bit

// 参数范围
#define FREQ_MIN 0 // 最小频率，单位 Hz
#define FREQ_MAX 30000000 // 最大频率，单位 Hz
#define VOLT_MIN -10000 // 最小输出电压，单位 mV
#define VOLT_MAX 10000 // 最大输出电压，单位 mV
#define PHASE_MIN 0 // 最小相位，单位 度（°）
#define PHASE_MAX 36000 // 最大相位，单位 度（°）
#define DUTY_MIN 0 // 最小占空比，单位 %
#define DUTY_MAX 10000 // 最大占空比，单位 %

// CRC8计算参数
#define CRC_POLYNOMIAL 0x07
#define CRC_INIT 0x00

// ACK响应定义
#define ACK_CRC_ERROR 0x08
#define ACK_BUSY_ERROR 0x04
#define ACK_ADDR_ERROR 0x02
#define ACK_OK 0x01

// 模拟板寄存器基地址定义
#define SYSTEM_BASE 0x00
#define CHANNEL1_BASE 0x10
#define CHANNEL2_BASE 0x20
#define DACMAP_BASE 0x30

// 模拟板系统控制寄存器地址偏移量
#define SYS_ID_L 0x00
#define SYS_ID_H 0x01
#define SYS_CTRL 0x02
#define SYS_STATUS 0x03

// 模拟板通道控制寄存器地址偏移量
#define CHx_CTRL 0x00
#define CHx_FREQ_L 0x01
#define CHx_FREQ_M 0x02
#define CHx_FREQ_H 0x03
#define CHx_AMPL 0x04
#define CHx_OFFSET 0x05
#define CHx_PHASE 0x06
#define CHx_DUTY 0x07

// 系统控制寄存器位定义

#define SYS_CTRL_UPDATE         0x0008 
#define SYS_CTRL_DAC_CMD_WRITE  0x0004 
#define SYS_CTRL_CLOCK_SRC      0x0002 
#define SYS_CTRL_SOFT_RST       0x0001 

// 通道控制寄存器位定义
#define CH_CTRL_ENABLE          0x8000 
#define CH_CTRL_WAVEFORM        0x0000  
#define CH_CTRL_WAVEFORM_MASK   0x3F     // 低6位

// 寄存器地址计算宏
#define REG_BASE_ADDR(channel) ((channel == 1) ? CHANNEL1_BASE : CHANNEL2_BASE)
#define REG_ADDR(base, offset) ((base) + (offset))

// 波形枚举
typedef enum {
    WAVE_SINE    = 0x00,
    WAVE_SQUARE  = 0x01,
    WAVE_TRIANGLE= 0x02,
    WAVE_SAWTOOTH= 0x03,
    WAVE_NOISE   = 0x04
} WaveType_t;

// 模拟板配置结构体
// 这两个结构体只是镜像数据，向模拟板写入数据必须通过 setXXX 函数，操作这两个结构体不会影响模拟板
typedef struct{
    bool enable;            // 通道使能
    WaveType_t waveType;    // 波形类型
    uint64_t freq_uHz;      // 频率，单位 uHz
    int16_t amplitude_mV;   // 幅度，单位 mV
    int16_t offset_mV;      // 偏移量，单位 mV
    uint16_t phase;          // 相位，单位 度（°）（0~36000，对应 0.00°~360.00°）
    uint16_t duty;          // 占空比/对称度，单位 %，仅对三角波和方波有效（0~10000，对应 0~100.00）
} ChConfig_t;

typedef struct{
    bool extern_clock;      // 外部时钟使能
    ChConfig_t ch1;
    ChConfig_t ch2;
} AnalogBoardConfig_t;

void analogBoard_waitReady(void);
uint8_t analogBoard_setEnable(uint8_t channel, bool enable);
uint8_t analogBoard_setWave(uint8_t channel, WaveType_t waveType);
uint8_t analogBoard_setFreq(uint8_t channel, uint64_t freq_uHz);
uint8_t analogBoard_setAmplitude(uint8_t channel, int16_t amplitude_mV);
uint8_t analogBoard_setOffset(uint8_t channel, int16_t offset_mV);
uint8_t analogBoard_setPhase(uint8_t channel, uint16_t phase);
uint8_t analogBoard_setDuty(uint8_t channel, uint16_t duty);


#endif // ANALOG_BOADR_DRIVE_H
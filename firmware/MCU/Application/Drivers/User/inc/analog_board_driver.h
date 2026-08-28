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

#ifndef ANALOG_BOARD_DRIVER_H
#define ANALOG_BOARD_DRIVER_H

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
#define VOLT_MIN -10 // 最小输出电压，单位 V
#define VOLT_MAX 10 // 最大输出电压，单位 V
#define PHASE_MIN 0 // 最小相位，单位 度（°）
#define PHASE_MAX 360 // 最大相位，单位 度（°）
#define DUTY_MIN 0 // 最小占空比，单位 %
#define DUTY_MAX 100 // 最大占空比，单位 %

// CRC8计算参数
#define CRC_POLYNOMIAL 0x07
#define CRC_INIT 0x00

// ACK响应定义
#define ACK_CRC_ERROR 0x08
#define ACK_BUSY_ERROR 0x04
#define ACK_ADDR_ERROR 0x02
#define ACK_OK 0x01

// 波形枚举
typedef enum {
    WAVE_SINE     = 0x00,
    WAVE_SQUARE   = 0x01,
    WAVE_TRIANGLE = 0x02,
    WAVE_NOISE    = 0x03,
    WAVE_DC       = 0x04,
} WaveType_t;

// 系统状态枚举
typedef enum {
    PLL_LOCK = 0x01,
} SysStatus_t;


/* ---------------------------------------- 
 *  函数声明
 * --------------------------------------*/
// 系统控制函数
void analogBoard_waitReady(void);
uint8_t analogBoard_readSysId(char *sys_id);
uint8_t analogBoard_setClockSource(bool extern_clock);
uint8_t analogBoard_softReset(void);
void analogBoard_hardReset(void);
uint8_t analogBoard_readSysStatus(uint16_t *sys_status);

// 参数设置函数
void analogBoard_setImpedance(uint8_t channel, bool impedance);
uint8_t analogBoard_setEnable(uint8_t channel, bool enable);
uint8_t analogBoard_setWave(uint8_t channel, WaveType_t waveType);
uint8_t analogBoard_setFrequency(uint8_t channel, uint64_t freq_uHz);
uint8_t analogBoard_setAmplitude(uint8_t channel, int16_t amplitude_mV);
uint8_t analogBoard_setOffset(uint8_t channel, int16_t offset_mV);
uint8_t analogBoard_setPhase(uint8_t channel, uint16_t phase);
uint8_t analogBoard_setDuty(uint8_t channel, uint16_t duty);

// 参数读取函数
// 这些函数都是通过读取 MCU 中的镜像结构体来实现的，不需要与 FPGA 通信，不存在 ACK 响应，因此直接返回读取的数据即可
bool analogBoard_getImpedance(uint8_t channel);
bool analogBoard_getEnable(uint8_t channel);
WaveType_t analogBoard_getWave(uint8_t channel);
uint64_t analogBoard_getFrequency(uint8_t channel);
int16_t analogBoard_getAmplitude(uint8_t channel);
int16_t analogBoard_getOffset(uint8_t channel);
uint16_t analogBoard_getPhase(uint8_t channel);
uint16_t analogBoard_getDuty(uint8_t channel);

#endif // ANALOG_BOADR_DRIVE_H
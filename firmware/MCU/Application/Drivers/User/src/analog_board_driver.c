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

// Assisted-by: GitHub Copilot - CRC 校验函数

#include "analog_board_driver.h"
#include <stdlib.h>

// 外部变量声明
// 我一直觉得这种到处extern的写法很讨厌，但是CubeMX生成的代码就是这样，很难受
extern SPI_HandleTypeDef hspi1; // 声明SPI句柄 

// 数据帧结构体定义
typedef struct __packed community_frame {
	uint8_t address;
	uint16_t data;
	uint8_t crc;
	uint8_t ack;
} community_frame_t;

#define spi_cs(x) \
do { \
	if (x) HAL_GPIO_WritePin(SPI_CS_Port, SPI_CS_Pin, GPIO_PIN_SET); \
	else HAL_GPIO_WritePin(SPI_CS_Port, SPI_CS_Pin, GPIO_PIN_RESET); \
} while(0)

#define analog_board_rst(x) \
do { \
	if (x) HAL_GPIO_WritePin(ANALOG_BOARD_RST_Port, ANALOG_BOARD_RST_Pin, GPIO_PIN_SET); \
	else HAL_GPIO_WritePin(ANALOG_BOARD_RST_Port, ANALOG_BOARD_RST_Pin, GPIO_PIN_RESET); \
} while(0)

#define int_read() HAL_GPIO_ReadPin(INTERRUPT_Port, INTERRUPT_Pin)

// 模拟板配置结构体定义
AnalogBoardConfig_t analogBoardConfig = {
	.extern_clock = false,
	.ch1 = {
		.enable = false,
		.waveType = WAVE_SINE,
		.duty = 5000,
		.freq_uHz = 0,
		.amplitude_mV = 0,
		.offset_mV = 0,
		.phase = 0,
	},
	.ch2 = {
		.enable = false,
		.waveType = WAVE_SINE,
		.duty = 5000,
		.freq_uHz = 0,
		.amplitude_mV = 0,
		.offset_mV = 0,
		.phase = 0,
	}
};

/**
 * @brief  CRC8计算函数
 * @param  data: 指向数据的指针
 * @param  length: 数据长度
 * @retval CRC8值
 */
uint8_t crc8(uint8_t *data, uint8_t length)
{
	uint8_t crc = CRC_INIT;
	for (uint8_t i = 0; i < length; i++){
		crc ^= data[i];
		for (uint8_t j = 0; j < 8; j++){
			if (crc & 0x80)
				crc = (crc << 1) ^ CRC_POLYNOMIAL;
			else
				crc <<= 1;
		}
	}

	return crc;
}

// 这里用阻塞写法只是测试程序懒得写异步了，application中肯定会用异步方式的
void analogBoard_waitReady(void)
{
	while (int_read() != GPIO_PIN_RESET) {
		// 等待模拟板准备好
	}
}

/* ---------------------------------------- 
 *  寄存器操作函数
 * --------------------------------------*/

/**
 * @brief  发送命令到模拟板
 * @param  address: 模拟板地址
 * @param  command: 命令
 * @retval ACK响应
 */
uint8_t analogBoard_sendData(uint8_t address, uint16_t command)
{
	community_frame_t frame;
	frame.address = address;
	frame.data = command;
	// 计算 CRC 时要排除CRC和ACK字段
	frame.crc = crc8((uint8_t *)&frame, 3);
	frame.ack = 0;						

	spi_cs(0); // 拉低CS，开始通信
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&frame, 4, 1000); // 发送数据帧，减1是为了排除ACK字段
	HAL_SPI_Receive(&hspi1, (uint8_t *)&frame.ack, 1, 1000); // 接收ACK响应
	spi_cs(1); // 拉高CS，结束通信

	return frame.ack; // 返回ACK响应
}

/**
 * @brief  从模拟板读取数据
 * @param  address: 模拟板地址
 * @param  data: 指向数据存储位置的指针
 * @retval ACK响应
 */
uint8_t analogBoard_readData(uint8_t address, uint16_t *data)
{
	assert_param(data != NULL);

	community_frame_t frame;
	frame.address = address;
	frame.data = 0;
	// 计算 CRC 时要排除CRC和ACK字段
	frame.crc = crc8((uint8_t *)&frame, 3);
	frame.ack = 0;						

	spi_cs(0); // 拉低CS，开始通信
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&frame, 1, 1000); 
	HAL_SPI_Receive(&hspi1, (uint8_t *)&frame.data, 4, 1000); // 接收数据
	spi_cs(1); // 拉高CS，结束通信

	if (frame.crc != crc8((uint8_t *)&frame, 3)) {
		frame.ack |= ACK_CRC_ERROR; // CRC错误
		frame.ack &= ~ACK_OK; // 清除ACK_OK标志
	}

	if (frame.ack == ACK_OK) {
		*data = frame.data; 
	} else {
		*data = 0;
	}

	return frame.ack; // 返回ACK响应
}

/* ---------------------------------------- 
 *  系统功能函数
 * --------------------------------------*/

/**
 * @brief  读取系统ID
 * @param  sys_id: 指向存储系统ID的指针，大小为4字节
 * @retval ACK响应
**/
uint8_t analogBoard_readSysId(char *sys_id)
{
	uint8_t ack = 0;
	uint16_t *data = (uint16_t *)sys_id;

	if ((ack = analogBoard_readData(REG_ADDR(SYSTEM_BASE, SYS_ID_L),  &data[1])) != ACK_OK)
		goto error_handler;
	if ((ack = analogBoard_readData(REG_ADDR(SYSTEM_BASE, SYS_ID_H), &data[0])) != ACK_OK)
		goto error_handler;
	
	// 大小端转换
	data[0] = __builtin_bswap16(data[0]); 
	data[1] = __builtin_bswap16(data[1]);

	return ACK_OK;

error_handler:
	// 错误处理
	return ack;
}

/**
 * @brief  设置时钟源
 * @param  extern_clock: 是否使用外部时钟
 * @retval ACK响应
*/
uint8_t analogBoard_setClockSource(bool extern_clock)
{
	uint8_t ack = 0;
	uint16_t sys_ctrl = extern_clock << SYS_CTRL_CLOCK_SRC;

	if ((ack = analogBoard_sendData(REG_ADDR(SYSTEM_BASE, SYS_CTRL), sys_ctrl)) != ACK_OK)
		goto error_handler;

	analogBoardConfig.extern_clock = extern_clock;

	return ACK_OK;

error_handler:
	// 错误处理
	return ack;
}

/**
 * @brief  模拟板软复位（其实功能和模拟板硬复位一样，但既然模拟板留了，那就实现一下，指不定什么时候用得到）
 * @retval ACK响应
 */
uint8_t analogBoard_softReset(void)
{
	uint8_t ack = 0;
	uint16_t sys_ctrl = analogBoardConfig.extern_clock << SYS_CTRL_CLOCK_SRC | 1 << SYS_CTRL_SOFT_RST;

	if ((ack = analogBoard_sendData(REG_ADDR(SYSTEM_BASE, SYS_CTRL), sys_ctrl)) != ACK_OK)
		goto error_handler;

	return ACK_OK;

error_handler:
	// 错误处理
	return ack;
}

/**
 * @brief  模拟板硬复位（通过拉低RST引脚实现，建议使用这个函数进行复位）
 */
void analogBoard_hardReset(void)
{
    analog_board_rst(0);
    HAL_Delay(100);
	analog_board_rst(1);
}

/**
 * @brief  读取系统状态
 * @param  sys_status: 指向存储系统状态的指针
 * @retval ACK响应
 */
uint8_t analogBoard_readSysStatus(uint16_t *sys_status)
{
	uint8_t ack = 0;

	if ((ack = analogBoard_readData(REG_ADDR(SYSTEM_BASE, SYS_STATUS), sys_status)) != ACK_OK)
		goto error_handler;

	return ACK_OK;

error_handler:
	// 错误处理
	return ack;
}

/* ---------------------------------------- 
 *  通道功能函数
 * --------------------------------------*/

/**
 * @brief  设置通道使能状态
 * @param  channel: 通道号，1或2
 * @param  enable: 使能状态
 * @retval ACK响应
 */
uint8_t analogBoard_setEnable(uint8_t channel, bool enable)
{
	uint8_t ack = 0;
	uint8_t reg_base_addr = REG_CH_BASE_ADDR(channel);

	ChConfig_t *cfg = (channel == 1) ? &analogBoardConfig.ch1 : &analogBoardConfig.ch2;
	uint16_t ch_ctrl = (enable) << CH_CTRL_ENABLE | (cfg->waveType & CH_CTRL_WAVEFORM_MASK); // 构建通道控制寄存器值

	// 发送使能状态
	if ((ack = analogBoard_sendData(REG_ADDR(reg_base_addr, CHx_CTRL), ch_ctrl)) != ACK_OK)
		goto error_handler;

	// 更新配置结构体中的使能状态值
	cfg->enable = enable;

	return ACK_OK;

error_handler:
	// 错误处理
	return ack;
}

/**
 * @brief  设置波形类型
 * @param  channel: 通道号，1或2
 * @param  waveType: 波形类型
 * @retval ACK响应
 */
uint8_t analogBoard_setWave(uint8_t channel, WaveType_t waveType)
{
	uint8_t ack = 0;
	uint8_t reg_base_addr = REG_CH_BASE_ADDR(channel);

	ChConfig_t *cfg = (channel == 1) ? &analogBoardConfig.ch1 : &analogBoardConfig.ch2;
	uint16_t ch_ctrl = (cfg->enable) << CH_CTRL_ENABLE | (waveType & CH_CTRL_WAVEFORM_MASK); // 构建通道控制寄存器值

	// 发送波形类型
	if ((ack = analogBoard_sendData(REG_ADDR(reg_base_addr, CHx_CTRL), ch_ctrl)) != ACK_OK)
		goto error_handler;

	// 更新配置结构体中的波形类型值
	cfg->waveType = waveType;

	return ACK_OK;

error_handler:
	// 错误处理
	return ack;
}

/**
 * @brief  设置频率
 * @param  channel: 通道号，1或2
 * @param  freq_uHz: 频率，单位 uHz
 * @retval ACK响应
 */
uint8_t analogBoard_setFreq(uint8_t channel, uint64_t freq_uHz)
{
	// 限制频率范围
	if (freq_uHz > FREQ_MAX * 1000000ull) {
	    freq_uHz = FREQ_MAX * 1000000ull;
	} else if (freq_uHz < FREQ_MIN * 1000000ull) {
	    freq_uHz = FREQ_MIN * 1000000ull;
	}

	uint8_t ack = 0;
	uint8_t reg_base_addr = REG_CH_BASE_ADDR(channel);
	uint64_t freq_ctrl_word = freq_uHz * ((uint64_t)1 << FREQ_CTRL_WORD_LENGTH) / (ANALOG_BOARD_FREQ * 1000000ull); // 计算频率控制字

	// 发送频率控制字
	if ((ack = analogBoard_sendData(REG_ADDR(reg_base_addr, CHx_FREQ_L), (uint16_t)(freq_ctrl_word & 0xFFFF))) != ACK_OK)
		goto error_handler;
	if ((ack = analogBoard_sendData(REG_ADDR(reg_base_addr, CHx_FREQ_M), (uint16_t)((freq_ctrl_word >> 16) & 0xFFFF))) != ACK_OK)
		goto error_handler;
	if ((ack = analogBoard_sendData(REG_ADDR(reg_base_addr, CHx_FREQ_H), (uint16_t)((freq_ctrl_word >> 32) & 0xFFFF))) != ACK_OK)
		goto error_handler;
	
	// 更新配置结构体中的频率值
	if (channel == 1)
		analogBoardConfig.ch1.freq_uHz = freq_uHz; 
	else
		analogBoardConfig.ch2.freq_uHz = freq_uHz; 

	// 影子寄存器更新
	if ((ack = analogBoard_sendData(SYSTEM_BASE + SYS_CTRL, SYS_CTRL_UPDATE)) != ACK_OK)
		goto error_handler;

	return ACK_OK;

error_handler:
	// 错误处理
	// 可以在这里添加错误处理代码，例如重试、记录日志等
	return ack;
}

/**
 * @brief  设置幅度
 * @param  channel: 通道号，1或2
 * @param  amplitude_mV: 幅度，单位 mV
 * @retval ACK响应
 */
uint8_t analogBoard_setAmplitude(uint8_t channel, int16_t amplitude_mV)
{
	// 限制幅度范围
    // 用指针可消除选择通道 1/2 的 if 语句
    ChConfig_t *cfg = (channel == 1) ? &analogBoardConfig.ch1 : &analogBoardConfig.ch2;
    int16_t offset_mV = cfg->offset_mV;

    // 限幅逻辑
    if (abs(amplitude_mV) + abs(offset_mV) > VOLT_MAX) {
        int16_t limit = VOLT_MAX - abs(offset_mV); 
        amplitude_mV = (amplitude_mV >= 0) ? limit : -limit;
    }

	uint8_t ack = 0;
	uint8_t reg_base_addr = REG_CH_BASE_ADDR(channel);
	int16_t amplitude_ctrl_word = (int32_t)amplitude_mV * 0x7FFF / 10000; // 计算幅度控制字，强制类型转换是为了防止溢出
	
	// 发送幅度控制字
	if ((ack = analogBoard_sendData(REG_ADDR(reg_base_addr, CHx_AMPL), *(uint16_t *)(&amplitude_ctrl_word))) != ACK_OK)
		goto error_handler;

	// 更新配置结构体中的幅度值	
	cfg->amplitude_mV = amplitude_mV;

	// 影子寄存器更新
	if ((ack = analogBoard_sendData(SYSTEM_BASE + SYS_CTRL, SYS_CTRL_UPDATE)) != ACK_OK)
		goto error_handler;

	return ACK_OK;	

error_handler:
	// 错误处理
	// 可以在这里添加错误处理代码，例如重试、记录日志等
	return ack;
}

/**
 * @brief  设置偏移量
 * @param  channel: 通道号，1或2
 * @param  offset_mV: 偏移量，单位 mV
 * @retval ACK响应
 */	
uint8_t analogBoard_setOffset(uint8_t channel, int16_t offset_mV)
{
	// 限制偏移量范围
	// 用指针可消除选择通道 1/2 的 if 语句
    ChConfig_t *cfg = (channel == 1) ? &analogBoardConfig.ch1 : &analogBoardConfig.ch2;
    int16_t amplitude_mV = cfg->amplitude_mV;

    // 限幅逻辑
    if (abs(offset_mV) + abs(amplitude_mV) > VOLT_MAX) {
        int16_t limit = VOLT_MAX - abs(amplitude_mV); 
        offset_mV = (offset_mV >= 0) ? limit : -limit;
    }

	uint8_t ack = 0;
	uint8_t reg_base_addr = REG_CH_BASE_ADDR(channel);
	int16_t offset_ctrl_word = (int32_t)offset_mV * 0x7FFF / 10000; // 计算偏移量控制字，强制类型转换是为了防止溢出

	// 发送偏移量控制字
	if ((ack = analogBoard_sendData(REG_ADDR(reg_base_addr, CHx_OFFSET), offset_ctrl_word)) != ACK_OK)
		goto error_handler;

	// 更新配置结构体中的偏移量值
	cfg->offset_mV = offset_mV;

	// 影子寄存器更新
	if ((ack = analogBoard_sendData(REG_ADDR(SYSTEM_BASE, SYS_CTRL), SYS_CTRL_UPDATE)) != ACK_OK)
		goto error_handler;	
	
	return ACK_OK;

error_handler:
	// 错误处理
	return ack;
}

/**
 * @brief  设置相位
 * @param  channel: 通道号，1或2
 * @param  phase: 相位，单位 度（°）（0~36000，对应 0.00°~360.00°）
 * @retval ACK响应
 */
uint8_t analogBoard_setPhase(uint8_t channel, uint16_t phase)
{
	if (phase > PHASE_MAX) {
		phase = PHASE_MAX; // 限制相位范围
	} else if (phase < PHASE_MIN) {
		phase = PHASE_MIN;
	}

	uint8_t ack = 0;
	uint8_t reg_base_addr = REG_CH_BASE_ADDR(channel);
	int16_t phase_ctrl_word = (int32_t)phase * 0x7FFF / 36000; // 计算相位控制字，强制类型转换是为了防止溢出

	// 发送相位控制字
	if ((ack = analogBoard_sendData(REG_ADDR(reg_base_addr, CHx_PHASE), phase_ctrl_word)) != ACK_OK)
		goto error_handler;

	// 更新配置结构体中的相位值
	if (channel == 1)
		analogBoardConfig.ch1.phase = phase;
	else
		analogBoardConfig.ch2.phase = phase;

	// 影子寄存器更新
	if ((ack = analogBoard_sendData(REG_ADDR(SYSTEM_BASE, SYS_CTRL), SYS_CTRL_UPDATE)) != ACK_OK)
		goto error_handler;

	return ACK_OK;

error_handler:
	// 错误处理
	return ack;
}

/**
 * @brief  设置占空比/对称度，仅对三角波和方波有效
 * @param  channel: 通道号，1或2
 * @param  duty: 占空比/对称度，单位 %，仅对三角波和方波有效（0~10000，对应 0~100.00）
 * @retval ACK响应
 */
uint8_t analogBoard_setDuty(uint8_t channel, uint16_t duty)
{
	if (duty > DUTY_MAX) {
		duty = DUTY_MAX; // 限制占空比范围
	} else if (duty < DUTY_MIN) {
		duty = DUTY_MIN;
	}

	uint8_t ack = 0;
	uint8_t reg_base_addr = REG_CH_BASE_ADDR(channel);
	uint16_t duty_ctrl_word = (int32_t)duty * 0xFF / 10000; // 计算占空比控制字，强制类型转换是为了防止溢出

	// 发送占空比
	if ((ack = analogBoard_sendData(REG_ADDR(reg_base_addr, CHx_DUTY), duty_ctrl_word)) != ACK_OK)
		goto error_handler;

	// 更新配置结构体中的占空比值
	if (channel == 1)
		analogBoardConfig.ch1.duty = duty; 
	else
		analogBoardConfig.ch2.duty = duty; 
	// 影子寄存器更新
	if ((ack = analogBoard_sendData(SYSTEM_BASE + SYS_CTRL, SYS_CTRL_UPDATE)) != ACK_OK)
		goto error_handler;

	return ACK_OK;

error_handler:
	// 错误处理
	return ack;
}
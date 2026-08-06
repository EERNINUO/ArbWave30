/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 [Your Name/Organization]
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
	if (x) HAL_GPIO_WritePin(spi_cs_port, spi_cs_pin, GPIO_PIN_SET); \
	else HAL_GPIO_WritePin(spi_cs_port, spi_cs_pin, GPIO_PIN_RESET); \
} while(0)

#define alaog_board_rst(x) \
do { \
	if (x) HAL_GPIO_WritePin(analog_board_rst_port, analog_board_rst_pin, GPIO_PIN_SET); \
	else HAL_GPIO_WritePin(analog_board_rst_port, analog_board_rst_pin, GPIO_PIN_RESET); \
} while(0)

#define int_read() HAL_GPIO_ReadPin(interrupt_port, interrupt_pin)

/**
 * @brief  CRC8计算函数
 * @param  data: 指向数据的指针
 * @param  length: 数据长度
 * @retval CRC8值
 */
uint8_t crc8(uint8_t *data, uint8_t length)
{
	uint8_t crc = crc_init;
	for (uint8_t i = 0; i < length; i++){
		crc ^= data[i];
		for (uint8_t j = 0; j < 8; j++){
			if (crc & 0x80)
				crc = (crc << 1) ^ crc_polynomial;
			else
				crc <<= 1;
		}
	}

	return crc;
}

// 这里用阻塞写法只是测试程序懒得写异步了，application中肯定会用异步方式的
void wait_analog_board_ready(void)
{
	while (int_read() == GPIO_PIN_SET) {
		// 等待模拟板准备好
	}
}

/**
 * @brief  发送命令到模拟板
 * @param  address: 模拟板地址
 * @param  command: 命令
 * @retval ACK响应
 */
uint8_t send_command(uint8_t address, uint16_t command)
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

uint8_t read_data(uint8_t address, uint16_t data)
{
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
		frame.ack |= ack_crc_error; // CRC错误
		frame.ack &= ~ack_ok; // 清除ACK_OK标志
	}

	if (frame.ack == ack_ok) {
		data = frame.data; 
	} else {
		data = 0;
	}

	return frame.ack; // 返回ACK响应
}
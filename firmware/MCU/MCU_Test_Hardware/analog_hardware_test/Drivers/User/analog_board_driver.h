#ifndef ANALOG_BOADR_DRIVE_H
#define ANALOG_BOADR_DRIVE_H

#include "stm32f4xx.h"
#include "main.h"

// 模拟板控制引脚定义
#define spi_cs_port SPI1_CS_GPIO_Port
#define spi_cs_pin SPI1_CS_Pin

#define analog_board_rst_port FPGA_NRST_GPIO_Port
#define analog_board_rst_pin FPGA_NRST_Pin

#define interrupt_port FPGA_INT_GPIO_Port
#define interrupt_pin FPGA_INT_Pin

// CRC8计算参数
#define crc_polynomial 0x07
#define crc_init 0x00

// ACK响应定义
#define ack_crc_error 0x08
#define ack_busy_error 0x04
#define ack_addr_error 0x02
#define ack_ok 0x01

// 数据帧结构体定义
struct __packed community_frame {
	uint8_t address;
	uint16_t data;
	uint8_t crc;
	uint8_t ack;
};

#define spi_cs(x) \
do { \
	if (x) HAL_GPIO_WritePin(spi_cs_port, spi_cs_pin, GPIO_PIN_SET); \
	else HAL_GPIO_WritePin(spi_cs_port, spi_cs_pin, GPIO_PIN_RESET); \
} while(0)

void wait_analog_board_ready(void);
uint8_t send_command(uint8_t address, uint16_t command);
uint8_t read_data(uint8_t address, uint16_t data);

#endif
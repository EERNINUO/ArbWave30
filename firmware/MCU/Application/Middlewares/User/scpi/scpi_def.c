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

#include "scpi_def.h"
#include "analog_board_driver.h"
#include "usbd_cdc_if.h"
#include "analog_board_driver.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static char scpi_tx_buffer[SCPI_TX_BUFFER_SIZE]; // SCPI TX缓冲区   

static size_t SCPI_Write(scpi_t * context, const char * data, size_t len)
{
	USBD_CDC_HandleTypeDef *hcdc;
	uint32_t timeout;

	if (len > SCPI_TX_BUFFER_SIZE)  {
		len = SCPI_TX_BUFFER_SIZE;
	}
	memcpy(scpi_tx_buffer, data, len);

	hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
	if (hcdc == NULL)  {
		return len;
	}

	// 等待USB CDC IN端点空闲
	timeout = SCPI_TX_TIMEOUT_MS;
	while ((hcdc->TxState != 0U) && (timeout > 0U))  {
		osDelay(1);
		timeout--;
	}

	if (hcdc->TxState == 0U)  {
		CDC_Transmit_FS((uint8_t *)scpi_tx_buffer, (uint16_t)len);
	}

	return len;
}

static int SCPI_ErrorCallback(scpi_t * context, int_fast16_t error)
{
	(void)context;
	(void)error;
	// 可以在这里添加自定义的错误处理
	return 0;
}

// 设备控制 (可选)
static scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val)
{
	(void)context;
	(void)ctrl;
	(void)val;
	return SCPI_RES_OK;
}

// SCPI_Flush: 刷新输出缓冲区 (可选)
static scpi_result_t SCPI_Flush(scpi_t * context)
{
	(void)context;
	return SCPI_RES_OK;
}

static scpi_result_t ArbWave30_Reset(scpi_t * context)
{
	(void)context;

	analogBoard_hardReset();
	SCPI_ErrorClear(context);

	return SCPI_RES_OK;
}

// scpi 接口函数结构体
static scpi_interface_t scpi_interface =
{
	SCPI_ErrorCallback, /* error   */
	SCPI_Write,         /* write   */
	SCPI_Control,       /* control */
	SCPI_Flush,         /* flush   */
	ArbWave30_Reset     /* reset   */
};


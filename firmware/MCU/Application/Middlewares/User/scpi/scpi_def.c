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

// Assisted-by: DeepSeek - SCPI_Write 函数实现参考

#include "scpi_def.h"
#include "analog_board_driver.h"
#include "usbd_cdc_if.h"
#include "cmsis_os.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static char scpi_rx_buffer[SCPI_RX_BUFFER_SIZE]; // SCPI RX缓冲区
static char scpi_tx_buffer[SCPI_TX_BUFFER_SIZE]; // SCPI TX缓冲区 
static scpi_error_t scpi_error_queue[SCPI_ERROR_QUEUE_SIZE]; 		// SCPI 错误队列

static size_t SCPI_Write(scpi_t * context, const char * data, size_t len)
{
	USBD_CDC_HandleTypeDef *hcdc;
	uint32_t timeout;

	// 检查长度是否超过缓冲区大小，如果超过则截断
	// 虽然截断响应数据在 SCPI 协议中属于异常行为，但在嵌入式场景中，宁可截断长字符串，也绝不能发生缓冲区溢出，否则会发生 HardFault
	// 也可以直接返回 0 来通知 SCPI 库发送失败，未来实现
	if (len > SCPI_TX_BUFFER_SIZE)  {
		len = SCPI_TX_BUFFER_SIZE;
	}
	// 拷贝到自己的缓冲区是为了防止上一级函数在退出后 data 指针失效
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

static const scpi_command_t scpi_commands[] =
{
    // IEEE 488.2 公共命令
    { "*IDN?",                      SCPI_CoreIdnQ,            0 },
    { "*RST",                       ArbWave30_Reset,          0 },
    { "*CLS",                       SCPI_CoreCls,             0 },

    // SYSTem 命令
    { "SYSTem:ERRor?",              SCPI_SystemErrorNextQ,    0 },
    { "SYSTem:ERRor:NEXT?",         SCPI_SystemErrorNextQ,    0 },
    { "SYSTem:VERSion?",            SCPI_SystemVersionQ,      0 },

    SCPI_CMD_LIST_END
};

void ArbWave30_SCPI_Init(scpi_t * context)
{
	ArbWave30_Reset(context);

    SCPI_Init(context,
            scpi_commands,
            &scpi_interface,
            scpi_units_def,
            "EERNINUO", "ArbWave30", "V1.0.0", "SN123456",
            scpi_rx_buffer, sizeof(scpi_rx_buffer),
            scpi_error_queue, (int16_t)(sizeof(scpi_error_queue) / sizeof(scpi_error_queue[0])));
}
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
#include <stdbool.h>

extern USBD_HandleTypeDef hUsbDeviceFS;

static char scpi_rx_buffer[SCPI_RX_BUFFER_SIZE]; // SCPI RX缓冲区
static char scpi_tx_buffer[SCPI_TX_BUFFER_SIZE]; // SCPI TX缓冲区 
static scpi_error_t scpi_error_queue[SCPI_ERROR_QUEUE_SIZE]; 		// SCPI 错误队列

scpi_result_t ArbWave30_Frequency(scpi_t * context);
scpi_result_t ArbWave30_FrequencyQ(scpi_t * context);
scpi_result_t ArbWave30_Function(scpi_t * context);
scpi_result_t ArbWave30_FunctionQ(scpi_t * context);
scpi_result_t ArbWave30_VoltageAmplitude(scpi_t * context);
scpi_result_t ArbWave30_VoltageAmplitudeQ(scpi_t * context);
scpi_result_t ArbWave30_VoltageOffset(scpi_t * context);
scpi_result_t ArbWave30_VoltageOffsetQ(scpi_t * context);
scpi_result_t ArbWave30_OutputState(scpi_t * context);
scpi_result_t ArbWave30_OutputStateQ(scpi_t * context);
scpi_result_t ArbWave30_OutputImpedance(scpi_t * context);
scpi_result_t ArbWave30_OutputImpedanceQ(scpi_t * context);

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
	while ((hcdc->TxState != 0U) && (timeout > 0U)) {
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
static scpi_interface_t scpi_interface = {
	SCPI_ErrorCallback, /* error   */
	SCPI_Write,         /* write   */
	SCPI_Control,       /* control */
	SCPI_Flush,         /* flush   */
	ArbWave30_Reset     /* reset   */
};

static const scpi_command_t scpi_commands[] = {
    // IEEE 488.2 公共命令
    { "*IDN?",                      SCPI_CoreIdnQ,            0 },
    { "*RST",                       ArbWave30_Reset,          0 },
    { "*CLS",                       SCPI_CoreCls,             0 },

    // SYSTem 命令
    { "SYSTem:ERRor?",              SCPI_SystemErrorNextQ,    0 },
    { "SYSTem:ERRor:NEXT?",         SCPI_SystemErrorNextQ,    0 },
    { "SYSTem:VERSion?",            SCPI_SystemVersionQ,      0 },

	// SOURce 命令
	// [SOURce#:]FREQuency
    { "FREQuency",                  ArbWave30_Frequency,      0 },
    { "FREQuency?",                 ArbWave30_FrequencyQ,     0 },
    { "SOURce:FREQuency",           ArbWave30_Frequency,      0 },
    { "SOURce:FREQuency?",          ArbWave30_FrequencyQ,     0 },
    { "SOURce#:FREQuency",          ArbWave30_Frequency,      0 },
    { "SOURce#:FREQuency?",         ArbWave30_FrequencyQ,     0 },

    // [SOURce#:]FUNCtion 
    { "FUNCtion",                   ArbWave30_Function,       0 },
    { "FUNCtion?",                  ArbWave30_FunctionQ,      0 },
    { "SOURce:FUNCtion",            ArbWave30_Function,       0 },
    { "SOURce:FUNCtion?",           ArbWave30_FunctionQ,      0 },
    { "SOURce#:FUNCtion",           ArbWave30_Function,       0 },
    { "SOURce#:FUNCtion?",          ArbWave30_FunctionQ,      0 },

	// [SOURce#:]VOLTage:OFFSet
	{ "VOLTage:OFFSet",             ArbWave30_VoltageOffset,  0 },
	{ "VOLTage:OFFSet?",            ArbWave30_VoltageOffsetQ, 0 },
	{ "SOURce:VOLTage:OFFSet",      ArbWave30_VoltageOffset,  0 },
	{ "SOURce:VOLTage:OFFSet?",     ArbWave30_VoltageOffsetQ, 0 },
	{ "SOURce#:VOLTage:OFFSet",     ArbWave30_VoltageOffset,  0 },

	// [SOURce#:]VOLTage:AMPLitude
	{ "VOLTage:AMPLitude",          ArbWave30_VoltageAmplitude, 		  0 },
	{ "VOLTage:AMPLitude?",         ArbWave30_VoltageAmplitudeQ,		  0 },
	{ "SOURce:VOLTage:AMPLitude",   ArbWave30_VoltageAmplitude, 		  0 },
	{ "SOURce:VOLTage:AMPLitude?",  ArbWave30_VoltageAmplitudeQ,		  0 },
	{ "SOURce#:VOLTage:AMPLitude",  ArbWave30_VoltageAmplitude, 		  0 },
	{ "SOURce#:VOLTage:AMPLitude?", ArbWave30_VoltageAmplitudeQ,		  0 },

    // [SOURce#:]VOLTage 
    { "VOLTage",                    ArbWave30_VoltageAmplitude,        0 },
    { "VOLTage?",                   ArbWave30_VoltageAmplitudeQ,       0 },
    { "SOURce:VOLTage",             ArbWave30_VoltageAmplitude,        0 },
    { "SOURce:VOLTage?",            ArbWave30_VoltageAmplitudeQ,       0 },
    { "SOURce#:VOLTage",            ArbWave30_VoltageAmplitude,        0 },
    { "SOURce#:VOLTage?",           ArbWave30_VoltageAmplitudeQ,       0 },

	/* OUTPut#:STATe */
    { "OUTPut:STATe",               ArbWave30_OutputState,    0 },
    { "OUTPut:STATe?",              ArbWave30_OutputStateQ,   0 },
    { "OUTPut#:STATe",              ArbWave30_OutputState,    0 },
    { "OUTPut#:STATe?",             ArbWave30_OutputStateQ,   0 },

    /* OUTPut#:IMPedance */
    { "OUTPut:IMPedance",           ArbWave30_OutputImpedance, 0 },
	{ "OUTPut:IMPedance?",          ArbWave30_OutputImpedanceQ, 0 },
    { "OUTPut#:IMPedance",          ArbWave30_OutputImpedance, 0 },
    { "OUTPut#:IMPedance?",         ArbWave30_OutputImpedanceQ, 0 },

    SCPI_CMD_LIST_END
};

void ArbWave30_SCPI_Init(scpi_t * context)
{
	ArbWave30_Reset(context);

    SCPI_Init(context,
            scpi_commands,
            &scpi_interface,
            scpi_units_def,
            AUTHOR, MODEL, VERSION, SERIAL_NUMBER,
            scpi_rx_buffer, sizeof(scpi_rx_buffer),
            scpi_error_queue, (int16_t)(sizeof(scpi_error_queue) / sizeof(scpi_error_queue[0])));
}

/**
 * @brief  从命令中获取通道号，支持范围检查
 */
static scpi_bool_t ArbWave30_GetChannel(scpi_t *context, int32_t *channel)
{
	static int32_t numbers = 1;

	SCPI_CommandNumbers(context, &numbers, 1, 1);

	if ((numbers < 1) || (numbers > 2)) {
		SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
		return FALSE;
	}

	*channel = numbers;
	return TRUE;
}

/**
 * @brief  从命令中获取参数，支持范围检查
 * @param min 最小值
 * @param max 最大值
 * @param value 参数值
 */
static scpi_bool_t ArbWave30_ParamNumber(scpi_t *context, double *value, float min, float max)
{
	scpi_number_t param;

	if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, TRUE)) {
		return FALSE;
	}

	// 检查参数是否是特殊值
	if (param.special) {
		switch (param.content.tag) {
		case SCPI_NUM_MIN: *value = min; break;
		case SCPI_NUM_MAX: *value = max; break;
		default:
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return FALSE;
		}
	} else {
		// 解析器本身已经处理了单位转换，所以这里直接取值即可
		*value = param.content.value;
		if ((*value < min) || (*value > max)) {
			SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
			return FALSE;
		}
	}

	return TRUE;
}

/* ---------------------------------------- 
 *  SCPI 功能函数
 * --------------------------------------*/

// SOURce 子系统
/**
 * @brief  [SOURce#:]FREQuency {<frequency>|MIN|MAX|DEFAULT}
 */
scpi_result_t ArbWave30_Frequency(scpi_t *context)
{
	int32_t ch;
	double freq;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	if (!ArbWave30_ParamNumber(context, &freq, FREQ_MIN, FREQ_MAX)) {
		return SCPI_RES_ERR;
	}

	if (analogBoard_setFrequency(ch, (uint64_t)(freq * 1000000)) != ACK_OK) {
		SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_ERROR);
		return SCPI_RES_ERR;
	}
	return SCPI_RES_OK;
}

/**
 * @brief  [SOURce#:]FREQuency? [MIN|MAX]
 */
scpi_result_t ArbWave30_FrequencyQ(scpi_t *context)
{
	int32_t ch;
	scpi_number_t param;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	if (SCPI_ParamNumber(context, scpi_special_numbers_def, &param, FALSE)) {
		if (param.special) {
			if (param.content.tag == SCPI_NUM_MIN) {
				SCPI_ResultDouble(context, 0.0);
				return SCPI_RES_OK;
			} else if (param.content.tag == SCPI_NUM_MAX) {
				SCPI_ResultDouble(context, (double)FREQ_MAX);
				return SCPI_RES_OK;
			} else {
				SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
				return SCPI_RES_ERR;
			}
		} else {
			SCPI_ErrorPush(context, SCPI_ERROR_PARAMETER_NOT_ALLOWED);
			return SCPI_RES_ERR;
		}
	}

	SCPI_ResultDouble(context, analogBoard_getFrequency(ch)/1000000.0);
	return SCPI_RES_OK;
}

// 波形名称
static const scpi_choice_def_t wave_choices[] ={
	{"SINusoid", WAVE_SINE},
	{"SQUare", WAVE_SQUARE},
	{"TRIangle", WAVE_TRIANGLE},
	{"NOISe", WAVE_NOISE},
	{"DC", WAVE_DC},
	SCPI_CHOICE_LIST_END
};

/**
 * @brief  [SOURce#:]FUNCtion {SINusoid|SQUare|TRIangle|SAWtooth|DC}
 */
scpi_result_t ArbWave30_Function(scpi_t *context)
{
	int32_t ch;
	int32_t wave;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	if (!SCPI_ParamChoice(context, wave_choices, &wave, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (analogBoard_setWave(ch, wave) != ACK_OK) {
		SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_ERROR);
		return SCPI_RES_ERR;
	}
	return SCPI_RES_OK;
}

/**
 * @brief  [SOURce#:]FUNCtion?
 */
scpi_result_t ArbWave30_FunctionQ(scpi_t *context)
{
	int32_t ch;
	const char *name;

	if (!ArbWave30_GetChannel(context, &ch)){
		return SCPI_RES_ERR;
	}

	if (SCPI_ChoiceToName(wave_choices, analogBoard_getWave(ch), &name)){
		SCPI_ResultMnemonic(context, name);
	}

	return SCPI_RES_OK;
}

/**
 * @brief  [SOURce#:]VOLTage:OFFSet {<offset>|MIN|MAX|DEFAULT}
 */
scpi_result_t ArbWave30_VoltageOffset(scpi_t *context)
{
	int32_t ch;
	double offset;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	if (!ArbWave30_ParamNumber(context, &offset, VOLT_MIN, VOLT_MAX)) {
		return SCPI_RES_ERR;
	}

	if (analogBoard_setOffset(ch, (uint16_t)(offset * 1000)) != ACK_OK) {
		SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_ERROR);
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}

/**
 * @brief  [SOURce#:]VOLTage:OFFSet?
 */
scpi_result_t ArbWave30_VoltageOffsetQ(scpi_t *context)
{
	int32_t ch;
	scpi_number_t param;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	if (SCPI_ParamNumber(context, scpi_special_numbers_def, &param, FALSE)) {
		if (param.special) {
			if (param.content.tag == SCPI_NUM_MIN) {
				SCPI_ResultFloat(context, (float)VOLT_MIN);
				return SCPI_RES_OK;
			} else if (param.content.tag == SCPI_NUM_MAX) {
				SCPI_ResultFloat(context, (float)VOLT_MAX);
				return SCPI_RES_OK;
			} else {
				SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
				return SCPI_RES_ERR;
			}
		} else {
			SCPI_ErrorPush(context, SCPI_ERROR_PARAMETER_NOT_ALLOWED);
			return SCPI_RES_ERR;
		}
	}

	SCPI_ResultFloat(context, (float)analogBoard_getAmplitude(ch)/1000.0);
	return SCPI_RES_OK;
}

/**
 * @brief  [SOURce#:]VOLTage {<amplitude>|MIN|MAX|DEFAULT}
 */
scpi_result_t ArbWave30_VoltageAmplitude(scpi_t *context)
{
	int32_t ch;
	double amplitude;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	if (!ArbWave30_ParamNumber(context, &amplitude, VOLT_MIN, VOLT_MAX)) {
		return SCPI_RES_ERR;
	}

	if (analogBoard_setAmplitude(ch, (uint16_t)(amplitude * 1000)) != ACK_OK) {
		SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_ERROR);
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}

/**
 * @brief  [SOURce#:]VOLTage? [MIN|MAX]
 */
scpi_result_t ArbWave30_VoltageAmplitudeQ(scpi_t *context)
{
	int32_t ch;
	scpi_number_t param;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	if (SCPI_ParamNumber(context, scpi_special_numbers_def, &param, FALSE)) {
		if (param.special) {
			if (param.content.tag == SCPI_NUM_MIN) {
				SCPI_ResultFloat(context, (float)VOLT_MIN);
				return SCPI_RES_OK;
			} else if (param.content.tag == SCPI_NUM_MAX) {
				SCPI_ResultFloat(context, (float)VOLT_MAX);
				return SCPI_RES_OK;
			} else {
				SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
				return SCPI_RES_ERR;
			}
		} else {
			SCPI_ErrorPush(context, SCPI_ERROR_PARAMETER_NOT_ALLOWED);
			return SCPI_RES_ERR;
		}
	}

	SCPI_ResultFloat(context, (float)analogBoard_getAmplitude(ch)/1000.0);
	return SCPI_RES_OK;
}

// OUTPut 子系统
/**
 * @brief  [OUTPut#:]STATe {ON|OFF}
 */
scpi_result_t ArbWave30_OutputState(scpi_t * context)
{
	int32_t ch;
	scpi_bool_t on;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	if (!SCPI_ParamBool(context, &on, TRUE)){
		return SCPI_RES_ERR;
	}

	if (analogBoard_setEnable(ch, on) != ACK_OK) {
		SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_ERROR);
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}

/**
 * @brief  [OUTPut#:]STATe?
 */
scpi_result_t ArbWave30_OutputStateQ(scpi_t * context)
{
	int32_t ch;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	SCPI_ResultBool(context, analogBoard_getEnable(ch));

	return SCPI_RES_OK;
}

static const scpi_choice_def_t impedance_choices[] = {
    { "50",   false },
    { "HIGH", true  },
    SCPI_CHOICE_LIST_END
};

/**
 * @brief  [OUTPut#:]IMPedance {<impedance>|50|HIGH}
 */
scpi_result_t ArbWave30_OutputImpedance(scpi_t *context)
{
	int32_t ch;
	int32_t imp;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	if (!SCPI_ParamChoice(context, impedance_choices, &imp, TRUE)) {
		return SCPI_RES_ERR;
	}

	analogBoard_setImpedance(ch, imp);
	return SCPI_RES_OK;
}

/**
 * @brief  [OUTPut#:]IMPedance?
 */
scpi_result_t ArbWave30_OutputImpedanceQ(scpi_t * context)
{
	int32_t ch;

	if (!ArbWave30_GetChannel(context, &ch)) {
		return SCPI_RES_ERR;
	}

	SCPI_ResultBool(context, analogBoard_getImpedance(ch));

	return SCPI_RES_OK;
}

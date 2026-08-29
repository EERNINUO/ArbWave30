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

 // Assisted-by: DeepSeek

#include <string.h>
#include "scpi/scpi.h"
#include "cmsis_os.h"
#include "usbd_cdc_if.h"

#define ARBWAVE30_CHANNEL_COUNT  2
#define ARBWAVE30_FREQ_MAX       30000000.0f   /* 30 MHz */
#define ARBWAVE30_AMP_MAX        20.0f         /* 20 Vpp */

typedef enum
{
  ARBWAVE30_WAVE_SINE = 0,
  ARBWAVE30_WAVE_SQUARE,
  ARBWAVE30_WAVE_TRIANGLE,
  ARBWAVE30_WAVE_SAWTOOTH,
  ARBWAVE30_WAVE_DC
} arbwave30_wave_t;

typedef struct
{
  float frequency;            /* Hz  */
  float amplitude;            /* Vpp */
  float offset;               /* V   */
  arbwave30_wave_t wave;
  scpi_bool_t output_on;
  scpi_bool_t impedance_high; /* TRUE = HIGH impedance, FALSE = 50 ohm */
} arbwave30_channel_t;


#define SCPI_TX_BUFFER_SIZE   256
#define SCPI_TX_TIMEOUT_MS    1000

scpi_t scpi_context;

static char scpi_input_buffer[256];
static scpi_error_t scpi_error_queue[8];
static char scpi_tx_buffer[SCPI_TX_BUFFER_SIZE];
static arbwave30_channel_t channels[ARBWAVE30_CHANNEL_COUNT];

extern USBD_HandleTypeDef hUsbDeviceFS;

static size_t SCPI_Write(scpi_t * context, const char * data, size_t len);
static int SCPI_ErrorCallback(scpi_t * context, int_fast16_t error);
static scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val);
static scpi_result_t SCPI_Flush(scpi_t * context);
static scpi_bool_t ArbWave30_GetChannel(scpi_t * context, int32_t * channel);
static scpi_bool_t ArbWave30_ParamNumber(scpi_t * context, float * value, float min, float max);
static void ArbWave30_ResetDefaults(void);

/* Command callbacks (implemented below) */
scpi_result_t ArbWave30_Reset(scpi_t * context);
scpi_result_t ArbWave30_Frequency(scpi_t * context);
scpi_result_t ArbWave30_FrequencyQ(scpi_t * context);
scpi_result_t ArbWave30_Function(scpi_t * context);
scpi_result_t ArbWave30_FunctionQ(scpi_t * context);
scpi_result_t ArbWave30_Voltage(scpi_t * context);
scpi_result_t ArbWave30_VoltageQ(scpi_t * context);
scpi_result_t ArbWave30_OutputState(scpi_t * context);
scpi_result_t ArbWave30_OutputStateQ(scpi_t * context);
scpi_result_t ArbWave30_OutputImpedance(scpi_t * context);

/* ---------------------------------------- 
 *  SCPI 接口函数
 * --------------------------------------*/

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
  return 0;
}

static scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val)
{
  (void)context;
  (void)ctrl;
  (void)val;
  return SCPI_RES_OK;
}

static scpi_result_t SCPI_Flush(scpi_t * context)
{
  (void)context;
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

/* ---------------------------------------- 
 *  SCPI 命令表
 * --------------------------------------*/
static const scpi_command_t scpi_commands[] =
{
  /* IEEE 488.2 common commands */
  { "*IDN?",                      SCPI_CoreIdnQ,            0 },
  { "*RST",                       ArbWave30_Reset,          0 },
  { "*CLS",                       SCPI_CoreCls,             0 },

  /* SYSTem subsystem */
  { "SYSTem:ERRor?",              SCPI_SystemErrorNextQ,    0 },
  { "SYSTem:ERRor:NEXT?",         SCPI_SystemErrorNextQ,    0 },
  { "SYSTem:VERSion?",            SCPI_SystemVersionQ,      0 },

  /* [SOURce#:]FREQuency */
  { "FREQuency",                  ArbWave30_Frequency,      0 },
  { "FREQuency?",                 ArbWave30_FrequencyQ,     0 },
  { "SOURce:FREQuency",           ArbWave30_Frequency,      0 },
  { "SOURce:FREQuency?",          ArbWave30_FrequencyQ,     0 },
  { "SOURce#:FREQuency",          ArbWave30_Frequency,      0 },
  { "SOURce#:FREQuency?",         ArbWave30_FrequencyQ,     0 },

  /* [SOURce#:]FUNCtion */
  { "FUNCtion",                   ArbWave30_Function,       0 },
  { "FUNCtion?",                  ArbWave30_FunctionQ,      0 },
  { "SOURce:FUNCtion",            ArbWave30_Function,       0 },
  { "SOURce:FUNCtion?",           ArbWave30_FunctionQ,      0 },
  { "SOURce#:FUNCtion",           ArbWave30_Function,       0 },
  { "SOURce#:FUNCtion?",          ArbWave30_FunctionQ,      0 },

  /* [SOURce#:]VOLTage */
  { "VOLTage",                    ArbWave30_Voltage,        0 },
  { "VOLTage?",                   ArbWave30_VoltageQ,       0 },
  { "SOURce:VOLTage",             ArbWave30_Voltage,        0 },
  { "SOURce:VOLTage?",            ArbWave30_VoltageQ,       0 },
  { "SOURce#:VOLTage",            ArbWave30_Voltage,        0 },
  { "SOURce#:VOLTage?",           ArbWave30_VoltageQ,       0 },

  /* OUTPut#:STATe */
  { "OUTPut:STATe",               ArbWave30_OutputState,    0 },
  { "OUTPut:STATe?",              ArbWave30_OutputStateQ,   0 },
  { "OUTPut#:STATe",              ArbWave30_OutputState,    0 },
  { "OUTPut#:STATe?",             ArbWave30_OutputStateQ,   0 },

  /* OUTPut#:IMPedance */
  { "OUTPut:IMPedance",           ArbWave30_OutputImpedance, 0 },
  { "OUTPut#:IMPedance",          ArbWave30_OutputImpedance, 0 },

  SCPI_CMD_LIST_END
};

static void ArbWave30_ResetDefaults(void)
{
  int32_t i;

  for (i = 0; i < ARBWAVE30_CHANNEL_COUNT; i++)
  {
    channels[i].frequency      = 0.0f;
    channels[i].amplitude      = 0.0f;
    channels[i].offset         = 0.0f;
    channels[i].wave           = ARBWAVE30_WAVE_SINE;
    channels[i].output_on      = FALSE;
    channels[i].impedance_high = FALSE;
  }
}

/**
 * @brief  从命令中获取通道号，支持范围检查
 */
static scpi_bool_t ArbWave30_GetChannel(scpi_t * context, int32_t * channel)
{
  int32_t numbers[1];

  numbers[0] = 1;
  SCPI_CommandNumbers(context, numbers, 1, 1);

  if ((numbers[0] < 1) || (numbers[0] > ARBWAVE30_CHANNEL_COUNT)) {
    SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
    return FALSE;
  }

  *channel = numbers[0];
  return TRUE;
}

/**
 * @brief  从命令中获取参数，支持范围检查
 * @param min 最小值
 * @param max 最大值
 * @param value 参数值
 */
static scpi_bool_t ArbWave30_ParamNumber(scpi_t * context, float * value, float min, float max)
{
  scpi_number_t param;

  if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, TRUE)) {
    return FALSE;
  }

  if (param.special) {
    switch (param.content.tag) {
      case SCPI_NUM_MIN:
        *value = min;
        break;
      case SCPI_NUM_MAX:
        *value = max;
        break;
      case SCPI_NUM_DEF:
        *value = 0.0f;
        break;
      default:
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return FALSE;
    }
  } else {
    *value = (float)param.content.value;
    if ((*value < min) || (*value > max)) {
      SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
      return FALSE;
    }
  }

  return TRUE;
}

/**
  * @brief  *RST - reset all device parameters to factory defaults.
  */
scpi_result_t ArbWave30_Reset(scpi_t * context)
{
  ArbWave30_ResetDefaults();
  SCPI_ErrorClear(context);
  return SCPI_RES_OK;
}

/* ---------------------------------------- 
 *  SCPI 功能函数
 * --------------------------------------*/

// SOURce 子系统回调函数

/**
  * @brief  [SOURce#:]FREQuency {<frequency>|MIN|MAX|DEFAULT}
  */
scpi_result_t ArbWave30_Frequency(scpi_t * context)
{
  int32_t ch;
  float freq;

  if (!ArbWave30_GetChannel(context, &ch)){
    return SCPI_RES_ERR;
  }

  if (!ArbWave30_ParamNumber(context, &freq, 0.0f, ARBWAVE30_FREQ_MAX)) {
    return SCPI_RES_ERR;
  }

  channels[ch - 1].frequency = freq;
  return SCPI_RES_OK;
}

/**
  * @brief  [SOURce#:]FREQuency? [MIN|MAX]
  */
scpi_result_t ArbWave30_FrequencyQ(scpi_t * context)
{
  int32_t ch;
  scpi_number_t param;

  if (!ArbWave30_GetChannel(context, &ch))
  {
    return SCPI_RES_ERR;
  }

  if (SCPI_ParamNumber(context, scpi_special_numbers_def, &param, FALSE))
  {
    if (param.special)
    {
      if (param.content.tag == SCPI_NUM_MIN)
      {
        SCPI_ResultDouble(context, 0.0);
        return SCPI_RES_OK;
      }
      else if (param.content.tag == SCPI_NUM_MAX)
      {
        SCPI_ResultDouble(context, (double)ARBWAVE30_FREQ_MAX);
        return SCPI_RES_OK;
      }
      else
      {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
      }
    }
    else
    {
      SCPI_ErrorPush(context, SCPI_ERROR_PARAMETER_NOT_ALLOWED);
      return SCPI_RES_ERR;
    }
  }

  SCPI_ResultDouble(context, (double)channels[ch - 1].frequency);
  return SCPI_RES_OK;
}

// 波形名称
static const scpi_choice_def_t wave_choices[] =
{
  { "SINusoid",  ARBWAVE30_WAVE_SINE },
  { "SQUare",    ARBWAVE30_WAVE_SQUARE },
  { "TRIangle",  ARBWAVE30_WAVE_TRIANGLE },
  { "SAWtooth",  ARBWAVE30_WAVE_SAWTOOTH },
  { "DC",        ARBWAVE30_WAVE_DC },
  SCPI_CHOICE_LIST_END
};

/**
  * @brief  [SOURce#:]FUNCtion {SINusoid|SQUare|TRIangle|SAWtooth|DC}
  */
scpi_result_t ArbWave30_Function(scpi_t * context)
{
  int32_t ch;
  int32_t wave;

  if (!ArbWave30_GetChannel(context, &ch))
  {
    return SCPI_RES_ERR;
  }

  if (!SCPI_ParamChoice(context, wave_choices, &wave, TRUE))
  {
    return SCPI_RES_ERR;
  }

  channels[ch - 1].wave = (arbwave30_wave_t)wave;
  return SCPI_RES_OK;
}

/**
  * @brief  [SOURce#:]FUNCtion?
  */
scpi_result_t ArbWave30_FunctionQ(scpi_t * context)
{
  int32_t ch;
  const char * name;

  if (!ArbWave30_GetChannel(context, &ch))
  {
    return SCPI_RES_ERR;
  }

  if (SCPI_ChoiceToName(wave_choices, (int32_t)channels[ch - 1].wave, &name))
  {
    SCPI_ResultMnemonic(context, name);
  }

  return SCPI_RES_OK;
}

/**
  * @brief  [SOURce#:]VOLTage {<amplitude>|MIN|MAX|DEFAULT}
  */
scpi_result_t ArbWave30_Voltage(scpi_t * context)
{
  int32_t ch;
  float amplitude;

  if (!ArbWave30_GetChannel(context, &ch))
  {
    return SCPI_RES_ERR;
  }

  if (!ArbWave30_ParamNumber(context, &amplitude, 0.0f, ARBWAVE30_AMP_MAX))
  {
    return SCPI_RES_ERR;
  }

  channels[ch - 1].amplitude = amplitude;
  return SCPI_RES_OK;
}

/**
  * @brief  [SOURce#:]VOLTage? [MIN|MAX]
  */
scpi_result_t ArbWave30_VoltageQ(scpi_t * context)
{
  int32_t ch;
  scpi_number_t param;

  if (!ArbWave30_GetChannel(context, &ch))
  {
    return SCPI_RES_ERR;
  }

  if (SCPI_ParamNumber(context, scpi_special_numbers_def, &param, FALSE))
  {
    if (param.special)
    {
      if (param.content.tag == SCPI_NUM_MIN)
      {
        SCPI_ResultDouble(context, 0.0);
        return SCPI_RES_OK;
      }
      else if (param.content.tag == SCPI_NUM_MAX)
      {
        SCPI_ResultDouble(context, (double)ARBWAVE30_AMP_MAX);
        return SCPI_RES_OK;
      }
      else
      {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
      }
    }
    else
    {
      SCPI_ErrorPush(context, SCPI_ERROR_PARAMETER_NOT_ALLOWED);
      return SCPI_RES_ERR;
    }
  }

  SCPI_ResultDouble(context, (double)channels[ch - 1].amplitude);
  return SCPI_RES_OK;
}

// OUTPut 子系统回调函数

/**
  * @brief  OUTPut#:STATe {ON|OFF|1|0}
  */
scpi_result_t ArbWave30_OutputState(scpi_t * context)
{
  int32_t ch;
  scpi_bool_t on;

  if (!ArbWave30_GetChannel(context, &ch))
  {
    return SCPI_RES_ERR;
  }

  if (!SCPI_ParamBool(context, &on, TRUE))
  {
    return SCPI_RES_ERR;
  }

  channels[ch - 1].output_on = on;
  return SCPI_RES_OK;
}

/**
  * @brief  OUTPut#:STATe?
  */
scpi_result_t ArbWave30_OutputStateQ(scpi_t * context)
{
  int32_t ch;

  if (!ArbWave30_GetChannel(context, &ch))
  {
    return SCPI_RES_ERR;
  }

  SCPI_ResultBool(context, channels[ch - 1].output_on);
  return SCPI_RES_OK;
}

static const scpi_choice_def_t impedance_choices[] =
{
  { "50",   0 },
  { "HIGH", 1 },
  { "INF",  1 },
  SCPI_CHOICE_LIST_END
};

/**
  * @brief  OUTPut#:IMPedance {50|HIGH}
  */
scpi_result_t ArbWave30_OutputImpedance(scpi_t * context)
{
  int32_t ch;
  int32_t imp;

  if (!ArbWave30_GetChannel(context, &ch))
  {
    return SCPI_RES_ERR;
  }

  if (!SCPI_ParamChoice(context, impedance_choices, &imp, TRUE))
  {
    return SCPI_RES_ERR;
  }

  channels[ch - 1].impedance_high = (imp != 0);
  return SCPI_RES_OK;
}

/**
  * @brief  初始化SCPI解析器上下文和设备模型。
  * @note   在MX_USB_DEVICE_Init()之后从scpi_task中调用一次。
  */
void ArbWave30_SCPI_Init(void)
{
  ArbWave30_ResetDefaults();

  SCPI_Init(&scpi_context,
            scpi_commands,
            &scpi_interface,
            scpi_units_def,
            "EERNINUO", "ArbWave30", "V1.0.0", "SN123456",
            scpi_input_buffer, sizeof(scpi_input_buffer),
            scpi_error_queue, (int16_t)(sizeof(scpi_error_queue) / sizeof(scpi_error_queue[0])));
}


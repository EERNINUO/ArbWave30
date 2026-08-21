/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

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

 // Assisted-by: DeepSeek - scpi_Task 具体实现

/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "scpi_def.h"
#include "usbd_core.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE END Variables */
/* Definitions for scpi_task */
osThreadId_t scpi_taskHandle;
const osThreadAttr_t scpi_task_attributes = {
  .name = "scpi_task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for USB_ReceiveChar */
osMessageQueueId_t USB_ReceiveCharHandle;
const osMessageQueueAttr_t USB_ReceiveChar_attributes = {
  .name = "USB_ReceiveChar"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void scpi_Task(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of USB_ReceiveChar */
  USB_ReceiveCharHandle = osMessageQueueNew (256, sizeof(char), &USB_ReceiveChar_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of scpi_task */
  scpi_taskHandle = osThreadNew(scpi_Task, NULL, &scpi_task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_scpi_Task */
/**
  * @brief  Function implementing the scpi_task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_scpi_Task */
void scpi_Task(void *argument)
{
  /* init code for USB_DEVICE */
  /* USER CODE BEGIN scpi_Task */
  // 当在 CubeMX 中 Generate 后，上面会有一条 `MX_USB_DEVICE_Init()` 调用，请手动删掉它
  char ch;
  uint32_t flags;
  static scpi_bool_t usb_started = FALSE;
  static scpi_bool_t usb_initialized = FALSE;

  // 初始化 SCPI
  ArbWave30_SCPI_Init();

  /* 无限循环
    - USB 接收到的字符被消费并输入到 SCPI 解析器中。
      当队列为空时，50 ms 的队列超时也作为 VBUS 轮询周期。
    - VBUS_Check (PA9, 高电平 = 连接电缆) 控制USB设备：
      连接时启用 (USBD_Start / 第一次调用 MX_USB_DEVICE_Init),
      断开连接时禁用 (USBD_Stop)。
  */
  for(;;) {
    if (osMessageQueueGet(USB_ReceiveCharHandle, &ch, NULL, 50) == osOK) {
      SCPI_Input(&scpi_context, &ch, 1);
    }

    // 从 EXTI9_5 获取 VBUS 边沿事件（检测 USB 线缆插入/拔出） -> 短延迟进行去抖动
    flags = osThreadFlagsWait(VBUS_EVENT_FLAG, osFlagsWaitAny, 0U);
    if ((flags & VBUS_EVENT_FLAG) != 0U) {
      osDelay(20);  // 去抖动延时
      
      // 启用/禁用 USB
      if (HAL_GPIO_ReadPin(VBUS_Check_GPIO_Port, VBUS_Check_Pin) == GPIO_PIN_SET) {
        if (!usb_started) {
          if (usb_initialized) {
            USBD_Start(&hUsbDeviceFS);      /* re-connect after unplug */
          } else {
            MX_USB_DEVICE_Init();                 /* first cable connection */
            usb_initialized = TRUE;
          }
          usb_started = TRUE;
        }
      } else {
        if (usb_started) {
          USBD_Stop(&hUsbDeviceFS);         /* cable disconnected */ 
          usb_started = FALSE;
        }
      }
    }
  }
  /* USER CODE END scpi_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


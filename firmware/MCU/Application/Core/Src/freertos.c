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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
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
/* Definitions for scpi_Task */
osThreadId_t scpi_TaskHandle;
const osThreadAttr_t scpi_Task_attributes = {
  .name = "scpi_Task",
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

void start_scpiTask(void *argument);

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
  /* creation of scpi_Task */
  scpi_TaskHandle = osThreadNew(start_scpiTask, NULL, &scpi_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_start_scpiTask */
/**
  * @brief  Function implementing the scpi_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_start_scpiTask */
void start_scpiTask(void *argument)
{
  /* init code for USB_DEVICE */
  /* USER CODE BEGIN start_scpiTask */
  // 当在 CubeMX 中 Generate 后，上面会有一条 `MX_USB_DEVICE_Init()` 调用，请手动删掉它

  uint32_t flags;                       // 用于存储从 osThreadFlagsWait 获取的标志位

  static bool usb_started = false;      // 记录 USB 是否已经启动
  static bool usb_initialized = false;  // 记录 USB 是否已经初始化
  /* Infinite loop */
  for(;;) {
    // 从 EXTI9_5 获取 VBUS 边沿事件（检测 USB 线缆插入/拔出） -> 短延迟进行去抖动
    flags = osThreadFlagsWait(VBUS_EVENT_FLAG, osFlagsWaitAny, 0U);
    if ((flags & VBUS_EVENT_FLAG) != 0U) {
      osDelay(20);  // 去抖动延时
      
      // 启用/禁用 USB
      if (HAL_GPIO_ReadPin(VBUS_Check_GPIO_Port, VBUS_Check_Pin) == GPIO_PIN_SET) {
        if (!usb_started) {
          if (usb_initialized) {
            USBD_Start(&hUsbDeviceFS);      // 重新连接
          } else {
            MX_USB_DEVICE_Init();           // 第一次连接，初始化 USB
            usb_initialized = true;
          }
          usb_started = true;
        }
      } else {
        if (usb_started) {
          USBD_Stop(&hUsbDeviceFS);         // 断开连接
          usb_started = false;
        }
      }
    }
  }
  /* USER CODE END start_scpiTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


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
#include "app_config.h"
#include "app_rtos.h"
#include  "app_task.h"
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

/* USER CODE END Variables */
/* Definitions for CollectTask */
osThreadId_t CollectTaskHandle;
const osThreadAttr_t CollectTask_attributes = {
  .name = "CollectTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ShowTask */
osThreadId_t ShowTaskHandle;
const osThreadAttr_t ShowTask_attributes = {
  .name = "ShowTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for SerialTask */
osThreadId_t SerialTaskHandle;
const osThreadAttr_t SerialTask_attributes = {
  .name = "SerialTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for AlarmTask */
osThreadId_t AlarmTaskHandle;
const osThreadAttr_t AlarmTask_attributes = {
  .name = "AlarmTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for KeyTask */
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LogQueue */
osMessageQueueId_t LogQueueHandle;
const osMessageQueueAttr_t LogQueue_attributes = {
  .name = "LogQueue"
};
/* Definitions for CollectTimer */
osTimerId_t CollectTimerHandle;
const osTimerAttr_t CollectTimer_attributes = {
  .name = "CollectTimer"
};
/* Definitions for myMutex01 */
osMutexId_t myMutex01Handle;
const osMutexAttr_t myMutex01_attributes = {
  .name = "myMutex01"
};
/* Definitions for ADCBinarySem */
osSemaphoreId_t ADCBinarySemHandle;
const osSemaphoreAttr_t ADCBinarySem_attributes = {
  .name = "ADCBinarySem"
};
/* Definitions for AlarmEvent */
osEventFlagsId_t AlarmEventHandle;
const osEventFlagsAttr_t AlarmEvent_attributes = {
  .name = "AlarmEvent"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartCollectTask(void *argument);
void StartShowTask(void *argument);
void StartSerialTask(void *argument);
void StartAlarmTask(void *argument);
void StartKeyTask(void *argument);
void Callback01(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of myMutex01 */
  myMutex01Handle = osMutexNew(&myMutex01_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of ADCBinarySem */
  ADCBinarySemHandle = osSemaphoreNew(1, 0, &ADCBinarySem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of CollectTimer */
  CollectTimerHandle = osTimerNew(Callback01, osTimerPeriodic, NULL, &CollectTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of LogQueue */
  LogQueueHandle = osMessageQueueNew (4, sizeof(LogData_t), &LogQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of CollectTask */
  CollectTaskHandle = osThreadNew(StartCollectTask, NULL, &CollectTask_attributes);

  /* creation of ShowTask */
  ShowTaskHandle = osThreadNew(StartShowTask, NULL, &ShowTask_attributes);

  /* creation of SerialTask */
  SerialTaskHandle = osThreadNew(StartSerialTask, NULL, &SerialTask_attributes);

  /* creation of AlarmTask */
  AlarmTaskHandle = osThreadNew(StartAlarmTask, NULL, &AlarmTask_attributes);

  /* creation of KeyTask */
  KeyTaskHandle = osThreadNew(StartKeyTask, NULL, &KeyTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* creation of AlarmEvent */
  AlarmEventHandle = osEventFlagsNew(&AlarmEvent_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartCollectTask */
/**
  * @brief  Function implementing the CollectTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartCollectTask */
void StartCollectTask(void *argument)
{
  /* USER CODE BEGIN StartCollectTask */
  /* Infinite loop */
	for(;;)
	{
		osThreadSuspend(osThreadGetId());		
	}
	
	/* USER CODE END StartCollectTask */
}

/* USER CODE BEGIN Header_StartShowTask */
/**
* @brief Function implementing the ShowTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartShowTask */
void StartShowTask(void *argument)
{
  /* USER CODE BEGIN StartShowTask */
  /* Infinite loop */
  for(;;)
  {
		osThreadSuspend(osThreadGetId());		
  }
  /* USER CODE END StartShowTask */
}

/* USER CODE BEGIN Header_StartSerialTask */
/**
* @brief Function implementing the SerialTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSerialTask */
void StartSerialTask(void *argument)
{
  /* USER CODE BEGIN StartSerialTask */
  /* Infinite loop */
	
	SerialTask_Entry(argument);
	
  /* USER CODE END StartSerialTask */
}

/* USER CODE BEGIN Header_StartAlarmTask */
/**
* @brief Function implementing the AlarmTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartAlarmTask */
void StartAlarmTask(void *argument)
{
  /* USER CODE BEGIN StartAlarmTask */
  /* Infinite loop */
  for(;;)
  {
		osThreadSuspend(osThreadGetId());		
  }
  /* USER CODE END StartAlarmTask */
}

/* USER CODE BEGIN Header_StartKeyTask */
/**
* @brief Function implementing the KeyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartKeyTask */
void StartKeyTask(void *argument)
{
  /* USER CODE BEGIN StartKeyTask */
	
	KeyTask_Entry(argument);
	
	
	/* Infinite loop */
  /* USER CODE END StartKeyTask */
}

/* Callback01 function */
void Callback01(void *argument)
{
  /* USER CODE BEGIN Callback01 */

  /* USER CODE END Callback01 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */


#ifndef __APP_RTOS_H__
#define __APP_RTOS_H__

#include "cmsis_os.h"

extern osThreadId_t   SerialTaskHandle;
extern osThreadId_t   KeyTaskHandle;
extern osThreadId_t   CollectTaskHandle;
extern osThreadId_t   ShowTaskHandle;
extern osThreadId_t   AlarmTaskHandle;
extern osMutexId_t    myMutex01Handle;
extern osSemaphoreId_t ADCBinarySemHandle;
extern osTimerId_t    CollectTimerHandle;
extern osMessageQueueId_t LogQueueHandle;
extern osEventFlagsId_t  AlarmEventHandle;


#endif /* __APP_RTOS_H__ */


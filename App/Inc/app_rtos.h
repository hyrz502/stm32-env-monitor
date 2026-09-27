#ifndef __APP_RTOS_H__
#define __APP_RTOS_H__

#include "cmsis_os.h"

extern osThreadId_t SerialTaskHandle;
extern osThreadId_t KeyTaskHandle;
extern osThreadId_t CollectTaskHandle;

#endif

#ifndef __APP_TASK_H__
#define __APP_TASK_H__

#include "main.h"
#include "cmsis_os.h"

void SerialTask_Entry(void *argument);
void KeyTask_Entry(void* argument);


#endif /* __APP_TASK_H__ */

#ifndef __APP_TASK_H__
#define __APP_TASK_H__

#include "main.h"
#include "cmsis_os.h"
#include "task.h"   /* ulTaskNotifyTake 等 */

void CollectTask_Entry(void *argument);
void DisplayTask_Entry(void *argument);
void SerialTask_Entry(void *argument);
void AlarmTask_Entry(void *argument);
void KeyTask_Entry(void *argument);


#endif /* __APP_TASK_H__ */


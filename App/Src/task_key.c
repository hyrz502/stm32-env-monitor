#include "app_task.h"
#include "bsp_gpio.h"

void KeyTask_Entry(void* argument)
{
	(void)argument;
	
	for(;;)
	{
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		
		osDelay(20);
	
	}
}

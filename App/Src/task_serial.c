#include "app_data.h"
#include "app_task.h"
#include "bsp_uart.h"

void SerialTask_Entry(void *argument)
{
	(void)argument;
	bsp_uart_rxStart();
	for(;;)
	{
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		
		
		osDelay(20);
	
	}
}

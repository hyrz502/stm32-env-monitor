#include "app_data.h"
#include "app_task.h"
#include "bsp_uart.h"
#include "app_rtos.h"
#include <string.h>

void SerialTask_Entry(void *argument)
{
	(void)argument;
	rb_init(rb,shortage,BUFFER_MAXSIZE);
	bsp_uart_rxStart(dma_rx_buf,sizeof(dma_rx_buf));
	for(;;)
	{
		ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
		
		uint8_t local_buf[64];
		uint32_t n;
		while((n=rb_available(rb))>0U)
		{
			if(n>sizeof(local_buf)) n=sizeof(local_buf);
			n=rb_read_block(rb,local_buf,n);
			if(n==0U) break;
			bsp_uart_transmit(local_buf,n,200);
		}
	}
}

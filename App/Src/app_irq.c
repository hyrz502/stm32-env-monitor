#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "Freertos.h"
#include "app_data.h"
#include "app_rtos.h"
#include "bsp_uart.h"
#include "task.h"

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart->Instance!=USART1||Size<=0U) return;
	
	rb_write_block(rb,dma_rx_buf,Size);
	
	bsp_uart_rxStart(dma_rx_buf,sizeof(dma_rx_buf));
	
	BaseType_t HigherPriorityTaskWoken=pdFALSE;
	vTaskNotifyGiveFromISR(SerialTaskHandle,&HigherPriorityTaskWoken);
	portYIELD_FROM_ISR(HigherPriorityTaskWoken);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin!=Key1_Pin&&GPIO_Pin!=Key2_Pin) return;
	
	BaseType_t HigherPriorityTaskWoken=pdFALSE;
	vTaskNotifyGiveFromISR(KeyTaskHandle,&HigherPriorityTaskWoken);
	portYIELD_FROM_ISR(HigherPriorityTaskWoken);
}

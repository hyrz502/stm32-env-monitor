#include "app_task.h"
#include "app_data.h"
#include "app_rtos.h"
#include "app_config.h"
#include "bsp_gpio.h"

/**
  * @brief  报警任务：当 [温度超阈值] 与 [自动报警开启] 两事件同时成立时，
  *         LED 进入闪烁；否则熄灭。
  */
void AlarmTask_Entry(void *argument)
{
	(void)argument;

	for(;;)
	{
		uint32_t ret = osEventFlagsWait(AlarmEventHandle,
		                                BIT_TEMP_OVER | BIT_AUTO_ALARM,
		                                osFlagsWaitAll | osFlagsNoClear,
		                                300);

		uint8_t led;
		osMutexAcquire(myMutex01Handle, osWaitForever);
		if((ret & (BIT_TEMP_OVER | BIT_AUTO_ALARM)) == (BIT_TEMP_OVER | BIT_AUTO_ALARM))
		{
			g_led_flag = LED_MODE_BLINK;
		}
		else
		{
			g_led_flag = LED_MODE_OFF;
		}
		led = g_led_flag;
		osMutexRelease(myMutex01Handle);

		if(led == LED_MODE_BLINK)
		{
			bsp_gpio_toggle(LED_GPIO_Port, LED_Pin);
		}
		else
		{
			bsp_gpio_write(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		}
		osDelay(300);
	}
}


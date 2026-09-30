#include "app_task.h"
#include "app_data.h"
#include "app_rtos.h"
#include "app_config.h"
#include "bsp_uart.h"
#include "bsp_dht11.h"
#include "bsp_gpio.h"
#include "adc.h"
#include "dma.h"
#include <string.h>

/**
  * @brief  采集任务：由 CollectTimer 周期通知唤醒。
  *         依次采样光照 ADC / 温湿度，更新共享数据，写历史 log，
  *         若开启自动报警则上报当前状态帧。
  */
void CollectTask_Entry(void *argument)
{
	(void)argument;
	uint8_t  local_period  = 1;            /* 记录已生效的采集周期，用于外设重定时 */
	uint16_t Log[3][10]    = {0};          /* 历史数据缓冲 */
	uint8_t  times         = 0;
	uint8_t  sendbuf[64];

	/* 启动周期定时器（默认 1000ms，周期变化后再按其调整） */
	osTimerStart(CollectTimerHandle, 1000);

	for(;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   /* 等待定时器通知 */

		osMutexAcquire(myMutex01Handle, osWaitForever);

		/* 采集周期变化 -> 重设定时器 */
		if(local_period != g_period)
		{
			osTimerStop(CollectTimerHandle);
			osTimerStart(CollectTimerHandle, 1000u * g_period);
			local_period = g_period;
		}

		/* 历史数据移位滚动 */
		if(times >= 10)
		{
			for(uint8_t i=0; i<9; i++)
			{
				Log[0][i] = Log[0][i+1];
				Log[1][i] = Log[1][i+1];
				Log[2][i] = Log[2][i+1];
			}
			times = 9;
		}

		if(g_led_flag == LED_MODE_OFF)            /* LED 关闭模式下，保持熄灭 */
		{
			bsp_gpio_write(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		}

		/* 光照 ADC 采样 */
		osSemaphoreAcquire(ADCBinarySemHandle, 0);          /* 预清除信号量 */
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&g_light_raw, 1);
		osSemaphoreAcquire(ADCBinarySemHandle, osWaitForever);
		HAL_ADC_Stop_DMA(&hadc1);

		/* 温湿度采样 */
		Bsp_Dht11Sample(&g_temp, &g_humi);

		/* 在锁内快照所需参数并更新业务标志 */
		uint8_t  temp = g_temp;
		uint8_t  humi = g_humi;
		uint8_t  thr  = g_threshold;
		uint8_t  led  = g_led_flag;
		uint8_t  alarm= g_auto_alarm;
		uint16_t volt = (uint16_t)(g_light_raw * 3300U / 4095U);

		if(temp >= thr)  osEventFlagsSet(AlarmEventHandle, BIT_TEMP_OVER);
		else             osEventFlagsClear(AlarmEventHandle, BIT_TEMP_OVER);

		Log[0][times] = temp;
		Log[1][times] = humi;
		Log[2][times] = volt;

		osMutexRelease(myMutex01Handle);

		/* 自动报警开启时，上报当前状态帧（锁外发送，避免阻塞其他任务） */
		if(alarm == AUTO_ALARM_ON)
		{
			memset(sendbuf, 0, sizeof(sendbuf));
			if(led == LED_MODE_BLINK && temp < thr)
			{
				sprintf((char*)sendbuf, "[CLEAR] Temp:%dC Threshold:%dC\r\n", temp, thr);
			}
			else if(temp < thr)
			{
				sprintf((char*)sendbuf, "[NORMAL] Temp:%dC Threshold:%dC\r\n", temp, thr);
			}
			else
			{
				sprintf((char*)sendbuf, "[ALARM] Temp:%dC Threshold:%dC\r\n", temp, thr);
			}
			bsp_uart_transmit(sendbuf, (uint32_t)strlen((char*)sendbuf), 200);
		}

		/* 最新一帧历史入队 */
		osMessageQueuePut(LogQueueHandle, Log, 0, 0);
		times++;
	}
}


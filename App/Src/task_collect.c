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
	AppDataSnapshot_t snap;

	/* 启动周期定时器（默认 1000ms，周期变化后再按其调整） */
	osTimerStart(CollectTimerHandle, 1000);

	for(;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   /* 等待定时器通知 */

		/* 一次加锁取回配置快照，后续比较与上报均基于同一时刻数据 */
		App_Data_GetSnapshot(&snap);

		/* 采集周期变化 -> 重设定时器 */
		if(local_period != snap.period)
		{
			osTimerStop(CollectTimerHandle);
			osTimerStart(CollectTimerHandle, 1000u * snap.period);
			local_period = snap.period;
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

		if(snap.led_flag == LED_MODE_OFF)            /* LED 关闭模式下，保持熄灭 */
		{
			bsp_gpio_write(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		}

		/* 光照 ADC 采样（DMA 直接写入局部变量，无需加锁） */
		uint16_t light_raw = 0;
		osSemaphoreAcquire(ADCBinarySemHandle, 0);          /* 预清除信号量 */
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&light_raw, 1);
		osSemaphoreAcquire(ADCBinarySemHandle, osWaitForever);
		HAL_ADC_Stop_DMA(&hadc1);

		/* 温湿度采样（阻塞读取，放在锁外，避免长锁阻塞其他任务） */
		uint8_t temp = 0, humi = 0;
		Bsp_Dht11Sample(&temp, &humi);

		/* 采样结果一次性发布：内部短锁更新三个共享变量 */
		App_Data_PublishSensor(temp, humi, light_raw);

		/* 依据最新采样与快照阈值更新报警事件位 */
		uint16_t volt = (uint16_t)(light_raw * 3300U / 4095U);

		if(temp >= snap.threshold)  osEventFlagsSet(AlarmEventHandle, BIT_TEMP_OVER);
		else                        osEventFlagsClear(AlarmEventHandle, BIT_TEMP_OVER);

		Log[0][times] = temp;
		Log[1][times] = humi;
		Log[2][times] = volt;

		/* 自动报警开启时，上报当前状态帧（锁外发送，避免阻塞其他任务） */
		if(snap.auto_alarm == AUTO_ALARM_ON)
		{
			memset(sendbuf, 0, sizeof(sendbuf));
			if(snap.led_flag == LED_MODE_BLINK && temp < snap.threshold)
			{
				sprintf((char*)sendbuf, "[CLEAR] Temp:%dC Threshold:%dC\r\n", temp, snap.threshold);
			}
			else if(temp < snap.threshold)
			{
				sprintf((char*)sendbuf, "[NORMAL] Temp:%dC Threshold:%dC\r\n", temp, snap.threshold);
			}
			else
			{
				sprintf((char*)sendbuf, "[ALARM] Temp:%dC Threshold:%dC\r\n", temp, snap.threshold);
			}
			bsp_uart_transmit(sendbuf, (uint32_t)strlen((char*)sendbuf), 200);
		}

		/* 最新一帧历史入队 */
		osMessageQueuePut(LogQueueHandle, Log, 0, 0);
		times++;
	}
}


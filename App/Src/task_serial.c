/***
*   @note: 串口助手{波特率：15200，文本模式接收，hex协议帧发送}
**/

#include "app_task.h"
#include "app_data.h"
#include "app_rtos.h"
#include "app_config.h"
#include "bsp_uart.h"
#include "bsp_gpio.h"
#include <string.h>

/*
 * 帧格式: [0xAA][LEN][CMD][DATA...][CRC][0x55]
 *        CRC = CMD..DATA 的异或（不含头尾）
 * 上位机协议与 V0 保持一致。
 */

/* ---- 异或校验：对 CMD..CRC 前的 DATA 求异或 ---- */
static uint8_t App_FrameXor(const uint8_t *pData, uint16_t len)
{
	uint8_t xorv = 0;
	for(uint16_t i = 2; i < len - 2; i++)   /* i 从 CMD(2) 到 CRC 前 */
	{
		xorv ^= pData[i];
	}
	return xorv;
}

/* ---- 帧首尾与校验检查 ---- */
static uint8_t App_FrameValid(const uint8_t *pData, uint16_t len)
{
	if(len < 5) return 0;                       /* 至少 [AA][LEN][CMD][CRC][55] */
	if(pData[0] != FRAME_H || pData[len-1] != FRAME_T) return 0;
	if(pData[len-2] != App_FrameXor(pData, len)) return 0;
	return 1;
}

/* ---- 处理一条完整命令帧 ---- */
static void App_SerialProcess(const uint8_t *frame, uint16_t len)
{
	uint8_t sendbuf[64];
	char *ack;

	switch(frame[2])   /* CMD */
	{
		case CMD_GET_CURRENT:   /* 请求当前数据 */
		{
			uint16_t volt;
			osMutexAcquire(myMutex01Handle, osWaitForever);
			volt = (uint16_t)(g_light_raw * 3300U / 4095U);
			sprintf((char*)sendbuf, "Temp:%dC Light:%dmv Threshold:%dC Interval:%ds\r\n",
			        g_temp, volt, g_threshold, g_period);
			osMutexRelease(myMutex01Handle);
			bsp_uart_transmit(sendbuf, (uint32_t)strlen((char*)sendbuf), 200);
		}
		break;

		case CMD_SET_INTERVAL:   /* 设置采样间隔(1~60s) */
			if(frame[3] >= 1 && frame[3] <= 60)
			{
				osMutexAcquire(myMutex01Handle, osWaitForever);
				g_period = frame[3];
				osMutexRelease(myMutex01Handle);
				ack = "OK\r\n";
			}
			else ack = "ERR\r\n";
			bsp_uart_transmit((uint8_t*)ack, (uint32_t)strlen(ack), 200);
			break;

		case CMD_SET_THRESHOLD:  /* 设置报警阈值(1~127) */
			if(frame[3] >= 1 && frame[3] <= 127)
			{
				osMutexAcquire(myMutex01Handle, osWaitForever);
				g_threshold = frame[3];
				osMutexRelease(myMutex01Handle);
				ack = "OK\r\n";
			}
			else ack = "ERR\r\n";
			bsp_uart_transmit((uint8_t*)ack, (uint32_t)strlen(ack), 200);
			break;

		case CMD_SET_LED:   /* 控制 LED: 0灭 1亮 2闪 */
			osMutexAcquire(myMutex01Handle, osWaitForever);
			if(frame[3] == 0)
			{
				g_led_flag = LED_MODE_OFF;
				bsp_gpio_write(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
			}
			else if(frame[3] == 1)
			{
				g_led_flag = LED_MODE_ON;
				bsp_gpio_write(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
			}
			else if(frame[3] == 2)
			{
				g_led_flag = LED_MODE_BLINK;
			}
			osMutexRelease(myMutex01Handle);
			break;

		case CMD_GET_HISTORY:   /* 请求历史数据 */
		{
			LogData_t log = {0};
			if(osMessageQueueGet(LogQueueHandle, log, 0, 200) == osOK)
			{
				for(uint8_t i = 0; i < 10; i++)
				{
					sprintf((char*)sendbuf, "%d: Temp:%dC Humi:%d%%RH Light:%dmv\r\n",
					        i+1, log[0][i], log[1][i], log[2][i]);
					bsp_uart_transmit(sendbuf, (uint32_t)strlen((char*)sendbuf), 200);
				}
			}
		}
		break;

		case CMD_ALARM_SWITCH:  /* 自动报警开关：1=ON 其它=OFF */
			osMutexAcquire(myMutex01Handle, osWaitForever);
			if(frame[3] == 1)
			{
				g_auto_alarm = AUTO_ALARM_ON;
				osEventFlagsSet(AlarmEventHandle, BIT_AUTO_ALARM);
				ack = "AutoAlarm ON\r\n";
			}
			else
			{
				g_auto_alarm = AUTO_ALARM_OFF;
				osEventFlagsClear(AlarmEventHandle, BIT_AUTO_ALARM);
				ack = "AutoAlarm OFF\r\n";
			}
			osMutexRelease(myMutex01Handle);
			bsp_uart_transmit((uint8_t*)ack, (uint32_t)strlen(ack), 200);
			break;

		default:
			ack = "Cmd is warning\r\n";
			bsp_uart_transmit((uint8_t*)ack, (uint32_t)strlen(ack), 200);
			break;
	}
}

void SerialTask_Entry(void *argument)
{
	(void)argument;

	rb_init(rb, shortage, BUFFER_MAXSIZE);
	bsp_uart_rxStart(dma_rx_buf, sizeof(dma_rx_buf));

	/* 帧组包状态 */
	uint8_t frame[SERIAL_MAXSIZE];
	uint16_t fcnt   = 0;
	uint8_t started = 0;

	for(;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);   /* 等待串口中断通知 */

		/* 逐字节从环形缓冲读出，组出一帧或多帧 */
		while(rb_available(rb) > 0U)
		{
			uint8_t b;
			if(rb_read(rb, &b) != ERR_OK) break;

			if(!started)
			{
				if(b == FRAME_H)   /* 捕获帧头 */
				{
					started = 1;
					fcnt = 0;
					frame[fcnt++] = b;
				}
			}
			else
			{
				frame[fcnt++] = b;
				if(b == FRAME_T)        /* 收到帧尾，一帧完成 */
				{
					if(App_FrameValid(frame, fcnt))
					{
						App_SerialProcess(frame, fcnt);
					}
					started = 0;
					fcnt = 0;
				}
				else if(fcnt >= SERIAL_MAXSIZE)  /* 超长，丢弃重找帧头 */
				{
					started = 0;
					fcnt = 0;
				}
			}
		}
	}
}


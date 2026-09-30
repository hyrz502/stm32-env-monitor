#include "app_task.h"
#include "app_data.h"
#include "app_rtos.h"
#include "app_config.h"
#include "bsp_gpio.h"
#include "main.h"

#define PRESS_NONE   0U
#define PRESS_SHORT  1U
#define PRESS_LONG   2U

/* 读取按键并区分 短按/长按（按下>500ms 判为长按），带消抖 */
static uint8_t Key_Detect(GPIO_TypeDef *Port, uint16_t Pin)
{
	osDelay(15);                       /* 消抖 */
	if(bsp_gpio_read(Port, Pin) != GPIO_PIN_RESET) return PRESS_NONE;  /* 已释放 */

	uint32_t start = HAL_GetTick();
	/* 逐次重读引脚：按住期间计时区分长短按，松开即退出 */
	while(bsp_gpio_read(Port, Pin) == GPIO_PIN_RESET)
	{
		if((HAL_GetTick() - start) >= 500) return PRESS_LONG;
		osDelay(1);
	}
	return PRESS_SHORT;
}

/**
  * @brief  按键任务：由 GPIO 外部中断通知唤醒。
  *         Key1: 短按->配置页移动箭头；长按->页面切换。
  *         Key2: 短按->调整当前项值；长按->恢复当前项默认。
  */
void KeyTask_Entry(void *argument)
{
	(void)argument;

	for(;;)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		/* ---- Key1 ---- */
		if(bsp_gpio_read(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_RESET)
		{
			uint8_t press = Key_Detect(Key1_GPIO_Port, Key1_Pin);
			if(press == PRESS_LONG)          /* 长按：页面切换 */
			{
				osMutexAcquire(myMutex01Handle, osWaitForever);
				g_page_flag = (g_page_flag == 1) ? 2 : 1;
				osMutexRelease(myMutex01Handle);
			}
			else if(press == PRESS_SHORT && g_page_flag == 2)   /* 短按：移动箭头 */
			{
				osMutexAcquire(myMutex01Handle, osWaitForever);
				g_arrow = (g_arrow >= 7) ? 1 : (g_arrow + 1);
				osMutexRelease(myMutex01Handle);
			}
		}

		/* ---- Key2 ---- */
		if(bsp_gpio_read(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_RESET)
		{
			uint8_t press = Key_Detect(Key2_GPIO_Port, Key2_Pin);
			if(press == PRESS_SHORT && g_page_flag == 2)   /* 短按：调整当前项 */
			{
				osMutexAcquire(myMutex01Handle, osWaitForever);
				switch(g_arrow)
				{
					case 1:   /* 采集周期 */
						g_period = (g_period >= PERIOD_U8_MAX) ? 1 : (g_period + 1);
						break;
					case 2:   /* 自动报警开关 */
						if(g_auto_alarm == AUTO_ALARM_OFF)
						{
							g_auto_alarm = AUTO_ALARM_ON;
							osEventFlagsSet(AlarmEventHandle, BIT_AUTO_ALARM);
						}
						else
						{
							g_auto_alarm = AUTO_ALARM_OFF;
							osEventFlagsClear(AlarmEventHandle, BIT_AUTO_ALARM);
						}
						break;
					case 3:   /* 报警阈值 */
						g_threshold = (g_threshold >= THRESHOLD_MAX) ? 1 : (g_threshold + 1);
						break;
					default:
						break;
				}
				osMutexRelease(myMutex01Handle);
			}
			else if(press == PRESS_LONG && g_page_flag == 2)   /* 长按：恢复默认 */
			{
				osMutexAcquire(myMutex01Handle, osWaitForever);
				switch(g_arrow)
				{
					case 1: g_period = DEFAULT_PERIOD; break;
					case 2:
						g_auto_alarm = AUTO_ALARM_OFF;
						osEventFlagsClear(AlarmEventHandle, BIT_AUTO_ALARM);
						break;
					case 3: g_threshold = DEFAULT_THRESHOLD; break;
					default: break;
				}
				osMutexRelease(myMutex01Handle);
			}
		}
	}
}


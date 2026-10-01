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
				App_Data_TogglePage();
			}
			else if(press == PRESS_SHORT && App_Data_GetPageFlag() == 2)   /* 短按：移动箭头 */
			{
				App_Data_ArrowNext();
			}
		}

		/* ---- Key2 ---- */
		if(bsp_gpio_read(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_RESET)
		{
			uint8_t press = Key_Detect(Key2_GPIO_Port, Key2_Pin);
			if(press == PRESS_SHORT && App_Data_GetPageFlag() == 2)   /* 短按：调整当前项 */
			{
				switch(App_Data_GetArrow())
				{
					case 1:   /* 采集周期 */
						App_Data_PeriodNext();
						break;
					case 2:   /* 自动报警开关 */
						App_Data_ToggleAutoAlarm();
						break;
					case 3:   /* 报警阈值 */
						App_Data_ThresholdNext();
						break;
					default:
						break;
				}
			}
			else if(press == PRESS_LONG && App_Data_GetPageFlag() == 2)   /* 长按：恢复默认 */
			{
				switch(App_Data_GetArrow())
				{
					case 1: App_Data_PeriodReset(); break;
					case 2: App_Data_SetAutoAlarm(AUTO_ALARM_OFF); break;
					case 3: App_Data_ThresholdReset(); break;
					default: break;
				}
			}
		}
	}
}


#include "app_task.h"
#include "app_data.h"
#include "app_rtos.h"
#include "app_config.h"
#include "bsp_oled.h"
#include <string.h>

/**
  * @brief  显示任务：维护一个运行时钟，按 g_page_flag 绘制首页/配置页。
  */
void DisplayTask_Entry(void *argument)
{
	(void)argument;
	uint32_t s    = 0;
	uint16_t min  = 0;
	uint8_t  hour = 0;

	for(;;)
	{
		/* ---- 运行时钟（1s 递增） ---- */
		if(++s >= 60) { s = 0; if(++min >= 60) { min = 0; if(++hour >= 24) hour = 0; } }

		if(g_page_flag == 1)   /* 首页：时间 / 温湿度 / 光照 */
		{
			uint8_t  temp, humi;
			uint16_t light_v_int, light_v_poi;

			osMutexAcquire(myMutex01Handle, 100);
			temp = g_temp;
			humi = g_humi;
			light_v_int = (uint16_t)(g_light_raw * 3300U / 4095U / 1000U);
			light_v_poi = (uint16_t)(g_light_raw * 3300U / 4095U / 10U % 100U);
			osMutexRelease(myMutex01Handle);

			OLED_NewFrame();
			OLED_ShowString(1,0,"Time:");
			OLED_ShowString(1,2,"Humi:");
			OLED_ShowString(1,3,"Temp:");
			OLED_ShowString(1,4,"Light:");

			OLED_ShowNum(8,3,temp,2);
			OLED_ShowNum(8,2,humi,2);
			OLED_ShowNum(8,4,light_v_int,1);
			OLED_ShowChar(9,4,'.');
			OLED_ShowNum(10,4,light_v_poi,2);

			OLED_ShowNum(6,0,hour,2);
			OLED_ShowNum(9,0,min,2);
			OLED_ShowNum(12,0,s,2);
			OLED_ShowChar(8,0,':');
			OLED_ShowChar(11,0,':');
			OLED_ShowFrame();
		}
		else if(g_page_flag == 2)   /* 配置页：周期 / 报警开关 / 阈值 */
		{
			const char *alarmStr;
			uint8_t thr, period, arrow;

			osMutexAcquire(myMutex01Handle, 100);
			thr   = g_threshold;
			period= g_period;
			arrow = g_arrow;
			alarmStr = (g_auto_alarm == AUTO_ALARM_ON) ? "ON" : "OFF";
			osMutexRelease(myMutex01Handle);

			OLED_NewFrame();
			OLED_ShowString(5,0,"CONFIG");
			OLED_ShowString(0,1,"Period:");
			OLED_ShowString(0,2,"AutoAlarm:");
			OLED_ShowString(0,3,"Threshold:");

			OLED_ShowNum(11,3,thr,2);
			OLED_ShowChar(17,arrow,'<');
			OLED_ShowString(12,2,(char*)alarmStr);
			OLED_ShowNum(8,1,period,2);
			OLED_ShowChar(10,1,'s');
			OLED_ShowFrame();
		}

		osDelay(999);
	}
}


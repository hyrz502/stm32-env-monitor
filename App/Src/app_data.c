#include "app_data.h"
#include "app_config.h"
#include "app_rtos.h"

static ring_buffer_t s_rb;
ring_buffer_t* rb = &s_rb;

uint8_t shortage[BUFFER_MAXSIZE];

/* ---- 业务共享数据定义（默认值对齐 V0） ---- */
uint8_t   g_temp          = 0;
uint8_t   g_humi          = 0;
uint16_t  g_light_raw     = 0;
uint8_t   g_threshold     = DEFAULT_THRESHOLD;   /* 30 */
uint8_t   g_period        = DEFAULT_PERIOD;      /* 1 */
uint8_t   g_page_flag     = 1;
uint8_t   g_led_flag      = LED_MODE_OFF;
uint8_t   g_arrow         = 1;
uint8_t   g_auto_alarm    = AUTO_ALARM_OFF;

/* ==================== 数据访问层实现：互斥锁全部封装在函数内部 ==================== */

/* 快照读取全部共享数据：一次加锁，保证读取的一组数据是同一时刻的一致快照 */
void App_Data_GetSnapshot(AppDataSnapshot_t *snap)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);

	snap->temp       = g_temp;
	snap->humi       = g_humi;
	snap->light_raw  = g_light_raw;
	snap->threshold  = g_threshold;
	snap->period     = g_period;
	snap->page_flag  = g_page_flag;
	snap->led_flag   = g_led_flag;
	snap->arrow      = g_arrow;
	snap->auto_alarm = g_auto_alarm;

	osMutexRelease(myMutex01Handle);
}

/* 采集结果一次性发布：内部一次加锁更新三个传感器值 */
void App_Data_PublishSensor(uint8_t temp, uint8_t humi, uint16_t light_raw)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);

	g_temp      = temp;
	g_humi      = humi;
	g_light_raw = light_raw;

	osMutexRelease(myMutex01Handle);
}

/* ---- 显示页 ---- */
uint8_t App_Data_GetPageFlag(void)
{
	uint8_t v;
	osMutexAcquire(myMutex01Handle, osWaitForever);
	v = g_page_flag;
	osMutexRelease(myMutex01Handle);
	return v;
}

void App_Data_TogglePage(void)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);
	g_page_flag = (g_page_flag == 1) ? 2 : 1;
	osMutexRelease(myMutex01Handle);
}

/* ---- 配置箭头 ---- */
uint8_t App_Data_GetArrow(void)
{
	uint8_t v;
	osMutexAcquire(myMutex01Handle, osWaitForever);
	v = g_arrow;
	osMutexRelease(myMutex01Handle);
	return v;
}

void App_Data_ArrowNext(void)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);
	g_arrow = (g_arrow >= 7) ? 1 : (g_arrow + 1);
	osMutexRelease(myMutex01Handle);
}

/* ---- 采集周期 ---- */
void App_Data_SetPeriod(uint8_t period)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);
	g_period = period;
	osMutexRelease(myMutex01Handle);
}

void App_Data_PeriodNext(void)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);
	g_period = (g_period >= PERIOD_U8_MAX) ? 1 : (g_period + 1);
	osMutexRelease(myMutex01Handle);
}

void App_Data_PeriodReset(void)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);
	g_period = DEFAULT_PERIOD;
	osMutexRelease(myMutex01Handle);
}

/* ---- 报警阈值 ---- */
void App_Data_SetThreshold(uint8_t threshold)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);
	g_threshold = threshold;
	osMutexRelease(myMutex01Handle);
}

void App_Data_ThresholdNext(void)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);
	g_threshold = (g_threshold >= THRESHOLD_MAX) ? 1 : (g_threshold + 1);
	osMutexRelease(myMutex01Handle);
}

void App_Data_ThresholdReset(void)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);
	g_threshold = DEFAULT_THRESHOLD;
	osMutexRelease(myMutex01Handle);
}

/* ---- LED 模式 ---- */
void App_Data_SetLedFlag(uint8_t mode)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);
	g_led_flag = mode;
	osMutexRelease(myMutex01Handle);
}

/* ---- 自动报警开关（同时同步事件位，保证数据与事件一致） ---- */
void App_Data_SetAutoAlarm(uint8_t on)
{
	osMutexAcquire(myMutex01Handle, osWaitForever);
	g_auto_alarm = on;
	osMutexRelease(myMutex01Handle);

	if(on == AUTO_ALARM_ON)
	{
		osEventFlagsSet(AlarmEventHandle, BIT_AUTO_ALARM);
	}
	else
	{
		osEventFlagsClear(AlarmEventHandle, BIT_AUTO_ALARM);
	}
}

void App_Data_ToggleAutoAlarm(void)
{
	uint8_t new_state;

	osMutexAcquire(myMutex01Handle, osWaitForever);
	new_state = (g_auto_alarm == AUTO_ALARM_OFF) ? AUTO_ALARM_ON : AUTO_ALARM_OFF;
	g_auto_alarm = new_state;
	osMutexRelease(myMutex01Handle);

	if(new_state == AUTO_ALARM_ON)
	{
		osEventFlagsSet(AlarmEventHandle, BIT_AUTO_ALARM);
	}
	else
	{
		osEventFlagsClear(AlarmEventHandle, BIT_AUTO_ALARM);
	}
}


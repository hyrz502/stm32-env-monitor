#ifndef __APP_DATA_H__
#define __APP_DATA_H__

#include "ring_buffer.h"
#include "app_config.h"

extern ring_buffer_t* rb;
extern uint8_t shortage[BUFFER_MAXSIZE];

/* ==================== 业务共享数据（跨任务，由 myMutex01 / 事件 / 通知保护） ==================== */
extern uint8_t   g_temp;          /* 温度 ℃ */
extern uint8_t   g_humi;          /* 湿度 %RH */
extern uint16_t  g_light_raw;     /* 光照 ADC 原始值 */
extern uint8_t   g_threshold;     /* 报警阈值 ℃ */
extern uint8_t   g_period;        /* 采集周期 s */
extern uint8_t   g_page_flag;     /* 显示页：1=首页 2=配置 */
extern uint8_t   g_led_flag;      /* LED 模式：LED_MODE_* */
extern uint8_t   g_arrow;         /* 配置页箭头位置 1~7 */
extern uint8_t   g_auto_alarm;    /* 自动报警：AUTO_ALARM_ON / AUTO_ALARM_OFF */

/* ==================== 数据访问层（互斥锁封装在函数内部，调用方无需加锁） ==================== */

/* 一次加锁取回全部共享数据的快照，避免多次加锁导致数据半新半旧 */
typedef struct
{
	uint8_t   temp;          /* 温度 ℃ */
	uint8_t   humi;          /* 湿度 %RH */
	uint16_t  light_raw;     /* 光照 ADC 原始值 */
	uint8_t   threshold;     /* 报警阈值 ℃ */
	uint8_t   period;        /* 采集周期 s */
	uint8_t   page_flag;     /* 显示页：1=首页 2=配置 */
	uint8_t   led_flag;      /* LED 模式：LED_MODE_* */
	uint8_t   arrow;         /* 配置页箭头位置 1~7 */
	uint8_t   auto_alarm;    /* 自动报警：AUTO_ALARM_ON / AUTO_ALARM_OFF */
} AppDataSnapshot_t;

void    App_Data_GetSnapshot(AppDataSnapshot_t *snap);                    /* 快照读取全部共享数据 */
void    App_Data_PublishSensor(uint8_t temp, uint8_t humi, uint16_t light_raw);  /* 采集结果一次性发布 */

uint8_t App_Data_GetPageFlag(void);                                       /* 读当前显示页 */
void    App_Data_TogglePage(void);                                        /* 页面切换 1<->2 */
uint8_t App_Data_GetArrow(void);                                          /* 读箭头位置 */
void    App_Data_ArrowNext(void);                                         /* 箭头下移 1~7 循环 */

void    App_Data_SetPeriod(uint8_t period);                               /* 设置采集周期 */
void    App_Data_PeriodNext(void);                                        /* 周期+1（回绕） */
void    App_Data_PeriodReset(void);                                       /* 周期恢复默认 */

void    App_Data_SetThreshold(uint8_t threshold);                         /* 设置报警阈值 */
void    App_Data_ThresholdNext(void);                                     /* 阈值+1（回绕） */
void    App_Data_ThresholdReset(void);                                    /* 阈值恢复默认 */

void    App_Data_SetLedFlag(uint8_t mode);                                /* 设置 LED 模式 */

void    App_Data_SetAutoAlarm(uint8_t on);                                /* 开关自动报警（含事件位同步） */
void    App_Data_ToggleAutoAlarm(void);                                   /* 自动报警取反（含事件位同步） */


#endif /* __APP_DATA_H__ */


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


#endif /* __APP_DATA_H__ */


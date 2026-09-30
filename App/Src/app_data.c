#include "app_data.h"
#include "app_config.h"

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


#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

#define BUFFER_MAXSIZE 256U

typedef uint16_t LogData_t[3][10];   /* [0]=Temp [1]=Humi [2]=光强电压(mV)，电压需 u16 */

/* ======================== 串口帧协议 ======================== */
/* [0xAA] [LEN] [CMD] [DATA...] [CRC] [0x55] ，CRC = CMD..DATA 的异或 */
#define SERIAL_MAXSIZE   20U        /* 串口接收缓冲大小 */
#define FRAME_H          0xAA       /* 帧头 */
#define FRAME_T          0x55       /* 帧尾 */

/* 命令码 */
#define CMD_GET_CURRENT  0x01       /* 查询当前数据 */
#define CMD_SET_INTERVAL 0x02       /* 设置采样间隔(1~60s) */
#define CMD_SET_THRESHOLD 0x03      /* 设置报警阈值(1~127) */
#define CMD_SET_LED      0x04       /* 控制 LED: 0灭 1亮 2闪 */
#define CMD_GET_HISTORY  0x05       /* 请求历史数据 */
#define CMD_ALARM_SWITCH 0x06       /* 自动报警开关: 1=ON 其它=OFF */

/* ======================== LED 模式 ======================== */
#define LED_MODE_OFF     0x00
#define LED_MODE_ON      0x88
#define LED_MODE_BLINK   0x99

/* ======================== 事件标志位 ======================== */
#define BIT_TEMP_OVER    (1UL << 0)  /* 温度超过阈值 */
#define BIT_AUTO_ALARM   (1UL << 1)  /* 自动报警已开启 */

/* ======================== 自动报警开关取值 ======================== */
#define AUTO_ALARM_OFF   0U
#define AUTO_ALARM_ON    1U

/* ======================== 默认参数与范围 ======================== */
#define DEFAULT_THRESHOLD   30U
#define DEFAULT_PERIOD      1U
#define PERIOD_MAX          60U      /* 串口 0x02 允许范围 1~60 */
#define THRESHOLD_MAX       127U     /* 串口 0x03 允许范围 1~127 */
#define PERIOD_U8_MAX       128U     /* 按键调整回绕上限(MAXSIZE_U8 语义) */


#endif /* __APP_CONFIG_H__ */


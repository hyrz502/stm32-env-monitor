#ifndef __BSP_DHT11_H__
#define __BSP_DHT11_H__

#include "bsp_delay.h"
#include "main.h"
#include "gpio.h"
#include "err.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/**
  * @brief  执行一次 DHT11 温湿度采样
  * @param  pTemp 温度整数输出（℃）
  * @param  pHumi 湿度整数输出（%RH）
  * @retval ERR_OK       采样成功且校验通过
  *         ERR_TIMEOUT  无应答或数据位超时
  *         ERR_CRC      校验和错误
  */
err_t Bsp_Dht11Sample(uint8_t *pTemp, uint8_t *pHumi);

#endif /* __BSP_DHT11_H__ */

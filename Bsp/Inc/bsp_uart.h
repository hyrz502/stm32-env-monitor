#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include "main.h"
#include "gpio.h"
#include "err.h"
#include "usart.h"
#include <stdio.h>

err_t bsp_uart_transmit(uint8_t *pData,uint32_t Size,uint32_t Timeout);
err_t bsp_uart_ReceiveToIdle_DMA(uint8_t *pData,uint32_t Size);
err_t bsp_uart_rxStart(void);	/* uart1 dma 关闭半满中断,接收进入环形缓冲区 rb  */

int fputc(int ch,FILE *f);



#endif /* __BSP_UART_H__ */

#include "bsp_uart.h"
#include "cmsis_os.h"
//ring_buffer_t local_buf=

err_t bsp_uart_transmit(uint8_t *pData,uint32_t Size,uint32_t Timeout)
{
	HAL_UART_Transmit(&huart1,pData,Size,Timeout);
	return ERR_OK;
}

err_t bsp_uart_ReceiveToIdle_DMA(uint8_t *pData,uint32_t Size)
{
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1,pData,Size);
	return ERR_OK;
}

err_t bsp_uart_rxStart(void)
{
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1,rb->buf,sizeof(rb->buf));
	__HAL_DMA_DISABLE_IT(huart1.hdmarx,DMA_IT_HT);
	return ERR_OK;
}

/*
*  @brief:重定向printf打印; <stdio.h> MicroLIB
*  @note:阻塞操作 仅用于调试时使用
**/

int fputc(int ch,FILE *f)
{
	HAL_UART_Transmit(&huart1,(uint8_t*)&ch,1,200);
	return ch;
}

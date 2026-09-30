#include "bsp_uart.h"
#include "cmsis_os.h"

err_t bsp_uart_transmit(uint8_t *pData,uint32_t Size,uint32_t Timeout)
{
	if(HAL_UART_Transmit(&huart1,pData,Size,Timeout)!=HAL_OK)
	{
		return ERR_IO;
	}
	return ERR_OK;
}

uint8_t dma_rx_buf[128];

err_t bsp_uart_rxStart(uint8_t *pData,uint16_t Size)
{
	if(pData==NULL||Size==0U)
	{
		return ERR_PARAM;
	}
	if(HAL_UARTEx_ReceiveToIdle_DMA(&huart1,pData,Size)!=HAL_OK)
	{
		return ERR_IO;
	}
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

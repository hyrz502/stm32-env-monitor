#include "spi.h"
#include "gpio.h"
#include "bsp_w25q64.h"


static uint8_t W25Q64_SPI_RW(uint8_t tData)
{
	uint8_t rData=0;
	HAL_SPI_TransmitReceive(&hspi1,&tData,&rData,sizeof(tData),100);
	return rData;
}

static uint8_t W25Q64_SPI_ReadStatus(void)
{
	W25Q_CS_Low();
	W25Q64_SPI_RW(W25Q_CMD_Read_Status_Register1);
	uint8_t ret=W25Q64_SPI_RW(0xFF);
	W25Q_CS_High();
	return ret;
}

static void W25Q64_WaitBusy(void)
{
	while(W25Q64_SPI_ReadStatus()&0x01);
}

static void W25Q64_WriteEnable(void)
{
	W25Q_CS_Low();
	W25Q64_SPI_RW(W25Q_CMD_Write_Enable);
	W25Q_CS_High();
}

void W25Q64_ReadID(uint8_t* id_buf)
{
	W25Q_CS_Low();
	W25Q64_SPI_RW(W25Q_CMD_JESEC_ID);
	id_buf[0]=W25Q64_SPI_RW(0xFF);
	id_buf[1]=W25Q64_SPI_RW(0xFF);
	id_buf[2]=W25Q64_SPI_RW(0xFF);
	W25Q_CS_High();
}

void W25Q64_Read(uint8_t* buf,uint32_t addr,uint32_t len)
{
	W25Q_CS_Low();
	W25Q64_SPI_RW(W25Q_CMD_Read_Data);
	W25Q64_SPI_RW((addr>>16)&0xFF);
	W25Q64_SPI_RW((addr>>8)&0xFF);
	W25Q64_SPI_RW((addr)&0xFF);
	while(len--)
	{
		*buf=W25Q64_SPI_RW(0xFF);
		buf++;
	}
	W25Q_CS_High();
}

void W25Q64_PageWrite(uint8_t*buf,uint32_t addr,uint16_t len)
{
	if(len>256) len=256;
	if((addr&0xFF)+len>256) return;
	W25Q64_WriteEnable();
	W25Q_CS_Low();
	W25Q64_SPI_RW(W25Q_CMD_Page_Program);
	W25Q64_SPI_RW((addr>>16)&0xFF);
	W25Q64_SPI_RW((addr>>8)&0xFF);
	W25Q64_SPI_RW((addr)&0xFF);
	for(uint16_t i=0;i<len;i++)
	{
		W25Q64_SPI_RW(buf[i]);
	}
	W25Q_CS_High();
	W25Q64_WaitBusy();
}

void W25Q64_SectorErase(uint32_t addr)
{
	if(addr%4096) return;	
	W25Q64_WriteEnable();
	W25Q_CS_Low();
	
	W25Q64_SPI_RW(W25Q_CMD_Sector_Erase4K);
	W25Q64_SPI_RW((addr>>16)&0xFF);
	W25Q64_SPI_RW((addr>>8)&0xFF);
	W25Q64_SPI_RW((addr)&0xFF);
	
	W25Q_CS_High();
	W25Q64_WaitBusy();

}

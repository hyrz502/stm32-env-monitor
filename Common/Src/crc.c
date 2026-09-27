#include "crc.h"

uint16_t crc_modbus(const uint8_t*data,uint32_t len)
{
    if(data==NULL)
    {
        return 0U;
    }
    uint16_t crc=0xFFFFU;
    for(uint32_t i=0;i<len;i++)
    {
        crc^=data[i];
        for(uint8_t bit=0U;bit<8U;bit++)
        {
            if((crc&0x0001U)!=0U)
            {
                crc=(uint16_t)(crc>>1)^0xA001U;
            }
            else
            {
                crc=(uint16_t)(crc>>1);
            }
        }
    }
    return crc;
}
uint32_t crc_calc(const uint8_t* data,uint32_t len)
{
    if(data==NULL)
    {
        return 0;
    }
    uint32_t crc=0xFFFFFFFFU;
    for(uint32_t i=0;i<len;i++)
    {
        crc^=data[i];
        for(uint8_t bit=0U;bit<8U;bit++)
        {
            if((crc&1U)!=0)
            {
                crc=(crc>>1)^0xEDB88320U;
            }
            else
            {
                crc>>=1;
            }
        }
    }
    return crc;
}

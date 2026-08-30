#ifndef __DHT11_H_
#define __DHT11_H_

#include "main.h"

void Delay_us(uint32_t us);
void Delay_ms(uint32_t ms);
void Delay_s(uint32_t s);
uint8_t DHT11_Samping(uint8_t *Temp,uint8_t*Humi);

#endif

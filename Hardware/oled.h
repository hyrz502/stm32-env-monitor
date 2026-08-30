#ifndef __OLED_H_
#define __OLED_H_

#include "i2c.h"

#define OLED_ADDRESS          0x78     // OLED I2C地址

void OLED_SendCmd(uint8_t cmd);
void OLED_Init(void);
void OLED_NewFrame(void);
void OLED_ShowFrame(void);
void OLED_SetPixel(uint8_t x,uint8_t y);
void OLED_ShowChar(uint8_t x, uint8_t page, char ch);
void OLED_ShowString(uint8_t x, uint8_t page, const char *str);
void OLED_ShowNum(uint16_t Column,uint16_t Page,uint32_t num,uint8_t len);


#endif

/**
  ******************************************************************************
  * @file    bsp_oled.h
  * @brief   0.96" OLED（SSD1315/SSD1306，I2C1，PB8/PB9）驱动接口。
  *          屏幕为 128x64，按 8 页 x 128 列分页；字符 6x8。
  ******************************************************************************
  */
#ifndef __BSP_OLED_H
#define __BSP_OLED_H

#include <stdint.h>

#define OLED_ADDRESS  0x78U     /* 屏 I2C 地址（含写位） */

/* ---- 基础命令 ---- */
void OLED_SendCmd(uint8_t cmd);

/* ---- 初始化 ---- */
void OLED_Init(void);

/* ---- 双缓冲：NewFrame 清屏(内存) -> 绘制 -> ShowFrame 整页刷新(同步) ---- */
void OLED_NewFrame(void);
void OLED_ShowFrame(void);

/* ---- 绘制原语 ---- */
void OLED_SetPixel(uint8_t x, uint8_t y);

/* x: 字符列(0~20)，page: 页(0~7) */
void OLED_ShowChar(uint8_t x, uint8_t page, char ch);
void OLED_ShowString(uint8_t x, uint8_t page, const char *str);
void OLED_ShowNum(uint16_t column, uint16_t page, uint32_t num, uint8_t len);

#endif /* __BSP_OLED_H */

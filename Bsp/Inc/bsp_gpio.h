#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#include "main.h"

void bsp_gpio_write(GPIO_TypeDef* GPIO, uint16_t GPIO_Pin, GPIO_PinState PinState);
void bsp_gpio_toggle(GPIO_TypeDef* GPIO, uint16_t GPIO_Pin);
GPIO_PinState bsp_gpio_read(GPIO_TypeDef* GPIO, uint16_t GPIO_Pin);

#endif /* __BSP_GPIO_H__ */

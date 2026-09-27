#include "bsp_gpio.h"

void bsp_gpio_write(GPIO_TypeDef* GPIO, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    HAL_GPIO_WritePin(GPIO, GPIO_Pin, PinState);
}

void bsp_gpio_toggle(GPIO_TypeDef* GPIO, uint16_t GPIO_Pin)
{
    HAL_GPIO_TogglePin(GPIO, GPIO_Pin);
}

GPIO_PinState bsp_gpio_read(GPIO_TypeDef* GPIO, uint16_t GPIO_Pin)
{
    return HAL_GPIO_ReadPin(GPIO, GPIO_Pin);
}

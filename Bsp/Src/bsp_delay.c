#include "bsp_delay.h"

void bsp_delay_us(uint32_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_Base_Start(&htim2);
    while(__HAL_TIM_GET_COUNTER(&htim2) < us);
    HAL_TIM_Base_Stop(&htim2);
}

void bsp_delay_ms(uint32_t ms)
{
	while(ms--)
	{
		bsp_delay_us(1000);
	}
}

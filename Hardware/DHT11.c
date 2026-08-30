#include "DHT11.h"
#include "main.h"
#include "string.h"
#include "cmsis_os.h"
#include "task.h"


void Delay_us(uint32_t us)
{
	__HAL_TIM_SET_COUNTER(&htim2,0);
	HAL_TIM_Base_Start(&htim2);
	while(__HAL_TIM_GET_COUNTER(&htim2)<us);
	HAL_TIM_Base_Stop(&htim2);
}

void Delay_ms(uint32_t ms)
{
	while(ms--)
	{
		Delay_us(1000);
	}
}

void Delay_s(uint32_t s)
{
	while(s--)
	{
		Delay_ms(1000);
	}
}

void DHT11_Input(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();


  /*Configure GPIO pins : DHT11_Pin */
  GPIO_InitStruct.Pin = DHT11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void DHT11_Output(void)
{

	GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

	HAL_GPIO_WritePin(DHT11_GPIO_Port,DHT11_Pin,GPIO_PIN_RESET);
  
  /*Configure GPIO pins : DHT11_Pin */
  GPIO_InitStruct.Pin = DHT11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void DHT11_W_Line(uint8_t Bit)
{
	if(Bit==0)
	{
		HAL_GPIO_WritePin(DHT11_GPIO_Port,DHT11_Pin,GPIO_PIN_RESET);
	}
	
	else
	{
		HAL_GPIO_WritePin(DHT11_GPIO_Port,DHT11_Pin,GPIO_PIN_SET);
	}
}

uint8_t DHT11_R_Line(void)
{
	uint8_t Bit;
	if(HAL_GPIO_ReadPin(DHT11_GPIO_Port,DHT11_Pin)==GPIO_PIN_RESET)
	{
		Bit=0;
	}
	else
	{
		Bit=1;
	}
	return Bit;
}

void DHT11_WaitLine(uint8_t line)
{
	uint16_t timeout=2000;
	while(DHT11_R_Line()==line&&timeout)
	{
		timeout--;
	}
}
/**************************************************************************************************************/
/*--------------------------------------三步骤---开始---应答---接收-----------------------------------------------*/
void DHT11_Start(void)
{
	DHT11_Output();				//开启输出模式
	DHT11_W_Line(0);
	osDelay(25);			//拉低>=18ms
}

uint8_t DHT11_ACK(void)
{
	uint16_t timeout=2000;
	
	while(HAL_GPIO_ReadPin(DHT11_GPIO_Port,DHT11_Pin)==GPIO_PIN_SET)
	{
		timeout--;
		if(timeout==0) return 0;
	}

	timeout=2000;
	while(HAL_GPIO_ReadPin(DHT11_GPIO_Port,DHT11_Pin)==GPIO_PIN_RESET)
	{
		timeout--;
		if(timeout==0) return 0;
	}
	timeout=2000;
	while(HAL_GPIO_ReadPin(DHT11_GPIO_Port,DHT11_Pin)==GPIO_PIN_SET)
	{
		timeout--;
		if(timeout==0) return 0;
	}
	
	return 1;
}

uint8_t DHT11_RecieveByte(uint8_t *Temp,uint8_t*Humi)
{
	uint8_t Byte[5];
		for(uint8_t j=0;j<5;j++)
		{
			Byte[j]=0;
			for(uint8_t i=0;i<8;i++)
			{
				DHT11_WaitLine(0);
				Delay_us(40);
				if(DHT11_R_Line()==1)
				{
					Byte[j]|=(0x80>>i);
					DHT11_WaitLine(1);
				}

			}
		}		
	if(Byte[4]==Byte[0]+Byte[1]+Byte[2]+Byte[3])
	{
		
		*Temp=Byte[2];
		*Humi=Byte[0];
		return 1;
	}
	else
	{
		memset(Byte,0,sizeof(Byte));
		return 0;
	}
}
uint8_t DHT11_Samping(uint8_t *Temp,uint8_t*Humi)
{
	DHT11_Start();

	taskENTER_CRITICAL();
	DHT11_W_Line(1);
	Delay_us(27);
	DHT11_Input();

	uint8_t result = 0;
	if(DHT11_ACK()==1)
	{
		result = DHT11_RecieveByte(Temp,Humi);
	}
	taskEXIT_CRITICAL();
	return result;
}

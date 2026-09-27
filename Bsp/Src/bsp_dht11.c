/**
  ******************************************************************************
  * @file    bsp_dht11.c
  * @brief   DHT11 温湿度传感器驱动（单总线，PB12）。
  *
  * 时序说明（标准单总线协议）：
  *   主机拉低 >18ms 发起采样 -> 释放总线 -> DHT11 应答（低80us+高80us）
  *   -> 每位数据：50us 低电平 + 26~28us(0) 或 70us(1) 高电平。
  * 本驱动在临界区（关中断）内完成时序读取，防止被调度打断导致位宽失真；
  * 关键等待均带超时计数，避免卡死。
  ******************************************************************************
  */
#include "bsp_dht11.h"
#include "bsp_delay.h"

/* ---- GPIO 方向切换（内部使用） ---- */
static void Dht11_SetInput(void)
{
  GPIO_InitTypeDef gpio = {0};
  __HAL_RCC_GPIOB_CLK_ENABLE();
  gpio.Pin   = DHT11_Pin;
  gpio.Mode  = GPIO_MODE_INPUT;
  gpio.Pull  = GPIO_PULLUP;
  HAL_GPIO_Init(DHT11_GPIO_Port, &gpio);
}

static void Dht11_SetOutput(void)
{
  GPIO_InitTypeDef gpio = {0};
  __HAL_RCC_GPIOB_CLK_ENABLE();
  gpio.Pin   = DHT11_Pin;
  gpio.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_GPIO_Port, &gpio);
}

static void Dht11_WriteBit(uint8_t level)
{
  HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin,
                    level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint8_t Dht11_ReadBit(void)
{
  return (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == GPIO_PIN_SET) ? 1U : 0U;
}

/* 带上限的等待：等待电平变为 expect。返回 0=超时 */
static uint8_t Dht11_WaitLevel(uint8_t expect, uint32_t timeout_cnt)
{
  while (timeout_cnt--)
  {
    if (Dht11_ReadBit() == expect)
    {
      return 1U;
    }
  }
  return 0U;
}

/**
  * @brief 发起时序并读取 5 字节数据（40bit）。
  * @note  应处于临界区（中断关闭）内调用。
  * @retval ERR_OK 读取成功；ERR_TIMEOUT 无应答或数据位超时
  */
static err_t Dht11_ReadData(uint8_t byte[5])
{
  uint32_t i, j;

  /* 主机发送起始信号后释放总线，读取应答：低80us -> 高80us */
  Dht11_WriteBit(1);
  bsp_delay_us(27);
  Dht11_SetInput();

  /* 等待 DHT11 拉低应答起始 */
  if (!Dht11_WaitLevel(0, 10000))
  {
    return ERR_TIMEOUT;
  }
  /* 完成应答低电平 */
  if (!Dht11_WaitLevel(1, 10000))
  {
    return ERR_TIMEOUT;
  }
  /* 完成应答高电平 */
  if (!Dht11_WaitLevel(0, 10000))
  {
    return ERR_TIMEOUT;
  }

  /* 读取 40 bit */
  for (j = 0; j < 5; j++)
  {
    byte[j] = 0;
    for (i = 0; i < 8; i++)
    {
      byte[j] <<= 1;
      /* 每位起始：50us 低电平 */
      if (!Dht11_WaitLevel(1, 10000))
      {
        return ERR_TIMEOUT;     /* 数据位低电平未结束即超时 */
      }
      bsp_delay_us(36);       /* 采样点置于 50us 之后 -> 高低电平宽度决定位值 */
      if (Dht11_ReadBit())
      {
        byte[j] |= 1U;
      }
      if (!Dht11_WaitLevel(0, 10000))
      {
        return ERR_TIMEOUT;
      }
    }
  }
  return ERR_OK;
}

/**
  * @brief  一次完整的采样流程。
  * @retval ERR_OK 成功；ERR_TIMEOUT 无应答/超时；ERR_CRC 校验和错误
  */
err_t Bsp_Dht11Sample(uint8_t *pTemp, uint8_t *pHumi)
{
  uint8_t byte[5] = {0};
  err_t ok = ERR_OK;

  /* 主机起始：输出模式，拉低 >= 18ms */
  Dht11_SetOutput();
  Dht11_WriteBit(0);
  bsp_delay_ms(20);

  /* 时序敏感段：关中断，避免任务切换/中断破坏时隙 */
  taskENTER_CRITICAL();
  ok = Dht11_ReadData(byte);
  if (ERR_IS_OK(ok))
  {
    /* 数据格式：humi_int humi_dec temp_int temp_dec check */
    if (byte[4] == (byte[0] + byte[1] + byte[2] + byte[3]))
    {
      *pTemp = byte[2];
      *pHumi = byte[0];
    }
    else
    {
      ok = ERR_CRC;           /* 数据读到但校验和不匹配 */
    }
  }
  taskEXIT_CRITICAL();

  /* 恢复输入并释放总线（上拉） */
  Dht11_SetInput();
  Dht11_WriteBit(1);
  (void)memset(byte, 0, sizeof(byte));
  return ok;
}

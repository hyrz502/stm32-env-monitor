/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef uint16_t LogData_t[3][10];
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
uint8_t CaliReceived(uint8_t* Data,uint8_t len);
uint8_t CaliXOR(uint8_t *Data,uint16_t len);	

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_3
#define LED_GPIO_Port GPIOA
#define Key2_Pin GPIO_PIN_6
#define Key2_GPIO_Port GPIOA
#define Key2_EXTI_IRQn EXTI9_5_IRQn
#define Key1_Pin GPIO_PIN_1
#define Key1_GPIO_Port GPIOB
#define Key1_EXTI_IRQn EXTI1_IRQn
#define DHT11_Pin GPIO_PIN_12
#define DHT11_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define Short_Press 0x78
#define Long_Press 0x56
#define UART_RX_BUF_LEN 128
#define MAXSIZE_U8 128
#define MAXSIZE_U16 65535
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

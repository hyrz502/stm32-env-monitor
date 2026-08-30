/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "DHT11.h"
#include "OLED.h"
#include "Task.h"
#include "usart.h"
#include "String.h"
#include "semphr.h"
#include "adc.h"
#include "dma.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define SERIAL_MAXSIZE 20		//串口接收最大值
#define FRAME_H 0xAA		//串口包帧头
#define FRAME_T 0x55		//串口包帧尾
#define LED_TWINKLE 0x99	//LED闪烁标志
#define LED_ON 0x88	//LED开启标志


#define BIT_TEMP_OVER (1U<<0)	//温度超阈值
#define BIT_AUTOALERM_ON (1U<<1)		//警报开启动
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
	uint8_t Temp=0;			//温度
	uint8_t Humi=0;			//湿度
	uint16_t light=0;		//光照
	uint8_t Threshold=30;		//警报阈值
	uint8_t Collect_Period=1;	//采集周期
	uint8_t PageFlag=1;			//页面标志
    uint8_t rx_buf[SERIAL_MAXSIZE];		//串口接收缓冲区
	uint8_t LEDflag=0;			//LED状态标志
	uint8_t Arrow=1;		//滑动箭头
	uint8_t CMD=0;			//命令帧
	char AutoAlarmFlag[5]="OFF";	//自动警报标志


/* USER CODE END Variables */
/* Definitions for CollectTask */
osThreadId_t CollectTaskHandle;
const osThreadAttr_t CollectTask_attributes = {
  .name = "CollectTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ShowTask */
osThreadId_t ShowTaskHandle;
const osThreadAttr_t ShowTask_attributes = {
  .name = "ShowTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for SerialTask */
osThreadId_t SerialTaskHandle;
const osThreadAttr_t SerialTask_attributes = {
  .name = "SerialTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for AlarmTask */
osThreadId_t AlarmTaskHandle;
const osThreadAttr_t AlarmTask_attributes = {
  .name = "AlarmTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for KeyTask */
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LogQueue */
osMessageQueueId_t LogQueueHandle;
const osMessageQueueAttr_t LogQueue_attributes = {
  .name = "LogQueue"
};
/* Definitions for CollectTimer */
osTimerId_t CollectTimerHandle;
const osTimerAttr_t CollectTimer_attributes = {
  .name = "CollectTimer"
};
/* Definitions for myMutex01 */
osMutexId_t myMutex01Handle;
const osMutexAttr_t myMutex01_attributes = {
  .name = "myMutex01"
};
/* Definitions for SerialBinarySem */
osSemaphoreId_t SerialBinarySemHandle;
const osSemaphoreAttr_t SerialBinarySem_attributes = {
  .name = "SerialBinarySem"
};
/* Definitions for ADCBinarySem */
osSemaphoreId_t ADCBinarySemHandle;
const osSemaphoreAttr_t ADCBinarySem_attributes = {
  .name = "ADCBinarySem"
};
/* Definitions for AlarmEvent */
osEventFlagsId_t AlarmEventHandle;
const osEventFlagsAttr_t AlarmEvent_attributes = {
  .name = "AlarmEvent"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartCollectTask(void *argument);
void StartShowTask(void *argument);
void StartSerialTask(void *argument);
void StartAlarmTask(void *argument);
void StartKeyTask(void *argument);
void Callback01(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	OLED_Init();
  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of myMutex01 */
  myMutex01Handle = osMutexNew(&myMutex01_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of SerialBinarySem */
  SerialBinarySemHandle = osSemaphoreNew(1, 0, &SerialBinarySem_attributes);

  /* creation of ADCBinarySem */
  ADCBinarySemHandle = osSemaphoreNew(1, 0, &ADCBinarySem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of CollectTimer */
  CollectTimerHandle = osTimerNew(Callback01, osTimerPeriodic, NULL, &CollectTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of LogQueue */
  LogQueueHandle = osMessageQueueNew (4, sizeof(LogData_t), &LogQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of CollectTask */
  CollectTaskHandle = osThreadNew(StartCollectTask, NULL, &CollectTask_attributes);

  /* creation of ShowTask */
  ShowTaskHandle = osThreadNew(StartShowTask, NULL, &ShowTask_attributes);

  /* creation of SerialTask */
  SerialTaskHandle = osThreadNew(StartSerialTask, NULL, &SerialTask_attributes);

  /* creation of AlarmTask */
  AlarmTaskHandle = osThreadNew(StartAlarmTask, NULL, &AlarmTask_attributes);

  /* creation of KeyTask */
  KeyTaskHandle = osThreadNew(StartKeyTask, NULL, &KeyTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* creation of AlarmEvent */
  AlarmEventHandle = osEventFlagsNew(&AlarmEvent_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartCollectTask */
/**
  * @brief  Function implementing the CollectTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartCollectTask */
void StartCollectTask(void *argument)
{
  /* USER CODE BEGIN StartCollectTask */
  /* Infinite loop */
	uint8_t CollectPeriodDefault=1;
	uint16_t Log[3][10]={0};
	uint8_t times=0;
	osTimerStart(CollectTimerHandle,1000);		//开始采集，只运行一次
	uint16_t Voltage_int = 0;
	
  for(;;)
  {
	  
	  ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
/*******互斥锁：保护全局变量（light,Temp,Humi,Collect_Period）**************************************/	  
	  osMutexAcquire(myMutex01Handle,500);
	  
	  if(CollectPeriodDefault!=Collect_Period)		//修改采集周期
	  {
		  osTimerStop(CollectTimerHandle);
		  osTimerStart(CollectTimerHandle,1000*Collect_Period);
		  CollectPeriodDefault=Collect_Period;
	  }
	  
  	  if(times>=10)
	  {
		  for(uint8_t i=0;i<9;i++)
		  {
			  Log[0][i]=Log[0][i+1];
			  Log[1][i]=Log[1][i+1];
			  Log[2][i]=Log[2][i+1];	  
		  }
		  times=9;
	  }
	  
	  if(LEDflag==0)
	  {
		  HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
	  }
	  
	  osSemaphoreAcquire(ADCBinarySemHandle,0);			//预清除信号量
	  HAL_ADC_Start_DMA(&hadc1,(uint32_t*)&light,1);	//光采集
	  osSemaphoreAcquire(ADCBinarySemHandle,osWaitForever);	
	  HAL_ADC_Stop_DMA(&hadc1);
	  DHT11_Samping(&Temp,&Humi);		//温湿度采集
	  
	  char sendbuf[64]={0};		//发送缓冲
	  if(strcmp(AutoAlarmFlag,"ON")==0)		//判断自动发送警报是否开启
	  { 
		  memset(sendbuf,0,sizeof(sendbuf));
		  
		  if(LEDflag==LED_TWINKLE&&Temp<Threshold)	//解除警报
		  {
			  CMD=0x83;
			  sprintf(sendbuf,"[CLEAR] Temp:%dC Threshold:%dC\r\n",Temp,Threshold);
		  }
		  else if(Temp<Threshold)		//正常帧
		  {
			  CMD=0x81;
			  sprintf(sendbuf,"[NORMAL] Temp:%dC Threshold:%dC\r\n",Temp,Threshold);
		  }
		  else if(Temp>=Threshold)			//警报帧
		  {
			 CMD=0x82;
			 sprintf(sendbuf,"[ALARM] Temp:%dC Threshold:%dC\r\n",Temp,Threshold);
		  }  
	  }
	  
	  if(Temp>=Threshold)		//温度过高事件标志
	  {
		  osEventFlagsSet(AlarmEventHandle,BIT_TEMP_OVER);
	  }
	  else
	  {
		  osEventFlagsClear(AlarmEventHandle,BIT_TEMP_OVER);
	  }
	  
	  
	  Voltage_int=light*3300/4095;
	  
		
	  
	  Log[0][times]=Temp;		//存入log日志
	  Log[1][times]=Humi;
	  Log[2][times]=Voltage_int;
	  
	  osMutexRelease(myMutex01Handle);	/**释放锁**/
	  
	  if(strcmp(AutoAlarmFlag,"ON")==0)		//判断自动发送警报是否开启--锁外再判断
	  {
		  while(huart1.gState==HAL_UART_STATE_BUSY_TX);
		  HAL_UART_Transmit_DMA(&huart1,(uint8_t*)sendbuf,strlen(sendbuf));
	  }
	  times++;
      osMessageQueuePut(LogQueueHandle,Log,0,0);
/******************************************************************************/	  
  }
  /* USER CODE END StartCollectTask */
}

/* USER CODE BEGIN Header_StartShowTask */
/**
* @brief Function implementing the ShowTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartShowTask */
void StartShowTask(void *argument)
{
  /* USER CODE BEGIN StartShowTask */
  /* Infinite loop */
	  uint32_t s = 0;
	  uint16_t min = 0;
	  uint8_t hour = 0;
	  uint8_t flag=0;
	  uint8_t Voltage_int = 0;
	  uint8_t Voltage_poi = 0;
  for(;;)
  {
	  
/************************************数据处理**************************************/	  
	  s++;
	  if(s>=60)
	  {
		  s=0;
		  min++;
	  }
	  if(min>=60)
	  {
		  min=0;
		  hour++;
	  }
	  
	  if(hour>=24) hour=0;
	  
	  flag=0;
	  osMutexAcquire(myMutex01Handle,osWaitForever);
	  flag=PageFlag;
	  Voltage_int=light*3300/4095/1000;
	  Voltage_poi=light*3300/4095/10%100;
	  osMutexRelease(myMutex01Handle);
/****************************************************************************/
	  
	  
/*********************************数据显示************************************/	
	  if(flag==1) 
	  {
		OLED_NewFrame();
		OLED_ShowString(1,0,"Time:");
	    OLED_ShowString(1,2,"Humi:");
	    OLED_ShowString(1,3,"Temp:");
		OLED_ShowString(1,4,"Light:");
	    
  	    
	    osMutexAcquire(myMutex01Handle,100);
        OLED_ShowNum(8,3,Temp,2);
	    OLED_ShowNum(8,2,Humi,2);

	    osMutexRelease(myMutex01Handle);
		OLED_ShowNum(8,4,Voltage_int,1);
		OLED_ShowChar(9,4,'.');
		OLED_ShowNum(10,4,Voltage_poi,2);
		
/********************************时间显示*****************************************/	    
	    OLED_ShowNum(6,0,hour,2);
	    OLED_ShowNum(9,0,min,2);
	    OLED_ShowNum(12,0,s,2);
	    OLED_ShowChar(8,0,':');
	    OLED_ShowChar(11,0,':');

	    OLED_ShowFrame();
	  }
/***************************************************************************/


/*********************************配置显示*************************************/
	  else if(flag==2)
	  {
		  OLED_NewFrame();
		  
		  osMutexAcquire(myMutex01Handle,osWaitForever);
		  OLED_ShowNum(11,3,Threshold,2);
		  OLED_ShowChar(17,Arrow,'<');
		  OLED_ShowString(12,2,AutoAlarmFlag);
		  OLED_ShowNum(8,1,Collect_Period,2);
		  osMutexRelease(myMutex01Handle);

		  OLED_ShowString(5,0,"CONFIG");
		  OLED_ShowString(0,1,"Period:");
		  
		  OLED_ShowChar(10,1,'s');
		  OLED_ShowString(0,3,"Threshold:");
		  OLED_ShowString(0,2,"AutoAlarm:");
		  
		  OLED_ShowFrame();
	  }
/***************************************************************************/

	  osDelay(999);
  }
  /* USER CODE END StartShowTask */
}

/* USER CODE BEGIN Header_StartSerialTask */
/**
* @brief Function implementing the SerialTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSerialTask */
void StartSerialTask(void *argument)
{
  /* USER CODE BEGIN StartSerialTask */
  /* Infinite loop */
	
/*****帧格式***************************************************************/
/*[0xAA] [LEN] [CMD] [DATA...] [CRC] [0x55]*/

/* LEN = CMD + DATA + CRC 的总字节数 */
/* CRC = 简单异或校验，从CMD到DATA结束 */
/*************************************************************************/
	uint16_t Voltage_int=0;
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1,rx_buf,sizeof(rx_buf));
	__HAL_DMA_DISABLE_IT(huart1.hdmarx,DMA_IT_HT);

	for(;;)
    {
		osSemaphoreAcquire(SerialBinarySemHandle, osWaitForever);
		uint8_t local_buf[SERIAL_MAXSIZE];
		uint8_t BAG[SERIAL_MAXSIZE];
		uint8_t i=0;
		uint16_t len=0;

		memcpy(local_buf,rx_buf,sizeof(local_buf));
		memset(rx_buf,0,sizeof(rx_buf));
		memset(BAG,0,sizeof(BAG));

		while(local_buf[i]!=FRAME_H)		//等待帧头
		{
			i++;
			if(i>=SERIAL_MAXSIZE) break;
											//无头
		}
		if(i>SERIAL_MAXSIZE-6)		//跳过无效帧
		{
			continue;
		}
		for(uint8_t j=0;j<SERIAL_MAXSIZE;j++)
		{
			BAG[j]=local_buf[i];
			i++;
			if(local_buf[i-1]==FRAME_T)		//正确结束
			{
				break;
			}
			else if(i>=SERIAL_MAXSIZE)		//有头没尾
			{						
				break;
			}
		}
		

		
		char SendBuf[64];					//定义发送缓冲
		memset(SendBuf,0,sizeof(SendBuf));
		
		for(uint8_t j=0;j<SERIAL_MAXSIZE;j++)		//计算实际包长-仅用于校验
		{
			len++;
			if(BAG[j]==0x55)
			{
				break;
			}
		}
				
		if(CaliReceived(BAG,len)==0)		//校验错误
		{
			memset(SendBuf,0,sizeof(SendBuf));				
			sprintf(SendBuf,"CaliReceive erorr\r\n");
			HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuf,strlen(SendBuf));
		}
		else		//校验成功
		{


			
			switch(BAG[2])
			{
				case 0x01:	//请求当前数据
			
					

					memset(SendBuf,0,sizeof(SendBuf));
					osMutexAcquire(myMutex01Handle,osWaitForever);
					Voltage_int=light*3300/4095;
					sprintf(SendBuf,"Temp:%d℃ Light:%dmv Threshold:%d℃ Interval:%ds\r\n",Temp,Voltage_int,Threshold,Collect_Period);
					osMutexRelease(myMutex01Handle);

					HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuf,strlen(SendBuf));
					
				
					
				break;
				case 0x02:		//设置采样间隔
					
					
					if(BAG[3]<=60&&BAG[3]>=1)
					{
						osMutexAcquire(myMutex01Handle,osWaitForever);
						Collect_Period=BAG[3];
						osMutexRelease(myMutex01Handle);
						
						memset(SendBuf,0,sizeof(SendBuf));
						sprintf(SendBuf,"OK\r\n");
						HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuf,strlen(SendBuf));
						
					}
					else
					{
						memset(SendBuf,0,sizeof(SendBuf));
						sprintf(SendBuf,"ERR\r\n");
						HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuf,strlen(SendBuf));
						
					}
					
				break;
				case 0x03:		//设置报警阈值
					
					
					if(BAG[3]<=127&&BAG[3]>=1)
					{
						osMutexAcquire(myMutex01Handle,osWaitForever);
						Threshold=BAG[3];
						osMutexRelease(myMutex01Handle);
						
						memset(SendBuf,0,sizeof(SendBuf));				
						sprintf(SendBuf,"OK\r\n");
						HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuf,strlen(SendBuf));
					}
					else
					{
						memset(SendBuf,0,sizeof(SendBuf));				
						sprintf(SendBuf,"ERR\r\n");
						HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuf,strlen(SendBuf));
					}
					
				break;	
				case 0x04:		//控制LED
					if(BAG[3]==0)
					{
						osMutexAcquire(myMutex01Handle,osWaitForever);
						LEDflag=0;
						osMutexRelease(myMutex01Handle);
						HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
					}
					else if(BAG[3]==1)
					{
						osMutexAcquire(myMutex01Handle,osWaitForever);
						LEDflag=LED_ON;
						osMutexRelease(myMutex01Handle);
						HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);			
					}
					else if(BAG[3]==2)
					{
						osMutexAcquire(myMutex01Handle,osWaitForever);
						LEDflag=LED_TWINKLE;
						osMutexRelease(myMutex01Handle);
					}
					break;
				case 0x05:		//请求历史数据
					memset(SendBuf,0,sizeof(SendBuf));
					uint16_t Log[3][10]={0};
					osMessageQueueGet(LogQueueHandle,Log,0,200);
				
					for(uint8_t i=0;i<10;i++)
					{				
						while(huart1.gState==HAL_UART_STATE_BUSY_TX);
						memset(SendBuf,0,strlen(SendBuf));
						sprintf(SendBuf,"%d: Temp:%d℃ Humi:%d%%RH Light:%dmv\r\n",i+1,Log[0][i],Log[1][i],Log[2][i]);
						HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuf,strlen(SendBuf));
					}
					break;
				case 0x06:			//警报开关
					if(BAG[3]==1)
					{
						osMutexAcquire(myMutex01Handle,osWaitForever);
						strcpy(AutoAlarmFlag,"ON");
						osMutexRelease(myMutex01Handle);
						osEventFlagsSet(AlarmEventHandle,BIT_AUTOALERM_ON);		//自动警报开
						
						while(huart1.gState==HAL_UART_STATE_BUSY_TX);
						memset(SendBuf,0,strlen(SendBuf));
						sprintf(SendBuf,"AutoAlarm ON\r\n");
						HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuf,strlen(SendBuf));
					}
					else
					{
						osMutexAcquire(myMutex01Handle,osWaitForever);
						strcpy(AutoAlarmFlag,"OFF");
						osMutexRelease(myMutex01Handle);
						osEventFlagsClear(AlarmEventHandle,BIT_AUTOALERM_ON);	//自动警报关
						
						while(huart1.gState==HAL_UART_STATE_BUSY_TX);
						memset(SendBuf,0,strlen(SendBuf));
						sprintf(SendBuf,"AutoAlarm OFF\r\n");
						HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuf,strlen(SendBuf));
					}
					break;					
				default:
						memset(SendBuf,0,sizeof(SendBuf));				
						sprintf(SendBuf,"Cmd is warning\r\n");
						HAL_UART_Transmit_DMA(&huart1,(uint8_t*)SendBuf,strlen(SendBuf));	
			}
		}
		
			
    }
  /* USER CODE END StartSerialTask */
}

/* USER CODE BEGIN Header_StartAlarmTask */
/**
* @brief Function implementing the AlarmTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartAlarmTask */
void StartAlarmTask(void *argument)
{
  /* USER CODE BEGIN StartAlarmTask */
  /* Infinite loop */
	uint8_t LED_local=0;
	
	
  for(;;)
  {
		uint32_t ret=osEventFlagsWait(AlarmEventHandle,BIT_TEMP_OVER|BIT_AUTOALERM_ON,osFlagsWaitAll|osFlagsNoClear,300);
	  
		osMutexAcquire(myMutex01Handle,osWaitForever);
 
	  	if((ret&(BIT_TEMP_OVER|BIT_AUTOALERM_ON))==(BIT_TEMP_OVER|BIT_AUTOALERM_ON))	//判断全部事件标志置位
		{
			LEDflag=LED_TWINKLE;
		}
		else
		{
			LEDflag=0;
		}
		
		LED_local=LEDflag;
		
		osMutexRelease(myMutex01Handle);
		
		
		if(LED_local==LED_TWINKLE)		//LED警报闪烁
		{
			HAL_GPIO_TogglePin(LED_GPIO_Port,LED_Pin);
		}
		else
		{
			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
		}
		osDelay(300);
  }
  /* USER CODE END StartAlarmTask */
}

/* USER CODE BEGIN Header_StartKeyTask */
/**
* @brief Function implementing the KeyTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartKeyTask */
void StartKeyTask(void *argument)
{
  /* USER CODE BEGIN StartKeyTask */
  /* Infinite loop */
	
	
  for(;;)
  {
	  
	  ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
	  if(HAL_GPIO_ReadPin(Key1_GPIO_Port,Key1_Pin)==GPIO_PIN_RESET)			//按键1
	  {
		  osDelay(15);
		  if(HAL_GPIO_ReadPin(Key1_GPIO_Port,Key1_Pin)==GPIO_PIN_RESET)
		  {
			  uint8_t Press_Time=0;
			  uint16_t StartTime=HAL_GetTick();
			  while(HAL_GPIO_ReadPin(Key1_GPIO_Port,Key1_Pin)==GPIO_PIN_RESET)
			  {
				  uint16_t EndTime=HAL_GetTick();
				  if((EndTime-StartTime)>=500)
				  {				  
					  Press_Time=Long_Press;
					  break;
				  }
				  else
				  {
					  Press_Time=Short_Press;
				  }
			  }
			  if(Press_Time==Long_Press)
			  {
				  osMutexAcquire(myMutex01Handle,osWaitForever);
				  if(PageFlag==1) PageFlag=2;
				  else if(PageFlag==2) PageFlag=1;
				  osMutexRelease(myMutex01Handle);
			  }
			  else if(Press_Time==Short_Press)
			  {
				  if(PageFlag==2)
				  {
					  osMutexAcquire(myMutex01Handle,osWaitForever);
					  if(Arrow==7) Arrow=0;
					  Arrow++; 
					  osMutexRelease(myMutex01Handle);
				  }

			  }
		  }
	  }
	  if(HAL_GPIO_ReadPin(Key2_GPIO_Port,Key2_Pin)==GPIO_PIN_RESET)			//按键2
	  {
		  osDelay(15);
		  if(HAL_GPIO_ReadPin(Key2_GPIO_Port,Key2_Pin)==GPIO_PIN_RESET)
		  {
			  uint8_t Press_Time=0;
			  uint16_t StartTime=HAL_GetTick();
			  while(HAL_GPIO_ReadPin(Key2_GPIO_Port,Key2_Pin)==GPIO_PIN_RESET)
			  {
				  uint16_t EndTime=HAL_GetTick();
				  if((EndTime-StartTime)>=500)
				  {
					  Press_Time=Long_Press;
					  break;
				  }
				  else
				  {
					  Press_Time=Short_Press;
				  }
			  }
			  
			  if(Press_Time==Long_Press) 		//长按
			  {
				  if(PageFlag==2)
				  {
					  switch(Arrow)
					  {
						  case 1:
							  osMutexAcquire(myMutex01Handle,osWaitForever);
							  Collect_Period=1;
							  osMutexRelease(myMutex01Handle);
						  break;
						  case 2:
							  osMutexAcquire(myMutex01Handle,osWaitForever);
							  strcpy(AutoAlarmFlag,"OFF");
							  osMutexRelease(myMutex01Handle);
							  osEventFlagsClear(AlarmEventHandle,BIT_AUTOALERM_ON); 	//清除自动警报事件标志
						  break;
						  case 3:
							  osMutexAcquire(myMutex01Handle,osWaitForever);
							  Threshold=30;
							  osMutexRelease(myMutex01Handle);
						  break;
					  }
				  }
			  }
			  else if(Press_Time==Short_Press)		//短按
			  {
				  if(PageFlag==2)
				  {
					  switch(Arrow)
					  { 
						  case 1:
							  osMutexAcquire(myMutex01Handle,osWaitForever);
							  if(Collect_Period==MAXSIZE_U8) Collect_Period=0;
							  Collect_Period++;
							  osMutexRelease(myMutex01Handle);
						  break;
						  case 2:
							  osMutexAcquire(myMutex01Handle,osWaitForever);
							  if(strcmp(AutoAlarmFlag,"OFF")==0)
							  {
								  strcpy(AutoAlarmFlag,"ON");
								  osEventFlagsSet(AlarmEventHandle,BIT_AUTOALERM_ON);	//开启自动警报事件标志
							  }
							  else
							  {
								  strcpy(AutoAlarmFlag,"OFF");
								  osEventFlagsClear(AlarmEventHandle,BIT_AUTOALERM_ON);		//清除自动警报事件标志
							  }
							  osMutexRelease(myMutex01Handle);
						  break;
						  case 3:
							  osMutexAcquire(myMutex01Handle,osWaitForever);
							  if(Collect_Period==MAXSIZE_U8) Threshold=30;
							  Threshold++;
							  osMutexRelease(myMutex01Handle);
						  break;
					  }
					  
				  }

			  }
		  }
	  }

  }
  /* USER CODE END StartKeyTask */
}

/* Callback01 function */
void Callback01(void *argument)
{
  /* USER CODE BEGIN Callback01 */
	xTaskNotifyGive(CollectTaskHandle);

  /* USER CODE END Callback01 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) 				//串口DMA
{
	if(huart->Instance==USART1)
	{
		if(Size==0) return;
		
		
		BaseType_t HigherPriorityTaskWoken=pdFALSE;
		xSemaphoreGiveFromISR(SerialBinarySemHandle,&HigherPriorityTaskWoken);
		portYIELD_FROM_ISR(HigherPriorityTaskWoken);
		
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1,rx_buf,sizeof(rx_buf));			//开启接收DMA
		__HAL_DMA_DISABLE_IT(huart1.hdmarx,DMA_IT_HT);			//关闭DMA传输过半
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)					//按键发送通知任务
{
	if(GPIO_Pin==Key1_Pin||GPIO_Pin==Key2_Pin)
	{
		BaseType_t HigherPriorityTaskWoken=pdFALSE;
		vTaskNotifyGiveFromISR(KeyTaskHandle,&HigherPriorityTaskWoken);
		portYIELD_FROM_ISR(HigherPriorityTaskWoken);
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)					//发送ADC采集完成信号量
{
	if(hadc->Instance==ADC1)
	{
		BaseType_t HigherPriorityTaskWoken=pdFALSE;
		xSemaphoreGiveFromISR(ADCBinarySemHandle,&HigherPriorityTaskWoken);
		portYIELD_FROM_ISR(HigherPriorityTaskWoken);
	}
}

uint8_t CaliXOR(uint8_t *Data,uint16_t len)					//简单异或校验XOR
{
	uint8_t xor_val=0;
	for(uint8_t i=2;i<len-2;i++)		//所有的[CMD]和[DATA]
	{
		xor_val^=Data[i];
	}
	return xor_val;
}
uint8_t CaliReceived(uint8_t* Data,uint8_t len)
{
	if(Data[0]!=0xAA||Data[len-1]!=0x55) return 0;	//首尾
	if(Data[len-2]!=CaliXOR(Data,len))  return 0;		//异或校验[XOR]
	return 1;
}

int fputc(int ch,FILE*f)		//printf重定向
{
	HAL_UART_Transmit(&huart1,(uint8_t*)&ch,1,100);
	return ch;
}

/* USER CODE END Application */


/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
#include "rtc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JUNE;
  DateToUpdate.Date = 5;
  DateToUpdate.Year = 26;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours =1;
  sAlarm.AlarmTime.Minutes = 0;
  sAlarm.AlarmTime.Seconds = 5;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();

    /* RTC interrupt Init */
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* RTC interrupt Deinit */
    HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
	RTC_TimeTypeDef time;
  RTC_DateTypeDef date;
  uint8_t *week[8] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
void show_time(void)
{
  // OLED_Clear();
	if(HAL_RTC_GetTime(&hrtc,&time,RTC_FORMAT_BIN) == HAL_OK)
	{
		HAL_RTC_GetDate(&hrtc,&date,RTC_FORMAT_BIN);
    //显示日期
    OLED_ShowNum(24,0,20,2,16,0);
    if (date.Year <10)
    {
      OLED_ShowNum(40,0,0,1,16,0);
      OLED_ShowNum(48,0,date.Year,1,16,0);
    }
    else
    {
      OLED_ShowNum(40,0,date.Year,2,16,0);
    }
    OLED_ShowChar(56,0,'/',16,0);
    if (date.Month <10)
    {
      OLED_ShowNum(64,0,0,1,16,0);
      OLED_ShowNum(72,0,date.Month,1,16,0);
    }
    else
    {
      OLED_ShowNum(64,0,date.Month,2,16,0);
    }
    OLED_ShowChar(80,0,'/',16,0);
    if (date.Date <10)
    {
      OLED_ShowNum(88,0,0,1,16,0);
      OLED_ShowNum(96,0,date.Date,1,16,0);
    }
    else
    {
      OLED_ShowNum(88,0,date.Date,2,16,0);
    }
    //显示时间
		if(time.Hours<10){
      OLED_ShowNum(32,2,0,1,16,0);
      OLED_ShowNum(40,2,time.Hours,1,16,0);
		}
		else
		OLED_ShowNum(32,2,time.Hours,2,16,0);
		OLED_ShowChar(48,2,':',16,0);
    if(time.Minutes<10){
      OLED_ShowNum(56,2,0,1,16,0);
      OLED_ShowNum(64,2,time.Minutes,1,16,0);
		}
		else
      OLED_ShowNum(56,2,time.Minutes,2,16,0);
		OLED_ShowChar(72,2,':',16,0);
		if(time.Seconds<10){
      OLED_ShowNum(80,2,0,1,16,0);
      OLED_ShowNum(88,2,time.Seconds,1,16,0);
		}
		else
				OLED_ShowNum(80,2,time.Seconds,2,16,0);
  }
  //显示星期
  OLED_ShowChar(32,4,'W',16,0);
  OLED_ShowChar(40,4,'e',16,0);
  OLED_ShowChar(48,4,'e',16,0);
  OLED_ShowChar(56,4,'k',16,0);
  OLED_ShowChar(64,4,':',16,0);
  OLED_ShowString(72,4,week[date.WeekDay],16,0);
  
}




// 时间数值限制函数（保证时间不会越界）
void numLimit(uint8_t *num, uint8_t type)
{
    switch(type)
    {
        case 0: // 年  0~99
            if(*num > 99)  *num = 0;
            if(*num < 0)   *num = 99;
            break;
            
        case 1: // 月  1~12
            if(*num < 1)   *num = 12;
            if(*num > 12)  *num = 1;
            break;
            
        case 2: // 日  1~31
            if(*num < 1)   *num = 31;
            if(*num > 31)  *num = 1;
            break;
            
        case 3: // 星期 1~7
            if(*num < 1)   *num = 7;
            if(*num > 7)   *num = 1;
            break;
            
        case 4: // 时  0~23
            if(*num > 23)  *num = 0;
            if(*num < 0)   *num = 23;
            break;
            
        case 5: // 分  0~59
        case 6: // 秒  0~59
            if(*num > 59)  *num = 0;
            if(*num < 0)   *num = 59;
            break;
    }
}


volatile uint8_t alarm_flag = 0;
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  alarm_flag = 1; // 闹钟中断标志     
}
/* USER CODE END 1 */

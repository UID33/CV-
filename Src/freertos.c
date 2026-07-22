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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define pageNone    0
#define pageMain    1
#define pageTimeSet 2
#define pageAlarmSet 3
#define pageSecond  4
#define pageHeart   5
#define pageStep    6
#define pageInfo    7

volatile uint8_t Settings = 0;
volatile uint8_t StepFlag = 0;

#define TOTAL_PAGE 7


#define HEART_PERIOD    2   // PulseSensor心率标准采样周期 2ms（500Hz）
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
static uint8_t pageNum = pageNone;
uint8_t secondflag = 0; // 秒钟标志位
sportsInfo_t *sportInfo;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

void resumeScreen(uint8_t pageNum);
/* USER CODE END Variables */
/* Definitions for keyTask */
osThreadId_t keyTaskHandle;
const osThreadAttr_t keyTask_attributes = {
  .name = "keyTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for mainMenuTask */
osThreadId_t mainMenuTaskHandle;
const osThreadAttr_t mainMenuTask_attributes = {
  .name = "mainMenuTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for timeSetTask */
osThreadId_t timeSetTaskHandle;
const osThreadAttr_t timeSetTask_attributes = {
  .name = "timeSetTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for alarmSetTask */
osThreadId_t alarmSetTaskHandle;
const osThreadAttr_t alarmSetTask_attributes = {
  .name = "alarmSetTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for secondTask */
osThreadId_t secondTaskHandle;
const osThreadAttr_t secondTask_attributes = {
  .name = "secondTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for alarmDispTask */
osThreadId_t alarmDispTaskHandle;
const osThreadAttr_t alarmDispTask_attributes = {
  .name = "alarmDispTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for heartTask */
osThreadId_t heartTaskHandle;
const osThreadAttr_t heartTask_attributes = {
  .name = "heartTask",
  .stack_size = 128 * 6,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for stepTask */
osThreadId_t stepTaskHandle;
const osThreadAttr_t stepTask_attributes = {
  .name = "stepTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for walkTask */
osThreadId_t walkTaskHandle;
const osThreadAttr_t walkTask_attributes = {
  .name = "walkTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for timeCheckTask */
osThreadId_t timeCheckTaskHandle;
const osThreadAttr_t timeCheckTask_attributes = {
  .name = "timeCheckTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for infoMenuTask */
osThreadId_t infoMenuTaskHandle;
const osThreadAttr_t infoMenuTask_attributes = {
  .name = "infoMenuTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for queueKeyHandle */
osMessageQueueId_t queueKeyHandleHandle;
const osMessageQueueAttr_t queueKeyHandle_attributes = {
  .name = "queueKeyHandle"
};
/* Definitions for tmrSingleHandler */
osTimerId_t tmrSingleHandlerHandle;
const osTimerAttr_t tmrSingleHandler_attributes = {
  .name = "tmrSingleHandler"
};
/* Definitions for tmrCycleHandler */
osTimerId_t tmrCycleHandlerHandle;
const osTimerAttr_t tmrCycleHandler_attributes = {
  .name = "tmrCycleHandler"
};
/* Definitions for smaMutexHandle */
osMutexId_t smaMutexHandleHandle;
const osMutexAttr_t smaMutexHandle_attributes = {
  .name = "smaMutexHandle"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osSemaphoreId_t binarySemHandle;

osSemaphoreAttr_t binarySem_attributes = {
  .name = "binarySem"
};

osMutexId_t UsartMutexHandle;
const osMutexAttr_t UsartMutexHandle_attributes = {
  .name = "UsartMutexHandle"
};

/* USER CODE END FunctionPrototypes */

void StartKeyTask(void *argument);
void StartMainMenuTask(void *argument);
void StartTimeSetTask(void *argument);
void StartAlarmSetTask(void *argument);
void StartSecondTask(void *argument);
void StartAlarmDispTask(void *argument);
void StartHeartTask(void *argument);
void StartStepTask(void *argument);
void StartWalkTask(void *argument);
void StartTimeCheckTask(void *argument);
void StartInfoMenuTask(void *argument);
void tmrSingleCallback(void *argument);
void tmrCycleCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of smaMutexHandle */
  smaMutexHandleHandle = osMutexNew(&smaMutexHandle_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
    UsartMutexHandle = osMutexNew(&UsartMutexHandle_attributes);
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of tmrSingleHandler */
  tmrSingleHandlerHandle = osTimerNew(tmrSingleCallback, osTimerOnce, NULL, &tmrSingleHandler_attributes);

  /* creation of tmrCycleHandler */
  tmrCycleHandlerHandle = osTimerNew(tmrCycleCallback, osTimerPeriodic, NULL, &tmrCycleHandler_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of queueKeyHandle */
  queueKeyHandleHandle = osMessageQueueNew (2, sizeof(uint8_t), &queueKeyHandle_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of keyTask */
  keyTaskHandle = osThreadNew(StartKeyTask, NULL, &keyTask_attributes);

  /* creation of mainMenuTask */
  mainMenuTaskHandle = osThreadNew(StartMainMenuTask, NULL, &mainMenuTask_attributes);

  /* creation of timeSetTask */
  timeSetTaskHandle = osThreadNew(StartTimeSetTask, NULL, &timeSetTask_attributes);

  /* creation of alarmSetTask */
  alarmSetTaskHandle = osThreadNew(StartAlarmSetTask, NULL, &alarmSetTask_attributes);

  /* creation of secondTask */
  secondTaskHandle = osThreadNew(StartSecondTask, NULL, &secondTask_attributes);

  /* creation of alarmDispTask */
  alarmDispTaskHandle = osThreadNew(StartAlarmDispTask, NULL, &alarmDispTask_attributes);

  /* creation of heartTask */
  heartTaskHandle = osThreadNew(StartHeartTask, NULL, &heartTask_attributes);

  /* creation of stepTask */
  stepTaskHandle = osThreadNew(StartStepTask, NULL, &stepTask_attributes);

  /* creation of walkTask */
  walkTaskHandle = osThreadNew(StartWalkTask, NULL, &walkTask_attributes);

  /* creation of timeCheckTask */
  timeCheckTaskHandle = osThreadNew(StartTimeCheckTask, NULL, &timeCheckTask_attributes);

  /* creation of infoMenuTask */
  infoMenuTaskHandle = osThreadNew(StartInfoMenuTask, NULL, &infoMenuTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  binarySemHandle = osSemaphoreNew(1, 0, &binarySem_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
    suspendScreen();
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartKeyTask */
/**
  * @brief  Function implementing the keyTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartKeyTask */
void StartKeyTask(void *argument)
{
  /* USER CODE BEGIN StartKeyTask */
  /* Infinite loop */
	uint8_t key_value = 0;

  for(;;)
  {        
		Key_Scan();
		key_value = Key_GetResult();
    if (alarm_flag)
    {
      alarm_flag = 0;
      // OLED_Clear();
      suspendScreen();
      osSemaphoreRelease(binarySemHandle);
    }

    // OLED_ShowNum(0, 0, key_value, 2, 16,0);
    if (key_value)
    {
      if (osMutexAcquire(smaMutexHandleHandle,osWaitForever) == osOK)
      {
        OLED_ShowNum(0, 0, key_value, 2, 16, 0);
        if(key_value == 42)
          {
            suspendScreen();
            OLED_Clear();
            pageNum = pageNone;
          }
        else if (key_value == 11)
          {
            suspendScreen();
            if(++pageNum > TOTAL_PAGE)
            pageNum = pageMain;
            OLED_Clear();
            resumeScreen(pageNum);
          }
        osMutexRelease(smaMutexHandleHandle);
      }
        osMessageQueuePut(queueKeyHandleHandle, &key_value, 0, 10);
        // osTimerStart(tmrSingleHandlerHandle, 6000);
    }
    if(Settings)
    {
      Settings = 0;
      suspendScreen();
      pageNum = pageMain;
      resumeScreen(pageNum);
    }
    // osThreadResume(infoMenuTaskHandle);
    osDelay(10); // 10ms 扫描一次按键  
  }
  /* USER CODE END StartKeyTask */
}

/* USER CODE BEGIN Header_StartMainMenuTask */
/**
* @brief Function implementing the mainMenuTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMainMenuTask */
void StartMainMenuTask(void *argument)
{

  /* USER CODE BEGIN StartMainMenuTask */
  /* Infinite loop */
  uint8_t keyValue = 0;
  // static uint8_t i = 0;
  // OLED_Clear();
  // OLED_ShowNum(0, 6, i++, 2, 16,0);
  for(;;)
  {
    if (osMutexAcquire(smaMutexHandleHandle, osWaitForever) == osOK)
    {
    show_time();
    }
    osMutexRelease(smaMutexHandleHandle);
    osDelay(50);
    osMessageQueueGet(queueKeyHandleHandle, &keyValue, NULL, 10);

  }
  /* USER CODE END StartMainMenuTask */
}

/* USER CODE BEGIN Header_StartTimeSetTask */
/**
* @brief Function implementing the timeSetTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTimeSetTask */
void StartTimeSetTask(void *argument)
{
  /* USER CODE BEGIN StartTimeSetTask */
  /* Infinite loop */
    static uint8_t timeSetPosi = 0;
    uint8_t keyValue, flashFlag = 0;
    int8_t dateTime[7] = {0};  /* 2021-07-16 Fri 18:16:16 */
    char cDateTime[30];
    /* 读取 RTC 当前时间和日期 */
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);

    /* 放入数组，方便闪烁显示设置的时间 */
    dateTime[0] = date.Year;
    dateTime[1] = date.Month;
    dateTime[2] = date.Date;
    dateTime[3] = date.WeekDay;
    dateTime[4] = time.Hours;
    dateTime[5] = time.Minutes;
    dateTime[6] = time.Seconds;
    const uint8_t *week[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  for(;;)
  {
    /* 刷新界面 */
    if (osMutexAcquire(smaMutexHandleHandle, osWaitForever) == osOK)
    {
        OLED_ShowString(32, 0, "TimeSet!", 16, 0);
        sprintf(cDateTime, "20%02d/%02d/%02d", dateTime[0], dateTime[1],
                dateTime[2]);
        OLED_ShowString(20, 2, cDateTime, 16, 0);
        sprintf(cDateTime,"week:%s", week[dateTime[3]]);
        OLED_ShowString(32, 4, cDateTime, 16, 0);
        sprintf(cDateTime, "%02d:%02d:%02d", dateTime[4], dateTime[5], dateTime[6]);
        OLED_ShowString(30, 6, cDateTime, 16, 0);

        if (flashFlag) /* 设置闪烁 */
        {
            if (timeSetPosi < 3) /* 年-月-日 */
                OLED_ShowString((20 + 24 + timeSetPosi * 24), 2, (uint8_t *)" ", 16, 0);
            else if (timeSetPosi == 3) /* 星期 */
                OLED_ShowString(72, 4, "   ", 16, 0);
            else /* 时:分:秒 */
                OLED_ShowString((38 + (timeSetPosi - 4) * 24), 6, (uint8_t *)" ", 16, 0);
        }
    }
    osMutexRelease(smaMutexHandleHandle);
        // OLED_RefreshGram();

        /* 获取按键值（示例，需根据实际按键队列实现） */
        if (osMessageQueueGet(queueKeyHandleHandle, &keyValue, NULL, 10) == osOK)
        {
          switch (keyValue)
          {
          case 21:
            dateTime[timeSetPosi]++;
            numLimit(&dateTime[timeSetPosi], timeSetPosi);
            break;
          case 31:
            dateTime[timeSetPosi]--;
            numLimit(&dateTime[timeSetPosi], timeSetPosi);
            break;
          case 41:
            if (++timeSetPosi > 6)
            {
              timeSetPosi = 0;
              /* 设置 RTC 时间和日期 */
            }
            break;
            case 12:
              date.Year = dateTime[0];
              date.Month = dateTime[1];
              date.Date = dateTime[2];
              date.WeekDay = dateTime[3];
              time.Hours = dateTime[4];
              time.Minutes = dateTime[5];
              time.Seconds = dateTime[6];
              HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
              HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
              Settings = 1;
          }
        }
      
        flashFlag = !flashFlag;  /* 切换闪烁状态 */
        /* 根据按键值调整时间设置位置或增减数值，并调用 numLimit() 进行边界检查 */

        osDelay(500);  // CMSIS-V2 延时，单位：tick（假设 1 tick = 1ms）
  }
  /* USER CODE END StartTimeSetTask */
}

/* USER CODE BEGIN Header_StartAlarmSetTask */
/**
* @brief Function implementing the alarmSetTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartAlarmSetTask */
void StartAlarmSetTask(void *argument)
{
  /* USER CODE BEGIN StartAlarmSetTask */
  /* Infinite loop */
    static uint8_t timeSetPosi = 4;
    uint8_t keyValue, flashFlag = 0;
    char cDateTime[16];
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    RTC_AlarmTypeDef alarmSet;
    uint8_t alarmbuff[4] = {time.Hours, time.Minutes, time.Seconds,0};

  for(;;)
  {
    /* 刷新界面 */
  if(osMutexAcquire(smaMutexHandleHandle, osWaitForever) == osOK)
   { 
    OLED_ShowString(32, 2, "AlarmSet", 16, 0);
    sprintf(cDateTime, "%02d:%02d:%02d", alarmbuff[0], alarmbuff[1], alarmbuff[2]);
    OLED_ShowString(32, 4, cDateTime, 16, 0);

    /* 设置值闪烁 */
    if(flashFlag)
    {
        OLED_ShowString((40 + (timeSetPosi-4)*24), 4, " ", 16, 0);
    }
  
    osMutexRelease(smaMutexHandleHandle);
   } // OLED_RefreshGram();

    /* 读取按键队列 */
    if(osMessageQueueGet(queueKeyHandleHandle, &keyValue, NULL, 0) == osOK)
    {
        switch(keyValue)
        {
            case 21: /* 数值加 */
                           
                alarmbuff[timeSetPosi-4]++;
                numLimit(&alarmbuff[timeSetPosi-4], timeSetPosi);
                break;

            case 31: /* 数值减 */
                alarmbuff[timeSetPosi-4]--;
                numLimit(&alarmbuff[timeSetPosi-4], timeSetPosi);
                break;

            case 41: /* 设置位置改变 */  
                if(++timeSetPosi > 6) /* 完成一轮设置，保存 */
                    timeSetPosi = 4;
                break;
            case 22:
                    alarmSet.AlarmTime.Hours = alarmbuff[0];
                    alarmSet.AlarmTime.Minutes = alarmbuff[1];
                    alarmSet.AlarmTime.Seconds = alarmbuff[2];
                    HAL_RTC_SetAlarm_IT(&hrtc, &alarmSet, RTC_FORMAT_BIN);
                    if(osMutexAcquire(smaMutexHandleHandle, osWaitForever) == osOK)
                    { 
                      OLED_Clear();
                      OLED_ShowString(32, 2, "AlarmSet", 16, 0);
                      osDelay(1000); 
                      Settings = 1;
                      timeSetPosi = 4;
                      osMutexRelease(smaMutexHandleHandle);
                    }
        }
    }
    flashFlag = !flashFlag;           /* 闪烁标志 */
    osDelay(500); 
  }
  /* USER CODE END StartAlarmSetTask */
}

/* USER CODE BEGIN Header_StartSecondTask */
/**
* @brief Function implementing the secondTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSecondTask */
void StartSecondTask(void *argument)
{
    uint8_t keyValue;

    TickType_t lastWakeTime;

    lastWakeTime = xTaskGetTickCount();

    uint32_t lastDisplayTick = 0;

    uint8_t cDateTime[16];

    volatile uint32_t stopwatch_ms = 0;
    volatile uint8_t stopwatch_run = 0;

    for(;;)
    {
        /* 精确10ms周期 */
        if(secondflag == 1)
        {
          lastWakeTime = xTaskGetTickCount();
          secondflag = 0;
        }
        
        lastWakeTime += pdMS_TO_TICKS(10);
        osDelayUntil(lastWakeTime);

        /* 按键处理 */
        if(osMessageQueueGet(queueKeyHandleHandle,&keyValue,NULL,0) == osOK)
        {
            if(keyValue == 41)
            {
                stopwatch_run = !stopwatch_run;
            }
            else if(keyValue == 12 && !stopwatch_run)
            {
                stopwatch_ms = 0;
                OLED_ShowString(32,4, "00:00:00", 16, 0);
            }
        }

        /* 精确计时 */
        if(stopwatch_run)
        {
            stopwatch_ms += 10;
        }

        /* OLED只50ms刷新一次 */
        if((xTaskGetTickCount() - lastDisplayTick) >= 50)
        {
            lastDisplayTick = xTaskGetTickCount();

            uint32_t total = stopwatch_ms;

            uint8_t min  = total / 60000;
            uint8_t sec  = (total % 60000) / 1000;
            uint8_t msec = (total % 1000) / 10;

            if(osMutexAcquire(smaMutexHandleHandle,
                              5) == osOK)
            {
                sprintf(cDateTime, "%02d:%02d:%02d", min, sec, msec);
                OLED_ShowString(32, 2, "Stopwatch", 16, 0);
                OLED_ShowString(32, 4, cDateTime, 16, 0);

                osMutexRelease(smaMutexHandleHandle);
            }
        }
        // osDelay(1);
    }
}

/* USER CODE BEGIN Header_StartAlarmDispTask */
/**
* @brief Function implementing the alarmDispTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartAlarmDispTask */
void StartAlarmDispTask(void *argument)
{
  /* USER CODE BEGIN StartAlarmDispTask */
  /* Infinite loop */
  uint8_t ucKeyValue;
  for(;;)
  {
    if(osSemaphoreAcquire(binarySemHandle, osWaitForever) == osOK)
    {
      /* 闹钟响起，显示闹钟界面 */
    if(osMutexAcquire(smaMutexHandleHandle,5) == osOK)
      {
        OLED_Clear();
        OLED_ClearALLGRAM();
        OLED_DrawBmp(gImage_123, 32, 8, 64, 64, 1);
        OLED_RefreshGram();
        // osDelay(50);
        OLED_ShowChinese(40,1,2,16,0);
        OLED_ShowChinese(56,1,3,16,0);
        OLED_ShowChinese(72,1,4,16,0);
        OLED_ShowChar(88, 1, '!', 16, 0);
        osMutexRelease(smaMutexHandleHandle);
      }
    __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,50);
    while(1)
    {
    if(osMessageQueueGet(queueKeyHandleHandle, &ucKeyValue, NULL, 0) == osOK)
      {
        if(ucKeyValue == 41 || ucKeyValue ==11)
        {
          if(ucKeyValue == 11)
            // --pageNum;
          if(osMutexAcquire(smaMutexHandleHandle,5) == osOK)
          {
            OLED_Clear();
            OLED_ClearALLGRAM();
            osMutexRelease(smaMutexHandleHandle);
          }
          __HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_3,100);
          break;
        }
        // __HAL_TIM_SET_COMPARE(&time);
        
          // if(ucKeyValue == KEYPWR_PRES)
          // {
          //     /* CMSIS-RTOS V2 定时器启停控制 */
          //     if(osTimerIsRunning(tmrCycleHandlerHandle))
          //     {
          //         osTimerStop(tmrCycleHandlerHandle);
          //         tmrFlag = 0;
          //     }
          //     else
          //     {
          //         osTimerStart(tmrCycleHandlerHandle, 10);
          //         tmrFlag = 1;
          //     }
        }
      }
      osDelay(10);
      resumeScreen(pageNum);
    }
    // osDelay(50);
  }
    
    // osDelay(50);
  /* USER CODE END StartAlarmDispTask */
}

/* USER CODE BEGIN Header_StartHeartTask */
/**
* @brief Function implementing the heartTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartHeartTask */
void StartHeartTask(void *argument)
{
    uint8_t keyValue;
    uint8_t runFlag = 0;
    uint32_t lastRefreshTick = 0;
    const uint32_t REFRESH_INTERVAL_MS = 500;
    static uint16_t last_adc = 2048;
    PulseSensorHandle_t heart_sensor;
    uint8_t *wave_buf = NULL;
    uint32_t uxPreviousWakeTime;
    
    PulseSensor_Init(&heart_sensor);
    
    uxPreviousWakeTime = osKernelGetTickCount();
    
    for(;;)
    {
        if(osMessageQueueGet(queueKeyHandleHandle, &keyValue, NULL, 0) == osOK)
        {
            if(keyValue == 41)
            {
                runFlag = !runFlag;
                if(runFlag)
                {
                    OLED_Clear();
                    OLED_DrawBmp(gImage_456,80, 0, 36, 31, 1);
                    OLED_RefreshGram();
                    OLED_ClearALLGRAM();
                    
                    HAL_ADC_Start(&hadc1);
                    PulseSensor_Reset(&heart_sensor);
                    lastRefreshTick = osKernelGetTickCount();
                    uxPreviousWakeTime = osKernelGetTickCount();
                }
                else
                {
                    HAL_ADC_Stop(&hadc1);
                    if(osMutexAcquire(smaMutexHandleHandle, 5) == osOK)
                    {
                        OLED_Clear();
                        OLED_ClearGRAM();
                        OLED_ShowString(32, 2, "HeartRate", 16, 0);
                        OLED_ShowString(32, 4, "Stopped", 16, 0);
                        OLED_RefreshGram();
                        osMutexRelease(smaMutexHandleHandle);
                    }
                }
            }
        }

        if(runFlag)
        {
            uxPreviousWakeTime += 2;
            osDelayUntil(uxPreviousWakeTime);
            
              // ? 稳定读取 ADC
            if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK)
            {
                last_adc = HAL_ADC_GetValue(&hadc1);
            }

            uint16_t adcValue = last_adc;
            int current_bpm = PulseSensor_Process(&heart_sensor, adcValue);
            
            uint32_t currentTick = osKernelGetTickCount();
            if((currentTick - lastRefreshTick) >= REFRESH_INTERVAL_MS)
            {
                lastRefreshTick = currentTick;
                
                if(osMutexAcquire(smaMutexHandleHandle, 5) == osOK)
                {
                    wave_buf = PulseSensor_GetWaveBuffer(&heart_sensor);
                    
                    uint8_t wavMax = 0, wavMin = 255;
                    for(int i = 0; i < 128; i++)
                    {
                        if(wave_buf[i] > wavMax) wavMax = wave_buf[i];
                        if(wave_buf[i] < wavMin) wavMin = wave_buf[i];
                    }
                    
                    if(wavMax - wavMin < 10) wavMax = wavMin + 10;
                    
                    OLED_ClearGRAM();
                    
                    // OLED_ShowString(0, 0, "HeartRate", 16, 0);
                    OLED_ShowChinese(0,1, 6, 12, 0);
                    OLED_ShowChinese(12,1, 7, 12, 0);
                    OLED_ShowChar(24, 1, ':', 16, 0);
                    // OLED_ShowString(80, 1, "BPM", 16, 0);
                    if(current_bpm > 0)
                    {
                        if(current_bpm < 100)
                        {
                            OLED_ShowChar(40,1,' ',16,0);
                            OLED_ShowNum(48, 1, current_bpm, 2, 16, 0);
                        }
                        else
                            OLED_ShowNum(40, 1, current_bpm, 3, 16, 0);
                            
                    }
                    // else
                    // {
                    //     OLED_ShowString(32, 2, "---BPM", 16, 0);
                    // }
                    
                    // 显示原始ADC用于调试
                    // OLED_ShowNum(0, 4, adcValue, 4, 12, 0);
                    // printf("adcValue:%d\n",adcValue);
                    // printf("current_bpm:%d\n",current_bpm);
                    for(int i = 0; i < 128; i++)
                    {
                        uint16_t range = wavMax - wavMin;
                        uint8_t y_offset = ((wave_buf[i] - wavMin) * 31) / range;
                        y_offset = 31 - y_offset;
                        
                        uint8_t y = 32 + y_offset;
                        if(y < 32) y = 32;
                        if(y > 63) y = 63;
                        OLED_DrawPoint(i, y, 1);
                    }
                    
                    OLED_HalfRefreshGram();
                    osMutexRelease(smaMutexHandleHandle);
                }
            }
        }
        else
        {
        if(osMutexAcquire(smaMutexHandleHandle, 5) == osOK)
          {
              // OLED_Clear();
              // OLED_ClearGRAM();
              OLED_ShowString(32, 2, "HeartRate", 16, 0);
              OLED_ShowString(32, 4, "Stopped", 16, 0);
              // OLED_RefreshGram();
              osMutexRelease(smaMutexHandleHandle);
          }
        }
        osDelay(10);
    }
}
/* USER CODE BEGIN Header_StartStepTask */
/**
* @brief Function implementing the stepTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartStepTask */
void StartStepTask(void *argument)
{
  /* USER CODE BEGIN StartStepTask */
  /* Infinite loop */
    
    uint8_t flag = 0, tmrFlag = 0;
    uint8_t ucKeyValue;
  for(;;)
  {
      sportInfo = getStep(); /* 获取后台计步数据 */
      if(osMutexAcquire(smaMutexHandleHandle,5) == osOK)
      {
      /* 刷新界面 */
      // OLED_Clear();
      if(StepFlag)
      {
      OLED_ClearALLGRAM();
      OLED_DrawBmp(gImage_321, 3, 24, 36, 35, 1);
      OLED_RefreshGram();
      OLED_ClearALLGRAM();
      StepFlag = 0;
      }
      OLED_ShowChinese(88, 0, 4, 12, 0);
      OLED_ShowChinese(104, 0, 5, 12,0);
      OLED_ShowChinese(104, 2, 0, 12,0);
      OLED_ShowChinese(104, 4, 1, 12,0);
      OLED_ShowChinese(96, 6, 2, 12, 0);
      OLED_ShowChinese(112, 6, 3, 12,0);
      
      OLED_ShowString(36,0,"upLoad",16,0);
      OLED_ShowNum(38, 2, sportInfo->step_count, 6, 16, 0);
      OLED_Showdecimal(38, 4, sportInfo->distance, 4, 2, 16, 0);
      OLED_Showdecimal(38, 6, sportInfo->calories, 4, 2, 16, 0);
      osMutexRelease(smaMutexHandleHandle);
      }

      /* 读取按键队列，控制蓝牙上报定时器启停 */
      if(osMessageQueueGet(queueKeyHandleHandle, &ucKeyValue, NULL, 0) == osOK)
      {
          if(ucKeyValue == 41)
          {
              /* CMSIS-RTOS V2 定时器启停控制 */
              if(osTimerIsRunning(tmrCycleHandlerHandle))
              {
                  osTimerStop(tmrCycleHandlerHandle);
                  tmrFlag = 0;
              }
              else
              {
                  osTimerStart(tmrCycleHandlerHandle, 5000);
                  tmrFlag = 1;
              }
          }
      }
      if(osMutexAcquire(smaMutexHandleHandle,5) == osOK)
      {
      /* 显示蓝牙上报标志 */
      if(tmrFlag) 
          // OLED_ShowString(32, 0, (uint8_t *)"upLoad", 16);

      /* 动态显示脚印图标 */
      if(flag)
      {
          // OLED_DrawBmp(footL, 0, 2, 28, 60, 1);
      }
      else
      {
          // OLED_DrawBmp(footR, 0, 2, 27, 60, 1);
      }

      // OLED_RefreshGram();
      flag = !flag;
      osMutexRelease(smaMutexHandleHandle);
    }
      /* 阻塞延时500ms */
    
      osDelay(500);
  }
  /* USER CODE END StartStepTask */
}

/* USER CODE BEGIN Header_StartWalkTask */
/**
* @brief Function implementing the walkTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWalkTask */
void StartWalkTask(void *argument)
{
  /* USER CODE BEGIN StartWalkTask */
  /* Infinite loop */
  for(;;)
  {
    walkTask();
    osDelay(50);
  }
  /* USER CODE END StartWalkTask */
}

/* USER CODE BEGIN Header_StartTimeCheckTask */
/**
* @brief Function implementing the timeCheckTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTimeCheckTask */
void StartTimeCheckTask(void *argument)
{
  /* USER CODE BEGIN StartTimeCheckTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartTimeCheckTask */
}

/* USER CODE BEGIN Header_StartInfoMenuTask */
/**
* @brief Function implementing the infoMenuTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartInfoMenuTask */
void StartInfoMenuTask(void *argument)
{
  /* USER CODE BEGIN StartInfoMenuTask */
  /* Infinite loop */
  uint8_t ucKeyValue = 0, ucTaskTotal = 0;
  uint32_t uHeapSize = 0;
  char *pcTaskInfo = NULL;
  char ATSEND_CMD[80];
  char NumLen[80];
  for(;;)
  {
    ucTaskTotal = uxTaskGetNumberOfTasks();
    uHeapSize = xPortGetFreeHeapSize();
    if(osMutexAcquire(smaMutexHandleHandle,5) == osOK)
    {
      OLED_ShowString(16,0,"TaskInfo",16,0);
      OLED_ShowString(0,2,"cpu:--STM32F103C8T6--",12,0);
      OLED_ShowString(4, 3, "Task Total = ", 16,0);
      OLED_ShowNum(108, 3, ucTaskTotal, 2, 16, 0);
      OLED_ShowString(0, 5, "FreeRTOS Mem:   B", 12,0);
      OLED_ShowNum(78, 5, uHeapSize, 6, 12,0);
      osMutexRelease(smaMutexHandleHandle);
    }
    if(osMessageQueueGet(queueKeyHandleHandle, &ucKeyValue, NULL, 5) == osOK)
    {
        osThreadSuspend(keyTaskHandle);
        // osTimerStop(tmrSingleHandlerHandle);
        // osTimerStop(tmrCycleHandlerHandle);
        if(ucKeyValue == 21)
        {
            // pcTaskInfo = (char *)pvPortMalloc(512);
            if(pcTaskInfo != NULL)
            {
                vTaskList(pcTaskInfo);
                // printf("Task List:\r\n%s\r\n", pcTaskInfo);
                // vPortFree(pcTaskInfo);
                HAL_UART_Transmit(&huart1,(uint8_t *)"AT+CIPSEND=0,44\r\n", strlen("AT+CIPSEND=0,44\r\n"), 0xffff);
                osDelay(50);
                HAL_UART_Transmit( &huart1, (uint8_t *)"任务名  任务状态  优先级  剩余栈大小  任务号", strlen("任务名  任务状态  优先级  剩余栈大小  任务号"), 0xffff);
                osDelay(150);
                if(osMutexAcquire(UsartMutexHandle, 100) == osOK)
                {
                    // OLED_ShowString(0,6,"string",16,0);
                    // OLED_ShowNum(80,6,strlen(pcTaskInfo), 6, 16, 0);
                    sprintf(ATSEND_CMD, "AT+CIPSEND=0,%d\r\n", strlen(pcTaskInfo));
                    // OLED_ShowString(0,6,"strings",16,0);
                    HAL_UART_Transmit(&huart1,(uint8_t *)ATSEND_CMD, strlen(ATSEND_CMD), 0xffff);
                    osDelay(50);
                    // OLED_ShowString(0,6,"stringss",16,0);
                    HAL_UART_Transmit(&huart1, (uint8_t *)pcTaskInfo, strlen(pcTaskInfo), 0xffff);
                    osDelay(500);
                    // OLED_ShowString(0,6,"stringsss",16,0);
                    osMutexRelease(UsartMutexHandle);
                }
                // vPortFree(pcTaskInfo);
                // pcTaskInfo = NULL;
                // OLED_ShowString(0,6,"stringssss",16,0);
            }
            else
            {
                HAL_UART_Transmit(&huart1,(uint8_t *)"AT+CIPSEND=0,31\r\n", strlen("AT+CIPSEND=0,31\r\n"), 0xffff);
                osDelay(50);
                HAL_UART_Transmit(&huart1, (uint8_t *)"请先通过按键 KEYPWR 申请内存!\r\n", strlen("请先通过按键 KEYPWR 申请内存!\r\n"), 0xffff);
                osDelay(50);
            }
        }
        else if(ucKeyValue == 31)
        {
            // osThreadSuspend(keyTaskHandle);
            if(pcTaskInfo != NULL)  /* 已申请了动态内存 */
            {
                vTaskGetRunTimeStats(pcTaskInfo);
                HAL_UART_Transmit(&huart1,(uint8_t *)"AT+CIPSEND=0,24\r\n", strlen("AT+CIPSEND=0,24\r\n"), 0xffff);
                osDelay(50);
                HAL_UART_Transmit(&huart1, (uint8_t *)"任务名\t运行时间\t百分比\r\n", strlen("任务名\t运行时间\t百分比\r\n"), 0xffff);
                osDelay(150);


                /* 获取互斥信号量 */
                if(osMutexAcquire(UsartMutexHandle, 100) == osOK)
                {
                    sprintf(ATSEND_CMD, "AT+CIPSEND=0,%d\r\n", strlen(pcTaskInfo));
                    HAL_UART_Transmit(&huart1,(uint8_t *)ATSEND_CMD, strlen(ATSEND_CMD), 0xffff);
                    osDelay(50);
                    HAL_UART_Transmit(&huart1, (uint8_t *)pcTaskInfo, strlen(pcTaskInfo), 0xffff);
                    osDelay(500);
                    osMutexRelease(UsartMutexHandle); /* 归还互斥信号量 */
                }
                // vPortFree(pcTaskInfo);  /* 释放内存 */
                // pcTaskInfo = NULL;
            }
            else
            {
                HAL_UART_Transmit(&huart1,(uint8_t *)"AT+CIPSEND=0,31\r\n", strlen("AT+CIPSEND=0,31\r\n"), 1000);
                osDelay(50);
                HAL_UART_Transmit(&huart1, (uint8_t *)"请先通过按键 KEYPWR 申请内存!\r\n", strlen("请先通过按键 KEYPWR 申请内存!\r\n"), 1000);
                osDelay(50);
            }
            // osThreadResume(keyTaskHandle);
        }
        else if (ucKeyValue == 41)
        {
          if(pcTaskInfo != NULL)
            {
                vPortFree(pcTaskInfo);
                pcTaskInfo = NULL; 
            }
          uHeapSize = xPortGetFreeHeapSize();
          sprintf(NumLen, "KEYPWR键申请内存,内存堆剩余%8d字节\r\n", uHeapSize);
          sprintf(ATSEND_CMD, "AT+CIPSEND=0,%d\r\n", strlen(NumLen));
          HAL_UART_Transmit(&huart1,(uint8_t *)ATSEND_CMD, strlen(ATSEND_CMD), 0xffff);
          osDelay(50);
          HAL_UART_Transmit(&huart1, (uint8_t *)NumLen, strlen(NumLen), 0xffff);
          osDelay(50);
          pcTaskInfo = pvPortMalloc(1024);
          if(pcTaskInfo != NULL)
          {
          memset(pcTaskInfo, 0, 512);
          osDelay(100);
          uHeapSize = xPortGetFreeHeapSize();
          sprintf(NumLen, "动态申请内存成功，内存堆剩余%8d字节\r\n", uHeapSize);
          sprintf(ATSEND_CMD, "AT+CIPSEND=0,%d\r\n", strlen(NumLen));
          HAL_UART_Transmit(&huart1,(uint8_t *)ATSEND_CMD, strlen(ATSEND_CMD), 0xffff);
          osDelay(50);
          HAL_UART_Transmit(&huart1, (uint8_t *)NumLen, strlen(NumLen), 0xffff);
          osDelay(100);
          sprintf(NumLen, "动态内存地址: %x\r\n\r\n", (uint32_t)pcTaskInfo);
          sprintf(ATSEND_CMD, "AT+CIPSEND=0,%d\r\n", strlen(NumLen));
          HAL_UART_Transmit(&huart1,(uint8_t *)ATSEND_CMD, strlen(ATSEND_CMD), 0xffff);
          osDelay(50);
          HAL_UART_Transmit(&huart1, (uint8_t *)NumLen, strlen(NumLen), 0xffff);
          osDelay(100);
        }
        else
        {
          sprintf(ATSEND_CMD, "AT+CIPSEND=0,%d\r\n", strlen("申请动态内存失败! \r\n"));
          HAL_UART_Transmit(&huart1,(uint8_t *)ATSEND_CMD, strlen(ATSEND_CMD), 0xffff);
          osDelay(50);
          HAL_UART_Transmit(&huart1, (uint8_t *)"申请动态内存失败! \r\n", strlen("申请动态内存失败! \r\n"), 0xffff);
          osDelay(50);
        }
    }
    osThreadResume(keyTaskHandle);
  }
    osDelay(500);
  }
  /* USER CODE END StartInfoMenuTask */
}

/* tmrSingleCallback function */
void tmrSingleCallback(void *argument)
{
  /* USER CODE BEGIN tmrSingleCallback */
  suspendScreen();
  if(pageNum>pageNone)
    pageNum--;
  OLED_Clear();
  /* USER CODE END tmrSingleCallback */
}

/* tmrCycleCallback function */
void tmrCycleCallback(void *argument)
{
  /* USER CODE BEGIN tmrCycleCallback */
  char txbuff[80],cmdbuff[80];
  sprintf(txbuff,"步数：%3d步,里程:%3.1f米,卡路里:%3.1f卡",sportInfo->step_count,sportInfo->distance,sportInfo->calories);
  sprintf(cmdbuff,"AT+CIPSEND=0,%d\r\n",strlen(txbuff));
  if(osMutexAcquire(UsartMutexHandle,10) == osOK)
  {
    HAL_UART_Transmit(&huart1,(uint8_t *)cmdbuff,strlen(cmdbuff),0xffff);
    osDelay(50);
    HAL_UART_Transmit(&huart1,(uint8_t *)txbuff,strlen(txbuff),0xffff);
    osDelay(50);
    osMutexRelease(UsartMutexHandle);
  }

  /* USER CODE END tmrCycleCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* 挂起所有与屏幕操作相关的任务 */
void suspendScreen(void)
{
    /* CMSIS-RTOS V2 任务挂起 API */
    osThreadSuspend(mainMenuTaskHandle);
    osThreadSuspend(timeSetTaskHandle);
    osThreadTerminate(alarmSetTaskHandle);
    osThreadSuspend(secondTaskHandle);
    osThreadSuspend(stepTaskHandle);
    osThreadSuspend(heartTaskHandle);
    osThreadSuspend(infoMenuTaskHandle);
    osThreadSuspend(timeCheckTaskHandle);
    /* 关闭指示灯 */
    // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
    // HAL_GPIO_WritePin(GPIOB, LED_RUN_Pin, GPIO_PIN_SET);
    // HAL_GPIO_WritePin(GPIOB, LED_ALARM_Pin, GPIO_PIN_SET);
}

/* 根据页面编号恢复对应的屏幕任务 */
void resumeScreen(uint8_t pageNum)
{
    // OLED_Clear();
    switch(pageNum)
    {
        case pageNone:
            OLED_Clear();
            break;
        case pageMain:
            osThreadResume(mainMenuTaskHandle);
            break;
        case pageTimeSet:
            osThreadResume(timeSetTaskHandle);
            break;
        case pageAlarmSet:
            alarmSetTaskHandle = osThreadNew(StartAlarmSetTask, NULL, &alarmSetTask_attributes);
            osThreadResume(alarmSetTaskHandle);
            break;
        case pageSecond:
            secondflag = 1;
            osThreadResume(secondTaskHandle);
            break;
        case pageHeart:
            osThreadResume(heartTaskHandle);
            break;
        case pageStep:
            StepFlag = 1;
            osThreadResume(stepTaskHandle);
            break;
        case pageInfo:
            osThreadResume(infoMenuTaskHandle);
            break;
        default:
            OLED_Clear();
            break;
    }
}
/* USER CODE END Application */


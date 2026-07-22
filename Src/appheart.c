#include "appheart.h"
#include "adc.h"
#include "pulse_sensor.h"

// 全局心率传感器句柄
PulseSensorHandle_t pulseSensor;
uint8_t heartRate = 0;

// 采样周期：20ms（和你的HEART_PERIOD一致）
#define SAMPLE_PERIOD 20

/**
 * @brief  心率传感器初始化
 */
void HeartSensor_Init(void)
{
    PulseSensor_Init(&pulseSensor);
    // ✅ 只启动一次ADC连续转换模式
    HAL_ADC_Start(&hadc1);
    // 等待第一次转换完成
    HAL_ADC_PollForConversion(&hadc1, 10);
}

/**
 * @brief  读取心率值（每20ms调用一次）
 * @retval 心率BPM，0表示无有效心跳
 */
uint8_t getPulse(void)
{
    uint16_t adcValue;
    
    // ✅ 连续模式下直接读取结果，不需要再次启动ADC
    adcValue = HAL_ADC_GetValue(&hadc1);
    
    // 处理心率算法
    heartRate = (uint8_t)PulseSensor_Process(&pulseSensor, adcValue);
    
    // ========== 新增：空闲检测（不测量时显示0） ==========
    // 如果连续3秒没有心跳，强制重置为0
    static uint32_t noBeatTimer = 0;
    if(heartRate == 0)
    {
        noBeatTimer += 2; // 2ms采样周期
        if(noBeatTimer > 3000)
        {
            PulseSensor_Reset(&pulseSensor);
            noBeatTimer = 0;
        }
    }
    else
    {
        noBeatTimer = 0;
    }
    
    // ========== 新增：心率范围限制（正常人心率60-180） ==========
    if(heartRate < 50 || heartRate > 200)
    {
        heartRate = 0;
    }
    
    return heartRate;
}

/**
 * @brief  获取心率波形数据（用于OLED显示）
 */
uint8_t* getHeartWave(void)
{
    return PulseSensor_GetWaveBuffer(&pulseSensor);
}
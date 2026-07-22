#ifndef __PULSE_SENSOR_H
#define __PULSE_SENSOR_H

#include "main.h"
#include <stdbool.h>

/**
 * @brief  PulseSensor 心率算法句柄
 * @note   所有算法状态封装在此结构体中，无全局变量
 */
typedef struct
{
    /* 算法核心状态变量 */
    int Signal;             // 当前处理后的信号值
    int BPM;                // 最终心率值（次/分钟）
    int IBI;                // 两次心跳间隔(ms)
    bool Pulse;             // 当前是否处于脉搏波中

    int rate[10];           // 最近10次心跳间隔（滑动平均滤波用）
    uint32_t sampleCounter; // 采样计时器（单位：ms）
    uint32_t lastBeatTime;  // 上一次检测到心跳的时间

    int P;                  // 信号波峰
    int T;                  // 信号波谷
    int thresh;             // 动态检测阈值
    int amp;                // 信号幅值

    bool firstBeat;         // 第一次心跳标记
    bool secondBeat;        // 第二次心跳标记

    /* OLED波形绘制缓存 */
    uint8_t wave_buf[128];  // 128点波形缓存
    uint8_t wave_pos;       // 波形缓存索引
} PulseSensorHandle_t;

/**
 * @brief  初始化心率算法
 * @param  hs: 心率算法句柄指针
 */
void PulseSensor_Init(PulseSensorHandle_t *hs);

/**
 * @brief  心率算法处理函数（核心：传参调用+返回心率值）
 * @param  hs: 心率算法句柄指针
 * @param  adc_value: ADC采样的原始值（12位 0~4095）
 * @retval 当前心率值BPM（0=未检测到有效心跳）
 */
int PulseSensor_Process(PulseSensorHandle_t *hs, uint16_t adc_value);

/**
 * @brief  重置心率算法状态（启停测量时调用）
 * @param  hs: 心率算法句柄指针
 */
void PulseSensor_Reset(PulseSensorHandle_t *hs);

/**
 * @brief  获取波形缓存指针（用于OLED绘制波形）
 * @param  hs: 心率算法句柄指针
 * @retval 128点波形缓存首地址
 */
uint8_t* PulseSensor_GetWaveBuffer(PulseSensorHandle_t *hs);

#endif
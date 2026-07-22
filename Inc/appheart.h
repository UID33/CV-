#ifndef __APPHEART_H
#define __APPHEART_H

#include "main.h"
#include "pulse_sensor.h"

// 采样周期20ms（和你的代码保持一致）
#define HEART_PERIOD 20

extern uint8_t heartRate;

void HeartSensor_Init(void);
uint8_t getPulse(void);
uint8_t* getHeartWave(void);

#endif
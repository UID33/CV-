#ifndef __appstep_H
#define __appstep_H

#include "oled.h"
#include "main.h"
#include <math.h>
#include <stdio.h>
#include "mpu6500.h"

//=============================================================================
// 宏定义
//=============================================================================
#define TRUE 1
#define FALSE 0
#define WAVE_NUM 4
#define PEEK_MIN_VALUE 20000 // 注意：原代码拼写为 PEEK，建议改为 PEAK，但此处保持一致性

// 步长默认值 (米)
#define DEFAULT_STEP_LENGTH 0.6f

//=============================================================================
// 数据结构体
//=============================================================================

typedef struct {
    float calories;      // 卡路里
    float distance;      // 距离 (米)
    uint32_t step_count; // 步数
} sportsInfo_t;

typedef struct {
    float height;        // 身高 (cm)
    float weight;        // 体重 (kg)
} personInfo_t;

//=============================================================================
// 全局变量声明 (需要在 .c 文件中定义)
//=============================================================================
extern sportsInfo_t sprotsInfo; // 注意：原代码拼写为 sprotsInfo，建议改为 sportsInfo
extern personInfo_t personInfo;
extern uint32_t stepTimeCount;  // 总步数计数器(临时)
//=============================================================================
// 函数声明
//=============================================================================

/**
 * @brief 获取运动信息结构体指针
 * @return sportsInfo_t* 
 */
sportsInfo_t *getStep(void);

/**
 * @brief 步行任务入口 (FreeRTOS Task)
 * @param pvParameters 
 */
void walkTask();

/**
 * @brief 计步主任务 (FreeRTOS Task)
 * @param pvParameters 
 */
// void stepTask(void *pvParameters);

/**
 * @brief 初始化计步模块 (可选，用于重置数据)
 */
void Step_Module_Init(void);

#endif /* __appstep_H */
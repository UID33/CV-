#include "appstep.h"

//=============================================================================
// 全局变量定义
//=============================================================================
sportsInfo_t sprotsInfo = {0.0f, 0.0f, 0};
personInfo_t personInfo = {175.0f, 65.0f}; // 默认身高175cm, 体重65kg，可根据实际修改

//=============================================================================
// 内部静态变量 (用于计步算法状态保持)
//=============================================================================
static uint32_t timeOfLastPeak = 0;
static uint32_t timeOfPeak = 0;
static uint32_t timeOfNow = 0;

static uint32_t peakOfWave = 0;
static uint32_t valleyOfWave = 0;
static uint32_t ThreadValue = 20000; // 初始阈值

uint32_t stepTimeCount = 0;   // 总步数计数器(临时)
static uint32_t stepBy2second = 0;   // 每2秒内的步数
static uint32_t stepTempCount = 0;   // 临时步数缓存

static uint32_t gravityOld = 0;
static uint8_t isDirectionUp = FALSE;
static uint8_t lastStatus = FALSE;
static uint32_t continueUpCount = 0;
static uint32_t continueUpCountLastPoint = 0;

static uint32_t tempValue[WAVE_NUM] = {0}; // 用于动态阈值计算的环形缓冲区

//=============================================================================
// 内部函数原型
//=============================================================================
sportsInfo_t *useAccToStep(void); 
static uint8_t DetectorPeak(uint32_t gravityNew, uint32_t gravityOld);
static sportsInfo_t *CalculateStep(int16_t *AccData);
static sportsInfo_t *DetectorNewStep(uint32_t gravityNew);
static uint32_t peakValleyThread(uint32_t value);
static uint32_t averageValue(uint32_t value[], int n);

//=============================================================================
// 公共函数实现
//=============================================================================

/**
 * @brief 获取当前运动统计数据
 */
sportsInfo_t *getStep(void)
{
    return &sprotsInfo;
}

/**
 * @brief FreeRTOS 任务: 定期读取加速度并计算步数
 */
// void stepTask(void *pvParameters)
// {
//     while (1)
//     {
//         useAccToStep();
//     }
// }

/**
 * @brief FreeRTOS 任务: 处理显示或其他步行相关逻辑
 */
void walkTask()
{
    // while (1)
    // {
        useAccToStep();
    // }
}

/**
 * @brief 初始化计步模块
 */
void Step_Module_Init(void)
{
    sprotsInfo.calories = 0.0f;
    sprotsInfo.distance = 0.0f;
    sprotsInfo.step_count = 0;
    
    gravityOld = 0;
    timeOfLastPeak = 0;
    stepBy2second = 0;
    ThreadValue = 20000;
}

//=============================================================================
// 内部算法实现
//=============================================================================

/**
 * @brief 读取加速度并触发计步计算
 * @return 运动信息指针
 */
sportsInfo_t *useAccToStep(void)
{
    MPU6500_RawData_t rawData;
    int16_t AccData[3];
    
    // 读取原始加速度数据
    MPU6500_Read_Raw(&rawData, MPU_READ_ACCEL_ALL);
    
    AccData[0] = rawData.Accel_X;
    AccData[1] = rawData.Accel_Y;
    AccData[2] = rawData.Accel_Z;
    
    return CalculateStep(AccData);
}

/**
 * @brief 计算合加速度并检测新步态
 * @param AccData 三轴加速度原始数据
 * @return 运动信息指针
 */
static sportsInfo_t *CalculateStep(int16_t *AccData)
{
    uint32_t gravityNew = 0;
    
    // 计算矢量模长 (去除平方根以提高效率可选，但为了阈值匹配保留sqrt)
    // 注意：MPU6500原始数据范围较大，sqrt前可能需要考虑溢出，uint32_t通常足够
    gravityNew = (uint32_t)sqrt((float)(AccData[0] * AccData[0] + 
                                        AccData[1] * AccData[1] + 
                                        AccData[2] * AccData[2]));
    
    return DetectorNewStep(gravityNew);
}

/**
 * @brief 核心计步逻辑：检测波峰并更新统计数据
 * @param gravityNew 当前合加速度值
 * @return 运动信息指针
 */
static sportsInfo_t *DetectorNewStep(uint32_t gravityNew)
{
    static uint32_t time_old = 0; // 上次统计时间点
    float stepLength = DEFAULT_STEP_LENGTH;
    float walkSpeed, walkDistance, Calories;
    uint32_t time_now;

    if (gravityOld == 0)
    {
        gravityOld = gravityNew;
        return &sprotsInfo;
    }
    
    // 检测是否出现波峰
    if (DetectorPeak(gravityNew, gravityOld))
    {
        timeOfLastPeak = timeOfPeak;
        time_now = HAL_GetTick();
        timeOfNow = time_now;

        // 判断是否为有效步态：
        // 1. 时间间隔 > 250ms (排除高频抖动)
        // 2. 波峰波谷差值 > 动态阈值
        if ((timeOfNow - timeOfLastPeak >= 250) && 
            (peakOfWave - valleyOfWave >= ThreadValue))
        {
            timeOfPeak = timeOfNow;
            
            // 更新动态阈值
            ThreadValue = peakValleyThread(peakOfWave - valleyOfWave);
            
            stepTimeCount++;
            stepBy2second++;
            stepTempCount++;

            // 简单的防抖或连续步态确认逻辑
            // 如果长时间没有新步态，重置临时计数
            if ((stepTempCount < 5) && (timeOfNow - timeOfLastPeak >= 3000))
            {
                stepTempCount = 0;
            }
            // 如果连续检测到步态，累加到总步数
            else if ((stepTempCount >= 5) && (timeOfNow - timeOfLastPeak < 3000))
            {
                // sprotsInfo.step_count += stepTempCount;
                stepTempCount = 0;
            }
            sprotsInfo.step_count = stepTimeCount;
            // 每2秒统计一次速度和距离
            if ((time_now - time_old) > 2000)
            {
                // 速度 = 步数 * 步长 / 时间 (m/s)
                walkSpeed = (stepBy2second * stepLength) / 2.0f;
                
                // 距离增量 = 步数 * 步长
                walkDistance = stepBy2second * stepLength;
                
                // 卡路里估算公式 (简化版): METs * Weight(kg) * Time(h)
                // 这里使用用户提供的公式: 4.5 * speed * (weight/2) / 1800
                // 注意：原公式逻辑可能需根据实际物理意义调整，此处保持原意
                Calories = 4.5f * walkSpeed * (personInfo.weight / 2.0f) / 1800.0f;
                
                sprotsInfo.calories += Calories;
                sprotsInfo.distance += walkDistance;
                
                time_old = time_now;
                stepBy2second = 0;
            }   
        }            
    }        
    
    gravityOld = gravityNew;
    return &sprotsInfo;    
}

/**
 * @brief 检测加速度波形峰值
 * @param gravityNew 当前值
 * @param gravityOld 上一时刻值
 * @return TRUE 如果检测到波峰, FALSE 否则
 */
static uint8_t DetectorPeak(uint32_t gravityNew, uint32_t gravityOld)
{
    uint8_t result = FALSE;
    
    lastStatus = isDirectionUp;
    // OLED_ShowNum(0,7,gravityNew,5,12,0);
    // 判断趋势方向
    if (gravityNew >= gravityOld)
    {
        isDirectionUp = TRUE;
        continueUpCount++;
    }
    else
    {
        continueUpCountLastPoint = continueUpCount;
        continueUpCount = 0;
        isDirectionUp = FALSE;
    }
    
    // 检测波峰: 方向由上变下，且之前持续上升超过2个点，且值大于最小阈值
    if ((!isDirectionUp) && lastStatus && 
        (continueUpCountLastPoint >= 2) && 
        (gravityOld >= PEEK_MIN_VALUE))
    {
        peakOfWave = gravityOld;
        result = TRUE;
    }
    // 检测波谷: 方向由下变上
    else if ((!lastStatus) && isDirectionUp)
    {
        valleyOfWave = gravityOld;
        result = FALSE;
    }
    
    return result;
}

/**
 * @brief 动态计算波峰波谷差值的阈值
 * @param value 当前的波峰波谷差值
 * @return 新的阈值
 */
static uint32_t peakValleyThread(uint32_t value)
{
    static int tempCount = 0;
    uint32_t tempThread = ThreadValue;
    
    if (tempCount < WAVE_NUM)
    {
        tempValue[tempCount] = value;
        tempCount++;
        // 数据不足时返回当前值或默认值，避免早期波动
        if(tempCount < WAVE_NUM) return ThreadValue; 
    }
    else
    {
        // 计算平均值
        tempThread = averageValue(tempValue, WAVE_NUM);
        
        // 环形缓冲区移位
        for (uint8_t i = 1; i < WAVE_NUM; i++)
        {
            tempValue[i - 1] = tempValue[i];
        }
        tempValue[WAVE_NUM - 1] = value;        
    }
    
    return tempThread;   
}

/**
 * @brief 计算数组平均值并根据范围限幅
 * @param value 数组
 * @param n 长度
 * @return 处理后的平均值
 */
static uint32_t averageValue(uint32_t value[], int n)
{
    uint32_t sum = 0;
    uint32_t average = 0;
    
    for (uint8_t i = 0; i < n; i++)
    {
        sum += value[i];
    }
    
    if (n > 0)
    {
        average = sum / n;
    }
    
    // 根据平均值大小映射到固定阈值档位 (启发式规则)
    if (average >= 80000)
    {
        average = 30000;
    }
    else if (average >= 60000)
    {
        average = 20000;
    }
    else if (average >= 40000)
    {
        average = 12000;
    }
    else if (average >= 20000)
    {
        average = 8000;
    }
    else
    {
        average = 5000;
    }
    
    return average;
}
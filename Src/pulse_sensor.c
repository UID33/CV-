#include "pulse_sensor.h"

void PulseSensor_Init(PulseSensorHandle_t *hs)
{
    PulseSensor_Reset(hs);
}

void PulseSensor_Reset(PulseSensorHandle_t *hs)
{
    hs->Signal = 0;
    hs->BPM = 0;
    hs->IBI = 700;
    hs->Pulse = false;

    for (uint8_t i = 0; i < 10; i++)
        hs->rate[i] = 700;

    hs->sampleCounter = 0;
    hs->lastBeatTime = 0;

    // 3.3V 12位ADC 正确初始基线 2048
    hs->P = 2100;
    hs->T = 1996;
    hs->thresh = 2048;
    hs->amp = 100;

    hs->firstBeat = true;
    hs->secondBeat = false;

    for (uint8_t i = 0; i < 128; i++)
        hs->wave_buf[i] = 128;
    hs->wave_pos = 0;
}

int PulseSensor_Process(PulseSensorHandle_t *hs, uint16_t adc_value)
{
    // ==============================================
    //  Step1：低通滤波（滤除50Hz工频噪声）
    // ==============================================
    static int32_t filter = 2048;
    filter = filter * 0.92f + adc_value * 0.08f;
    hs->Signal = (int)filter;

    hs->sampleCounter += 2;
    int N = hs->sampleCounter - hs->lastBeatTime;

    // ==============================================
    //  Step2：无效值直接过滤
    // ==============================================
    if (adc_value < 1800 || adc_value > 3500)
        goto WAVE_SAVE;

    // ==============================================
    //  Step3：动态峰值/谷值 跟踪（官方稳定算法）
    // ==============================================
    if (hs->Signal < hs->thresh && N > (hs->IBI / 5) * 3)
    {
        if (hs->Signal < hs->T)
            hs->T = hs->Signal;
    }

    if (hs->Signal > hs->thresh && hs->Signal > hs->P)
    {
        hs->P = hs->Signal;
    }

    // ==============================================
    //  Step4：心跳检测（真正的脉搏特征）
    // ==============================================
    if (N > 300) // 最小间隔300ms → 最高200BPM
    {
        if ((hs->Signal > hs->thresh) && (!hs->Pulse) && (N > (hs->IBI / 5) * 3))
        {
            hs->Pulse = true;
            hs->IBI = hs->sampleCounter - hs->lastBeatTime;
            hs->lastBeatTime = hs->sampleCounter;

            if (hs->firstBeat)
            {
                hs->firstBeat = false;
                hs->secondBeat = true;
                goto WAVE_SAVE;
            }

            if (hs->secondBeat)
            {
                hs->secondBeat = false;
                for (int i = 0; i < 10; i++)
                    hs->rate[i] = hs->IBI;
            }

            // 滑动平均滤波
            uint32_t total = 0;
            for (int i = 0; i < 9; i++)
            {
                hs->rate[i] = hs->rate[i + 1];
                total += hs->rate[i];
            }
            hs->rate[9] = hs->IBI;
            total += hs->rate[9];
            total /= 10;

            int bpm = 60000 / total;

            // 强制合理范围
            if (bpm >= 50 && bpm <= 180)
                hs->BPM = bpm;
            else
                hs->BPM = 0;
        }
    }

    // ==============================================
    //  Step5：心跳结束 → 更新阈值
    // ==============================================
    if (hs->Signal < hs->thresh && hs->Pulse)
    {
        hs->Pulse = false;
        hs->amp = hs->P - hs->T;

        if (hs->amp < 30)  // 振幅太小 → 不是心跳
        {
            hs->amp = 30;
            hs->BPM = 0;
        }

        hs->thresh = hs->T + hs->amp / 2;
        hs->P = hs->thresh;
        hs->T = hs->thresh;
    }

    // ==============================================
    //  Step6：2.5秒无心跳 → 重置
    // ==============================================
    if (N > 2500)
    {
        hs->BPM = 0;
        hs->thresh = 2048;
        hs->P = 2100;
        hs->T = 1996;
        hs->lastBeatTime = hs->sampleCounter;
        hs->firstBeat = true;
    }

WAVE_SAVE:
    // 波形显示
    int16_t wv = 128 + (hs->Signal - 2048) / 8;
    if (wv < 0) wv = 0;
    if (wv > 255) wv = 255;
    hs->wave_buf[hs->wave_pos] = wv;

    hs->wave_pos++;
    if (hs->wave_pos >= 128)
        hs->wave_pos = 0;

    return hs->BPM;
}

uint8_t* PulseSensor_GetWaveBuffer(PulseSensorHandle_t *hs)
{
    return hs->wave_buf;
}
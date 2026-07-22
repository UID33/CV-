#ifndef __KEY_H
#define __KEY_H

#include "main.h"

// 按键定义（PA12~PA15）
#define KEY1_PIN  GPIO_PIN_12   // KEYDWN
#define KEY2_PIN  GPIO_PIN_5   // KEYUP
#define KEY3_PIN  GPIO_PIN_4   // KEYMODE
#define KEY4_PIN  GPIO_PIN_15   // KEYPWR
#define KEY_PORT  GPIOA

#define KEYDWN_PRES  1
#define KEYUP_PRES   2
#define KEYMODE_PRES 3
#define KEYPWR_PRES  4

// 长按标志（高4位）
#define KEY_EVENT_SHORT    1
#define KEY_EVENT_LONG     2

// ==================== 可自定义参数 ====================
#define DEBOUNCE_TIME       2     // 消抖时间（单位：扫描次数，推荐2~3，10ms扫描则20~30ms）
#define SHORT_PRESS_TIME    50    // 短按最大时间（单位：扫描次数，50=500ms）
#define LONG_PRESS_TIME     80    // 长按最小时间（单位：扫描次数，80=800ms）
// ====================================================

void Key_Init(void);
void Key_Scan(void);        // 请在10ms定时器或任务中调用
uint8_t Key_GetResult(void);

#endif
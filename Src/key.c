#include "key.h"

static uint8_t key_value = 0;
static uint8_t key_state = 0;     // 0:空闲  1:消抖  2:按下保持
static uint16_t key_cnt = 0;
static uint8_t key_temp = 0;

/**
 * @brief 按键初始化
 */
void Key_Init(void)
{
    key_state = 0;
    key_value = 0;
    key_cnt = 0;
    key_temp = 0;
}

/**
 * @brief 按键扫描（建议10ms调用一次）
 */
void Key_Scan(void)
{
    uint8_t now = 0;

    // 读取按键（可根据需要打开其他按键）
    if (HAL_GPIO_ReadPin(KEY_PORT, KEY1_PIN) == 0) now = KEYDWN_PRES;
    else if (HAL_GPIO_ReadPin(KEY_PORT, KEY2_PIN) == 0) now = KEYUP_PRES;
    else if (HAL_GPIO_ReadPin(KEY_PORT, KEY3_PIN) == 0) now = KEYMODE_PRES;
    else if (HAL_GPIO_ReadPin(KEY_PORT, KEY4_PIN) == 0) now = KEYPWR_PRES;  // 注意高电平有效

    switch (key_state)
    {
        case 0:  // 空闲状态
            if (now != 0)
            {
                key_temp = now;
                key_cnt = 0;
                key_state = 1;        // 进入消抖
            }
            break;

        case 1:  // 消抖
            if (now == key_temp)
            {
                key_cnt++;
                if (key_cnt >= DEBOUNCE_TIME)
                {
                    key_state = 2;    // 消抖完成，进入按下状态
                    key_cnt = 0;
                }
            }
            else
            {
                key_state = 0;        // 抖动，回到空闲
            }
            break;

        case 2:  // 按键保持按下
            if (now == 0)            // 按键释放
            {
                if (key_cnt < SHORT_PRESS_TIME)
                    key_value = key_temp * 10 + KEY_EVENT_SHORT;   // 短按

//                else
//                    key_value = key_temp * 10 + KEY_EVENT_SHORT;   // 中间按也算短按

                key_state = 0;
                key_cnt = 0;
            }
            else
            {
                if (key_cnt == LONG_PRESS_TIME)
                    key_value = key_temp * 10 + KEY_EVENT_LONG;    // 长按
                key_cnt++;
                if (key_cnt > 1000) key_cnt = 1000;   // 防止溢出
            }
            break;
    }
}

/**
 * @brief 获取按键值（读取后自动清除）
 */
uint8_t Key_GetResult(void)
{
    uint8_t ret = key_value;
    key_value = 0;
    return ret;
}
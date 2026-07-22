# 代码变更总结

## 1. pulse_sensor.h - 数据结构扩展

### 变更内容
在 `PulseSensorHandle_t` 结构体中添加了显示缓冲成员：

```c
/* 显示缓冲（待显示的数据） */
uint8_t display_wave[128];  // 待显示的波形数据（128点）
int display_bpm;            // 待显示的心率值
uint16_t display_max;       // 待显示的波形最大值（用于自适应缩放）
bool display_ready;         // 显示数据是否就绪标志
```

### 新增函数声明
```c
bool PulseSensor_GetDisplayData(PulseSensorHandle_t *hs);
```

---

## 2. pulse_sensor.c - 核心算法改进

### 2.1 PulseSensor_Reset() - 显示缓冲初始化
```c
/* 初始化显示缓冲 */
for(uint8_t i=0; i<128; i++) hs->display_wave[i] = 0;
hs->display_bpm = 0;
hs->display_max = 100;
hs->display_ready = false;
```

### 2.2 PulseSensor_Process() - 采样完毕标志
关键改进：当采集满 128 点时，自动设置显示就绪标志
```c
WAVE_SAVE:
    hs->wave_buf[hs->wave_pos] = hs->Signal >> 4; 
    hs->wave_pos++;
    
    /* 128点收集完毕，标记显示数据就绪 */
    if(hs->wave_pos >= 128)
    {
        hs->wave_pos = 0;
        hs->display_ready = true;  // 核心：设置标志
    }
```

### 2.3 新增 PulseSensor_GetDisplayData() - 显示数据提取
```c
bool PulseSensor_GetDisplayData(PulseSensorHandle_t *hs)
{
    if(!hs->display_ready)
        return false;
    
    /* 1. 复制采集缓冲到显示缓冲 */
    for(uint8_t i = 0; i < 128; i++)
        hs->display_wave[i] = hs->wave_buf[i];
    
    /* 2. 保存心率值 */
    hs->display_bpm = hs->BPM;
    
    /* 3. 计算波形最大值（用于自适应缩放） */
    hs->display_max = 0;
    for(uint8_t i = 0; i < 128; i++)
        if(hs->display_wave[i] > hs->display_max)
            hs->display_max = hs->display_wave[i];
    
    /* 4. 防止最大值过小 */
    if(hs->display_max < 10)
        hs->display_max = 10;
    
    /* 5. 重置标志，等待下一波数据 */
    hs->display_ready = false;
    
    return true;
}
```

---

## 3. freertos.c - StartHeartTask() 完全重构

### 3.1 新增变量
```c
uint32_t last_display_time = 0;  // 上次显示的时间戳
uint32_t current_time = 0;
```

### 3.2 初始化
```c
if(runFlag)
{
    PulseSensor_Reset(&heart_sensor);
    last_display_time = osKernelGetTickCount();  // 重置时间戳
}
```

### 3.3 核心改进：2秒刷新周期实现
```c
current_time = osKernelGetTickCount();

// 检查是否达到2秒刷新周期
if((current_time - last_display_time) >= 2000)
{
    last_display_time = current_time;  // 更新时间戳
    
    // 获取显示数据
    if(PulseSensor_GetDisplayData(&heart_sensor))
    {
        // 只在这里更新OLED（每2秒一次）
        OLED_ClearGRAM();
        // ... 绘制波形和数值
        OLED_RefreshGram();
    }
}
```

### 3.4 波形显示优化
```c
// 波形显示在屏幕下半部分，高度40像素
uint8_t wave_start_y = 24;
uint8_t wave_height = 40;

for(i = 0; i < 128; i++)
{
    // 自适应缩放
    uint16_t scaled_height = (heart_sensor.display_wave[i] * wave_height) 
                             / heart_sensor.display_max;
    
    // 防止越界
    if(scaled_height > wave_height) 
        scaled_height = wave_height;
    
    // 计算Y坐标（从下往上绘制）
    uint8_t y = wave_start_y + wave_height - scaled_height;
    
    // 绘制波形
    OLED_DrawPoint(i, y, 1);
}
```

### 3.5 显示心率数值
```c
// 屏幕上半部分显示心率值
OLED_ShowNum(40, 0, heart_sensor.display_bpm, 3, 16, 0);
OLED_ShowString(64, 0, (char *)"bpm", 16, 0);

// 显示波形最大值
OLED_ShowNum(0, 0, heart_sensor.display_max, 3, 16, 0);
```

---

## 关键改进总结

| 改进项 | 效果 |
|--------|------|
| **时间戳控制** | 从每2ms更新一次改为每2秒更新一次 |
| **显示缓冲分离** | 采集缓冲和显示缓冲分开，稳定数据 |
| **自适应缩放** | 根据信号幅度自动调整波形高度 |
| **波形高度增加** | 从24像素增加到40像素，更清晰 |
| **最大值防护** | 防止信号过弱导致波形无法显示 |

---

## 编译指令

VS Code 中按 `Ctrl+Shift+B` 选择 build 任务编译：
```
shell: build
```

预期编译输出应该包含：
```
[ DONE ] build successfully !, elapsed time 0:0:X
```

---

## 测试步骤

1. 编译并烧录固件
2. 启动心率测量（按相关按键）
3. 观察OLED屏幕：
   - ✓ 数值每2秒更新一次（不再频繁跳动）
   - ✓ 曲线清晰可见，波动幅度大约40像素
   - ✓ 最大值显示在左上角

---

*修改日期: 2026-05-21*

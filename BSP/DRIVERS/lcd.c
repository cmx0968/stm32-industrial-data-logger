#include "lcd.h"
#include "font.h"
#include "task_app.h"
#include "sd_log.h"
#include <stdint.h>
#include <stdio.h>

#define WAVE_Y_TOP      50U                            // 波形区顶部（留标题栏）
#define WAVE_HEIGHT     (LCD_HEIGHT - WAVE_Y_TOP)        // 波形区高度 = 210
#define WAVE_POINTS     LCD_WIDTH                        // 波形点数 = 320


static uint16_t s_wave_buf[WAVE_POINTS];   // 存滤波后的 ADC 值
static uint16_t s_wave_head  = 0U;         // 写指针（下一个写入位置）
static uint16_t s_wave_count = 0U;         // 当前有效点数（0~WAVE_POINTS）

// 来自task.c的全局变量
extern volatile uint16_t g_last_adc;        // 最近一次 ADC 原始值
extern volatile uint32_t g_last_voltage;    // 最近一次电压（mV）
extern volatile uint8_t  g_alarm_flag;      // 告警标志：0=正常，1=告警



static void LCD_WriteCmd(uint16_t cmd)
{
    TFTLCD->LCD_CMD_ADDR = cmd;
}

static void LCD_WriteData(uint16_t data)
{
    TFTLCD->LCD_DATA_ADDR = data;
}

// 设置显示区域 
static void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    LCD_WriteCmd(0x2A);   // 列地址
    LCD_WriteData(x0 >> 8);   LCD_WriteData(x0 & 0xFF);
    LCD_WriteData(x1 >> 8);   LCD_WriteData(x1 & 0xFF);

    LCD_WriteCmd(0x2B);   // 行地址
    LCD_WriteData(y0 >> 8);   LCD_WriteData(y0 & 0xFF);
    LCD_WriteData(y1 >> 8);   LCD_WriteData(y1 & 0xFF);

    LCD_WriteCmd(0x2C);   // 开始写GRAM
}

static void LCD_WriteData_Color(uint16_t color)
{
    TFTLCD->LCD_DATA_ADDR = color >> 8;
    TFTLCD->LCD_DATA_ADDR = color & 0xFF;
}

// HX8357DN 初始化
void LCD_Init(void)
{
    LCD_BL_ON();
    HAL_Delay(50);

    LCD_WriteCmd(0x01);   // 软复位
    HAL_Delay(120);

    LCD_WriteCmd(0x11);   // 退出睡眠
    HAL_Delay(120);

    LCD_WriteCmd(0x36);   // 内存访问控制
    LCD_WriteData(0x4C);  // 行列交换 + BGR

    LCD_WriteCmd(0x3A);   // 像素格式
    LCD_WriteData(0x55);  // 16位 RGB565

    LCD_WriteCmd(0x29);   // 开显示

    LCD_Clear(LCD_BLACK);
}


// 清屏 
void LCD_Clear(uint16_t color)
{
    uint16_t i, j;
    LCD_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    for (i = 0; i < LCD_WIDTH ; i++)
    {
        for (j = 0; j < LCD_HEIGHT; j++)
        {
            LCD_WriteData_Color(color);
        }
    }
}

// 画点 
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    LCD_SetWindow(x, y, x, y);
    LCD_WriteData_Color(color);
}


// 画线
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;

    delta_x = x2 - x1; //计算坐标增量
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if(delta_x > 0) incx = 1; //设置单步方向
    else if(delta_x == 0) incx = 0; //垂直线
    else { incx = -1; delta_x = -delta_x; }

    if(delta_y > 0) incy = 1;
    else if(delta_y == 0) incy = 0; //水平线
    else { incy = -1; delta_y = -delta_y; }

    if( delta_x > delta_y ) distance = delta_x; //选取基本增量坐标轴
    else distance = delta_y;

    for(t = 0; t <= distance + 1; t++) //画线输出
    {
        LCD_DrawPixel(uRow, uCol, color); //画点
        xerr += delta_x;
        yerr += delta_y;

        if(xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if(yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

// 在指定位置显示一个字符 (固定8x16大小，非叠加模式)
// x,y:起始坐标
// ch:要显示的字符:" "--->"~"
// color:字符颜色
// bg:字符背景颜色
void LCD_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg)
{
    uint8_t temp, t1, t;
    uint16_t y0 = y;

    // 边界保护和偏移计算 
    if (ch < ' ' || ch > '~') ch = ' ';
    uint8_t num = ch - ' '; // 得到偏移后的值

    // 固定为8x16字体，共占用16个字节（16行，每行1个字节）
    for (t = 0; t < 16; t++)
    {
        temp = ascii_1608[num][t]; // 固定调用1608字体

        for (t1 = 0; t1 < 8; t1++)
        {
            // 直接画字符颜色或背景色
            if (temp & 0x80) 
                LCD_DrawPixel(x, y, color); // 画字符颜色
            else 
                LCD_DrawPixel(x, y, bg);    // 画背景颜色

            temp <<= 1;
            y++;

            // 超区域了 
            if (y > LCD_HEIGHT) return; 

            // 自动换行逻辑 (固定高度为16)
            if ((y - y0) == 16)
            {
                y = y0;
                x++;
                if (x > LCD_WIDTH) return; // 超区域了
                break;
            }
        }
    }
}
// 显示字符串 
void LCD_ShowString(uint16_t x, uint16_t y, char *str, uint16_t color, uint16_t bg)
{
    while (*str)
    {
        LCD_ShowChar(x, y, *str, color, bg);
        x += 8;
        if (x > LCD_WIDTH - 8) { x = 0; y += 16; }
        str++;
    }
}

void LCD_ClearArea(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint32_t total;
    uint32_t i;

    // 边界保护
    if (x0 >= LCD_WIDTH || y0 >= LCD_HEIGHT) return;
    if (x1 >= LCD_WIDTH)  x1 = LCD_WIDTH - 1U;
    if (y1 >= LCD_HEIGHT) y1 = LCD_HEIGHT - 1U;
    if (x0 > x1 || y0 > y1) return;

    // 设置显示窗口
    LCD_SetWindow(x0, y0, x1, y1);

    // 批量写黑色
    total = (uint32_t)(x1 - x0 + 1U) * (uint32_t)(y1 - y0 + 1U);
    for (i = 0U; i < total; i++)
    {
        LCD_WriteData_Color(LCD_BLACK);
    }
}

// 将滤波后的 ADC 值写入波形缓冲区
void LCD_PushWaveValue(uint16_t filtered_value)
{
    s_wave_buf[s_wave_head] = filtered_value;
    s_wave_head++;
    if (s_wave_head >= WAVE_POINTS)
    {
        s_wave_head = 0U;
    }
    if (s_wave_count < WAVE_POINTS)
    {
        s_wave_count++;
    }
}

/* ===== 显示任务 ===== */

static void LCD_ShowWaveInfo(void)
{
    char buf[40];
    uint16_t color;

    // 清文字栏
    LCD_ClearArea(0U, 0U, LCD_WIDTH - 1U, WAVE_Y_TOP - 1U);

    if (g_alarm_flag != 0U)
    {
        // 告警状态：红色
        color = LCD_RED;
        LCD_ShowString(0, 0, "! ALARM !", color, LCD_BLACK);

       snprintf(buf, sizeof(buf), "ADC:%-5u Volt:%-6lumV OVER  ", (unsigned)g_last_adc, (unsigned long)g_last_voltage);
        LCD_ShowString(0, 20, buf, color, LCD_BLACK);
    }
    else
    {
        // 正常状态：绿色
        color = LCD_GREEN;
        LCD_ShowString(0, 0, "Waveform", LCD_CYAN, LCD_BLACK);

        snprintf(buf, sizeof(buf), "ADC:%-5u Volt:%-6lumV  OK ", (unsigned)g_last_adc, (unsigned long)g_last_voltage);
        LCD_ShowString(0, 20, buf, color, LCD_BLACK);
    }
}


//波形显示
void LCD_ShowWaveform(void)
{
    uint16_t i;
    uint16_t start_idx;
    uint16_t y1, y2;
    uint16_t adc1, adc2;
    uint16_t wave_color;

    // 1. 刷新文字栏（显示 ADC 值、电压、告警状态）
    LCD_ShowWaveInfo();

    // 2. 根据告警状态选择波形颜色（告警时红色）
    wave_color = (g_alarm_flag != 0U) ? LCD_RED : LCD_GREEN;

    // 3. 只清波形区域
    LCD_ClearArea(0U, WAVE_Y_TOP, LCD_WIDTH - 1U, LCD_HEIGHT - 1U);

    // 4. 数据不够，直接返回
    if (s_wave_count < 2U) return;

    // 5. 计算起始索引
    start_idx = (s_wave_count < WAVE_POINTS) ? 0U : s_wave_head;

    // 6. 逐段绘制斜线
    for (i = 0U; i < s_wave_count - 1U; i++)
    {
        uint16_t idx1 = (start_idx + i) % WAVE_POINTS;
        uint16_t idx2 = (start_idx + i + 1U) % WAVE_POINTS;

        adc1 = s_wave_buf[idx1];
        adc2 = s_wave_buf[idx2];

        // 映射 Y：ADC=4095 → 顶部，ADC=0 → 底部
        y1 = (uint16_t)(WAVE_Y_TOP + (4095U - adc1) * WAVE_HEIGHT / 4095U);
        y2 = (uint16_t)(WAVE_Y_TOP + (4095U - adc2) * WAVE_HEIGHT / 4095U);

        if (y1 >= LCD_HEIGHT) y1 = LCD_HEIGHT - 1U;
        if (y2 >= LCD_HEIGHT) y2 = LCD_HEIGHT - 1U;

        LCD_DrawLine(i, y1, i + 1U, y2, wave_color);
    }
}


void LCD_ShowSystemStatus(void)
{
    char buf[64]; // 改大到64
    
    LCD_Clear(LCD_BLACK);
    LCD_ShowString(0, 0, "System Status", LCD_CYAN, LCD_BLACK);

    // ---- 栈余量 和 CPU占用率 合并显示 ----
    LCD_ShowString(0, 30, "Task    Stack  CPU", LCD_WHITE, LCD_BLACK);
    
    // Acq 任务
    snprintf(buf, sizeof(buf), "Acq :%4u b  %3d%%", 
             (unsigned)uxTaskGetStackHighWaterMark(xTaskAcqHandle), 
             Get_Task_CPU_Percent("Acq"));
    LCD_ShowString(10, 50, buf, LCD_GREEN, LCD_BLACK);

    // Proc 任务
    snprintf(buf, sizeof(buf), "Proc:%4u b  %3d%%", 
             (unsigned)uxTaskGetStackHighWaterMark(xTaskProcHandle), 
             Get_Task_CPU_Percent("Proc"));
    LCD_ShowString(10, 70, buf, LCD_GREEN, LCD_BLACK);

    // Key 任务
    snprintf(buf, sizeof(buf), "Key :%4u b  %3d%%", 
             (unsigned)uxTaskGetStackHighWaterMark(xTaskKeyHandle), 
             Get_Task_CPU_Percent("KeyScan"));
    LCD_ShowString(10, 90, buf, LCD_GREEN, LCD_BLACK);

    // Power 任务
    snprintf(buf, sizeof(buf), "Pwr :%4u b  %3d%%", 
             (unsigned)uxTaskGetStackHighWaterMark(xTaskPowerHandle), 
             Get_Task_CPU_Percent("PowerMon"));
    LCD_ShowString(10, 110, buf, LCD_GREEN, LCD_BLACK);

    // SDLog 任务
    snprintf(buf, sizeof(buf), "SD  :%4u b  %3d%%", 
             (unsigned)uxTaskGetStackHighWaterMark(xTaskSDLogHandle), 
             Get_Task_CPU_Percent("SDLog"));
    LCD_ShowString(10, 130, buf, LCD_GREEN, LCD_BLACK);

    // Lcd 任务
    snprintf(buf, sizeof(buf), "Lcd :%4u b  %3d%%", 
             (unsigned)uxTaskGetStackHighWaterMark(xTaskLcdHandle), 
             Get_Task_CPU_Percent("Lcd"));
    LCD_ShowString(10, 150, buf, LCD_GREEN, LCD_BLACK);

    // ---- 总 CPU 占用率 ----
    uint32_t cpu_usage_x10 = Get_CPU_Usage();
    snprintf(buf, sizeof(buf), "Total CPU: %lu.%lu%%   ", cpu_usage_x10 / 10, cpu_usage_x10 % 10);
    LCD_ShowString(0, 170, buf, LCD_YELLOW, LCD_BLACK);

    // ---- SD 卡容量 ----
      if (SD_IsMounted()) {
        uint32_t total_mb = 0, free_mb = 0;
        SD_GetCapacity(&total_mb, &free_mb);
        snprintf(buf, sizeof(buf), "SD: %lu/%lu MB", free_mb, total_mb);
        LCD_ShowString(0, 190, buf, LCD_WHITE, LCD_BLACK);
    } else {
        LCD_ShowString(0, 190, "SD Not Found!", LCD_RED, LCD_BLACK);
    }
}
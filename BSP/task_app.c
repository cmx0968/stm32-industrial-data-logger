#include "task_app.h"
#include "adc_app.h"
#include "ring_buffer.h"
#include "filter.h"
#include "usart.h"
#include "power_monitor.h"
#include "lcd.h"
#include "sd_log.h"
#include "rtc_helper.h"
#include "key.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TASK_ACQ_PRIORITY 40U  // 采集任务优先级
#define TASK_PROC_PRIORITY 30U  // 处理任务优先级
#define TASK_KEY_PRIORITY 30U // 按键任务优先级
#define TASK_POWER_PRIORITY 15U // 电源监测优先级
#define TASK_LCD_PRIORITY 35U // LCD显示优先级
#define TASK_SDLOG_PRIORITY 20U // SD日志任务优先级

#define ALARM_LOW_THRESHOLD  100U
#define ALARM_HIGH_THRESHOLD 3900U

SemaphoreHandle_t xSemAdcDone = NULL;   // ADC DMA 完成信号量
QueueHandle_t     xQueueAdcData = NULL; // ADC 数据队列

//任务句柄
TaskHandle_t xTaskAcqHandle = NULL;
TaskHandle_t xTaskProcHandle = NULL;
TaskHandle_t xTaskKeyHandle = NULL;
TaskHandle_t xTaskLcdHandle = NULL;
TaskHandle_t xTaskPowerHandle = NULL;
TaskHandle_t xTaskSDLogHandle = NULL;

// 全局缓冲区
ring_buffer_t g_rb;
filter_t      g_filter;

// 供lcd显示
volatile uint16_t g_last_adc     = 0U;
volatile uint32_t g_last_voltage = 0U;

volatile uint8_t g_alarm_flag    = 0U;     // 告警标志
volatile uint8_t g_alarm_latched = 0U;     // 告警锁存标志
volatile uint8_t g_display_mode  = 0U;

volatile uint32_t cpu_runtime_counter = 0U;
static char stats_buf[400]; // 静态缓存，避免频繁申请大栈空间

// 采集任务：等待信号量，把 DMA 数据写入环形缓冲区
static void vTaskAcquisition(void *pvParameters)
{
    while (1)
    {
        // 永久等待 DMA 完成信号量
        if (xSemaphoreTake(xSemAdcDone, portMAX_DELAY) == pdTRUE)
        {
            // 从 adc_dma_buffer 读出 100 个数据，写入环形缓冲区
            uint16_t *buf = ADC_App_GetBufferPtr();
            uint32_t len = ADC_App_GetBufferSize();
            for (uint32_t i = 0; i < len; i++)
            {
                ring_buffer_write(&g_rb, buf[i]);
            }
        }
    }
}

// 处理任务
static void vTaskProcess(void *pvParameters)
{
    uint16_t data;
    while (1)
    {
        // 读数据
        if (ring_buffer_read(&g_rb, &data) == 1U)
        {
            // 获取滤波后的值
            uint16_t filtered = filter_update(&g_filter, data);
            
            // 写入波形缓冲区，为后续显示波形做准备
            LCD_PushWaveValue(filtered);

            g_last_adc     =  filtered;
            g_last_voltage = (uint32_t)(filtered * 3300U / 4095U); // 参考电压为 3.3V

            // 检测电压
            if (filtered < ALARM_LOW_THRESHOLD || filtered > ALARM_HIGH_THRESHOLD) 
            {
                g_alarm_flag = 1U;
            }
            
            // 若已挂载SD卡，越界且未锁存，则记录日志 
            if (SD_Log_IsMounted())
            {
            
                if (g_alarm_flag && !g_alarm_latched)
                {
                    SD_LogRequest_t req =
                    {
                        .type       = SD_LOG_EVENT_ALARM,
                        .adc_value  = filtered,
                        .voltage_mv = g_last_voltage,
                        .alarm      = 1U,
                    };
                    RTC_GetDateTimeStr(req.timestamp, sizeof(req.timestamp));

                    if (SD_Log_Push(&req))
                    {
                        g_alarm_latched = 1U;
                        printf("Fault recorded, latched\r\n");
                    }
                }
            }

            //printf("ADC: %u, V:%lu mV, Filtered: %u\r\n", data, g_last_voltage, filtered);
            vTaskDelay(1);
        }
        else
        {
            // 缓冲区空，延时 10ms 再试
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

unsigned long getRunTimeCounterValue(void)
{
    return (unsigned long)cpu_runtime_counter;
}



uint32_t Get_CPU_Usage(void)
{
    static uint32_t last_calc_tick = 0;
    static uint32_t cpu_usage_x10 = 0;
    
    if (xTaskGetTickCount() - last_calc_tick < 1000) return cpu_usage_x10; 
    last_calc_tick = xTaskGetTickCount();

    vTaskGetRunTimeStats(stats_buf);

    char *idle_ptr = strstr(stats_buf, "IDLE");
    if (idle_ptr != NULL)
    {
        char *percent_ptr = strchr(idle_ptr, '%');
        if (percent_ptr != NULL)
        {
            char *num_ptr = percent_ptr;
            while (num_ptr > idle_ptr && (*(num_ptr - 1) >= '0' && *(num_ptr - 1) <= '9')) num_ptr--;
            int idle_percent = atoi(num_ptr);
            cpu_usage_x10 = (uint32_t)((100 - idle_percent) * 10);
        }
    }
    return cpu_usage_x10;
}
uint8_t Get_Task_CPU_Percent(char *task_name)
{
    static uint32_t last_calc_tick = 0;
    static uint8_t is_parsed = 0;
    
    // 每 1000 个 tick（1秒）重新获取并解析一次
    if (xTaskGetTickCount() - last_calc_tick >= 1000) {
        last_calc_tick = xTaskGetTickCount();
        vTaskGetRunTimeStats(stats_buf); // 刷新缓冲区
    }

    // 在缓冲区中查找任务名
    char *task_ptr = strstr(stats_buf, task_name);
    if (task_ptr != NULL)
    {
        // 向后找百分号
        char *percent_ptr = strchr(task_ptr, '%');
        if (percent_ptr != NULL)
        {
            // 回退寻找数字开始的位置
            char *num_ptr = percent_ptr;
            while (num_ptr > task_ptr && (*(num_ptr - 1) >= '0' && *(num_ptr - 1) <= '9')) {
                num_ptr--;
            }
            return (uint8_t)atoi(num_ptr);
        }
    }
    return 0; // 没找到
}

// 供电电压检测
static void vTaskPowerMonitor(void *pvParameters)
{   
    uint32_t voltage = 0U;
    while (1)
    {
		voltage = Power_Monitor_GetVoltage();
        if (voltage < POWER_VOLTAGE_MIN || voltage > POWER_VOLTAGE_MAX)
        {
            printf("Power Abnormal! %lu mV\r\n", voltage);
        }
        else
        {
            printf("%lu mV\r\n", voltage);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));   // 每 1 秒监测一次
    }
}

// lcd显示任务
static void vTaskLcd(void *pvParameters)
{
 
    while(1)
    {
        switch(g_display_mode)
        {
            case 0:
                LCD_ShowWaveform();
                break;
            case 1:
                LCD_ShowSystemStatus();
                break;
                
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void Task_App_Init(void)
{
    BaseType_t rc;

    // 创建二值信号量
    xSemAdcDone = xSemaphoreCreateBinary();
    if (xSemAdcDone == NULL)
    {
        printf("Sem create failed\r\n");
        while(1)
        {
        }
    }

    xQueueAdcData = xQueueCreate(16U, sizeof(uint16_t));
    if (xQueueAdcData == NULL)
    {
        printf("ADC queue create failed\r\n");
        while(1)
        {
        }
    }

    // 初始化环形缓冲区和滤波器
    ring_buffer_init(&g_rb);
    filter_init(&g_filter);

    // 创建任务，并检查返回值，避免任务创建失败后只剩空闲任务
    rc = xTaskCreate(vTaskAcquisition, "Acq", 256U, NULL, TASK_ACQ_PRIORITY, &xTaskAcqHandle);
    if (rc != pdPASS) { printf("Create Acq failed\r\n"); while(1) { } }

    rc = xTaskCreate(vTaskProcess,     "Proc", 256U, NULL, TASK_PROC_PRIORITY, &xTaskProcHandle);
    if (rc != pdPASS) { printf("Create Proc failed\r\n"); while(1) { } }

    rc = xTaskCreate(vTaskPowerMonitor, "PowerMon", 256U, NULL, TASK_POWER_PRIORITY, &xTaskPowerHandle);
    if (rc != pdPASS) { printf("Create PowerMon failed\r\n"); while(1) { } }

    rc = xTaskCreate(vTaskLcd, "Lcd", 512U, NULL, TASK_LCD_PRIORITY, &xTaskLcdHandle);
    if (rc != pdPASS) { printf("Create Lcd failed\r\n"); while(1) { } }

    rc = xTaskCreate(vTaskSDLog, "SDLog", 2048U, NULL, TASK_SDLOG_PRIORITY, &xTaskSDLogHandle);
    if (rc != pdPASS) { printf("Create SDLog failed\r\n"); while(1) { } }

    rc = xTaskCreate(vTaskKey, "KeyScan", 256U, NULL, TASK_KEY_PRIORITY, &xTaskKeyHandle);
    if (rc != pdPASS) { printf("Create Key failed\r\n"); while(1) { } }
}
#ifndef _SD_LOG_H_
#define _SD_LOG_H_

#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"


typedef enum {
    SD_LOG_EVENT_KEY   = 0,   // 按键手动记录
    SD_LOG_EVENT_ALARM = 1,   // 故障触发记录
} SD_LogType_t;


typedef struct {
    SD_LogType_t type;
    char         timestamp[20];   // 时间字符串
    uint16_t     adc_value;       // 滤波后的 ADC 值
    uint32_t     voltage_mv;      // 电压（mV）
    uint8_t      alarm;           // 是否越界：0=正常，1=越界
} SD_LogRequest_t;

/**
 * @brief  初始化 SD 卡日志模块，挂载 FATFS。
 * @return 0=成功，1=失败。
 */
uint8_t SD_Log_Init(void);

/**
 * @brief  检查 SD 卡是否已挂载。
 * @return 1=已挂载，0=未挂载。
 */
uint8_t SD_Log_IsMounted(void);

uint8_t SD_IsMounted(void);
uint32_t SD_GetCapacity(uint32_t *total_mb, uint32_t *free_mb);

/**
 * @brief  发送一条日志请求到队列（由按键任务/处理任务调用）。
 * @param  req  日志请求指针。
 * @return 1=成功入队，0=失败（队列满或 SD 未挂载）。
 */
uint8_t SD_Log_Push(const SD_LogRequest_t *req);

/**
 * @brief  SD 卡日志任务：从队列接收请求并写入文件。
 * @param  pvParameters  未使用。
 */
void vTaskSDLog(void *pvParameters);

#endif
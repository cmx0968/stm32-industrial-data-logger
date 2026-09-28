#ifndef _TASK_APP_H_
#define _TASK_APP_H_

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern SemaphoreHandle_t xSemAdcDone;   // ADC DMA 完成信号量
extern QueueHandle_t     xQueueAdcData; // ADC 数据队列
extern TaskHandle_t      xTaskAcqHandle;
extern TaskHandle_t      xTaskProcHandle;
extern TaskHandle_t      xTaskKeyHandle;
extern TaskHandle_t      xTaskLcdHandle;
extern TaskHandle_t      xTaskPowerHandle;
extern TaskHandle_t      xTaskSDLogHandle;
extern  volatile uint16_t          g_last_adc;
extern volatile uint32_t          g_last_voltage;
extern volatile uint8_t           g_alarm_flag;
extern volatile uint8_t           g_alarm_latched;
extern volatile uint8_t           g_display_mode;


uint32_t Get_CPU_Usage(void);
uint8_t Get_Task_CPU_Percent(char *task_name);

/**
* @brief  初始化缓冲区和滤波器，创建任务列表
 */
void Task_App_Init(void);

#endif /* _TASK_APP_H_ */

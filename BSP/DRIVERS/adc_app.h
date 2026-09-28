#ifndef _ADC_APP_H_
#define _ADC_APP_H_

#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"

//ADC DMA 缓冲区大小：100 个采样点 
#define ADC_DMA_BUFFER_SIZE  100U

//由其它模块（task_app.c）创建的信号量，DMA 完成时释放 
extern SemaphoreHandle_t xSemAdcDone;

/**
 * @brief  ADC 采集的 DMA 目标缓冲区，定义在 adc_app.c 中。
 */
extern uint16_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE];

/**
 * @brief  初始化 ADC+DMA+TIM 采集链路，启动后台自动采集。
 */
void ADC_App_Init(void);

/**
 * @brief  获取 DMA 目标缓冲区首地址。
 * @return 指向 adc_dma_buffer 的指针。
 */
uint16_t *ADC_App_GetBufferPtr(void);

/**
 * @brief  获取 DMA 目标缓冲区长度（元素个数）。
 * @return 缓冲区元素个数（ADC_DMA_BUFFER_SIZE）。
 */
uint32_t ADC_App_GetBufferSize(void);

#endif /* _ADC_APP_H_ */
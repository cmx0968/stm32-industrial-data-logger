#include "adc_app.h"
#include "adc.h"
#include "tim.h"


// DMA 目标缓冲区：存放 ADC 的 100 个采样值 
uint16_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE];


void ADC_App_Init(void)
{
    // 启动 ADC 的 DMA 传输，目标为 adc_dma_buffer，长度 100 个 Half Word
    // 注意：HAL 库函数原型要求 uint32_t*，但 DMA 实际按 Half Word 搬运
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma_buffer, ADC_DMA_BUFFER_SIZE);

    // 启动 TIM2，开始产生 TRGO 触发脉冲（每 2ms 触发一次 ADC）
    HAL_TIM_Base_Start(&htim2);
}


uint16_t *ADC_App_GetBufferPtr(void)
{
    return adc_dma_buffer;
}


uint32_t ADC_App_GetBufferSize(void)
{
    return (uint32_t)ADC_DMA_BUFFER_SIZE;
}

// DMA搬运完成后触发中断
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // 只在 ADC1 触发时处理
    if (hadc->Instance == ADC1)
    {
        // 任务创建前/配置未完成时，避免给空句柄，防止 ISR 中访问失效对象
        if (xSemAdcDone != NULL)
        {
            // 释放信号量唤醒采集任务。xSemAdcDone为在task_app.c中创建的信号量
            xSemaphoreGiveFromISR(xSemAdcDone, &xHigherPriorityTaskWoken);
        }

        // 如果采集任务优先级比当前任务高，请求一次上下文切换
        // 不写这行的话，采集任务可能要等到下一个SysTick才会运行，延迟了处理时间
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
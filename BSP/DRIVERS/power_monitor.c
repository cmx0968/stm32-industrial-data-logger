#include "power_monitor.h"
#include "adc.h"

void Power_Monitor_Init(void)
{
    // ADC2 已在 CubeMX 中初始化，此函数预留
}

/**
 * @brief  读取电源电压。
 * @return 电源电压（单位：mV）。
 */
uint32_t Power_Monitor_GetVoltage(void)
{
    uint32_t adc_value = 0U;
    uint32_t voltage = 0U;

    // 启动 ADC2 转换
    HAL_ADC_Start(&hadc2);

    // 等待转换完成（超时 10ms）
    if (HAL_ADC_PollForConversion(&hadc2, 10) == HAL_OK)
    {
        adc_value = HAL_ADC_GetValue(&hadc2);
    }

    HAL_ADC_Stop(&hadc2);

    // 计算分压点电压（mV）：adc_value / 4095 * 3300
    uint32_t v_adc_mv = (adc_value * 3300U) / 4095U;

    // 反推电源电压：V_power = V_adc * (R1 + R2) / R2
    voltage = v_adc_mv * (POWER_R1 + POWER_R2) / POWER_R2;

    return voltage;
}
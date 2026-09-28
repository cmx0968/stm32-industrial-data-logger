#ifndef _POWER_MONITOR_H_
#define _POWER_MONITOR_H_

#include "main.h"

// 分压电阻值（单位：欧姆） 
#define POWER_R1  10000U
#define POWER_R2  10000U

// 电源电压告警阈值（单位：mV） 
#define POWER_VOLTAGE_MIN  4300U   // 低于 4.3V 告警
#define POWER_VOLTAGE_MAX  5500U   // 高于 5.5V 告警

/**
 * @brief  初始化电源监测 ADC。
 */
void Power_Monitor_Init(void);

/**
 * @brief  读取电源电压。
 * @return 电源电压（单位：mV）。
 */
uint32_t Power_Monitor_GetVoltage(void);

#endif /* _POWER_MONITOR_H_ */
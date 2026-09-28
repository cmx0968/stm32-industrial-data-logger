#ifndef _FILTER_H_
#define _FILTER_H_

#include <stdint.h>
#include <stddef.h>


#ifndef FILTER_WINDOW_SIZE
#define FILTER_WINDOW_SIZE  10U
#endif

#if (FILTER_WINDOW_SIZE < 1U)
#error "FILTER_WINDOW_SIZE must be >= 1"
#endif

#if (FILTER_WINDOW_SIZE > 32U)
#warning "FILTER_WINDOW_SIZE > 32: 延迟大；若误用 uint16_t 做累加和会溢出"
#endif

typedef struct
{
    uint16_t window[FILTER_WINDOW_SIZE]; // 固定窗口，环形存放最近若干采样
    uint16_t index;                      // 下一个要被替换的位置（最旧数据）
    uint16_t count;                      // 窗口内已有点数，0 ~ FILTER_WINDOW_SIZE 
    uint32_t sum;                        // 窗口内数值之和，用 32 位避免 uint16_t 溢出 
} filter_t;

/**
 * @brief  初始化滑动平均滤波器（窗口为空）。
 * @param  f  滤波器实例，须为静态或全局变量。
 */
void filter_init(filter_t *f);

/**
 * @brief  清空窗口，下次从空窗口重新积累。
 * @param  f  滤波器实例。
 */
void filter_reset(filter_t *f);

/**
 * @brief  写入一个新采样，替换最旧点，返回当前窗口平均值。
 * @param  f       滤波器实例。
 * @param  sample  新的 ADC 采样值。
 */
uint16_t filter_update(filter_t *f, uint16_t sample);

#endif /* _FILTER_H_ */

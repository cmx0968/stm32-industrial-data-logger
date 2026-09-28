// rtc_helper.c
#include "rtc_helper.h"
#include "rtc.h"
#include <stdio.h>

/**
 * @brief  获取当前 RTC 时间，格式化为字符串。
 * @param  buf  输出缓冲区，建议至少 20 字节。
 * @param  len  缓冲区长度。
 */
void RTC_GetDateTimeStr(char *buf, uint8_t len)
{
    RTC_TimeTypeDef s_time;
    RTC_DateTypeDef s_date;

    // 必须先读 Time，再读 Date（释放影子寄存器）
    HAL_RTC_GetTime(&hrtc, &s_time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &s_date, RTC_FORMAT_BIN);

    snprintf(buf, len, "%02u-%02u-%02u %02u:%02u:%02u",
             (unsigned)s_date.Year,
             (unsigned)s_date.Month,
             (unsigned)s_date.Date,
             (unsigned)s_time.Hours,
             (unsigned)s_time.Minutes,
             (unsigned)s_time.Seconds);
}
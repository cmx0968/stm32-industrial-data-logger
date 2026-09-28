#ifndef _RTC_HELPER_H_
#define _RTC_HELPER_H_

#include "main.h"

/**
 * @brief  获取当前 RTC 时间，格式化为字符串。
 * @param  buf  输出缓冲区，建议至少 20 字节。
 * @param  len  缓冲区长度。
 */
void RTC_GetDateTimeStr(char *buf, uint8_t len);

#endif
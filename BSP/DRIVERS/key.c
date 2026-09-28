// key.c
#include "key.h"
#include "sd_log.h"
#include "task_app.h"
#include "rtc_helper.h"
#include <stdio.h>

extern volatile uint8_t g_alarm_flag;
extern volatile uint8_t g_alarm_latched;
extern volatile uint8_t g_display_mode;
extern volatile uint16_t g_last_adc;
extern volatile uint32_t g_last_voltage;

// 按键消抖参数
#define KEY_DEBOUNCE_CNT  3U   // 连续 3 次扫描到同一状态才确认


typedef struct {
    uint8_t last_state;    // 上次稳定状态
    uint8_t debounce_cnt;  // 消抖计数
} KeyState_t;

static KeyState_t keys[4] = {0};   // KEY_UP, KEY0, KEY1, KEY2

/**
 * @brief  读取四个按键的当前按下状态。
 */
static void Key_ReadAll(uint8_t *pressed)
{
    pressed[0] = (KEY_UP == 1) ? 1U : 0U;   // 高电平按下
    pressed[1] = (KEY0 == 0)   ? 1U : 0U;   // 低电平按下
    pressed[2] = (KEY1 == 0)   ? 1U : 0U;
    pressed[3] = (KEY2 == 0)   ? 1U : 0U;
}

/**
 * @brief  按键扫描任务
 */
void vTaskKey(void *pvParameters)
{
    uint8_t pressed[4];

    while (1)
    {
        Key_ReadAll(pressed);

        for (uint8_t i = 0U; i < 4U; i++)
        {
            if (pressed[i] != keys[i].last_state) // 检测边沿信号
            {
                keys[i].debounce_cnt++;
                if (keys[i].debounce_cnt >= KEY_DEBOUNCE_CNT)
                {
                    keys[i].last_state = pressed[i];
                    keys[i].debounce_cnt = 0U;

                    // 只在"按下"时触发事件
                    if (pressed[i] == 1U)
                    {
                        switch (i)
                        {
                            case 0:   // KEY_UP
                                g_alarm_latched = 0U;
                                g_alarm_flag    = 0U;
                                printf("Alarm confirmed\r\n");
                                break;

                            case 1:   // KEY0
                            {
                                if (SD_Log_IsMounted()) 
								{
									SD_LogRequest_t req = 
									{
										.type       = SD_LOG_EVENT_KEY,
										.adc_value  = g_last_adc,
										.voltage_mv = g_last_voltage,
										.alarm      = g_alarm_flag,
									};
									RTC_GetDateTimeStr(req.timestamp, sizeof(req.timestamp));
									if (SD_Log_Push(&req)) 
									{
                                        printf("Key event logged\r\n");
                                    }
                                }
                                else 
                                    printf("SD not inserted, key log ignored\r\n");
                                break;
                            }

                            case 2:   // KEY1
                                g_display_mode = (g_display_mode + 1U) % 2U;
                                printf("Display mode: %u\r\n", g_display_mode);
                                break;

                            default:
                                break;
                        }
                    }
                }
            }
            else
            {
                keys[i].debounce_cnt = 0U;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));   // 每 10ms 扫描一次
    }
}
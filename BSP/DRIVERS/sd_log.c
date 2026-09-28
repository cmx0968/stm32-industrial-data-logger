#include "sd_log.h"
#include "led.h"
#include "ring_buffer.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>

// FATFS 文件系统对象 
static FATFS g_fs;
static uint8_t g_sd_mounted = 0U;

uint8_t SD_IsMounted(void)
{
    return SD_Log_IsMounted();
}

uint32_t SD_GetCapacity(uint32_t *total_mb, uint32_t *free_mb)
{
    FATFS *fs;
    DWORD fre_clust;
    DWORD total_sect;
    DWORD free_sect;

    if (total_mb != NULL) *total_mb = 0U;
    if (free_mb  != NULL) *free_mb  = 0U;

    if (!g_sd_mounted) return 1U;

    // 获取剩余簇数，同时拿到 fs 指针
    if (f_getfree("", &fre_clust, &fs) != FR_OK) return 1U;

    // 总扇区数 = (总簇数 - 2) × 每簇扇区数
    total_sect = (fs->n_fatent - 2) * fs->csize;

    // 剩余扇区数 = 剩余簇数 × 每簇扇区数
    free_sect  = fre_clust * fs->csize;

    // 每扇区 512 字节 → 除以 1024 得到 KB → 除以 1024 得到 MB
    // 简化：(sect × 512) / (1024 × 1024) = sect / 2048
    if (total_mb != NULL) *total_mb = total_sect / 2048U;
    if (free_mb  != NULL) *free_mb  = free_sect  / 2048U;

    return 0U;
}

// 日志队列 
static QueueHandle_t xQueueLog = NULL;

// 日志文件名 
#define LOG_FILE_NAME   "fault_log.csv"
// 事故前数据快照长度（从 ring_buffer 取最近 N 个点） 
#define PRE_FAULT_POINTS   50U

extern ring_buffer_t g_rb;


uint8_t SD_Log_Init(void)
{
    printf("SD_Log_Init: start\r\n");
    // 创建日志队列
    xQueueLog = xQueueCreate(10, sizeof(SD_LogRequest_t));
    if (xQueueLog == NULL)
    {
        printf("SD Queue Create Failed! Heap Out!\r\n"); 
        return 1U;
    } 
    printf("SD_Log_Init: queue ok\r\n");

    // 挂载 FATFS
    FRESULT res = f_mount(&g_fs, "", 1);
    printf("SD_Log_Init: f_mount res=%d\r\n", res);
    if (res != FR_OK) {
        printf("SD_Log_Init: mount failed, res=%d\r\n", res);
        g_sd_mounted = 0U;
        return 1U;
    }

    g_sd_mounted = 1U;
    printf("SD_Log_Init: mount ok\r\n");
    return 0U;
}

// 检查SD卡是否挂载
uint8_t SD_Log_IsMounted(void)
{
    return g_sd_mounted;
}

/**
 * @brief  发送一条日志请求到队列。
 * @param  req  日志请求指针。
 * @return 1=成功入队，0=失败。
 */
uint8_t SD_Log_Push(const SD_LogRequest_t *req)
{
    if (!g_sd_mounted) return 0U;   // SD 未挂载，直接拒绝
    if (req == NULL)   return 0U;
    if (xQueueLog == NULL) return 0U;

    // 非阻塞发送，队列满则返回失败
    return (xQueueSend(xQueueLog, req, 0) == pdTRUE) ? 1U : 0U;
}

/**
 * @brief  写入一条日志到文件（内部函数）。
 * @param  req  日志请求指针。
 */
static void SD_WriteOneLog(const SD_LogRequest_t *req)
{
    static FIL file;
    UINT bw;
    static char line[128];

    // 以追加方式打开文件（不存在则创建）
    if (f_open(&file, LOG_FILE_NAME, FA_OPEN_ALWAYS | FA_WRITE) != FR_OK) {
        printf("Open log file failed\r\n");
        return;
    }

    // 移动到文件末尾
    f_lseek(&file, f_size(&file));

    // 构造日志行
    snprintf(line, sizeof(line),
             "%s,%s,ADC:%u,Volt:%lumV,Alarm:%u\r\n",
             req->timestamp, //字符串
             (req->type == SD_LOG_EVENT_ALARM) ? "ALARM" : "KEY",
             (unsigned)req->adc_value,
             (unsigned long)req->voltage_mv,
             (unsigned)req->alarm);

    f_write(&file, line, strlen(line), &bw);
    f_sync(&file);  //同步文件
    f_close(&file);

    printf("Log written: %s", line);
}

/**
 * @brief  故障时写入事故前数据和当前数据。
 * @param  req  日志请求指针。
 */
static void SD_WriteAlarmLog(const SD_LogRequest_t *req)
{
    static FIL file;
    UINT bw;
    static char line[128];
    static uint16_t pre_data[PRE_FAULT_POINTS];
    uint16_t i;
    uint16_t idx;
    uint16_t count = 0U;
    uint16_t adc_val;

    // 从 ring_buffer 提取最近 PRE_FAULT_POINTS 个点（事故前数据）
    for (i = 0U; i < PRE_FAULT_POINTS; i++) {
        if (ring_buffer_read(&g_rb, &adc_val) == 1U) {
            pre_data[count++] = adc_val;
        } else {
            break;
        }
    }

    // 打开文件
    if (f_open(&file, LOG_FILE_NAME, FA_OPEN_ALWAYS | FA_WRITE) != FR_OK) {
        printf("Open file failed\r\n");
        return;
    }

    f_lseek(&file, f_size(&file));

    // 1. 写故障摘要
    snprintf(line, sizeof(line),
             "\r\n=== FAULT @ %s ===\r\nADC:%u Volt:%lumV\r\nPRE-FAULT DATA:\r\n",
             req->timestamp,
             (unsigned)req->adc_value,
             (unsigned long)req->voltage_mv);
    f_write(&file, line, strlen(line), &bw);

    // 2. 写事故前数据（每行 10 个点，便于阅读）
    for (i = 0U; i < count; i++) {
        idx = i % 10U;
        snprintf(&line[idx * 6U], 7U, "%5u,", (unsigned)pre_data[i]);
        if (idx == 9U || i == count - 1U) {
            uint16_t line_len = (idx + 1U) * 6U;
            line[line_len] = '\r';
            line[line_len + 1U] = '\n';
            f_write(&file, line, line_len + 2U, &bw);
        }
    }

    static const char END_MARK[] = "=== END FAULT ===\r\n\r\n";
    f_write(&file, END_MARK, sizeof(END_MARK) - 1U, &bw);
    f_sync(&file);
    f_close(&file);

    printf("Alarm log written, including %u pre-fault samples\r\n", count);
}

/**
 * @brief  SD 卡日志任务：从队列接收请求并写入文件。
 * @param  pvParameters  未使用。
 */
void vTaskSDLog(void *pvParameters)
{
    SD_LogRequest_t req;
    if (SD_Log_Init() != 0) {
    printf("SDLog Init Failed! Task Deleted.\r\n");
    vTaskDelete(NULL); // 直接删除自己，别往下跑
    return;
    }


    while (1)
    {
        // 阻塞等待日志请求
        if (xQueueReceive(xQueueLog, &req, portMAX_DELAY) == pdTRUE)
        {
            // 再检查一次 SD 是否挂载（防止中途拔出）
            if (!g_sd_mounted) {
                printf("SD not mounted, log dropped\r\n");
                continue;
            }

            // 点亮 LED0，表示正在写卡
            LED0_On();

            if (req.type == SD_LOG_EVENT_ALARM) {
                SD_WriteAlarmLog(&req);     // 故障日志（含事故前数据）
            } else {
                SD_WriteOneLog(&req);       // 普通按键日志
            }
            vTaskDelay(pdMS_TO_TICKS(300));
            // 写完灭灯
            LED0_Off();
        }
    }
}
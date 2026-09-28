#include "ring_buffer.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief  指针+1，到最大值后绕回0
 * @note   为什么不使用取模：更耗时；人工复制的时候可能出错
 */
static uint16_t rb_next_index(uint16_t index)
{
    index++; //指针+1
    if (index >= (uint16_t)RING_BUFFER_SIZE)
    {
        index = 0U; //指针绕回0，实现环形缓冲区
    }
    return index;
}


void ring_buffer_init(ring_buffer_t *rb)
{
    ring_buffer_clear(rb);
}


void ring_buffer_write(ring_buffer_t *rb, uint16_t data)
{
    if (rb == NULL)
    {
        return;
    }

    taskENTER_CRITICAL();
    // 覆盖最旧：若已满：tail 前进一步（丢弃最旧），count 先减 1，给新数据腾出一个逻辑槽位。
    if (rb->count >= (uint16_t)RING_BUFFER_SIZE)
    {
        rb->tail = rb_next_index(rb->tail);
        rb->count--;
    }

    rb->buffer[rb->head] = data; //把 data 写到 buffer[head]。
    rb->head = rb_next_index(rb->head); //head 前进一步；
    rb->count++; //槽位减少一个
    taskEXIT_CRITICAL();
}


uint8_t ring_buffer_read(ring_buffer_t *rb, uint16_t *data)
{
    uint8_t ok;

    if ((rb == NULL) || (data == NULL)) //参数无效
    {
        return 0U;
    }

    taskENTER_CRITICAL();

    if (rb->count == 0U) //空
    {
        ok = 0U;
    }
    else
    {
        *data = rb->buffer[rb->tail]; //读出最旧的一个采样值
        rb->tail = rb_next_index(rb->tail); //tail 前进一步；
        rb->count--; //槽位增加一个
        ok = 1U; //成功
    }

    taskEXIT_CRITICAL();
    return ok;
}


uint8_t ring_buffer_is_empty(const ring_buffer_t *rb)
{
    uint8_t empty;

    if (rb == NULL)
    {
        return 1U;
    }

    taskENTER_CRITICAL();
    empty = (rb->count == 0U) ? 1U : 0U; //是否为空，空（1），非空（0）
    taskEXIT_CRITICAL();

    return empty;
}

//本函数只提供“是否已在覆盖窗口”的信息。
uint8_t ring_buffer_is_full(const ring_buffer_t *rb)
{
    uint8_t full;

    if (rb == NULL)
    {
        return 0U;
    }

    taskENTER_CRITICAL();
    full = (rb->count >= (uint16_t)RING_BUFFER_SIZE) ? 1U : 0U;
    taskEXIT_CRITICAL();

    return full;
}


uint16_t ring_buffer_count(const ring_buffer_t *rb)
{
    uint16_t n;

    if (rb == NULL)
    {
        return 0U;
    }

    taskENTER_CRITICAL();
    n = rb->count; //当前有效采样个数
    taskEXIT_CRITICAL();

    return n;
}

//清空：等价于丢弃所有未读采样。常用于模式切换、故障恢复。
void ring_buffer_clear(ring_buffer_t *rb)
{
    if (rb == NULL)
    {
        return;
    }

    // 防止 clear 过程中 ISR 已经开始 write
    taskENTER_CRITICAL();
    rb->head = 0U;
    rb->tail = 0U;
    rb->count = 0U;
    taskEXIT_CRITICAL();
}

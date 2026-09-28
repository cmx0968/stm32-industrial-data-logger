#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <stdint.h>
#include <stddef.h>

// 固定容量：可按 RAM 和采样速率改。不必是 2 的幂，但 2 的幂时取模可改成按位与。
#ifndef RING_BUFFER_SIZE
#define RING_BUFFER_SIZE  256U
#endif

#if (RING_BUFFER_SIZE < 1U) || (RING_BUFFER_SIZE > 65535U)
#error "RING_BUFFER_SIZE must be in range 1 .. 65535"
#endif


typedef struct
{
    uint16_t buffer[RING_BUFFER_SIZE]; //实际数据区
    uint16_t head; //下一个“写入位置”（写完后 head 前进）
    uint16_t tail; //下一个“读取位置”（读完后 tail 前进）
    uint16_t count; //当前有效数据个数，范围 0 ~ RING_BUFFER_SIZE
} ring_buffer_t;

/**
 * @brief  初始化环形缓冲区为空。
 */
void ring_buffer_init(ring_buffer_t *rb);

/**
 * @brief  写入一个 uint16_t。缓冲区满时覆盖最旧数据。
 * @param  rb     缓冲区
 * @param  data   ADC 原始值（12 位 ADC 也放在 uint16_t 低 12 位即可）
 */
void ring_buffer_write(ring_buffer_t *rb, uint16_t data);

/**
 * @brief  读取一个 uint16_t（取出最旧的有效数据）。
 * @param  rb     缓冲区
 * @param  data   读出值的存放地址，不能为 NULL
 */
uint8_t ring_buffer_read(ring_buffer_t *rb, uint16_t *data);

/**
 * @brief  判断缓冲区是否为空。
 * @param  rb     缓冲区
 */
uint8_t ring_buffer_is_empty(const ring_buffer_t *rb);

/**
 * @brief  判断缓冲区是否已满。
 * @param  rb     缓冲区
 */
uint8_t ring_buffer_is_full(const ring_buffer_t *rb);

/**
 * @brief  获取当前已存数据个数。
 * @param  rb     缓冲区
 */
uint16_t ring_buffer_count(const ring_buffer_t *rb);

/**
 * @brief  丢弃全部数据，恢复为空（不擦除 buffer 内容）。
 * @param  rb     缓冲区
 */
void ring_buffer_clear(ring_buffer_t *rb);

#endif /* _RING_BUFFER_H_ */

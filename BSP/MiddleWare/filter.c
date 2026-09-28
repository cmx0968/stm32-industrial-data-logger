#include "filter.h"

void filter_reset(filter_t *f)
{
    if (f == NULL)
    {
        return;
    }

    // 只丢逻辑状态；window[] 旧值在 count==0 时不会参与平均
    f->index = 0U;
    f->count = 0U;
    f->sum   = 0U;
}

void filter_init(filter_t *f)
{
    filter_reset(f);
}

uint16_t filter_update(filter_t *f, uint16_t sample)
{
    uint16_t average;

    if (f == NULL)
    {
        return 0U;
    }

    if (f->count >= (uint16_t)FILTER_WINDOW_SIZE)
    {
        // 窗口已满：先从总和里减去即将被覆盖的最旧值
        f->sum -= f->window[f->index];
    }
    else
    {
        // 尚未填满：有效点数加 1，平均按实际点数算，避免开头被 0 拉低
        f->count++;
    }

    // 新数据覆盖最旧槽位，并加进总和
    f->window[f->index] = sample;
    f->sum += sample;

    // 写指针环绕，下次仍指向最旧数据
    f->index++;
    if (f->index >= (uint16_t)FILTER_WINDOW_SIZE)
    {
        f->index = 0U;
    }

    // 整数除法，结果仍是 uint16_t
    average = (uint16_t)(f->sum / (uint32_t)f->count);
    return average;
}

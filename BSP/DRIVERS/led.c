#include "led.h"

void led_init(led_desc_t led)
{
};

void led_set(led_desc_t led, _Bool onoff)
{
	HAL_GPIO_WritePin(led->Port, led->Pin, onoff ? led->OnBit : led->OffBit);
};

void led_on(led_desc_t led)
{
	HAL_GPIO_WritePin(led->Port, led->Pin, led->OnBit);
};

void led_off(led_desc_t led)
{
	HAL_GPIO_WritePin(led->Port, led->Pin, led->OffBit);
};

void led_toggle(led_desc_t led)
{
	HAL_GPIO_TogglePin(led->Port, led->Pin);
}

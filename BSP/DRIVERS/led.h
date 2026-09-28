#ifndef _LED_H_
#define _LED_H_

#include "main.h"

struct led_desc
{
    GPIO_TypeDef *Port;
    uint16_t Pin;
    GPIO_PinState OnBit;
    GPIO_PinState OffBit;
};

typedef struct led_desc *led_desc_t;

extern struct led_desc led0;
extern struct led_desc led1;

#define LED0_On()   led_on(&led0)
#define LED0_Off()  led_off(&led0)
#define LED1_On()   led_on(&led1)
#define LED1_Off()  led_off(&led1)

void led_init(led_desc_t led);
void led_set(led_desc_t led, _Bool onoff);
void led_on(led_desc_t led);
void led_off(led_desc_t led);
void led_toggle(led_desc_t led);

#endif
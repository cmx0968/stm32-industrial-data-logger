#include "board.h"
#include "led.h"
#include "task_app.h"
#include "adc_app.h"
#include "lcd.h"

//LED0 PF9
//LED1 PF10
struct led_desc led0 = {GPIOF, GPIO_PIN_9, GPIO_PIN_RESET, GPIO_PIN_SET};
struct led_desc led1 = {GPIOF, GPIO_PIN_10, GPIO_PIN_RESET, GPIO_PIN_SET};

void led_off_all(void)
{
    led_off(&led0);
    led_off(&led1);
}

void Board_Init(void)
{
    led_off_all();
    LCD_Init();
    Task_App_Init();
    ADC_App_Init();
}
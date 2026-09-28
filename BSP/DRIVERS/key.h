#ifndef _KEY_H_
#define _KEY_H_

#include "main.h"

//定义管脚
#define KEY0_PIN GPIO_PIN_4 
#define KEY1_PIN GPIO_PIN_3 
#define KEY2_PIN GPIO_PIN_2 
#define KEY_UP_PIN GPIO_PIN_0 

//定义端口
#define KEY_PORT GPIOE 
#define KEY_UP_PORT GPIOA

#define KEY_UP HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)
#define KEY0 HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)
#define KEY1 HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3)
#define KEY2 HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2)

void vTaskKey(void *pvParameters);



#endif
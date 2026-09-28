#ifndef _LCD_H_
#define _LCD_H_

#include "main.h"
#include <stdint.h>

// LCD 尺寸 
#define LCD_WIDTH   320U
#define LCD_HEIGHT  480U

// 背光：PB15，高电平点亮 
#define LCD_BL_Pin        GPIO_PIN_15
#define LCD_BL_Port       GPIOB
#define LCD_BL_ON()       HAL_GPIO_WritePin(LCD_BL_Port, LCD_BL_Pin, GPIO_PIN_SET)
#define LCD_BL_OFF()      HAL_GPIO_WritePin(LCD_BL_Port, LCD_BL_Pin, GPIO_PIN_RESET)

typedef struct 
{
    uint16_t LCD_CMD_ADDR;
    uint16_t LCD_DATA_ADDR;
}TFTLCD_TypeDef;

#define TFTLCD_BASE ((uint32_t)(0x6C000000 | 0x0000007E))
#define TFTLCD ((TFTLCD_TypeDef *) TFTLCD_BASE)

// 常用颜色（RGB565） 
#define LCD_WHITE   0xFFFFU
#define LCD_BLACK   0x0000U
#define LCD_RED     0xF800U
#define LCD_GREEN   0x07E0U
#define LCD_BLUE    0x001FU
#define LCD_YELLOW  0xFFE0U
#define LCD_CYAN    0x7FFFU

void LCD_Init(void);
void LCD_Clear(uint16_t color);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg);
void LCD_ShowString(uint16_t x, uint16_t y, char *str,  uint16_t color, uint16_t bg);
void LCD_PushWaveValue(uint16_t filtered_value);
void LCD_ShowWaveform(void);
void LCD_ShowSystemStatus(void);

#endif /* _LCD_H_ */
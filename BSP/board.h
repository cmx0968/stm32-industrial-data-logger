#ifndef _BOARD_H_
#define _BOARD_H_

struct led_desc;

extern struct led_desc led0;
extern struct led_desc led1;
void led_off_all(void);
void Board_Init(void);

#endif
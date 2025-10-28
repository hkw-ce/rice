#ifndef _KEY_H
#define _KEY_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "platform.h"
#include "multi_button.h"

#define K_1       GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)
#define K_2       GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6)
//#define K_3       GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12)

#define LONG_PRESS_THRESHOLD 1000  // 长按阈值(ms)

typedef enum
{
    KEY_IDLE = 0,
    KEY_PRESSED,
} Key_state_t;

typedef enum
{
    LONG_PRESS_LEFT_KEY = 1,    //左键长按
    LEFT_KEY = 2,               //左键短按
    RIGHT_KEY = 3,              //右键
} Key_des_t;

int Key_Init(void);
uint8_t Key_Scan(void);

#endif  /* _KEY_H */

#ifndef _KEY_H
#define _KEY_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "platform.h"
#include "multi_button.h"

#define K_1       GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11)
#define K_2       GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)
//#define K_3       GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12)

// 状态机
typedef enum {
    STANDBY,
    RICE_SEL,
    MODE_SEL,
    BOOK_SET,
    COOKING,
    WARMING,
    BOOK_WAIT
} sys_state_t;

extern sys_state_t state ;
extern sys_state_t last_state;

static uint8_t rice_type = 0;
static uint8_t last_rice_type = 0xFF;

static uint8_t cook_mode = 0;
static uint8_t last_cook_mode = 0xFF;

static uint8_t book_hour = 0;
static uint8_t last_book_hour = 0xFF;

// === 烹饪计时变量（非阻塞）===
static rt_uint32_t cook_total_sec = 0;     // 总烹饪时间（秒）
static rt_tick_t   cook_start_tick = 0;   // 开始烹饪的 tick
static rt_bool_t   cook_active = RT_FALSE;

// === 预约计时变量 ===
static rt_tick_t   book_start_tick = 0;
static rt_bool_t   book_active = RT_FALSE;

static uint32_t cook_timer = 0;   // 秒
static uint32_t book_timer = 0;   // 秒

// 米种名称
extern const char *rice_name[] ;

// 模式名称
extern const char *mode_name[];

// 状态名称（用于日志）
extern const char *state_name[] ;
/* thread_logic_task.c */
static rt_uint32_t last_book_remain = 0xFFFFFFFF;  // 上次打印的预约剩余秒数
static rt_uint32_t last_cook_remain = 0xFFFFFFFF;  // 上次打印的烹饪剩余秒数

int Key_Init(void);
uint8_t Key_Scan(void);

#endif  /* _KEY_H */

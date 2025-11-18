
/*=====================================================================
 * File: key.c
 * 商用级 UI 状态机 | 防误操作 | 米种/模式/预约三步确认
 * 特性：
 *   - 短按：切换选项
 *   - 长按：确认并进入下一级
 *   - 双击：仅在 BOOK_SET 有效（快速启动）
 *   - BTN2：一键热饭（普通米 + 热饭 + 立即）
 *   - 所有状态切换带详细日志
=====================================================================*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "key.h"


#define LOG_TAG     "KEY"
#define LOG_LVL     ELOG_LVL_INFO

//==================== 按键结构体 ====================
struct Button btn1;
struct Button btn2;

//==================== 选项名称 ====================
const char *rice_name[] = {"普通米", "糯米", "东北米"};
const char *mode_name[] = {"清洁", "快煮", "稀饭", "热饭"};
const char *state_name[] = {
    "STANDBY", "RICE_SEL", "MODE_SEL", "BOOK_SET",
    "COOKING", "WARMING", "BOOK_WAIT"
};


//==================== 全局状态 & 参数 ====================
sys_state_t state = STANDBY;
sys_state_t last_state = (sys_state_t)-1;

 sys_state_t state ;
 sys_state_t last_state;
 uint8_t rice_type = 0;
 uint8_t last_rice_type = 0xFF;
 uint8_t cook_mode = 0;
 uint8_t last_cook_mode = 0xFF;
 uint8_t book_hour = 0;
 uint8_t last_book_hour = 0xFF;

// === 烹饪计时变量（非阻塞）===
 rt_uint32_t cook_total_sec = 0;     // 总烹饪时间（秒）
 rt_tick_t   cook_start_tick = 0;   // 开始烹饪的 tick
 rt_bool_t   cook_active = RT_FALSE;

// === 预约计时变量 ===
 rt_tick_t   book_start_tick = 0;
 rt_bool_t   book_active = RT_FALSE;
 uint32_t cook_timer = 0;   // 秒
 uint32_t book_timer = 0;   // 秒


//uint8_t rice_type = 0;
//uint8_t cook_mode = 0;
//uint8_t book_hour = 0;

// 煮饭控制标志（来自 ih_control.c）
extern rt_bool_t cook_active;
extern rt_bool_t book_active;
extern rt_tick_t cook_start_tick;
extern rt_tick_t book_start_tick;
extern uint32_t last_cook_remain;
extern uint32_t last_book_remain;

//==================== GPIO 初始化 ====================
int gpio_init(void)
{
    gpio_config(GPIOC, GPIO_Pin_15, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);  // 系统运行灯
    gpio_config(GPIOA, GPIO_Pin_1,  GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
    gpio_config(GPIOB, GPIO_Pin_8,  GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
    GPIO_SetBits(GPIOB, GPIO_Pin_8);
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
	WT588F_GPIO_Init();
    return 0;
}
INIT_APP_EXPORT(gpio_init);

//==================== 按键引脚读取 ====================
enum KEY { KEY1, KEY2 };
uint8_t read_button_GPIO(uint8_t button_id)
{
    switch (button_id) {
        case KEY1: return K_1;  // 主按键
        case KEY2: return K_2;  // 一键启动
        default:   return 1;    // 未按下
    }
}

//==================== 状态切换日志 ====================
static void log_state_change(void)
{
    if (state == last_state) return;
    LOG_I("状态切换 → %s", state_name[state]);
    last_state = state;
}

//==================== 按键事件处理 ====================

// 短按：切换选项
void BTN1_SINGLE_CLICK_Handler(void *btn)
{
    LOG_I("[EVENT] KEY Short Press");
    log_state_change();

    switch (state) {
        case STANDBY:
            state = RICE_SEL;
            rice_type = 0;
//			WT588F_Play_Voice(0x00);
            LOG_I("→ Enter RICE_SEL");
            break;

        case RICE_SEL:
            rice_type = (rice_type + 1) % 3;
//			switch(rice_type)
//			{
//				case 0: WT588F_Play_Voice(0x01); break;
//				case 1:	WT588F_Play_Voice(0x02); break;
//				case 2: WT588F_Play_Voice(0x03); break;
//			}
			
            LOG_I("→ Rice: %s", rice_name[rice_type]);
            break;

        case MODE_SEL:
            cook_mode = (cook_mode + 1) % 4;
//            switch (cook_mode)
//            {
//            case 0:
//                WT588F_Play_Voice(0x05); break;
//            case 1:
//                WT588F_Play_Voice(0x07); break;
//            case 2:
//                WT588F_Play_Voice(0x09); break;
//            case 3:
//                WT588F_Play_Voice(0x0B); break;
//            default:
//                break;
//            }
            LOG_I("→ Mode: %s", mode_name[cook_mode]);
            break;

        case BOOK_SET:
            book_hour = (book_hour + 1) % 13;
            LOG_I("→ Book Hour: %d", book_hour);
            break;

        default:
            break;
    }
}

// 双击：仅在 BOOK_SET 有效
void BTN1_DOUBLE_CLICK_Handler(void *btn)
{
    LOG_I("[EVENT] KEY Double Click");
    log_state_change();

    if (state == BOOK_SET) {
        if (book_hour == 0) {
            LOG_I("[WARN] Book 0h, Start Cooking");
            start_cooking();
            state = COOKING;
        } else {
            book_active = RT_TRUE;
            book_start_tick = rt_tick_get();
            state = BOOK_WAIT;
            LOG_I("→ Booking Confirmed: %d小时", book_hour);
        }
    }
    // 其他状态双击无效，防止误操作
}

// 长按：确认或取消
void BTN1_LONG_PRESS_START_Handler(void *btn)
{
    LOG_I("[EVENT] KEY Long Press");
    log_state_change();

    switch (state) {
        case RICE_SEL:
            LOG_I("米种确认: %s", rice_name[rice_type]);
            state = MODE_SEL;
//            WT588F_Play_Voice(0x04);
            break;

        case MODE_SEL:
            LOG_I("模式确认: %s", mode_name[cook_mode]);
            state = BOOK_SET;
//            WT588F_Play_Voice(0x12);
            book_hour = 0;
            break;

        case COOKING:
        case WARMING:
        case BOOK_WAIT:
            LOG_I("用户取消 → STANDBY");
//            WT588F_Play_Voice(0x10);
            state = STANDBY;
            cook_active = RT_FALSE;
            book_active = RT_FALSE;
//           set_ih_power_w(0);
            break;

        default:
            state = STANDBY;
            LOG_I("state-->STANDBY");
            break;
    }
}

// BTN2：一键热饭（普通米 + 热饭 + 立即）
void BTN2_SINGLE_CLICK_Handler(void *btn)
{
    
}

// 可选：BTN2 长按关机
void BTN2_LONG_PRESS_START_Handler(void *btn)
{
    LOG_I("BTN2 长按 → 关机");
    gpio_toggle(GPIOB, GPIO_Pin_8);
}

//==================== 按键任务入口 ====================
void thread_key_task_entry(void *parameter)
{
    // 初始化按键引脚（上拉输入）
    gpio_config(GPIOC, GPIO_Pin_11, GPIO_Mode_IPU, GPIO_Speed_50MHz);  // KEY1
    gpio_config(GPIOC, GPIO_Pin_13, GPIO_Mode_IPU, GPIO_Speed_50MHz);  // KEY2

    // 初始化按键
    button_init(&btn1, read_button_GPIO, 0, KEY1);
    button_attach(&btn1, SINGLE_CLICK,      BTN1_SINGLE_CLICK_Handler);
    button_attach(&btn1, DOUBLE_CLICK,      BTN1_DOUBLE_CLICK_Handler);
    button_attach(&btn1, LONG_PRESS_START,  BTN1_LONG_PRESS_START_Handler);
    button_start(&btn1);

    button_init(&btn2, read_button_GPIO, 0, KEY2);
    button_attach(&btn2, SINGLE_CLICK,      BTN2_SINGLE_CLICK_Handler);
    button_attach(&btn2, LONG_PRESS_START,  BTN2_LONG_PRESS_START_Handler);
    button_start(&btn2);

    LOG_I("=== 按键任务启动 | 短按=切换 | 长按=确认 | 双击=启动 ===");

    while (1) {
        button_ticks();           // 按键状态机刷新
        rt_thread_mdelay(5);      // 5ms 轮询
    }
}

//==================== 导出命令（可选）====================
void cmd_key_status(int argc, char **argv)
{
    rt_kprintf("\n=== 按键状态 ===\n");
    rt_kprintf("当前状态: %s\n", state_name[state]);
    rt_kprintf("米种: %s\n", rice_name[rice_type]);
    rt_kprintf("模式: %s\n", mode_name[cook_mode]);
    rt_kprintf("预约: %d小时\n", book_hour);
    rt_kprintf("================\n");
}
MSH_CMD_EXPORT(cmd_key_status, Show key status);
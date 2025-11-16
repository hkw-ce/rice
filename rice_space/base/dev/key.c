#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "key.h"


#define LOG_TAG     "key"
#define LOG_LVL     ELOG_LVL_VERBOSE

struct Button btn1;
struct Button btn2;
struct Button btn3;
struct Button btn4;
struct Button btn5;

const char *rice_name[] = {"普通米", "糯米", "东北米"};
const char *mode_name[] = {"精煮", "快煮", "稀饭", "热饭"};
const char *state_name[] = {
    "STANDBY",
    "RICE_SEL",
    "MODE_SEL",
    "BOOK_SET",
    "COOKING",
    "WARMING",
    "BOOK_WAIT"
};

int gpio_init(void)
{
	// gpio_config(GPIOC, GPIO_Pin_13, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	// gpio_config(GPIOC, GPIO_Pin_14, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	// gpio_config(GPIOC, GPIO_Pin_8, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	// gpio_config(GPIOC, GPIO_Pin_15, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);  //enable pin for 12V power
	// GPIO_SetBits(GPIOC, GPIO_Pin_15); //打开12V电源
	// GPIO_ResetBits(GPIOC, GPIO_Pin_13); //打开3.3V电源
	// GPIO_SetBits(GPIOC, GPIO_Pin_14); //打开5V电源
     gpio_config(GPIOC, GPIO_Pin_15, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);  //系统运行指示灯
	 gpio_config(GPIOA, GPIO_Pin_1, GPIO_Mode_Out_PP, GPIO_Speed_50MHz); 
	gpio_config(GPIOB, GPIO_Pin_8, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	GPIO_SetBits(GPIOB, GPIO_Pin_8);
	GPIO_SetBits(GPIOA, GPIO_Pin_1);
	return 0;
}                                            
INIT_APP_EXPORT(gpio_init);

enum KEY
{
	KEY1,
	KEY2,
	KEY3,
	KEY4,
	KEY5
};
uint8_t read_button_GPIO(uint8_t button_id)
{
	// you can share the GPIO read function with multiple Buttons
	switch (button_id)
	{
	case KEY1:
		return K_1;
	case KEY2:
        return K_2;
    default:
        return 1; // 默认返回未按下状态    
    }
}
sys_state_t state = STANDBY;
sys_state_t last_state = (sys_state_t)-1;
extern void gn1650_demo(uint16_t freq);
extern RCC_ClocksTypeDef      RCC_Clocks;
extern void start_cooking(void);

#define KEY_NUM 1
// ==================== 按键事件处理函数 ====================
#if KEY_NUM == 2
    void BTN1_SINGLE_CLICK_Handler(void *btn)
    {
    LOG_I("BTN1_SINGLE_CLICK_Handler");
        switch (state)
        {
        case STANDBY:     LOG_I("→ Enter RICE_SEL"); state = RICE_SEL; rice_type = 0; break;
        case RICE_SEL:    LOG_I("→ Rice: %s", rice_name[rice_type]); rice_type = (rice_type + 1) % 3; break;
        case MODE_SEL:    LOG_I("→ Mode: %s", mode_name[cook_mode]); cook_mode = (cook_mode + 1) % 4; break;
        case BOOK_SET:    LOG_I("→ Book Hour: %d", book_hour); book_hour = (book_hour + 1) % 13; break;
        }
    }

    void BTN1_DOUBLE_CLICK_Handler(void *btn)
    {
        LOG_I("BTN1_DOUBLE_CLICK_Handler");
        if (state == RICE_SEL) { LOG_I("→ Jump to MODE_SEL"); state = MODE_SEL; }
        else if (state == MODE_SEL) { LOG_I("→ Jump to BOOK_SET"); state = BOOK_SET; }
    }
    void BTN1_LONG_PRESS_START_Handler(void *btn)
    {
        LOG_I("BTN1_LONG_PRESS_START_Handler → Force Back to STANDBY");
        state = STANDBY;
    //    heater_off();
    //    seg_display("----");
    //    ws2812_set_off();
    }
    extern void start_cooking(void);
    void BTN2_SINGLE_CLICK_Handler(void *btn)
    {
        LOG_I("BTN2_SINGLE_CLICK_Handler");
        switch (state)
        {
        case RICE_SEL: LOG_I("→ Enter MODE_SEL"); state = MODE_SEL; break;
        case MODE_SEL:
            if (cook_mode == 3) { LOG_I("→ Enter BOOK_SET"); state = BOOK_SET; }
            else { LOG_I("→ Start Cooking: %s", mode_name[cook_mode]); start_cooking(); state = COOKING; }
            break;
        case BOOK_SET:
            LOG_I("→ Booking Confirmed: %d小时", book_hour);
            book_timer = book_hour * 3600;
            state = BOOK_WAIT;
            break;
        }
    }

    void BTN2_LONG_PRESS_START_Handler(void *btn)
    {
        LOG_I("BTN2_LONG_PRESS_START_Handler → Cancel & Back to STANDBY");
        state = STANDBY;
    //    heater_off();
    //    seg_display("----");
    //    ws2812_set_color(0x0000FF);
    }
 #elif KEY_NUM == 1
    // ==================== 多按键任务入口 ====================
    void BTN1_SINGLE_CLICK_Handler(void *btn)
    {
        LOG_I("[EVENT] KEY Short Press\r\n");
        switch (state)
        {
        case STANDBY: state = RICE_SEL; rice_type = 0; LOG_I("→ Enter RICE_SEL\r\n"); break;
        case RICE_SEL: rice_type = (rice_type + 1) % 3; LOG_I("→ Rice: %s\r\n", rice_name[rice_type]); break;
        case MODE_SEL: cook_mode = (cook_mode + 1) % 4; LOG_I("→ Mode: %s\r\n", mode_name[cook_mode]); break;
        case BOOK_SET: book_hour = (book_hour + 1) % 13; LOG_I("→ Book Hour: %d\r\n", book_hour); break;
        }
    }

    void BTN1_DOUBLE_CLICK_Handler(void *btn)
    {
        LOG_I("[EVENT] KEY Double Click\r\n");
        switch (state)
        {
        case RICE_SEL: state = MODE_SEL; LOG_I("→ Jump to MODE_SEL\r\n"); break;
        case MODE_SEL:
            if (cook_mode == 3) state = BOOK_SET;  // 热饭进入预约
            else { start_cooking(); state = COOKING; LOG_I("→ Start COOKING\r\n"); }
            break;
        case BOOK_SET:
            if (book_hour == 0) {
                LOG_I("[WARN] Book 0h, Start Cooking\r\n");
                start_cooking();
                state = COOKING;
            } else {
                book_active = RT_TRUE;
                book_start_tick = rt_tick_get();
                state = BOOK_WAIT;
                LOG_I("→ Booking Confirmed: %d小时\r\n", book_hour);
            }
            break;
        }
    }

    void BTN1_LONG_PRESS_START_Handler(void *btn)
    {
        LOG_I("[EVENT] KEY Long Press\r\n");
		state = STANDBY;
		LOG_I("state-->STANDBY\r\n");
    }

    void BTN2_SINGLE_CLICK_Handler(void *btn)
    {
        LOG_I("BTN2_SINGLE_CLICK_Handler");

    }

    void BTN2_LONG_PRESS_START_Handler(void *btn)
    {
        LOG_I("BTN2_LONG_PRESS_START_Handler → Cancel & Back to STANDBY");
		gpio_toggle(GPIOB, GPIO_Pin_8);
    }

#endif

void thread_key_task_entry(void *parameter)
{

    // 初始化多按键模块
    gpio_config(GPIOC, GPIO_Pin_11, GPIO_Mode_IPU, GPIO_Speed_50MHz);
    gpio_config(GPIOC, GPIO_Pin_13, GPIO_Mode_IPU, GPIO_Speed_50MHz);
    button_init(&btn1, read_button_GPIO, 0, KEY1);    //控制按键
	button_attach(&btn1, LONG_PRESS_START, BTN1_LONG_PRESS_START_Handler);
	button_attach(&btn1, DOUBLE_CLICK, BTN1_DOUBLE_CLICK_Handler);
	button_attach(&btn1, SINGLE_CLICK, BTN1_SINGLE_CLICK_Handler);
	button_start(&btn1);

    button_init(&btn2, read_button_GPIO, 0, KEY2); //一键启动
//	button_attach(&btn2, DOUBLE_CLICK, BTN2_DOUBLE_CLICK_Handler);
	button_attach(&btn2, SINGLE_CLICK, BTN2_SINGLE_CLICK_Handler);
	button_attach(&btn2, LONG_PRESS_START, BTN2_LONG_PRESS_START_Handler);
	button_start(&btn2);
    LOG_I("Key task started.\r\n");
    while (1)
    {
	button_ticks();

	rt_thread_mdelay(5);
    }
}

// // 状态切换日志
// void log_state_enter(void)
// {
//     if (state == last_state) return;
//     const char *name[] = {"STANDBY", "RICE_SEL", "MODE_SEL", "BOOK_SET",
//                           "COOKING", "WARMING", "BOOK_WAIT"};
//     LOG_I("State → %s\r\n", name[state]);
//     last_state = state;
// }

// // 状态机核心（带详细日志）
// void state_machine(void)
// {
// rt_tick_t now = rt_tick_get();  // 当前 tick 数
// 	log_state_enter();
//     log_rice_change();
//     log_mode_change();
//     log_book_change();
//     switch (state)
//     {
//     case STANDBY:
// //        seg_display("----");
// //        ws2812_set_color(0x0000FF);
// //        LOG_I("Standby Mode: Waiting for KEY1");
//         break;

//     case RICE_SEL:
// //        seg_display_rice(rice_type);
// //        ws2812_flash(0x0000FF, 500);
// //        LOG_I("Rice Select: %s (r%d)", rice_name[rice_type], rice_type + 1);
//         break;

//     case MODE_SEL:
// //        seg_display_mode(cook_mode);
// //        ws2812_flash(0x0000FF, 500);
// //        LOG_I("Mode Select: %s (P%d)", mode_name[cook_mode], cook_mode + 1);
//         break;

//     case BOOK_SET:
// //        seg_display_book(book_hour);
// //        ws2812_flash(0x0000FF, 1000);
	
// //        LOG_I("Booking Set: %02d小时 (t%02d)", book_hour, book_hour);
//         break;

//     case BOOK_WAIT:
// 	{
//     if (!book_active)
//     {
//         book_start_tick = rt_tick_get();
//         book_active = RT_TRUE;
//         LOG_I("Booking Start: %d小时\r\n", book_hour);
//     }
//     now =rt_tick_get(); 
//         rt_uint32_t elapsed = ( now - book_start_tick  ) / RT_TICK_PER_SECOND;
//         rt_uint32_t remain = (book_hour * 3600 > elapsed) ? (book_hour * 3600 - elapsed) : 0;

    

//         // 每分钟打印一次（60秒）
//         if (remain % 60 == 0 && remain != last_book_remain && remain > 0)
//         {
//             LOG_I("Booking Wait: %02d:%02d 剩余\r\n", remain/3600, (remain%3600)/60);
//             last_book_remain = remain;
//         }

//         if (remain == 0)
//         {
//             LOG_I("Booking Done → Start Cooking\r\n");
//             start_cooking();
//             state = COOKING;
//             book_active = RT_FALSE;
//             last_book_remain = 0xFFFFFFFF;  // 重置
//         }
//     }
//     break;

//     case COOKING:
// //        seg_display_countdown(cook_timer);
// //        ws2812_set_color(0xFF0000);
// 	{
//    if (!cook_active) break;
// now =rt_tick_get(); 
//     rt_uint32_t elapsed = (now - cook_start_tick) / RT_TICK_PER_SECOND;
//     rt_uint32_t remain = (cook_total_sec > elapsed) ? (cook_total_sec - elapsed) : 0;

//     // 每分钟打印一次
//     if (remain % 60 == 0 && remain != last_cook_remain && remain > 0)
//     {
//         LOG_I("Cooking: %02d:%02d 剩余\r\n", remain/60, remain%60);
//         last_cook_remain = remain;
//     }

//     if (remain == 0)
//     {
//         LOG_I("Cooking Done → WARMING\r\n");
//         state = WARMING;
//         cook_active = RT_FALSE;
//         last_cook_remain = 0xFFFFFFFF;

//     }
// }
//     break;
//     case WARMING:
// //        seg_display("Hot");
// //        ws2812_set_color(0xFFFF00);
//           LOG_I("Warming Mode: 保温中 (Hot)");
//         break;
//     }
// }

// // ==================== 任务入口 ====================
// void thread_logic_task_entry(void *parameter)
// {
//     last_state = STANDBY;
//     LOG_I("Logic Task Started. Initial State: STANDBY");

//     while (1)
//     {
//         state_machine();
//         rt_thread_mdelay(10);  // 10ms 刷新
//     }
// }





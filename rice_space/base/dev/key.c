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

int gpio_init(void)
{
	gpio_config(GPIOC, GPIO_Pin_13, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	gpio_config(GPIOC, GPIO_Pin_14, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	gpio_config(GPIOC, GPIO_Pin_8, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	gpio_config(GPIOC, GPIO_Pin_15, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);  //enable pin for 12V power
	GPIO_SetBits(GPIOC, GPIO_Pin_15); //打开12V电源
	GPIO_ResetBits(GPIOC, GPIO_Pin_13); //打开3.3V电源
	GPIO_SetBits(GPIOC, GPIO_Pin_14); //打开5V电源
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
		return (K_1==0 && K_2 == 0 )?0:1;
	case KEY2:
        return (K_1==0 && K_2 == 1 )?0:1;
    default:
        return 1; // 默认返回未按下状态    
    }
}

/**
 * @brief  初始化按键
 * @note   引脚映射：PA11（OUT1）、P12（OUT2），（触摸按键编码输出端口）
 *          * 触摸按键与 OUT1/OUT2 输出电平对应关系表
 *          +------------+-------+-------+o·
 *          | 触摸按键   | OUT2  | OUT1  |
 *          +------------+-------+-------+
 *          | KEY1       | o     | 0     |
 *          | KEY2       | 0     | 1     |
 *          | 无按键      | 1     | 1     |
 *          +------------+-------+-------+
 */
// int Key_Init(void)
// {
//  	GPIO_InitTypeDef GPIO_InitStructure;

//     GPIO_StructInit(&GPIO_InitStructure);
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOC, &GPIO_InitStructure);
// 	return 0;
// }
// INIT_APP_EXPORT(Key_Init);
uint16_t freq = 40000;
bool pwm_started = false;
extern void gn1650_demo(uint16_t freq);
extern RCC_ClocksTypeDef      RCC_Clocks;
void BTN1_LONG_PRESS_START_Handler(void *btn)
{
	    LOG_I("BTN1_LONG_PRESS_START_Handler\r\n");
	  uint16_t arr,psc;
   	 float duty_cycle1,duty_cycle2;
	 if (pwm_started == false)
	 {
		gpio_config(GPIOA, GPIO_Pin_4, GPIO_Mode_Out_PP, GPIO_Speed_50MHz); // PWM使能
	 GPIO_SetBits(GPIOA, GPIO_Pin_4); // 使能PWM输出
	RCC_GetClocksFreq(&RCC_Clocks);
    LOG_I("SystemCoreClock: %lu", RCC_Clocks.SYSCLK_Frequency);
    LOG_I("HCLK_Frequency: %lu", RCC_Clocks.HCLK_Frequency);
    LOG_I("PCLK1_Frequency: %lu", RCC_Clocks.PCLK1_Frequency);
    LOG_I("PCLK2_Frequency: %lu", RCC_Clocks.PCLK2_Frequency);
    LOG_I("ADCCLK_Frequency: %lu", RCC_Clocks.ADCCLK_Frequency);
    arr =RCC_Clocks.PCLK1_Frequency*2/ (freq*2); // 计算ARR
    psc = 1; // 预分频器设为1
    LOG_I("ARR: %u,PSC: %u", arr,psc);
    PSFB_TIM->ARR = arr - 1;
    LOG_I("PWM Started at %d Hz", freq);
	gn1650_demo(freq/100);
	 duty_cycle1 = 0.5;
    duty_cycle2 = 0.51;
	LOG_I("ARR: %u", PSFB_TIM->ARR);
	PSFB_TIM->CCR1 = PSFB_TIM->ARR*duty_cycle1;
    LOG_I("PWM1 CCR1: %d", PSFB_TIM->CCR1);
    PSFB_TIM->CCR2 = PSFB_TIM->ARR*duty_cycle2;
    LOG_I("PWM2 CCR2: %d", PSFB_TIM->CCR2);
	pwm_started=true;
	LOG_I("PWM Started");
	 }
	 
     

	// rt_event_send(&env.btn_event, EVENT_START_FLAG);
}
void BTN1_DOUBLE_CLICK_Handler(void *btn)
{
	// do something...
    LOG_I("BTN1_DOUBLE_CLICK_Handler\r\n");

	// rt_event_send(&env.btn_event, EVENT_STOP_FLAG);
}
void BTN1_SINGLE_CLICK_Handler(void *btn)
{
	// do something...
	LOG_I("BTN1_SINGLE_CLICK_Handler\r\n");
	if (pwm_started)
	{
		freq=freq-500;
	if(freq<25000)
	{
		freq=25000;
	}
	LOG_I(" PWM Frequency: %u Hz", freq);
	gn1650_demo(freq/100);
	uint16_t arr,psc;
	float duty_cycle1,duty_cycle2;
	arr =RCC_Clocks.PCLK1_Frequency*2/ (freq*2); // 计算ARR
	psc = 1; // 预分频器设为1
	LOG_I("ARR: %u,PSC: %u", arr,psc);
	PSFB_TIM->ARR = arr - 1;
	LOG_I("PWM Frequency Changed to %u Hz", freq);
	duty_cycle1 = 0.5;
    duty_cycle2 = 0.51;
	LOG_I("ARR: %u", PSFB_TIM->ARR);
	PSFB_TIM->CCR1 = PSFB_TIM->ARR*duty_cycle1;
    LOG_I("PWM1 CCR1: %d", PSFB_TIM->CCR1);
    PSFB_TIM->CCR2 = PSFB_TIM->ARR*duty_cycle2;
    LOG_I("PWM2 CCR2: %d", PSFB_TIM->CCR2);
	}
	
	
	// rt_event_send(&env.btn_event, BTN1_SINGLE_CLICK_FLAG);
}

void BTN2_SINGLE_CLICK_Handler(void *btn)
{
	// do something...
    LOG_I("BTN2_SINGLE_CLICK_Handler\r\n");
	if (pwm_started)
	{
		freq=freq+500;
	
	if(freq>40000)
	{
		freq=40000;
	}
	LOG_I(" PWM Frequency: %u Hz", freq);
	gn1650_demo(freq/100);
	uint16_t arr,psc;
	float duty_cycle1,duty_cycle2;
   
	arr =RCC_Clocks.PCLK1_Frequency*2/ (freq*2); // 计算ARR
	psc = 1; // 预分频器设为1
	LOG_I("ARR: %u,PSC: %u", arr,psc);
	PSFB_TIM->ARR = arr - 1;
	LOG_I("PWM Frequency Changed to %u Hz", freq);
	duty_cycle1 = 0.5;
    duty_cycle2 = 0.51;
	LOG_I("ARR: %u", PSFB_TIM->ARR);
	PSFB_TIM->CCR1 = PSFB_TIM->ARR*duty_cycle1;
    LOG_I("PWM1 CCR1: %d", PSFB_TIM->CCR1);
    PSFB_TIM->CCR2 = PSFB_TIM->ARR*duty_cycle2;
    LOG_I("PWM2 CCR2: %d", PSFB_TIM->CCR2);

	}
	
	
	// rt_event_send(&env.btn_event, BTN2_SINGLE_CLICK_FLAG2);
}

void BTN2_LONG_PRESS_START_Handler(void *btn)
{
    LOG_I("BTN2_LONG_PRESS_START_Handler\r\n");
	// do something...
}

void BTN2_DOUBLE_CLICK_Handler(void *btn)
{
	
	LOG_I("BTN2_DOUBLE_CLICK_Handler\r\n"); 
	if(pwm_started==true)
	{
		pwm_started=false;
		// do something...
 
	float duty_cycle1,duty_cycle2;	
    duty_cycle1 = 0;
    duty_cycle2 = 0;
	LOG_I("ARR: %u", PSFB_TIM->ARR);
	PSFB_TIM->CCR1 = PSFB_TIM->ARR*duty_cycle1;
    LOG_I("PWM1 CCR1: %d", PSFB_TIM->CCR1);
    PSFB_TIM->CCR2 = PSFB_TIM->ARR*duty_cycle2;
    LOG_I("PWM2 CCR2: %d", PSFB_TIM->CCR2);
	freq=40000;
	gn1650_demo(000);
    LOG_I("PWM Stopped");
	}

	
}

void thread_key_task_entry(void *parameter)
{

    // 初始化多按键模块
    gpio_config(GPIOC, GPIO_Pin_6, GPIO_Mode_IPU, GPIO_Speed_50MHz);
    gpio_config(GPIOC, GPIO_Pin_7, GPIO_Mode_IPU, GPIO_Speed_50MHz);
    button_init(&btn1, read_button_GPIO, 0, KEY1);
	button_attach(&btn1, LONG_PRESS_START, BTN1_LONG_PRESS_START_Handler);
	button_attach(&btn1, DOUBLE_CLICK, BTN1_DOUBLE_CLICK_Handler);
	button_attach(&btn1, SINGLE_CLICK, BTN1_SINGLE_CLICK_Handler);
	button_start(&btn1);

    button_init(&btn2, read_button_GPIO, 0, KEY2);
	button_attach(&btn2, DOUBLE_CLICK, BTN2_DOUBLE_CLICK_Handler);
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

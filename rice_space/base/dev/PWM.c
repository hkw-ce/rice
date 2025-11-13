#include "platform.h"
#include "stdlib.h"
#define LOG_TAG    "PWM"
#define LOG_LVL     ELOG_LVL_VERBOSE


/**
 *  @brief  初始化TIM2
 *  @param  arr: 自动重装载寄存器值
 *  @param  psc: 时钟预分频值
 *  @note   移相全桥
 */
void full_bridge_init(uint16_t arr,uint16_t psc)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM1, ENABLE);
    GPIO_PinAFConfig(PSFB_GPIO, PSFB_PWM1_PIN_SOURCE, PSFB_GPIO_AF);   //TIM2_CH1
    GPIO_PinAFConfig(PSFB_GPIO, PSFB_PWM2_PIN_SOURCE, PSFB_GPIO_AF);   //TIM2_CH2

    GPIO_InitStructure.GPIO_Pin = PSFB_PWM1 | PSFB_PWM2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PSFB_GPIO, &GPIO_InitStructure);

    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = arr - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = psc - 1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(PSFB_TIM, &TIM_TimeBaseStructure);

    TIM_ARRPreloadConfig(PSFB_TIM, ENABLE);

    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OC1Init(PSFB_TIM, &TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OC2Init(PSFB_TIM, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(PSFB_TIM, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(PSFB_TIM, TIM_OCPreload_Enable);

    TIM_PWMShiftConfig(PSFB_TIM, TIM_PDER_CCR1SHIFTEN, ENABLE);
    TIM_PWMShiftConfig(PSFB_TIM, TIM_PDER_CCR2SHIFTEN, ENABLE);

    TIM_Cmd(PSFB_TIM, ENABLE);
	TIM_CtrlPWMOutputs(PSFB_TIM, ENABLE);
    TIM_CtrlPWMOutputs(PSFB_TIM, ENABLE);
}
int full_bridge_init_app(void)
{
    full_bridge_init(2000, 1); // 默认1kHz
    return 0;
}
INIT_APP_EXPORT(full_bridge_init_app);
 RCC_ClocksTypeDef      RCC_Clocks;
void pwm_frequency_test(int argc, char **argv[])
{
   uint16_t arr,psc;
   float duty_cycle1,duty_cycle2;
    if (argc < 2) {
        // 无参数：
        rt_kprintf("输入ARR与PSC\n");
    } else if (argc > 2) {
        // 参数过多：错误提示
        rt_kprintf("Error: Only two parameter allowed.\n");
    } else
    {
    gpio_config(GPIOA, GPIO_Pin_4, GPIO_Mode_Out_PP, GPIO_Speed_50MHz); // PWM使能
    if(atoi(argv[1])==0)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4); // 使能PWM输出
        LOG_I("PWM Stopped");
 
    }
    else
    {
    GPIO_SetBits(GPIOA, GPIO_Pin_4); // 使能PWM输出
	RCC_GetClocksFreq(&RCC_Clocks);
    LOG_I("SystemCoreClock: %lu", RCC_Clocks.SYSCLK_Frequency);
    LOG_I("HCLK_Frequency: %lu", RCC_Clocks.HCLK_Frequency);
    LOG_I("PCLK1_Frequency: %lu", RCC_Clocks.PCLK1_Frequency);
    LOG_I("PCLK2_Frequency: %lu", RCC_Clocks.PCLK2_Frequency);
    LOG_I("ADCCLK_Frequency: %lu", RCC_Clocks.ADCCLK_Frequency);
    arr =RCC_Clocks.PCLK1_Frequency*2/ (atoi(argv[1])*2); // 计算ARR
    psc = 1; // 预分频器设为1
    LOG_I("ARR: %u,PSC: %u", arr,psc);
    PSFB_TIM->ARR = arr - 1;
    LOG_I("PWM Started at %s Hz", argv[1]);

    }
    }

}
//MSH_CMD_EXPORT(pwm_frequency_test,pwm_frequency_test);


void set_pwm_duty(int argc, char **argv[])
{
    float duty_cycle1,duty_cycle2;
    if (argc < 3) {
        // 无参数：
        rt_kprintf("输入占空比1与占空比2(0~1)\n");
    } else if (argc > 3) {
        // 参数过多：错误提示
        rt_kprintf("Error: Only two parameter allowed.\n");
    } else
    {
       
	
    duty_cycle1 = atof(argv[1]);
    duty_cycle2 = atof(argv[2]);
	LOG_I("ARR: %u", PSFB_TIM->ARR);
		  PSFB_TIM->CCR1 = PSFB_TIM->ARR*duty_cycle1;
          LOG_I("PWM1 CCR1: %d", PSFB_TIM->CCR1);
          PSFB_TIM->CCR2 = PSFB_TIM->ARR*duty_cycle2;
          LOG_I("PWM2 CCR2: %d", PSFB_TIM->CCR2);
    }
}
//MSH_CMD_EXPORT(set_pwm_duty,set_pwm_duty);
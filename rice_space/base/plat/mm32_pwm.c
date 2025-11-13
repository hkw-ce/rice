/**
 * @file mm32_pwm.c
 * @author zhang
 * @brief
 * @version 0.1
 * @date 2025-09-18
 *
 * @copyright Copyright (c) 2025
 * */

#include "platform.h"   
#include <string.h>

/*
    * @brief TIM1 PWM初始化，互补输出，死区时间1个时钟
    * @param 无
*/
/**
 * @brief  配置 TIM1 单通道互补 PWM 输出
 * @param  channel  TIM1 通道号（1~3）
 * @param  period   PWM 周期计数值（ARR）
 * @param  deadtime 死区时间（单位：定时器时钟周期）
 */
void TIM1_PWM_Complementary_SingleChannel_Config(uint8_t channel, uint16_t period, uint16_t deadtime)
{
    GPIO_InitTypeDef        GPIO_InitStruct;
    TIM_OCInitTypeDef       TIM_OCInitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_BDTRInitTypeDef     TIM_BDTRInitStruct;
    /* --- 打开时钟 --- */
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOB, ENABLE);

    /* --- 基本定时器配置 --- */
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
    TIM_TimeBaseStructure.TIM_Period = period - 1;
    TIM_TimeBaseStructure.TIM_Prescaler =0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    /* --- PWM 输出通道配置 --- */
    TIM_OCStructInit(&TIM_OCInitStruct);
    TIM_OCInitStruct.TIM_OCMode       = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState  = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStruct.TIM_Pulse        = 0;
    TIM_OCInitStruct.TIM_OCPolarity   = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_OCNPolarity  = TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_OCIdleState  = TIM_OCIdleState_Reset;
    TIM_OCInitStruct.TIM_OCNIdleState = TIM_OCIdleState_Reset;

    /* --- 按通道号配置 GPIO 和通道 --- */
    switch (channel)
    {
    case 1:
        /* CH1: PA8 / CH1N: PB13 */
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_2);
        GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_8;
        GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_2);
        GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_13;
        GPIO_Init(GPIOB, &GPIO_InitStruct);

        TIM_OC1Init(TIM1, &TIM_OCInitStruct);
        TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
        break;

    case 2:
        /* CH2: PA9 / CH2N: PB14 */
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_2);
        GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_9;
        GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_2);
        GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_14;
        GPIO_Init(GPIOB, &GPIO_InitStruct);

        TIM_OC2Init(TIM1, &TIM_OCInitStruct);
        TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
        break;

    case 3:
        /* CH3: PA10 / CH3N: PB15 */
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_2);
        GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_10;
        GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_AF_PP;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_2);
        GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_15;
        GPIO_Init(GPIOB, &GPIO_InitStruct);

        TIM_OC3Init(TIM1, &TIM_OCInitStruct);
        TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
        break;

    default:
        return; // 非法通道号
    }

//    /* --- 刹车与死区配置 --- */
//    TIM_BDTRStructInit(&TIM_BDTRInitStruct);
//    TIM_BDTRInitStruct.TIM_OSSRState       = TIM_OSSRState_Enable;
//    TIM_BDTRInitStruct.TIM_OSSIState       = TIM_OSSIState_Enable;
//    TIM_BDTRInitStruct.TIM_LOCKLevel       = TIM_LOCKLevel_OFF;
//    TIM_BDTRInitStruct.TIM_DeadTime        = deadtime;
//    TIM_BDTRInitStruct.TIM_Break           = TIM_Break_Disable;
//    TIM_BDTRInitStruct.TIM_BreakPolarity   = TIM_BreakPolarity_High;
//    TIM_BDTRInitStruct.TIM_AutomaticOutput = TIM_AutomaticOutput_Enable;
//    TIM_BDTRConfig(TIM1, &TIM_BDTRInitStruct);
		TIM1->BDTR |= deadtime;   

    /* --- 启动定时器与输出 --- */
    TIM_Cmd(TIM1, ENABLE);
//    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}


void comp_pwm_cmd(int argc, char** argv)
{
    if (argc < 2) {
        LOG_I("Usage: comp_pwm_cmd <on|off>\n");
        return;
    }

    if (strcmp(argv[1], "on") == 0) {
        TIM1_PWM_Complementary_SingleChannel_Config(1, 2000, 10); // 配置 TIM1 通道1，周期1000，死区时间10
        TIM1->CCR1 = 1000; // 设置占空比为50%
        TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
        TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
        TIM_CtrlPWMOutputs(TIM1, ENABLE);
        LOG_I("Complementary PWM output enabled.\n");
    } else if (strcmp(argv[1], "off") == 0) {
        TIM_CtrlPWMOutputs(TIM1, DISABLE);
        TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Disable);
        TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Disable);
        LOG_I("Complementary PWM output disabled.\r\n");
    } else {
        LOG_I("Invalid argument. Use 'on' or 'off'.\r\n");
    }
}

MSH_CMD_EXPORT(comp_pwm_cmd, Enable or disable complementary PWM output on TIM1 channel 1);

/**
 * @brief shell 命令：设置 TIM1 互补 PWM 的频率和占空比/死区时间
 * @usage pwm_set_freq <freq_hz> [deadtime_ns]
 * 示例： pwm_set_freq 20000 100  -> 20kHz, 100ns 死区时间
 */
void pwm_set_freq_cmd(int argc, char **argv)
{
    if (argc < 2) {
        LOG_I("Usage: pwm_set_freq <freq_hz> [deadtime_ns]\n");
        return;
    }

    uint32_t freq = (uint32_t)atoi(argv[1]);
    uint32_t deadtime_ns = 104;  // 默认 10 个时钟周期（96MHz -> 104.2ns）
    if (argc >= 3) deadtime_ns = (uint32_t)atoi(argv[2]);
    if (freq == 0) {
        LOG_I("Invalid arguments. freq>0\n");
        return;
    }

    /* 获取 PCLK2 频率并计算定时器时钟（若 APB2 有分频则定时器时钟为 PCLK2*2） */
    RCC_ClocksTypeDef RCC_Clocks;
    RCC_GetClocksFreq(&RCC_Clocks);
    uint32_t pclk2 = RCC_Clocks.PCLK2_Frequency;
    uint32_t tim_clk = pclk2;
    /* 当 APB2 有分频时，定时器时钟通常为 PCLK2 * 2（大多数 STM32 系列规律）。
     * 这里通过检测 RCC->CFGR 中 PPRE2 字段是否为 0 来简单判断是否有分频，
     * 如果非 0 则认为定时器时钟 = PCLK2 * 2。
     */
#if defined(RCC) && defined(RCC_CFGR_PPRE2)
    if ((RCC->CFGR & RCC_CFGR_PPRE2) != 0) {
        tim_clk = pclk2 * 2;
    }
#endif

    /* 计算 ARR (period)。
       中心对齐模式下，计数器上升到 ARR，再下降到 0，因此 PWM 周期 = 2*ARR*T_clk
       所以：频率 = f_clk / (2*ARR)，即 ARR = f_clk / (2*freq)
       注意 TIM1 在此实现中使用 16-bit ARR 上限 65535 */
    uint32_t period = tim_clk / ( freq);  /* 中心对齐模式要除以 2 */
    if (period < 1) period = 1; /* 最小周期限制 */
    if (period > 0xFFFF) {
        LOG_I("Requested frequency too low for 16-bit ARR (period=%lu). Reduce desired frequency or modify driver.\n", period);
        return;
    }

    uint16_t period16 = (uint16_t)period;
    uint16_t ccr = period16 / 2;  /* 默认 50% 占空比（中心对齐模式） */

    /* 将死区时间（纳秒）转换为定时器时钟周期数 */
    /* 时钟周期（ns）= 1e9 / tim_clk (Hz)
       死区周期数 = deadtime_ns / (1e9 / tim_clk) = deadtime_ns * tim_clk / 1e9 */
    /* 使用 64-bit 避免 deadtime_ns * tim_clk 溢出 (deadtime_ns up to 1e9, tim_clk up to ~1e8) */
    uint64_t tmp = (uint64_t)deadtime_ns * (uint64_t)tim_clk + 500000000ULL; /* 四舍五入 */
    uint32_t deadtime_cycles = (uint32_t)(tmp / 1000000000ULL);
    if (deadtime_cycles < 1) deadtime_cycles = 1;
    if (deadtime_cycles > 255) deadtime_cycles = 255;  /* TIM1 BDTR 中 DeadTime 为 8-bit */
    
    uint16_t deadtime16 = (uint16_t)deadtime_cycles;
    uint64_t actual_deadtime_ns_u64 = ((uint64_t)deadtime16 * 1000000000ULL) / (uint64_t)tim_clk;  /* 实际死区时间（纳秒） */
    uint32_t actual_deadtime_ns = (uint32_t)actual_deadtime_ns_u64;

    /* 重新配置 TIM1（通道1，使用计算的死区时间）并启动输出 */
    TIM1_PWM_Complementary_SingleChannel_Config(1, period16, deadtime16);
    TIM1->CCR1 = ccr;
    TIM_CCxCmd(TIM1, TIM_Channel_1, TIM_CCx_Enable);
    TIM_CCxNCmd(TIM1, TIM_Channel_1, TIM_CCxN_Enable);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    uint32_t actual_freq = tim_clk / (2 * period16);  /* 中心对齐模式：频率 = f_clk / (2*ARR) */
    uint64_t clk_ns_u64 = (1000000000ULL + (tim_clk/2)) / (uint64_t)tim_clk; /* 四舍五入的时钟周期(ns) */

    LOG_I("PWM set: freq=%lu Hz (actual=%lu Hz) ARR=%u CCR=%u\n",
        (unsigned long)freq, (unsigned long)actual_freq, period16, ccr);
    LOG_I("DeadTime: requested=%lu ns -> cycles=%u actual=%lu ns (tim_clk=%lu Hz, period_ns=%llu)\n",
        (unsigned long)deadtime_ns, deadtime16, (unsigned long)actual_deadtime_ns, (unsigned long)tim_clk, (unsigned long long)clk_ns_u64);
}

MSH_CMD_EXPORT(pwm_set_freq_cmd, Set TIM1 complementary PWM frequency and duty: pwm_set_freq <hz> [duty%%]);
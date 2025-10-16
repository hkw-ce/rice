#include "hal_conf.h"
#include "platform.h"

/**
 * @brief 根据 ADC 通道号返回对应 GPIO 端口和引脚
 */
int adc_channel_to_gpio(uint8_t adc_channel, GPIO_TypeDef **gpio_port, uint16_t *gpio_pin)
{
    switch (adc_channel)
    {
    case ADC_Channel_0:
        *gpio_port = GPIOA;
        *gpio_pin = GPIO_Pin_0;
        return 0;
    case ADC_Channel_1:
        *gpio_port = GPIOA;
        *gpio_pin = GPIO_Pin_1;
        return 0;
    case ADC_Channel_2:
        *gpio_port = GPIOA;
        *gpio_pin = GPIO_Pin_2;
        return 0;
    case ADC_Channel_3:
        *gpio_port = GPIOA;
        *gpio_pin = GPIO_Pin_3;
        return 0;
    case ADC_Channel_4:
        *gpio_port = GPIOA;
        *gpio_pin = GPIO_Pin_4;
        return 0;
    case ADC_Channel_5:
        *gpio_port = GPIOA;
        *gpio_pin = GPIO_Pin_5;
        return 0;
    case ADC_Channel_6:
        *gpio_port = GPIOA;
        *gpio_pin = GPIO_Pin_6;
        return 0;
    case ADC_Channel_7:
        *gpio_port = GPIOA;
        *gpio_pin = GPIO_Pin_7;
        return 0;
    case ADC_Channel_8:
        *gpio_port = GPIOB;
        *gpio_pin = GPIO_Pin_0;
        return 0;
    case ADC_Channel_9:
        *gpio_port = GPIOB;
        *gpio_pin = GPIO_Pin_1;
        return 0;
//    case ADC_Channel_10:
//        *gpio_port = GPIOB;
//        *gpio_pin = GPIO_Pin_3;
//        return 0;
//    case ADC_Channel_11:
//        *gpio_port = GPIOB;
//        *gpio_pin = GPIO_Pin_4;
//        return 0;
//    case ADC_Channel_12:
//        *gpio_port = GPIOB;
//        *gpio_pin = GPIO_Pin_5;
//        return 0;
    default:
        return -1;
    }
}

/**
 * @brief 使能对应GPIO端口时钟
 */
void enable_gpio_clock(GPIO_TypeDef *gpio_port)
{
    if (gpio_port == GPIOA)
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);
    else if (gpio_port == GPIOB)
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOB, ENABLE);
    else if (gpio_port == GPIOC)
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOC, ENABLE);
    else if (gpio_port == GPIOD)
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOD, ENABLE);
    // 需要可继续扩展
}

/**
 * @brief ADC单通道初始化函数
 * @param hadc ADC句柄指针
 * @param channel ADC通道号
 * @note ADC 的输入时钟不得超过 16M, 是由 APB2 时钟(PCLK2)分频产生。
 */
void adc_config_single(ADC_TypeDef *hadc)
{
    ADC_InitTypeDef ADC_InitStruct;
    // 使能ADC时钟 -- 假设是ADC1
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_ADC1, ENABLE);

    ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStruct.ADC_PRESCARE = ADC_PCLK2_PRESCARE_16;
    ADC_InitStruct.ADC_Mode = ADC_CR_IMM;
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStruct.ADC_ExternalTrigConv = ADC1_ExternalTrigConv_T1_CC1;
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_Init(hadc, &ADC_InitStruct);

    ADC_Cmd(hadc, ENABLE);
}

/**
 * @brief 读取单通道ADC值（轮询方式）
 * @param hadc ADC句柄指针
 * @return ADC转换结果（12位）
 */
uint16_t adc_read_single(ADC_TypeDef *hadc, ADCCHANNEL_TypeDef channel)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    // 配置单通道采样时间
    ADC_RegularChannelConfig(hadc, channel, 0, ADC_Samctl_240_5);
    // 配置采样通道数量 从0开始
    ADC_ANY_NUM_Config(hadc, 0);
    ADC_ANY_CH_Config(ADC1, 0, channel);
    ADC_ANY_Cmd(ADC1, ENABLE);

    // 配置对应的GPIO模拟输入
    GPIO_TypeDef *port = NULL;
    uint16_t pin = 0;
    if (adc_channel_to_gpio(channel, &port, &pin) == 0)
    {
        enable_gpio_clock(port);
        GPIO_StructInit(&GPIO_InitStruct);
        GPIO_InitStruct.GPIO_Pin = pin;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_Init(port, &GPIO_InitStruct);
    }
    else
    {
        // 处理错误：通道不支持
    }

    ADC_SoftwareStartConvCmd(hadc, ENABLE);

    while (RESET == ADC_GetFlagStatus(hadc, ADC_FLAG_EOC))
    {
        // 等待转换完成
    }

    ADC_ClearFlag(hadc, ADC_FLAG_EOC);

    return ADC_GetChannelConvertedValue(hadc, channel);
}

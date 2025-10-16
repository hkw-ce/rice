#include "hal_conf.h"
#include "platform.h"

/**
 * @brief ���� ADC ͨ���ŷ��ض�Ӧ GPIO �˿ں�����
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
 * @brief ʹ�ܶ�ӦGPIO�˿�ʱ��
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
    // ��Ҫ�ɼ�����չ
}

/**
 * @brief ADC��ͨ����ʼ������
 * @param hadc ADC���ָ��
 * @param channel ADCͨ����
 * @note ADC ������ʱ�Ӳ��ó��� 16M, ���� APB2 ʱ��(PCLK2)��Ƶ������
 */
void adc_config_single(ADC_TypeDef *hadc)
{
    ADC_InitTypeDef ADC_InitStruct;
    // ʹ��ADCʱ�� -- ������ADC1
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
 * @brief ��ȡ��ͨ��ADCֵ����ѯ��ʽ��
 * @param hadc ADC���ָ��
 * @return ADCת�������12λ��
 */
uint16_t adc_read_single(ADC_TypeDef *hadc, ADCCHANNEL_TypeDef channel)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    // ���õ�ͨ������ʱ��
    ADC_RegularChannelConfig(hadc, channel, 0, ADC_Samctl_240_5);
    // ���ò���ͨ������ ��0��ʼ
    ADC_ANY_NUM_Config(hadc, 0);
    ADC_ANY_CH_Config(ADC1, 0, channel);
    ADC_ANY_Cmd(ADC1, ENABLE);

    // ���ö�Ӧ��GPIOģ������
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
        // �������ͨ����֧��
    }

    ADC_SoftwareStartConvCmd(hadc, ENABLE);

    while (RESET == ADC_GetFlagStatus(hadc, ADC_FLAG_EOC))
    {
        // �ȴ�ת�����
    }

    ADC_ClearFlag(hadc, ADC_FLAG_EOC);

    return ADC_GetChannelConvertedValue(hadc, channel);
}

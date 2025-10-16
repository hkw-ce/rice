/**
 * @file mm32_gpio.c
 * @author yang
 * @brief
 * @version 0.1
 * @date 2025-09-18
 *
 * @copyright Copyright (c) 2025
 *
 * @note 1 uart���ã�2 ��תgpio��ƽ
 */

#include "hal_conf.h"
/**
 * @brief  GPIO ��ʼ������
 * @param  GPIOx: ָ��Ҫ���õ� GPIO �����ָ��
 * @param  pin: Ҫ���õ�����
 * @param  mode: GPIO ģʽ
 * @param  speed: GPIO �ٶ�
 * @retval None
 */
void gpio_config(GPIO_TypeDef *GPIOx, uint16_t pin, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // ʹ�ܶ�ӦGPIO�˿�ʱ��
    if (GPIOx == GPIOA)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);
    }
    else if (GPIOx == GPIOB)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOB, ENABLE);
    }
    else if (GPIOx == GPIOC)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOC, ENABLE);
    }
    else if (GPIOx == GPIOD)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOD, ENABLE);
    }
    // ����GPIO�ڸ���оƬ�ֲ����

    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = pin;
    GPIO_InitStruct.GPIO_Mode = mode;
    GPIO_InitStruct.GPIO_Speed = speed;

    GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void gpio_toggle(GPIO_TypeDef *GPIOx, uint16_t pin)
{
    if (GPIO_ReadOutputDataBit(GPIOx, pin))
    {
        // ��ǰΪ�ߵ�ƽ������
        GPIO_ResetBits(GPIOx, pin);
    }
    else
    {
        // ��ǰΪ�͵�ƽ������
        GPIO_SetBits(GPIOx, pin);
    }
}

/* use demo
// ��ʼ�� PA15�����������50MHz
gpio_config(GPIOA, GPIO_Pin_15, GPIO_Mode_Out_PP, GPIO_Speed_High);
// ��ʼ�� PB3��PB4��PB5�����������50MHz
gpio_config(GPIOB, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5, GPIO_Mode_Out_PP, GPIO_Speed_High);

*/

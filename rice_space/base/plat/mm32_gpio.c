/**
 * @file mm32_gpio.c
 * @author yang
 * @brief
 * @version 0.1
 * @date 2025-09-18
 *
 * @copyright Copyright (c) 2025
 *
 * @note 1 uart配置；2 反转gpio电平
 */

#include "hal_conf.h"
/**
 * @brief  GPIO 初始化函数
 * @param  GPIOx: 指向要配置的 GPIO 外设的指针
 * @param  pin: 要配置的引脚
 * @param  mode: GPIO 模式
 * @param  speed: GPIO 速度
 * @retval None
 */
void gpio_config(GPIO_TypeDef *GPIOx, uint16_t pin, GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 使能对应GPIO端口时钟
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
    // 其他GPIO口根据芯片手册添加

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
        // 当前为高电平，拉低
        GPIO_ResetBits(GPIOx, pin);
    }
    else
    {
        // 当前为低电平，拉高
        GPIO_SetBits(GPIOx, pin);
    }
}

/* use demo
// 初始化 PA15，推挽输出，50MHz
gpio_config(GPIOA, GPIO_Pin_15, GPIO_Mode_Out_PP, GPIO_Speed_High);
// 初始化 PB3、PB4、PB5，推挽输出，50MHz
gpio_config(GPIOB, GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5, GPIO_Mode_Out_PP, GPIO_Speed_High);

*/

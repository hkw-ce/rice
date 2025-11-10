#include "platform.h"



static uint8_t GPIO_PinToSource(uint16_t GPIO_Pin)
{
    uint8_t pinSource = 0;
    while (GPIO_Pin > 1)
    {
        GPIO_Pin >>= 1;
        pinSource++;
    }
    return pinSource;
}

void mm32_exti_init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                    EXTITrigger_TypeDef TriggerType, GPIOMode_TypeDef InputMode)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    uint8_t pinSource = GPIO_PinToSource(GPIO_Pin);
    uint8_t portSource = 0;

    /* 1?? ???? */
    if (GPIOx == GPIOA) { RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE); portSource = EXTI_PortSourceGPIOA; }
    else if (GPIOx == GPIOB) { RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOB, ENABLE); portSource = EXTI_PortSourceGPIOB; }
    else if (GPIOx == GPIOC) { RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOC, ENABLE); portSource = EXTI_PortSourceGPIOC; }
    else if (GPIOx == GPIOD) { RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOD, ENABLE); portSource = EXTI_PortSourceGPIOD; }
    else if (GPIOx == GPIOF) { RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOF, ENABLE); portSource = EXTI_PortSourceGPIOF; }

    RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFG, ENABLE);

    /* 2?? ?? GPIO ???? */
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin;
    GPIO_InitStruct.GPIO_Mode = InputMode;
    GPIO_Init(GPIOx, &GPIO_InitStruct);

    /* 3?? ?? EXTI Line ??? GPIO */
    SYSCFG_EXTILineConfig(portSource, pinSource);

    /* 4?? ?? EXTI ?? */
    EXTI_StructInit(&EXTI_InitStruct);
    EXTI_InitStruct.EXTI_Line    = GPIO_Pin;        // ? GPIO_Pin ????
    EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = TriggerType;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    /* 5?? ?? NVIC */
    if (pinSource <= 1)
        NVIC_InitStruct.NVIC_IRQChannel = EXTI0_1_IRQn;
    else if (pinSource <= 3)
        NVIC_InitStruct.NVIC_IRQChannel = EXTI2_3_IRQn;
    else
        NVIC_InitStruct.NVIC_IRQChannel = EXTI4_15_IRQn;

    NVIC_InitStruct.NVIC_IRQChannelPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

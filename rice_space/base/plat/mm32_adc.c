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
   case ADC_Channel_10:
       *gpio_port = GPIOC;
       *gpio_pin = GPIO_Pin_0;
       return 0;
   case ADC_Channel_11:
       *gpio_port = GPIOC;
       *gpio_pin = GPIO_Pin_1;
       return 0;
   case ADC_Channel_12:
       *gpio_port = GPIOC;
       *gpio_pin = GPIO_Pin_2;
       return 0;
    case ADC_Channel_13:
        *gpio_port = GPIOC;
        *gpio_pin = GPIO_Pin_3;
        return 0;   
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
//    if (channel == ADC_Channel_VoltReference)
//    {
//    ADC_InitTypeDef ADC_InitStruct;

//    RCC_APB2PeriphClockCmd(RCC_APB2ENR_ADC1, ENABLE);

//    ADC_StructInit(&ADC_InitStruct);
//    ADC_InitStruct.ADC_Resolution         = ADC_Resolution_12b;
//    ADC_InitStruct.ADC_PRESCARE           = ADC_PCLK2_PRESCARE_16;
//    ADC_InitStruct.ADC_Mode               = ADC_Mode_Scan;
//    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
//    ADC_InitStruct.ADC_ExternalTrigConv   = ADC1_ExternalTrigConv_T1_CC1;
//    ADC_InitStruct.ADC_DataAlign          = ADC_DataAlign_Right;
//    ADC_Init(ADC1, &ADC_InitStruct);

//    ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor,    0, ADC_Samctl_240_5);
//    ADC_RegularChannelConfig(ADC1, ADC_Channel_VoltReference, 0, ADC_Samctl_240_5);

//    ADC_TempSensorVrefintCmd(ENABLE);

//    ADC_Cmd(ADC1, ENABLE);

//    
//    uint16_t Value;

//    float Voltage, Temperature;

//		uint16_t OffsetV = *(volatile uint16_t *)(0x1FFFF7E0);
//		uint16_t OffsetT = *(volatile uint16_t *)(0x1FFFF7F6) & 0x0FFF;
//      ADC_SoftwareStartConvCmd(ADC1, ENABLE);
//      while (RESET == ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
//        {
//        }

//        ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

//        /* Internal voltage sensor */
//        Value = ADC_GetChannelConvertedValue(ADC1, ADC_Channel_VoltReference);

//        Voltage = (float)(OffsetV) * (float)3.3 / (float)Value;

//        /* Internal temperature sensor */
//        Value = ADC_GetChannelConvertedValue(ADC1, ADC_Channel_TempSensor);

//        Temperature = 25 + ((float)Value * (float)Voltage * (float)1000 - (float)OffsetT * (float)3300) / ((float)4096 * (float)4.955);
//        return (uint16_t)(Voltage * 1000);
//    }
//    else
//	{
//    GPIO_InitTypeDef GPIO_InitStruct;
//    // 配置单通道采样时间
//    ADC_RegularChannelConfig(hadc, channel, 0, ADC_Samctl_240_5);
//    // 配置采样通道数量 从0开始
//    ADC_ANY_NUM_Config(hadc, 0);
//    ADC_ANY_CH_Config(ADC1, 0, channel);
//    ADC_ANY_Cmd(ADC1, ENABLE);

//    // 配置对应的GPIO模拟输入
//    GPIO_TypeDef *port = NULL;
//    uint16_t pin = 0;
//    if (adc_channel_to_gpio(channel, &port, &pin) == 0)
//    {
//        enable_gpio_clock(port);
//        GPIO_StructInit(&GPIO_InitStruct);
//        GPIO_InitStruct.GPIO_Pin = pin;
//        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
//        GPIO_Init(port, &GPIO_InitStruct);
//    }
//    else
//    {
//        // 处理错误：通道不支持
//    }

//    ADC_SoftwareStartConvCmd(hadc, ENABLE);
// 
//     while (RESET == ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
//        {
//        }

//    ADC_ClearFlag(hadc, ADC_FLAG_EOC);

//    return ADC_GetChannelConvertedValue(hadc, channel);
//}

    ADC_InitTypeDef ADC_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    uint16_t value = 0;

    // 每次读取前重新使能并初始化 ADC 时钟和配置，避免上一状态影响（修复卡死核心）
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_ADC1, ENABLE);
    ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStruct.ADC_PRESCARE = ADC_PCLK2_PRESCARE_16;
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStruct.ADC_ExternalTrigConv = ADC1_ExternalTrigConv_T1_CC1;
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;

    if (channel == ADC_Channel_VoltReference)
    {
        // 为 VREFINT 使用单通道模式（非扫描），避免与外部通道冲突
        ADC_InitStruct.ADC_Mode = ADC_CR_IMM;  // 单次模式

        ADC_Init(ADC1, &ADC_InitStruct);

        // 启用内部参考电压
        ADC_TempSensorVrefintCmd(ENABLE);

        // 配置 VREFINT 通道，rank 从 1 开始（修正无效 rank=0）
        ADC_RegularChannelConfig(ADC1, ADC_Channel_VoltReference, 1, ADC_Samctl_240_5);

        // 配置通道数量为 1（MM32 特定，如果库需要）
        ADC_ANY_NUM_Config(ADC1, 0);  // 假设 0 表示 1 个通道
        ADC_ANY_CH_Config(ADC1, 0, ADC_Channel_VoltReference);  // 兼容原代码
        ADC_ANY_Cmd(ADC1, ENABLE);

        ADC_Cmd(ADC1, ENABLE);

        // 启动转换
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        while (RESET == ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        {
        }
        ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

        // 读取值，使用标准函数（替换非标准 GetChannelConvertedValue）
        value = ADC_GetConversionValue(ADC1);

        // 计算 VDD (mV)
        uint16_t offset_v = *(volatile uint16_t *)(0x1FFFF7E0);  // 校准值地址（确认 datasheet）
        float voltage = (float)offset_v * 3.3f / (float)value;
        return (uint16_t)(voltage * 1000);
    }
    else
    {
        // 为外部通道使用单通道模式
        ADC_InitStruct.ADC_Mode = ADC_CR_IMM;

        ADC_Init(hadc, &ADC_InitStruct);

        // 配置对应的 GPIO 为模拟输入
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
            // 处理错误：通道不支持，返回 0 或错误码
            return 0;
        }

        // 配置通道，rank 从 1 开始（修正无效 rank=0）
        ADC_RegularChannelConfig(hadc, channel, 1, ADC_Samctl_240_5);

        // 配置通道数量为 1
        ADC_ANY_NUM_Config(hadc, 0);
        ADC_ANY_CH_Config(ADC1, 0, channel);  // 假设位置 0 对应 rank 1
        ADC_ANY_Cmd(ADC1, ENABLE);

        ADC_Cmd(hadc, ENABLE);

        // 启动转换
        ADC_SoftwareStartConvCmd(hadc, ENABLE);
        while (RESET == ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        {
        }
        ADC_ClearFlag(hadc, ADC_FLAG_EOC);

        // 读取值，使用标准函数
        return ADC_GetConversionValue(hadc);
    }
}



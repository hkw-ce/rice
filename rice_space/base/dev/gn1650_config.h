#include "platform.h"

#ifndef GN1650_CONFIG_H_
#define GN1650_CONFIG_H_

//==========【配置IIC驱动引脚】========
#define GN1650_SCL_GPIO_PORT    GPIOB
#define GN1650_SCL_GPIO_PIN     GPIO_Pin_10
#define GN1650_SDA_GPIO_PORT    GPIOB
#define GN1650_SDA_GPIO_PIN     GPIO_Pin_11
// 配置驱动SCL的gpio为开漏输出模式
#define  GN1650_IIC_SCL_MODE_OD   \
do{ \
    GPIO_InitTypeDef GPIO_InitStruct; \
    GPIO_InitStruct.GPIO_Pin = GN1650_SCL_GPIO_PIN; \
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD; \
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz; \
    GPIO_Init(GN1650_SCL_GPIO_PORT, &GPIO_InitStruct); \
}while(0)

// 配置驱动SDA的gpio为开漏输出模式
#define  GN1650_IIC_SDA_MODE_OD   \
do{ \
    GPIO_InitTypeDef GPIO_InitStruct; \
    GPIO_InitStruct.GPIO_Pin = GN1650_SDA_GPIO_PIN; \
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD; \
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz; \
    GPIO_Init(GN1650_SDA_GPIO_PORT, &GPIO_InitStruct); \
}while(0)

//=====================================

//========【配置IIC总线的信号读写和时序】=======
// 主机拉高SCL
#define GN1650_IIC_SCL_HIGH     GPIO_SetBits(GN1650_SCL_GPIO_PORT, GN1650_SCL_GPIO_PIN)

// 主机拉低SCL
#define GN1650_IIC_SCL_LOW      GPIO_ResetBits(GN1650_SCL_GPIO_PORT, GN1650_SCL_GPIO_PIN)

// 主机拉高SDA
#define GN1650_IIC_SDA_HIGH     GPIO_SetBits(GN1650_SDA_GPIO_PORT, GN1650_SDA_GPIO_PIN)

// 主机拉低SDA
#define GN1650_IIC_SDA_LOW      GPIO_ResetBits(GN1650_SDA_GPIO_PORT, GN1650_SDA_GPIO_PIN)

// 参数b为0时主机拉低SDA，非0则拉高SDA
#define GN1650_IIC_SDA_WR(b)    do{ \
    if(b) GPIO_SetBits(GN1650_SDA_GPIO_PORT, GN1650_SDA_GPIO_PIN); \
    else  GPIO_ResetBits(GN1650_SDA_GPIO_PORT, GN1650_SDA_GPIO_PIN); \
}while(0)

// 主机读取SDA线电平状态，返回值为0为低电平，非0为高电平
#define GN1650_IIC_SDA_RD()     GPIO_ReadInputDataBit(GN1650_SDA_GPIO_PORT, GN1650_SDA_GPIO_PIN)

// 软件延时2us
#define GN1650_IIC_DELAY_2US    do{for(int ii_=0;ii_<22;ii_++);}while(0)

// 软件延时4us
#define GN1650_IIC_DELAY_4US    do{for(int ii_=0;ii_<40;ii_++);}while(0)
//================================

#endif //GN1650_CONFIG_H_

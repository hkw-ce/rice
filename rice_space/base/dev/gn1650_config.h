#include "platform.h"

#ifndef GN1650_CONFIG_H_
#define GN1650_CONFIG_H_

//==========【配置IIC驱动引脚】========

// 配置驱动SCL的gpio为开漏输出模式
#define  GN1650_IIC_SCL_MODE_OD   \
do{ \
    GPIO_InitTypeDef GPIO_InitStruct; \
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7; \
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD; \
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz; \
    GPIO_Init(GPIOA, &GPIO_InitStruct); \
}while(0)

// 配置驱动SDA的gpio为开漏输出模式
#define  GN1650_IIC_SDA_MODE_OD   \
do{ \
    GPIO_InitTypeDef GPIO_InitStruct; \
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6; \
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD; \
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz; \
    GPIO_Init(GPIOA, &GPIO_InitStruct); \
}while(0)

//=====================================

//========【配置IIC总线的信号读写和时序】=======
// 主机拉高SCL
#define GN1650_IIC_SCL_HIGH     GPIO_SetBits(GPIOA, GPIO_Pin_7)

// 主机拉低SCL
#define GN1650_IIC_SCL_LOW      GPIO_ResetBits(GPIOA, GPIO_Pin_7)

// 主机拉高SDA
#define GN1650_IIC_SDA_HIGH     GPIO_SetBits(GPIOA, GPIO_Pin_6)

// 主机拉低SDA
#define GN1650_IIC_SDA_LOW      GPIO_ResetBits(GPIOA, GPIO_Pin_6)

// 参数b为0时主机拉低SDA，非0则拉高SDA
#define GN1650_IIC_SDA_WR(b)    do{ \
    if(b) GPIO_SetBits(GPIOA, GPIO_Pin_6); \
    else  GPIO_ResetBits(GPIOA, GPIO_Pin_6); \
}while(0)

// 主机读取SDA线电平状态，返回值为0为低电平，非0为高电平
#define GN1650_IIC_SDA_RD()     GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_6)

// 软件延时2us
#define GN1650_IIC_DELAY_2US    do{for(int ii_=0;ii_<22;ii_++);}while(0)

// 软件延时4us
#define GN1650_IIC_DELAY_4US    do{for(int ii_=0;ii_<40;ii_++);}while(0)
//================================

#endif //GN1650_CONFIG_H_

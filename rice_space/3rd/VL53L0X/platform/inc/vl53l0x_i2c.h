#ifndef __VL53L0_I2C_H
#define __VL53L0_I2C_H

//#include "stm32f10x.h"
#include "platform.h"

#ifndef u8
#define u8 uint8_t
#endif
#ifndef u16
#define u16 uint16_t
#endif
#ifndef u32
#define u32 uint32_t
#endif

//端口移植
#define RCC_VL53L0x		RCC_AHBENR_GPIOA
#define PORT_VL53L0x 	GPIOA

#define GPIO_SDA 		GPIO_Pin_9
#define GPIO_SCL 		GPIO_Pin_10

//设置SDA输出模式
#define SDA_OUT()   {        \
                        GPIO_InitTypeDef  GPIO_InitStructure; \
                        GPIO_InitStructure.GPIO_Pin = GPIO_SDA; \
                        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; \
                        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; \
                        GPIO_Init(PORT_VL53L0x, &GPIO_InitStructure); \
                     }
//设置SDA输入模式
#define SDA_IN()    {        \
                        GPIO_InitTypeDef  GPIO_InitStructure; \
                        GPIO_InitStructure.GPIO_Pin = GPIO_SDA; \
                        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; \
                        GPIO_Init(PORT_VL53L0x, &GPIO_InitStructure); \
                    }
//获取SDA引脚的电平变化
#define SDA_GET()       GPIO_ReadInputDataBit(PORT_VL53L0x, GPIO_SDA)
//SDA与SCL输出
#define SDA(x)          GPIO_WriteBit(PORT_VL53L0x, GPIO_SDA, (x?Bit_SET:Bit_RESET) )
#define SCL(x)          GPIO_WriteBit(PORT_VL53L0x, GPIO_SCL, (x?Bit_SET:Bit_RESET) )

//状态
#define STATUS_OK       0x00
#define STATUS_FAIL     0x01

//IIC操作函数
void VL53L0X_i2c_init(void);//初始化IIC的IO口

u8 VL53L0X_write_byte(u8 address,u8 index,u8 data);              //IIC写一个8位数据
u8 VL53L0X_write_word(u8 address,u8 index,u16 data);             //IIC写一个16位数据
u8 VL53L0X_write_dword(u8 address,u8 index,u32 data);            //IIC写一个32位数据
u8 VL53L0X_write_multi(u8 address, u8 index,u8 *pdata,u16 count);//IIC连续写

u8 VL53L0X_read_byte(u8 address,u8 index,u8 *pdata);             //IIC读一个8位数据
u8 VL53L0X_read_word(u8 address,u8 index,u16 *pdata);            //IIC读一个16位数据
u8 VL53L0X_read_dword(u8 address,u8 index,u32 *pdata);           //IIC读一个32位数据
u8 VL53L0X_read_multi(u8 address,u8 index,u8 *pdata,u16 count);  //IIC连续读


#endif 



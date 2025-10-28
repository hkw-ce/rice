#ifndef __MYIIC_H
#define __MYIIC_H
//#include "sys.h"
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//IIC 驱动函数	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/6/10 
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  
//#define GPIO_PIN_11                ((uint16_t)0x0800)  /* Pin 11 selected   */
//#define GPIO_PIN_12                ((uint16_t)0x1000)  /* Pin 12 selected   */
   	   		   
//IO方向设置
#define SDA_IN()  {GPIOA->CRH&=0XFFFF0FFF;GPIOA->CRH|=8<<12;}
#define SDA_OUT() {GPIOA->CRH&=0XFFFF0FFF;GPIOA->CRH|=3<<12;}

//IO操作函数	 
#define IIC_SCL(PinState)    if (PinState != GPIO_PIN_RESET){GPIOA->BSRR = ((uint16_t)0x0800);}else{GPIOA->BSRR = (uint32_t)((uint16_t)0x0800) << 16u;}   //PAout(12) //SCL
#define IIC_SDA(PinState)    if (PinState != GPIO_PIN_RESET){GPIOA->BSRR = ((uint16_t)0x1000);}else{GPIOA->BSRR = (uint32_t)((uint16_t)0x1000) << 16u;}   //PAout(11) //SDA	 
#define READ_SDA   HAL_GPIO_ReadPin(GPIOA, ((uint16_t)0x0800))    //PAin(11)  //输入SDA 


//IIC所有操作函数
//void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);			//IIC发送一个字节
uint8_t IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
uint8_t IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

extern void delay_us(uint8_t t);
extern void delay_ms(uint8_t t);
	  
#endif

















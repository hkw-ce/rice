#ifndef __MYIIC_H
#define __MYIIC_H
//#include "sys.h"
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//IIC ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/6/10 
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  
//#define GPIO_PIN_11                ((uint16_t)0x0800)  /* Pin 11 selected   */
//#define GPIO_PIN_12                ((uint16_t)0x1000)  /* Pin 12 selected   */
   	   		   
//IO��������
#define SDA_IN()  {GPIOA->CRH&=0XFFFF0FFF;GPIOA->CRH|=8<<12;}
#define SDA_OUT() {GPIOA->CRH&=0XFFFF0FFF;GPIOA->CRH|=3<<12;}

//IO��������	 
#define IIC_SCL(PinState)    if (PinState != GPIO_PIN_RESET){GPIOA->BSRR = ((uint16_t)0x0800);}else{GPIOA->BSRR = (uint32_t)((uint16_t)0x0800) << 16u;}   //PAout(12) //SCL
#define IIC_SDA(PinState)    if (PinState != GPIO_PIN_RESET){GPIOA->BSRR = ((uint16_t)0x1000);}else{GPIOA->BSRR = (uint32_t)((uint16_t)0x1000) << 16u;}   //PAout(11) //SDA	 
#define READ_SDA   HAL_GPIO_ReadPin(GPIOA, ((uint16_t)0x0800))    //PAin(11)  //����SDA 


//IIC���в�������
//void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(uint8_t txd);			//IIC����һ���ֽ�
uint8_t IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
uint8_t IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

extern void delay_us(uint8_t t);
extern void delay_ms(uint8_t t);
	  
#endif

















/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 立创论坛：https://oshwhub.com/forum
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 * Change Logs:
 * Date           Author       Notes
 * 2024-03-29     LCKFB-LP    first version
 */
#ifndef __VL53L0X_H
#define __VL53L0X_H

#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"
#include "vl53l0x_gen.h"
#include "vl53l0x_cali.h"
#include "vl53l0x_it.h"
#include "platform.h"
//#include "board.h"
//#include "sys.h"

#define  delay_ms(x) rt_thread_delay(x)    


//VL53L0X传感器上电默认IIC地址为0X52(不包含最低位)
#define VL53L0X_Addr 0x52


#define BIT_ADDR(Addr, Bit_Num) 		*((volatile uint32_t *)(0x42000000 + ((Addr - 0x40000000) * 32) + (Bit_Num * 4)))


#define GPIOA_ODR_ADDR    (GPIOA_BASE+12) //0x40000000 + 0x10000 + 0x0800 + 0x0C
#define GPIOB_ODR_ADDR    (GPIOB_BASE+12) //0x40000000 + 0x10000 + 0x0C00 + 0x0C
#define GPIOC_ODR_ADDR    (GPIOC_BASE+12) //0x40000000 + 0x10000 + 0x1000 + 0x0C


#define GPIOA_IDR_ADDR    (GPIOA_BASE+8) //0x40000000 + 0x10000 + 0x0800 + 0x08
#define GPIOB_IDR_ADDR    (GPIOB_BASE+8) //0x40000000 + 0x10000 + 0x0C00 + 0x08
#define GPIOC_IDR_ADDR    (GPIOC_BASE+8) //0x40000000 + 0x10000 + 0x1000 + 0x08


#define PAout(Pin)	BIT_ADDR(GPIOA_ODR_ADDR, Pin) // GPIOA输出
#define PBout(Pin)	BIT_ADDR(GPIOB_ODR_ADDR, Pin) // GPIOB输出
#define PCout(Pin)	BIT_ADDR(GPIOC_ODR_ADDR, Pin) // GPIOC输出

#define PAin(Pin)	BIT_ADDR(GPIOA_IDR_ADDR, Pin) // GPIOA输入
#define PBin(Pin)	BIT_ADDR(GPIOB_IDR_ADDR, Pin) // GPIOB输入
#define PCin(Pin)	BIT_ADDR(GPIOC_IDR_ADDR, Pin) // GPIOC输入


//控制Xshut电平,从而使能VL53L0X工作 1:使能 0:关闭
#define VL53L0X_Xshut(x)  		((x) ? (GPIOA->BSRR = (1UL << 12)) \
                               : (GPIOA->BSRR = (1UL << (12 + 16))))

//使能2.8V IO电平模式
#define USE_I2C_2V8  1

//测量模式
#define Default_Mode   0// 默认
#define HIGH_ACCURACY  1//高精度
#define LONG_RANGE     2//长距离
#define HIGH_SPEED     3//高速

//vl53l0x模式配置参数集
typedef struct packed_2
{
	FixPoint1616_t signalLimit;    //Signal极限数值 
	FixPoint1616_t sigmaLimit;     //Sigmal极限数值
	uint32_t timingBudget;         //采样时间周期
	uint8_t preRangeVcselPeriod ;  //VCSEL脉冲周期
	uint8_t finalRangeVcselPeriod ;//VCSEL脉冲周期范围
	
}mode_data;


extern mode_data Mode_data[];
extern uint8_t AjustOK;

VL53L0X_Error vl53l0x_init(VL53L0X_Dev_t *dev);//初始化vl53l0x
void print_pal_error(VL53L0X_Error Status);//错误信息打印
void mode_string(u8 mode,char *buf);//模式字符串显示
void vl53l0x_test(void);//vl53l0x测试
void vl53l0x_reset(VL53L0X_Dev_t *dev);//vl53l0x复位

void vl53l0x_info(void);//获取vl53l0x设备ID信息
void One_measurement(u8 mode);//获取一次测量距离数据
#endif


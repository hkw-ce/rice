#include <math.h>
#include "app.h"
#include "platform.h"
#include <stdint.h>
#include "types.h" 


#define INA226_I2C i2c3

i2c_bus_t i2c3 = {
.scl_port = GPIOC,
.scl_pin  = GPIO_Pin_2,
.sda_port = GPIOC,
.sda_pin  = GPIO_Pin_3,
.delay_us = default_delay_us,
};
uint16_t g_ina226_getcurrent = 0;
uint16_t g_ina226_getavgcurrent = 0;

/* INA226_CALIBRATION ---------------------------------------------------------------------------------*/
//#define Calibration_H 0x04
//#define Calibration_L 0x00

//MAX_Current = Full-scale range / R·SHUNT = 81.92mV / 0.1R = 819.2mA
//Current_LSB = 819.2mA / 2^15 = 0.025mA 
//选择接近值：0.05mA = 0.00005A 手册推荐1mA 但是便于CAL计算和实际调试则重新选择为 0.05mA
//(手册：While this value yields the highest resolution, it is common to select a value for the Current_LSB to the nearest round number above this value to simplify the conversion of the Current Register (04h) and Power Register (03h) to amperes and watts respectively.)
//(虽然该值产生最高分辨率，但通常为Current_LSB选择一个值，使其与高于该值的最接近的整数相匹配，以简化电流寄存器（04h）和功率寄存器（03h）分别转换为安培和瓦的过程。)
//CAL = 0.00512 / (Current_LSB * R·SHUNT) = 0.00512 / (0.00005A * 0.1R) = 1024 = 0x0400
//Power_LSB = Current_LSB * 25 = 0.05mA * 25 = 1.25mW
//(手册：The power LSB has a fixed ratio to the Current_LSB of 25)
//(power LSB与Current_LSB的固定比率为25)
#define INA226_ADDRESS 0x40
#define Calibration_H 0x0A
#define Calibration_L 0x00
//实测发现电流偏大，采用手册校准公式 Corrected_Full_Scale_Cal = (Cal * MeasShuntCurrent) / INA226_CURRENT 
//5V 测试条件下 MeasShuntCurrent = 31 mA 这是在5V测试条件下，通过其他方法实际测量到的分流电阻上的真实电流值  INA226_CURRENT = 32mA这是INA226传感器本身测量并输出的电流值 
//// Corrected_Full_Scale_Cal = 31 * 1024 / 32= 992 = 0x03E0
///**
//* @brief 向INA226寄存器写入数据
//*
//* 使用I2C通信协议向INA226的指定寄存器写入16位数据。
//*
//* @param Register 要写入的寄存器地址
//* @param Data_H 要写入寄存器的高8位数据
//* @param Data_L 要写入寄存器的低8位数据
//*/
extern i2c_bus_t INA226_I2C;
void INA226_WriteReg(uint8_t Register, uint8_t Data_H, uint8_t Data_L)
{
	i2c_write_bytes(&INA226_I2C, INA226_ADDRESS, Register, (uint8_t[]){Data_H, Data_L}, 2);
}
/**
* @brief 从INA226寄存器读取数据
*
* 通过I2C总线从INA226指定的寄存器地址读取数据。
*
* @param RegAddress 要读取的寄存器地址
* @return 从寄存器读取的数据
*/
uint16_t INA226_ReadReg(uint8_t RegAddress)
{
	uint8_t Data[2];
	i2c_read_bytes(&INA226_I2C, INA226_ADDRESS, RegAddress, (uint8_t*)&Data, 2);
	return (Data[0] << 8) | Data[1];
}
/**
* @brief 初始化INA226芯片
*
* 该函数用于初始化INA226芯片，包括配置寄存器和校准寄存器的设置。
*
* @details
* 1. 调用INA226_WriteReg函数，设置INA226_CONFIGURATION寄存器，
*    配置为4次平均、1.1ms转换时间、连续检测模式。
* 2. 调用INA226_WriteReg函数，设置INA226_CALIBRATION寄存器，
*    设置基准值为0x0200。
*/


void INA226_Init(void)
{
    i2c_bus_init(&INA226_I2C);// 确保使用I2C3配置
 	INA226_WriteReg(INA226_CONFIGURATION,Configuration_H,Configuration_L);   //4次平均  1.1ms转换时间  连续检测
	INA226_WriteReg(INA226_CALIBRATION,Calibration_H,Calibration_L);         //基准值 
}

/**
* @brief 获取分流电压值
*
* 从INA226设备读取分流电压寄存器的值，并将其转换为实际电压值（单位为毫伏）。
*
* @return 分流电压值，单位为毫伏。
*/
uint32_t INA226_GetShuntVoltage(void)
{
	uint32_t ShuntVoltage;
	ShuntVoltage = (uint32_t)((INA226_ReadReg(INA226_SHUNTVOLTAGE)) * 2.5 );
 //ShuntVoltage = (uint32_t)((INA226_ReadReg(INA226_SHUNTVOLTAGE)) * 2.5 / 1000);
	
	return ShuntVoltage;
}
/**
* @brief 获取总线电压值
*
* 从INA226传感器读取总线电压寄存器值，并将其转换为电压值（单位为mV）。
* 总线电压值计算公式为：总线电压值 = 寄存器值 * LSB(1.25mV)。
*
* @return 返回总线电压值（单位为mV），类型为uint32_t。
*/
uint32_t INA226_GetBusVoltage(void)
{
	uint32_t BusVoltage;
	BusVoltage = (uint32_t)((INA226_ReadReg(INA226_BUSVOLTAGE)) * 1.25);
	
	return BusVoltage;
}
/**
* @brief 获取INA226芯片的当前电流值
*
* 通过读取INA226芯片的电流寄存器值，将其转换为实际电流值（单位为mA）。
* 电流值计算公式为：电流值 = 寄存器值 * Current_LSB（0.05mA）。
*
* @return 当前电流值（单位：mA），类型为uint32_t
*/
uint32_t INA226_GetCurrent(void)
{
	uint32_t Current;

	Current = (uint32_t)((INA226_ReadReg(INA226_CURRENT)) *1.831 );
//Current = (uint32_t)((INA226_ReadReg(INA226_CURRENT)) * 0.05);
	return Current;
}
/**
* @brief 获取INA226的功率值
*
* 获取INA226传感器的功率值，计算方法为：功率 = 寄存器值 * Power_LSB(1.25mW)。
*
* @return 返回计算得到的功率值（单位：mW）。
*/
uint32_t INA226_GetPower(void) 
{
	uint32_t Power = 0;
	uint32_t regValue = 0;
	
	regValue = (uint32_t)INA226_ReadReg(INA226_POWER);

	Power = (uint32_t)(regValue *50);// 等价于乘以1.25但避免浮点运算

	return Power;
}

/**
* @brief 获取INA226芯片的芯片ID
*
* 该函数通过读取INA226的DIEID寄存器来获取芯片的ID。
*
* @return 返回一个16位无符号整数，表示芯片的ID。
*/
uint16_t INA226_GetDieID(void) 
{
	uint16_t dieid = 0;

	dieid  = (uint16_t)INA226_ReadReg(INA226_DIEID);

	return dieid;
}

void ina226_read_rice_info(rice_information_t *info)
{
    info->V_supply = INA226_GetBusVoltage()*2;
    info->I_supply = INA226_GetCurrent();
    info->P_supply = INA226_GetPower()*2;
	#if INA226_DEBUG
	LOG_I("INA226 Read Rice Info: V=%lu mV, I=%lu mA, P=%lu mW", info->V_supply, info->I_supply, info->P_supply);
	#endif
}
void INA226_Test(void)
{
	uint16_t dieid = 0;
	uint32_t get_power = 0;
	u32 get_curr=0;
	u32 get_busvolt=0;
	u32 get_shuntvolt=0;
	INA226_Init();
	dieid = INA226_GetDieID();
	LOG_I("INA226 DieID: 0x%04X", dieid);
	for (int i = 0; i < 50; i++)
	{
	get_power = INA226_GetPower();
	LOG_I("INA226 Power: %lu mW", get_power*2);
	get_curr = INA226_GetCurrent();
	LOG_I("INA226 Current: %lu mA", get_curr);
	get_busvolt = INA226_GetBusVoltage();
	LOG_I("INA226 Bus Voltage: %lu mV", get_busvolt*2);
	get_shuntvolt = INA226_GetShuntVoltage();
	LOG_I("INA226 Shunt Voltage: %lu mV", get_shuntvolt);
	rt_thread_delay(50);
	}
	
	
}
MSH_CMD_EXPORT(INA226_Test, ina226 test function);


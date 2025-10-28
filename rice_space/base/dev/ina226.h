#ifndef __INA226_H__
#define __INA226_H__

#include <stdint.h>
#include "types.h" 

/*IIC 地址---------------------------------------------------------------------------------------------*/
#define INA226_W 0x80
#define INA226_R 0x81
/*寄存器地址--------------------------------------------------------------------------------------------*/
#define	INA226_CONFIGURATION                          0x00
#define INA226_SHUNTVOLTAGE                           0x01
#define INA226_BUSVOLTAGE                             0x02
#define INA226_POWER                                  0x03
#define INA226_CURRENT                                0x04
#define INA226_CALIBRATION                            0x05
#define	INA226_MASK                                   0x06
#define	INA226_ALERTLIMIT                             0x07
#define	INA226_MANUFACTURERID                         0xFE
#define	INA226_DIEID                                  0xFF
/* INA226_curation Bit15-0 --------------------------------------------------------------------------*/
#define RST 			        0 		// 0   设置成1复位 （Bit15）
#define Reservation       0x04  // 100 （Bit14-12 保留）
#define AVG 							0x01  // 001 平均次数 4 （Bit11-9）
#define VBUSCT  					0x04	// 100 总线电压转换时间 1.1ms （Bit116-8）
#define VSHCT		  				0x04	// 100 分流电压转换时间 1.1ms（Bit3-5）
#define	MODE	            0x07 	// 111 运行模式  连续检测（默认）（Bit0–2）

#define Configuration_H (RST << 7)|(Reservation << 4)|(AVG << 1)|(VBUSCT >> 2)
#define Configuration_L ((VBUSCT & 0x03) << 6)|(VSHCT << 3)|(MODE)






extern uint16_t g_ina226_getcurrent;
extern uint16_t g_ina226_getavgcurrent;
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
extern void INA226_Init(void);
					  
/**
* @brief 向INA226寄存器写入数据
*
* 使用I2C通信协议向INA226的指定寄存器写入16位数据。
*
* @param Register 要写入的寄存器地址
* @param Data_H 要写入寄存器的高8位数据
* @param Data_L 要写入寄存器的低8位数据
*/
extern void INA226_WriteReg(uint8_t Register, uint8_t Data_H, uint8_t Data_L);

/**
* @brief 从INA226寄存器读取数据
*
* 通过I2C总线从INA226指定的寄存器地址读取数据。
*
* @param RegAddress 要读取的寄存器地址
* @return 从寄存器读取的数据
*/
extern uint16_t INA226_ReadReg(uint8_t RegAddress);

/**
* @brief 获取INA226的分流电压值
*
* 获取INA226的分流电压值，分流电压值 = 寄存器值 * LSB(2.5uA)
*
* @return 返回分流电压值，单位为微安（uA）
*/
extern uint32_t INA226_GetShuntVoltage(void);

/**
* @brief 获取总线电压值
*
* 从INA226传感器读取总线电压寄存器值，并将其转换为电压值（单位为mV）。
* 总线电压值计算公式为：总线电压值 = 寄存器值 * LSB(1.25mV)。
*
* @return 返回总线电压值（单位为mV），类型为uint32_t。
*/
extern uint32_t INA226_GetBusVoltage(void);


/**
* @brief 获取INA226芯片的当前电流值
*
* 通过读取INA226芯片的电流寄存器值，将其转换为实际电流值（单位为mA）。
* 电流值计算公式为：电流值 = 寄存器值 * Current_LSB（0.05mA）。
*
* @return 当前电流值（单位：mA），类型为uint32_t
*/
extern uint32_t INA226_GetCurrent(void);

/**
* @brief 获取INA226的功率值
*
* 获取INA226传感器的功率值，计算方法为：功率 = 寄存器值 * Power_LSB(1.25mW)。
*
* @return 返回计算得到的功率值（单位：mW）。
*/
extern uint32_t INA226_GetPower(void) ;

/**
* @brief 获取INA226芯片的芯片ID
*
* 该函数通过读取INA226的DIEID寄存器来获取芯片的ID。
*
* @return 返回一个16位无符号整数，表示芯片的ID。
*/
extern uint16_t INA226_GetDieID(void);

#endif /* __INA226_H__ */





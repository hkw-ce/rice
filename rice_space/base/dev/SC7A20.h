#ifndef __SL_SC7A20_ANGLE_DRIVER_H__
#define __SL_SC7A20_ANGLE_DRIVER_H__

#include <stdint.h>
#include "types.h" 

#define BOOL unsigned char
#define TRUE 1
#define FALSE 0

/***使用驱动前请根据实际接线情况配置******/
/**SC7A20的SDO 脚接地：  0****************/
/**SC7A20的SDO 脚接电源：1****************/
#define SC7A20_SDO_VDD_GND            1
/*****************************************/

/***使用驱动前请根据实际IIC情况进行配置***/
/**SC7A20的IIC 接口地址类型 7bits：  0****/
/**SC7A20的IIC 接口地址类型 8bits：  1****/
#define SC7A20_IIC_7BITS_8BITS        0
/*****************************************/

#define  SL_SC7A20_16BIT_8BIT         1
/**SC7A20的数据位数选择  16bits：    1****/
/**SC7A20的数据位数选择   8bits：    0****/
/*****************************************/

#define  SL_SC7A20_SPI_IIC_MODE       1
/**SC7A20 SPI IIC 选择    SPI：      0****/
/**SC7A20 SPI IIC 选择    IIC：      1****/
/*****************************************/

#define SC7A20_CHIP_ID_ADDRESS    (unsigned char)0x0F
#define SC7A20_CHIP_ID_VALUE      (unsigned char)0x11

#define  SL_SC7A20_CTRL_REG1      (unsigned char)0x20
#define  SL_SC7A20_CTRL_REG2      (unsigned char)0x21
#define  SL_SC7A20_CTRL_REG3      (unsigned char)0x22
#define  SL_SC7A20_CTRL_REG4      (unsigned char)0x23 
#define  SL_SC7A20_CTRL_REG5      (unsigned char)0x24
#define  SL_SC7A20_CTRL_REG6      (unsigned char)0x25

#define  SL_SC7A20_STATUS_REG     (unsigned char)0x27

#define  SL_SC7A20_OUT_X_L        (unsigned char)0x28
#define  SL_SC7A20_OUT_X_H        (unsigned char)0x29
#define  SL_SC7A20_OUT_Y_L        (unsigned char)0x2A
#define  SL_SC7A20_OUT_Y_H        (unsigned char)0x2B
#define  SL_SC7A20_OUT_Z_L        (unsigned char)0x2C
#define  SL_SC7A20_OUT_Z_H        (unsigned char)0x2D

#define  SL_SC7A20_FIFO_CTRL_REG  (unsigned char)0x2E
#define  SL_SC7A20_FIFO_SRC_REG   (unsigned char)0x2F

#define  SL_SC7A20_AOI1_CFG    	  (unsigned char)0x30
#define  SL_SC7A20_AOI1_SRC       (unsigned char)0x31
#define  SL_SC7A20_AOI1_THS    	  (unsigned char)0x32
#define  SL_SC7A20_AOI1_DURATION  (unsigned char)0x33

#define  SL_SC7A20_AOI2_CFG    	  (unsigned char)0x34
#define  SL_SC7A20_AOI2_SRC       (unsigned char)0x35
#define  SL_SC7A20_AOI2_THS    	  (unsigned char)0x36
#define  SL_SC7A20_AOI2_DURATION  (unsigned char)0x37
#define  SL_SC7A20_CLICK_CFG   	  (unsigned char)0x38
#define  SL_SC7A20_CLICK_SRC   	  (unsigned char)0x39
#define  SL_SC7A20_CLICK_THS   	  (unsigned char)0x3A
#define  SL_SC7A20_TIME_LIMIT     (unsigned char)0x3B
#define  SL_SC7A20_TIME_LATENCY   (unsigned char)0x3C
#define  SL_SC7A20_TIME_WINDOW    (unsigned char)0x3D
#define  SL_SC7A20_ACT_THS        (unsigned char)0x3E
#define  SL_SC7A20_ACT_DURATION   (unsigned char)0x3F
	
/*连续读取数据时的数据寄存器地址*/
#define  SL_SC7A20_DATA_OUT       (unsigned char)(SL_SC7A20_OUT_X_L|0x80)

/**********特殊功能寄存器**********/
/*非原厂技术人员请勿修改*/
#define  SL_SC7A20_MTP_ENABLE    	             0x00
#define  SL_SC7A20_MTP_CFG    	  (unsigned char)0x1E
#define  SL_SC7A20_MTP_VALUE   	  (unsigned char)0x05
#define  SL_SC7A20_SDOI2C_PU_CFG  (unsigned char)0x57
#define  SL_SC7A20_SDO_PU_MSK     (unsigned char)0x08
#define  SL_SC7A20_I2C_PU_MSK     (unsigned char)0x04
#define  SL_SC7A20_HR_ENABLE      (unsigned char)0X08
#define  SL_SC7A20_BOOT_ENABLE    (unsigned char)0X80   
/*非原厂技术人员请勿修改*/


/***************数据更新速率**加速度计使能**********/
#define  SL_SC7A20_ODR_POWER_DOWN (unsigned char)0x00
#define  SL_SC7A20_ODR_1HZ        (unsigned char)0x17
#define  SL_SC7A20_ODR_10HZ       (unsigned char)0x27
#define  SL_SC7A20_ODR_25HZ       (unsigned char)0x37
#define  SL_SC7A20_ODR_50HZ       (unsigned char)0x47
#define  SL_SC7A20_ODR_100HZ      (unsigned char)0x57
#define  SL_SC7A20_ODR_200HZ      (unsigned char)0x67
#define  SL_SC7A20_ODR_400HZ      (unsigned char)0x77
#define  SL_SC7A20_ODR_1600HZ     (unsigned char)0x87
#define  SL_SC7A20_ODR_1250HZ     (unsigned char)0x97
#define  SL_SC7A20_ODR_5000HZ     (unsigned char)0x9F
    
#define  SL_SC7A20_LOWER_POWER_ODR_1HZ        (unsigned char)0x1F
#define  SL_SC7A20_LOWER_POWER_ODR_10HZ       (unsigned char)0x2F
#define  SL_SC7A20_LOWER_POWER_ODR_25HZ       (unsigned char)0x3F
#define  SL_SC7A20_LOWER_POWER_ODR_50HZ       (unsigned char)0x4F
#define  SL_SC7A20_LOWER_POWER_ODR_100HZ      (unsigned char)0x5F
#define  SL_SC7A20_LOWER_POWER_ODR_200HZ      (unsigned char)0x6F
#define  SL_SC7A20_LOWER_POWER_ODR_400HZ      (unsigned char)0x7F
/***************数据更新速率**加速度计使能**********/


/***************传感器量程设置**********************/
#define  SL_SC7A20_FS_2G          (unsigned char)0x00		
#define  SL_SC7A20_FS_4G          (unsigned char)0x10
#define  SL_SC7A20_FS_8G          (unsigned char)0x20	
#define  SL_SC7A20_FS_16G         (unsigned char)0x30
/***************传感器量程设置**********************/


/***取值在0-127之间，此处仅举例****/
#define SL_SC7A20_INT_THS_5PERCENT   (unsigned char)0x06
#define SL_SC7A20_INT_THS_8PERCENT   (unsigned char)0x09    
#define SL_SC7A20_INT_THS_10PERCENT  (unsigned char)0x0C
#define SL_SC7A20_INT_THS_20PERCENT  (unsigned char)0x18
#define SL_SC7A20_INT_THS_40PERCENT  (unsigned char)0x30
#define SL_SC7A20_INT_THS_80PERCENT  (unsigned char)0x60


/***取值在0-127之间，此处仅举例 乘以ODR单位时间****/
#define SL_SC7A20_INT_DURATION_2CLK  (unsigned char)0x02
#define SL_SC7A20_INT_DURATION_5CLK  (unsigned char)0x05
#define SL_SC7A20_INT_DURATION_10CLK (unsigned char)0x0A

/***中断有效时的电平设置，高电平相当于上升沿，低电平相当于下降沿****/
#define SL_SC7A20_INT_ACTIVE_LOWER_LEVEL 0x02 //0x02:中断时INT1脚输出 低电平
#define SL_SC7A20_INT_ACTIVE_HIGH_LEVEL  0x00 //0x00:中断时INT1脚输出 高电平

/***中断有效时的电平设置，高电平相当于上升沿，低电平相当于下降沿****/
#define SL_SC7A20_INT_AOI1_INT1          0x40 //AOI1 TO INT1
#define SL_SC7A20_INT_AOI2_INT1          0x20 //AOI2 TO INT1

typedef struct
{
  int16_t		gravity_x;
  int16_t 	gravity_y;
  int16_t 	gravity_z;
  uint8_t		int1_tag;
  uint8_t		int2_tag;
  uint16_t	z_angle;
  bool      tilted;               // 当前状态（true=倾倒）
  uint32_t  last_change_tick; // 上次变化的时间
  float     angle_deg;           // 当前倾角
}SL_SC7A20_t;

extern  SL_SC7A20_t sl_sc7A20;
extern int16_t x_acc, y_acc, z_acc;
/********客户需要进行的IIC接口封包函数****************/
extern unsigned char SL_SC7A20_I2c_Spi_Write(unsigned char sl_spi_iic,unsigned char reg, unsigned char data);
extern unsigned char SL_SC7A20_I2c_Spi_Read(unsigned char sl_spi_iic,unsigned char reg, unsigned char len, unsigned char *buf);


signed char  SL_SC7A20_Online_Test(void);
signed char  SL_SC7A20_BOOT(void);
signed char  SL_SC7A20_FS_Config(unsigned char Sc7a20_FS_Reg);
signed char  SL_SC7A20_INT_Config(void);
signed char  SL_SC7A20_INT_RESET(void);
signed char  SL_SC7A20_Power_Config(unsigned char Power_Config_Reg);


#if   SL_SC7A20_16BIT_8BIT==0
signed char  SL_SC7A20_Read_XYZ_Data(signed char *SL_SC7A20_Data_XYZ_Buf);
#elif SL_SC7A20_16BIT_8BIT==1
signed char  SL_SC7A20_Read_XYZ_Data(signed short *SL_SC7A20_Data_XYZ_Buf);
#endif


uint8_t SC7A20_Init(void);
uint8_t SC7A20_ReadXYZ(int16_t *x, int16_t *y, int16_t *z);
void SC7A20_GetZAxisAngle(int16_t Xa, int16_t Ya, int16_t Za);
uint8_t SC7A20_Init_With_ThresholdINT(void) ;
uint8_t SC7A20_Click_Init(void) ;
uint8_t SC7A20_Fall_Init(void) ;
void read_accelerometer_data(void);
uint8_t SC7A20_Lift_Init(void);
void SC7A20_EXTI_Configure(void);
/**
 * @brief 打开SC7A20的外部中断
 * @note 用于重新启用之前被禁用的SC7A20外部中断
 */
void SC7A20_EXTI_Enable(void);

/**
 * @brief 关闭SC7A20的外部中断
 * @note 用于临时禁用SC7A20的外部中断
 */
void SC7A20_EXTI_Disable(void);

/**
 * @brief 切换SC7A20的外部中断状态
 * @param enable: TRUE-启用中断，FALSE-禁用中断
 */
void SC7A20_EXTI_SetState(BOOL enable);

/**
 * @brief 获取SC7A20的外部中断状态
 * @return BOOL: TRUE-中断已启用，FALSE-中断已禁用
 */
BOOL SC7A20_EXTI_GetState(void);


void SC7A20_UpdateTiltState(SL_SC7A20_t *dev);

#endif /* __SL_SC7A20_DRIVER_H */




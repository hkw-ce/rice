#include "SC7A20.h"
#include <math.h>
#include "platform.h"
#include <stdint.h>
#include "types.h" 
#include "SC7A20.h"
SL_SC7A20_t sl_sc7A20;

#define SC7A20_i2c i2c4

i2c_bus_t i2c4 = {
.scl_port = GPIOA,
.scl_pin  = GPIO_Pin_10,
.sda_port = GPIOA,
.sda_pin  = GPIO_Pin_9,
.delay_us = default_delay_us,
};

#define PERIPH_SC7A20_ADD  0x18
int16_t x_acc, y_acc, z_acc;  // 用于存储三个轴的加速度数据

signed char  SL_SC7A20_Online_Test(void)
{
    unsigned char SL_Read_Reg=0xff;
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SC7A20_CHIP_ID_ADDRESS,&SL_Read_Reg, 1);
    if(SL_Read_Reg==SC7A20_CHIP_ID_VALUE)  
			return  1;
    else                                    
			return -1;
}

/***************BOOT 重载内部寄存器值*********************/
signed char  SL_SC7A20_BOOT(void)
{
    unsigned char SL_Read_Reg=0xff;
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG5, &SL_Read_Reg, 1);
    SL_Read_Reg=SL_SC7A20_BOOT_ENABLE|SL_Read_Reg;
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG5, SL_Read_Reg);
    return  1;
}

/***************传感器量程设置**********************/
signed char  SL_SC7A20_FS_Config(unsigned char Sc7a20_FS_Reg)
{
    unsigned char SL_Read_Reg=0xff,SL_Write_Reg;
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG4,&SL_Read_Reg, 1);
    SL_Write_Reg=0x80|Sc7a20_FS_Reg|SL_SC7A20_HR_ENABLE;	//被读取后更新	高精度
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG4, SL_Write_Reg);
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG4,&SL_Read_Reg, 1);
    if(SL_Read_Reg==SL_Write_Reg)   return  1;
    else                            return -1;
}

//AOI2_INT1  位置检测，检测是否离开Y+位置  1D位置识别
/***************中断设置*************/

signed char  SL_SC7A20_INT_Config(void)
{
	unsigned char SL_Read_Reg;
	//---------- 中断1配置 ------------
	SL_Read_Reg=0x00;            //0xC8
	SL_Read_Reg=SL_Read_Reg|0x20;//Z轴高
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_AOI1_CFG, SL_Read_Reg);
	//中断阈值设置
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_AOI1_THS, SL_SC7A20_INT_THS_20PERCENT);
	//大于阈值多少时间触发中断
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_AOI1_DURATION, SL_SC7A20_INT_DURATION_5CLK);
	//---------- 中断2配置 ------------
	SL_Read_Reg=0x00;            //0xC8
	//SL_Read_Reg=SL_Read_Reg|0x40;//方向位置识别模式
	//修改08即可切换到任意轴朝上的情况
	SL_Read_Reg=SL_Read_Reg|0x20;//Z轴高
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_AOI2_CFG, SL_Read_Reg);
	//中断阈值设置
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_AOI2_THS, SL_SC7A20_INT_THS_80PERCENT);
	//大于阈值多少时间触发中断
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_AOI2_DURATION, SL_SC7A20_INT_DURATION_10CLK);
	
	//-----------
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG5,&SL_Read_Reg, 1);
	SL_Read_Reg=SL_Read_Reg&0xFD;//AOI2 NO LATCH   
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_AOI1_CFG, SL_Read_Reg);
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG5, SL_Read_Reg);
	
	//H_LACTIVE SET	    AOI2 TO INT2		INT2 ON
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG6,&SL_Read_Reg, 1);
	SL_Read_Reg=SL_SC7A20_INT_ACTIVE_LOWER_LEVEL|SL_Read_Reg|0x20;
//	SL_Read_Reg=SL_SC7A20_INT_ACTIVE_LOWER_LEVEL|SL_Read_Reg|0x20;  0x40,可能配置问题
	//interrupt happen,int pin output lower level
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG6, SL_Read_Reg);

	//HPF SET
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG2,&SL_Read_Reg, 1);
	SL_Read_Reg=SL_Read_Reg&0xFD;//NO HPF TO AOI2 
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG2, SL_Read_Reg);

	//AOI1 TO INT1
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG3,&SL_Read_Reg, 1);
	SL_Read_Reg=SL_Read_Reg|0x40; //AOI2 TO INT1   AOI1 TO INT1
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG3, SL_Read_Reg);

	return  1;
}

/******外部中断处理函数中需要调用该函数*******/

signed char  SL_SC7A20_INT_RESET(void)
{
	unsigned char SL_Read_Reg1;
	unsigned char SL_Read_Reg2;

	/*****为了避免读取数据过程中又产生中断，可以暂时关闭中断*****/
	//SL_Read_Reg1 display the int1 type
	i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_AOI1_SRC, &SL_Read_Reg1, 1);
	//SL_Read_Reg2 display the int2 type
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_AOI2_SRC,&SL_Read_Reg2, 1);
	
	//close INT1
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_AOI1_CFG, 0x00);
	//close INT2
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_AOI2_CFG, 0x00);
	
	if(SL_Read_Reg1!=0||SL_Read_Reg2!=0)
	{
			return 1;
	}
	else
	{
			return 0;
	}
}

/***************数据更新速率**加速度计使能**********/
signed char  SL_SC7A20_Power_Config(unsigned char Power_Config_Reg)
{
    unsigned char SL_Read_Reg;

#if  SL_SC7A20_MTP_ENABLE == 0X01
    SL_Read_Reg  = 0x00;
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_MTP_CFG, SL_SC7A20_MTP_VALUE);
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_SDOI2C_PU_CFG, &SL_Read_Reg, 1); 
    SL_Read_Reg=SL_Read_Reg|SL_SC7A20_SDO_PU_MSK|SL_SC7A20_I2C_PU_MSK;
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_SDOI2C_PU_CFG, SL_Read_Reg);
#endif
    SL_Read_Reg  = 0xff;
	i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG1, Power_Config_Reg);
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_CTRL_REG1,&SL_Read_Reg, 1);

    if(SL_Read_Reg==Power_Config_Reg)   return  1;
    else                                return -1;
}


#if   SL_SC7A20_16BIT_8BIT==0
/***************加速度计数据读取*8bits*********/
signed char  SL_SC7A20_Read_XYZ_Data(signed char *SL_SC7A20_Data_XYZ_Buf)
{
    unsigned char SL_Read_Buf[7];
    float         SL_Angle[3];
    
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_STATUS_REG, &SL_Read_Buf[0], 1);
    
    if((SL_Read_Buf[0]&0x0f)==0x0f)
    {
#if   SL_SC7A20_SPI_IIC_MODE ==1//IIC
        i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_DATA_OUT, &SL_Read_Buf[1], 6);
#elif SL_SC7A20_SPI_IIC_MODE ==0//SPI
        i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_STATUS_REG, &SL_Read_Buf[0], 7);  
#endif
        SL_SC7A20_Data_XYZ_Buf[0]=(signed char)SL_Read_Buf[2];
        SL_SC7A20_Data_XYZ_Buf[1]=(signed char)SL_Read_Buf[4];
        SL_SC7A20_Data_XYZ_Buf[2]=(signed char)SL_Read_Buf[6];    

#if 0
        SL_Angle[0]=(float)SL_SC7A20_Data_XYZ_Buf[0]/64;
        SL_Angle[1]=(float)SL_SC7A20_Data_XYZ_Buf[1]/64;       
        SL_Angle[2]=(float)SL_SC7A20_Data_XYZ_Buf[2]/64;
        
        if(SL_Angle[0]>1.0)    SL_Angle[0]= 1.0;
        if(SL_Angle[0]<-1.0)   SL_Angle[0]=-1.0;
        if(SL_Angle[1]>1.0)    SL_Angle[1]= 1.0;
        if(SL_Angle[1]<-1.0)   SL_Angle[1]=-1.0;
        if(SL_Angle[2]>1.0)    SL_Angle[2]= 1.0;
        if(SL_Angle[2]<-1.0)   SL_Angle[2]=-1.0;
        
        SL_Angle[0] = asinf(SL_Angle[0])*57.32484;
				SL_Angle[1] = asinf(SL_Angle[1])*57.32484;//y angle
				SL_Angle[2] = asinf(SL_Angle[2])*57.32484;

        SL_SC7A20_Data_XYZ_Buf[2]=SL_Angle[1];
#endif       
        return  1;
    }
    else
    {
        return 0;
    }
}
#elif SL_SC7A20_16BIT_8BIT==1
/***************加速度计数据读取*16bits*********/

#define SL_SC7A20EE_POS_SEL  5
//??SC7A22??????????
const signed char accel_pos_change[8][3][3]=
{//x       y        z 
{{1,0,0}, {0,0,-1},{0,1,0}},//0???????
{{0,1,0},{0,0,-1},{-1,0,0}},
{{-1,0,0},{0,0,-1},{0,-1,0}},
{{0,-1,0}, {0,0,-1},{1,0,0}},

{{-1,0,0}, {0,0,1}, {0,1,0}},//4???????
{{0,1,0}, {0,0,1},{1,0,0}},
{{1,0,0},{0,0,1},{0,-1,0}},
{{0,-1,0},{0,0,1},{-1,0,0}}
};

#define SL_Filter_Order 16
static signed short  SL_Filter_BUF[3][SL_Filter_Order]={{0},{0},{0}};
static unsigned char SL_Filter_i                      =0;
/**used to solid signal**/
static signed short  SL_XYZ_AVG[3]                   ={0,0,0};
/*		滑动窗口滤波				*/
static void SL_SC7A20E_Filter(signed short *Input1_s16,signed short *Output1_s16,unsigned char axis)
{
	// 1. 更新累加和：减去最早的值，加上新输入值
    SL_XYZ_AVG[axis]=SL_XYZ_AVG[axis]+Input1_s16[axis]-SL_Filter_BUF[axis][SL_Filter_i];
    SL_Filter_BUF[axis][SL_Filter_i]=Input1_s16[axis];
    Output1_s16[axis]=SL_XYZ_AVG[axis];
	SL_Filter_i = (SL_Filter_i + 1) % SL_Filter_Order;
}

signed char  SL_SC7A20_Read_XYZ_Data(signed short *SL_SC7A20_Data_XYZ_Buf)
{
    unsigned char SL_Read_Buf[7];
    float         SL_Angle[3];
	float roll,pitch;
	
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_STATUS_REG,&SL_Read_Buf[0], 1);
    
    if((SL_Read_Buf[0]&0x0f)==0x0f)
    {
#if   SL_SC7A20_SPI_IIC_MODE ==1//IIC
        i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_DATA_OUT, &SL_Read_Buf[1], 6);
#elif SL_SC7A20_SPI_IIC_MODE ==0//SPI
        i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_STATUS_REG, &SL_Read_Buf[0], 7);  
#endif   
        SL_SC7A20_Data_XYZ_Buf[0]=(signed short)((SL_Read_Buf[2]<<8) + SL_Read_Buf[1]);
        SL_SC7A20_Data_XYZ_Buf[1]=(signed short)((SL_Read_Buf[4]<<8) + SL_Read_Buf[3]);
        SL_SC7A20_Data_XYZ_Buf[2]=(signed short)((SL_Read_Buf[6]<<8) + SL_Read_Buf[5]);
        
#if 1
		SL_SC7A20E_Filter(&SL_SC7A20_Data_XYZ_Buf[0],&SL_SC7A20_Data_XYZ_Buf[0],0);
		SL_SC7A20E_Filter(&SL_SC7A20_Data_XYZ_Buf[1],&SL_SC7A20_Data_XYZ_Buf[1],1);
		SL_SC7A20E_Filter(&SL_SC7A20_Data_XYZ_Buf[2],&SL_SC7A20_Data_XYZ_Buf[2],2);
		SL_Angle[0]=SL_SC7A20_Data_XYZ_Buf[0];
		SL_Angle[1]=SL_SC7A20_Data_XYZ_Buf[1];
		SL_Angle[2]=SL_SC7A20_Data_XYZ_Buf[2];	    
		
		SL_Angle[0]=(float)SL_Angle[0]/16384;			//16个数据的滤波  *  2G的1024
		SL_Angle[1]=(float)SL_Angle[1]/16384;
		SL_Angle[2]=(float)SL_Angle[2]/16384;
		if(SL_Angle[0]>1.0)    SL_Angle[0]= 1.0;
		if(SL_Angle[0]<-1.0)   SL_Angle[0]=-1.0;
		if(SL_Angle[1]>1.0)    SL_Angle[1]= 1.0;
		if(SL_Angle[1]<-1.0)   SL_Angle[1]=-1.0;
		if(SL_Angle[2]>1.0)    SL_Angle[2]= 1.0;
		if(SL_Angle[2]<-1.0)   SL_Angle[2]=-1.0;
		
		//z轴朝上
		if(SL_Angle[2]!=0)
			pitch= atanf(SL_Angle[1]/SL_Angle[2])*57.32484;
		else
			pitch= 90;
		if(SL_Angle[1]!=0||SL_Angle[2]!=0)
			roll = -atanf(SL_Angle[0]/sqrt(SL_Angle[2]*SL_Angle[2]+SL_Angle[1]*SL_Angle[1]))*57.32484;
		else
			roll = 90;
		
		//x轴朝上
//		if(SL_Angle[0]!=0)
//			pitch= atanf(SL_Angle[2]/SL_Angle[0])*57.32484;
//		else
//			pitch= 90;
//		if(SL_Angle[0]!=0||SL_Angle[2]!=0)
//			roll = -atanf(SL_Angle[1]/sqrt(SL_Angle[0]*SL_Angle[0]+SL_Angle[2]*SL_Angle[2]))*57.32484;
//		else
//			roll = 90;
		
		//y轴朝上
//		if(SL_Angle[1]!=0)
//			pitch= atanf(SL_Angle[0]/SL_Angle[1])*57.32484;
//		else
//			pitch= 90;
//		if(SL_Angle[0]!=0||SL_Angle[1]!=0)
//			roll = -atanf(SL_Angle[2]/sqrt(SL_Angle[0]*SL_Angle[0]+SL_Angle[1]*SL_Angle[1]))*57.32484;
//		else
//			roll = 90;
		
		SL_SC7A20_Data_XYZ_Buf[0]=(signed short)roll;
		SL_SC7A20_Data_XYZ_Buf[1]=(signed short)pitch;	
#endif 
        
        return  1;
    }
    else
    {
        return 0;
    }
}

signed char  SL_SC7A20_Read_XYZ_Data1(signed short *SL_SC7A20_Data_XYZ_Buf)
{
    unsigned char SL_Read_Buf[7],sl_i=0;
    float         SL_Angle[3];
	
		float roll,pitch;
    
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD,SL_SC7A20_STATUS_REG,&SL_Read_Buf[0], 1);
    
    if((SL_Read_Buf[0]&0x0f)==0x0f)
    {
#if   SL_SC7A20_SPI_IIC_MODE ==1//IIC
        i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_DATA_OUT, &SL_Read_Buf[1], 6);
#elif SL_SC7A20_SPI_IIC_MODE ==0//SPI
        i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, SL_SC7A20_STATUS_REG, &SL_Read_Buf[0], 7);  
#endif
        SL_SC7A20_Data_XYZ_Buf[0]=(signed short)((SL_Read_Buf[2]<<8) + SL_Read_Buf[1]);
        SL_SC7A20_Data_XYZ_Buf[1]=(signed short)((SL_Read_Buf[4]<<8) + SL_Read_Buf[3]);
        SL_SC7A20_Data_XYZ_Buf[2]=(signed short)((SL_Read_Buf[6]<<8) + SL_Read_Buf[5]);
						
				//新计算方法
			SL_SC7A20_Data_XYZ_Buf[1]=(signed short)((SL_Read_Buf[2]<<8) + SL_Read_Buf[1]);
			SL_SC7A20_Data_XYZ_Buf[2]=(signed short)((SL_Read_Buf[4]<<8) + SL_Read_Buf[3]);
			SL_SC7A20_Data_XYZ_Buf[0]=(signed short)((SL_Read_Buf[6]<<8) + SL_Read_Buf[5]);
		
			SL_SC7A20_Data_XYZ_Buf[0]=SL_SC7A20_Data_XYZ_Buf[0];
			SL_SC7A20_Data_XYZ_Buf[1]=SL_SC7A20_Data_XYZ_Buf[1]+500;			//消除传感器的零漂误差
			SL_SC7A20_Data_XYZ_Buf[2]=SL_SC7A20_Data_XYZ_Buf[2]+1500;
			//坐标变换
			for(sl_i=0;sl_i<3;sl_i++)
			{
				SL_Angle[sl_i]=\
				accel_pos_change[SL_SC7A20EE_POS_SEL][sl_i][0]*SL_SC7A20_Data_XYZ_Buf[0]+\
				accel_pos_change[SL_SC7A20EE_POS_SEL][sl_i][1]*SL_SC7A20_Data_XYZ_Buf[1]+\
				accel_pos_change[SL_SC7A20EE_POS_SEL][sl_i][2]*SL_SC7A20_Data_XYZ_Buf[2];
			}
			SL_SC7A20_Data_XYZ_Buf[0]=SL_Angle[0];
			SL_SC7A20_Data_XYZ_Buf[1]=SL_Angle[1];
			SL_SC7A20_Data_XYZ_Buf[2]=SL_Angle[2];
		
			SL_SC7A20E_Filter(&SL_SC7A20_Data_XYZ_Buf[0],&SL_SC7A20_Data_XYZ_Buf[0],0);
			SL_SC7A20E_Filter(&SL_SC7A20_Data_XYZ_Buf[1],&SL_SC7A20_Data_XYZ_Buf[1],1);
			SL_SC7A20E_Filter(&SL_SC7A20_Data_XYZ_Buf[2],&SL_SC7A20_Data_XYZ_Buf[2],2);
			SL_Angle[0]=SL_SC7A20_Data_XYZ_Buf[0];
			SL_Angle[1]=SL_SC7A20_Data_XYZ_Buf[1];
			SL_Angle[2]=SL_SC7A20_Data_XYZ_Buf[2];
			
			
			
#if 1
        
        SL_Angle[0]=(float)SL_Angle[0]/16384;
        SL_Angle[1]=(float)SL_Angle[1]/16384;
        SL_Angle[2]=(float)SL_Angle[2]/16384;
        if(SL_Angle[0]>1.0)    SL_Angle[0]= 1.0;
        if(SL_Angle[0]<-1.0)   SL_Angle[0]=-1.0;
        if(SL_Angle[1]>1.0)    SL_Angle[1]= 1.0;
        if(SL_Angle[1]<-1.0)   SL_Angle[1]=-1.0;
        if(SL_Angle[2]>1.0)    SL_Angle[2]= 1.0;
        if(SL_Angle[2]<-1.0)   SL_Angle[2]=-1.0;
        
				if(SL_Angle[2]!=0)
					pitch= atanf(SL_Angle[1]/SL_Angle[2])*57.32484f;
				else
					pitch= 90;
				if(SL_Angle[1]!=0||SL_Angle[2]!=0)
					roll = -atanf(SL_Angle[0]/sqrt(SL_Angle[2]*SL_Angle[2]+SL_Angle[1]*SL_Angle[1]))*57.32484f;
				else
					roll = 90;

				SL_SC7A20_Data_XYZ_Buf[0]=(signed short)roll;
        SL_SC7A20_Data_XYZ_Buf[1]=(signed short)pitch;
        SL_SC7A20_Data_XYZ_Buf[2]=(signed short)SL_Angle[2];
#endif 
        
        return  1;
    }
    else{
      return 0;
    }
}
#endif


//////////////////////////////////////////////////////////////////
// 设备地址与寄存器地址（文档P5、P12、P14）
#define CTRL_REG1         0x20        // 控制寄存器1
#define CTRL_REG3         0x22        // 中断路由寄存器
#define CTRL_REG4         0x23        // 量程与精度寄存器
#define INT1_CFG          0x30        // XYZ阈值中断使能寄存器
#define INT1_THS          0x32        // 中断阈值寄存器
#define INT1_DURATION     0x33        // 中断持续时间寄存器
#define CTRL_REG6         0x25        // 中断极性寄存器
#define CTRL_REG2     	  0x21        // 
#define CLICK_CFG     	  0x38        // 
#define CLICK_THS     	  0x3A        // 
#define TIME_LIMIT    	  0x3B        // 
#define TIME_LATENCY 	  0x3C        // 
#define TIME_WINDOW   	  0x3D        // 
#define CTRL_REG5    	  0x24        // 
/**
 * @brief  三轴检测加检查完成中断触发
 * @param 
 * @return 0-成功，1-数据未就绪，2-IIC读取失败
 */
uint8_t reg_ok[11];
uint8_t SC7A20_Init(void) {
    uint8_t ret;

    // 1. 基础配置（12位精度、100Hz输出率、三轴使能）
    i2c_write_reg(&SC7A20_i2c,PERIPH_SC7A20_ADD, CTRL_REG1, 0x47);  // ODR=100Hz，XYZ使能
//    if (ret != 0) 
//		return 1;
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD, CTRL_REG1,reg_ok, 1);
    i2c_write_reg(&SC7A20_i2c,PERIPH_SC7A20_ADD, CTRL_REG4, 0x88);  // HR=1（12位），±2G，BDU=1
//    if (ret != 0) 
//		return 1;
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD, CTRL_REG4,reg_ok+1, 1);
    // 2. 配置DRY信号路由到INT1（文档2.2节）
    // CTRL_REG3（22h）：I1_DRDY1=1（B4位），DRDY信号输出到INT1
    i2c_write_reg(&SC7A20_i2c,PERIPH_SC7A20_ADD, CTRL_REG3, 0x10);  // 0b00010000
//    if (ret != 0) 
//		return 1;
	i2c_read_bytes(&SC7A20_i2c,PERIPH_SC7A20_ADD, CTRL_REG3,reg_ok+2, 1);

    // 3. 配置INT1中断极性（低电平有效）
    // CTRL_REG6（25h）：H_LACTIVE=1（B1位），INT1低电平表示数据就绪
    i2c_write_reg(&SC7A20_i2c,PERIPH_SC7A20_ADD, CTRL_REG6, 0x02);  // 0b00000010
//    if (ret != 0) 
//		return 1;
	i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG6,reg_ok+3, 1);

    return 0;
}



/**三轴阈值中断触发
 * @brief  三轴检测加
 * @param 
 * @return 0-成功，1-数据未就绪，2-IIC读取失败
 */
uint8_t SC7A20_Init_With_ThresholdINT(void) {
    uint8_t ret;

    // 1. 基础配置：±2G量程、12bit精度、100Hz输出率（文档P2、P13）
    i2c_write_reg(&SC7A20_i2c,PERIPH_SC7A20_ADD, CTRL_REG1, 0x27);  // ODR=100Hz，XYZ轴使能

    i2c_write_reg(&SC7A20_i2c,PERIPH_SC7A20_ADD, CTRL_REG4, 0x88);  // HR=1（12bit），BDU=1，±2G
    

    // 2. XYZ轴阈值中断使能（INT1_CFG，文档P17）
    // 使能X高、X低、Y高、Y低、Z高、Z低事件中断（B5-B0=1）
    i2c_write_reg(&SC7A20_i2c,PERIPH_SC7A20_ADD, INT1_CFG, 0x1A);    // 0b00011010


    // 3. 设置中断阈值（INT1_THS，文档P18）
    // 阈值=50mg（±2G量程HR模式，灵敏度1mg/digit，50→0x32）
	//按照百分比算的，比如2G ，0x1A/128 = 20.3%    2000*20.3% = 406; 
	//是绝对值范围 < -406 ,-406 < x <406, 406 < 
	//0x1A * 16 = 416
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, INT1_THS, 0x1A);   

    // 4. 设置中断持续时间（INT1_DURATION，文档P18）
    // 持续2个ODR周期（100Hz→20ms，避免噪声误触发）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, INT1_DURATION, 0x02);

    // 5. 路由中断到INT1引脚（CTRL_REG3，文档P14）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG3, 0x40);    // B6(I1_AOI1)=1，AOI1中断到INT1

    // 6. （可选）设置INT1低电平有效（CTRL_REG6，文档P15）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG6, 0x02);    // B1(H_LACTIVE)=1

    return 0;
}

/**
 * @brief  三轴检测加双击功能中断触发
 * @param 
 * @return 0-成功，1-数据未就绪，2-IIC读取失败
 */
uint8_t SC7A20_Click_Init(void) {
    uint8_t ret;

    // 1. 基础配置（12位精度、100Hz输出率、三轴使能）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG1, 0x37);  // ODR=100Hz，XYZ使能    57
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG1, reg_ok, 1);

    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG4, 0x88);  // HR=1（12位），±2G，BDU=1
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG4, reg_ok+1, 1);

    // 2. 配置DRY信号路由到INT1
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG3, 0x10);  // 新增：I1_CLICK=1（双击中断到INT1）+ I1_DRDY1=1
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG3, reg_ok+2, 1);

    // 3. 配置INT1中断极性（低电平有效）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG6, 0x82);  // H_LACTIVE=1（低电平有效）
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG6, reg_ok+3, 1);

    // 4. 配置高通滤波（用于双击识别）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG2, 0xBC);  // 高通模式+滤波使能
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG2, reg_ok+4, 1);

    // 5. 双击功能核心配置
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CLICK_CFG, 0x2A);   // Z轴双击使能
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CLICK_CFG, reg_ok+5, 1);

    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CLICK_THS, 0x03);   // 双击阈值=6mg（±2G量程下）
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CLICK_THS, reg_ok+6, 1);

    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, TIME_LIMIT, 0x08);  // 单次点击时间窗口=8*40ms=320ms
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, TIME_LIMIT, reg_ok+7, 1);

    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, TIME_LATENCY, 0x1F); // 双击间隔延迟=2*40ms=20ms
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, TIME_LATENCY, reg_ok+8, 1);

    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, TIME_WINDOW, 0x1F);  // 双击检测窗口=31*40ms=310ms
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, TIME_WINDOW, reg_ok+9, 1);

    // 6. 锁存中断信号（避免漏读）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG5, 0x02);    // LIR_INT2=1（锁存INT1中断）
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG5, reg_ok+10, 1);

    return 0;
}
/**
 * @brief  三轴检测加自由下落中断触发
 * @param 
 * @return 0-成功，1-数据未就绪，2-IIC读取失败
 */
uint8_t SC7A20_Fall_Init(void) {
    uint8_t ret;

    // 1. 基础配置（12位精度、100Hz输出率、三轴使能）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG1, 0x47);  // ODR=50Hz，XYZ使能
    
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG1, reg_ok, 1);

    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG4, 0x88);  // HR=1（12位），±2G，BDU=1
 
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG4, reg_ok+1, 1);

    // 2. 配置DRY信号路由到INT1（保留原有配置）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG3, 0x40);  // I1_DRDY1=1
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG3, reg_ok+2, 1);

    // 3. 配置INT1中断极性（保留原有配置）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG6, 0x02);  // 新增：I2_AOI2=1（中断2路由到INT2）+ H_LACTIVE=1（低电平有效）
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, CTRL_REG6, reg_ok+3, 1);

    // 4. 自由落体触发中断2核心配置
    // 4.1 配置中断2源：XYZ轴低事件"与"触发（自由落体）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x30, 0x95);  // INT2_CFG（34h）：AOI=1（与），ZLIE=1, YLIE=1, XLIE=1
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x34, reg_ok+4, 1);

    // 4.2 设置自由落体阈值（±2G量程下，0x1F对应31*16mg=496mg，三轴均低于此值触发）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x32, 0x10);  // INT2_THS（36h）：阈值=0x1F
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x36, reg_ok+5, 1);

    // 4.3 设置持续时间（5个ODR周期，100Hz下为50ms，避免瞬时抖动）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x37, 0x05);  // INT2_DURATION（37h）：持续时间=5
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x37, reg_ok+6, 1);

    // 4.4 锁存中断2信号（避免漏读）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x24, 0x08);  // CTRL_REG5（24h）：LIR_INT2=1（B1位）
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x24, reg_ok+7, 1);

    return 0;
}

uint8_t SC7A20_Lift_Init(void) {
    uint8_t ret;
    uint8_t reg_ok[8] = {0};  // 存储读回的寄存器值，用于配置验证

    // 1. 基础配置：100Hz输出率、XYZ轴使能、正常工作模式
    // CTRL_REG1(20h)配置：ODR3-0=0101(100Hz)，LPen=0(正常模式)，Zen=1/Yen=1/Xen=1(三轴使能)
    // 文档依据：ODR配置见1-236（0101对应100Hz），CTRL_REG1位定义见1-227/1-228
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x20, 0x57);  // 0x57 = 0101 0111
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x20, reg_ok, 1);

    // 2. 精度与量程配置：12位数据、±2G量程、块数据更新（避免读数中途更新）
    // CTRL_REG4(23h)配置：BDU=1(位7，数据不更新直到LSB/MSB读完)，FS1-0=00(±2G)，其他默认
    // 文档依据：CTRL_REG4位定义见1-254/1-255，±2G量程见1-14/1-40
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x23, 0x88);  // 0x88 = 1000 1000
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x23, reg_ok+1, 1);

    // 3. 中断路由配置：将Z轴高事件（AOI1）路由到INT1管脚
    // CTRL_REG3(22h)配置：I1_AOI1=1(中断1的AOI事件输出到INT1)，其他中断源禁用
    // 文档依据：CTRL_REG3位定义见1-251/1-252（I1_AOI1对应位6）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x22, 0x40);  // 0x40 = 0100 0000
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x22, reg_ok+2, 1);

    // 4. 中断极性配置：INT1低电平有效（避免悬空误触发）
    // CTRL_REG6(25h)配置：H_LACTIVE=1(位1，中断触发输出低电平)，其他默认
    // 文档依据：CTRL_REG6位定义见1-274（H_LACTIVE对应位1）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x25, 0x02);  // 0x02 = 0000 0010
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x25, reg_ok+3, 1);

    // 5. 中断源配置：Z轴高事件触发（仅Z轴高，其他轴禁用）
    // INT1_CFG(30h)配置：AOI=0(或事件，单轴触发即可)，6D=0(非方向检测)，ZHIE=1(Z轴高事件使能)
    // 文档依据：中断1配置寄存器位定义见1-310（AOI=0、6D=0、ZHIE=1对应位7=0/位6=0/位5=1）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x30, 0x20);  // 0x20 = 0010 0000
        // ret = i2c_write_reg(PERIPH_SC7A20_ADD, 0x30, 0x10);  // 0x20 = 0010 0000
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x30, reg_ok+4, 1);

    // 6. 上提阈值配置：±2G量程下，阈值=1024mg（1LSB=16mg，0x40=64*16=1024mg）
    // 静置时Z轴为1000mg（1G），超过1024mg判定为“上提”，避免振动误触发
    // 文档依据：中断1阈值寄存器定义见1-322（FS=2G时1LSB=16mg）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x32, 0x4B);  // 0x40 = 0100 0000
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x32, reg_ok+5, 1);

    // 7. 中断持续时间配置：5个ODR周期（100Hz下为50ms），过滤瞬时抖动
    // INT1_DURATION(33h)配置：D6-D0=0x05（持续5个周期才触发中断）
    // 文档依据：中断1持续时间寄存器定义见1-325（持续时间以ODR为时钟）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x33, 0x05);  // 0x05 = 0000 0101
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x33, reg_ok+6, 1);

    // 8. 中断锁存配置：锁存INT1信号（避免中断漏读，读状态寄存器后自动清除）
    // CTRL_REG5(24h)配置：LIR_INT1=1(位3，锁存中断1信号)
    // 文档依据：CTRL_REG5位定义见1-267（LIR_INT1对应位3）
    i2c_write_reg(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x24, 0x08);  // 0x08 = 0000 1000
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x24, reg_ok+7, 1);
    if (reg_ok[7] != 0x08) return 2;

    // 新增：清除初始化后的残留中断锁存
    uint8_t dummy;
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x31, &dummy, 1);
    return 0;  // 所有配置完成且验证通过
}

void SC7A20_EXTI_Configure(void)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    RCC_AHBPeriphClockCmd(RCC_AHBENR_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFG, ENABLE);

    /* K1->PC1->EXTI_Line1 */
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource7);

    EXTI_StructInit(&EXTI_InitStruct);
    EXTI_InitStruct.EXTI_Line    = EXTI_Line7;
    EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    /* EXTI Interrupt */
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_15_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0x01;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/*					应用层					*/
// 数据寄存器起始地址（文档P12/16）
#define OUT_X_L 0x28  // X轴低字节
#define STATUS_REG 0x27  // 状态寄存器

/**
 * @brief  读取SC7A20三轴加速度数据（12位精度）
 * @param  x: X轴数据指针（输出，单位：mg，1LSB=1mg@±2G HR模式）
 * @param  y: Y轴数据指针
 * @param  z: Z轴数据指针
 * @return 0-成功，1-数据未就绪，2-IIC读取失败
 */
uint8_t SC7A20_ReadXYZ(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t status;
    uint8_t raw_data[6];  // 存储X_L、X_H、Y_L、Y_H、Z_L、Z_H

    // 1. 读取状态寄存器，检查数据是否就绪（ZYXDA位=1，文档P16）
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, STATUS_REG, &status, 1);
    if ((status & 0x0F) == 0) {  // ZYXDA=0，数据未就绪
        return 1;
    }

    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x28, raw_data, 1);
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x29, raw_data+1, 1);
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x2A, raw_data+2, 1);
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x2B, raw_data+3, 1);
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x2C, raw_data+4, 1);
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x2D, raw_data+5, 1);

    // 3. 拼接12位有效数据（小端模式，右移4位提取高12位，文档P5/2.4节）
	
		*x = raw_data[1];
		*x <<= 8;
		*x |= raw_data[0];
		*x >>= 4;	//取12bit精度
 
		//Y轴
		*y = raw_data[3];
		*y <<= 8;
		*y |= raw_data[2];
		*y >>= 4;	//取12bit精度
		
		//Z轴
		*z = raw_data[5];
		*z <<= 8;
		*z |= raw_data[4];
		*z >>= 4;	//取12bit精度

    return 0;  // 读取成功
}
 
void read_accelerometer_data(void) {
	i2c_bus_init(&SC7A20_i2c);
    uint8_t result;  // 用于存储函数返回值
    SC7A20_Init();

    static uint8_t angle_out_times = 0;
    // 静态变量定义在函数作用域顶部，确保两个代码块共享同一个状态标志
    static uint8_t zangle_triggered = 0;
    // 调用SC7A20_ReadXYZ函数读取加速度数据

    result = SC7A20_ReadXYZ(&sl_sc7A20.gravity_x, &sl_sc7A20.gravity_y, &sl_sc7A20.gravity_z);
    SC7A20_GetZAxisAngle(sl_sc7A20.gravity_x, sl_sc7A20.gravity_y, sl_sc7A20.gravity_z);
    
    // 根据返回值进行不同处理
    switch (result) {
        case 0:  // 读取成功
            ulog_i("ACC", "加速度数据读取成功！\r\n");
            ulog_i("ACC", "X轴加速度: %d\r\n", sl_sc7A20.gravity_x);
            ulog_i("ACC", "Y轴加速度: %d\r\n", sl_sc7A20.gravity_y);
            ulog_i("ACC", "Z轴加速度: %d\r\n", sl_sc7A20.gravity_z);
            ulog_i("ACC", "Z轴倾角: %d 度\r\n", sl_sc7A20.z_angle);
		
            // 修正逻辑：result == 0 表示成功，返回 TRUE
         
            
            break;
        case 1:  // 数据未就绪
            ulog_w("ACC", "加速度数据未就绪，请稍后再试。\r\n");
          
            break;
        case 2:  // IIC读取失败
            ulog_e("ACC", "IIC通信失败，请检查硬件连接。\r\n");
           
            break;
        default:
            ulog_e("ACC", "未知错误: %d\r\n", result);
        
            break;
    }
  
}

MSH_CMD_EXPORT(read_accelerometer_data,test SCA720);

#define PI 3.1415926535898
/*************************************************************************************************************************
*函数        	:	bool SC7A20_GetZAxisAngle(SC7A20_HANDLE* pHandle, s16 AcceBuff[3], float* pAngleZ)
*功能        	:	SC7A20 获取Z轴倾角
*参数        	:	pHandle:句柄；AcceBuff:3个轴的加速度；pAngleZ：Z方向倾角
*返回        	:	TRUE:成功；FALSE:失败
*依赖			: 	底层宏定义
*时间     		:	2022-07-02
*最后修改时间		:	2022-07-02
*说明        	:
*************************************************************************************************************************/
void SC7A20_GetZAxisAngle(int16_t Xa, int16_t Ya, int16_t Za)
{
	double fx, fy, fz;
	double A;
 
//	if(Za < 0) 
//	{
//		sl_sc7A20.z_angle = 90;
//		return;
//	}
	fx = Xa;
	fx *= 2.0 / 4096;
	fy = Ya;
	fy *= 2.0 / 4096;
	fz = Za;
	fz *= 2.0 / 4096;
 
	//uart_printf("fx：%.04f\tfy：%.04f\tfz：%.04f\t\r\n",fx,fy,fz);
 
	//Z方向
	A = fx * fx + fy * fy;
	A = sqrt(A);
	A = (double)A / fz;
	A = atan(A);
	A = A * 180 / PI;
	
	sl_sc7A20.z_angle = A;
}



/**
 * @brief 打开SC7A20的外部中断
 * @note 用于重新启用之前被禁用的SC7A20外部中断
 */
void SC7A20_EXTI_Enable(void)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    // 配置EXTI线7
    EXTI_StructInit(&EXTI_InitStruct);
    EXTI_InitStruct.EXTI_Line    = EXTI_Line7;
    EXTI_InitStruct.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
    
    // 配置NVIC
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_15_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPriority = 0x01;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    // 新增：清除初始化后的残留中断锁存
    uint8_t dummy;
    i2c_read_bytes(&SC7A20_i2c, PERIPH_SC7A20_ADD, 0x31, &dummy, 1);

}

/**
 * @brief 关闭SC7A20的外部中断
 * @note 用于临时禁用SC7A20的外部中断
 */
void SC7A20_EXTI_Disable(void)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    // 禁用EXTI线7
    EXTI_StructInit(&EXTI_InitStruct);
    EXTI_InitStruct.EXTI_Line    = EXTI_Line7;
    EXTI_InitStruct.EXTI_LineCmd = DISABLE;
    EXTI_Init(&EXTI_InitStruct);
    
    // 禁用NVIC中断通道
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_15_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStruct);
}

/**
 * @brief 切换SC7A20的外部中断状态
 * @param enable: TRUE-启用中断，FALSE-禁用中断
 */
void SC7A20_EXTI_SetState(BOOL enable)
{
    if (enable)
    {
        SC7A20_EXTI_Enable();
    }
    else
    {
        SC7A20_EXTI_Disable();
    }
}

/**
 * @brief 获取SC7A20的外部中断状态
 * @return BOOL: TRUE-中断已启用，FALSE-中断已禁用
 */
BOOL SC7A20_EXTI_GetState(void)
{
    // 检查EXTI线7的使能状态
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        return TRUE;
    }
    
    // 检查NVIC通道的使能状态
    // 注意：这里无法直接读取NVIC的使能状态，所以返回EXTI的状态
    return FALSE;
}

//---------------------------------------------
// SC7A20 寄存器定义
//---------------------------------------------
#define REG_CTRL_REG1      0x20
#define REG_CTRL_REG2      0x21
#define REG_CTRL_REG3      0x22
#define REG_CTRL_REG5      0x24
#define REG_INT1_CFG       0x30
#define REG_INT1_SRC       0x31
#define REG_INT1_THS       0x32
#define REG_INT1_DURATION  0x33


#define DEG_THRESHOLD     20.0f   // 倾倒判定角度阈值
#define DEG_RECOVER       10.0f   // 恢复阈值
#define DEBOUNCE_MS       200     // 防抖时间（毫秒）

//sc7a20 倾倒检测初始化
void sc7a20_init_tilt_detection(i2c_bus_t *bus)
{
  // 复位
    i2c_write_reg(bus, PERIPH_SC7A20_ADD, REG_CTRL_REG2, 0x80);
    rt_thread_mdelay(10);

    // 1️⃣ ODR=25Hz, 使能XYZ
    i2c_write_reg(bus, PERIPH_SC7A20_ADD, REG_CTRL_REG1, 0x27);

    // 2️⃣ 路由 AOI1 -> INT1
    i2c_write_reg(bus, PERIPH_SC7A20_ADD, REG_CTRL_REG3, 0x40);

    // 3️⃣ 启用 latch
    i2c_write_reg(bus, PERIPH_SC7A20_ADD, REG_CTRL_REG5, 0x08);

    // 4️⃣ 6D 模式 (AOI=1,6D=1, XH/YH/ZH检测)
    i2c_write_reg(bus, PERIPH_SC7A20_ADD, REG_INT1_CFG, 0x2A);

    // 5️⃣ 阈值和持续时间
    i2c_write_reg(bus, PERIPH_SC7A20_ADD, REG_INT1_THS, 0x08);
    i2c_write_reg(bus, PERIPH_SC7A20_ADD, REG_INT1_DURATION, 0x05);

    rt_kprintf("[SC7A20] 6D tilt interrupt initialized.\n");
}


/**
 * @brief   计算与Z轴的夹角（单位°）
 * @param   dev SC7A20 结构体（包含 x/y/z 原始值）
 * @return  倾角（单位°）
 */
static float SC7A20_GetZAxisAngle2(SL_SC7A20_t *dev)
{
    // 转换为浮点
    float gx = (float)(dev->gravity_x);
    float gy = (float)(dev->gravity_y);
    float gz = (float)(dev->gravity_z);

    // 右移 4 位对齐（12-bit 输出）
    gx /= 16.0f;
    gy /= 16.0f;
    gz /= 16.0f;

    // 计算模长（归一化）
    float gnorm = sqrtf(gx * gx + gy * gy + gz * gz);
    if (gnorm < 1e-6f) gnorm = 1.0f;

    // 与Z轴夹角
    float cos_theta = gz / gnorm;

    // 限幅 [-1, 1]
    if (cos_theta > 1.0f)  cos_theta = 1.0f;
    if (cos_theta < -1.0f) cos_theta = -1.0f;

    // 计算角度（取绝对值 → 不区分正反）
    float angle_rad = acosf(fabsf(cos_theta));
    float angle_deg = angle_rad * 180.0f / 3.1415926536f;

    return angle_deg;
}

static inline uint32_t now_ms(void)
{
    return (uint32_t)(rt_tick_get() * 1000 / RT_TICK_PER_SECOND); // RT_TICK_PER_SECOND=1000 -> 已是ms
}

/**
 * @brief   根据Z轴角度判断倾倒状态（更新结构体）
 * @param   dev SC7A20 数据结构体（包含X/Y/Z）
 */
void SC7A20_UpdateTiltState(SL_SC7A20_t *dev)
{
    SC7A20_ReadXYZ(&dev->gravity_x, &dev->gravity_y, &dev->gravity_z);
    // 计算角度
    dev->angle_deg = SC7A20_GetZAxisAngle2(dev);
    dev->z_angle   = (uint16_t)(dev->angle_deg * 100.0f);  // 保留两位小数 *100

    uint32_t now = now_ms(); 

    if (!dev->tilted)
    {
        // 检测是否进入倾倒
        if (dev->angle_deg > DEG_THRESHOLD)
        {
            if (dev->last_change_tick == 0)
                dev->last_change_tick = now;
            else if (now - dev->last_change_tick > DEBOUNCE_MS)
            {
                dev->tilted = true;
                dev->last_change_tick = now;
                rt_kprintf("[SC7A20] Tilt detected! angle=%.1f°\n", dev->angle_deg);
            }
        }
        else
        {
            dev->last_change_tick = 0;
        }
    }
    else
    {
        // 检测是否恢复直立
        if (dev->angle_deg < DEG_RECOVER)
        {
            if (dev->last_change_tick == 0)
                dev->last_change_tick = now;
            else if (now - dev->last_change_tick > DEBOUNCE_MS)
            {
                dev->tilted = false;
                dev->last_change_tick = now;
                rt_kprintf("[SC7A20] Recovered. angle=%.1f°\n", dev->angle_deg);
            }
        }
        else
        {
            dev->last_change_tick = 0;
        }
    }
}

void thread_sc7a20_task_entry(void *parameter)
{
    i2c_bus_init(&SC7A20_i2c);
    // SC7A20_Init();
    sc7a20_init_tilt_detection(&SC7A20_i2c);

    while (1)
    {
        SC7A20_UpdateTiltState(&sl_sc7A20);
        rt_thread_mdelay(100); // 每100ms更新一次
    }
}
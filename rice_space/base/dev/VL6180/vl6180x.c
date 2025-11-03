#include "vl6180x.h"    
#include "platform.h"
#include "ulog.h"
#define addr_write 0X29
#define addr_read  0x29
/*
  初始化GPIO_0，GPIO_1端口
  并置输出为高电平
*/
//void VL6180X_GPIO_Init()
//{
//    GPIO_InitTypeDef  GPIO_InitStructure;
//	/*初始化GPIO1*/
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
//    GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;
//	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	GPIO_SetBits(GPIOB,GPIO_Pin_9);
//	/*初始化GPIO0*/
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
//	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;
//	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
//    GPIO_Init(GPIOC, &GPIO_InitStructure);
//    GPIO_SetBits(GPIOC,GPIO_Pin_13);
//}

extern i2c_bus_t i2c1;
i2c_bus_t i2c3 = {
.scl_port = GPIOA,
.scl_pin  = GPIO_Pin_10,
.sda_port = GPIOA,
.sda_pin  = GPIO_Pin_9,
.delay_us = default_delay_us,
};

/*
   参数：reg 寄存器地址
         data 写入寄存器地址数据
*/
void WriteByte(uint16_t reg,uint8_t data)
{
	i2c_write_bytes(&i2c3, addr_write, (reg>>8)&0xff, (uint8_t[]){reg&0xff, data}, 2);
}

/*
   参数：reg要读取寄存器地址

*/

extern uint8_t i2c_read_reg16(i2c_bus_t *bus, uint8_t dev, uint16_t reg);
  uint8_t ReadByte(uint16_t reg)
{
	uint8_t data=0;
	data=i2c_read_reg16(&i2c3, addr_read, reg);
	return data;
}


/*
   VL6180X初始化
*/
  uint8_t VL6180X_Init()
  {
     uint8_t reset;
	  
//	 GPIO_0=0;
//     delay_ms(10);
//     GPIO_0=1;	
//     delay_ms(1);
	  
	 reset=ReadByte(0x016);//kanshifoufuwei
	 if(reset==1)
	 {
		WriteByte(0X0207,0X01);
		 WriteByte(0X0208,0X01);
		 WriteByte(0X0096,0X00);
		 WriteByte(0X0097,0XFD);
		 WriteByte(0X00E3,0X00);
		 WriteByte(0X00E4,0X04);
		 WriteByte(0X00E5,0X02);
		 WriteByte(0X00E6,0X01);
		 WriteByte(0X00E7,0X03);
		 WriteByte(0X00F5,0X02);
		 WriteByte(0X00D9,0X05);
		 WriteByte(0X00DB,0XCE);
		 WriteByte(0X02DC,0X03);
		 WriteByte(0X00DD,0XF8);
		 WriteByte(0X009F,0X00);
		 WriteByte(0X00A3,0X3C);
		 WriteByte(0X00B7,0X00);
		 WriteByte(0X00BB,0X3C);
		 WriteByte(0X00B2,0X09);
		 WriteByte(0X00CA,0X09);
		 WriteByte(0X0198,0X01);
		 WriteByte(0X01B0,0X17);
		 WriteByte(0X01AD,0X00);
		 WriteByte(0X00FF,0X05);
		 WriteByte(0X0100,0X05);
		 WriteByte(0X0199,0X05);
		 WriteByte(0X01A6,0X1B);
		 WriteByte(0X01AC,0X3E);
		 WriteByte(0X01A7,0X1F);
		 WriteByte(0X0030,0X00);
		 
		 
		 WriteByte(0X0011,0X10);
		 WriteByte(0X010A,0X30);
		 WriteByte(0X003F,0X46);
		 WriteByte(0X0031,0XFF);
		 WriteByte(0X0040,0X63);
		 WriteByte(0X002E,0X01);
		 WriteByte(0X001B,0X10);
		 WriteByte(0X003E,0X31);
		 WriteByte(0X0014,0X24);
		 
//		 //user
//		 WriteByte(0X001C,0X50);
//		 WriteByte(0X0022,0X08);
//		 WriteByte(0X002d,0X01);
//		 
		 WriteByte(0x016,0x00);
	 }
     return 0;
  }
/*
    开始测量模式选择
    选择单次测量模式
*/
 uint8_t VL6180X_Start_Range()
  {
     WriteByte(0x018,0x01);
	 return 0;
  }
  
uint16_t timeoutcnt=0; 
/*poll for new sample ready */
uint8_t VL6180X_Poll_Range()
{
		uint16_t t=0;
    uint8_t status;
	  uint8_t range_status;
	  status=ReadByte(0x04f);
    range_status=status&0x07;
		while(range_status!=0x04)
		{
			  timeoutcnt++;
				if(timeoutcnt > 2)
				{
					break;
				}
				status=ReadByte(0x04f);
				range_status=status&0x07;
				rt_thread_mdelay(1);
			
		}		  
		return 0;
}
   
  
  /*read range result (mm)*/
 uint8_t VL6180_Read_Range()
  {
    int range;
	range=ReadByte(0x062);
	return range;
  }
  
  /*clear interrupt*/
  void VL6180X_Clear_Interrupt()
  {
     WriteByte(0x015,0x07); 
  }
int i2c3_auto_scan_init(void)
{
    i2c_bus_init(&i2c3);
		rt_thread_mdelay(10);
    i2c_scan(&i2c3);

//    i2c_bus_init(&i2c2);
//    i2c_scan(&i2c2);
    return 0;
}
//INIT_APP_EXPORT(i2c3_auto_scan_init);
/**
 * @brief 执行一次测距并使用 ulog 打印结果
 *
 * 该函数启动一次测距，轮询就绪，读取距离值（mm），打印日志并清中断。
 * 返回 0 表示成功，非 0 表示超时或错误。
 */
void VL6180_ReadAndLog(void)
{
	uint8_t status;
	uint8_t range;
		i2c_bus_init(&i2c3);
	VL6180X_Init();
	LOG_I("VL6180 initialized.");
	VL6180X_Start_Range();
	status = VL6180X_Poll_Range();
	if (status != 0)
	{
		LOG_I("VL6180: measurement timeout/error: %d", status);
	}
	rt_thread_mdelay(100);
	range = VL6180_Read_Range();
	LOG_I("VL6180 range: %d mm", range);
	VL6180X_Clear_Interrupt();

}
MSH_CMD_EXPORT(VL6180_ReadAndLog, VL6180 perform a ranging measurement and log the result);



void thread_vl6180_task_entry(void *parameter)
{
    rt_uint32_t count = 0;

    /* One-time initialization */
    i2c_bus_init(&i2c3);  // Or directly i2c_bus_init(&i2c3) if preferred
	LOG_I("IIC initialization successfully.\r\n");
    rt_thread_mdelay(10);
    if (VL6180X_Init() != 0) {
        LOG_E("VL6180 initialization failed!");
        return;  // Exit task on init failure
    }
    LOG_I("VL6180 task started successfully.");

    while (1) {
        /* Start single-shot ranging */
        VL6180X_Start_Range();

        /* Poll for measurement ready (with timeout handling) */
        uint8_t poll_status = VL6180X_Poll_Range()*2;
        if (poll_status != 0) {
            LOG_W("VL6180 poll timeout or error: %d", poll_status);
        }
		rt_thread_mdelay(100);
        /* Read range value */
        uint16_t range = VL6180_Read_Range();

        /* Log the result */
        LOG_I("VL6180[%d] range: %d mm", count++, range);

        /* Clear interrupt for next measurement */
//        VL6180X_Clear_Interrupt();

        /* Delay 500ms before next measurement */
        rt_thread_mdelay(500);
    }
}
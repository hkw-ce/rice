#include "gn1650.h"
#include "gn1650_config.h"

//产生IIC总线起始信号
static void GN1650_IIC_start(void)
{
	GN1650_IIC_SCL_HIGH;     //SCL=1
	GN1650_IIC_SDA_HIGH;    //SDA=1
	GN1650_IIC_DELAY_4US;
	GN1650_IIC_SDA_LOW;     //SDA=0
	GN1650_IIC_DELAY_4US;
	GN1650_IIC_SCL_LOW;      //SCL=0
}
 
//产生IIC总线结束信号
static void GN1650_IIC_stop(void)
{
	GN1650_IIC_SCL_LOW;      //SCL=0
	GN1650_IIC_SDA_LOW;      //SDA=0
	GN1650_IIC_DELAY_4US;
	GN1650_IIC_SCL_HIGH;     //SCL=1
	GN1650_IIC_DELAY_4US;
	GN1650_IIC_SDA_HIGH;    //SDA=1
}
 
//通过IIC总线发送一个字节
static void GN1650_IIC_write_byte(uint8_t dat)
{
	uint8_t i;
    
	GN1650_IIC_SCL_LOW;
	for(i=0;i<8;i++)
	{
		GN1650_IIC_SDA_WR(dat&0x80);
		dat<<=1;    
        
		GN1650_IIC_DELAY_2US;
		GN1650_IIC_SCL_HIGH;
		GN1650_IIC_DELAY_2US;
		GN1650_IIC_SCL_LOW;
		GN1650_IIC_DELAY_2US;
	}
}
 
//通过IIC总线接收从机响应的ACK信号
static uint8_t GN1650_IIC_wait_ack(void)
{
	uint8_t ack_signal = 0;
    
	GN1650_IIC_SDA_HIGH;    //SDA=1
	GN1650_IIC_DELAY_2US;
	GN1650_IIC_SCL_HIGH;
	GN1650_IIC_DELAY_2US;
	if(GN1650_IIC_SDA_RD()) ack_signal = 1;   //如果读取到的是NACK信号
	GN1650_IIC_SCL_LOW;
	GN1650_IIC_DELAY_2US;
	return ack_signal;
}
 
 
//GN1650初始化
void GN1650_init(void)
{
	GN1650_IIC_SCL_MODE_OD;  //SCL开漏输出
	GN1650_IIC_SDA_MODE_OD;  //SDA开漏输出
 
	GN1650_IIC_SDA_HIGH;   //释放SDA线
	GN1650_IIC_SCL_HIGH;   //释放SCL线
    
	GN1650_cfg_display(GN1650_BRIGHT5);   //初始化为5级亮度，打开显示
	GN1650_clear();     //将显存内容清0
}
 
 
//作用：设置显示参数
//备注：这个操作不影响显存中的数据
//用例：
//	设置亮度并打开显示:GN1650_cfg_display(GN1650_BRIGHTx)
//	关闭显示:GN1650_cfg_display(GN1650_DSP_OFF)
void GN1650_cfg_display(uint8_t param)
{

	GN1650_IIC_start();
	GN1650_IIC_write_byte(0x48);  GN1650_IIC_wait_ack();     //固定命令
	GN1650_IIC_write_byte(param); GN1650_IIC_wait_ack();    //参数值
	GN1650_IIC_stop();
}
 
 
//将显存数据全部刷为0，清空显示
void GN1650_clear(void)
{
	uint8_t dig;
	for(dig = GN1650_DIG1 ; dig<= GN1650_DIG4 ;dig++)
	{
		GN1650_print(dig,0);   //将显存数据刷为0
	}
}
 
//往一个指定的数码管位写入指定的显示数据
//共阴数码管段码表：
//const uint8_t TUBE_TABLE_0[10] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};                                  //共阴，0~9的数字
//const uint8_t TUBE_TABLE_0[16] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};    //共阴，0~9~A~F
//用例：
//	在DIG1位上显示数字3: GN1650_print(GN1650_DIG1,TUBE_TABLE_0[3]);
void GN1650_print(uint8_t dig,uint8_t seg_data)
{
	GN1650_IIC_start();
	GN1650_IIC_write_byte(dig*2+0x68); GN1650_IIC_wait_ack();  //显存起始地址为0x68
	GN1650_IIC_write_byte(seg_data); GN1650_IIC_wait_ack();    //发送段码
	GN1650_IIC_stop();
}

// 0-9 段码 (您的自定义, bit2 DP off for no unit)
const uint8_t seg_codes_digits[10] = {
    0xEF & ~0x04u,  // 0: no unit
    0x2C & ~0x04u,  // 1
    0xB7 & ~0x04u,  // 2
    0xBE & ~0x04u,  // 3
    0x7C & ~0x04u,  // 4
    0xDE & ~0x04u,  // 5
    0xDF & ~0x04u,  // 6
    0xAC & ~0x04u,  // 7
    0xFF & ~0x04u,  // 8
    0xFE & ~0x04u   // 9
};

// A-F 段码 (基于映射推测, bit2 DP=0 off, no unit)
const uint8_t seg_codes_letters[6] = {
    0xF9,  // A
    0x7B,  // B
    0xC3,  // C
    0x3B,  // D
    0xD3,  // E
    0xD1   // F
};

void display_test(void)
{
    uint8_t i, ack_err = 0;

    LOG_I("GN1650 Test Start: 0-9 + A-F on DIG1/DIG2/DIG3 (no unit DP bit2 off, 1s delay each)");

    GN1650_init();  // Init brightness 5
    GN1650_clear();  // Clear all
    GN1650_cfg_display(GN1650_BRIGHT8);  // Max brightness

    // Clear DIG4
    GN1650_print(GN1650_DIG4, 0x00);

    // Display 0-9
    LOG_I("Displaying 0-9 (no unit)");
    for (i = 0; i < 10; i++) {
        uint8_t code = seg_codes_digits[i];

        GN1650_print(GN1650_DIG1, code);
        if (GN1650_IIC_wait_ack() != 0) ack_err++;

        GN1650_print(GN1650_DIG2, code);
        if (GN1650_IIC_wait_ack() != 0) ack_err++;

        GN1650_print(GN1650_DIG3, code);
        if (GN1650_IIC_wait_ack() != 0) ack_err++;

        LOG_I("Digit %u: code 0x%02X on DIG1/2/3, ACK err %u (page 4.1)", i, code, ack_err);

        rt_thread_mdelay(1000);
    } 

    // Display A-F
    LOG_I("Displaying A-F (no unit)");
    for (i = 0; i < 6; i++) {
        uint8_t code = seg_codes_letters[i];

        GN1650_print(GN1650_DIG1, code);
        if (GN1650_IIC_wait_ack() != 0) ack_err++;

        GN1650_print(GN1650_DIG2, code);
        if (GN1650_IIC_wait_ack() != 0) ack_err++;

        GN1650_print(GN1650_DIG3, code);
        if (GN1650_IIC_wait_ack() != 0) ack_err++;

        LOG_I("Letter %c: code 0x%02X on DIG1/2/3, ACK err %u (page 4.1)", 'A' + i, code, ack_err);

        rt_thread_mdelay(1000);
    }

    GN1650_clear();
    GN1650_cfg_display(GN1650_DSP_OFF);

    LOG_I("Test End: ACK errors %u (0=OK; 0-9 + A-F no unit, page 2 A~G/DP pins)", ack_err);
}

MSH_CMD_EXPORT(display_test, GN1650 0-9 + A-F test on DIG1/DIG2/DIG3 no unit (DP bit2 off));
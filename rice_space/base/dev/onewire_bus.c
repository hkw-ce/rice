

#include "onewire_bus.h"
#include "platform.h"
#include "stdint.h"

// 1. 引脚定义
#define WT588F_DATA_PIN    GPIO_Pin_5    // DATA 线（输出）
#define WT588F_BUSY_PIN    GPIO_Pin_4    // BUSY 脚（输入）
#define WT588F_DATA_PORT   GPIOB         
#define WT588F_BUSY_PORT   GPIOF         

// 双字节指令宏定义（同前）
#define WT588F_CMD_STOP_DUAL    0xFFFE
#define WT588F_CMD_CONT_DUAL    0xFFF3
#define WT588F_VOL_MAX_DUAL     0xFFEF

// 函数声明（新增 BUSY 脚读取函数）
void WT588F_GPIO_Init(void);                  // 初始化 DATA+BUSY 脚
void WT588F_Send_Word(uint16_t send_word);    // 同前，双字节发送
void WT588F_Cont_Play_Dual(uint16_t* addr_list, uint8_t addr_num); // 同前，连码播放
void WT588F_Stop_Voice_Dual(void);            // 同前，双字节停止
uint8_t WT588F_Read_BUSY(void);               // 新增：读取 BUSY 脚电平
void WT588F_Wait_Play_Finish(void);           // 新增：等待播放完成
void WT588F_Send_Byte(uint8_t send_data);
#define CPU_FREQUENCY_MHZ    120		// STM32????
void OneBus_DelayUs(__IO uint32_t delay)
{
	
    int last, curr, val;
    int temp;
 
    while (delay != 0)
    {
        temp = delay > 900 ? 900 : delay;
        last = SysTick->VAL;
        curr = last - CPU_FREQUENCY_MHZ * temp;
        if (curr >= 0)
        {
            do
            {
                val = SysTick->VAL;
            }
            while ((val < last) && (val >= curr));
        }
        else
        {
            curr += CPU_FREQUENCY_MHZ * 1000;
            do
            {
                val = SysTick->VAL;
            }
            while ((val <= last) || (val > curr));
        }
        delay -= temp;
    }
}

void rt_voice_us_delay(rt_uint32_t us)
{
    rt_uint32_t start, now, delta, reload, us_tick;
    start = SysTick->VAL;
    reload = SysTick->LOAD;
    us_tick =CPU_FREQUENCY_MHZ;
    do
    {
        now = SysTick->VAL;
        delta = start > now ? start - now : reload + start - now;
    } while (delta < us_tick * us);
}

void WT588F_GPIO_Init(void)
{
    // --------------------------
    // 1. DATA 线（PA0）：推挽输出（同前）
    // --------------------------
    gpio_config(WT588F_DATA_PORT, WT588F_DATA_PIN, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
    GPIO_ResetBits(WT588F_DATA_PORT, WT588F_DATA_PIN); // 初始高电平
    
    // --------------------------
    // 2. BUSY 脚（PA1）：浮空输入
    // --------------------------
    gpio_config(WT588F_BUSY_PORT, WT588F_BUSY_PIN, GPIO_Mode_IPU, GPIO_Speed_50MHz);
}

/**
 * 读取 BUSY 脚当前电平
 * @return 1：BUSY 高（播放结束/待机）；0：BUSY 低（播放中/初始化）
 */
uint8_t WT588F_Read_BUSY(void)
{
    // 读取 PC8 引脚电平，返回 1（高）或 0（低）
    return GPIO_ReadInputDataBit(WT588F_BUSY_PORT, WT588F_BUSY_PIN);
}

/**
 * 等待播放完成（阻塞式）：直到 BUSY 脚变高，或超时退出（避免死等）
 * @param timeout_ms：最大等待超时时间（ms），0 表示无限等待
 * @return 0：播放正常完成；1：等待超时
 */
//uint8_t WT588F_Wait_Play_Finish(uint32_t timeout_ms)
//{
//    uint32_t start_time = 0;
//    // 记录开始等待的时间（基于延时函数的毫秒计数，需确保 delay_get_tick 函数可用）
//    start_time = delay_get_tick(); // 需实现：返回当前系统运行的毫秒数（如基于 SysTick）
//    
//    // 等待逻辑：BUSY 为低（播放中）则持续等待，直到变高或超时
//    while (WT588F_Read_BUSY() == 0)
//    {
//        // 若设置了超时时间，判断是否超过最大等待时长
//        if (timeout_ms > 0)
//        {
//            if (delay_get_tick() - start_time > timeout_ms)
//            {
//                return 1; // 超时返回 1
//            }
//        }
//        rt_thread_mdelay(10); // 每 10ms 读取一次，降低 CPU 占用
//    }
//    
//    return 0; // 正常完成返回 0
//}

/**
 * 播放指定地址的语音（单字节模式：地址 00H~DFH，对应第 0~223 段）
 * @param voice_addr：语音地址（如 01H 对应第 1 段语音）
 */
void WT588F_Play_Voice(uint8_t voice_addr)
{
    // 校验地址合法性（单字节模式下地址 ≤ DFH）
    if (voice_addr > 0xDF)
        return;
    
    WT588F_Send_Byte(voice_addr);  // 直接发送地址，芯片自动播放
}

/**
 * 调节音量（16 级，单字节模式：E0H 最小，EFH 最大）
 * @param volume_code：音量指令码（E0H~EFH，默认 EFH 最大音量）
 */
void WT588F_Set_Volume(uint8_t volume_code)
{
    // 校验音量指令合法性
    if (volume_code < 0xE0 || volume_code > 0xEF)
        return;
    
    WT588F_Send_Byte(volume_code);  // 发送音量指令，立即生效
}

/**
 * 停止当前播放的语音（单字节指令：FEH）
 */
void WT588F_Stop_Voice(void)
{
    WT588F_Send_Byte(0xFE);  // 发送停止指令，中断当前播放/循环
}

void WT588F_Send_Byte(uint8_t send_data)
{
    uint8_t i;
    uint8_t bit_data;  // 单个数据位（0/1）
    
    // 1. 发码前准备：若 DATA 为低，先拉高 ≥5ms（确保芯片识别启动信号）

    if (GPIO_ReadInputDataBit(WT588F_DATA_PORT, WT588F_DATA_PIN) == 0)
    {
        GPIO_SetBits(WT588F_DATA_PORT, WT588F_DATA_PIN);
        rt_thread_mdelay(5);  // 推荐延时 5ms，满足 ≥5ms 要求
    }
    
    // 2. 发送启动信号：拉低 DATA 线 5ms（芯片识别的“指令开始”标志）
    GPIO_ResetBits(WT588F_DATA_PORT, WT588F_DATA_PIN);
    rt_thread_mdelay(5);
    
    // 3. 发送 8 位数据（低位先发送，符合芯片时序要求）
    for (i = 0; i < 8; i++)
    {
        bit_data = send_data & 0x01;  // 取当前最低位
        
        if (bit_data == 1)  // 表示数据位 1：高电平:低电平 = 3:1（推荐 600us:200us）
        {
            GPIO_SetBits(WT588F_DATA_PORT, WT588F_DATA_PIN);
            rt_voice_us_delay(600);  // 高电平持续 600us
            GPIO_ResetBits(WT588F_DATA_PORT, WT588F_DATA_PIN);
            rt_voice_us_delay(200);  // 低电平持续 200us
        }
        else  // 表示数据位 0：高电平:低电平 = 1:3（推荐 200us:600us）
        {
            GPIO_SetBits(WT588F_DATA_PORT, WT588F_DATA_PIN);
            rt_voice_us_delay(200);  // 高电平持续 200us
            GPIO_ResetBits(WT588F_DATA_PORT, WT588F_DATA_PIN);
            rt_voice_us_delay(600);  // 低电平持续 600us
        }
        
        send_data >>= 1;  // 数据右移，准备下一位（低位到高位）
    }
    
    // 4. 指令发送结束：拉高 DATA 线，等待下一次指令
    GPIO_SetBits(WT588F_DATA_PORT, WT588F_DATA_PIN);
    rt_thread_mdelay(2);  // 指令间隔 ≥2ms，避免连续发码冲突
}

void WT588F_Init(void)
{
    WT588F_GPIO_Init();     // DATA+BUSY 脚初始化
    rt_thread_mdelay(200);          // 等待 WT588F 上电初始化
	//设置音量为中等（E8H，16 级中第 9 级）
    WT588F_Set_Volume(0xEF);
    rt_thread_mdelay(100);  // 等待指令生效
	// // 步骤 2：播放第 1 段语音（地址 01H）
	 WT588F_Play_Voice(0x01);
     rt_thread_mdelay(5000);  // 等待语音播放完成（需根据实际语音时长调整）
		 WT588F_Play_Voice(0x02);
     rt_thread_mdelay(5000);  // 等待语音播放完成（需根据实际语音时长调整）
		 WT588F_Play_Voice(0x03);
     rt_thread_mdelay(5000);  // 等待语音播放完成（需根据实际语音时长调整）
		 WT588F_Play_Voice(0x04);
     rt_thread_mdelay(5000);  // 等待语音播放完成（需根据实际语音时长调整）
	 // 步骤 3：停止播放
    WT588F_Stop_Voice();
     rt_thread_mdelay(2000);  // 暂停 2s 后重复流程
}
MSH_CMD_EXPORT(WT588F_Init, wt588f );
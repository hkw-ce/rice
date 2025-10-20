

#include "platform.h"
#ifndef USER_I2C_V1
/* 用户修改这4个宏为你的 GPIO 操作 */
#define I2C1_SDA_GPIO_PORT GPIOB
#define I2C1_SDA_PIN GPIO_Pin_11
#define I2C1_SCL_GPIO_PORT GPIOB
#define I2C1_SCL_PIN GPIO_Pin_10
#define enter_irq() rt_exit_critical();
#define quit_irq() rt_enter_critical();
#define SDA_HIGH() GPIO_SetBits(I2C1_SDA_GPIO_PORT, I2C1_SDA_PIN)          // 置 SDA 为高电平（输入或输出高）
#define SDA_LOW() GPIO_ResetBits(I2C1_SDA_GPIO_PORT, I2C1_SDA_PIN)         // 置 SDA 为低电平
#define SCL_HIGH() GPIO_SetBits(I2C1_SCL_GPIO_PORT, I2C1_SCL_PIN)          // 置 SCL 为高电平
#define SCL_LOW() GPIO_ResetBits(I2C1_SCL_GPIO_PORT, I2C1_SCL_PIN);        // 置 SCL 为低电平
#define SDA_READ() GPIO_ReadInputDataBit(I2C1_SDA_GPIO_PORT, I2C1_SDA_PIN) // 读取 SDA 引脚电平（返回0或1）


#define I2C_DELAY() rt_hw_us_delay(2)
/* === 延时宏定义 === */
#define I2C_DELAY_LOW() rt_hw_us_delay(5)  // SCL 低电平时间
#define I2C_DELAY_HIGH() rt_hw_us_delay(5) // SCL 高电平时间
#define I2C_DELAY_ACK() rt_hw_us_delay(30) // ACK阶段专用延时
/* === 基础操作 === */
// SCL: ────────????????────────
// SDA: ?????\_________________
//          ↑
//        Start: SDA 高→低, SCL 高
//        SCL 高电平期间SDA 由高拉低→ 起始信号
static void i2c_start(void)
{
    SDA_HIGH();
    SCL_HIGH();
    I2C_DELAY_HIGH(); // 确保总线空闲
    SDA_LOW();        // START
    I2C_DELAY_HIGH(); // SDA稳定时间
    SCL_LOW();
    I2C_DELAY_LOW();
}
// SCL: ────────????????────────
// SDA: _______/?????????
//          ↑
//        Stop: SDA 低→高, SCL 高
//        SCL 高电平期间SDA 由低拉高→ 停止信号

static void i2c_stop(void)
{
    SDA_LOW();
    I2C_DELAY_LOW();
    SCL_HIGH();
    I2C_DELAY_HIGH(); // 保持稳定
    SDA_HIGH();       // STOP
    I2C_DELAY_HIGH();
}

static uint8_t i2c_clock_cycle(uint8_t data, int input_mode, int is_ack_phase)
{
    uint8_t bit = 0;

    if (!input_mode)
        data ? SDA_HIGH() : SDA_LOW();
    else
        SDA_HIGH(); // 输入模式（释放 SDA）

    I2C_DELAY_LOW(); // SCL低电平阶段

    SCL_HIGH();
    if (is_ack_phase)
        I2C_DELAY_ACK(); // ACK阶段延时更长
    else
        I2C_DELAY_HIGH(); // 正常高电平阶段

    if (input_mode)
        bit = SDA_READ(); // 上升沿稳定后采样

    SCL_LOW();
    I2C_DELAY_LOW(); // 再次进入低电平
    return bit;
}

// Bit:   7   6   5   4   3   2   1   0   ACK
// SDA:   ─?─?─?─?─?─?─?─?─     (数据电平)
// SCL: ──??─??─??─??─??─??─??
// 每位在SCL 上升沿采样
// 第9 位由从机拉低表示ACK
static uint8_t i2c_write_byte(uint8_t d)
{
    for (int i = 0; i < 8; i++)
    {
        i2c_clock_cycle(d & 0x80, 0, 0); // 正常数据位
        d <<= 1;
    }
    return i2c_clock_cycle(1, 1, 1); // 第9位采ACK
}
// Bit:   7   6   5   4   3   2   1   0   MCU_ACK
// SDA:   d7  d6  d5  d4  d3  d2  d1  d0   ↓ (MCU ACK/NACK)
// SCL: ──??─??─??─??─??─??─??─??─??
// SDA 高阻输入，从机输出数据

// MCU 在第9个时钟周期拉低SDA（ACK）或保持高（NACK）
static uint8_t i2c_read_byte(int ack)
{
    uint8_t d = 0;
    for (int i = 0; i < 8; i++)
    {
        d <<= 1;
        d |= i2c_clock_cycle(1, 1, 0); // 数据位采样
    }
    i2c_clock_cycle(!ack, 0, 1); // 第9位MCU发ACK/NACK
    return d;
}

/* === 高层函数 === */
/*
→写操作：
    1 发送从机地址+ 写位
    2 发送寄存器地址
    3 发送数据
*/
void i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t val)
{
    enter_irq();
    i2c_start();
    i2c_write_byte((dev_addr << 1) | 0); // 写命令
    i2c_write_byte(reg);
    i2c_write_byte(val);
    i2c_stop();
    quit_irq();
}
/*
→读操作：
    1 发送从机地址+ 写位（告诉从机要访问哪个寄存器）
    2 发送寄存器地址
    3 发送重复起始
    4 发送从机地址+ 读位
    5 读取数据
*/
uint8_t i2c_read_reg(uint8_t dev_addr, uint8_t reg)
{
    uint8_t val;
    enter_irq();
    i2c_start();
    i2c_write_byte((dev_addr << 1) | 0);
    i2c_write_byte(reg);
    i2c_start();
    i2c_write_byte((dev_addr << 1) | 1);
    val = i2c_read_byte(0);
    i2c_stop();
    quit_irq();
    return val;
}

void i2c_write_bytes(uint8_t dev_addr, uint8_t reg, const uint8_t *buf, uint8_t len)
{
    i2c_start();
    i2c_write_byte((dev_addr << 1) | 0);
    i2c_write_byte(reg);
    while (len--)
        i2c_write_byte(*buf++);
    i2c_stop();
}

void i2c_read_bytes(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    i2c_start();
    i2c_write_byte((dev_addr << 1) | 0);
    i2c_write_byte(reg);
    i2c_start();
    i2c_write_byte((dev_addr << 1) | 1);
    while (len--)
        *buf++ = i2c_read_byte(len ? 1 : 0);
    i2c_stop();
}
void i2c_init(void)
{
    gpio_config(I2C1_SDA_GPIO_PORT, I2C1_SDA_PIN, GPIO_Mode_Out_OD, GPIO_Speed_High);
    gpio_config(I2C1_SCL_GPIO_PORT, I2C1_SCL_PIN, GPIO_Mode_Out_OD, GPIO_Speed_High);
    SDA_HIGH();
    SCL_HIGH();
}
// 改造后的安全版写字节并等待 ACK
#define I2C_ACK_TIMEOUT 50
static uint8_t i2c_write_byte_ack(uint8_t d)
{
    for (int i = 0; i < 8; i++)
    {
        i2c_clock_cycle(d & 0x80, 0, 0); // 输出数据位
        d <<= 1;
    }
    return i2c_clock_cycle(1, 1, 1); // 第9位释放SDA并读ACK(0=ACK,1=NACK)
}

int i2c_scan(void)
{

    i2c_init();
    uint8_t addr;

    for (addr = 0; addr < 128; addr++)
    {
        i2c_start();

        if (i2c_write_byte_ack((addr << 1) | 0) == 0)
        {
            rt_kprintf("I2C device found at 0x%02X\r\n", addr);
        }
        i2c_stop();
        rt_hw_us_delay(50); // 短延时
    }
    //		uint8_t buf[2];
    //		i2c_read_bytes(0x15, 0x04, buf, 2);
    //		rt_kprintf("xpos 0x%02X,0x%02X\r\n", buf[0],buf[1]);
    return 0;
}

//INIT_APP_EXPORT(i2c_scan);

#endif

#include "platform.h"
#include <stdint.h>
#include <stdio.h>
#include "mm32_soft_i2c_v1.h"
#include "rtthread.h" 
#ifdef USER_I2C_V1
/*
 * Software I2C timing parameters (units: microseconds).
 * Target: ~100 kHz SCL. These are conservative defaults — you can tune
 * them if your MCU and delay implementation allow shorter delays.
 *
 * I2C clock cycle in this implementation (non-ACK bit):
 *   T = PRE + HIGH + POST
 * For data bits we set PRE/HIGH/POST to 3/3/3 => T = 9us (≈111 kHz).
 * ACK bit uses a longer HIGH to give slave time to pull SDA low.
 */
#ifndef I2C_DELAY_PRE_US
#define I2C_DELAY_PRE_US        3
#define I2C_DELAY_HIGH_US       3
#define I2C_DELAY_POST_US       3
#define I2C_DELAY_ACK_HIGH_US  12
#endif

#define I2C_DELAY_PRE(bus)      i2c_delay_us(bus, I2C_DELAY_PRE_US)
#define I2C_DELAY_HIGH(bus)     i2c_delay_us(bus, I2C_DELAY_HIGH_US)
#define I2C_DELAY_POST(bus)     i2c_delay_us(bus, I2C_DELAY_POST_US)
#define I2C_DELAY_ACK_HIGH(bus) i2c_delay_us(bus, I2C_DELAY_ACK_HIGH_US)

/* 如果 bus->delay_us 为 NULL，则使用这个默认实现 */
 void default_delay_us(uint32_t us)
{
    rt_hw_us_delay(us);
}

/* ---- 简单 GPIO 包装函数 ---- */
static inline void gpio_set_high(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_SetBits(port, pin);
}
static inline void gpio_set_low(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_ResetBits(port, pin);
}
static inline uint8_t gpio_get(GPIO_TypeDef *port, uint16_t pin)
{
    return (uint8_t)GPIO_ReadInputDataBit(port, pin);
}

/* ==== I2C 基本信号（面向 i2c_bus_t 实例） ==== */
static void i2c_scl(i2c_bus_t *bus, uint8_t level)
{
    if (level)
        gpio_set_high(bus->scl_port, bus->scl_pin);
    else
        gpio_set_low(bus->scl_port, bus->scl_pin);
}

static void i2c_sda(i2c_bus_t *bus, uint8_t level)
{
    if (level)
        gpio_set_high(bus->sda_port, bus->sda_pin);
    else
        gpio_set_low(bus->sda_port, bus->sda_pin);
}

static uint8_t i2c_read_sda(i2c_bus_t *bus)
{
    return gpio_get(bus->sda_port, bus->sda_pin);
}

/* ==== 时序辅助：调用 bus 的 delay 或默认 delay ==== */
static inline void i2c_delay_us(i2c_bus_t *bus, uint32_t us)
{
    if (bus && bus->delay_us)
        bus->delay_us(us);
    else
        default_delay_us(us);
}

/*
    i2c_clock_cycle：
    - bus: 指定总线
    - data: 当 output_mode 时该位的电平（0/1）
    - input_mode: 1 表示 SDA 释放为输入（从机驱动 SDA），0 表示输出 data
    - is_ack_phase: ACK阶段可做更长延时（让从机有时间拉低）
    返回：在 input_mode 时读取到的 SDA 值，否则返回 0
*/
static uint8_t i2c_clock_cycle(i2c_bus_t *bus, uint8_t data, int input_mode, int is_ack_phase)
{
    uint8_t bit = 0;

    /* SDA 输出或释放 */
    if (!input_mode)
    {
        /* 输出模式：把 SDA 设置为 data 电平（主机驱动） */
        i2c_sda(bus, data ? 1 : 0);
    }
    else
    {
        /* 释放 SDA（高阻）——通过拉高来实现 */
        i2c_sda(bus, 1);
    }

    /* SCL 低电平阶段保持一小段时间 */
    I2C_DELAY_PRE(bus);

    /* SCL 上升到高电平，期间从机/主机应稳定 SDA */
    i2c_scl(bus, 1);

    if (is_ack_phase)
        I2C_DELAY_ACK_HIGH(bus); /* ACK阶段延时更久，容错性好 */
    else
        I2C_DELAY_HIGH(bus);

    /* 采样 SDA（如果是输入模式） */
    if (input_mode)
        bit = i2c_read_sda(bus);

    /* SCL 拉回低电平，完成一个时钟 */
    i2c_scl(bus, 0);
    I2C_DELAY_POST(bus);

    return bit;
}

/* ==== 字节级函数（基于 i2c_clock_cycle） ==== */

/* 发送 1 字节并读取 ACK（0 表示 ACK）——用于 scan */
static uint8_t i2c_write_byte_ack(i2c_bus_t *bus, uint8_t d)
{
    for (int i = 0; i < 8; i++)
    {
        i2c_clock_cycle(bus, d&0x80, 0, 0);
        d <<= 1;
    }

    /* 第9 位：释放 SDA，读从机 ACK */
    return i2c_clock_cycle(bus, 1, 1, 1);
 /* 0=ACK, 1=NACK */
}

/* 发送 1 字节并返回 ACK（兼容原先 i2c_write_byte 的语义） */
static uint8_t i2c_write_byte(i2c_bus_t *bus, uint8_t data)
{
    return i2c_write_byte_ack(bus, data);
}

/* 读取 1 字节并在第9位发送 MCU ACK/NACK（ack=1 表示 MCU 发 ACK，ack=0 表示 MCU 发 NACK） */
static uint8_t i2c_read_byte(i2c_bus_t *bus, int ack)
{
    uint8_t data = 0;
    for (int i = 0; i < 8; i++)
    {
        data <<= 1;
        data |= i2c_clock_cycle(bus, 1, 1, 0);
    }
    /* 第9位：主机发送 ACK/NACK（0->拉低表示 ACK, 1->保持高表示 NACK） */
    i2c_clock_cycle(bus, (ack ? 0 : 1), 0, 1);
    return data;
}

/* ==== 高层操作：start/stop/read/write 多字节 等 ==== */

static void i2c_start(i2c_bus_t *bus)
{
    i2c_sda(bus, 1);
    i2c_scl(bus, 1);
    I2C_DELAY_PRE(bus);
    i2c_sda(bus, 0);
    I2C_DELAY_POST(bus);
    i2c_scl(bus, 0);
}

static void i2c_stop(i2c_bus_t *bus)
{
    i2c_sda(bus, 0);
    I2C_DELAY_PRE(bus);
    i2c_scl(bus, 1);
    I2C_DELAY_HIGH(bus);
    i2c_sda(bus, 1);
    I2C_DELAY_POST(bus);
}

/* 向设备寄存器写一个字节（不检查返回值） */
void i2c_write_reg(i2c_bus_t *bus, uint8_t dev, uint8_t reg, uint8_t val)
{
    i2c_start(bus);
    i2c_write_byte(bus, (dev << 1) | 0);
    i2c_write_byte(bus, reg);
    i2c_write_byte(bus, val);
    i2c_stop(bus);
}

/* 读设备寄存器一个字节 */
uint8_t i2c_read_reg(i2c_bus_t *bus, uint8_t dev, uint8_t reg)
{
    uint8_t val = 0;
    i2c_start(bus);
    i2c_write_byte(bus, (dev << 1) | 0);
    i2c_write_byte(bus, reg);
    i2c_start(bus);
    i2c_write_byte(bus, (dev << 1) | 1);
    val = i2c_read_byte(bus, 0); /* NACK after single byte read */
    i2c_stop(bus);
    return val;
}

/*读取设置u16寄存器*/
uint8_t i2c_read_reg16(i2c_bus_t *bus, uint8_t dev, uint16_t reg)
{
    uint16_t val = 0;
    i2c_start(bus);
    i2c_write_byte(bus, (dev << 1) | 0);
    i2c_write_byte(bus, reg>>8);
    i2c_write_byte(bus, reg&0xFF);
    i2c_start(bus);
    i2c_write_byte(bus, (dev << 1) | 1);
    val = i2c_read_byte(bus, 0); /* ACK after high byte read */
    i2c_stop(bus);
    return val;
}

void i2c_write_bytes(i2c_bus_t *bus, uint8_t dev, uint8_t reg, const uint8_t *buf, uint8_t len)
{
    i2c_start(bus);
    i2c_write_byte(bus, (dev << 1) | 0);
    i2c_write_byte(bus, reg);
    while (len--)
    {
        i2c_write_byte(bus, *buf++);
    }
    i2c_stop(bus);
}

void i2c_read_bytes(i2c_bus_t *bus, uint8_t dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    i2c_start(bus);
    i2c_write_byte(bus, (dev << 1) | 0);
    i2c_write_byte(bus, reg);
    i2c_start(bus);
    i2c_write_byte(bus, (dev << 1) | 1);
    while (len--)
    {
        *buf++ = i2c_read_byte(bus, len ? 1 : 0); /* 对于最后一个字节 NACK */
    }
    i2c_stop(bus);
}

/* ==== 扫描函数 ==== */
void i2c_scan(i2c_bus_t *bus)
{
//    rt_kprintf("\r\nScanning I2C bus: SDA=%p.%04X  SCL=%p.%04X\r\n",
//               (void *)bus->sda_port, (unsigned)bus->sda_pin,
//               (void *)bus->scl_port, (unsigned)bus->scl_pin);

//    rt_enter_critical(); /* 禁用中断 */
    for (uint8_t addr = 0; addr < 128; addr++)
    {
        i2c_start(bus);
        uint8_t ack = i2c_write_byte_ack(bus, (addr << 1) | 0);
         i2c_stop(bus);
        if (ack == 0)
        {
            rt_kprintf("I2C device found at 0x%02X (ACK=%d)\r\n", addr, ack);
        }
				i2c_stop(bus);
        i2c_delay_us(bus, 50);
    }
//    rt_exit_critical(); /* 恢复中断 */
}

/* ==== 总线初始化（配置 GPIO） ==== */
void i2c_bus_init(i2c_bus_t *bus)
{
    if (!bus) {
        rt_kprintf("I2C bus init failed: null bus pointer\n");
        return;
    }

    if (!bus->delay_us) bus->delay_us = default_delay_us;

    rt_kprintf("Initializing I2C: SDA=%p.%04X, SCL=%p.%04X\n",
               (void *)bus->sda_port, bus->sda_pin,
               (void *)bus->scl_port, bus->scl_pin);

    gpio_config(bus->sda_port, bus->sda_pin, GPIO_Mode_Out_OD, GPIO_Speed_50MHz);
    gpio_config(bus->scl_port, bus->scl_pin, GPIO_Mode_Out_OD, GPIO_Speed_50MHz);

//    gpio_config(bus->sda_port, bus->sda_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
//    gpio_config(bus->scl_port, bus->scl_pin, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
    i2c_sda(bus, 1);
    i2c_scl(bus, 1);

    /* 验证总线状态 */
    if (i2c_read_sda(bus) != 1 ) {
        rt_kprintf("I2C bus init failed: SDA not high\n");
    }
		if (gpio_get(bus->scl_port, bus->scl_pin) != 1) {
        rt_kprintf("I2C bus init failed: SCL not high\n");
    }
}
/* ==== 示例两路 I2C 定义（你可以放在别的文件引用） ==== */


i2c_bus_t i2c1 = {
.scl_port = GPIOA,
.scl_pin  = GPIO_Pin_10,
.sda_port = GPIOA,
.sda_pin  = GPIO_Pin_9,
.delay_us = default_delay_us,
};


int i2c_auto_scan_init(void)
{
    i2c_bus_init(&i2c1);
		rt_thread_mdelay(10);
    i2c_scan(&i2c1);

//    i2c_bus_init(&i2c2);
//    i2c_scan(&i2c2);
    return 0;
}
//INIT_APP_EXPORT(i2c_auto_scan_init);
#endif 


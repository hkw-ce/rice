#include "platform.h"
#include <stdint.h>
#include <stdio.h>
#include "mm32_soft_i2c_v1.h"
#include "rtthread.h" 
#ifdef USER_I2C_V1
/* ��� bus->delay_us Ϊ NULL����ʹ�����Ĭ��ʵ�� */
static void default_delay_us(uint32_t us)
{
    rt_hw_us_delay(us);
}

/* ---- �� GPIO ��װ���� ---- */
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

/* ==== I2C �����źţ����� i2c_bus_t ʵ���� ==== */
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

/* ==== ʱ���������� bus �� delay ��Ĭ�� delay ==== */
static inline void i2c_delay_us(i2c_bus_t *bus, uint32_t us)
{
    if (bus && bus->delay_us)
        bus->delay_us(us);
    else
        default_delay_us(us);
}

/*
    i2c_clock_cycle��
    - bus: ָ������
    - data: �� output_mode ʱ��λ�ĵ�ƽ��0/1��
    - input_mode: 1 ��ʾ SDA �ͷ�Ϊ���루�ӻ����� SDA����0 ��ʾ��� data
    - is_ack_phase: ACK�׶ο���������ʱ���ôӻ���ʱ�����ͣ�
    ���أ��� input_mode ʱ��ȡ���� SDA ֵ�����򷵻� 0
*/
static uint8_t i2c_clock_cycle(i2c_bus_t *bus, uint8_t data, int input_mode, int is_ack_phase)
{
    uint8_t bit = 0;

    /* SDA ������ͷ� */
    if (!input_mode)
    {
        /* ���ģʽ���� SDA ����Ϊ data ��ƽ������������ */
        i2c_sda(bus, data ? 1 : 0);
    }
    else
    {
        /* �ͷ� SDA�����裩����ͨ��������ʵ�� */
        i2c_sda(bus, 1);
    }

    /* SCL �͵�ƽ�׶α���һС��ʱ�� */
    i2c_delay_us(bus, 5);

    /* SCL �������ߵ�ƽ���ڼ�ӻ�/����Ӧ�ȶ� SDA */
    i2c_scl(bus, 1);

    if (is_ack_phase)
        i2c_delay_us(bus, 30); /* ACK�׶���ʱ���ã��ݴ��Ժ� */
    else
        i2c_delay_us(bus, 5);

    /* ���� SDA�����������ģʽ�� */
    if (input_mode)
        bit = i2c_read_sda(bus);

    /* SCL ���ص͵�ƽ�����һ��ʱ�� */
    i2c_scl(bus, 0);
    i2c_delay_us(bus, 5);

    return bit;
}

/* ==== �ֽڼ����������� i2c_clock_cycle�� ==== */

/* ���� 1 �ֽڲ���ȡ ACK��0 ��ʾ ACK���������� scan */
static uint8_t i2c_write_byte_ack(i2c_bus_t *bus, uint8_t d)
{
    for (int i = 0; i < 8; i++)
    {
        i2c_clock_cycle(bus, d&0x80, 0, 0);
        d <<= 1;
    }

    /* ��9 λ���ͷ� SDA�����ӻ� ACK */
    return i2c_clock_cycle(bus, 1, 1, 1);
 /* 0=ACK, 1=NACK */
}

/* ���� 1 �ֽڲ����� ACK������ԭ�� i2c_write_byte �����壩 */
static uint8_t i2c_write_byte(i2c_bus_t *bus, uint8_t data)
{
    return i2c_write_byte_ack(bus, data);
}

/* ��ȡ 1 �ֽڲ��ڵ�9λ���� MCU ACK/NACK��ack=1 ��ʾ MCU �� ACK��ack=0 ��ʾ MCU �� NACK�� */
static uint8_t i2c_read_byte(i2c_bus_t *bus, int ack)
{
    uint8_t data = 0;
    for (int i = 0; i < 8; i++)
    {
        data <<= 1;
        data |= i2c_clock_cycle(bus, 1, 1, 0);
    }
    /* ��9λ���������� ACK/NACK��0->���ͱ�ʾ ACK, 1->���ָ߱�ʾ NACK�� */
    i2c_clock_cycle(bus, (ack ? 0 : 1), 0, 1);
    return data;
}

/* ==== �߲������start/stop/read/write ���ֽ� �� ==== */

static void i2c_start(i2c_bus_t *bus)
{
    i2c_sda(bus, 1);
    i2c_scl(bus, 1);
    i2c_delay_us(bus, 5);
    i2c_sda(bus, 0);
    i2c_delay_us(bus, 5);
    i2c_scl(bus, 0);
}

static void i2c_stop(i2c_bus_t *bus)
{
    i2c_sda(bus, 0);
    i2c_delay_us(bus, 5);
    i2c_scl(bus, 1);
    i2c_delay_us(bus, 5);
    i2c_sda(bus, 1);
    i2c_delay_us(bus, 5);
}

/* ���豸�Ĵ���дһ���ֽڣ�����鷵��ֵ�� */
void i2c_write_reg(i2c_bus_t *bus, uint8_t dev, uint8_t reg, uint8_t val)
{
    i2c_start(bus);
    i2c_write_byte(bus, (dev << 1) | 0);
    i2c_write_byte(bus, reg);
    i2c_write_byte(bus, val);
    i2c_stop(bus);
}

/* ���豸�Ĵ���һ���ֽ� */
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
        *buf++ = i2c_read_byte(bus, len ? 1 : 0); /* �������һ���ֽ� NACK */
    }
    i2c_stop(bus);
}

/* ==== ɨ�躯�� ==== */
void i2c_scan(i2c_bus_t *bus)
{
//    rt_kprintf("\r\nScanning I2C bus: SDA=%p.%04X  SCL=%p.%04X\r\n",
//               (void *)bus->sda_port, (unsigned)bus->sda_pin,
//               (void *)bus->scl_port, (unsigned)bus->scl_pin);

//    rt_enter_critical(); /* �����ж� */
    for (uint8_t addr = 0; addr < 128; addr++)
    {
        i2c_start(bus);
        uint8_t ack = i2c_write_byte_ack(bus, (addr << 1) | 0);
        
        if (ack == 0)
        {
            rt_kprintf("I2C device found at 0x%02X\r\n", addr);
        }
				i2c_stop(bus);
        i2c_delay_us(bus, 50);
    }
//    rt_exit_critical(); /* �ָ��ж� */
}

/* ==== ���߳�ʼ�������� GPIO�� ==== */
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

    i2c_sda(bus, 1);
    i2c_scl(bus, 1);

    /* ��֤����״̬ */
    if (i2c_read_sda(bus) != 1 ) {
        rt_kprintf("I2C bus init failed: SDA not high\n");
    }
		if (gpio_get(bus->scl_port, bus->scl_pin) != 1) {
        rt_kprintf("I2C bus init failed: SCL not high\n");
    }
}
/* ==== ʾ����· I2C ���壨����Է��ڱ���ļ����ã� ==== */

i2c_bus_t i2c1 = {
    .scl_port = GPIOB,
    .scl_pin  = GPIO_Pin_10,
    .sda_port = GPIOB,
    .sda_pin  = GPIO_Pin_11,
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


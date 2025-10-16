

#include "platform.h"
#ifndef USER_I2C_V1
/* �û��޸���4����Ϊ��� GPIO ���� */
#define I2C1_SDA_GPIO_PORT GPIOB
#define I2C1_SDA_PIN GPIO_Pin_11
#define I2C1_SCL_GPIO_PORT GPIOB
#define I2C1_SCL_PIN GPIO_Pin_10
#define enter_irq() rt_exit_critical();
#define quit_irq() rt_enter_critical();
#define SDA_HIGH() GPIO_SetBits(I2C1_SDA_GPIO_PORT, I2C1_SDA_PIN)          // �� SDA Ϊ�ߵ�ƽ�����������ߣ�
#define SDA_LOW() GPIO_ResetBits(I2C1_SDA_GPIO_PORT, I2C1_SDA_PIN)         // �� SDA Ϊ�͵�ƽ
#define SCL_HIGH() GPIO_SetBits(I2C1_SCL_GPIO_PORT, I2C1_SCL_PIN)          // �� SCL Ϊ�ߵ�ƽ
#define SCL_LOW() GPIO_ResetBits(I2C1_SCL_GPIO_PORT, I2C1_SCL_PIN);        // �� SCL Ϊ�͵�ƽ
#define SDA_READ() GPIO_ReadInputDataBit(I2C1_SDA_GPIO_PORT, I2C1_SDA_PIN) // ��ȡ SDA ���ŵ�ƽ������0��1��


#define I2C_DELAY() rt_hw_us_delay(2)
/* === ��ʱ�궨�� === */
#define I2C_DELAY_LOW() rt_hw_us_delay(5)  // SCL �͵�ƽʱ��
#define I2C_DELAY_HIGH() rt_hw_us_delay(5) // SCL �ߵ�ƽʱ��
#define I2C_DELAY_ACK() rt_hw_us_delay(30) // ACK�׶�ר����ʱ
/* === �������� === */
// SCL: ����������������????????����������������
// SDA: ?????\_________________
//          ��
//        Start: SDA �ߡ���, SCL ��
//        SCL �ߵ�ƽ�ڼ�SDA �ɸ����͡� ��ʼ�ź�
static void i2c_start(void)
{
    SDA_HIGH();
    SCL_HIGH();
    I2C_DELAY_HIGH(); // ȷ�����߿���
    SDA_LOW();        // START
    I2C_DELAY_HIGH(); // SDA�ȶ�ʱ��
    SCL_LOW();
    I2C_DELAY_LOW();
}
// SCL: ����������������????????����������������
// SDA: _______/?????????
//          ��
//        Stop: SDA �͡���, SCL ��
//        SCL �ߵ�ƽ�ڼ�SDA �ɵ����ߡ� ֹͣ�ź�

static void i2c_stop(void)
{
    SDA_LOW();
    I2C_DELAY_LOW();
    SCL_HIGH();
    I2C_DELAY_HIGH(); // �����ȶ�
    SDA_HIGH();       // STOP
    I2C_DELAY_HIGH();
}

static uint8_t i2c_clock_cycle(uint8_t data, int input_mode, int is_ack_phase)
{
    uint8_t bit = 0;

    if (!input_mode)
        data ? SDA_HIGH() : SDA_LOW();
    else
        SDA_HIGH(); // ����ģʽ���ͷ� SDA��

    I2C_DELAY_LOW(); // SCL�͵�ƽ�׶�

    SCL_HIGH();
    if (is_ack_phase)
        I2C_DELAY_ACK(); // ACK�׶���ʱ����
    else
        I2C_DELAY_HIGH(); // �����ߵ�ƽ�׶�

    if (input_mode)
        bit = SDA_READ(); // �������ȶ������

    SCL_LOW();
    I2C_DELAY_LOW(); // �ٴν���͵�ƽ
    return bit;
}

// Bit:   7   6   5   4   3   2   1   0   ACK
// SDA:   ��?��?��?��?��?��?��?��?��     (���ݵ�ƽ)
// SCL: ����??��??��??��??��??��??��??
// ÿλ��SCL �����ز���
// ��9 λ�ɴӻ����ͱ�ʾACK
static uint8_t i2c_write_byte(uint8_t d)
{
    for (int i = 0; i < 8; i++)
    {
        i2c_clock_cycle(d & 0x80, 0, 0); // ��������λ
        d <<= 1;
    }
    return i2c_clock_cycle(1, 1, 1); // ��9λ��ACK
}
// Bit:   7   6   5   4   3   2   1   0   MCU_ACK
// SDA:   d7  d6  d5  d4  d3  d2  d1  d0   �� (MCU ACK/NACK)
// SCL: ����??��??��??��??��??��??��??��??��??
// SDA �������룬�ӻ��������

// MCU �ڵ�9��ʱ����������SDA��ACK���򱣳ָߣ�NACK��
static uint8_t i2c_read_byte(int ack)
{
    uint8_t d = 0;
    for (int i = 0; i < 8; i++)
    {
        d <<= 1;
        d |= i2c_clock_cycle(1, 1, 0); // ����λ����
    }
    i2c_clock_cycle(!ack, 0, 1); // ��9λMCU��ACK/NACK
    return d;
}

/* === �߲㺯�� === */
/*
��д������
    1 ���ʹӻ���ַ+ дλ
    2 ���ͼĴ�����ַ
    3 ��������
*/
void i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t val)
{
    enter_irq();
    i2c_start();
    i2c_write_byte((dev_addr << 1) | 0); // д����
    i2c_write_byte(reg);
    i2c_write_byte(val);
    i2c_stop();
    quit_irq();
}
/*
����������
    1 ���ʹӻ���ַ+ дλ�����ߴӻ�Ҫ�����ĸ��Ĵ�����
    2 ���ͼĴ�����ַ
    3 �����ظ���ʼ
    4 ���ʹӻ���ַ+ ��λ
    5 ��ȡ����
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
// �����İ�ȫ��д�ֽڲ��ȴ� ACK
#define I2C_ACK_TIMEOUT 50
static uint8_t i2c_write_byte_ack(uint8_t d)
{
    for (int i = 0; i < 8; i++)
    {
        i2c_clock_cycle(d & 0x80, 0, 0); // �������λ
        d <<= 1;
    }
    return i2c_clock_cycle(1, 1, 1); // ��9λ�ͷ�SDA����ACK(0=ACK,1=NACK)
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
        rt_hw_us_delay(50); // ����ʱ
    }
    //		uint8_t buf[2];
    //		i2c_read_bytes(0x15, 0x04, buf, 2);
    //		rt_kprintf("xpos 0x%02X,0x%02X\r\n", buf[0],buf[1]);
    return 0;
}

//INIT_APP_EXPORT(i2c_scan);

#endif

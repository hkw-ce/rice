#include "platform.h"

void i2c_config(I2C_TypeDef *i2c, uint8_t dev_addr, GPIO_TypeDef *GPIOx, uint16_t sck_pin, uint16_t sda_pin)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    I2C_InitTypeDef I2C_InitStruct;

    /* 1. 使能 I2C 时钟 */
   if (i2c == I2C1)
       RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
   else if (i2c == I2C2)
       RCC_APB1PeriphClockCmd(RCC_APB1ENR_I2C2, ENABLE);
#if defined(I2C3)
    else if (i2c == I2C3)
        RCC_APB1PeriphClockCmd(RCC_APB1ENR_I2C3, ENABLE);
#endif

    I2C_DeInit(i2c);

    /* 2. 配置 I2C 参数 */
    I2C_StructInit(&I2C_InitStruct);
    I2C_InitStruct.I2C_Mode = I2C_Mode_MASTER;
    I2C_InitStruct.I2C_OwnAddress = I2C_OWN_ADDRESS;
    I2C_InitStruct.I2C_ClockSpeed = 100000; // 100kHz
    I2C_Init(i2c, &I2C_InitStruct);

    /* 3. 配置目标设备地址 */
    I2C_TargetAddressConfig(i2c, dev_addr);

    /* 4. 使能对应 GPIO 时钟 */
    if (GPIOx == GPIOA)
        RCC_APB2PeriphClockCmd(RCC_AHBENR_GPIOA, ENABLE);
    else if (GPIOx == GPIOB)
        RCC_APB2PeriphClockCmd(RCC_AHBENR_GPIOB, ENABLE);
    else if (GPIOx == GPIOC)
        RCC_APB2PeriphClockCmd(RCC_AHBENR_GPIOC, ENABLE);
    else if (GPIOx == GPIOD)
        RCC_APB2PeriphClockCmd(RCC_AHBENR_GPIOD, ENABLE);
#if defined(GPIOE)
    else if (GPIOx == GPIOE)
        RCC_APB2PeriphClockCmd(RCC_AHBENR_GPIOE, ENABLE);
#endif

    /* 5. 配置复用功能 */
    uint8_t sck_pin_source = 0, sda_pin_source = 0;

    if (sck_pin & GPIO_Pin_6)
        sck_pin_source = GPIO_PinSource6;
    else if (sck_pin & GPIO_Pin_8)
        sck_pin_source = GPIO_PinSource8;
    else if (sck_pin & GPIO_Pin_10)
        sck_pin_source = GPIO_PinSource10;

    if (sda_pin & GPIO_Pin_7)
        sda_pin_source = GPIO_PinSource7;
    else if (sda_pin & GPIO_Pin_9)
        sda_pin_source = GPIO_PinSource9;
    else if (sda_pin & GPIO_Pin_11)
        sda_pin_source = GPIO_PinSource11;

    GPIO_PinAFConfig(GPIOx, sck_pin_source, GPIO_AF_4);
    GPIO_PinAFConfig(GPIOx, sda_pin_source, GPIO_AF_4);

    /* 6. 配置 GPIO 为复用开漏 */
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = sck_pin | sda_pin;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOx, &GPIO_InitStruct);

    /* 7. 使能 I2C */
    I2C_Cmd(i2c, ENABLE);
}

void i2c_tx_polling(I2C_TypeDef *i2c, uint8_t *Buffer, uint8_t Length)
{
    uint8_t i = 0;

    for (i = 0; i < Length; i++)
    {
        I2C_SendData(i2c, Buffer[i]);

        while (RESET == I2C_GetFlagStatus(i2c, I2C_STATUS_FLAG_TFE))
        {
        }
    }
}
void i2c_rx_polling(I2C_TypeDef *i2c, uint8_t *Buffer, uint16_t Length)
{
    uint8_t i = 0;

    for (i = 0; i < Length; i++)
    {
        I2C_ReadCmd(i2c);

        while (RESET == I2C_GetFlagStatus(i2c, I2C_STATUS_FLAG_RFNE))
        {
        }

        Buffer[i] = I2C_ReceiveData(i2c);
    }
}

/**
 * @brief  扫描 I2C 总线上存在的设备
 * @param  i2c       I2C 外设 (比如 I2C2)
 * @param  found     存放找到的设备地址数组
 * @param  max_found 数组最大容量
 * @retval 返回找到的设备个数
 */
//uint8_t i2c_scan(I2C_TypeDef *i2c, uint8_t *found, uint8_t max_found)
//{
//    uint8_t addr;
//    uint8_t count = 0;
//    uint8_t dummy;

//    for (addr = 0x03; addr <= 0x77; addr++)
//    {
//        /* 配置目标地址 */
//        I2C_TargetAddressConfig(i2c, addr);

//        /* 发起一次读命令 */
//        I2C_ReadCmd(i2c);

//        /* 等待接收 FIFO 有数据（设备应答） */
//        int timeout = 10000;
//        while ((RESET == I2C_GetFlagStatus(i2c, I2C_STATUS_FLAG_RFNE)) && (--timeout));

//        if (timeout > 0)
//        {
//            /* 有设备应答 */
//            dummy = I2C_ReceiveData(i2c);
//            if (count < max_found)
//            {
//                found[count++] = addr;
//            }
//        }
//        else
//        {
//            /* 没有设备应答 -> 控制器会自动生成 STOP */
//        }
//    }

//    return count;
//}
//#define MAX_DEVICES   10
//int init_i2c(void)
//{
//	  uint8_t found[MAX_DEVICES];
//    uint8_t num;
//    uint8_t i;
//		i2c_config(I2C2, 0x15, GPIOC, GPIO_Pin_8, GPIO_Pin_9);
//	 /* 2. 扫描总线 */
//    num = i2c_scan(I2C2, found, MAX_DEVICES);

//    /* 3. 打印结果 */
//    if (num == 0)
//    {
//        rt_kprintf("No I2C device found.\r\n");
//    }
//    else
//    {
//        rt_kprintf("Found %d I2C device(s):\r\n", num);
//        for (i = 0; i < num; i++)
//        {
//            rt_kprintf("  - 0x%02X\r\n", found[i]);
//        }
//    }

//  return 0;
//}
//INIT_APP_EXPORT(init_i2c);



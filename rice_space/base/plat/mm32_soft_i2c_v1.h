#ifndef __MM32_SOFT_I2C_V1_H__
#define __MM32_SOFT_I2C_V1_H__

#include <stdint.h>
#include "hal_conf.h"
//#ifdef USER_I2C_V1   

#ifdef __cplusplus
extern "C" {
#endif

// ============================ 类型定义 ============================


// ============================ 前向声明 ============================
// 先声明结构体名称（解决函数指针参数引用自身的问题）
struct i2c_bus_t;

// ============================ 类型定义 ============================

/**
 * @brief 软件 I2C 总线结构体
 */
typedef struct i2c_bus_t
{
    GPIO_TypeDef *scl_port;  // SCL引脚端口
    uint16_t scl_pin;        // SCL引脚编号
    GPIO_TypeDef *sda_port;  // SDA引脚端口
    uint16_t sda_pin;        // SDA引脚编号

    void (*delay_us)(uint32_t us);   // 延时函数
    void (*set_scl)(struct i2c_bus_t *bus, uint8_t level);
    void (*set_sda)(struct i2c_bus_t *bus, uint8_t level);
    uint8_t (*read_sda)(struct i2c_bus_t *bus);
} i2c_bus_t;




// ============================ 函数声明 ============================
extern i2c_bus_t i2c1;
/**
 * @brief 初始化 I2C 软件总线（配置 GPIO）
 */
void i2c_bus_init(i2c_bus_t *bus);

/**
 * @brief 初始化 I2C 硬件参数（封装）
 */
void i2c_init(i2c_bus_t *i2c,
              GPIO_TypeDef *scl_port, uint16_t scl_pin,
              GPIO_TypeDef *sda_port, uint16_t sda_pin);


/**
 * @brief 写多字节
 */
void i2c_write_bytes(i2c_bus_t *i2c, uint8_t dev, uint8_t reg, const uint8_t *buf, uint8_t len);

/**
 * @brief 读多字节
 */
void i2c_read_bytes(i2c_bus_t *i2c, uint8_t dev, uint8_t reg, uint8_t *buf, uint8_t len);

/**
 * @brief 扫描 I2C 设备地址
 */
void i2c_scan(i2c_bus_t *bus);

#ifdef __cplusplus
}
#endif
//#endif

#endif /* __MM32_SOFT_I2C_V1_H__ */



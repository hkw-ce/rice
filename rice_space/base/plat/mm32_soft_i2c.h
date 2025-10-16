#ifndef MM32_SOFT_I2C_H
#define MM32_SOFT_I2C_H

#if 0 // USER_I2C_V1

#include <stdint.h>
#include <stdbool.h>

// void i2c_start(void);
// void i2c_stop(void);
// bool i2c_write(uint8_t data);
// void i2c_scan(void);
void i2c_init(void);
uint8_t i2c_read(bool ack);
void i2c_read_bytes(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len);
void i2c_write_bytes(uint8_t dev_addr, uint8_t reg, const uint8_t *buf, uint8_t len);
int i2c_scan(void);
#endif
#endif
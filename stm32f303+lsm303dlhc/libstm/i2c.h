#ifndef I2C_H
#define I2C_H

#include <stm32f3xx.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "gpio.h"

void i2c_init(I2C_TypeDef *i2c);
void i2c_write(I2C_TypeDef *i2c, uint8_t address, uint8_t *buffer, uint8_t n);
void i2c_read(I2C_TypeDef *i2c, uint8_t address, uint8_t *buffer, uint8_t n);

#endif

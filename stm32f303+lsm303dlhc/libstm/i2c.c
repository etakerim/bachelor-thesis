#include "i2c.h"

void i2c_init(I2C_TypeDef *i2c)
{
    clear_bits(i2c->CR1, I2C_CR1_PE);
    set_bits(i2c->TIMINGR, 0x1 << I2C_TIMINGR_PRESC_Pos);
    set_bits(i2c->TIMINGR, 0x13 << I2C_TIMINGR_SCLL_Pos);
    set_bits(i2c->TIMINGR, 0xf << I2C_TIMINGR_SCLL_Pos);
    set_bits(i2c->TIMINGR, 0x2 << I2C_TIMINGR_SDADEL_Pos);
    set_bits(i2c->TIMINGR, 0x4 << I2C_TIMINGR_SCLDEL_Pos);
    set_bits(i2c->CR1, I2C_CR1_PE);
}

void i2c_write(I2C_TypeDef *i2c, uint8_t address, uint8_t *buffer, uint8_t n)
{
    clear_bits(i2c->CR2, I2C_CR2_ADD10);            // 7-bit addresses
    clear_bits(i2c->CR2, I2C_CR2_RD_WRN);           // Write
    clear_bits(i2c->CR2, I2C_CR2_SADD);             // Set Address
    set_bits(i2c->CR2, (address << 1) << I2C_CR2_SADD_Pos); 
    clear_bits(i2c->CR2, I2C_CR2_NBYTES);           // Set number of bytes
    set_bits(i2c->CR2, n << I2C_CR2_NBYTES_Pos);
    set_bits(i2c->CR2, I2C_CR2_AUTOEND);            // Autogenerate STOP after NBYTES
    
    set_bits(i2c->CR2, I2C_CR2_START);              // Send start
    while (n--) {
        while (!(i2c->ISR & I2C_ISR_TXIS))          // Wait for byte to be send
            ;
        i2c->TXDR = *buffer++;                       // Send byte
    }
}

void i2c_read(I2C_TypeDef *i2c, uint8_t address, uint8_t *buffer, uint8_t n)
{
    clear_bits(i2c->CR2, I2C_CR2_ADD10);            // 7-bit addresses
    set_bits(i2c->CR2, I2C_CR2_RD_WRN);             // Read
    clear_bits(i2c->CR2, I2C_CR2_SADD);             // Set address
    set_bits(i2c->CR2, (address << 1) << I2C_CR2_SADD_Pos);
    clear_bits(i2c->CR2, I2C_CR2_NBYTES);           // Set number of bytes
    set_bits(i2c->CR2, n << I2C_CR2_NBYTES_Pos);

    set_bits(i2c->CR2, I2C_CR2_START);
    set_bits(i2c->CR2, I2C_CR2_AUTOEND);

    for (size_t i = 0; i < n; i++) {
        while (!(i2c->ISR & I2C_ISR_RXNE))
            ;
        buffer[i] = i2c->TXDR;
    }
}

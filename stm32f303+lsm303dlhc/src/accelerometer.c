/* 
 * Hardware: STM32F3 Discovery Board (MCU: STM32F303VCT6, Unit: LSM303DLHC) 
 * Author: Miroslav Hájek
 * 
 * TODO: Fix bug - I2C stop is probably not send
 * TODO: Implement sampling frequency, bluetooth and algorithms
 */
#include <stm32f3xx.h>
#include <gpio.h>
#include <usart.h>
#include <i2c.h>

#define ACCEL_ADDR      0x19
#define ACC_CTRL_REG1_A 0x20
#define ACC_OUT_X_L_A   0x28
#define ACC_OUT_X_H_A   0x29

// OUT_X_L_A (28h), OUT_X_H_A (29h)
// OUT_Y_L_A (2Ah), OUT_Y_H_A (2Bh)
// OUT_Z_L_A (2Ch), OUT_Z_H_A (2Dh)

void itoa(char *str, int16_t x)
{
    int i;
    bool negative = (x < 0);
    
    if (x == 0) {
        str[0] = '0'; str[1] = '\0';
        return;
    }

    for (i = 0; x > 0; x /= 10, i++) {
        str[i] = x % 10 + '0';
    }
    if (negative)
        str[i++] = '-';
    str[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char tmp = str[i-j-1];
        str[i-j-1] = str[j];
        str[j] = tmp;
    }
}

int main(void)
{
    // USART
    rcc_ahb_enable(RCC_AHBENR_GPIODEN, true);
    rcc_apb1_enable(RCC_APB1ENR_USART2EN, true);

    struct gpio tx = {GPIOD, 5};
    struct gpio rx = {GPIOD, 6};

    gpio_mode(&tx, GPIO_AF);
    gpio_alternate(&tx, 7);
    gpio_mode(&rx, GPIO_AF);
    gpio_alternate(&rx, 7);

    usart_baudrate(USART2, 9600, false);
    usart_databits(USART2, USART_DATA_8);
    usart_stopbits(USART2, USART_STOP_1);
    usart_parity(USART2, USART_PARITY_NONE);
    usart_enable(USART2, true);
    NVIC_EnableIRQ(USART2_IRQn);

    // I2C
    rcc_ahb_enable(RCC_AHBENR_GPIOBEN, true);
    rcc_apb1_enable(RCC_APB1ENR_I2C1EN, true);

    struct gpio scl = {GPIOB, 6};
    struct gpio sda = {GPIOB, 7};

    gpio_mode(&scl, GPIO_AF);
    gpio_otype(&scl, GPIO_OPENDRAIN);
    gpio_ospeed(&scl, GPIO_HIGH_SPEED);
    gpio_alternate(&scl, 4);

    gpio_mode(&sda, GPIO_AF);
    gpio_otype(&sda, GPIO_OPENDRAIN);
    gpio_ospeed(&sda, GPIO_HIGH_SPEED);
    gpio_alternate(&sda, 4);

    i2c_init(I2C1);
    //NVIC_EnableIRQ(I2C1_EV_IRQn);


    int16_t acc_x = 0;

    uint8_t data;
    uint8_t ini[2] = {ACC_CTRL_REG1_A, 0x27};
    char num[50];
    i2c_write(I2C1, ACCEL_ADDR, &ini, 2);

    while (1) {
        usart_print("loop\n");
        uint8_t cmd = ACC_OUT_X_L_A;
        i2c_write(I2C1, ACCEL_ADDR, &cmd, 1);
        usart_print("write\n");
        i2c_read(I2C1, ACCEL_ADDR, &data, 1);
        usart_print("read\n");
        acc_x = data;

        cmd = ACC_OUT_X_H_A;
        i2c_write(I2C1, ACCEL_ADDR, &cmd, 1);
        i2c_read(I2C1, ACCEL_ADDR, &data, 1);
        acc_x |= ((uint16_t)data << 8);

        itoa(&num, acc_x);
        usart_print("ACC X: ");
        usart_print(&num);
        usart_putc('\n');
    }
}



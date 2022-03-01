#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "soc/gpio_struct.h"
#include "soc/uart_struct.h"

#include "esp_dsp.h"


/*
    SPI: Akcelerometer LSM9DS1
    ESP-DSP: spracoavnie
    UART: OpenLog
    Wifi+HTTP/MQTT: server
*/


#define IMU_DEN         18
#define IMU_SDO         23
#define IMU_SDA         19
#define IMU_SCL         21
#define IMU_CS_AG       22
#define IMU_CS_M        14
#define IMU_INT1_M      12
#define IMU_DRDY_M      4
#define IMU_INT2_AG     16
#define IMU_INT1_AG     17


typedef struct {
    gpio_num_t clk, miso, mosi, xgcs, mcs;
    gpio_num_t int1, int2, en_data;
    spi_device_handle_t dev;
} IMU_LSM9DS1;

typedef struct {
    int16_t x, y, z;
} Vector;

#define LSM9DS1_INT_GEN_CFG_XL      0x06
#define LSM9DS1_INT1_CTRL           0x0C
#define LSM9DS1_INT2_CTRL           0x0D
#define LSM9DS1_CTRL_REG8           0x22
#define LSM9DS1_WHO_AM_I_XG         0x0F
#define LSM9DS1_CTRL_REG5_XL        0x1F
#define LSM9DS1_CTRL_REG6_XL        0x20
#define LSM9DS1_OUT_X_L_XL          0x28


#define LSM9DS1_XG_ID               0x68  // 0x3D
#define LSM9DS1_ACCELRANGE_2G       (0 << 3)
#define LSM9DS1_ACCELRANGE_16G      (1 << 3)
#define LSM9DS1_ACCELRANGE_4G       (2 << 3)
#define LSM9DS1_ACCELRANGE_8G       (3 << 3)
#define LSM9DS1_ACCELRANGE_MASK     LSM9DS1_ACCELRANGE_8G

#define LSM9DS1_ODR_MASK            5
#define LSM9DS1_ODR_OFF             (0 << LSM9DS1_ODR_MASK)
#define LSM9DS1_ODR_10HZ            (1 << LSM9DS1_ODR_MASK)
#define LSM9DS1_ODR_50HZ            (2 << LSM9DS1_ODR_MASK)
#define LSM9DS1_ODR_119HZ           (3 << LSM9DS1_ODR_MASK)
#define LSM9DS1_ODR_238HZ           (4 << LSM9DS1_ODR_MASK)
#define LSM9DS1_ODR_476HZ           (5 << LSM9DS1_ODR_MASK)
#define LSM9DS1_ODR_952HZ           (6 << LSM9DS1_ODR_MASK)

#define LSM9DS1_REG8_IF_ADD_INC     4
#define LSM9DS1_REG8_SW_RESET       1


void spi_send(spi_device_handle_t spi, uint8_t reg, uint8_t value)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    uint8_t buffer[2] = {reg & 0x7F, value};
    t.length = 8 * sizeof(buffer);
    t.tx_buffer = buffer;

    spi_device_transmit(spi, &t);
}

void spi_read_buffer(spi_device_handle_t spi, uint8_t reg, uint8_t len, uint8_t *buffer)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    uint8_t tx_buffer[1] = {reg | 0x80};
    t.length = 8 * sizeof(tx_buffer);
    t.rxlength = 8 * len;
    t.tx_buffer = tx_buffer;
    t.rx_buffer = buffer;

    spi_device_transmit(spi, &t);
}

uint8_t spi_recv(spi_device_handle_t spi, uint8_t reg)
{
    uint8_t buffer[1];
    spi_read_buffer(spi, reg, 1, buffer);
    return buffer[0];
}

void imu_set_accel(IMU_LSM9DS1 *imu, uint8_t precision)
{
    uint8_t reg = spi_recv(imu->dev, LSM9DS1_CTRL_REG6_XL);
    reg &= ~LSM9DS1_ACCELRANGE_MASK;
    reg |= precision;
    spi_send(imu->dev, LSM9DS1_CTRL_REG6_XL, reg);
}

bool imu_config(IMU_LSM9DS1 *imu)
{
    /*
    gpio_config_t int1_config = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1 << imu->int1),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE
    };
    gpio_config(&int1_config);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(imu->int1, int1_ready, NULL);
    */

    spi_bus_config_t spi_bus = {
        .miso_io_num = imu->miso,
        .mosi_io_num = imu->mosi,
        .sclk_io_num = imu->clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };
    spi_bus_initialize(SPI2_HOST, &spi_bus, SPI_DMA_DISABLED);

    spi_device_interface_config_t spi_iface = {
        .clock_speed_hz=4000000,
        .mode=0,
        .spics_io_num=imu->xgcs,
        .queue_size=3
    };
    spi_bus_add_device(SPI2_HOST, &spi_iface, &imu->dev);

    spi_send(imu->dev, LSM9DS1_CTRL_REG8, 
             LSM9DS1_REG8_IF_ADD_INC | LSM9DS1_REG8_SW_RESET);
    vTaskDelay(10 / portTICK_RATE_MS);
    
    uint8_t id = spi_recv(imu->dev, LSM9DS1_WHO_AM_I_XG);
    if (id != LSM9DS1_XG_ID)
        return false;

    spi_send(imu->dev, LSM9DS1_CTRL_REG5_XL, 0x38);  // Enable X,Y,Z axis
    spi_send(imu->dev, LSM9DS1_CTRL_REG6_XL, LSM9DS1_ODR_952HZ);

    imu_set_accel(imu, LSM9DS1_ACCELRANGE_2G);
    return true;
}

void imu_acceleration(IMU_LSM9DS1 *imu, Vector *a)
{
    uint8_t buffer[6];
    spi_read_buffer(imu->dev, 0x80 | LSM9DS1_OUT_X_L_XL, 6, buffer);

    uint8_t xlo = buffer[0];
    int16_t xhi = buffer[1];

    uint8_t ylo = buffer[2];
    int16_t yhi = buffer[3];
    
    uint8_t zlo = buffer[4];
    int16_t zhi = buffer[5];

    a->x = (xhi << 8) | xlo;
    a->y = (yhi << 8) | ylo;
    a->z = (zhi << 8) | zlo;
}


#define OPENLOG_UART        0
#define OPENLOG_TX          13
#define OPENLOG_RX          5
#define OPEN_LOG_BAUDRATE   115200
#define UART_BUFFER         1024

#define OPENLOG_VCC     25
#define OPENLOG_VCC1    26
#define OPENLOG_VCC2    27

void openlog_setup(void)
{
    gpio_config_t openlog_vcc = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1 << OPENLOG_VCC),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    gpio_config(&openlog_vcc);
    gpio_set_level(OPENLOG_VCC, 1);
    vTaskDelay(10000 / portTICK_RATE_MS);

    uart_config_t uart_config = {
        .baud_rate = OPEN_LOG_BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };

    uart_driver_install(OPENLOG_UART, UART_BUFFER * 2, 0, 0, NULL, 0);
    uart_param_config(OPENLOG_UART, &uart_config);
    uart_set_pin(OPENLOG_UART, OPENLOG_TX, OPENLOG_RX, 
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}


void app_main(void)
{
    Vector accel;
    char buffer[50];

    openlog_setup();

    IMU_LSM9DS1 imu = {
        .clk = IMU_SCL,
        .miso = IMU_SDO,
        .mosi = IMU_SDA,
        .xgcs = IMU_CS_AG,
        .mcs = IMU_CS_M,
        .int1 = IMU_INT1_AG,
        .int2 = IMU_INT2_AG,
        .en_data = IMU_DEN
    };
    if (!imu_config(&imu)) {
        while (1);
    }
    // imu_threshold_interrupt();

    while (1) {
        imu_acceleration(&imu, &accel);
        sprintf(buffer, "%d %d %d\n", accel.x, accel.y, accel.z);
        uart_write_bytes(OPENLOG_UART, buffer, strlen(buffer));
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    spi_bus_remove_device(imu.dev);
}

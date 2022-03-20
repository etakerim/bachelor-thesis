#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "inertial_unit.h"


typedef enum IMURegister {
    IMU_ACT_THS = 0x04,             // Activity threshold register.
    IMU_ACT_DUR = 0x05,             // Inactivity duration register
    IMU_INT_GEN_CFG_XL = 0x06,      // Linear acceleration sensor interrupt generator configuration register
    IMU_INT_GEN_THS_X_XL = 0x07,    // Linear acceleration sensor interrupt threshold register
    IMU_INT_GEN_THS_Y_XL = 0x08,    // Linear acceleration sensor interrupt threshold register
    IMU_INT_GEN_THS_Z_XL = 0x09,    // Linear acceleration sensor interrupt threshold register
    IMU_INT_GEN_DUR_XL = 0x0A,      // Linear acceleration sensor interrupt duration register
    IMU_REFERENCE_G = 0x0B,         // Angular rate sensor reference value register for digital high-pass filter
    IMU_INT1_CTRL = 0x0C,           // INT1_A/G pin control register
    IMU_INT2_CTRL = 0x0D,           // INT2_A/G pin control register
    IMU_WHO_AM_I = 0x0F,            // Who_AM_I register
    IMU_CTRL_REG1_G = 0x10,         // Angular rate sensor Control Register 1
    IMU_CTRL_REG2_G = 0x11,         // Angular rate sensor Control Register 2
    IMU_CTRL_REG3_G = 0x12,         // Angular rate sensor Control Register 3
    IMU_ORIENT_CFG_G = 0x13,        // Angular rate sensor sign and orientation register
    IMU_INT_GEN_SRC_G = 0x14,       // Angular rate sensor interrupt source register
    IMU_OUT_TEMP_L = 0x15,          // Temperature data output register
    IMU_OUT_TEMP_H = 0x16,          
    IMU_STATUS_REG = 0x17,          // Status register
    IMU_OUT_X_L_G = 0x18,           // Angular rate sensor pitch axis (X)
    IMU_OUT_X_H_G = 0x19,
    IMU_OUT_Y_L_G = 0x1A,           // Angular rate sensor roll axis (Y)
    IMU_OUT_Y_H_G = 0x1B,
    IMU_OUT_Z_L_G = 0x1C,           // Angular rate sensor Yaw axis (Z)
    IMU_OUT_Z_H_G = 0x1D,
    IMU_CTRL_REG4 = 0x1E,           // Angular rate sensor Control Register 3
    IMU_CTRL_REG5_XL = 0x1F,        // Linear acceleration sensor Control Register 5
    IMU_CTRL_REG6_XL = 0x20,        // Linear acceleration sensor Control Register 6
    IMU_CTRL_REG7_XL = 0x21,        // Linear acceleration sensor Control Register 7
    IMU_CTRL_REG8 = 0x22,           // Control register 8
    IMU_CTRL_REG9 = 0x23,           // Control register 9
    IMU_CTRL_REG10 = 0x24,          // Control register 10 Self-test
    IMU_INT_GEN_SRC_XL = 0x26,      // Linear acceleration sensor interrupt source register
    IMU_STATUS_REG_2 = 0x27,        // Status register 2
    IMU_OUT_X_L_XL = 0x28,          // Linear acceleration sensor X-axis output register
    IMU_OUT_X_H_XL = 0x29,
    IMU_OUT_Y_L_XL = 0x2A,          // Linear acceleration sensor Y-axis output register
    IMU_OUT_Y_H_XL = 0x2B,
    IMU_OUT_Z_L_XL = 0x2C,          // Linear acceleration sensor Z-axis output register
    IMU_OUT_Z_H_XL = 0x2D,
    IMU_FIFO_CTRL = 0x2E,           // FIFO control register
    IMU_FIFO_SRC = 0x2F,            // FIFO status control register
} IMURegister;


typedef enum IMURangeRegister {
    IMU_ACCELRANGE_2G_REG = 0x0,
    IMU_ACCELRANGE_4G_REG = 0x10,
    IMU_ACCELRANGE_8G_REG = 0x18,
    IMU_ACCELRANGE_16G_REG = 0x8
} IMURangeRegister;


#define SPI_BUS_FREQUENCY       4000000
#define IMU_XG_ID               0x68
#define IMU_ACCELRANGE_MASK     IMU_ACCELRANGE_8G_REG
#define IMU_ODR_MASK            IMU_ODR_952HZ

// Accelerometer interrupt generator (INT1/2_A/G)
#define IMU_INT_IG_XL           (1 << 6)
// Accelerometer data ready on (INT1/2_A/G)
#define IMU_INT_DRDY_XL         (1 << 0)
#define IMU_REG9_DRDY_mask      (1 << 3)

#define IMU_REG5_Zen_XL         (1 << 5)
#define IMU_REG5_Yen_XL         (1 << 4)
#define IMU_REG5_Xen_XL         (1 << 3)
#define IMU_REG8_IF_ADD_INC     4
#define IMU_REG8_SW_RESET       1



static void spi_send(spi_device_handle_t spi, uint8_t reg, uint8_t value)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    uint8_t buffer[2] = {reg & 0x7F, value};
    t.length = 8 * sizeof(buffer);
    t.tx_buffer = buffer;

    spi_device_transmit(spi, &t);
}

static void spi_read_buffer(spi_device_handle_t spi, uint8_t reg, uint8_t len, uint8_t *buffer)
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

static uint8_t spi_recv(spi_device_handle_t spi, uint8_t reg)
{
    uint8_t buffer[1];
    spi_read_buffer(spi, reg, 1, buffer);
    return buffer[0];
}

static void imu_isr_install(InertialUnit *imu)
{
    gpio_config_t interrupt_pin = {
        .intr_type = GPIO_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1 << imu->int1),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };

    gpio_install_isr_service(0);
    if (imu->isr_int1 != NULL) {
        spi_send(imu->dev, IMU_INT1_CTRL, IMU_INT_DRDY_XL);
        spi_send(imu->dev, IMU_CTRL_REG9, IMU_REG9_DRDY_mask);

        gpio_config(&interrupt_pin);
        gpio_isr_handler_add(imu->int1, imu->isr_int1, NULL);
    }
    if (imu->isr_int2 != NULL) {
        spi_send(imu->dev, IMU_INT2_CTRL, IMU_INT_IG_XL);

        interrupt_pin.pin_bit_mask = (1 << imu->int2);
        gpio_config(&interrupt_pin);
        gpio_isr_handler_add(imu->int2, imu->isr_int2, NULL);

    }
}

bool imu_setup(InertialUnit *imu)
{
    spi_bus_config_t spi_bus = {
        .miso_io_num = imu->miso,
        .mosi_io_num = imu->mosi,
        .sclk_io_num = imu->clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32
    };
    spi_bus_initialize(imu->spi, &spi_bus, SPI_DMA_DISABLED);

    spi_device_interface_config_t spi_iface = {
        .clock_speed_hz=SPI_BUS_FREQUENCY,
        .flags=SPI_DEVICE_HALFDUPLEX,
        .mode=0,
        .spics_io_num=imu->xgcs,
        .queue_size=1
    };
    spi_bus_add_device(imu->spi, &spi_iface, &imu->dev);

    spi_send(imu->dev, IMU_CTRL_REG8, IMU_REG8_IF_ADD_INC | IMU_REG8_SW_RESET);
    vTaskDelay(10 / portTICK_RATE_MS);

    uint8_t id = spi_recv(imu->dev, IMU_WHO_AM_I);
    if (id != IMU_XG_ID)
        return false;

    spi_send(imu->dev, IMU_CTRL_REG5_XL,
             IMU_REG5_Zen_XL | IMU_REG5_Yen_XL | IMU_REG5_Xen_XL );
    imu_output_data_rate(imu, IMU_ODR_952HZ);
    imu_acceleration_range(imu, IMU_2G);
    // imu_isr_install(imu);   FIFO configuration

    return true;
}

void imu_output_data_rate(InertialUnit *imu, AccelerationODR odr)
{
    uint8_t reg = spi_recv(imu->dev, IMU_CTRL_REG6_XL);
    reg = (reg & ~IMU_ODR_MASK) | odr;
    spi_send(imu->dev, IMU_CTRL_REG6_XL, reg);
}

void imu_acceleration_range(InertialUnit *imu, AccelerationRange range)
{
    uint8_t reg = spi_recv(imu->dev, IMU_CTRL_REG6_XL);
    IMURangeRegister regval = IMU_ACCELRANGE_2G_REG;

    switch (range) {
        case IMU_2G:
            regval = IMU_ACCELRANGE_2G_REG;
            imu->precision = IMU_MG_LSB_2G;
            break;
        case IMU_4G:
            regval = IMU_ACCELRANGE_4G_REG;
            imu->precision = IMU_MG_LSB_4G;
            break;
        case IMU_8G:
            regval = IMU_ACCELRANGE_8G_REG;
            imu->precision = IMU_MG_LSB_8G;
            break;
        case IMU_16G:
            regval = IMU_ACCELRANGE_16G_REG;
            imu->precision = IMU_MG_LSB_16G;
            break;
        default:
            return;
    }

    reg = (reg & ~IMU_ACCELRANGE_MASK) | regval;
    spi_send(imu->dev, IMU_CTRL_REG6_XL, reg);
}


void imu_acceleration(InertialUnit *imu, float *x, float *y, float *z)
{
    uint8_t buffer[6];
    spi_read_buffer(imu->dev, 0x80 | IMU_OUT_X_L_XL, 6, buffer);

    uint8_t xlo = buffer[0];
    int16_t xhi = buffer[1];

    uint8_t ylo = buffer[2];
    int16_t yhi = buffer[3];
    
    uint8_t zlo = buffer[4];
    int16_t zhi = buffer[5];

    xhi = (xhi << 8) | xlo;
    yhi = (yhi << 8) | ylo;
    zhi = (zhi << 8) | zlo;

    if (x != NULL) *x = xhi * imu->precision * G_CONSTANT;
    if (y != NULL) *y = yhi * imu->precision * G_CONSTANT;
    if (z != NULL) *z = zhi * imu->precision * G_CONSTANT;
}
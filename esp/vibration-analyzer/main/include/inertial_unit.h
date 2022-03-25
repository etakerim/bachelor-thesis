#ifndef INTERTIAL_UNIT_H
#define INTERTIAL_UNIT_H

#include <stdbool.h>
#include <stdint.h>

#include "hal/gpio_types.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

#define G_CONSTANT      9.80665  / 1000


typedef enum AccelerationRange {
    IMU_2G,
    IMU_4G,
    IMU_8G,
    IMU_16G,
    IMU_RANGE_COUNT
} AccelerationRange;

typedef float AccelerationPrecision;
#define IMU_MG_LSB_2G       0.061
#define IMU_MG_LSB_4G       0.122
#define IMU_MG_LSB_8G       0.244
#define IMU_MG_LSB_16G      0.732

typedef enum AccelerationODR {
    IMU_ODR_10HZ = 0x20,
    IMU_ODR_50HZ = 0x40,
    IMU_ODR_119HZ = 0x60,
    IMU_ODR_238HZ = 0x80,
    IMU_ODR_476HZ = 0xA0,
    IMU_ODR_952HZ = 0xE0
} AccelerationODR;

typedef struct {
    gpio_num_t clk;
    gpio_num_t miso;
    gpio_num_t mosi;
    gpio_num_t xgcs;
    gpio_num_t mcs;
    gpio_num_t int1;
    gpio_num_t int2;
    gpio_num_t en_data;
    gpio_isr_t isr_int1;
    gpio_isr_t isr_int2;
    spi_host_device_t spi;
    spi_device_handle_t dev;
    AccelerationPrecision precision;
} InertialUnit;

bool imu_setup(InertialUnit *imu);
void imu_acceleration_range(InertialUnit *imu, AccelerationRange range);
void imu_output_data_rate(InertialUnit *imu, uint16_t fs);
void imu_acceleration(InertialUnit *imu, float *x, float *y, float *z);


#endif
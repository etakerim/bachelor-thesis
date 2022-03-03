#ifndef INTERTIAL_UNIT_H
#define INTERTIAL_UNIT_H

#include <stdbool.h>
#include <stdint.h>

#include "hal/gpio_types.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

typedef struct {
    uint8_t spi;
    gpio_num_t clk;
    gpio_num_t miso;
    gpio_num_t mosi; 
    gpio_num_t xgcs; 
    gpio_num_t mcs;
    gpio_num_t int1; 
    gpio_num_t int2;
    gpio_num_t en_data;
    spi_host_device_t spi_host;
    spi_device_handle_t dev;
} InertialUnit;

typedef struct {
    int16_t x, y, z;
} Vector;

typedef enum AccelerationRange {
    IMU_ACCELRANGE_2G = 0x0,
    IMU_ACCELRANGE_4G =  0x10,
    IMU_ACCELRANGE_8G =  0x18,
    IMU_ACCELRANGE_16G = 0x8
} AccelerationRange;

typedef enum AccelerationODR {
    IMU_ODR_10HZ = 0x20,
    IMU_ODR_50HZ = 0x40,
    IMU_ODR_119HZ = 0x60,
    IMU_ODR_238HZ = 0x80,
    IMU_ODR_476HZ = 0xA0,
    IMU_ODR_952HZ = 0xE0
} AccelerationODR;


bool imu_setup(InertialUnit *imu);
void imu_acceleration_range(InertialUnit *imu, AccelerationRange range);
void imu_output_data_rate(InertialUnit *imu, AccelerationODR odr);
void imu_acceleration(InertialUnit *imu, Vector *acceleration);


#endif INTERTIAL_UNIT_H
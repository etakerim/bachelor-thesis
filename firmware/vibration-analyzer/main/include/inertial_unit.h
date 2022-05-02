#ifndef INTERTIAL_UNIT_H
#define INTERTIAL_UNIT_H

#include <stdbool.h>
#include <stdint.h>

#include "hal/gpio_types.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

/** 
 * @defgroup imu Akcelerometer
 * @brief Adaptér pre SPI rozhranie senzora LSM9DS1 lineárnej 3D akcelerácie (IMU).
 * @{
 */

/**
 * @brief Konverzný faktor pre premenu tiažového zrýchlenia (preťaženia) z mili-g na m/s^2
 */
#define G_CONSTANT      9.80665  / 1000

/**
 * @brief Dynamický rozsah akcelerometra v g
 */
typedef enum AccelerationRange {
    IMU_2G,             /**< Rozsah: \f$ \pm 2\;\mathrm{g}  = \pm 19.6133 \;\mathrm{m/s^2} \f$ */
    IMU_4G,             /**< Rozsah: \f$ \pm 4\;\mathrm{g} = \pm 39.2266 \;\mathrm{m/s^2} \f$ */
    IMU_8G,             /**< Rozsah: \f$ \pm 8\;\mathrm{g}  = \pm 78.4532 \;\mathrm{m/s^2} \f$ */
    IMU_16G,            /**< Rozsah: \f$ \pm 16\;\mathrm{g}  = \pm 156.9064 \;\mathrm{m/s^2} \f$ */
    IMU_RANGE_COUNT
} AccelerationRange;

/**
 * @brief Citlivosť akcelerometra podľa dynamického rozsahu v mg/LSB
 */
typedef float AccelerationPrecision;

/**
 * @brief Inerciálna meracia jednotka
 */
typedef struct {
    gpio_num_t clk;         /**< @brief GPIO pin SPI hodinového signálu */
    gpio_num_t miso;        /**< @brief GPIO pin SPI Master In Slave Out */
    gpio_num_t mosi;        /**< @brief GPIO pin SPI Master Out Slave In */
    gpio_num_t xgcs;        /**< @brief GPIO pin SPI Chip select akcelometra a gyroskopu */
    gpio_num_t mcs;         /**< @brief GPIO pin SPI Chip select magnetometra */
    gpio_num_t int1;        /**< @brief GPIO vstup prerušenia č.1 */
    gpio_num_t int2;        /**< @brief GPIO vstup prerušenia č.2 */
    gpio_num_t en_data;     /**< @brief GPIO vstup príznaku pripravených dát */
    gpio_isr_t isr_int1;    /**< @brief Podprogram prerušenia pre INT č.1 */
    gpio_isr_t isr_int2;    /**< @brief Podprogram prerušenia pre INT č.2 */
    spi_host_device_t spi;  /**< @brief SPI zbernica */
    spi_device_handle_t dev;   /**< @brief SPI zariadenie pre akcelerometer */
    AccelerationPrecision precision; /**< @brief Zistená citlivosť akcelerometra v mg/LSB */
} InertialUnit;

/**
 * @brief Inicializácia senzora lineárnej akcelerácie
 * 
 * @param  imu   Senzor
 * @return Úspešnosť inicializácie senzora
 */
esp_err_t imu_setup(InertialUnit *imu);

/**
 * @brief Nastavenie dynamického rozsah lineárneho 3D akcelerometra v g
 * 
 * @param  imu      Senzor
 * @param  range    Dynamický rozsah akcelerometra
 */
void imu_acceleration_range(InertialUnit *imu, AccelerationRange range);

/**
 * @brief Nastavenie výstupného dátového toku (ODR) akcelerometra podľa vzorkovanej frekvencie
 * 
 * @param  imu      Senzor
 * @param  fs       Vzorkovacia frekvencia v Herzoch. Hardvér povoľuje max. ODR 956 Hz
 */
void imu_output_data_rate(InertialUnit *imu, uint16_t fs);

/**
 * @brief Meranie aktuálnej hodnoty 3D akcelerácie v m/s^2
 * 
 * @param       imu     Senzor
 * @param[out]  x       Zrýchlenie v osi x. Rozsah je podľa nastavenia dynamického rozsahu.
 * @param[out]  y       Zrýchlenie v osi y
 * @param[out]  z       Zrýchlenie v osi z
 */
void imu_acceleration(InertialUnit *imu, float *x, float *y, float *z);

/** @} */

#endif

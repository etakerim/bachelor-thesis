#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_dsp.h"
#include "mpack.h"

#include "inertial_unit.h"
#include "peripheral.h"
#include "pipeline.h"


TaskHandle_t sampler;

static void imu_isr_sample_ready(void *args)
{
    BaseType_t higher_priority_woken;
    vTaskNotifyGiveFromISR(sampler, &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}

static InertialUnit imu = {
    .spi = SPI2_HOST,
    .clk = 21,
    .miso = 23,
    .mosi = 19,
    .xgcs = 22,
    .mcs = 14,
    .int1 = 17,
    .int2 = 16,
    .en_data = 18,
    .isr_int1 = imu_isr_sample_ready
    //  .isr_int2 = imu_isr_above_threshold
};
static OpenLog logger = {
    .vcc = 25,
    .uart = 0, 
    .rx = 5,
    .tx = 13,
    .baudrate = 9600,
    .buffer = 1024
};


void sampler_task()
{
    Vector accel;

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        imu_acceleration(&imu, &accel);
        printf("%d %d %d\n", accel.x, accel.y, accel.z);
        //xQueueSendToBack(samples, &accel, WAIT_TICKS);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}


void app_main(void)
{
    if (!imu_setup(&imu)) while (1);
    openlog_setup(&logger);

    // nvs_load_config();
    // peripheral_setup();
    // wifi_connect();
    // mqtt_setup("mqtt://mqtt.eclipseprojects.io");
    // samples = xQueueCreate(QUEUE_SIZE, sizeof(uint16_t));
    vTaskDelay(10000 / portTICK_RATE_MS);

    xTaskCreate(sampler_task, "sampler_task", 1024, NULL, 1, &sampler);
}

/*
typedef struct {
    uint16_t frequency;
    uint8_t dynamic_range;
    uint16_t threshold;
    uint16_t buffer_length;

    int16_t *buffer;
} SamplerConf;

typedef struct {
    enum {
        BOXCAR_WINDOW,
        BARTLETT_WINDOW,
        HANN_WINDOW,
        HAMMING_WINDOW,
        BLACKAMN_WINDOW
    } window_type;
    uint8_t repeat;
    uint16_t length;

    float *window;
} WindowConf;

typedef struct {
    int min: 1;
    int max: 1;
    int peak_to_peak: 1;
    int rms: 1;
    int mean: 1;
    int median: 1;
    int q1: 1;
    int q3: 1;
    int iqr: 1;
    int variance: 1;
    int standard_deviation: 1;
    int average_absolute_deviate: 1;
    int median_absolute_deviate: 1;
    int skewness: 1;
    int kurtosis: 1;
    int axis_correlation_xy: 1;
    int axis_correlation_yz: 1;
    int axis_correlation_xz: 1;
} StatisticsConf;


typedef struct {
    enum {
        DFT, DCTII, DCTIV, MDCT
    } transform;
    enum {
        LOG_NONE, LOG_POWER, LOG_DECIBEL
    } log;
} FFTTransformConf;


typedef struct {
    // Nastavenia zdroja konfigurácie + mqtt broker
    SamplerConf sampler;
    WindowConf time_smoothing;
    // Uložiť vzorky + decimácia
    StatisticsConf statistics;
    // PeakDetection
    WindowConf freq_windowing;
    FFTTransformConf freq_transform;
    WindowConf freq_smoothing;
    uint8_t welch_averages_count;
    // PeakDetection

    // SaveFormat
} Configuration;*/

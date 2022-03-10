#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/soc.h"
#include "esp_system.h"

#include "esp_dsp.h"
#include "mpack.h"

#include "inertial_unit.h"
#include "peripheral.h"
#include "pipeline.h"


#define WIFI_SSID     "Etakerim Crew"
#define WIFI_PASS     "4priv.hacker-turaw"
#define MQTT_BROKER   "mqtt://192.168.1.103"
#define MSG_MAX_LEN    25
#define BUFFER_LEN     6

typedef struct {
    char msg[MSG_MAX_LEN];
} MessageStream;

typedef struct {
    Vector a[BUFFER_LEN];
} SamplesStream;


static InertialUnit imu = {
    .spi = SPI2_HOST,
    .clk = 21,
    .miso = 23,
    .mosi = 19,
    .xgcs = 22,
    .mcs = 14,
    .int1 = 17,
    .int2 = 16,
    .en_data = 18
};
static OpenLog logger = {
    .vcc = 25,
    .uart = 0, 
    .rx = 5,
    .tx = 13,
    .baudrate = 9600,
    .buffer = 1024
};

static TaskHandle_t sample_tick;
static QueueHandle_t samples;
static QueueHandle_t mailbox;
static esp_mqtt_client_handle_t mqttclient;

static bool IRAM_ATTR isr_sample(void *args)
{
    BaseType_t higher_priority_woken = pdFALSE;
    vTaskNotifyGiveFromISR(sample_tick, &higher_priority_woken);
    return higher_priority_woken == pdTRUE;
}

void sampler_task()
{
    SamplesStream accel;
    uint16_t idx = 0;

    while (1) {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE) {
            imu_acceleration(&imu, &accel.a[idx++]);
            if (idx >= BUFFER_LEN - 1) {
                idx = 0;
                xQueueSend(samples, &accel, portMAX_DELAY);
            }
        }
    }
}

void mqtt_sender_task()
{
    MessageStream msg;
    while (1) {
        if (xQueueReceive(mailbox, &msg, portMAX_DELAY) == pdTRUE) {
            esp_mqtt_client_publish(mqttclient, "imu/1/stream", msg.msg, 0, 1, 0);
        }
    }
}

void app_main(void)
{
     wifi_config_t wifi_login = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK
        }
    };

    if (!imu_setup(&imu)) while (1);
    peripheral_setup();
    // nvs_load_config();
    wifi_connect(&wifi_login);
    mqttclient = mqtt_setup(MQTT_BROKER);
    // openlog_setup(&logger);
    // vTaskDelay(10000 / portTICK_RATE_MS);

    MessageStream msg;
    SamplesStream buffer;  // TODO: prenos ako ukazatele podľa FreeRTOS návodu

    samples = xQueueCreate(6, sizeof(SamplesStream));
    mailbox = xQueueCreate(12, sizeof(MessageStream));
    xTaskCreate(sampler_task, "sampler_task", 1024, NULL, 1, &sample_tick);
    xTaskCreate(mqtt_sender_task, "mqtt_sender_task", 4096, NULL, 1, NULL);
    clock_setup(1, isr_sample);

    while (1) {
        if (xQueueReceive(samples, &buffer, portMAX_DELAY) == pdTRUE) {
            // buffer data: buffer.a

            // Pipeline: 
            // Globálny konfiguračný objekt (štruktúra)
            // 0. Bežiaci priemer na odstránenie zerog
            // 1. Vyhladzovanie v časovej doméne
            // 2. Transformácia do frekvenčnej domény
            // 3. Priemerovanie spektrogramov (Welch)
            // 4. Vyhlazovanie vo frekvenciach
            // 5. Hľadanie špičiek
            // 6. Identifikácia udalostí
            // 7. Odoslanie udalostí

            //Doplnkové: Odoslanie vzoriek po decimácii, Odoslanie štatistík, Odoslanie spektra


            // Send via MQTT (experimental)
            for (int i = 0; i < BUFFER_LEN; i++) {
                snprintf(msg.msg, MSG_MAX_LEN, "%d %d %d", buffer.a[i].x, buffer.a[i].y, buffer.a[i].z);
                xQueueSend(mailbox, &msg, portMAX_DELAY);
            }

            // ESP_LOGI("main", msg.msg);
            //OpenLog: printf("%d %d %d\n", accel.x, accel.y, accel.z);
        }
    }
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

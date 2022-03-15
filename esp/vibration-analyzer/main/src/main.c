#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/message_buffer.h"
#include "soc/soc.h"
#include "esp_system.h"

#include "mpack.h"

#include "inertial_unit.h"
#include "peripheral.h"
#include "pipeline.h"


#define WIFI_SSID     ""
#define WIFI_PASS     ""
#define MQTT_BROKER   ""


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

// Nastavenia - pri zmene realokovať všetky polia a uvoľniť
static Configuration conf = {
    .sensor = {
        .frequency = 8,   // nastav podľa toho hw (min toľko)
        .range = IMU_2G,
        .n = 16,
        .overlap = 0.5
    },
    .tsmooth = {
        .enable = false,
        .n = 3,
        .repeat = 1
    },
    .stats = {
        .min = 1,
        .max = 1,
        .rms = 1,
        .mean = 1,
        .variance = 1,
        .std = 1,
        .skewness = 1,
        .kurtosis = 1,
        .median = 1,
        .mad = 1,
        .corr_xy = 0,
        .corr_yz = 0,
        .corr_xz = 0
    },
    .transform = {
        .window = HAMMING_WINDOW,
        .func = DFT,
        .log = true
    },
    .welch = {
        .enable = false,
        .history = 2
    },
    .fsmooth = {
        .enable = false,
        .n = 4,
        .repeat = 1
    },
    .peak = {
        .min_duration = 1,
        .time_proximity = 0,
        .strategy = THRESHOLD,
        .threshold = {  // warn, alert
            .t = -15
        },
        .neighbours = {
            .k = 8,
            .e = 0,
            .h_rel = 10,
            .h = -50
        },
        .zero_crossing = {
            .k = 4,
            .slope = 3
        },
        .hill_walker = {
            .tolerance = 0,
            .hole = 0,
            .prominence = 10,
            .isolation = 3
        }
    },
    .logger = {
        .enable = false,
        .decimation = 1,
        .samples = false,
        .stats = false,
        .events = false,
        .spectra = false
    },
    .mqtt = {
        .enable = true,
        .decimation = 1,
        .samples = true,
        .stats = true,
        .events = false,
        .spectra = false
    }
};

static TaskHandle_t sample_tick;
static BufferPipeline pipeline;
static esp_mqtt_client_handle_t mqttclient;
static MessageBufferHandle_t sender;


static bool IRAM_ATTR isr_sample(void *args)
{
    BaseType_t higher_priority_woken = pdFALSE;
    vTaskNotifyGiveFromISR(sample_tick, &higher_priority_woken);
    return higher_priority_woken == pdTRUE;
}


void sampler_task()
{
    uint16_t idx = 0;
    uint16_t leftover = conf.sensor.n * (1 - conf.sensor.overlap);

    float *axis[AXIS_COUNT];
    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        axis[i] = malloc(conf.sensor.n * sizeof(*axis[i]));
    }

    while (1) {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE) {
            imu_acceleration(&imu, &axis[0][idx], &axis[1][idx], &axis[2][idx]);

            if (++idx >= conf.sensor.n - 1) {
                xQueueSend(pipeline.queue[0], axis[0], portMAX_DELAY);
                buffer_shift_left(axis[0], conf.sensor.n, leftover);
                /* for (uint8_t i = 0; i < AXIS_COUNT; i++) {
                    xQueueSend(pipeline.queue[i], axis[i], portMAX_DELAY);
                    buffer_shift_left(axis[i], conf.sensor.n, leftover);
                }*/
                idx = conf.sensor.n - leftover;
            }
        }
    }

    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        free(axis[i]);
    }
}

size_t stats_serialize(char *msg, size_t size, const Statistics *stats, const StatisticsConfig *c)
{
    mpack_writer_t writer;
    mpack_writer_init(&writer, msg, size);

    mpack_build_map(&writer);
    if (c->min) {
        mpack_write_cstr(&writer, "min");
        mpack_write_float(&writer, stats->min);
    }
    if (c->max) {
        mpack_write_cstr(&writer, "max");
        mpack_write_float(&writer, stats->max);
    }
    if (c->rms) {
        mpack_write_cstr(&writer, "rms");
        mpack_write_float(&writer, stats->rms);
    }
    if (c->mean) {
        mpack_write_cstr(&writer, "avg");
        mpack_write_float(&writer, stats->mean);
    }
    if (c->std) {
        mpack_write_cstr(&writer, "std");
        mpack_write_float(&writer, stats->std);
    }
    if (c->skewness) {
        mpack_write_cstr(&writer, "skew");
        mpack_write_float(&writer, stats->skew);
    }
    if (c->kurtosis) {
        mpack_write_cstr(&writer, "kurt");
        mpack_write_float(&writer, stats->kurtosis);
    }
    if (c->median) {
        mpack_write_cstr(&writer, "med");
        mpack_write_float(&writer, stats->median);
    }
    if (c->mad) {
        mpack_write_cstr(&writer, "mad");
        mpack_write_float(&writer, stats->mad);
    }

    mpack_complete_map(&writer);
    mpack_writer_destroy(&writer); 

    return mpack_writer_buffer_used(&writer);
}

void pipeline_task(void *args)
{
    uint32_t x = (uint32_t)args;
    QueueHandle_t *source = &pipeline.queue[x];
    BufferPipelineKernel *k = &pipeline.kernel;
    BufferPipelineAxis *p = &pipeline.axis[x];
    Statistics stats;

    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    char msg[128];

    while (1) {
        if (xQueueReceive(*source, p->stream, portMAX_DELAY) == pdTRUE) {
            for (int i = 0; i < conf.sensor.n; i++)
                ESP_LOGI("main", " %.3f", p->stream[i]);
            ESP_LOGI("main", "\n");

            process_smoothing(p->stream, p->tmp_conv, conf.sensor.n, k->t_smooth, &conf.tsmooth);
            // TODO: Odoslať vzorky po decimácii
            for (int i = 0; i < conf.sensor.n; i++)
                ESP_LOGI("main", " %.3f", p->stream[i]);
            ESP_LOGI("main", "\n");

            process_statistics(p->stream, conf.sensor.n, &stats, &conf.stats);
            size_t len = stats_serialize(msg, 128, &stats, &conf.stats);
            if (conf.mqtt.stats) {
                xMessageBufferSend(sender, msg, len, portMAX_DELAY);
            }

            ESP_LOGI("main", "MIN: %.4f", stats.min);
            ESP_LOGI("main", "MAX: %.4f", stats.max);
            ESP_LOGI("main", "RMS: %.4f", stats.rms);
            ESP_LOGI("main", "MEAN: %.4f", stats.mean);
            ESP_LOGI("main", "VAR: %.4f", stats.var);
            ESP_LOGI("main", "STD: %.4f", stats.std);
            ESP_LOGI("main", "SKEW: %.4f", stats.skew);
            ESP_LOGI("main", "KURT: %.4f", stats.kurtosis);
            ESP_LOGI("main", "MEDIAN: %.4f", stats.median);
            ESP_LOGI("main", "MAD: %.4f", stats.mad);
            ESP_LOGI("main", "\n");

            int bins = process_spectrum(p->spectrum, p->stream, k->window, conf.sensor.n, &conf.transform);
            // TODO: Odoslať spektrum cez MQTT
            for (int i = 0; i < bins; i++)
                ESP_LOGI("main", "%.3f", p->spectrum[i]);
            ESP_LOGI("main", "\n");

            process_smoothing(p->spectrum, p->tmp_conv, bins, k->f_smooth, &conf.fsmooth);
            process_peak_finding(p->peaks, p->spectrum, bins, &conf.peak);
            for (int i = 0; i < bins; i++)
                ESP_LOGI("main", "%d", p->peaks[i]);
            ESP_LOGI("main", "\n");

            event_detection(p->events, p->peaks, p->spectrum, bins, conf.peak.min_duration, conf.peak.time_proximity);
            for (int i = 0; i < bins; i++)
                ESP_LOGI("main", "[%d t=%d d=%d prev=%d freq=%.3f tol=%.3f amp=%.3f]", 
                            p->events[i].action, p->events[i].start, p->events[i].duration, 
                            p->events[i].last_seen, p->events[i].frequency, 
                            p->events[i].tolerance, p->events[i].amplitude);
            ESP_LOGI("main", "\n\n---------");
            // TODO: Odoslať udalosti
        }
    }
    process_release(&pipeline);
}


void mqtt_sender_task()
{
    char msg[1024];
    size_t recv = 0;
    while (1) {
        recv = xMessageBufferReceive(sender, msg, 1024, portMAX_DELAY);
        if (recv > 0) {
            esp_mqtt_client_publish(mqttclient, "imu/1/stats", msg, recv, 1, 0);
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
    
    nvs_save_config(&conf);  // Factory reset for Debug
    nvs_load_config(&conf);

    wifi_connect(&wifi_login);
    mqttclient = mqtt_setup(MQTT_BROKER);
    // openlog_setup(&logger);
    // vTaskDelay(10000 / portTICK_RATE_MS);

    process_allocate(&pipeline, &conf);
    sender = xMessageBufferCreate(2048);

    xTaskCreate(sampler_task, "sampling", 1024, NULL, 1, &sample_tick);
    xTaskCreate(pipeline_task, "x_pipeline", 4096, (void *)0, 1, NULL);
    // xTaskCreate(pipeline_task, "y_pipeline", 4096, (void *)1, 1, NULL);
    // xTaskCreate(pipeline_task, "z_pipeline", 4096,(void *)2, 1, NULL);
    xTaskCreate(mqtt_sender_task, "mqtt_sender", 4096, NULL, 1, NULL);
    clock_setup(conf.sensor.frequency, isr_sample);
}

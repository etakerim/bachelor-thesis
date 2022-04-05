#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "driver/uart.h"

#include "inertial_unit.h"
#include "peripheral.h"
#include "pipeline.h"
#include "statistics.h"

// Measurements:
// #define FACTORY_RESET                1
// #define MESSAGES_MEASUREMENT         1
#define MEMORY_MEASUREMENT              1
// #define EXECUTION_TIME_ALGORITHMS   1
// #define EXECUTION_TIME_PIPELINE     1

#ifdef EXECUTION_TIME_ALGORITHMS
    #define MEASUREMENTS_CYCLES         100
#endif

#ifdef MEMORY_MEASUREMENT
void memory_measure(const char *reason)
{
    ESP_LOGW("main", "MEM, %s, %u, %u, %u", reason,
        heap_caps_get_total_size(MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)
    );
}
#endif
/*
Provisioning login = {
    .wifi_ssid="Montenegro",
    .wifi_pass="1234512345",
    .mqtt_url="mqtt://192.168.100.198"
};
*/

static Provisioning login = {
    .wifi_ssid="Etakerim Crew",
    .wifi_pass="4priv.hacker-turaw",
    .mqtt_url="mqtt://192.168.1.103"
};

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
    .baudrate = 115200,
    .buffer = MAX_BUFFER_SAMPLES * 2
};

static Configuration conf = {
    .sensor = {
        .frequency = 256,
        .range = IMU_2G,
        .n = 1024,
        .overlap = 0.5,
        .axis = {true, true, true}
    },
    .tsmooth = {
        .enable = false,
        .n = 8,
        .repeat = 1
    },
    .stats = {
        .min = true,
        .max = true,
        .rms = true,
        .mean = true,
        .variance = true,
        .std = true,
        .skewness = true,
        .kurtosis = true,
        .median = true,
        .mad = true,
        .correlation = true
    },
    .transform = {
        .window = HANN_WINDOW,
        .func = DFT,
        .log = true
    },
    .fsmooth = {
        .enable = false,
        .n = 8,
        .repeat = 1
    },
    .peak = {
        .min_duration = 4,
        .time_proximity = 5,
        .strategy = THRESHOLD,
        .threshold = {
            .t = -15
        },
        .neighbours = {
            .k = 9,
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
        .subsampling = 1,
        .local = false,
        .mqtt = true,
        .mqtt_samples = RAW_NONE_SEND,
        .mqtt_stats = false,
        .mqtt_events = false
    }
};

static TaskHandle_t sample_tick;
static esp_mqtt_client_handle_t mqttclient;
static BufferPipelineKernel pipeline;
static Sender sender;


void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    bool change, error;
    static char config[LARGEST_CONFIG];

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_REQUEST, 1);
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_CONFIG, 1);
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_LOGIN, 1);
            // V prevádzke vypni z bezpečnostných dôvodov
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_LOGIN_GET, 1);
            esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "imu started", 0, 1, 1);
            break;

        case MQTT_EVENT_DATA:
            if (strncmp(event->topic, MQTT_TOPIC_REQUEST, event->topic_len) == 0) {
                size_t n = config_serialize(config, LARGEST_CONFIG, &conf);
                esp_mqtt_client_publish(client, MQTT_TOPIC_RESPONSE, config, n, 1, 0);

            } else if (strncmp(event->topic, MQTT_TOPIC_LOGIN_GET, event->topic_len) == 0) {
                size_t n = login_serialize(config, LARGEST_CONFIG, &login);
                esp_mqtt_client_publish(client, MQTT_TOPIC_LOGIN_SEND, config, n, 1, 0);

            } else if (strncmp(event->topic, MQTT_TOPIC_CONFIG, event->topic_len) == 0) {
                esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "config received", 0, 1, 0);

                Configuration c;
                memcpy(&c, &conf, sizeof(c));
                change = config_parse(event->data, event->data_len, &c, &error);

                if (error) {
                    esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "config malformed", 0, 1, 0);

                } else if (change) {
                    nvs_save_config(&c);
                    esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "config applied", 0, 1, 0);
                    vTaskDelay(500 / portTICK_PERIOD_MS);
                    esp_wifi_stop();
	                vTaskDelay(200 / portTICK_PERIOD_MS);
                    esp_wifi_deinit();
                    vTaskDelay(200 / portTICK_PERIOD_MS);
                    if (conf.logger.local)
                        uart_driver_delete(logger.uart);

                    esp_restart();
                }

            } else if (strncmp(event->topic, MQTT_TOPIC_LOGIN, event->topic_len) == 0) {
                Provisioning cred;
                memcpy(&cred, &login, sizeof(cred));
                error = login_parse(event->data, event->data_len, &cred);

                if (error) {
                    esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "login malformed", 0, 1, 0);
                } else {
                    nvs_save_login(&cred);
                    esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "login saved", 0, 1, 0);
                }
            }
            break;
        default:
            break;
    }
}

static bool IRAM_ATTR isr_sample(void *args)
{
    BaseType_t higher_priority_woken = pdFALSE;
    vTaskNotifyGiveFromISR(sample_tick, &higher_priority_woken);
    return higher_priority_woken == pdTRUE;
}

void sampler_task()
{
    uint8_t i;
    float axis[AXIS_COUNT];

    uint16_t subsample = 0;
    const bool logger = (
        conf.logger.local || conf.logger.mqtt_samples != RAW_NONE_SEND
    );

    while (1) {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE) {

            imu_acceleration(&imu, &axis[0], &axis[1], &axis[2]);

            for (i = 0; i < AXIS_COUNT; i++) {
                if (conf.sensor.axis[i]) {
                    xQueueSend(pipeline.queue[i], &axis[i], 0);
                }
            }

            if (logger && ++subsample == conf.logger.subsampling) {
                subsample = 0;
                for (i = 0; i < AXIS_COUNT; i++) {
                    xQueueSend(sender.raw_stream, &axis[i], 0);
                }
            }
        }
    }
}

void logger_task()
{
    uint16_t idx = 0;
    float *stream = malloc(sender.max_send_samples * sizeof(*stream));
    static char message[LARGEST_MESSAGE];


#ifdef MEMORY_MEASUREMENT 
    memory_measure("logger_task");
#endif

    while (1) {
        if (xQueueReceive(sender.raw_stream, &stream[idx], portMAX_DELAY) == pdTRUE) {
            if (++idx < sender.max_send_samples)
                continue;

            idx = 0;
            if (conf.logger.mqtt_samples == RAW_TIME_SEND) {
                size_t len = stream_serialize(
                    message, LARGEST_MESSAGE, stream, sender.max_send_samples
                );
                if (len > 0)
                    esp_mqtt_client_publish(mqttclient, MQTT_TOPIC_STREAM, message, len, 0, 0);
            }

            if (conf.logger.local) {
                for (uint16_t i = 0; i < sender.max_send_samples; i += 3)
                    printf("%.3f %.3f %.3f\n", stream[i+0], stream[i+1], stream[i+2]);
            }
        }
    }

    free(stream);
}

void pipeline_task(void *args)
{
    uint32_t x = (uint32_t)args;
    BufferPipelineKernel *k = &pipeline;
    static char serialize[LARGEST_MESSAGE];

    BufferPipelineAxis p;
    Statistics stats;
    MqttAxisTopics mqtt_topics;

    size_t no = 0;
    uint16_t idx = 0;
    const uint16_t leftover = conf.sensor.n * (1 - conf.sensor.overlap);

    size_t changes;
    const int bins = conf.sensor.n / 2;
    const float bin_width = conf.sensor.frequency / (float)bins;

    axis_allocate(&p, &conf);
    axis_mqtt_topics(&mqtt_topics, x);

#ifdef MEMORY_MEASUREMENT 
    memory_measure("pipeline_task");
#endif

    while (1) {
        if (xQueueReceive(k->queue[x], &p.stream[idx], portMAX_DELAY) == pdTRUE) {

            if (++idx < conf.sensor.n)
                continue;

#ifdef EXECUTION_TIME_PIPELINE
            uint64_t start, end, tp;
            start = esp_timer_get_time();
#endif
            process_smoothing(p.stream, p.tmp_conv, conf.sensor.n, k->smooth, &conf.tsmooth);

            if (conf.logger.mqtt_stats) {
                process_statistics(p.stream, conf.sensor.n, &stats, &conf.stats);
                if (conf.stats.correlation) {
                    process_correlation(x, p.stream, &stats, &k->corr, &conf.sensor);
                }
                size_t len = stats_serialize(no, serialize, LARGEST_MESSAGE, &stats, &conf.stats);
                esp_mqtt_client_publish(mqttclient, mqtt_topics.stats, serialize, len, 0, 0);
            }

#ifdef EXECUTION_TIME_ALGORITHMS
            uint64_t t0 = esp_timer_get_time();
            for (int i = 0; i < MEASUREMENTS_CYCLES; i++) {
#endif
            process_spectrum(p.spectrum, p.stream, k->window, conf.sensor.n, &conf.transform);

#ifdef EXECUTION_TIME_ALGORITHMS
            }
            uint64_t t1 = esp_timer_get_time();
            ESP_LOGI("main", "S:, %llu, us", (t1 - t0) / MEASUREMENTS_CYCLES);
#endif

            if (conf.logger.mqtt_samples == RAW_FREQUENCY_SEND) {
                size_t len = spectra_serialize(no, serialize, LARGEST_MESSAGE, p.spectrum, bins, conf.sensor.frequency);
                esp_mqtt_client_publish(mqttclient, mqtt_topics.spectra, serialize, len, 0, 0);
            }

            process_smoothing(p.spectrum, p.tmp_conv, bins, k->smooth, &conf.fsmooth);

#ifdef EXECUTION_TIME_ALGORITHMS
            t0 = esp_timer_get_time();
            for (int i = 0; i < MEASUREMENTS_CYCLES; i++) {
#endif
            process_peak_finding(p.peaks, p.spectrum, bins, &conf.peak);
#ifdef EXECUTION_TIME_ALGORITHMS
            }
            t1 = esp_timer_get_time();
            ESP_LOGI("main", "P:, %llu, us", (t1 - t0) / MEASUREMENTS_CYCLES);
#endif

#ifdef EXECUTION_TIME_ALGORITHMS
            t0 = esp_timer_get_time();
            for (int i = 0; i < MEASUREMENTS_CYCLES; i++) {
#endif
            changes = event_detection(
                no, p.events, p.peaks, p.spectrum, bins,
                conf.peak.min_duration, conf.peak.time_proximity
            );
#ifdef EXECUTION_TIME_ALGORITHMS
            }
            t1 = esp_timer_get_time();
            ESP_LOGI("main", "E:, %llu, us", (t1 - t0) / MEASUREMENTS_CYCLES);
#endif

            if (conf.logger.mqtt_events && changes > 0) {
                size_t len = events_serialize(no, bin_width, serialize, LARGEST_MESSAGE, p.events, bins);
                esp_mqtt_client_publish(mqttclient, mqtt_topics.events, serialize, len, 0, 0);
            }

            buffer_shift_left(p.stream, conf.sensor.n, leftover);
            idx = conf.sensor.n - leftover;
            no++;

#ifdef EXECUTION_TIME_PIPELINE
            end = esp_timer_get_time();
            tp = end - start;
            ESP_LOGI("main", "%d, %llu, us", x, tp);
#endif
        }
    }
    axis_release(&p);
}


#ifdef FACTORY_RESET
void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());     // NVS partition was truncated and needs to be erased 
        ESP_ERROR_CHECK(nvs_flash_init());      //  Retry nvs_flash_init
    }

    nvs_save_config(&conf);
    nvs_save_login(&login);
    ESP_LOGW("main", "Config written");
}
#else
void app_main(void)
{
#ifdef MEMORY_MEASUREMENT
    ESP_LOGW("main", "MEM, State, Total, Free, Largest");
    memory_measure("app_main");
#endif
    // Initialize Non-Volatile Storage 
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());     // NVS partition was truncated and needs to be erased 
        ESP_ERROR_CHECK(nvs_flash_init());      //  Retry nvs_flash_init
    }

    // Load configuration from nvs or apply defaults
    nvs_load(&conf, &login);

    // Initialize accelerometer
    ESP_ERROR_CHECK(imu_setup(&imu));
    imu_output_data_rate(&imu, conf.sensor.frequency);
    imu_acceleration_range(&imu, conf.sensor.range);

#ifdef MESSAGES_MEASUREMENT
    ESP_LOGW("main", "N: %d", conf.sensor.n);
    ESP_LOGW("main", "Size of config(B): %d", sizeof(Configuration));
    ESP_LOGW("main", "Size of statistics(B): %d", sizeof(Statistics));
    ESP_LOGW("main", "Size of event(B): %d", sizeof(SpectrumEvent));
    ESP_LOGW("main", "Size of sample(B): %d", sizeof(float));
    ESP_LOGW("main", "Size of max. message samples(B): %d",
            (conf.sensor.n / AXIS_COUNT + 1) * AXIS_COUNT* sizeof(float));
#endif

#ifdef MEMORY_MEASUREMENT 
    memory_measure("imu");
#endif 
    // Allocate common buffers (4x becaus of DCT table lookup)
    dsps_fft2r_init_fc32(NULL, 4 * conf.sensor.n); 
    process_allocate(&pipeline, &conf);

#ifdef MEMORY_MEASUREMENT 
    memory_measure("fft&buffers");
#endif 


    wifi_config_t wifi_login = {};

    if (conf.logger.local || conf.logger.mqtt) {
        sender_allocate(&sender, conf.sensor.n);
    }

    if (conf.logger.mqtt) {
        strcpy((char *)wifi_login.sta.ssid, (char *)login.wifi_ssid);
        strcpy((char *)wifi_login.sta.password, (char *)login.wifi_pass);
        wifi_login.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        ESP_ERROR_CHECK(esp_netif_init());                  // Initialize TCP/IP
        ESP_ERROR_CHECK(esp_event_loop_create_default());   // Initialize event loop

        wifi_connect(&wifi_login);
        mqttclient = mqtt_setup(login.mqtt_url);
    }

    if (conf.logger.local) {
        openlog_setup(&logger);
        vTaskDelay(10000 / portTICK_RATE_MS);
    }

    // Start up tasks ----------------------------------
#ifdef MEMORY_MEASUREMENT 
    memory_measure("wifi");
#endif
    if (conf.sensor.axis[0])
        xTaskCreate(pipeline_task, "x_pipeline", 10000, (void *)0, 1, NULL);
#ifdef MEMORY_MEASUREMENT 
    vTaskDelay(1000 / portTICK_RATE_MS);
#endif
    if (conf.sensor.axis[1])
        xTaskCreate(pipeline_task, "y_pipeline", 10000, (void *)1, 1, NULL);
#ifdef MEMORY_MEASUREMENT
    vTaskDelay(1000 / portTICK_RATE_MS);
#endif
    if (conf.sensor.axis[2])
        xTaskCreate(pipeline_task, "z_pipeline", 10000, (void *)2, 1, NULL);
#ifdef MEMORY_MEASUREMENT 
    vTaskDelay(1000 / portTICK_RATE_MS);
#endif

    xTaskCreate(sampler_task, "sampling", 1024, NULL, 1, &sample_tick);

    if (conf.logger.local|| conf.logger.mqtt_samples != RAW_NONE_SEND)
        xTaskCreate(logger_task, "logger", 4096, NULL, 1, NULL);

    // Start sampling timer
    clock_setup(conf.sensor.frequency, isr_sample);
    
#ifdef MEMORY_MEASUREMENT 
    vTaskDelay(1000 / portTICK_RATE_MS);
    memory_measure("all");
#endif
}
#endif

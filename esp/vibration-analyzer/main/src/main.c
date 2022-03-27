#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "inertial_unit.h"
#include "peripheral.h"
#include "pipeline.h"
#include "driver/uart.h"

/*
Provisioning login = {
    .wifi_ssid="Montenegro",
    .wifi_pass="1234512345",
    .mqtt_url="mqtt://192.168.100.198"
};
*/

Provisioning login = {
    .wifi_ssid="Etakerim Crew",
    .wifi_pass="4priv.hacker-turaw",
    .mqtt_url="mqtt://192.168.1.103"
};


InertialUnit imu = {
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

OpenLog logger = {
    .vcc = 25,
    .uart = 0, 
    .rx = 5,
    .tx = 13,
    .baudrate = 9600,
    .buffer = 1024
};

Configuration conf = {
    .sensor = {
        .frequency = 952,
        .range = IMU_2G,
        .n = 1024,
        .overlap = 0.5,
        .axis = {true, true, true}
    },
    .tsmooth = {
        .enable = false,
        .n = 3,
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
        .window = HAMMING_WINDOW,
        .func = DFT,
        .log = true
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
        .threshold = {
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
        .subsampling = 1,
        .openlog_raw_samples = false,
        .mqtt_samples = false,
        .mqtt_stats = true,
        .mqtt_spectra = false,
        .mqtt_events = true
    }
};


TaskHandle_t sample_tick;
esp_mqtt_client_handle_t mqttclient;
BufferPipelineKernel pipeline;
Sender sender;

char logger_content[LARGEST_MESSAGE];
char mqtt_handler_content[LARGEST_MESSAGE];
char axis_serialize[AXIS_COUNT][LARGEST_MESSAGE];


void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    bool change, error;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_REQUEST, 1);
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_CONFIG, 1);
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_LOGIN, 1);
            esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "imu started", 0, 1, 1);
            break;

        case MQTT_EVENT_DATA:
            if (strncmp(event->topic, MQTT_TOPIC_REQUEST, event->topic_len) == 0) {
                size_t n = config_serialize(mqtt_handler_content, LARGEST_MESSAGE, &conf);
                esp_mqtt_client_publish(client, MQTT_TOPIC_RESPONSE, mqtt_handler_content, n, 1, 0);

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

                    if (conf.logger.mqtt_samples || conf.logger.mqtt_stats ||
                            conf.logger.mqtt_spectra || conf.logger.mqtt_events)
                        esp_wifi_deinit();

                    if (conf.logger.openlog_raw_samples)
                        uart_driver_delete(logger.uart);

                    vTaskDelay(500 / portTICK_RATE_MS);
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
        conf.logger.openlog_raw_samples || conf.logger.mqtt_samples
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

    while (1) {
        if (xQueueReceive(sender.raw_stream, &stream[idx], portMAX_DELAY) == pdTRUE) {
            if (++idx < sender.max_send_samples)
                continue;

            idx = 0;
            if (conf.logger.mqtt_samples) {
                size_t len = stream_serialize(
                    logger_content, LARGEST_MESSAGE, stream, sender.max_send_samples
                );
                if (len > 0)
                    esp_mqtt_client_publish(mqttclient, MQTT_TOPIC_STREAM, logger_content, len, 0, 0);
            }

            if (conf.logger.openlog_raw_samples) {
                for (uint16_t i = 0; i < sender.max_send_samples; i += 3)
                    printf("%g %g %g", stream[i+0], stream[i+1], stream[i+2]);
            }
        }
    }

    free(stream);
}

void pipeline_task(void *args)
{
    uint32_t x = (uint32_t)args;
    BufferPipelineKernel *k = &pipeline;
    char *serialize = axis_serialize[x];

    BufferPipelineAxis p;
    Statistics stats;
    MqttAxisTopics mqtt_topics;

    size_t no = 0;
    uint16_t idx = 0;
    const uint16_t leftover = conf.sensor.n * (1 - conf.sensor.overlap);
    uint64_t start, end;

    axis_allocate(&p, &conf);
    axis_mqtt_topics(&mqtt_topics, x);

    while (1) {
        if (xQueueReceive(k->queue[x], &p.stream[idx], portMAX_DELAY) == pdTRUE) {

            if (++idx < conf.sensor.n)
                continue;

            start = esp_timer_get_time();

            process_smoothing(p.stream, p.tmp_conv, conf.sensor.n, k->smooth, &conf.tsmooth);

            if (conf.logger.mqtt_stats) {
                process_statistics(p.stream, conf.sensor.n, &stats, &conf.stats);
                if (conf.stats.correlation) {
                    process_correlation(x, p.stream, &stats, &k->corr, &conf.sensor);
                }
                size_t len = stats_serialize(no, serialize, LARGEST_MESSAGE, &stats, &conf.stats);
                esp_mqtt_client_publish(mqttclient, mqtt_topics.stats, serialize, len, 0, 0);
            }

            int bins = process_spectrum(p.spectrum, p.stream, k->window, conf.sensor.n, &conf.transform);
            float bin_width = conf.sensor.frequency / (float)bins;
            
            if (conf.logger.mqtt_spectra) {
                size_t len = spectra_serialize(no, serialize, LARGEST_MESSAGE, p.spectrum, bins, conf.sensor.frequency);
                esp_mqtt_client_publish(mqttclient, mqtt_topics.spectra, serialize, len, 0, 0);
            }

            process_smoothing(p.spectrum, p.tmp_conv, bins, k->smooth, &conf.fsmooth);
            process_peak_finding(p.peaks, p.spectrum, bins, &conf.peak);
            size_t changes = event_detection(
                no, p.events, p.peaks, p.spectrum, bins,
                conf.peak.min_duration, conf.peak.time_proximity
            );

            if (conf.logger.mqtt_events && changes > 0) {
                size_t len = events_serialize(no, bin_width, serialize, LARGEST_MESSAGE, p.events, bins);
                esp_mqtt_client_publish(mqttclient, mqtt_topics.events, serialize, len, 0, 0);
            }

            buffer_shift_left(p.stream, conf.sensor.n, leftover);
            idx = conf.sensor.n - leftover;
            no++;

            end = esp_timer_get_time();
            ESP_LOGI("main", "[%d] %llu us", x, end - start);  // / MEASUREMENTS
        }
    }
    axis_release(&p);
}

void app_main(void)
{
    //! ifdef nvs_save_config(&conf); nvs_save_login(&login); Factory reset for Debug
    
    ESP_ERROR_CHECK(imu_setup(&imu));
    imu_output_data_rate(&imu, conf.sensor.frequency);
    imu_acceleration_range(&imu, conf.sensor.range);

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());     // NVS partition was truncated and needs to be erased 
        ESP_ERROR_CHECK(nvs_flash_init());      //  Retry nvs_flash_init
    }

    // Load configuration from nvs or apply defaults
    // nvs_load(&conf, &login);

    const bool mqtt_running = (
        conf.logger.mqtt_samples || conf.logger.mqtt_stats ||
        conf.logger.mqtt_spectra || conf.logger.mqtt_events
    );

    wifi_config_t wifi_login = {};
    strcpy((char *)wifi_login.sta.ssid, (char *)login.wifi_ssid);
    strcpy((char *)wifi_login.sta.password, (char *)login.wifi_pass);
    wifi_login.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    if (mqtt_running) {
        ESP_ERROR_CHECK(esp_netif_init());                  // Initialize TCP/IP
        ESP_ERROR_CHECK(esp_event_loop_create_default());   // Initialize event loop

        wifi_connect(&wifi_login);
        mqttclient = mqtt_setup(login.mqtt_url);

        sender_allocate(&sender, conf.sensor.n);
    }

    if (conf.logger.openlog_raw_samples) {
        openlog_setup(&logger);
        vTaskDelay(10000 / portTICK_RATE_MS);
    }

    // Allocate common buffers
    dsps_fft2r_init_fc32(NULL, conf.sensor.n);
    process_allocate(&pipeline, &conf);

    ESP_LOGI("main", "[x] Total:%u Free:%u Largest:%u",
        heap_caps_get_total_size(MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)
    );

    // Start up tasks
    xTaskCreate(sampler_task, "sampling", 1024, NULL, 1, &sample_tick);
    if (conf.sensor.axis[0])
        xTaskCreate(pipeline_task, "x_pipeline", 5500, (void *)0, 1, NULL);
    if (conf.sensor.axis[1])
        xTaskCreate(pipeline_task, "y_pipeline", 5500, (void *)1, 1, NULL);
    if (conf.sensor.axis[2])
        xTaskCreate(pipeline_task, "z_pipeline", 5500, (void *)2, 1, NULL);
    if (conf.logger.openlog_raw_samples || conf.logger.mqtt_samples)
        xTaskCreate(logger_task, "logger", 4096, NULL, 1, NULL);
   
    vTaskDelay(1000 / portTICK_RATE_MS);
    ESP_LOGI("main", "[log] Total:%u Free:%u Largest:%u",
        heap_caps_get_total_size(MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)
    );
    vTaskDelay(2000 / portTICK_RATE_MS);

    // Start sampling timer
    clock_setup(conf.sensor.frequency, isr_sample);
}

/*
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/performance/speed.html
ESP_LOGI("main", "[app main] Total:%u Free:%u Largest:%u",
    heap_caps_get_total_size(MALLOC_CAP_8BIT),
    heap_caps_get_free_size(MALLOC_CAP_8BIT),
    heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)
);
*/

/* // DEBUG
for (int i = 0; i < conf.sensor.n; i++)
    ESP_LOGI("main", " %.3f", p.stream[i]);
ESP_LOGI("main", "\n");

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

for (int i = 0; i < bins; i++)
    ESP_LOGI("main", "%.3f", p.spectrum[i]);
ESP_LOGI("main", "\n");

for (int i = 0; i < bins; i++)
    ESP_LOGI("main", "%d", p.peaks[i]);
ESP_LOGI("main", "\n");

for (int i = 0; i < bins; i++)
    ESP_LOGI("main", "[%d t=%d d=%d prev=%d freq=%.3f tol=%.3f amp=%.3f]", 
                p.events[i].action, p.events[i].start, p.events[i].duration, 
                p.events[i].last_seen, p.events[i].frequency, 
                p.events[i].tolerance, p.events[i].amplitude);
ESP_LOGI("main", "\n\n---------");
*/

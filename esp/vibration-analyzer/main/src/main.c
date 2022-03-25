#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc.h"
#include "esp_system.h"

#include "inertial_unit.h"
#include "peripheral.h"
#include "pipeline.h"
#include "driver/uart.h"


#define WIFI_SSID     "Etakerim Crew"
#define WIFI_PASS     "4priv.hacker-turaw"
#define MQTT_BROKER   "mqtt://192.168.1.103"

/*
#define WIFI_SSID     "Montenegro"
#define WIFI_PASS     "1234512345"
#define MQTT_BROKER   "mqtt://192.168.100.198"

#define WIFI_SSID     "Ghubeneho"
#define WIFI_PASS     "Ghubeneho"
#define MQTT_BROKER   "mqtt://172.16.3.55"
*/

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
        .mad = 1
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
        .mqtt_events = true,
    }
};

void process_allocate(BufferPipelineKernel *p, Configuration *conf)
{
    const uint16_t n = conf->sensor.n;
    const uint16_t t_smooth = conf->tsmooth.n;
    const uint16_t f_smooth = conf->fsmooth.n;
    const uint16_t n_smooth = max(t_smooth, f_smooth);
    const WindowTypeConfig w = conf->transform.window;

    p->smooth = malloc(n_smooth * sizeof(*p->smooth));
    mean_kernel(p->smooth, n_smooth);
    
    p->window = malloc(n * sizeof(*p->window));
    switch (w) {
        case BOXCAR_WINDOW:
            boxcar_window(p->window, n); break;
        case BARTLETT_WINDOW:
            bartlett_window(p->window, n); break;
        case HANN_WINDOW:
            hann_window(p->window, n); break;
        case HAMMING_WINDOW:
            hamming_window(p->window, n); break;
        case BLACKMAN_WINDOW:
            blackman_window(p->window, n); break;
        default:
            break;
    }

    for (uint8_t i = 0; i < AXIS_COUNT; i++)
        p->queue[i] = xQueueCreate(
            conf->sensor.frequency / conf->sensor.n + 2, 
            sizeof(float) * conf->sensor.n
        );
}


void axis_allocate(BufferPipelineAxis *p, Configuration *conf)
{
    const uint16_t n = conf->sensor.n;
    const uint16_t bins = n / 2;
    const uint16_t t_smooth = conf->tsmooth.n;
    const uint16_t f_smooth = conf->fsmooth.n;
    const uint16_t n_smooth = max(t_smooth, f_smooth);

    p->stream = malloc(n * sizeof(*p->stream));
    p->tmp_conv = malloc((n + n_smooth - 1) * sizeof(*p->tmp_conv));
    p->spectrum = malloc(2 * n * sizeof(*p->spectrum));
    p->peaks = malloc(bins * sizeof(*p->peaks));
    p->events = malloc(bins * sizeof(*p->events));
    memset(p->events, 0, bins * sizeof(*p->events));
}

void sender_allocate(Sender *sender, uint16_t length)
{
    // Formát: topic, message, topic, message, ...
    sender->max_send_samples = length;
    sender->raw_stream = xQueueCreate(
        SAMPLES_QUEUE_SLOTS, sizeof(float) * sender->max_send_samples
    );
}

void sender_release(Sender *sender)
{
    vQueueDelete(sender->raw_stream);
}

void process_release(BufferPipelineKernel *p)
{
    free(p->smooth);
    free(p->window);
    for (uint8_t i = 0; i < AXIS_COUNT; i++)
        vQueueDelete(p->queue[i]);
}

void axis_release(BufferPipelineAxis *p)
{
    free(p->stream);
    free(p->tmp_conv);
    free(p->spectrum);
    free(p->peaks);
    free(p->events);
}


TaskHandle_t sample_tick;
BufferPipelineKernel pipeline;
esp_mqtt_client_handle_t mqttclient;
Sender sender;

char logger_content[LARGEST_MESSAGE];
char mqtt_handler_content[LARGEST_MESSAGE];
char axis_serialize[AXIS_COUNT][LARGEST_MESSAGE];


void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_REQUEST, 1);
            esp_mqtt_client_subscribe(client, MQTT_TOPIC_LOAD, 1);
            esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "imu connected", 0, 1, 1);
            break;

        case MQTT_EVENT_DATA:
            if (strncmp(event->topic, MQTT_TOPIC_REQUEST, event->topic_len) == 0) {
                size_t n = config_serialize(mqtt_handler_content, LARGEST_MESSAGE, &conf);
                esp_mqtt_client_publish(client, MQTT_TOPIC_RESPONSE, mqtt_handler_content, n, 1, 0);

            } else if (strncmp(event->topic, MQTT_TOPIC_LOAD, event->topic_len) == 0) {
                esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "config received", 0, 1, 0);

                Configuration c;
                memcpy(&c, &conf, sizeof(c));
                config_parse(event->data, event->data_len, &c);
                /*
                if error:
                    esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "config malformed", 0, 1, 0);
                if change:
                    esp_mqtt_client_publish(client, MQTT_TOPIC_SYSLOG, "config applied", 0, 1, 0);
                    nvs_save_config(&c);
                    if (conf.logger.mqtt_samples || conf.logger.mqtt_stats ||
                            conf.logger.mqtt_spectra || conf.logger.mqtt_events)
                        esp_wifi_deinit();
                    if (conf.logger.openlog_raw_samples)
                        uart_driver_delete();
                    esp_restart();
                */
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

void logger_task()
{
    float *stream = malloc(sender.max_send_samples * sizeof(*stream));

    while (1) {
        if (xQueueReceive(sender.raw_stream, stream, portMAX_DELAY) == pdTRUE) {
            size_t len = stream_serialize(mqtt_handler_content, LARGEST_MESSAGE, stream, sender.max_send_samples);
            esp_mqtt_client_publish(mqttclient, MQTT_TOPIC_STREAM, mqtt_handler_content, len, 0, 0);

            /*if (conf.logger.openlog_raw_samples) {
                printf("%g %g %g", axis[0][idx], axis[1][idx], axis[2][idx]);
            }*/
        }
    }

    free(stream);
}

void sampler_task()
{
    uint8_t i;
    float *samples;
    float *axis[AXIS_COUNT];

    uint16_t idx, subsample, i_logger;
    idx = subsample = i_logger = 0;
    const uint16_t leftover = conf.sensor.n * (1 - conf.sensor.overlap);

    samples = malloc(sender.max_send_samples * sizeof(*samples));
    for (i = 0; i < AXIS_COUNT; i++)
        axis[i] = malloc(conf.sensor.n * sizeof(*axis[i]));


    while (1) {
        // Wait for notfication on sampling interval
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE) {
            // Read acceleration on all axis via SPI bus
            imu_acceleration(&imu, &axis[0][idx], &axis[1][idx], &axis[2][idx]);

            // Send complete buffer via queues to pipelines for each axis
            if (++idx >= conf.sensor.n - 1) {
                for (i = 0; i < AXIS_COUNT; i++) {
                    xQueueSend(pipeline.queue[i], axis[i], 0);
                    buffer_shift_left(axis[i], conf.sensor.n, leftover);
                }
                idx = conf.sensor.n - leftover;
            }

            // Send non overlapping buffers according to set decimation
            if (conf.logger.mqtt_samples) { 
                if (++subsample == conf.logger.subsampling) {
                    subsample = 0;
                    if (i_logger - AXIS_COUNT >= sender.max_send_samples) {
                        i_logger = 0;
                        xQueueSend(sender.raw_stream, samples, 0);
                    }
                    for (i = 0; i < AXIS_COUNT; i++) {
                        samples[i_logger++] = axis[i][idx];
                    }
                }
            }
        }
    }

    free(samples);
    for (i = 0; i < AXIS_COUNT; i++)
        free(axis[i]);

}

void pipeline_task(void *args)
{
    size_t no = 0;
    uint32_t x = (uint32_t)args;
    BufferPipelineKernel *k = &pipeline;
    char *serialize = axis_serialize[x];

    BufferPipelineAxis p;
    Statistics stats;
    MqttAxisTopics mqtt_topics;

    axis_allocate(&p, &conf);
    axis_mqtt_topics(&mqtt_topics, x);

    while (1) {
        if (xQueueReceive(k->queue[x], p.stream, portMAX_DELAY) == pdTRUE) {

            process_smoothing(p.stream, p.tmp_conv, conf.sensor.n, k->smooth, &conf.tsmooth);
            process_statistics(p.stream, conf.sensor.n, &stats, &conf.stats);

            if (conf.logger.mqtt_stats) {
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
            no++;
        }
    }
    axis_release(&p);
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
    const bool mqtt_running = (
        conf.logger.mqtt_samples || conf.logger.mqtt_stats ||
        conf.logger.mqtt_spectra || conf.logger.mqtt_events);

    imu_setup(&imu);
    imu_output_data_rate(&imu, conf.sensor.frequency);
    imu_acceleration_range(&imu, conf.sensor.range);

    peripheral_setup();
    // nvs_save_config(&conf);     //! Factory reset for Debug
    // nvs_load_config(&conf);

    if (mqtt_running) {
        wifi_connect(&wifi_login);
        mqttclient = mqtt_setup(MQTT_BROKER);
        sender_allocate(&sender, conf.sensor.n);
    }
    if (conf.logger.openlog_raw_samples) {
        openlog_setup(&logger);
        vTaskDelay(10000 / portTICK_RATE_MS);
    }

    dsps_fft2r_init_fc32(NULL, conf.sensor.n);
    process_allocate(&pipeline, &conf);

    xTaskCreate(sampler_task, "sampling", 1024, NULL, 1, &sample_tick);
    xTaskCreate(pipeline_task, "x_pipeline", 6000, (void *)0, 1, NULL);
    xTaskCreate(pipeline_task, "y_pipeline", 6000, (void *)1, 1, NULL);
    xTaskCreate(pipeline_task, "z_pipeline", 6000, (void *)2, 1, NULL);
    if (mqtt_running) {
        xTaskCreate(logger_task, "logger", 4096, NULL, 1, NULL);
    }
    clock_setup(conf.sensor.frequency, isr_sample);
}

/*
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

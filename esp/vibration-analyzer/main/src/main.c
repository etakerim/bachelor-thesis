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


#define WIFI_SSID     "Etakerim Crew"
#define WIFI_PASS     "4priv.hacker-turaw"
#define MQTT_BROKER   "mqtt://192.168.1.103"


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
        .mad = 1
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
        .local = false,
        .mqtt = true,
        .samples = true,
        .stats = true,
        .spectra = false,
        .events = false,
        .subsampling = 1
    }
};

static TaskHandle_t sample_tick;
static BufferPipeline pipeline;
static esp_mqtt_client_handle_t mqttclient;
static Sender sender;

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    char content[SERIALIZE_BUFFER_LENGTH];

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            esp_mqtt_client_subscribe(client, "imu/1/config/request", 1);
            esp_mqtt_client_subscribe(client, "imu/1/config/load", 1);
            esp_mqtt_client_publish(client, "imu/1/syslog", "imu connected", 0, 1, 1);
            break;

        case MQTT_EVENT_DATA:
            if (strncmp(event->topic, "imu/1/config/request", event->topic_len) == 0) {
                size_t n = config_serialize(content, SERIALIZE_BUFFER_LENGTH, &conf);
                esp_mqtt_client_publish(client, "imu/1/config/response", content, n, 1, 0);

            } else if (strncmp(event->topic, "imu/1/config/load", event->topic_len) == 0) {
                Configuration c;
                memset(&c, 0, sizeof(c));
                // memcpy(&c, &conf, sizeof(c));
                // TODO: config load and save to nvs
                config_parse(event->data, event->data_len, &c); // Apply partial changes
                // memcpy(&conf, &c, sizeof(c));
                // nvs_save_config(&c);
                esp_mqtt_client_publish(client, "imu/1/config/syslog", "config received", 0, 1, 0);
            }
            break;
        default:
            break;
    }
}


void mqtt_sender_task()
{
    char topic[TOPIC_LENGTH] = DEVICE_MQTT_TOPIC;
    char content[SERIALIZE_BUFFER_LENGTH];

    while (1) {
        size_t n = message_recv(&sender, topic, content, SERIALIZE_BUFFER_LENGTH);
        if (n > 0) {
            esp_mqtt_client_publish(mqttclient, topic, content, n, 1, 0);
            strncpy(topic, DEVICE_MQTT_TOPIC, TOPIC_LENGTH);
        }
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
    uint16_t idx = 0;
    uint16_t leftover = conf.sensor.n * (1 - conf.sensor.overlap);

    float *axis[AXIS_COUNT];
    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        axis[i] = malloc(conf.sensor.n * sizeof(*axis[i]));
    }
    char content[SERIALIZE_BUFFER_LENGTH];
    
    mpack_writer_t writer;
    uint16_t subsample = 0;
    uint16_t n = 0;

    if (conf.logger.samples) {
        stream_serialize_init(&writer, content, SERIALIZE_BUFFER_LENGTH);
    }

    while (1) {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE) {
            imu_acceleration(&imu, &axis[0][idx], &axis[1][idx], &axis[2][idx]);

            // LOGGING
            if (conf.logger.samples && ++subsample == conf.logger.subsampling) {
                stream_serialize(&writer, axis[0][idx], axis[1][idx], axis[2][idx]);
                subsample = 0;

                if (++n == conf.sensor.n - 1) {
                    size_t len = stream_serialize_close(&writer);
                    message_send(&sender, "stream/samples", content, len);
                    n = 0;
                    stream_serialize_init(&writer, content, SERIALIZE_BUFFER_LENGTH);
                }
            }
        
            if (++idx >= conf.sensor.n - 1) {
                xQueueSend(pipeline.queue[0], axis[0], portMAX_DELAY);
                buffer_shift_left(axis[0], conf.sensor.n, leftover);
                /*
                for (uint8_t i = 0; i < AXIS_COUNT; i++) {
                    xQueueSend(pipeline.queue[i], axis[i], portMAX_DELAY);
                    buffer_shift_left(axis[i], conf.sensor.n, leftover);
                }
                */
                idx = conf.sensor.n - leftover;
            }
        }
    }

    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        free(axis[i]);
    }
}

void pipeline_task(void *args)
{
    uint32_t x = (uint32_t)args;
    QueueHandle_t *source = &pipeline.queue[x];
    BufferPipelineKernel *k = &pipeline.kernel;
    BufferPipelineAxis *p = &pipeline.axis[x];

    Statistics stats;
    MqttAxisTopics mqtt_topics;
    size_t t = 0;
    axis_mqtt_topics(&mqtt_topics, x);
    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);

    while (1) {
        if (xQueueReceive(*source, p->stream, portMAX_DELAY) == pdTRUE) {

            process_smoothing(p->stream, p->tmp_conv, conf.sensor.n, k->t_smooth, &conf.tsmooth);

            process_statistics(p->stream, conf.sensor.n, &stats, &conf.stats);
            if (conf.logger.stats) {
                size_t len = stats_serialize(p->serialize, SERIALIZE_BUFFER_LENGTH, &stats, &conf.stats);
                message_send(&sender, mqtt_topics.stats, p->serialize, len);
            }

            int bins = process_spectrum(p->spectrum, p->stream, k->window, conf.sensor.n, &conf.transform);
            if (conf.logger.spectra) {
                size_t len = spectra_serialize(p->serialize, SERIALIZE_BUFFER_LENGTH, p->spectrum, bins, conf.sensor.frequency);
                message_send(&sender, mqtt_topics.spectra, p->serialize, len);
            }

            // TODO: welch smoothing

            process_smoothing(p->spectrum, p->tmp_conv, bins, k->f_smooth, &conf.fsmooth);
            process_peak_finding(p->peaks, p->spectrum, bins, &conf.peak);

            size_t changes = event_detection(
                t++, p->events, p->peaks, p->spectrum, bins, 
                conf.peak.min_duration, conf.peak.time_proximity
            );
            if (conf.logger.events && changes > 0) {
                size_t len = events_serialize(p->serialize, SERIALIZE_BUFFER_LENGTH, p->events, bins);
                message_send(&sender, mqtt_topics.events, p->serialize, len);
            }

            // DEBUG
            /*
            for (int i = 0; i < conf.sensor.n; i++)
                ESP_LOGI("main", " %.3f", p->stream[i]);
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
                ESP_LOGI("main", "%.3f", p->spectrum[i]);
            ESP_LOGI("main", "\n");

            for (int i = 0; i < bins; i++)
                ESP_LOGI("main", "%d", p->peaks[i]);
            ESP_LOGI("main", "\n");

            for (int i = 0; i < bins; i++)
                ESP_LOGI("main", "[%d t=%d d=%d prev=%d freq=%.3f tol=%.3f amp=%.3f]", 
                            p->events[i].action, p->events[i].start, p->events[i].duration, 
                            p->events[i].last_seen, p->events[i].frequency, 
                            p->events[i].tolerance, p->events[i].amplitude);
            ESP_LOGI("main", "\n\n---------");
            */
        }
    }
    process_release(&pipeline);
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
    
    //nvs_save_config(&conf);  // Factory reset for Debug
    //nvs_load_config(&conf);

    ESP_LOGI("main", "CONFIG SIZE: %d\n\n", sizeof(conf)); //= 112 B

    wifi_connect(&wifi_login);
    mqttclient = mqtt_setup(MQTT_BROKER);
    if (conf.logger.local) {
        openlog_setup(&logger);
        vTaskDelay(10000 / portTICK_RATE_MS);
    }

    process_allocate(&pipeline, &conf);
    sender_setup(&sender);

    xTaskCreate(sampler_task, "sampling", 4096, NULL, 1, &sample_tick);
    xTaskCreate(pipeline_task, "x_pipeline", 4096, (void *)0, 1, NULL);
    // TODO: all axis at full sampling
    // xTaskCreate(pipeline_task, "y_pipeline", 4096, (void *)1, 1, NULL);
    // xTaskCreate(pipeline_task, "z_pipeline", 4096,(void *)2, 1, NULL);
    xTaskCreate(mqtt_sender_task, "mqtt_sender", 4096, NULL, 1, NULL);
    clock_setup(conf.sensor.frequency, isr_sample);
}

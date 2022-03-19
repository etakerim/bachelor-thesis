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
        .events = true,
        .spectra = true
    }
};

static TaskHandle_t sample_tick;
static BufferPipeline pipeline;
static esp_mqtt_client_handle_t mqttclient;
static Sender sender;

void sender_setup()
{
    sender.mutex = xSemaphoreCreateMutex();
    // topic, message, topic, message, ...
    sender.messages = xMessageBufferCreate(SERIALIZE_BUFFER_LENGTH);
}

void message_send(const char *topic, const char *content, size_t length)
{
    if (xSemaphoreTake(sender.mutex, portMAX_DELAY) == pdTRUE) {
        xMessageBufferSend(sender.messages, topic, strlen(topic) + 1, 0);
        xMessageBufferSend(sender.messages, content, length, 0);
        xSemaphoreGive(sender.mutex);
    }
}

void mqtt_sender_task()
{
    char topic[TOPIC_LENGTH] = DEVICE_MQTT_TOPIC;
    char content[SERIALIZE_BUFFER_LENGTH];

    while (1) {
        if (xSemaphoreTake(sender.mutex, portMAX_DELAY) == pdTRUE) {
            size_t topic_len = xMessageBufferReceive(
                sender.messages, &topic[DEVICE_MQTT_TOPIC_LENGTH - 1],
                TOPIC_LENGTH - DEVICE_MQTT_TOPIC_LENGTH, 0
            );
            size_t msg_length = xMessageBufferReceive(sender.messages, content, 2048, 0);
            xSemaphoreGive(sender.mutex);

            if (topic_len > 0 && msg_length > 0) {
                ESP_LOGI("main", "%d @[%s]@", DEVICE_MQTT_TOPIC_LENGTH, topic);
                esp_mqtt_client_publish(mqttclient, topic, content, msg_length, 1, 0);
                strncpy(topic, DEVICE_MQTT_TOPIC, TOPIC_LENGTH);
            }
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
    uint16_t n = 0;
    if (conf.mqtt.samples) {
        stream_serialize_init(&writer, content, SERIALIZE_BUFFER_LENGTH, conf.sensor.n * AXIS_COUNT);
    }

    while (1) {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE) {
            imu_acceleration(&imu, &axis[0][idx], &axis[1][idx], &axis[2][idx]);

            if (conf.mqtt.samples) {
                stream_serialize(&writer, axis[0][idx], axis[1][idx], axis[2][idx]);
                // subsampling, length + TODO: subsampling, test
                // {sub: 1, l: 512, stream: []}

                if (++n == conf.sensor.n - 1) {
                    stream_serialize_close(&writer);
                    message_send("stream", content, mpack_writer_buffer_used(&writer));
                    n = 0;
                    stream_serialize_init(&writer, content, SERIALIZE_BUFFER_LENGTH, conf.sensor.n * AXIS_COUNT);
                }
            }

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

void axis_mqtt_topics(MqttAxisTopics *topics, int axis)
{
    const static char *dirs[] = {"x", "y", "z"};

    strncpy(topics->stats, "stream/statistics/", SUBTOPIC_LENGTH);
    strcat(topics->stats, dirs[axis]);

    strncpy(topics->spectra, "stream/frequency/", SUBTOPIC_LENGTH);
    strcat(topics->spectra, dirs[axis]);

    strncpy(topics->events, "event/frequency/", SUBTOPIC_LENGTH);
    strcat(topics->events, dirs[axis]);
}

void pipeline_task(void *args)
{
    uint32_t x = (uint32_t)args;
    QueueHandle_t *source = &pipeline.queue[x];
    BufferPipelineKernel *k = &pipeline.kernel;
    BufferPipelineAxis *p = &pipeline.axis[x];
    Statistics stats;
    MqttAxisTopics mqtt_topics;

    axis_mqtt_topics(&mqtt_topics, x);
    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);

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
            if (conf.mqtt.stats) {
                size_t len = stats_serialize(p->serialize, SERIALIZE_BUFFER_LENGTH, &stats, &conf.stats);
                message_send(mqtt_topics.stats, p->serialize, len);
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
            if (conf.mqtt.spectra) {
                size_t len = spectra_serialize(p->serialize, SERIALIZE_BUFFER_LENGTH, p->spectrum, bins, conf.sensor.frequency);
                message_send(mqtt_topics.spectra, p->serialize, len);
            }
            for (int i = 0; i < bins; i++)
                ESP_LOGI("main", "%.3f", p->spectrum[i]);
            ESP_LOGI("main", "\n");

            process_smoothing(p->spectrum, p->tmp_conv, bins, k->f_smooth, &conf.fsmooth);
            process_peak_finding(p->peaks, p->spectrum, bins, &conf.peak);
            for (int i = 0; i < bins; i++)
                ESP_LOGI("main", "%d", p->peaks[i]);
            ESP_LOGI("main", "\n");

            size_t changes = event_detection(
                p->events, p->peaks, p->spectrum, bins, 
                conf.peak.min_duration, conf.peak.time_proximity
            );
            if (conf.mqtt.events && changes > 0) {
                size_t len = events_serialize(p->serialize, SERIALIZE_BUFFER_LENGTH, p->events, bins);
                message_send(mqtt_topics.events, p->serialize, len);
            }
            for (int i = 0; i < bins; i++)
                ESP_LOGI("main", "[%d t=%d d=%d prev=%d freq=%.3f tol=%.3f amp=%.3f]", 
                            p->events[i].action, p->events[i].start, p->events[i].duration, 
                            p->events[i].last_seen, p->events[i].frequency, 
                            p->events[i].tolerance, p->events[i].amplitude);
            ESP_LOGI("main", "\n\n---------");

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
    
    nvs_save_config(&conf);  // Factory reset for Debug
    nvs_load_config(&conf);

    wifi_connect(&wifi_login);
    mqttclient = mqtt_setup(MQTT_BROKER);
    // openlog_setup(&logger);
    // vTaskDelay(10000 / portTICK_RATE_MS);

    process_allocate(&pipeline, &conf);
    sender_setup();

    xTaskCreate(sampler_task, "sampling", 4096, NULL, 1, &sample_tick);
    xTaskCreate(pipeline_task, "x_pipeline", 4096, (void *)0, 1, NULL);
    // xTaskCreate(pipeline_task, "y_pipeline", 4096, (void *)1, 1, NULL);
    // xTaskCreate(pipeline_task, "z_pipeline", 4096,(void *)2, 1, NULL);
    xTaskCreate(mqtt_sender_task, "mqtt_sender", 4096, NULL, 1, NULL);
    clock_setup(conf.sensor.frequency, isr_sample);
}

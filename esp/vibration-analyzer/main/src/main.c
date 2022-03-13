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

// Set default values / / save, load from nvs
// TODO: ochráň mutexom
// Nastavenia - pri zmene realokovať všetky polia a uvoľniť
static const Configuration conf = {
    .sensor = {
        .frequency = 2,   // nastav podľa toho hw (min toľko)
        .n = 16,
        .range = IMU_2G
    },
    .tsmooth = {
        .enable = false,
        .n = 4,
        .repeat = 1
    },
    .stats = {
        .min = 0,
        .max = 0,
        .rms = 0,
        .mean = 0,
        .variance = 0,
        .std = 0,
        .skewness = 0,
        .kurtosis = 0,
        .median = 0,
        .mad = 0,
        .corr_xy = 0,
        .corr_yz = 0,
        .corr_xz = 0
    },
    .transform = {
        .window = HAMMING_WINDOW,
        .overlap = 0,   // zapamätaj si (1 - overlap) do ďalšieho kola
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
            .t = -10
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
        .stats = false,
        .events = false,
        .spectra = false
    }
};

static TaskHandle_t sample_tick;
static struct {
    QueueHandle_t x;
    QueueHandle_t y;
    QueueHandle_t z;
} samples;
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
    float *x = malloc(conf.sensor.n * sizeof(*x));
    float *y = malloc(conf.sensor.n * sizeof(*y));
    float *z = malloc(conf.sensor.n * sizeof(*z));
    uint16_t idx = 0;

    while (1) {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == pdTRUE) {
            imu_acceleration(&imu, &x[idx], &y[idx], &z[idx]);
            if (++idx >= conf.sensor.n - 1) {
                idx = 0;
                xQueueSend(samples.x, x, portMAX_DELAY);
                // xQueueSend(samples.y, y, portMAX_DELAY);
                // xQueueSend(samples.z, z, portMAX_DELAY);
                // Odošli v texte vzorky na MQTT / OpenLog
            }
        }
    }

    free(x);
    free(y);
    free(z);
}

void pipeline_task(void *args)
{
    // Process one axis
    QueueHandle_t *source = (QueueHandle_t *)args;

    // Create object for buffers of each axis to realloc after config change
    float *axis = malloc(conf.sensor.n * sizeof(*axis));

    uint16_t smooth_length = max(conf.tsmooth.n, conf.fsmooth.n);
    float *out = malloc((conf.sensor.n + smooth_length - 1) * sizeof(*out));

    float *spectrum = malloc(2 * conf.sensor.n * sizeof(*spectrum));
    bool *peaks = malloc(conf.sensor.n * sizeof(*peaks));
    // Events array of length conf.sensor.n

    // Okná vytvor spoločné pre viaceré osi
    float *t_smooth = malloc(conf.tsmooth.n * sizeof(*t_smooth));
    mean_kernel(t_smooth, conf.tsmooth.n);
    float *f_smooth = malloc(conf.fsmooth.n * sizeof(*f_smooth));
    mean_kernel(f_smooth, conf.tsmooth.n);
    float *window = malloc(conf.sensor.n * sizeof(*window));
    switch (conf.transform.window) {
        case BOXCAR_WINDOW:
            boxcar_window(window, conf.sensor.n); break;
        case BARTLETT_WINDOW:
            bartlett_window(window, conf.sensor.n); break;
        case HANN_WINDOW:
            hann_window(window, conf.sensor.n); break;
        case HAMMING_WINDOW:
            hamming_window(window, conf.sensor.n); break;
        case BLACKMAN_WINDOW:
            blackman_window(window, conf.sensor.n); break;
    }

    dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);

    while (1) {
        if (xQueueReceive(*source, axis, portMAX_DELAY) == pdTRUE) {
            // 0. Odoslať vzorky po decimácii /stream/x binárne dáta IEEE754 float 

            for (int i = 0; i < conf.sensor.n; i++)
                ESP_LOGI("main", " %.3f", axis[i]);
            ESP_LOGI("main", "\n");

            // 1. Vyhladzovanie v časovej doméne
            if (conf.tsmooth.enable) {
                for (uint8_t k = 0; k < conf.tsmooth.repeat; k++) {
                    dsps_conv_f32_ae32(axis, conf.sensor.n, t_smooth, conf.tsmooth.n, out);
                    memcpy(axis, out, sizeof(*axis) * conf.sensor.n);
                }
            }

            // 1A. Štatistiky

            // 2. Transformácia do frekvenčnej domény
            // FFT
            const size_t n_bins = conf.sensor.n / 2;
            switch (conf.transform.func) {
                case DFT:
                    for (size_t i = 0; i < conf.sensor.n; i++) {
                        spectrum[2*i] = axis[i] * window[i];
                        spectrum[2*i+1] = 0;
                    }
                    dsps_fft2r_fc32_ae32(spectrum, conf.sensor.n);
                    dsps_bit_rev2r_fc32(spectrum, conf.sensor.n);
                    dsps_cplx2reC_fc32(spectrum, conf.sensor.n);
                    break;
                case DCT:
                    for (size_t i = 0; i < n_bins; i++) {
                        spectrum[i] = axis[i] * window[i];
                        spectrum[i + conf.sensor.n/2] = 0;
                    }
                    dsps_dct_f32(spectrum, n_bins);
                    break;
            }
            for (size_t i = 0; i < n_bins; i++) {
                spectrum[i] = 10 * log10f((
                    square(spectrum[i*2]) + square(spectrum[i*2+1])) / conf.sensor.n
                );
            }

            // 4. Vyhlazovanie vo frekvenciach
            if (conf.fsmooth.enable) {
                for (uint8_t k = 0; k < conf.fsmooth.repeat; k++) {
                    dsps_conv_f32_ae32(spectrum, n_bins, f_smooth, conf.tsmooth.n, out);
                    memcpy(axis, out, sizeof(*axis) * conf.sensor.n);
                }
            }

            // 5. Hľadanie špičiek
            switch (conf.peak.strategy) {
                case THRESHOLD:
                    find_peaks_above_threshold(
                        peaks, spectrum, n_bins, conf.peak.threshold.t
                    );
                    break;
                case NEIGHBOURS:
                    find_peaks_neighbours(
                        peaks, spectrum, n_bins,
                        conf.peak.neighbours.k, conf.peak.neighbours.e,
                        conf.peak.neighbours.h_rel, conf.peak.neighbours.h
                    );
                    break;
                case ZERO_CROSSING:
                    find_peaks_zero_crossing(
                        peaks, spectrum, n_bins,
                        conf.peak.zero_crossing.k, conf.peak.zero_crossing.slope
                    );
                    break;
                case HILL_WALKER:
                    find_peaks_hill_walker(
                        peaks, spectrum, n_bins,
                        conf.peak.hill_walker.tolerance, conf.peak.hill_walker.hole,
                        conf.peak.hill_walker.prominence, conf.peak.hill_walker.isolation
                    );
                    break;
            }

            // 6. Identifikácia udalostí
            // events_detect();

            // Pipeline:
            // A: Nastavenie konfigurácie Globálny konfiguračný objekt (štruktúra)
            // 0. Bežiaci priemer na odstránenie zerog
            // 3. Priemerovanie spektrogramov (Welch)
            // 6. Identifikácia udalostí
            // 7. Odoslanie udalostí

            //Doplnkové: Odoslanie vzoriek po decimácii, Odoslanie štatistík, Odoslanie spektra

            /* Send via MQTT (experimental)
            for (int i = 0; i < BUFFER_LEN; i++) {
                snprintf(msg.msg, MSG_MAX_LEN, "%d %d %d", buffer.a[i].x, buffer.a[i].y, buffer.a[i].z);
                xQueueSend(mailbox, &msg, portMAX_DELAY);
            }*/

            // ESP_LOGI("main", msg.msg);
            //OpenLog: printf("%d %d %d\n", accel.x, accel.y, accel.z);
        }
    }

    free(axis);
    free(out);
    free(t_smooth);
    free(f_smooth);
    free(window);
    free(spectrum);
    free(peaks);
}

/*
void mqtt_sender_task()
{
    MessageStream msg;
    while (1) {
        if (xQueueReceive(mailbox, &msg, portMAX_DELAY) == pdTRUE) {
            esp_mqtt_client_publish(mqttclient, "imu/1/stream", msg.msg, 0, 1, 0);
        }
    }
}*/

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
    // wifi_connect(&wifi_login);
    // mqttclient = mqtt_setup(MQTT_BROKER);
    // openlog_setup(&logger);
    // vTaskDelay(10000 / portTICK_RATE_MS);

    samples.x = xQueueCreate(3, sizeof(float) * conf.sensor.n);
    samples.y = xQueueCreate(3, sizeof(float) * conf.sensor.n);
    samples.z = xQueueCreate(3, sizeof(float) * conf.sensor.n);

    xTaskCreate(sampler_task, "sampling", 1024, NULL, 1, &sample_tick);
    xTaskCreate(pipeline_task, "x_pipeline", 1024, &samples.x, 1, NULL);
    // xTaskCreate(pipeline_task, "y_pipeline", 1024, &samples.y, 1, NULL);
    // xTaskCreate(pipeline_task, "z_pipeline", 1024, &samples.z, 1, NULL);
    // xTaskCreate(mqtt_sender_task, "mqtt_sender", 4096, NULL, 1, NULL);
    clock_setup(conf.sensor.frequency, isr_sample);
}

#include "esp_system.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/event_groups.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "peripheral.h"


static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static void wifi_event_handler(void* arg, esp_event_base_t event_base, 
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && 
            (event_id == WIFI_EVENT_STA_START ||
             event_id == WIFI_EVENT_STA_DISCONNECTED)) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT &&
               event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_connect(wifi_config_t *wifi_config)
{
    s_wifi_event_group = xEventGroupCreate();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();

    esp_wifi_init(&config);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config);
    esp_wifi_start();

    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    vEventGroupDelete(s_wifi_event_group);
}

esp_mqtt_client_handle_t mqtt_setup(const char *broker_url)
{
    esp_mqtt_client_config_t config = {.uri = broker_url};
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&config);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    return client;
}

void openlog_setup(OpenLog *logger)
{
    gpio_config_t openlog_vcc = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1 << logger->vcc),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    gpio_config(&openlog_vcc);
    gpio_set_level(logger->vcc, 1);

    uart_config_t uart_config = {
        .baud_rate = logger->baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };

    uart_driver_install(logger->uart, logger->buffer * 2, 0, 0, NULL, 0);
    uart_param_config(logger->uart, &uart_config);
    uart_set_pin(logger->uart, logger->tx, logger->rx, 
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void clock_setup(uint16_t frequency, timer_isr_t action)
{
    timer_config_t timer_config = {
        .divider = TIMER_DIVIDER,
        .alarm_en = TIMER_ALARM_EN,
        .counter_en = TIMER_PAUSE,
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP,
        .auto_reload = TIMER_AUTORELOAD_EN,
    };
    timer_init(TIMER_GROUP_0, TIMER_0, &timer_config);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_SCALE / frequency);

    timer_enable_intr(TIMER_GROUP_0, TIMER_0);
    timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, action, NULL, 0);

    timer_start(TIMER_GROUP_0, TIMER_0);
}

void clock_reconfigure(uint16_t frequency)
{
    timer_pause(TIMER_GROUP_0, TIMER_0);
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, TIMER_SCALE / frequency);
    timer_start(TIMER_GROUP_0, TIMER_0);
}

esp_err_t nvs_load(Configuration *conf, Provisioning *login)
{
    nvs_handle_t storage;
    size_t len = 0;
    size_t len_login = 0;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &storage);
    if (err != ESP_OK) return err;

    err = nvs_get_blob(storage, "config", NULL, &len);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    err = nvs_get_blob(storage, "login", NULL, &len_login);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    // Set default values if does not exist
    if (len == 0 || len_login == 0) {
        if (len == 0) {
            err = nvs_set_blob(storage, "config", conf, len);
            if (err != ESP_OK) return err;
        }
        if (len_login == 0) {
            err = nvs_set_blob(storage, "login", conf, len_login);
            if (err != ESP_OK) return err;
        }
        err = nvs_commit(storage);
        if (err != ESP_OK) return err;
        len = sizeof(*conf);
    }

    err = nvs_get_blob(storage, "config", conf, &len);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    err = nvs_get_blob(storage, "login", login, &len_login);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    nvs_close(storage);
    return ESP_OK;
}

esp_err_t nvs_save_config(Configuration *conf)
{
    nvs_handle_t storage;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &storage);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(storage, "config", conf, sizeof(*conf));
    if (err != ESP_OK) return err;

    err = nvs_commit(storage);
    if (err != ESP_OK) return err;
    nvs_close(storage);
    return ESP_OK;
}

esp_err_t nvs_save_login(Provisioning *login)
{
    nvs_handle_t storage;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &storage);
    if (err != ESP_OK) return err;

    err = nvs_set_blob(storage, "login", login, sizeof(*login));
    if (err != ESP_OK) return err;

    err = nvs_commit(storage);
    if (err != ESP_OK) return err;
    nvs_close(storage);
    return ESP_OK;
}

void axis_mqtt_topics(MqttAxisTopics *topics, int axis)
{
    const static char *dirs[] = {"x", "y", "z"};

    strncpy(topics->stats, MQTT_TOPIC_STATS, TOPIC_LENGTH - 1);
    strcat(topics->stats, dirs[axis]);

    strncpy(topics->spectra, MQTT_TOPIC_SPECTRUM, TOPIC_LENGTH - 1);
    strcat(topics->spectra, dirs[axis]);

    strncpy(topics->events, MQTT_TOPIC_EVENT, TOPIC_LENGTH - 1);
    strcat(topics->events, dirs[axis]);
}

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
    window(w, p->window, n);

    for (uint8_t i = 0; i < AXIS_COUNT; i++)
        p->queue[i] = xQueueCreate(conf->sensor.frequency, sizeof(float));

    p->corr.barrier = xEventGroupCreate();
    p->corr.task_mask = 0;
    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        p->corr.diff[i] = malloc(conf->sensor.n * sizeof(float));
        if (conf->sensor.axis[i]) 
            p->corr.task_mask |= (1 << i);
    }
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
    sender->max_send_samples = length;
    sender->raw_stream = xQueueCreate(
        SAMPLES_QUEUE_SLOTS * sender->max_send_samples, sizeof(float)
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
    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        vQueueDelete(p->queue[i]);
        free(p->corr.diff[i]);
    }
}

void axis_release(BufferPipelineAxis *p)
{
    free(p->stream);
    free(p->tmp_conv);
    free(p->spectrum);
    free(p->peaks);
    free(p->events);
}

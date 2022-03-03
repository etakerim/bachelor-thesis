#include "esp_system.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/event_groups.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/uart.h"

#include "mqtt_client.h"
#include "esp_wifi.h"

#include "peripheral.h"


static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
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


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    // esp_mqtt_event_handle_t event = event_data;
    // int msg_id;
    // esp_mqtt_client_handle_t client = event->client;
    // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
    // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
    // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);

    // event->topic_len 
    // event->topic
    // event->data_len
    // event->data

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            break;
        case MQTT_EVENT_DISCONNECTED:
            break;
        case MQTT_EVENT_SUBSCRIBED:
            break;
        case MQTT_EVENT_DATA:
            break;
        default:
            break;
    }
}

void peripheral_setup(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }

    esp_netif_init();
    esp_event_loop_create_default();
}

void wifi_connect(void)
{
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
    	    .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    s_wifi_event_group = xEventGroupCreate();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();


    esp_wifi_init(&config);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    vEventGroupDelete(s_wifi_event_group);
}

void mqtt_setup(const char *broker_url)
{
    esp_mqtt_client_config_t config = {.uri = broker_url};
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&config);

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void nvs_load_rules(void)
{
    nvs_handle_t storage;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &storage);

    if (err == ESP_OK) {
        int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
        err = nvs_get_i32(storage, "restart_counter", &restart_counter);
        
        switch (err) {
            case ESP_OK:
                break;   // printf("Restart counter = %d\n", restart_counter);
            case ESP_ERR_NVS_NOT_FOUND:
                break;  //  printf("The value is not initialized yet!\n");
            default:
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        restart_counter++;
        err = nvs_set_i32(storage, "restart_counter", restart_counter);
        err = nvs_commit(storage);
        nvs_close(storage);
        // printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    }
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
    vTaskDelay(10000 / portTICK_RATE_MS);

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

int openlog_write(OpenLog *logger, char *str)    
{
    return uart_write_bytes(logger->uart, str, strlen(str));
}


#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include <stdint.h>
#include "hal/gpio_types.h"
#include "driver/timer.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "pipeline.h"


// TIMER_SCALE is value of timer at 1 second
#define TIMER_DIVIDER         16
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)

typedef struct {
    gpio_num_t vcc;
    uint8_t uart; 
    gpio_num_t rx;
    gpio_num_t tx;
    int baudrate;
    int buffer;
} OpenLog;


void peripheral_setup(void);
esp_err_t nvs_load_config(Configuration *conf);
esp_err_t nvs_save_config(Configuration *conf);

void openlog_setup(OpenLog *logger);
int openlog_write(OpenLog *logger, char *str);

void wifi_connect(wifi_config_t *wifi_config);
esp_mqtt_client_handle_t mqtt_setup(const char *broker_url);

void clock_setup(uint16_t frequency, timer_isr_t action);
void clock_reconfigure(uint16_t frequency);

#endif
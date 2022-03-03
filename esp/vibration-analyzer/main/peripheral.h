#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include <stdint.h>
#include "hal/gpio_types.h"


#define WIFI_SSID  ""
#define WIFI_PASS  ""

typedef struct {
    gpio_num_t vcc;
    uint8_t uart; 
    gpio_num_t rx;
    gpio_num_t tx;
    int baudrate;
    int buffer;
} OpenLog;


void peripheral_setup(void);
void nvs_load_rules(void);

void openlog_setup(OpenLog *logger);
int openlog_write(OpenLog *logger, char *str);

void wifi_connect(void);
void mqtt_setup(const char *broker_url);

#endif PERIPHERAL_H
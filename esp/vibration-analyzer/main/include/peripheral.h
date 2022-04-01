#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include <stdint.h>
#include "hal/gpio_types.h"
#include "driver/timer.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "pipeline.h"


/** @defgroup peripheral Hardvérové adaptéry
 *  @{
 */


/**
 * @brief Prefix pre všetky MQTT témy zariadenia. Vo formáte "imu/{device id}/"
 */
#define DEVICE_MQTT_TOPIC           "imu/1/"

#define TOPIC_LENGTH                64
#define LARGEST_MESSAGE             ((MAX_BUFFER_SAMPLES * (sizeof(float) + 1)) + (2 * TOPIC_LENGTH))
#define DEVICE_MQTT_TOPIC_LENGTH    sizeof(DEVICE_MQTT_TOPIC) / sizeof(char)

#define MQTT_TOPIC_REQUEST          DEVICE_MQTT_TOPIC "config/request"
#define MQTT_TOPIC_RESPONSE         DEVICE_MQTT_TOPIC "config/response"
#define MQTT_TOPIC_CONFIG           DEVICE_MQTT_TOPIC "config/set"
#define MQTT_TOPIC_LOGIN            DEVICE_MQTT_TOPIC "login/set"
#define MQTT_TOPIC_SYSLOG           DEVICE_MQTT_TOPIC "syslog"
#define MQTT_TOPIC_STREAM           DEVICE_MQTT_TOPIC "stream/samples"
#define MQTT_TOPIC_STATS            DEVICE_MQTT_TOPIC "stream/statistics/"
#define MQTT_TOPIC_SPECTRUM         DEVICE_MQTT_TOPIC "stream/frequency/"
#define MQTT_TOPIC_EVENT            DEVICE_MQTT_TOPIC "event/frequency/"

/**
 * @brief SparkFun OpenLog - zaznenávač údajov na SD kartu cez sériovú linku
 */
typedef struct {
    gpio_num_t vcc;     /**< @brief GPIO pin na ovládanie napájania cez FET tranzistor */
    uint8_t uart;       /**< @brief Číslo UART rozhrania */
    gpio_num_t rx;      /**< @brief GPIO pin UART RX */
    gpio_num_t tx;      /**< @brief GPIO pin UART TX */
    int baudrate;       /**< @brief Baudová rýchlosť komunikácie. Rovnaká rýchlosť musí byť nastavená v config.txt na SD karte */
    int buffer;         /**< @brief Dĺžka vyrovnávacej pamäte pre UART */
} OpenLog;

/**
 * @brief MQTT témy pre os zrýchlenia
 */
typedef struct {
    char stats[TOPIC_LENGTH];       /**< @brief Názov témy pre štatistické údaje */
    char spectra[TOPIC_LENGTH];     /**< @brief Názov témy pre frekvenčné spektum */
    char events[TOPIC_LENGTH];      /**< @brief Názov témy pre udalosti zmeny spektra */
} MqttAxisTopics;


/**
 * @brief Poskladanie názvu MQTT tém pre odosieanie dát o osi akcelerácie
 * 
 * @param[out]  topics  MQTT témy zložené s označením osi x, y, z
 * @param[in]   axis    Index osi vektora akcelerácie: 0, 1, 2
 */
void axis_mqtt_topics(MqttAxisTopics *topics, int axis);


/**
 * @brief Načítanie nastavení systému z nevolatilného úložiska
 * 
 * @param[out] conf    Globálne nastavenia spracovania dát
 * @param[out] login   Nastavenie sieťového pripojenia 
 */
esp_err_t nvs_load(Configuration *conf, Provisioning *login);

/**
 * @brief Uloženie nastavení systému na nevolatilné úložisko
 * 
 * @param[in] conf    Globálne nastavenia spracovania dát
 */
esp_err_t nvs_save_config(const Configuration *conf);

/**
 * @brief Uloženie nastavení sieťového pripojenia na nevolatilné úložisko
 * 
 * @param[in] login    Nastavenie sieťového pripojenia
 */
esp_err_t nvs_save_login(const Provisioning *login);

/**
 * @brief Nastavenie UART rozhrania pre zariadenie OpenLog
 * 
 * @param logger    OpenLog logger
 */
void openlog_setup(OpenLog *logger);

/**
 * @brief Dopredná deklarácia spätného volania. Implementáciu musí poskytnúť hlavný program
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

/**
 * @brief Pripojenie sa k MQTT broker a zaregistrovanie spätného volanie pre všetky udalosti
 * 
 * @param broker_url    URL MQTT broker
 */
esp_mqtt_client_handle_t mqtt_setup(const char *broker_url);


/**
 * @brief Pripojenie sa k Wifi AP blokujúce
 * 
 * @param wifi_config    Nastavenie Wifi AP
 */
void wifi_connect(wifi_config_t *wifi_config);

/**
 * @brief Spustenie časovača na vzorkovanie signálu
 * 
 * @param frequency    Vzorkovacia frekvencia v Hz
 * @param action       Obsluha prerušenia časovača s predpisom: `bool IRAM_ATTR f(void *args)`
 */
void clock_setup(uint16_t frequency, timer_isr_t action);

/**
 * @brief Zmena frekvencie časovača
 * 
 * @param frequency    Vzorkovacia frekvencia v Hz
 */
void clock_reconfigure(uint16_t frequency);

/** @} */

#endif
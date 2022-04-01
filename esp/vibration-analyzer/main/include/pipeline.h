#ifndef PIPELINE_H
#define PIPELINE_H

#include "esp_dsp.h"
#include "inertial_unit.h"
#include "freertos/queue.h"
#include "freertos/message_buffer.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "mpack.h"
#include "events.h"

/** @defgroup pipeline Dátovod
 *  @{
 */


#define AXIS_COUNT                  3

#define MAX_MPACK_FIELDS_COUNT      20
#define SAMPLES_QUEUE_SLOTS         3
#define MAX_CREDENTIALS_LENGTH      64

#define MAX_BUFFER_SAMPLES          1024
#define MAX_FREQUENCY               952
#define MAX_MQTT_URL                256
#define MAX_OVERLAP                 0.8
#define MAX_SMOOTH_REPEAT           8

typedef enum {
    BOXCAR_WINDOW,
    BARTLETT_WINDOW,
    HANN_WINDOW,
    HAMMING_WINDOW,
    BLACKMAN_WINDOW,
    WINDOW_TYPE_COUNT
} WindowTypeConfig;

typedef enum {
    THRESHOLD,
    NEIGHBOURS,
    ZERO_CROSSING,
    HILL_WALKER,
    STRATEGY_COUNT
} PeakFindingStrategy;

typedef enum {
    DFT, DCT,
    TRANSFORM_COUNT
} FrequencyTransform;

typedef struct {
    uint16_t frequency;
    AccelerationRange range;
    uint16_t n;
    float overlap;
    bool axis[AXIS_COUNT];
} SamplingConfig;

typedef struct {
    bool enable;
    uint16_t n;
    uint8_t repeat;
} SmoothingConfig;

typedef struct {
    bool min;
    bool max;
    bool rms;
    bool mean;
    bool variance;
    bool std;
    bool skewness;
    bool kurtosis;
    bool median;
    bool mad;
    bool correlation;
} StatisticsConfig;

typedef struct {
    WindowTypeConfig window;
    FrequencyTransform func;
    bool log;
} FFTTransformConfig;

typedef struct {
    uint16_t min_duration;
    uint16_t time_proximity;
    PeakFindingStrategy strategy;
    struct {
        float t;
    } threshold;
    struct {
        uint16_t k;
        float e;
        float h_rel;
        float h;
    } neighbours;
    struct {
        uint16_t k;
        float slope;
    } zero_crossing;
    struct {
        float tolerance;
        int hole;
        float prominence;
        float isolation;
    } hill_walker;
} EventDetectionConfig;


typedef struct {
    uint16_t subsampling;
    bool openlog_raw_samples;
    bool mqtt_samples;
    bool mqtt_stats;
    bool mqtt_spectra;
    bool mqtt_events;
    char mqtt_url[MAX_MQTT_URL];
} SaveFormatConfig;


typedef struct {
    SamplingConfig sensor;
    SmoothingConfig tsmooth;
    StatisticsConfig stats;
    FFTTransformConfig transform;
    SmoothingConfig fsmooth;
    EventDetectionConfig peak;
    SaveFormatConfig logger;
} Configuration;


typedef struct {
    float min;
    float max;
    float rms;
    float mean;
    float var;
    float std;
    float skew;
    float kurtosis;
    float median;
    float mad;
    float corr_xy;
    float corr_xz;
    float corr_yz;
} Statistics;


// Buffers
typedef struct {
    EventGroupHandle_t barrier;
    EventBits_t task_mask;
    float *diff[AXIS_COUNT];
    float std[AXIS_COUNT];
} Correlation;

typedef struct {
    float *smooth;
    float *window;
    QueueHandle_t queue[AXIS_COUNT];
    Correlation corr;
} BufferPipelineKernel;

typedef struct {
    float *stream;
    float *tmp_conv;
    float *spectrum;
    bool *peaks;
    SpectrumEvent *events;
} BufferPipelineAxis;

typedef struct {
    uint16_t max_send_samples;
    QueueHandle_t raw_stream;
} Sender;

/**
 * @brief Sieťové nastavenia pre pripojenie na Wifi AP s WPA2 a MQTT broker 
 */
typedef struct {
    char wifi_ssid[MAX_CREDENTIALS_LENGTH];     /**< @brief Wifi AP SSID */
    char wifi_pass[MAX_CREDENTIALS_LENGTH];     /**< @brief Wifi AP heslo */
    char mqtt_url[MAX_CREDENTIALS_LENGTH];      /**< @brief URL MQTT brokera "mqtt://..."" */
} Provisioning;


#define square(x)   ((x) * (x))
#define min(x, y)   (((x) < (y)) ? (x): (y))
#define max(x, y)   (((x) > (y)) ? (x): (y))

// window.c
/** @defgroup window Oknové funkcie
 *  @{
 */

/**
 * @brief Vyhladzovací filter kĺzavého priemeru
 * 
 *  \f$ w(n) = \frac{1}{n},\, n = 0, 1, ..., N - 1 \f$
 * 
 * @param[out]  w   1D maska kĺzavého priemeru
 * @param[in]   n   Dĺžka masky
 */
void mean_kernel(float *w, int n);

/**
 * @brief Obdĺžníkové okno
 * 
 * \f$ w(n) = \frac{2}{N - 1}\left(\frac{N - 1}{2} - \left|n - \frac{N - 1}{2} \right|\right) \f$
 * @param[out]  w   Váhy oknovej funkcie
 * @param[in]   n   Dĺžka okna
 */
void boxcar_window(float *w, int n);

/**
 * @brief Bartlettovo okno
 * 
 * \f$ w(n) = \frac{2}{N - 1}\left(\frac{N - 1}{2} - \left|n - \frac{N - 1}{2} \right|\right) \f$
 * @param[out]  w   Váhy oknovej funkcie
 * @param[in]   n   Dĺžka okna
 */
void bartlett_window(float *w, int n);

/**
 * @brief Hannovo okno
 * 
 * \f$ w(n) = \sin^2(\pi n / N) \f$
 * @param[out]  w   Váhy oknovej funkcie
 * @param[in]   n   Dĺžka okna
 */
void hann_window(float *w, int n);

/**
 * @brief Hammingovo okno
 * 
 * \f$ w(n) = 0.54 - 0.46\cos(2\pi n / N) \f$
 * @param[out]  w   Váhy oknovej funkcie
 * @param[in]   n   Dĺžka okna
 */
void hamming_window(float *w, int n);

/**
 * @brief Blackmanovo okno
 * 
 * \f$ w(n) = 0.42 - 0.5\cos(2\pi n / N) + 0.08\cos(4\pi n / N) \f$
 * 
 * @param[out]  w   Váhy oknovej funkcie
 * @param[in]   n   Dĺžka okna
 */
void blackman_window(float *w, int n);

/**
 * @brief Oknová funkcia podľa voľby
 * 
 * @param[out]  type    Oknová funkcia
 * @param[out]  w       Váhy oknovej funkcie
 * @param[in]   n       Dĺžka okna
 */
void window(WindowTypeConfig type, float *w, int n);

/** @} */

// statistics.c
/** @defgroup statistics Deskriptívna štatistika
 *  @{
 */

/**
 * @brief Najnižšia hodnota
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  Mimimum z hodnôt signálu
 */
float minimum(const float *x, int n);
/**
 * @brief Najvyššia hodnota
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  Maximum z hodnôt signálu
 */
float maximum(const float *x, int n);

/**
 * @brief Stredná kvadratická odchýlka
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  RMS z hodnôt signálu
 */
float root_mean_square(const float *x, int n);

/**
 * @brief Aritmetický výberový priemer
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  Priemer hodnôt signálu
 */
float mean(const float *x, int n);

/**
 * @brief Rozptyl populácie
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  Rozptyl hodnôt signálu
 */
float variance(const float *x, int n, float mean);

/**
 * @brief Smerodajná odchýlka
 *  
 * @param[in]  x   Vzorky signálu
 * @param[in]  n   Počet vzoriek signálu
 * @return  smerodajná odchýlka hodnôt signálu
 */
float standard_deviation(float variance);

/**
 * @brief Centrálny moment
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  m    Rád centrálneho momentu
 * @param[in]  mean Aritmetický priemer signálu
 * @return  Centrálny moment
 */
float moment(const float *x, int n, int m, float mean);

/**
 * @brief Šikmosť
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  mean Aritmetický priemer signálu
 * @return  Šikmosť
 */
float skewness(const float *x, int n, float mean);

/**
 * @brief Špicatosť
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  mean Aritmetický priemer signálu
 * @return  Špicatosť
 */
float kurtosis(const float *x, int n, float mean);

/**
 * @brief Korelácia
 *  
 * @param[in]  x_diff    Predspracované vzorky prvého signálu odčítané od aritmetického priemeru: \f$ (x_i - \bar{x}) \f$
 * @param[in]  y_diff    Predspracované vzorky druhého signálu odčítané od aritmetického priemeru: \f$ (y_i - \bar{y}) \f$
 * @param[in]  n         Počet vzoriek signálu. Dĺžky oboch polí musia byť rovnaké.
 * @param[in]  x_std     Smerodajná odchýlka prvého signálu
 * @param[in]  y_std     Smerodajná odchýlka druhého signálu
 * @return  Pearsonov korelačný koeficient
 */
float correlation(const float *x_diff, const float *y_diff, int n, float x_std, float y_std);


/**
 * @brief Quickselect. Algoritmus na nájdenie k-teho najmenšieho prvku v nezoradenom poli.
 *  
 * @param      x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  k    Rád k-teho najmenšieho prvku
 * @return  K-ty najmenší prvok
 */
float quickselect(const float *x, int n, int k);

/**
 * @brief Medián
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @return  Medián
 */
float median(const float *x, int n);

/**
 * @brief Mediánová absolútna odchýlka (MAD)
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  med  Medián signálu
 * @return  MAD
 */
float median_abs_deviation(const float *x, int n, float med);

/**
 * @brief Priemerná absolútna odchýlka (AAD)
 *  
 * @param[in]  x    Vzorky signálu
 * @param[in]  n    Počet vzoriek signálu
 * @param[in]  mean Aritmetický priemer signálu
 * @return  AAD
 */
float average_abs_deviation(const float *x, int n, float mean);
/** @} */


/** 
 * @defgroup pipeline_buffer Správa pamäti dátovodu
 * @{
 */
void process_allocate(BufferPipelineKernel *p, Configuration *conf);
void axis_allocate(BufferPipelineAxis *p, Configuration *conf);
void sender_allocate(Sender *sender, uint16_t length);
void sender_allocate(Sender *sender, uint16_t n);

void sender_release(Sender *sender);
void process_release(BufferPipelineKernel *p);
void axis_release(BufferPipelineAxis *p);
void sender_release(Sender *sender);
/** @} */



/** 
 * @defgroup stages Fázy spracovania oknovaného signálu
 * @{
 */
void buffer_shift_left(float *buffer, uint16_t n, uint16_t k);
void process_statistics(float *buffer, uint16_t n, Statistics *stats, const StatisticsConfig *c);
void process_correlation(uint8_t axis, const float *buffer, Statistics *stats, Correlation *corr, SamplingConfig *conf);
int process_spectrum(float *spectrum, const float *buffer, const float *window, uint16_t n, const FFTTransformConfig *c);
void process_smoothing(float *buffer, float *tmp, uint16_t n, float *kernel, const SmoothingConfig *c);
void process_peak_finding(bool *peaks, const float *spectrum, uint16_t bins, const EventDetectionConfig *c);
/** @} */

// serialize.c
/** 
 * @defgroup stages Serializácia a parsovanie nameraných dát a konfigurácie
 * @{
 */
size_t stream_serialize(char *msg, size_t size, float *stream, size_t n);
size_t stats_serialize(size_t timestamp, char *msg, size_t size, const Statistics *stats, const StatisticsConfig *c);
size_t spectra_serialize(size_t timestamp, char *msg, size_t size, float *spectrum, size_t n, uint16_t fs);
size_t events_serialize(size_t timestamp, float bin_width, char *msg, size_t size, SpectrumEvent *events, size_t n);

size_t config_serialize(char *msg, size_t size, const Configuration *config);
bool config_parse(char *msg, int size, Configuration *conf, bool *change);

size_t login_serialize(char *msg, size_t size, const Provisioning *conf);
bool login_parse(char *msg, size_t size, Provisioning *conf);
/** @} */

/** @} */

#endif
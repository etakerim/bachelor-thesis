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

/**
 * @brief Celkový počet osí akcelerácie
 */
#define AXIS_COUNT                  3
/**
 * @brief Maximálny počet dvojíc kľúč - hodnota v Message Pack slovníku
 */
#define MAX_MPACK_FIELDS_COUNT      20
/**
 * @brief Násobok veľkosti posuvného okna ako počet vzoriek čakajúcich na spracovanie vo fronte
 */
#define SAMPLES_QUEUE_SLOTS         3
/**
 * @brief Maximálna dĺžka prihlasovacieho údaju Wifi pripojenia
 */
#define MAX_CREDENTIALS_LENGTH      64
/**
 * @brief Dĺžka URL adresy na MQTT broker 
 */
#define MAX_MQTT_URL                256

/**
 * @brief Najdlhšie povolené posuvné okno (vyššia mocnina 2 ako 1024 sa nezmestí do DRAM)
 */
#define MAX_BUFFER_SAMPLES          1024
/**
 * @brief Najvyššia vzorkovacia frekvencia daná fyzickým obmedzením akcelerometra
 */
#define MAX_FREQUENCY               952
/**
 * @brief Maximálny prekryv posuvných okien v rozsahu 0 - 1
 */
#define MAX_OVERLAP                 0.8
/**
 * @brief Maximálny počet prechodu konvolučnej masky vyhladzovacieho filtra
 */
#define MAX_SMOOTH_REPEAT           8

/**
 * @brief Oknové funkcie 
 */
typedef enum {
    BOXCAR_WINDOW,      /**< @brief Obdĺžníkové okno */
    BARTLETT_WINDOW,    /**< @brief Bartlettovo okno */
    HANN_WINDOW,        /**< @brief Hannovo okno */
    HAMMING_WINDOW,     /**< @brief Hammingovo okno */
    BLACKMAN_WINDOW,    /**< @brief Blackmanovo okno */
    WINDOW_TYPE_COUNT   /**< @brief Počet dostupných oknových funkcií. Potrebné pre serializáciu */
} WindowTypeConfig;

/**
 * @brief Algoritmy na hľadanie špičiek 
 */
typedef enum {
    THRESHOLD,          /**< @brief Špičky nad prahovou úrovňou **/
    NEIGHBOURS,         /**< @brief Špičky najvýznačnejšieho bodu spomedzi susedov **/
    ZERO_CROSSING,      /**< @brief Špičky prechodou nulou do záporu **/
    HILL_WALKER,        /**< @brief Špičky algoritmom horského turistu **/
    STRATEGY_COUNT      /**< @brief Počet dostupných algoritmov. Potrebné pre serializáciu */
} PeakFindingStrategy;

/**
 * @brief Frekvenčné transformácie 
 */
typedef enum {
    DFT,                /**< @brief Rýchla Fourierová transformácia radix-2 */
    DCT,                /**< @brief Konsínusová transformácia DCT II */
    TRANSFORM_COUNT     /**< @brief Počet dostupných frekvenčných transformácií. Potrebné pre serializáciu */
} FrequencyTransform;

/**
 * @brief Nastavenie vzorkovania signálu
 */
typedef struct {
    uint16_t frequency;         /**< @brief Vzorkovacia frekvencia v Hz. Najviac `MAX_FREQUENCY` */
    AccelerationRange range;    /**< @brief Dynamický rozsah akcelerometra */
    uint16_t n;                 /**< @brief Veľkosť posuvného okna. Musí byť mocninou dvojky a najviac `MAX_BUFFER_SAMPLES` */
    float overlap;              /**< @brief Pomer prekryvu posuvných okien. Rozsah: 0 až `MAX_OVERLAP` */
    bool axis[AXIS_COUNT];      /**< @brief Osi akcelerácie povolené na spracovanie */
} SamplingConfig;

/**
 * @brief Nastavenie vyhladzovania signálu alebo frekvenčného spektra
 */
typedef struct {
    bool enable;        /**< @brief Vyhladzovanie signálu povolené */
    uint16_t n;         /**< @brief Dĺžka konvolučnej masky */
    uint8_t repeat;     /**< @brief Počet prechodov konvolučnej masky. Najviac `MAX_SMOOTH_REPEAT` */
} SmoothingConfig;

/**
 * @brief Nastavenie zberu štatistík posuvného okna
 */
typedef struct {
    bool min;           /**< @brief Výpočet minimálnej hodnoty povolený */
    bool max;           /**< @brief Výpočet maximálnej hodnoty povolený */
    bool rms;           /**< @brief Výpočet strednej kvadratickej odchýlky povolený */
    bool mean;          /**< @brief Výpočet aritmetického priemeru povolený */
    bool variance;      /**< @brief Výpočet rozptylu povolený */
    bool std;           /**< @brief Výpočet smerodajnej odchýlky povolený */
    bool skewness;      /**< @brief Výpočet šikmosti povolený */
    bool kurtosis;      /**< @brief Výpočet špicatosti povolený */
    bool median;        /**< @brief Výpočet mediánu povolený */
    bool mad;           /**< @brief Výpočet mediánovej absolútnej odchýlky povolený */
    bool correlation;   /**< @brief Výpočet korelácie medzi osami povolený */
} StatisticsConfig;

/**
 * @brief Nastavenie frekvenčnej transformácie
 */
typedef struct {
    WindowTypeConfig window;    /**< @brief Oknová funkcia */
    FrequencyTransform func;    /**< @brief Typ frekvenčnej transformácie */
    bool log;                   /**< @brief Prevod magnitúdy frekvencie do dB */
} FFTTransformConfig;


/**
 * @brief Nastavenia algoritmov na detekciu udalostí a ich parametrov 
 */
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

/**
 * @brief Nastavenia ukladania a posielania spracovaných dát
 * 
 * OpenLog bude po spustený aktívny iba ako je povolené `openlog_raw_samples`.
 * Wifi pripojenie a MQTT klient budú spusetný iba ak aspoň jedna z možností `mqtt_*` je povolená.
 */
typedef struct {
    uint16_t subsampling;       /**< @brief Podvzorkovanie pre záznam vzoriek bez ďalšieho spracovania. Preskočí sa každých `subsampling` vzoriek */
    bool openlog_raw_samples;   /**< @brief Záznam vzoriek na SD kartu povolený. OpenLog bude zapnutý po spustení */
    bool mqtt_samples;          /**< @brief Odosielanie vzoriek cez MQTT na topic podľa `MQTT_TOPIC_STREAM` */
    bool mqtt_stats;            /**< @brief Odosielanie štatistík cez MQTT na topic podľa `MQTT_TOPIC_STATS` */
    bool mqtt_spectra;          /**< @brief Odosielanie frekvenčného spektra cez MQTT na topic podľa `MQTT_TOPIC_SPECTRUM` */
    bool mqtt_events;           /**< @brief Odosielanie zmien spektra cez MQTT na topic podľa `MQTT_TOPIC_EVENT` */
} SaveFormatConfig;

/**
 * @brief Systémová konfigurácia dátovodu spracovania vzoriek z akcelerometra
 */
typedef struct {
    SamplingConfig sensor;
    SmoothingConfig tsmooth;
    StatisticsConfig stats;
    FFTTransformConfig transform;
    SmoothingConfig fsmooth;
    EventDetectionConfig peak;
    SaveFormatConfig logger;
} Configuration;


/**
 * @brief Sieťové nastavenia pre pripojenie na Wifi AP s WPA2 a MQTT broker 
 */
typedef struct {
    char wifi_ssid[MAX_CREDENTIALS_LENGTH];     /**< @brief Wifi AP SSID */
    char wifi_pass[MAX_CREDENTIALS_LENGTH];     /**< @brief Wifi AP heslo */
    char mqtt_url[MAX_MQTT_URL];      /**< @brief URL MQTT brokera "mqtt://..."" */
} Provisioning;


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


typedef struct {
    EventGroupHandle_t barrier;
    EventBits_t task_mask;
    float *diff[AXIS_COUNT];
    float std[AXIS_COUNT];
} Correlation;

/**
 * @brief Vyrovnávacie pamäte spoločné pre celú pipeline
 */
typedef struct {
    float *smooth;
    float *window;
    QueueHandle_t queue[AXIS_COUNT];
    Correlation corr;
} BufferPipelineKernel;

/**
 * @brief Vyrovnávacie pamäte samostatné pre každú os akcelerácie
 */
typedef struct {
    float *stream;
    float *tmp_conv;
    float *spectrum;
    bool *peaks;
    SpectrumEvent *events;
} BufferPipelineAxis;

/**
 * @brief Fronta pre záznam vzoriek bez ďalšiho spracovania
 */
typedef struct {
    uint16_t max_send_samples;
    QueueHandle_t raw_stream;
} Sender;


/**
 * @brief Druhá mocnina čísla
 */
#define square(x)   ((x) * (x))
/**
 * @brief Minimum z dvoch hodnôt
 */
#define min(x, y)   (((x) < (y)) ? (x): (y))
/**
 * @brief Maximum z dvoch hodnôt
 */
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

/**
 * @brief Alokácia a inicializácia dynamických vyrovnávacích pamätí a synchronizačných primitív
 *  
 * @param[out]  p       Dynamické vyrovnávacie pamäte s dĺžkami podľa nastavení
 * @param[in]   conf     Konfigurácia systému
 */
void process_allocate(BufferPipelineKernel *p, const Configuration *conf);

/**
 * @brief Alokácia dynamických vyrovnávacích pamätí pre jednu os akcelerácie
 *  
 * @param[out]  p       Dynamické vyrovnávacie pamäte s dĺžkami podľa nastavení
 * @param[in]   conf     Konfigurácia systému
 */
void axis_allocate(BufferPipelineAxis *p, const Configuration *conf);

/**
 * @brief Vytvorenie fronty na odosielanie nameraných vzoriek odlišnou systémovou úlohou
 *  
 * @param[out]  sender   Vyhradená fronta s požadovanou maximálnou kapacitou
 * @param[in]   length    Dĺžka fronty
 */
void sender_allocate(Sender *sender, uint16_t length);

/**
 * @brief Uvoľnenie pamäte pre dynamické vyrovnávacie pamäte
 *  
 * @param[out]  p       Dynamické vyrovnávacie pamäte
 */
void process_release(BufferPipelineKernel *p);

/**
 * @brief Uvoľnenie pamäte pre dynamické vyrovnávacie pamäte pre jednu os akcelerácie
 *  
 * @param[out]  p       Dynamické vyrovnávacie pamäte
 */
void axis_release(BufferPipelineAxis *p);
/**
 * @brief Odstránenie fronty na odosielanie nameraných vzoriek
 *  
 * @param  sender   Fronta s vyhradenou kapacitou
 */
void sender_release(Sender *sender);
/** @} */


/** 
 * @defgroup stages Fázy spracovania oknovaného signálu
 * @{
 */

/**
 * @brief Posun vzoriek vo vyrovnávacej pamäti doľava, čím sa dosahuje prekryv okien
 * 
 * Nadbytočné hodnoty od začiatku poľa budú nadhradené vzorkami o `k` pozícii vpravo.  
 *  
 * @param  buffer   Vyrovnávacia pamäť, ktorej obsah bude posunutý
 * @param  n        Dĺžka vyrovnávacej pamäte
 * @param  k        Počet pozícii o koľko sa majú posunúť hodnoty.
 */
void buffer_shift_left(float *buffer, uint16_t n, uint16_t k);

/**
 * @brief Požadované štatistiky podľa nastavení.
 *  
 * @param[in]  buffer       Posuvné okno vzoriek signálu
 * @param[in]  n            Dĺžka posuvného okna
 * @param[out] stats        Deskriptívne štatistiky zo vzoriek posuvného okna. Korektné hodnoty majú len tie povolené v nastaveniach `c`
 * @param[in]  c            Povolenia pre zber vybraných štatistík
 */
void process_statistics(const float *buffer, uint16_t n, Statistics *stats, const StatisticsConfig *c);

/**
 * @brief Korelácia medzi osami akcelerácie: XY, XZ, YZ. Dochádza k bariérovej synchronizácii! 
 * 
 * Úloha pre každú os zrýchlenia si nezávisle prepočíta rozdiely vzoriek od priemeru a smerodajné odchýlky.
 * Následne dochádza k bariérovej synchronizácii aktívnych osí. Každá úloha si dopočíta všetky korelácie
 * samostatne. 
 * 
 * @param[in]  axis         Os akcelerácie: 0, 1, 2
 * @param[in]  buffer       Posuvné okno vzoriek signálu          
 * @param[out] stats        Zistené medziosové korelácie
 * @param[out] corr         Pomocné polia pre výmenu predspracovaných údajov medz úlohami (osami)
 * @param[in]  c            Nastavenia vzorkovania. Využíva sa dĺžka posuvného okna a povolené osi.
 */
void process_correlation(uint8_t axis, const float *buffer, Statistics *stats, Correlation *corr, const SamplingConfig *c);

/**
 * @brief Frekvenčné spektrum (FFT, FCT) posuvného okna vzoriek vynásobené váhami oknovej funkcie
 * 
 * @param[out]  spectrum   Frekvenčné spektrum s dĺžkou `n / 2`  
 * @param[in]   buffer     Posuvné okno vzoriek signálu s dĺžkou `n`
 * @param[in]   window     Váhy oknovej funkcie  s dĺžkou `n`
 * @param[in]   n          Dĺžka posuvného okna
 * @param[in]   c          Nastavenia frekvenčnej transformácie
 */
int process_spectrum(float *spectrum, const float *buffer, const float *window, uint16_t n, const FFTTransformConfig *c);

/**
 * @brief Vyhladzovanie signálu
 * 
 * @param       buffer     Posuvné okno vzoriek signálu s dĺžkou `n`, ktoré bude vyhladené
 * @param       tmp        Pomocné pole o dĺžke `n + c.n - 1`
 * @param[in]   window     Váhy oknovej funkcie  s dĺžkou `n`
 * @param[in]   n          Dĺžka posuvného okna
 * @param[in]   kernel     Konvolučná maska vyhladzovania
 * @param[in]   c          Nastavenia vyhladzovanie
 */
void process_smoothing(float *buffer, float *tmp, uint16_t n, const float *kernel, const SmoothingConfig *c);

/**
 * @brief Hľadanie špičiek vo frekvenčnom spektre podľa nastavení aktívneho algoritmu
 * 
 * @param[out]  peaks      Váhy oknovej funkcie  s dĺžkou `bins`
 * @param[in]   spectrum   Frekvenčné spektrum s dĺžkou `bins`  
 * @param[in]   bins       Počet frekvenčných vedierok
 * @param[in]   c          Nastavenia spracovania udalostí
 */
void process_peak_finding(bool *peaks, const float *spectrum, uint16_t bins, const EventDetectionConfig *c);
/** @} */

// serialize.c
/** 
 * @defgroup stages Serializácia a parsovanie nameraných dát a konfigurácie
 * @{
 */

/**
 * @brief Serializácia prúdu vzoriek v posuvnom okne do formátu Message Pack
 * 
 * @param[out]  msg         Serializované vzorky signálu
 * @param[in]   size        Vyhradená veľkosť pre správu do `msg`
 * @param[in]   stream      Vzorky signálu
 * @param[in]   n           Počet vzoriek signálu
 * 
 * @return Dĺžka serializovanej správy
 */
size_t stream_serialize(char *msg, size_t size, const float *stream, size_t n);

/**
 * @brief Serializácia štatistík signálu v posuvnom okne do formátu Message Pack
 * 
 * @param[in]   timestamp   Poradové číslo posuvného okna
 * @param[out]  msg         Serializované štatistiky
 * @param[in]   size        Vyhradená veľkosť pre správu do `msg`
 * @param[in]   stats       Štatistiky signálu
 * @param[in]   n           Nastavenia zberu štatistík. Do serializovanej správy sa zahrnú len aktívne štatistiky.
 * 
 * @return Dĺžka serializovanej správy
 */
size_t stats_serialize(size_t timestamp, char *msg, size_t size, const Statistics *stats, const StatisticsConfig *c);

/**
 * @brief Serializácia frekvenčného spektra posuvného okna do formátu Message Pack
 * 
 * @param[in]   timestamp   Poradové číslo posuvného okna
 * @param[out]  msg         Serializované frekvenčné spektrum
 * @param[in]   size        Vyhradená veľkosť pre správu do `msg`
 * @param[in]   spectrum    Frekvenčné spektrum
 * @param[in]   n           Počet frekvenčných vedierok
 * @param[in]   fs          Vzorkovacia frekvencia v Hz
 * 
 * @return Dĺžka serializovanej správy
 */
size_t spectra_serialize(size_t timestamp, char *msg, size_t size, const float *spectrum, size_t n, uint16_t fs);

/**
 * @brief Serializácia udalostí zmien frekvenčného spektra do formátu Message Pack
 * 
 * @param[in]   timestamp   Poradové číslo posuvného okna
 * @param[in]   bin_width   Veľkosť frekvenčného vedierka v Hz: vzorkovacia frekvencia / n
 * @param[out]  msg         Serializované udalosti
 * @param[in]   size        Vyhradená veľkosť pre správu do `msg`
 * @param[in]   events      Udalosti frekvenčného spektra s dĺžkou `n`. Do správy budú pridané iba začiatočné a ukončujúce udalosti.
 * @param[in]   n           Počet frekvenčných vedierok
 * 
 * @return Dĺžka serializovanej správy
 */
size_t events_serialize(size_t timestamp, float bin_width, char *msg, size_t size, const SpectrumEvent *events, size_t n);

/**
 * @brief Serializácia systémovej konfigurácie do formátu Message Pack 
 * 
 * @param[out]  msg         Serializovaná konfigurácia
 * @param[in]   size        Vyhradená veľkosť pre správu do `msg`
 * @param[in]   config      Systémová konfigurácia
 * 
 * @return Dĺžka serializovanej správy
 */
size_t config_serialize(char *msg, size_t size, const Configuration *config);

/**
 * @brief Parsovanie systémovej konfigurácie z formátu Message Pack
 * 
 * @param[in]   msg       Serializované konfigurácia
 * @param[in]   size      Dĺžka konfigurácie v Message Pack
 * @param[out]  conf      Systémová konfigurácia
 * @param[out]  error     Chyba pri parsovaní
 * 
 * @return  Zmena konfigurácie oproti pôvodnému obsahu `conf`
 */
bool config_parse(const char *msg, int size, Configuration *conf, bool *error);

/**
 * @brief Serializácia údajov o sieťovom pripojení
 * 
 * @param[out]  msg         Serializované nastavenia pripojenia
 * @param[in]   size        Vyhradená veľkosť pre správu do `msg`
 * @param[in]   config      Nastavenia pripojenia
 * 
 * @return Dĺžka serializovanej správy
 */
size_t login_serialize(char *msg, size_t size, const Provisioning *conf);

/**
 * @brief Parsovanie nastavení sieťového pripojenia z formátu Message Pack
 * 
 * @param[in]   msg       Serializované konfigurácia
 * @param[in]   size      Vyhradená veľkosť pre správu do `msg`
 * @param[out]  conf      Nastavenia pripojenia
 * 
 * @return  Chyba pri parsovaní
 */
bool login_parse(const char *msg, size_t size, Provisioning *conf);
/** @} */

/** @} */

#endif
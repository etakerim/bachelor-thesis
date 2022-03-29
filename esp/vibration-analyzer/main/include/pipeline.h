#ifndef PIPELINE_H
#define PIPELINE_H

#include "esp_dsp.h"
#include "inertial_unit.h"
#include "freertos/queue.h"
#include "freertos/message_buffer.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "mpack.h"

#define AXIS_COUNT                  3

#define MAX_MPACK_FIELDS_COUNT      20
#define SAMPLES_QUEUE_SLOTS         3

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


typedef enum {
    SPECTRUM_EVENT_NONE,
    SPECTRUM_EVENT_START,
    SPECTRUM_EVENT_FINISH
} SpectrumEventAction;

typedef struct {
    SpectrumEventAction action;
    uint32_t start;
    uint32_t duration;
    int32_t last_seen;
    float amplitude;
} SpectrumEvent;


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


#define square(x)   ((x) * (x))
#define min(x, y)   (((x) < (y)) ? (x): (y))
#define max(x, y)   (((x) > (y)) ? (x): (y))

// window.c
void mean_kernel(float *w, int n);
void boxcar_window(float *w, int n);
void bartlett_window(float *w, int n);
void hann_window(float *w, int n);
void hamming_window(float *w, int n);
void blackman_window(float *w, int n);
void window(WindowTypeConfig type, float *w, int n);

// find_peaks.c
void find_peaks_above_threshold(bool *peaks, const float *y, int n, float t);
void find_peaks_neighbours(bool *peaks, const float *y, int n, int k, float e, float h_rel, float h);
void find_peaks_zero_crossing(bool *peaks, const float *y, int n, int k, float slope);
void find_peaks_hill_walker(bool *peaks, const float *y, int n, float tolerance,
                            int hole, float prominence, float isolation);
void event_init(SpectrumEvent *events, uint16_t bins);
size_t event_detection(size_t t, SpectrumEvent *events, const bool *peaks, const float *spectrum,
                       uint16_t bins, uint16_t min_duration, uint16_t time_proximity);

// statistics.c
float minimum(const float *x, int n);
float maximum(const float *x, int n);
float peek_to_peek(float minimum, float maximum);
float root_mean_square(const float *x, int n);
float mean(const float *x, int n);
float variance(const float *x, int n, float mean);
float standard_deviation(float variance);
float moment(const float *x, int n, int m, float mean);
float skewness(const float *x, int n, float mean);
float kurtosis(const float *x, int n, float mean);
float correlation(const float *x_diff, const float *y_diff, int n, float x_std, float y_std);

float quickselect(float *x, int n, int k);
float median(float *x, int n);
float median_abs_deviation(float *x, int n, float med);
float average_abs_deviation(float *x, int n, float mean);

// pipeline.c
void process_allocate(BufferPipelineKernel *p, Configuration *conf);
void axis_allocate(BufferPipelineAxis *p, Configuration *conf);
void sender_allocate(Sender *sender, uint16_t length);
void sender_release(Sender *sender);
void process_release(BufferPipelineKernel *p);
void axis_release(BufferPipelineAxis *p);

void buffer_shift_left(float *buffer, uint16_t n, uint16_t k);
void process_statistics(float *buffer, uint16_t n, Statistics *stats, const StatisticsConfig *c);
void process_correlation(uint8_t axis, const float *buffer, Statistics *stats, Correlation *corr, SamplingConfig *conf);
int process_spectrum(float *spectrum, const float *buffer, const float *window, uint16_t n, const FFTTransformConfig *c);
void process_smoothing(float *buffer, float *tmp, uint16_t n, float *kernel, const SmoothingConfig *c);
void process_peak_finding(bool *peaks, const float *spectrum, uint16_t bins, const EventDetectionConfig *c);

// serialize.c
size_t stream_serialize(char *msg, size_t size, float *stream, size_t n);
size_t stats_serialize(size_t timestamp, char *msg, size_t size, const Statistics *stats, const StatisticsConfig *c);
size_t spectra_serialize(size_t timestamp, char *msg, size_t size, float *spectrum, size_t n, uint16_t fs);
size_t events_serialize(size_t timestamp, float bin_width, char *msg, size_t size, SpectrumEvent *events, size_t n);

size_t config_serialize(char *msg, size_t size, const Configuration *config);
bool config_parse(char *msg, int size, Configuration *conf, bool *change);

#endif
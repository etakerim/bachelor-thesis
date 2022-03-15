#ifndef PIPELINE_H
#define PIPELINE_H

#include "esp_dsp.h"
#include "inertial_unit.h"
#include "freertos/queue.h"

#define AXIS_COUNT  3

typedef enum {
    BOXCAR_WINDOW,
    BARTLETT_WINDOW,
    HANN_WINDOW,
    HAMMING_WINDOW,
    BLACKMAN_WINDOW
} WindowTypeConfig;

typedef enum {
    THRESHOLD,
    NEIGHBOURS,
    ZERO_CROSSING,
    HILL_WALKER
} PeakFindingStrategy;

typedef enum {
    DFT, DCT
} FrequencyTransform;

typedef struct {
    uint16_t frequency;
    AccelerationRange range;
    uint16_t n;
    float overlap;
} SamplingConfig;

typedef struct {
    bool enable;
    uint16_t n;
    uint8_t repeat;
} SmoothingConfig;

typedef struct {
    int min: 1;
    int max: 1;
    int rms: 1;
    int mean: 1;
    int variance: 1;
    int std: 1;
    int skewness: 1;
    int kurtosis: 1;
    int median: 1;
    int mad: 1;
    int corr_xy: 1;
    int corr_yz: 1;
    int corr_xz: 1;
} StatisticsConfig;

typedef struct {
    WindowTypeConfig window;
    FrequencyTransform func;
    bool log;
} FFTTransformConfig;

typedef struct {
    bool enable;
    uint8_t history;
} WelchAverageConfig;

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
    bool enable;
    uint16_t decimation;
    bool samples;
    bool stats;
    bool events;
    bool spectra;
} SaveFormatConfig;


typedef struct {
    SamplingConfig sensor;
    SmoothingConfig tsmooth;
    StatisticsConfig stats;
    FFTTransformConfig transform;
    WelchAverageConfig welch;
    SmoothingConfig fsmooth;
    EventDetectionConfig peak;
    SaveFormatConfig logger;
    SaveFormatConfig mqtt;
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
    uint32_t last_seen;
    float frequency;
    float tolerance;
    float amplitude;
} SpectrumEvent;


typedef struct {
    float *t_smooth;
    float *f_smooth;
    float *window;
} BufferPipelineKernel;

typedef struct {
    float *stream;
    float *tmp_conv;
    float *spectrum;
    bool *peaks;
    SpectrumEvent *events;
} BufferPipelineAxis;

typedef struct {
    QueueHandle_t queue[AXIS_COUNT];
    BufferPipelineKernel kernel;
    BufferPipelineAxis axis[AXIS_COUNT];
} BufferPipeline;


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

// find_peaks.c
void find_peaks_above_threshold(bool *peaks, float *y, int n, float t);
void find_peaks_neighbours(bool *peaks, float *y, int n, int k, float e, float h_rel, float h);
void find_peaks_zero_crossing(bool *peaks, float *y, int n, int k, float slope);
void find_peaks_hill_walker(bool *peaks, float *y, int n, float tolerance, int hole, float prominence, float isolation);

void event_init(SpectrumEvent *events, uint16_t bins, uint16_t fs);
void event_detection(SpectrumEvent *events, bool *peaks, float *spectrum, uint16_t bins, uint16_t min_duration, uint16_t time_proximity);

// statistics.c
float minimum(float *x, int n);
float maximum(float *x, int n);
float peek_to_peek(float minimum, float maximum);
float root_mean_square(float *x, int n);
float mean(float *x, int n);
float variance(float *x, int n, float mean);
float standard_deviation(float variance);
float moment(float *x, int n, int m, float mean);
float skewness(float *x, int n, float mean);
float kurtosis(float *x, int n, float mean);
float correlation(float *x, float *y, int n,
                  float x_avg, float y_avg, float x_std, float y_std);
void welford_algorithm(float x, float *average, float *variance, int k);

float quickselect(float *x, int n, int k);
float median(float *x, int n);
float median_abs_deviation(float *x, int n, float med);
float average_abs_deviation(float *x, int n, float mean);

// pipeline.c
void buffer_shift_left(float *buffer, uint16_t n, uint16_t k);
void process_allocate(BufferPipeline *p, Configuration *conf);
void process_release(BufferPipeline *p);

void process_statistics(float *buffer, uint16_t n, Statistics *stats, const StatisticsConfig *c);
void process_spectrum(float *spectrum, float *buffer, float *window, uint16_t n, const FFTTransformConfig *c);
void process_smoothing(float *buffer, float *tmp, uint16_t n, float *kernel, const SmoothingConfig *c);
void process_peak_finding(bool *peaks, float *spectrum, uint16_t bins, const EventDetectionConfig *c);


#endif
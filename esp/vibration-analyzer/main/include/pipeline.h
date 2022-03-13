#ifndef PIPELINE_H
#define PIPELINE_H

#include "inertial_unit.h"


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
    uint16_t n;
    AccelerationRange range;
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
    float overlap;
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


#endif
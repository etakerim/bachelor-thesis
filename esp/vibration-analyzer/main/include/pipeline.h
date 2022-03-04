#ifndef PIPELINE_H
#define PIPELINE_H


// window.c
void bartlett_window(float *w, int n);
void hann_window(float *w, int n);
void hamming_window(float *w, int n);
void blackman_window(float *w, int n);

// find_peaks.c
int find_peaks_above_threshold(float *peaks, float *y, int w, float t);
int find_peaks_neighbours(float *peaks, float *y, int w, int k, float e, float h_rel, float h);
int find_peaks_zero_crossing(float *peaks, float *y, int w, int k, float slope);
int find_peaks_hill_walker(float *peaks, float *y, int w, float tolerance, int hole, float prominence, float isolation);

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
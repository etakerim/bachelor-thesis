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


float minimum(float *array, int length);
float maximum(float *array, int length);
float peek_to_peek(float minimum, float maximum);
float root_mean_square(float *array, int length);
float sample_average(float *array, int length);
float sample_variance(float *array, int length, float average);
float average_absolute_deviate(float *array, int length, float average);
float standard_deviation(float variance);
float quickselect_median(float *array, int length);
float quickselect(float *array, int length, int k);
float median_absolute_deviate(float *array, int length, float median);
float skewness(float *array, int length, float average, float variance);
float kurtosis(float *array, int length, float average, float variance);
float correlation(float *x, float *y, int length, 
                  float x_avg, float y_avg, float x_std, float y_std);
void welford_algorithm(float x, float *average, float *variance, int k);

#endif
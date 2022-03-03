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


#endif PIPELINE_H
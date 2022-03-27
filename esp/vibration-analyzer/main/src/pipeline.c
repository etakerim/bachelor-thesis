#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "pipeline.h"


void buffer_shift_left(float *buffer, uint16_t n, uint16_t k)
{
    for (uint16_t i = k; i < n; i++)
        buffer[i - k] = buffer[i];
}


void process_statistics(float *buffer, uint16_t n, Statistics *stats, const StatisticsConfig *c)
{
    memset(stats, 0, sizeof(*stats));

    if (c->min)
        stats->min = minimum(buffer, n);
    if (c->max)
        stats->max = maximum(buffer, n);
    if (c->rms)
        stats->rms = root_mean_square(buffer, n);
    if (c->mean || c->variance || c->variance || c->std || c->skewness || c->kurtosis)
        stats->mean = mean(buffer, n);
    if (c->variance || c->std)
        stats->var = variance(buffer, n, stats->mean);
    if (c->std)
        stats->std = standard_deviation(stats->var);
    if (c->skewness)
        stats->skew = skewness(buffer, n, stats->mean);
    if (c->kurtosis)
        stats->kurtosis = kurtosis(buffer, n, stats->mean);
    if (c->median || c->mad)
        stats->median = median(buffer, n);
    if (c->mad)
        stats->mad = median_abs_deviation(buffer, n, stats->median);
}

void process_correlation(uint8_t axis, float *buffer, Statistics *stats, Correlation *corr, SamplingConfig *conf)
{
    xEventGroupSync(corr->barrier, (1 << axis), corr->task_mask, portMAX_DELAY);

    float avg = mean(buffer, conf->n);
    corr->std[axis] = sqrt(variance(buffer, conf->n, avg));
    for (uint16_t i = 0; i < conf->n; i++)
        corr->diff[axis][i] = (buffer[i] - avg);

    xEventGroupSync(corr->barrier, (1 << axis), corr->task_mask, portMAX_DELAY);

    if (conf->axis[0] && conf->axis[1])
        stats->corr_xy = correlation(
            corr->diff[0], corr->diff[1], conf->n, corr->std[0], corr->std[1]
        );
    if (conf->axis[0] && conf->axis[2])
        stats->corr_xz = correlation(
            corr->diff[0], corr->diff[2], conf->n, corr->std[0], corr->std[2]
        );
    if (conf->axis[1] && conf->axis[2])
        stats->corr_yz = correlation(
            corr->diff[1], corr->diff[2], conf->n, corr->std[1], corr->std[2]
        );
}

int process_spectrum(float *spectrum, float *buffer, float *window, uint16_t n, const FFTTransformConfig *c)
{
    const uint16_t bins = n / 2;

    switch (c->func) {
        case DFT:
            for (uint16_t i = 0; i < n; i++) {
                spectrum[2*i + 0] = buffer[i] * window[i];
                spectrum[2*i + 1] = 0;
            }
            dsps_fft2r_fc32_ae32(spectrum, n);
            dsps_bit_rev2r_fc32(spectrum, n);
            dsps_cplx2reC_fc32(spectrum, n);
            break;

        case DCT:
            for (uint16_t i = 0; i < n; i++) {
                spectrum[i] = buffer[i] * window[i];
            }
            dsps_dct_f32(spectrum, n);
            break;
        default:
            break;
    }

    if (c->log) {
        for (uint16_t i = 0; i < bins; i++)
            spectrum[i] = square(spectrum[i*2]) + square(spectrum[i*2+1]);
        float ref = maximum(spectrum, bins);
        for (uint16_t i = 0; i < bins; i++)
            spectrum[i] = 10 * log10f(spectrum[i] / ref);
    }
    return bins;
}

void process_smoothing(float *buffer, float *tmp, uint16_t n, float *kernel, const SmoothingConfig *c)
{
    if (!c->enable)
        return;
    
    for (uint8_t k = 0; k < c->repeat; k++) {
        dsps_conv_f32_ae32(buffer, n, kernel, c->n, tmp);
        memcpy(buffer, tmp, sizeof(*buffer) * n);
    }
}

void process_peak_finding(bool *peaks, float *spectrum, uint16_t bins, const EventDetectionConfig *c)
{
    switch (c->strategy) {
        case THRESHOLD:
            find_peaks_above_threshold(peaks, spectrum, bins, c->threshold.t);
            break;
        case NEIGHBOURS:
            find_peaks_neighbours(
                peaks, spectrum, bins, 
                c->neighbours.k, c->neighbours.e,
                c->neighbours.h_rel, c->neighbours.h
            );
            break;
        case ZERO_CROSSING:
            find_peaks_zero_crossing(
                peaks, spectrum, bins,
                c->zero_crossing.k, c->zero_crossing.slope
            );
            break;
        case HILL_WALKER:
            find_peaks_hill_walker(
                peaks, spectrum, bins,
                c->hill_walker.tolerance, c->hill_walker.hole,
                c->hill_walker.prominence, c->hill_walker.isolation
            );
            break;
        default:
            break;
    }
}
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

void process_allocate(BufferPipeline *p, Configuration *conf)
{
    const uint16_t bins = conf->sensor.n / 2;
    const uint16_t n = conf->sensor.n;
    const uint16_t t_smooth = conf->tsmooth.n;
    const uint16_t f_smooth = conf->fsmooth.n;
    const WindowTypeConfig w = conf->transform.window;
    const uint16_t freq = conf->sensor.frequency;
    const uint16_t n_smooth = max(t_smooth, f_smooth);

    p->kernel.t_smooth = malloc(t_smooth * sizeof(*p->kernel.t_smooth));
    p->kernel.f_smooth = malloc(f_smooth * sizeof(*p->kernel.f_smooth));
    mean_kernel(p->kernel.t_smooth, t_smooth);
    mean_kernel(p->kernel.f_smooth, f_smooth);
    
    p->kernel.window = malloc(n * sizeof(*p->kernel.window));
    switch (w) {
        case BOXCAR_WINDOW:
            boxcar_window(p->kernel.window, n); break;
        case BARTLETT_WINDOW:
            bartlett_window(p->kernel.window, n); break;
        case HANN_WINDOW:
            hann_window(p->kernel.window, n); break;
        case HAMMING_WINDOW:
            hamming_window(p->kernel.window, n); break;
        case BLACKMAN_WINDOW:
            blackman_window(p->kernel.window, n); break;
    }

    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        p->queue[i] = xQueueCreate(3, sizeof(float) * n);

        p->axis[i].stream = malloc(n * sizeof(*p->axis[i].stream));
        p->axis[i].tmp_conv = malloc((n + n_smooth - 1) * sizeof(*p->axis[i].tmp_conv));
        p->axis[i].spectrum = malloc(2 * n * sizeof(*p->axis[i].spectrum));
        p->axis[i].peaks = malloc(bins * sizeof(*p->axis[i].peaks));
        p->axis[i].events = malloc(bins * sizeof(*p->axis[i].events));
        event_init(p->axis[i].events, bins, freq);
        p->axis[i].serialize = malloc(SERIALIZE_BUFFER_LENGTH * sizeof(*p->axis[i].serialize));

    }
}

void process_release(BufferPipeline *p)
{
    free(p->kernel.t_smooth);
    free(p->kernel.f_smooth);
    free(p->kernel.window);

    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        vQueueDelete( p->queue[i]);

        free(p->axis[i].stream);
        free(p->axis[i].tmp_conv);
        free(p->axis[i].spectrum);
        free(p->axis[i].peaks);
        free(p->axis[i].events);
        free(p->axis[i].serialize);
    }
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
    }
}
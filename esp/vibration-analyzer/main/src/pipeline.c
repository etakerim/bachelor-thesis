#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "pipeline.h"
#include "statistics.h"


void buffer_shift_left(float *buffer, uint16_t n, uint16_t k)
{
    for (uint16_t i = k; i < n; i++)
        buffer[i - k] = buffer[i];
}

void process_statistics(const float *buffer, uint16_t n, Statistics *stats, const StatisticsConfig *c)
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

void process_correlation(uint8_t axis, const float *buffer, Statistics *stats, Correlation *corr, const SamplingConfig *c)
{
    xEventGroupSync(corr->barrier, (1 << axis), corr->task_mask, portMAX_DELAY);

    float avg = mean(buffer, c->n);
    corr->std[axis] = sqrt(variance(buffer, c->n, avg));
    for (uint16_t i = 0; i < c->n; i++)
        corr->diff[axis][i] = (buffer[i] - avg);

    xEventGroupSync(corr->barrier, (1 << axis), corr->task_mask, portMAX_DELAY);

    if (c->axis[0] && c->axis[1])
        stats->corr_xy = correlation(
            corr->diff[0], corr->diff[1], c->n, corr->std[0], corr->std[1]
        );
    if (c->axis[0] && c->axis[2])
        stats->corr_xz = correlation(
            corr->diff[0], corr->diff[2], c->n, corr->std[0], corr->std[2]
        );
    if (c->axis[1] && c->axis[2])
        stats->corr_yz = correlation(
            corr->diff[1], corr->diff[2], c->n, corr->std[1], corr->std[2]
        );
}

int process_spectrum(float *spectrum, const float *buffer, const float *window, uint16_t n, const FFTTransformConfig *c)
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
            for (uint16_t i = 0; i < bins; i++)
                spectrum[i] = dsps_sqrtf_f32_ansi(
                    square(spectrum[i*2]) + square(spectrum[i*2+1])
                );
            break;

        case DCT:
            for (uint16_t i = 0; i < n; i++)
                spectrum[i] = buffer[i] * window[i];
            dsps_dct_f32(spectrum, n);
            for (uint16_t i = 0; i < bins; i++)
                spectrum[i] *= 2;
            break;
        default:
            break;
    }

    if (c->log) {
        float ref = maximum(spectrum, bins);
        for (uint16_t i = 0; i < bins; i++)
            spectrum[i] = 20 * log10f(spectrum[i] / ref);
    }

    return bins;
}

void process_smoothing(float *buffer, float *tmp, uint16_t n, const float *kernel, const SmoothingConfig *c)
{
    if (!c->enable)
        return;
    
    for (uint8_t k = 0; k < c->repeat; k++) {
        dsps_conv_f32_ae32(buffer, n, kernel, c->n, tmp);
        memcpy(buffer, tmp, sizeof(*buffer) * n);
    }
}

void process_peak_finding(bool *peaks, const float *spectrum, uint16_t bins, const EventDetectionConfig *c)
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


void process_allocate(BufferPipelineKernel *p, const Configuration *conf)
{
    const uint16_t n = conf->sensor.n;
    const uint16_t t_smooth = conf->tsmooth.n;
    const uint16_t f_smooth = conf->fsmooth.n;
    const uint16_t n_smooth = max(t_smooth, f_smooth);
    const WindowTypeConfig w = conf->transform.window;

    p->smooth = malloc(n_smooth * sizeof(*p->smooth));
    mean_kernel(p->smooth, n_smooth);

    p->window = malloc(n * sizeof(*p->window));
    window(w, p->window, n);

    for (uint8_t i = 0; i < AXIS_COUNT; i++)
        p->queue[i] = xQueueCreate(conf->sensor.frequency, sizeof(float));

    p->corr.barrier = xEventGroupCreate();
    p->corr.task_mask = 0;
    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        p->corr.diff[i] = malloc(conf->sensor.n * sizeof(float));
        if (conf->sensor.axis[i]) 
            p->corr.task_mask |= (1 << i);
    }
}

void axis_allocate(BufferPipelineAxis *p, const Configuration *conf)
{
    const uint16_t n = conf->sensor.n;
    const uint16_t bins = n / 2;
    const uint16_t t_smooth = conf->tsmooth.n;
    const uint16_t f_smooth = conf->fsmooth.n;
    const uint16_t n_smooth = max(t_smooth, f_smooth);

    p->stream = malloc(n * sizeof(*p->stream));
    p->tmp_conv = malloc((n + n_smooth - 1) * sizeof(*p->tmp_conv));
    p->spectrum = malloc(2 * n * sizeof(*p->spectrum));
    p->peaks = malloc(bins * sizeof(*p->peaks));
    p->events = malloc(bins * sizeof(*p->events));
    event_init(p->events, bins);
}

void sender_allocate(Sender *sender, uint16_t length)
{
    sender->max_send_samples = (length / AXIS_COUNT + 1) * AXIS_COUNT;
    sender->raw_stream = xQueueCreate(
        SAMPLES_QUEUE_SLOTS * sender->max_send_samples, sizeof(float)
    );
}

void sender_release(Sender *sender)
{
    vQueueDelete(sender->raw_stream);
}

void process_release(BufferPipelineKernel *p)
{
    free(p->smooth);
    free(p->window);
    for (uint8_t i = 0; i < AXIS_COUNT; i++) {
        vQueueDelete(p->queue[i]);
        free(p->corr.diff[i]);
    }
}

void axis_release(BufferPipelineAxis *p)
{
    free(p->stream);
    free(p->tmp_conv);
    free(p->spectrum);
    free(p->peaks);
    free(p->events);
}

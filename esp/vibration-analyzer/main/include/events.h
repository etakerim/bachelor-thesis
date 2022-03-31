#ifndef EVENTS_H
#define EVENTS_H

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

#define min(x, y)   (((x) < (y)) ? (x): (y))
#define max(x, y)   (((x) > (y)) ? (x): (y))


void find_peaks_above_threshold(bool *peaks, const float *y, int n, float t);
void find_peaks_neighbours(bool *peaks, const float *y, int n, int k, float e, float h_rel, float h);
void find_peaks_zero_crossing(bool *peaks, const float *y, int n, int k, float slope);
void find_peaks_hill_walker(bool *peaks, const float *y, int n, float tolerance,
                            int hole, float prominence, float isolation);
void event_init(SpectrumEvent *events, uint16_t bins);
size_t event_detection(size_t t, SpectrumEvent *events, const bool *peaks, const float *spectrum,
                       uint16_t bins, uint16_t min_duration, uint16_t time_proximity);

#endif
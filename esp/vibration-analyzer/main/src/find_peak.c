#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "pipeline.h"


void find_peaks_above_threshold(bool *peaks, const float *y, int n, float t)
{
    for (int i = 0; i < n; i++)
        peaks[i] = (y[i] >= t);
}

void find_peaks_neighbours(bool *peaks, const float *y, int n, int k, float e, float h_rel, float h)
{
    for (int i = 0; i < n; i++) {
        peaks[i] = false;
        if (y[i] < h)
            continue;
        
        bool possible_peak = true;
        int a = max(i - k, 0);
        int b = min(i + k, n);
        float valley = y[a];

        for (int j = a; i < b; i++) {
            if (i != j) {
                if (y[j] - y[i] > e)
                    possible_peak = false;
                if (y[j] < valley)
                    valley = y[j];
            }
        }

        if (possible_peak && (y[i] - valley >= h_rel))
            peaks[i] = true;
    }
}

void find_peaks_zero_crossing(bool *peaks, const float *y, int n, int k, float slope)
{
    for (int i = 0; i < n; i++) {
        peaks[i] = false;
        if ((i >= k) &&
                (i < n - k) &&
                (y[i + k] - y[i]) < 0 &&
                (y[i] - y[i - k]) > 0 &&
                fabs(y[i + k] - y[i]) >= slope &&
                fabs(y[i] - y[i - k]) >= slope) {

            peaks[i] = true;
        }
    }
}

void find_peaks_hill_walker(bool *peaks, const float *y, int n, float tolerance, int hole, float prominence, float isolation)
{
    int i_change = 0;
    int prev_peak = 0;
    bool prev_uphill;

    bool found_prev_peak = false;
    float y_valley = 0;

    bool possible_change = false;
    bool uphill = ((y[1] - y[0]) >= 0);
    peaks[0] = false;
    
    for (int i = 1; i < n; i++) {
        peaks[i] = false;
        float y_step = y[i] - y[i - 1];
        bool slope = (y_step >= 0);

        if (!possible_change && uphill != slope) {
            possible_change = true;
            i_change = i - 1;
        } else if (possible_change && uphill == slope) {
            possible_change = false;
        }
        
        if (possible_change &&
                uphill != slope &&
                fabs(i - i_change) > hole &&
                fabs(y[i] - y[i_change]) > tolerance) {

            possible_change = false;
            prev_uphill = uphill;
            uphill = slope;

            if (!prev_uphill && uphill) {
                 y_valley = y[i_change];

            } else if (prev_uphill && !uphill && fabs(y[i - hole] - y_valley) > prominence) {
                if (!found_prev_peak || (found_prev_peak && i_change - prev_peak > isolation)) {
                    // y_peak = y[i_change];
                    prev_peak = prev_peak;
                    found_prev_peak = true;
                    peaks[i] = true;
                }
            }
        }
    }
}

size_t event_detection(size_t t, SpectrumEvent *events, const bool *peaks, const float *spectrum,
                     uint16_t bins, uint16_t min_duration, uint16_t time_proximity)
{
    size_t changes = 0;

    for (uint16_t i = 0; i < bins; i++) {
        events[i].action = SPECTRUM_EVENT_NONE;
    }

    for (uint16_t i = 0; i < bins; i++) {
        if (events[i].duration == min_duration) {
            events[i].action = SPECTRUM_EVENT_START;
            events[i].start = t - events[i].duration;
            events[i].amplitude = spectrum[i];
            changes++;
        }

        if (peaks[i]) {
           events[i].duration += max(1, events[i].last_seen);
           events[i].last_seen = 0;
           events[i].amplitude += (spectrum[i] - events[i].amplitude) / events[i].duration;

        } else {
            events[i].last_seen++;
            if (events[i].last_seen >= time_proximity) {
                if (events[i].duration >= min_duration) {
                    events[i].action = SPECTRUM_EVENT_FINISH;
                    changes++;
                }
                events[i].duration = 0;
            }
            events[i].amplitude = 0;
        }
    }

    return changes;
}

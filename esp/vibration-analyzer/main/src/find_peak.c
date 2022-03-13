#include <math.h>
#include <stdbool.h>
#include "pipeline.h"


void find_peaks_above_threshold(bool *peaks, float *y, int n, float t)
{
    for (int i = 0; i < n; i++)
        peaks[i] = (fabs(y[i]) >= t);
}

void find_peaks_neighbours(bool *peaks, float *y, int n, int k, float e, float h_rel, float h)
{
    for (int i = 0; i < n; i++) {
        peaks[i] = false;
        if (fabs(y[i]) < h)
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

void find_peaks_zero_crossing(bool *peaks, float *y, int n, int k, float slope)
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

void find_peaks_hill_walker(bool *peaks, float *y, int n, float tolerance, int hole, float prominence, float isolation)
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

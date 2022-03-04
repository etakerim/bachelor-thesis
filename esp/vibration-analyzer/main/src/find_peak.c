#include <math.h>
#include <stdbool.h>

#define max(a, b) (((a) > (b)) ? (a): (b))
#define min(a, b) (((a) < (b)) ? (a): (b))


int find_peaks_above_threshold(float *peaks, float *y, int w, float t)
{
    int n = 0;
    for (int i = 0; i < w; i++) {
        if (fabs(y[i]) >= t) 
            peaks[n++] = i;
    }
    return n;
}

int find_peaks_neighbours(float *peaks, float *y, int w, int k, float e, float h_rel, float h)
{
    int n = 0;

    for (int i = 0; i < w; i++) {
        if (fabs(y[i]) < h)
            continue;
        
        bool possible_peak = true;
        int a = max(i - k, 0);
        int b = min(i + k, w);
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
            peaks[n++] = i;
    }
    return n;
}

int find_peaks_zero_crossing(float *peaks, float *y, int w, int k, float slope)
{
    int n = 0;
    for (int i = k; i < w - k; i++) {
         if ((y[i + k] - y[i]) < 0 && 
                (y[i] - y[i - k]) > 0 &&
                fabs(y[i + k] - y[i]) >= slope && 
                fabs(y[i] - y[i - k]) >= slope) {

            peaks[n++] = i;
        }

    }
    return n;
}

int find_peaks_hill_walker(float *peaks, float *y, int w, float tolerance, int hole, float prominence, float isolation)
{
    int n = 0;

    int i_change = 0;
    float y_valley = 0;
    bool possible_change = false;
    bool uphill = ((y[1] - y[0]) >= 0);
    
    for (int i = 1; i < w; i++) {
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
            bool prev_uphill = uphill;
            uphill = slope;

            if (!prev_uphill && uphill) {
                 y_valley = y[i_change];

            } else if (prev_uphill && !uphill && fabs(y[i - hole] - y_valley) > prominence) {
                if (n < 1 || (n >= 1 && i_change - peaks[n-1] > isolation)) {
                    float y_peak = y[i_change];
                    peaks[n++] = i_change;
                }
            }
        }
    }
    return n;
}



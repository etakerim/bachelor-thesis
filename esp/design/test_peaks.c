#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define min(x, y)   (((x) < (y)) ? (x): (y))
#define max(x, y)   (((x) > (y)) ? (x): (y))

void find_peaks_neighbours(bool *peaks, const float *y, int n, int k, float e, float h_rel, float h);
void find_peaks_zero_crossing(bool *peaks, const float *y, int n, int k, float slope);
void find_peaks_hill_walker(bool *peaks, const float *y, int n, float tolerance, int hole, float prominence, float isolation);

#define N    100

int main(void)
{
    const float y[N] = {
        -35.563, -31.149, -23.685, -16.869, -12.697, -17.164, -27.432, -36.280, -40.846, -43.076, -46.707, -47.520,
        -47.438, -47.076, -40.348, -36.603, -34.746, -34.123, -35.086, -36.950, -39.705, -39.833, -41.615, -43.003,
        -40.193, -40.481, -41.253, -39.947, -35.477, -27.478, -18.519, -17.828, -25.439, -33.641, -40.603, -41.057,
        -37.192, -36.042, -36.334, -37.645, -38.900, -41.637, -41.228, -40.540, -41.223, -38.165, -44.264, -44.124,
        -44.573, -45.627, -40.903, -40.294, -39.263, -41.272, -41.954, -45.377, -45.637, -42.140, -39.352, -36.603,
        -37.256, -39.388, -40.673, -40.549, -39.348, -37.949, -37.846, -36.819, -36.412, -37.267, -36.491, -36.813,
        -37.974, -36.452, -34.897, -33.590, -27.292, -19.040, -11.214, -6.776, -11.575, -20.990, -32.884, -40.532,
        -41.198, -40.559, -37.819, -36.635, -36.702, -36.222, -35.210, -36.704, -35.216, -34.071, -35.446, -35.438,
        -37.028, -38.276, -37.366, -36.207
    };
    bool peaks[N] = {};


    bool peaks_A[N] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int k = 9;
    float e = 0;
    float h_rel = 15;
    float h = -100;
    find_peaks_neighbours(peaks, y, N, k, e, h_rel, h);
    for (int i = 0; i < N; i++) {
        printf("%d %d\n", i, peaks[i]);
        assert(peaks[i] == peaks_A[i]);
    }

    bool peaks_B[N] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    k = 2;
    float slope = 4;
    find_peaks_zero_crossing(peaks, y, N, k, slope);
    for (int i = 0; i < N; i++) assert(peaks[i] == peaks_B[i]);

    bool peaks_C[N] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    float tolerance = 0;
    int hole = 0;
    float prominence = 10;
    float isolation = 5;
    find_peaks_hill_walker(peaks, y, N, tolerance, hole, prominence, isolation);
    for (int i = 0; i < N; i++) assert(peaks[i] == peaks_C[i]);
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

        for (int j = a; j < b; j++) {
            if (i != j) {
                if (y[j] - y[i] > e)
                    possible_peak = false;
                if (y[j] < valley)
                    valley = y[j];
            }
        }

        if (possible_peak && (y[i] - valley > h_rel))
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
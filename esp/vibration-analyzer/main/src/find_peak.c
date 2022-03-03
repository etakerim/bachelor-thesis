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

float minimum(float *array, int length) 
{
    float x = array[0];
    for (int i = 0; i < length; i++) {
        if (array[i] < x)
            x = array[i];
    } 
    return x; 
}

float maximum(float *array, int length) 
{
    float x = array[0];
    for (int i = 0; i < length; i++) {
        if (array[i] > x)
            x = array[i];
    } 
    return x; 
}

float peek_to_peek(float minimum, float maximum)
{
    return maximum - minimum;
}

float root_mean_square(float *array, int length)
{
    float x = 0;
    for (int i = 0; i < length; i++)
        x += array[i] * array[i];
    return sqrt(x / length);
}


float sample_average(float *array, int length)
{
    float x = 0;
    for (int i = 0; i < length; i++)
        x += array[i];
    return x / length;
}

float sample_variance(float *array, int length, float average)
{
    float x = 0;
    float xdiff = 0;

    for (int i = 0; i < length; i++) {
        xdiff = (array[i] - average);
        x += xdiff * xdiff;
    }
    return x / (length - 1);
}

float average_absolute_deviate(float *array, int length, float average)
{
    float x = 0;
    float xdiff = 0;

    for (int i = 0; i < length; i++) {
        x += fabs(array[i] - average);
    }
    return x / length;
}

float standard_deviation(float variance)
{
    return sqrt(variance);
}

static float swap(float *a, float *b)
{
    float tmp = *a;
    *a = *b;
    *b = tmp;
}

float quickselect_median(float *array, int length)
{
    if (length % 2)
        return quickselect(array, length, length / 2);
    else
        return (quickselect(array, length, length / 2 - 1) + 
                quickselect(array, length, length / 2)) / 2;
}

float quickselect(float *array, int length, int k)
{
    float x[length]; // dynamic allocation + median of medians
    int a = 0;
    int b = length;
    int j = a + 1;

    while (k != j) {
        j = a + 1;
        float pivot = x[a];
        for (int i = a+1; i < b; i++) {
            if (x[i] < pivot) {
                swap(&x[i], &x[j]);
                j++;
            }
        }
        swap(&x[0], &x[j]);

        if (k > j) {
            b = j;
        } else {
            a = j;
            b = length - j;
            k -= j;
        }
    }

    return x[j];
}

float median_absolute_deviate(float *array, int length, float median)
{
    float tmp[length]; // dynamic allocation
    for (int i = 0; i < length; i++)
        tmp[i] = fabs(array[i] - median);

    float m = quickselect_median(tmp, length);
    return m;

}

float skewness(float *array, int length, float average, float variance)
{
    float x = 0;
    for (int i = 0; i < length; i++) {
        x += pow((array[i] - average) / variance, 3);
    }
    return x / length;
}

float kurtosis(float *array, int length, float average, float variance)
{
    float x = 0;
    for (int i = 0; i < length; i++) {
        x += pow((array[i] - average) / variance, 4);
    }
    return (x / length) - 3;
}

float correlation(float *x, float *y, int length, 
                  float x_avg, float y_avg, float x_std, float y_std)
{
    float sum = 0;
    for (int i = 0; i < length; i++) {
        sum += (x[i] - x_avg) * (y[i] - y_avg);
    }
    return (sum / length) / (x_std * y_std);
}


// INIT with average = x, variance = 0, k = 1
void welford_algorithm(float x, float *average, float *variance, int k)
{
    float new_avg = *average + ((x - *average) / k);
    float new_var = *variance + ((x + *average) * (x + new_avg));

    *average = new_avg;
    *variance = new_var;   // Podeľ k pre výsledok
}



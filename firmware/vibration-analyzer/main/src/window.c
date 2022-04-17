#include <math.h>
#include "pipeline.h"

typedef void (*window_action_t)(float *, int);


void mean_kernel(float *w, int n)
{
    for (int i = 0; i < n; i++)
        w[i] = 1 / (float)n;
}

void boxcar_window(float *w, int n)
{
    for (int i = 0; i < n; i++)
        w[i] = 1;
}

void bartlett_window(float *w, int n)
{
    float k = 2.0 / (n - 1);
    float mid = (n - 1) / 2.0;
    
    for (int i = 0; i < n; i++)
        w[i] = k * (mid - fabs(i - mid));
} 

void hann_window(float *w, int n)
{
    float k = M_PI / n;
    for (int i = 0; i < n; i++) {
        w[i] = sin(k * i);
        w[i] *= w[i];
    }
}

void hamming_window(float *w, int n)
{
    float k = (2 * M_PI) / n;
    for (int i = 0; i < n; i++)
        w[i] = 0.54 - 0.46 * cos(k * i);
}

void blackman_window(float *w, int n)
{
    float k1 = (2 * M_PI) / n;
    float k2 = (4 * M_PI) / n;
    for (int i = 0; i < n; i++)
        w[i] = 0.42 - 0.5 * cos(k1 * i) + 0.08 * cos(k2 * i);
}

void window(WindowTypeConfig type, float *w, int n)
{
    const window_action_t action[WINDOW_TYPE_COUNT] = {
        boxcar_window, bartlett_window, hann_window,
        hamming_window, blackman_window
    };
    if (type < WINDOW_TYPE_COUNT) 
        action[type](w, n);
}
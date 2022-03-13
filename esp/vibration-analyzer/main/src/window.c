#include <math.h>


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
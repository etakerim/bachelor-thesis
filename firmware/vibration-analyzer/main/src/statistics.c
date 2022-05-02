#include <math.h>
#include <string.h>


float minimum(const float *x, int n) 
{
    float k = x[0];
    for (int i = 0; i < n; i++)
        if (x[i] < k) k = x[i];
    return k; 
}

float maximum(const float *x, int n) 
{
    float k = x[0];
    for (int i = 0; i < n; i++)
        if (x[i] > k) k = x[i];
    return k; 
}

float root_mean_square(const float *x, int n)
{
    float k = 0;
    for (int i = 0; i < n; i++)
        k += x[i] * x[i];
    return sqrt(k / n);
}


float mean(const float *x, int n)
{
    float k = 0;
    for (int i = 0; i < n; i++)
        k += x[i];
    return k / n;
}

float variance(const float *x, int n, float mean)
{
    float k = 0;
    float diff = 0;

    for (int i = 0; i < n; i++) {
        diff = (x[i] - mean);
        k += diff * diff;
    }
    return k / n;
}

float standard_deviation(float variance)
{
    return sqrt(variance);
}

float moment(const float *x, int n, int m, float mean)
{
    float k = 0;
    for (int i = 0; i < n; i++)
        k += pow(x[i] - mean, m);
    return k / n;
}

float skewness(const float *x, int n, float mean)
{
    float m2 = variance(x, n, mean);
    float m3 = moment(x, n, 3, mean);
    return m3 / pow(m2, 1.5); 
}

float kurtosis(const float *x, int n, float mean)
{
    float m2 = variance(x, n, mean);
    float m4 = moment(x, n, 4, mean);
    return (m4 / (m2 * m2)) - 3; 
}

float correlation(const float *x_diff, const float *y_diff, int n, float x_std, float y_std)
{
    float sum = 0;
    for (int i = 0; i < n; i++)
        sum += x_diff[i] * y_diff[i];
    return (sum / n) / (x_std * y_std);
}


static void swap(float *a, float *b)
{
    float tmp = *a;
    *a = *b;
    *b = tmp;
}

static int partition(float *x, int left, int right, int pivot_idx)
{
    float pivot = x[pivot_idx];
    swap(&x[pivot_idx], &x[right]);
    
    int split = left;
    for (int i = left; i < right; i++) {
        if (x[i] < pivot) {
            swap(&x[split], &x[i]);
            split++;
        }
    }
    swap(&x[split], &x[right]);
    return split;
}

float quickselect(const float *x, int n, int k)
{
    int left = 0;
    int right = n-1;
    float xx[n];
    memcpy(xx, x, n * sizeof(*x));

    while (left < right) {        
        int split = partition(xx, left, right, left);
        if (k == split)
            return xx[k];
        else if (k < split)
            right = split - 1;
        else
            left = split + 1;

    }
    return xx[left];
}


float median(const float *x, int n)
{
    if (n % 2)
        return quickselect(x, n, n / 2);
    else
        return (quickselect(x, n, n / 2 - 1) + 
                quickselect(x, n, n / 2)) / 2;
}

float median_abs_deviation(const float *x, int n, float med)
{
    float tmp[n];
    for (int i = 0; i < n; i++)
        tmp[i] = fabs(x[i] - med);

    float m = median(tmp, n);
    return m;
}

float average_abs_deviation(const float *x, int n, float mean)
{
    float k = 0;
    for (int i = 0; i < n; i++)
        k += fabs(x[i] - mean);
    return k / n;
}

/*
// INIT with average = x, variance = 0, k = 1
// Podeľ k pre výsledok variance
void welford_algorithm(float x, float *average, float *variance, int k)
{
    float new_avg = *average + ((x - *average) / k);
    float new_var = *variance + ((x + *average) * (x + new_avg));

    *average = new_avg;
    *variance = new_var;
}


float correlation(float *x, float *y, int n, 
                  float x_avg, float y_avg, float x_std, float y_std)
{
    float sum = 0;
    for (int i = 0; i < n; i++)
        sum += (x[i] - x_avg) * (y[i] - y_avg);
    return (sum / n) / (x_std * y_std);
}

int main(void)
{
    int N = 11;
    float nums[] = {6, 1, 19, -11, 13, 11, 17, 22, -11, -11, 8};

    printf("minimum: %.4f\n", minimum(nums, N));
    printf("maximum: %.4f\n", maximum(nums, N));
    printf("RMS: %.4f\n", root_mean_square(nums, N));

    float avg = mean(nums, N);
    float var = variance(nums, N, avg);
    printf("average: %.7f\n", avg);
    printf("variance: %.7f\n", var);
    printf("std: %.4f\n", standard_deviation(var));
    printf("AAD: %.4f\n", average_abs_deviation(nums, N, avg));

    printf("skewness: %.4f\n", skewness(nums, N, avg));
    printf("kurtosis: %.4f\n", kurtosis(nums, N, avg));

    float m = median(nums, N);
    printf("median: %.4f\n", m);
    printf("MAD: %.4f\n", median_abs_deviation(nums, N, m));
}
*/

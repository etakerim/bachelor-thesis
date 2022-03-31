#include <math.h>
#include <stdbool.h>
/*
 * Compile: gcc -Wall -std=c99 test_peaks.c ../src/events.c -I../include/ -o test
 */
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "events.h"

#define min(x, y)   (((x) < (y)) ? (x): (y))
#define max(x, y)   (((x) > (y)) ? (x): (y))


#define N    32

void evaluate_algorithm(const bool *ref, const bool *real)
{
    for (int i = 0; i < N; i++) {
        if (ref[i] != real[i])
            printf("(%d) má byť %d\n", i, ref[i]);
        assert(ref[i] == real[i]);
    }
    putchar('\n');
}

int main(void)
{
    const float y[N] = {
       -40.87050592638142, -32.53255956831915, -29.508994770203092, -10.691251340381175, -3.4110727158952074, -9.901527076932448,
       -35.79207520016154, -27.410607570106684, -25.96285661807546, -30.001244170003353, -36.81592465375243, -16.825688500452955,
       -8.790832758356737, -14.099424442180286, -23.364682932761077, -24.21486780321459, -8.435759219020643, -13.489832409078378,
       -30.236506623806196, -50.80225702436357, -33.939270043081095, -28.210243782623316, -31.068607397764826, -7.012901924718346,
       0.0, -7.635162340500302, -31.771872929240615, -27.859886972558172, -32.40580843772029, -37.237135014488125, -40.99332469229347,
       -27.37200436353509
    };
    bool peaks[N] = {};

    const bool peaks_A[N] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1
    };
    int k = 4;
    float e = 0;
    float h_rel = 10;
    float h = -100;
    find_peaks_neighbours(peaks, y, N, k, e, h_rel, h);
    puts("Algoritmus A");
    evaluate_algorithm(peaks_A, peaks);

    const bool peaks_B[N] = {
        0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
    };
    k = 2;
    float slope = 7;
    find_peaks_zero_crossing(peaks, y, N, k, slope);
    puts("Algoritmus B");
    evaluate_algorithm(peaks_B, peaks);
    

    const bool peaks_C[N] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
    };
    float tolerance = 1;
    int hole = 0;
    float prominence = 8;
    float isolation = 0;
    find_peaks_hill_walker(peaks, y, N, tolerance, hole, prominence, isolation);
    puts("Algoritmus C");
    evaluate_algorithm(peaks_C, peaks);
   
}
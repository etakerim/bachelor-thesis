/*
 * Compile: gcc -Wall -std=c99 test_events.c ../src/events.c -I../include/ -o test
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "events.h"


/* Iba s minimal duration (počiatočná implementácia) */
size_t event_detection_basic(size_t t, SpectrumEvent *events, const bool *peaks, const float *spectrum,
                       uint16_t bins, uint16_t min_duration)
{
    size_t changes = 0;

    for (uint16_t i = 0; i < bins; i++) {
        // Odstráň predošlú notifikáciu o udalosti vo frekvečnom vedierku
        if (events[i].action == SPECTRUM_EVENT_FINISH) {
            events[i].start = 0;
            events[i].duration = 0;
            events[i].amplitude = 0;
        }
        events[i].action = SPECTRUM_EVENT_NONE;

        if (peaks[i]) {
            events[i].duration++;
            events[i].amplitude += (spectrum[i] - events[i].amplitude) / events[i].duration;

            if (events[i].duration == min_duration) {
                events[i].action = SPECTRUM_EVENT_START;
                events[i].start = t - min_duration + 1;
                changes++;
            }

        } else {
            if (events[i].duration >= min_duration) {
                events[i].action = SPECTRUM_EVENT_FINISH;
                changes++;
            } else {
                events[i].duration = 0;
            }
        }
    }

    return changes;
}

#define WINDOW_COUNT       8
#define BINS               3


const bool peaks[WINDOW_COUNT][BINS] = {
    {0, 1, 1},
    {0, 1, 0},
    {1, 1, 0},
    {1, 0, 1},
    {1, 1, 0},
    {1, 1, 1},
    {0, 0, 0},
    {0, 0, 0}
};

const float spectrum[WINDOW_COUNT][BINS] = {
    {-60, -10, -10},
    {-60, -5, -50},
    {-26, -15, -52},
    {-20, -45, -12},
    {-26, -5, -50},
    {-20, -10, -10},
    {-74, -62, -61},
    {-50, -50, -48}
};

const SpectrumEvent result_dur_1_prox_0[WINDOW_COUNT][BINS] = {
    {{.action = 0, .start = 0, .duration = 0}, {.action = 1, .start = 0, .duration = 1}, {.action = 1, .start = 0, .duration = 1}},
    {{.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 2}, {.action = 2, .start = 0, .duration = 1}},
    {{.action = 1, .start = 2, .duration = 1}, {.action = 0, .start = 0, .duration = 3}, {.action = 0, .start = 0, .duration = 0}},
    {{.action = 0, .start = 2, .duration = 2}, {.action = 2, .start = 0, .duration = 3}, {.action = 1, .start = 3, .duration = 1}},
    {{.action = 0, .start = 2, .duration = 3}, {.action = 1, .start = 4, .duration = 1}, {.action = 2, .start = 3, .duration = 1}},
    {{.action = 0, .start = 2, .duration = 4}, {.action = 0, .start = 4, .duration = 2}, {.action = 1, .start = 5, .duration = 1}},
    {{.action = 2, .start = 2, .duration = 4}, {.action = 2, .start = 4, .duration = 2}, {.action = 2, .start = 5, .duration = 1}},
    {{.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 0}}
};

const SpectrumEvent result_dur_2_prox_0[WINDOW_COUNT][BINS] = {
    {{.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 1}, {.action = 0, .start = 0, .duration = 1}},
    {{.action = 0, .start = 0, .duration = 0}, {.action = 1, .start = 0, .duration = 2}, {.action = 0, .start = 0, .duration = 0}},
    {{.action = 0, .start = 0, .duration = 1}, {.action = 0, .start = 0, .duration = 3}, {.action = 0, .start = 0, .duration = 0}},
    {{.action = 1, .start = 2, .duration = 2}, {.action = 2, .start = 0, .duration = 3}, {.action = 0, .start = 0, .duration = 1}},
    {{.action = 0, .start = 2, .duration = 3}, {.action = 0, .start = 0, .duration = 1}, {.action = 0, .start = 0, .duration = 0}},
    {{.action = 0, .start = 2, .duration = 4}, {.action = 1, .start = 4, .duration = 2}, {.action = 0, .start = 0, .duration = 1}},
    {{.action = 2, .start = 2, .duration = 4}, {.action = 2, .start = 4, .duration = 2}, {.action = 0, .start = 0, .duration = 0}},
    {{.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 0}}
};
 
const SpectrumEvent result_dur_3_prox_0[WINDOW_COUNT][BINS] = {
    {{.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 1}, {.action = 0, .start = 0, .duration = 1}},
    {{.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 2}, {.action = 0, .start = 0, .duration = 0}},
    {{.action = 0, .start = 0, .duration = 1}, {.action = 1, .start = 0, .duration = 3}, {.action = 0, .start = 0, .duration = 0}},
    {{.action = 0, .start = 0, .duration = 2}, {.action = 2, .start = 0, .duration = 3}, {.action = 0, .start = 0, .duration = 1}},
    {{.action = 1, .start = 2, .duration = 3}, {.action = 0, .start = 0, .duration = 1}, {.action = 0, .start = 0, .duration = 0}},
    {{.action = 0, .start = 2, .duration = 4}, {.action = 0, .start = 0, .duration = 2}, {.action = 0, .start = 0, .duration = 1}},
    {{.action = 2, .start = 2, .duration = 4}, {.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 0}},
    {{.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 0}}
};

const SpectrumEvent result_dur_1_prox_1[WINDOW_COUNT][BINS] = {
    {{.action = 0, .start = 0, .duration = 0}, {.action = 1, .start = 0, .duration = 1}, {.action = 1, .start = 0, .duration = 1}},
    {{.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 2}, {.action = 0, .start = 0, .duration = 1}},
    {{.action = 1, .start = 2, .duration = 1}, {.action = 0, .start = 0, .duration = 3}, {.action = 2, .start = 0, .duration = 1}},
    {{.action = 0, .start = 2, .duration = 2}, {.action = 0, .start = 0, .duration = 3}, {.action = 1, .start = 3, .duration = 1}},
    {{.action = 0, .start = 2, .duration = 3}, {.action = 0, .start = 0, .duration = 5}, {.action = 0, .start = 3, .duration = 1}},
    {{.action = 0, .start = 2, .duration = 4}, {.action = 0, .start = 0, .duration = 6}, {.action = 0, .start = 3, .duration = 3}},
    {{.action = 0, .start = 2, .duration = 4}, {.action = 0, .start = 0, .duration = 6}, {.action = 0, .start = 3, .duration = 3}},
    {{.action = 2, .start = 2, .duration = 4}, {.action = 2, .start = 0, .duration = 6}, {.action = 2, .start = 3, .duration = 3}}
};


const SpectrumEvent result_dur_2_prox_2[WINDOW_COUNT][BINS] = {
    {{.action = 0, .start = 0, .duration = 0}, {.action = 0, .start = 0, .duration = 1}, {.action = 0, .start = 0, .duration = 1}},
    {{.action = 0, .start = 0, .duration = 0}, {.action = 1, .start = 0, .duration = 2}, {.action = 0, .start = 0, .duration = 1}},
    {{.action = 0, .start = 0, .duration = 1}, {.action = 0, .start = 0, .duration = 3}, {.action = 0, .start = 0, .duration = 1}},
    {{.action = 1, .start = 2, .duration = 2}, {.action = 0, .start = 0, .duration = 3}, {.action = 1, .start = 0, .duration = 4}},
    {{.action = 0, .start = 2, .duration = 3}, {.action = 0, .start = 0, .duration = 5}, {.action = 0, .start = 0, .duration = 4}},
    {{.action = 0, .start = 2, .duration = 4}, {.action = 0, .start = 0, .duration = 6}, {.action = 0, .start = 0, .duration = 6}},
    {{.action = 0, .start = 2, .duration = 4}, {.action = 0, .start = 0, .duration = 6}, {.action = 0, .start = 0, .duration = 6}},
    {{.action = 0, .start = 2, .duration = 4}, {.action = 0, .start = 0, .duration = 6}, {.action = 0, .start = 0, .duration = 6}}
};

void event_test(const int MINIMAL_DURATION, const int TIME_PROXIMITY)
{
    SpectrumEvent events[BINS] = {};
    const SpectrumEvent (*results)[WINDOW_COUNT][BINS];

    if (MINIMAL_DURATION == 1 && TIME_PROXIMITY == 0)  {
        results = &result_dur_1_prox_0;
    } else if (MINIMAL_DURATION == 2 && TIME_PROXIMITY == 0)  {
         results = &result_dur_2_prox_0;
    } else if (MINIMAL_DURATION == 3 && TIME_PROXIMITY == 0)  {
         results = &result_dur_3_prox_0;
    } else if (MINIMAL_DURATION == 3 && TIME_PROXIMITY == 0)  {
         results = &result_dur_3_prox_0;
    } else if (MINIMAL_DURATION == 1 && TIME_PROXIMITY == 1)  {
         results = &result_dur_1_prox_1;
    } else if (MINIMAL_DURATION == 2 && TIME_PROXIMITY == 2)  {
         results = &result_dur_2_prox_2;
    } else {
        puts("Not valid test settings");
        return;
    }

    // events init
    for (uint16_t i = 0; i < BINS; i++) {
        events[i].start = SPECTRUM_EVENT_NONE;
        events[i].start = 0;
        events[i].last_seen = -1;
        events[i].duration = 0;
        events[i].amplitude = 0;
    }

    for (size_t t = 0; t < WINDOW_COUNT; t++) {
        size_t ch = event_detection(t, events, peaks[t], spectrum[t], BINS, MINIMAL_DURATION, TIME_PROXIMITY);

        printf("[t = %ld]  %ld\n", t, ch);
        for (size_t i = 0; i < BINS; i++) {
            printf("\t[%ld] (%d) start=%d, duration=%d, last_seen=%d, amp=%g\n", 
                   i, events[i].action, events[i].start, events[i].duration,
                   events[i].last_seen, events[i].amplitude);

            assert(
                (events[i].action == (*results)[t][i].action) &&
                (events[i].start == (*results)[t][i].start) &&
                (events[i].duration == (*results)[t][i].duration)
            );
        }
    }
}

int main()
{
    const int durations[] = {1, 2, 2, 1, 2};   // Oneskorí start event
    const int proximities[] = {0, 0, 0, 1, 2}; // Oneskorí end event

    for (int k = 0; k < 5; k++) {
        printf("\n\n------MIN TRVANIE: %d, TIME PROXIMITY: %d ----\n", durations[k], proximities[k]);
        event_test(durations[k], proximities[k]);
    }
}
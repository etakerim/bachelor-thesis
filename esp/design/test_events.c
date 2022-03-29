#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>



typedef enum {
    SPECTRUM_EVENT_NONE,
    SPECTRUM_EVENT_START,
    SPECTRUM_EVENT_FINISH
} SpectrumEventAction;

typedef struct {
    SpectrumEventAction action;
    uint32_t start;
    uint32_t duration;
    int32_t last_seen;
    float amplitude;
} SpectrumEvent;

#define square(x)   ((x) * (x))
#define min(x, y)   (((x) < (y)) ? (x): (y))
#define max(x, y)   (((x) > (y)) ? (x): (y))

/* Iba s minimal duration (referenčná implementácia)*/
/*
size_t event_detection(size_t t, SpectrumEvent *events, const bool *peaks, const float *spectrum,
                       uint16_t bins, uint16_t min_duration, uint16_t time_proximity)
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
*/

// Online algoritmus (neumožňuje prechod bezo zmeny, počíta s o jedna sa zvyšujúcim t)
size_t event_detection(size_t t, SpectrumEvent *events, const bool *peaks, const float *spectrum,
                       uint16_t bins, uint16_t min_duration, uint16_t time_proximity)
{
    size_t changes = 0;
    // Minimal duration = najkratšia súvislá postupnosť 1, aby bola vo vedierku vytvorená udalosť
    // Time proximity = aj keď je medzera núl a bola predtým 1, nezruš udalosť, ale počkaj na možnú jednotku

    for (uint16_t i = 0; i < bins; i++) {
        // Odstráň predošlú notifikáciu o udalosti vo frekvečnom vedierku
        if (events[i].action == SPECTRUM_EVENT_FINISH) {
            events[i].start = 0;
            events[i].last_seen = -1;  // -1 znamená, že nebola videná ešte jednotka
            events[i].duration = 0;
            events[i].amplitude = 0;
        }
        events[i].action = SPECTRUM_EVENT_NONE;

        if (peaks[i]) {
            events[i].last_seen++;
            uint32_t fastforward = max(1, events[i].last_seen);

            // Detekcia udalosti po prekročení minimálneho trvania
            if (events[i].duration < min_duration && 
                    events[i].duration + fastforward >= min_duration) {

                events[i].action = SPECTRUM_EVENT_START;
                events[i].start = t - events[i].duration - fastforward + 1;
                changes++;
            }

            // Súvislý výskyt vrcholov vo vedierku zvyšuje trvanie udalosti
            events[i].duration += fastforward;
            events[i].amplitude += (spectrum[i] - events[i].amplitude) / events[i].duration;
            events[i].last_seen = 0;    // Výskyt vrchola 'n' okien dozadu

        } else if (events[i].last_seen >= 0) {
            // Ak predtým bola videná 1-tka (vrchol) zvýš last_seen
            events[i].last_seen++;
            // Ak sú 1-tky príliš vzdialené, odznač udalosť
            if (events[i].last_seen > time_proximity) { 
                if (events[i].duration >= min_duration) {
                    // Vo vedierku nie je detegovaný vrchol
                    events[i].action = SPECTRUM_EVENT_FINISH;
                    changes++;

                } else {
                    // udalosť je príliš krátka a nebol medzičasom nájdený iný vrchol
                    events[i].start = 0;
                    events[i].last_seen = -1;  // -1 znamená, že nebola videná ešte jednotka
                    events[i].duration = 0;
                    events[i].amplitude = 0;
                }
            }
        }
    }

    return changes;
}


int main()
{
#define WINDOW_COUNT       8
#define BINS               3

    SpectrumEvent events[BINS] = {};
    bool peaks[WINDOW_COUNT][BINS] = {
        {0, 1, 1},
        {0, 1, 0},
        {1, 1, 0},
        {1, 0, 1},
        {1, 1, 0},
        {1, 1, 1},
        {0, 0, 0},
        {0, 0, 0}
    };

    /* Result
    min_duration = 1 (aj oneskorenie od začiatku), time_proximity = 0
        bin: 0
            <2; 5>
        bin: 1
            <0; 2>
            <4; 5>
        bin 2:
            <0; 0>
            <3; 3>
            <5; 5>
    */

    float spectrum[WINDOW_COUNT][BINS] = {
        {-60, -10, -10},
        {-60, -5, -50},
        {-26, -15, -52},
        {-20, -45, -12},
        {-26, -5, -50},
        {-20, -10, -10},
        {-74, -62, -61},
        {-50, -50, -48}
    };

    // events init
    for (uint16_t i = 0; i < BINS; i++) {    
        events[i].start = 0;
        events[i].last_seen = -1;
        events[i].duration = 0;
        events[i].amplitude = 0;
    }

    for (size_t t = 0; t < WINDOW_COUNT; t++) {
        size_t ch = event_detection(t, events, peaks[t], spectrum[t], BINS, 1, 0);

        printf("[t = %d]  %d\n", t, ch);
        for (size_t i = 0; i < BINS; i++) {
            printf("\t[%d] (%d) start=%d, duration=%d, last_seen=%d, amp=%g\n", 
                   i, events[i].action, events[i].start, events[i].duration,
                   events[i].last_seen, events[i].amplitude);
        }
    }
}

/*

REF:
[t = 0]  2
        [0] (0) start=0, duration=0, last_seen=0, amp=0
        [1] (1) start=0, duration=1, last_seen=0, amp=-10
        [2] (1) start=0, duration=1, last_seen=0, amp=-10
[t = 1]  1
        [0] (0) start=0, duration=0, last_seen=0, amp=0
        [1] (0) start=0, duration=2, last_seen=0, amp=-7.5
        [2] (2) start=0, duration=1, last_seen=0, amp=-10
[t = 2]  1
        [0] (1) start=2, duration=1, last_seen=0, amp=-26
        [1] (0) start=0, duration=3, last_seen=0, amp=-10
        [2] (0) start=0, duration=0, last_seen=0, amp=0
[t = 3]  2
        [0] (0) start=2, duration=2, last_seen=0, amp=-23
        [1] (2) start=0, duration=3, last_seen=0, amp=-10
        [2] (1) start=3, duration=1, last_seen=0, amp=-12
[t = 4]  2
        [0] (0) start=2, duration=3, last_seen=0, amp=-24
        [1] (1) start=4, duration=1, last_seen=0, amp=-5
        [2] (2) start=3, duration=1, last_seen=0, amp=-12
[t = 5]  1
        [0] (0) start=2, duration=4, last_seen=0, amp=-23
        [1] (0) start=4, duration=2, last_seen=0, amp=-7.5
        [2] (1) start=5, duration=1, last_seen=0, amp=-10
[t = 6]  3
        [0] (2) start=2, duration=4, last_seen=0, amp=-23
        [1] (2) start=4, duration=2, last_seen=0, amp=-7.5
        [2] (2) start=5, duration=1, last_seen=0, amp=-10
[t = 7]  0
        [0] (0) start=0, duration=0, last_seen=0, amp=0
        [1] (0) start=0, duration=0, last_seen=0, amp=0
        [2] (0) start=0, duration=0, last_seen=0, amp=0

T_PROX:
[t = 0]  2
        [0] (0) start=0, duration=0, last_seen=-1, amp=0
        [1] (1) start=0, duration=1, last_seen=0, amp=-10
        [2] (1) start=0, duration=1, last_seen=0, amp=-10
[t = 1]  1
        [0] (0) start=0, duration=0, last_seen=-1, amp=0
        [1] (0) start=0, duration=2, last_seen=0, amp=-7.5
        [2] (2) start=0, duration=1, last_seen=1, amp=-10
[t = 2]  1
        [0] (1) start=2, duration=1, last_seen=0, amp=-26
        [1] (0) start=0, duration=3, last_seen=0, amp=-10
        [2] (0) start=0, duration=0, last_seen=-1, amp=0
[t = 3]  2
        [0] (0) start=2, duration=2, last_seen=0, amp=-23
        [1] (2) start=0, duration=3, last_seen=1, amp=-10
        [2] (1) start=3, duration=1, last_seen=0, amp=-12
[t = 4]  2
        [0] (0) start=2, duration=3, last_seen=0, amp=-24
        [1] (1) start=4, duration=1, last_seen=0, amp=-5
        [2] (2) start=3, duration=1, last_seen=1, amp=-12
[t = 5]  1
        [0] (0) start=2, duration=4, last_seen=0, amp=-23
        [1] (0) start=4, duration=2, last_seen=0, amp=-7.5
        [2] (1) start=5, duration=1, last_seen=0, amp=-10
[t = 6]  3
        [0] (2) start=2, duration=4, last_seen=1, amp=-23
        [1] (2) start=4, duration=2, last_seen=1, amp=-7.5
        [2] (2) start=5, duration=1, last_seen=1, amp=-10
[t = 7]  0
        [0] (0) start=0, duration=0, last_seen=-1, amp=0
        [1] (0) start=0, duration=0, last_seen=-1, amp=0
        [2] (0) start=0, duration=0, last_seen=-1, amp=0

*/
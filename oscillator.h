#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "common.h"

typedef struct _Oscillator {
    phase_t phase;
    float   freq;
} Oscillator;

void oscillator_init(Oscillator *osc);

void oscillator_reset(Oscillator *osc);

void oscillator_set_freq(Oscillator *osc, float freq);

void oscillator_process(Oscillator *osc, sample_t *buf, int size);

#endif /* OSCILLATOR_H */

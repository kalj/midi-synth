#ifndef SYNTH_H
#define SYNTH_H

#include "common.h"

#include "oscillator.h"

#include <stdbool.h>

typedef struct _Synth {
    Oscillator osc;

    float MAX_PITCH_BEND;

    bool  on;
    float bend;

    // Envelope env;
} Synth;

void synth_init(Synth *synth);

void synth_process(Synth *synth, sample_t *buffer, int size);

void synth_handle_note(Synth *synth, int on, int note);

void synth_handle_bend(Synth *synth, float value);

#endif /* SYNTH_H */

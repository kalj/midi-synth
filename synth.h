#ifndef SYNTH_H
#define SYNTH_H

#include "common.h"

#include "envelope.h"
#include "oscillator.h"

#include <stdbool.h>

typedef struct _Synth {
    Oscillator osc;

    float MAX_PITCH_BEND;

    bool  on;
    float bend;

    Envelope env;
} Synth;

void synth_init(Synth *synth, float A, float D, float S, float R);

void synth_process_block(Synth *synth, sample_t *buffer);

void synth_handle_note(Synth *synth, int on, int note);

void synth_handle_bend(Synth *synth, float value);

#endif /* SYNTH_H */

#ifndef SYNTH_H
#define SYNTH_H

#include "common.h"

typedef struct _Synth {
    int     samplerate;
    int     n_channels;
    phase_t phase;
    phase_t dphase;
    int     on;
} Synth;

void synth_init(Synth *synth, int samplerate, int n_channels);

void synth_fill_buffer(Synth *synth, void *void_buffer, int period_size);

void synth_handle_note(Synth *synth, int on, float freq);

#endif /* SYNTH_H */

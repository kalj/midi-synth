#ifndef SYNTH_H
#define SYNTH_H

#include "common.h"

typedef struct _Synth {
    int     samplerate;
    int     n_channels;
    phase_t phase;
    int note;
    int     on;
    float bend;
    float max_pitch_bend;
} Synth;

void synth_init(Synth *synth, int samplerate, int n_channels);

void synth_fill_buffer(Synth *synth, void *void_buffer, int period_size);

void synth_handle_note(Synth *synth, int on, int note);

void synth_handle_bend(Synth *synth, float value);

#endif /* SYNTH_H */

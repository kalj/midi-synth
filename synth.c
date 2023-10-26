#include "synth.h"

#include "common.h"

#include <math.h>
#include <string.h>

static float midi_freq_table[256];

void synth_process(Synth *synth, sample_t *buffer, int size)
{
    if (synth->on) {
        oscillator_process(&synth->osc, buffer, size);
        /* envelope_process(&synth->env, buffer, size); */
    } else {
        memset(buffer, 0, sizeof(sample_t) * size);
    }
}

void synth_init(Synth *synth)
{
    for (int i = 0; i < 128; i++) {
        midi_freq_table[i] = 440.0f * pow(2, (i - 69) / 12.f);
    }

    oscillator_init(&synth->osc);
    /* envelope_init(&synth->env); */
    synth->MAX_PITCH_BEND = powf(2, 2 / 12.0); // 200 cent
    synth->on             = 0;
}

static void update_oscillator_freq(Synth *synth)
{
    float freq = midi_freq_table[synth->note] * powf(synth->MAX_PITCH_BEND, synth->bend);
    oscillator_set_freq(&synth->osc, freq);
}

void synth_handle_note(Synth *synth, int on, int note)
{
    synth->note = note;
    if (on) {
        update_oscillator_freq(synth);

        if (!synth->on) {
            oscillator_reset(&synth->osc);
        }
    }
    synth->on = on;
}

void synth_handle_bend(Synth *synth, float value)
{
    synth->bend = value;
    update_oscillator_freq(synth);
}

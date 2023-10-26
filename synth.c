#include "synth.h"

#include "common.h"

#include <math.h>
#include <string.h>

#define PHASE_MIDPOINT (((phase_t)(-1)) >> 1)
#define SAMPLE_MAX     ((sample_t)(1 << (8 * sizeof(sample_t) - 1)))
#define SAMPLE_MIN     ((sample_t)((1 << (8 * sizeof(sample_t) - 1)) - 1))

float midi_freq_table[256];

void initFreqTable()
{
    for (int i = 0; i < 128; i++) {
        midi_freq_table[i] = 440.0f * pow(2, (i - 69) / 12.f);
    }
}

static inline sample_t square_wave(phase_t phase)
{
    return phase > PHASE_MIDPOINT ? SAMPLE_MAX : SAMPLE_MIN;
}

static inline sample_t saw_wave(phase_t phase)
{
    return (sample_t)(phase >> (8 * sizeof(phase_t) - 8 * sizeof(sample_t)));
}

static inline sample_t triangle_wave(phase_t phase)
{
    return phase > ((PHASE_MIDPOINT / 2) * 3) ? 2 * phase                 // end part
           : phase < (PHASE_MIDPOINT / 2)     ? 2 * phase                 // beginning
                                              : 2 * (PHASE_MIDPOINT - phase); // middle
}

void synth_fill_buffer(Synth *synth, void *void_buffer, int period_size)
{
    sample_t *buffer = (sample_t *)void_buffer;
    float     phase_per_sample =
        midi_freq_table[synth->note] * powf(synth->max_pitch_bend, synth->bend) / synth->samplerate;
    phase_t dphase = phase_per_sample * ((phase_t)(-1));

    if (synth->on) {
        sample_t value;
        for (int i = 0; i < period_size; i++) {
            /* sample_t value = square_wave(synth->phase); */
            /* sample_t value = saw_wave(synth->phase); */
            /* value = triangle_wave(synth->phase); */

            value = SAMPLE_MAX * sin(synth->phase / (float)(PHASE_MIDPOINT)*M_PI);
            for (int j = 0; j < synth->n_channels; j++) {
                buffer[i * synth->n_channels + j] = value;
            }
            synth->phase += dphase;
        }
    } else {
        memset(void_buffer, 0, sizeof(sample_t) * synth->n_channels * period_size);
    }
}

void synth_init(Synth *synth, int samplerate, int n_channels)
{
    initFreqTable();
    synth->max_pitch_bend = powf(2, 1 / 12.0);
    synth->max_pitch_bend *= synth->max_pitch_bend;

    synth->samplerate = samplerate;
    synth->n_channels = n_channels;
    synth->on         = 0;
}

void synth_handle_note(Synth *synth, int on, int note)
{
    if (on && !synth->on) {
        synth->phase = 0;
    }
    synth->on   = on;
    synth->note = note;
}

void synth_handle_bend(Synth *synth, float value)
{
    synth->bend = value;
}

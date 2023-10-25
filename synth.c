#include "synth.h"

#include "common.h"

#include <string.h>

#define PHASE_MIDPOINT (((phase_t)(-1)) >> 1)
#define SAMPLE_MAX     ((sample_t)(1 << (8 * sizeof(sample_t) - 1)))
#define SAMPLE_MIN     ((sample_t)((1 << (8 * sizeof(sample_t) - 1)) - 1))

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
    return phase > ((PHASE_MIDPOINT/2)*3) ? 2*phase // end part 
        : phase < (PHASE_MIDPOINT/2) ? 2*phase // beginning
        : 2*(PHASE_MIDPOINT-phase) ; // middle
}

void synth_fill_buffer(Synth *synth, void *void_buffer, int period_size)
{
    sample_t *buffer = (sample_t *)void_buffer;

    if (synth->on) {
        for (int i = 0; i < period_size; i++) {
            /* sample_t value = square_wave(synth->phase); */
            sample_t value = saw_wave(synth->phase);
            /* sample_t value = triangle_wave(synth->phase); */

            for (int j = 0; j < synth->n_channels; j++) {
                buffer[i * synth->n_channels + j] = value;
            }
            synth->phase += synth->dphase;
        }
    } else {
        memset(void_buffer, 0, sizeof(sample_t) * synth->n_channels * period_size);
    }
}

void synth_init(Synth *synth, int samplerate, int n_channels)
{
    synth->samplerate = samplerate;
    synth->n_channels = n_channels;
    synth->on         = 0;
}

void synth_handle_note(Synth *synth, int on, float freq)
{
    synth->on = on;
    if (synth->on) {
        float phase_per_sample = freq / synth->samplerate;
        synth->dphase          = phase_per_sample * ((phase_t)(-1));
    }
}

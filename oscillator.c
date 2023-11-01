#include "oscillator.h"

#include "common.h"

#include <math.h>

#define PHASE_MIDPOINT (((phase_t)(-1)) >> 1)
#define SAMPLE_MAX     ((sample_t)(1 << (8 * sizeof(sample_t) - 1)))
#define SAMPLE_MIN     ((sample_t)((1 << (8 * sizeof(sample_t) - 1)) - 1))

void oscillator_init(Oscillator *osc)
{
    (void)osc;
}

void oscillator_reset(Oscillator *osc)
{
    osc->phase = 0;
}

void oscillator_set_freq(Oscillator *osc, float freq)
{
    osc->freq = freq;
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

void oscillator_process(Oscillator *osc, sample_t *buffer, int size)
{
    float   phase_per_sample = osc->freq / SAMPLERATE;
    phase_t dphase           = phase_per_sample * ((phase_t)(-1));

    sample_t value;
    for (int i = 0; i < size; i++) {
        /* value = saw_wave(osc->phase); */
        /* value     = square_wave(osc->phase); */
        /* value     = triangle_wave(osc->phase); */
        value     = SAMPLE_MAX * sin(osc->phase / (float)(PHASE_MIDPOINT)*M_PI);
        buffer[i] = value;
        /* for (int j = 0; j < synth->n_channels; j++) { */
        /*     buffer[i * synth->n_channels + j] = value; */
        /* } */
        osc->phase += dphase;
    }
}

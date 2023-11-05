#include "oscillator.h"

#include "common.h"

#include <math.h>

#define PHASE_MIDPOINT (((phase_t)(-1)) >> 1)

#ifdef SAMPLE_TYPE_F32
#define SAMPLE_MAX 1.0f
#define SAMPLE_MIN -1.0f
#else
#define SAMPLE_MAX ((sample_t)(1 << (8 * sizeof(sample_t) - 1)))
#define SAMPLE_MIN ((sample_t)((1 << (8 * sizeof(sample_t) - 1)) - 1))
#endif

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

#define FLOAT_PHASE(ph) (ph / (float)((phase_t)~0))

static inline sample_t saw_wave(phase_t phase)
{
#ifdef SAMPLE_TYPE_F32
    return phase < PHASE_MIDPOINT ? 2 * FLOAT_PHASE(phase) : 2 * (FLOAT_PHASE(phase) - 1);
#else
    return (sample_t)(phase >> (8 * sizeof(phase_t) - 8 * sizeof(sample_t)));
#endif
}

static inline sample_t triangle_wave(phase_t phase)
{
#ifdef SAMPLE_TYPE_F32
    return phase > ((PHASE_MIDPOINT / 2) * 3) ? 4 * (FLOAT_PHASE(phase) - 1) // end part
           : phase < (PHASE_MIDPOINT / 2)     ? 4 * (FLOAT_PHASE(phase))     // beginning
                                              : 4 * (0.5f - FLOAT_PHASE(phase)); // middle

#else
    return phase > ((PHASE_MIDPOINT / 2) * 3) ? 2 * phase                 // end part
           : phase < (PHASE_MIDPOINT / 2)     ? 2 * phase                 // beginning
                                              : 2 * (PHASE_MIDPOINT - phase); // middle
#endif
}

void oscillator_process_block(Oscillator *osc, sample_t *buffer)
{
    float   phase_per_sample = osc->freq / SAMPLERATE;
    phase_t dphase           = phase_per_sample * ((phase_t)~0);

    sample_t value;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        /* value = saw_wave(osc->phase); */
        /* value     = square_wave(osc->phase); */
        value = triangle_wave(osc->phase);
        /* value     = SAMPLE_MAX * sin(osc->phase / (float)(PHASE_MIDPOINT)*M_PI); */
        buffer[i] = value;
        osc->phase += dphase;
    }
}

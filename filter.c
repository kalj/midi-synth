#include "filter.h"

#include <math.h>
#include <stdio.h>

#ifdef FILTER_VARIANT_BW
void filter_init(Filter *flt, float cutoff)
{
    // clamp to [FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX] Hz
    if (cutoff < FILTER_CUTOFF_MIN) cutoff = FILTER_CUTOFF_MIN;
    if (cutoff > FILTER_CUTOFF_MAX) cutoff = FILTER_CUTOFF_MAX;

    float     a                 = tan(M_PI * cutoff / SAMPLERATE);
    float     a2                = a * a;
    const int butterworth_order = 2 * FILTER_N_BIQUADS;

    for (int j = 0; j < FILTER_N_BIQUADS; ++j) {
        float r = sin(M_PI * (2.0 * j + 1.0) / (2.0 * butterworth_order));
        float s = a2 + 2.0 * a * r + 1.0;

        flt->bq[j].A  = a2 / s;
        flt->bq[j].d1 = 2.0 * (1 - a2) / s;
        flt->bq[j].d2 = -(a2 - 2.0 * a * r + 1.0) / s;
        /* flt->bq[j].w1 = 0; */
        /* flt->bq[j].w2 = 0; */
    }
}

void filter_process(Filter *flt, sample_t *buffer, int len)
{
    for (int i = 0; i < len; i++) {
        sample_t y = buffer[i];
        for (int j = 0; j < FILTER_N_BIQUADS; ++j) {
            Biquad *bq = &flt->bq[j];

            sample_t w0 = bq->d1 * bq->w1 + bq->d2 * bq->w2 + y;
            y           = bq->A * (w0 + 2.0 * bq->w1 + bq->w2);
            bq->w2      = bq->w1;
            bq->w1      = w0;
        }
        buffer[i] = y;
    }
}

#else

static void filter_update_coeffs(Filter *flt)
{
    float w_cutoff = flt->cutoff * (1.0f / (SAMPLERATE / 2.0f));
    float q1       = 1.0 - w_cutoff;
    flt->p         = w_cutoff + 0.8 * w_cutoff * q1;
    flt->f         = flt->p + flt->p - 1.0;
    flt->q         = flt->resonance * (1.0 + 0.5 * q1 * (1.0 - q1 + 5.6 * q1 * q1));
}

void filter_init(Filter *flt, float cutoff, float resonance)
{
    // clamp to [FILTER_CUTOFF_MIN, FILTER_CUTOFF_MAX] Hz
    if (cutoff < FILTER_CUTOFF_MIN) cutoff = FILTER_CUTOFF_MIN;
    if (cutoff > FILTER_CUTOFF_MAX) cutoff = FILTER_CUTOFF_MAX;
    flt->cutoff = cutoff;

    if (resonance < 0.0f) resonance = 0;
    if (resonance > 1.0f) resonance = 1.0f;
    flt->resonance = resonance;

    filter_update_coeffs(flt);
}

void filter_process(Filter *flt, sample_t *buffer, int len)
{
    for (int i = 0; i < len; i++) {

        sample_t b0new = buffer[i] - flt->q * flt->b4; //feedback

        float b1new = (b0new + flt->b0) * flt->p - flt->b1 * flt->f;

        float b2new = (b1new + flt->b1) * flt->p - flt->b2 * flt->f;
        float b3new = (b2new + flt->b2) * flt->p - flt->b3 * flt->f;
        float b4new = (b3new + flt->b3) * flt->p - flt->b4 * flt->f;
        b4new       = b4new - b4new * b4new * b4new * 0.166667; //clipping

        flt->b0 = b0new;
        flt->b1 = b1new;
        flt->b2 = b2new;
        flt->b3 = b3new;
        flt->b4 = b4new;

        buffer[i] = b4new; // lp
        /* buffer[i] = b0new-b4new; //hp */
        /* buffer[i] = 3.0 * (b3new - b4new); // bp */
    }
}

#endif

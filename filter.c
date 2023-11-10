#include "filter.h"

#include <math.h>
#include <stdio.h>

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

#ifndef FILTER_H
#define FILTER_H

#include "common.h"

// #define FILTER_VARIANT_BW

#ifdef FILTER_VARIANT_BW
typedef struct {
    // coeffs
    sample_t A, d1, d2;
    // state
    sample_t w1, w2;
} Biquad;

#define FILTER_N_BIQUADS  3
#define FILTER_CUTOFF_MIN 200.0f
#define FILTER_CUTOFF_MAX 5000.0f

typedef struct _Filter {
    Biquad bq[FILTER_N_BIQUADS];
} Filter;

void filter_init(Filter *flt, float cutoff);
void filter_process(Filter *flt, sample_t *buffer, int len);

#else

#define FILTER_CUTOFF_MIN 20.0f
#define FILTER_CUTOFF_MAX 2020.0f

typedef struct _Filter {
    // coeffs
    float resonance, cutoff;
    float p, f, q;
    // state
    float b0, b1, b2, b3, b4;
} Filter;

void filter_init(Filter *flt, float cutoff, float resonance);
void filter_process(Filter *flt, sample_t *buffer, int len);

#endif

#endif /* FILTER_H */

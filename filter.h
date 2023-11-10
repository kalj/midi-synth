#ifndef FILTER_H
#define FILTER_H

#include "common.h"

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

#endif /* FILTER_H */

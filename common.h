#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define SAMPLERATE 44100

// #define BIT_DEPTH 8
// #define BIT_DEPTH 16
#define BIT_DEPTH 32

// typedef uint8_t phase_t;
// typedef uint16_t phase_t;
typedef uint32_t phase_t;
#if BIT_DEPTH == 8
typedef int8_t sample_t;
#elif BIT_DEPTH == 16
typedef int16_t sample_t;
#elif BIT_DEPTH == 32
typedef int32_t sample_t;
#endif

#endif /* COMMON_H */

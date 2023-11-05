#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define BLOCK_SIZE 256
#define SAMPLERATE 44100
#define SAMPLE_TYPE_F32
// #define SAMPLE_TYPE_I32

// typedef uint8_t phase_t;
// typedef uint16_t phase_t;
typedef uint32_t phase_t;

#ifdef SAMPLE_TYPE_I8
typedef int8_t sample_t;
#elif defined(SAMPLE_TYPE_I16)
typedef int16_t sample_t;
#elif defined(SAMPLE_TYPE_I32)
typedef int32_t sample_t;
#elif defined(SAMPLE_TYPE_F32)
typedef float sample_t;
#endif

#endif /* COMMON_H */

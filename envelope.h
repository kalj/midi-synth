#ifndef ENVELOPE_H
#define ENVELOPE_H

#include "common.h"

#include <stdbool.h>

typedef enum {
    ENV_STATE_OFF,
    ENV_STATE_ATTACK,
    ENV_STATE_DECAY,
    ENV_STATE_SUSTAIN,
    ENV_STATE_RELEASE,
} EnvState;

typedef struct _Envelope {
    float attack;
    float decay;
    float sustain;
    float release;

    EnvState state;
    union {
        float attack_left;
        float decay_left;
        float release_left;
    };
} Envelope;

void envelope_init(Envelope *env, float attack, float decay, float sustain, float release);
void envelope_gate(Envelope *env, bool on);
void envelope_process(Envelope *env, sample_t *buffer, int len);

#endif /* ENVELOPE_H */

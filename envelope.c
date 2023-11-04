#include "envelope.h"

void envelope_init(Envelope *env, float attack, float decay, float sustain, float release)
{
    env->attack  = attack;
    env->decay   = decay;
    env->sustain = sustain;
    env->release = release;

    env->state = ENV_STATE_OFF;
}

void envelope_gate(Envelope *env, bool on)
{
    if (on) {
        env->state       = ENV_STATE_ATTACK;
        env->attack_left = env->attack;
    } else {
        env->state        = ENV_STATE_RELEASE;
        env->release_left = env->release;
    }
}

void envelope_process(Envelope *env, sample_t *buffer, int len)
{
    float dt    = 1.0f / SAMPLERATE;
    float inv_a = 1.0f / env->attack;
    for (int i = 0; i < len; i++) {
        float gain = 0.0f;

        switch (env->state) {
            case ENV_STATE_ATTACK:
                gain = (env->attack - env->attack_left) * inv_a;
                env->attack_left -= dt;
                if (env->attack_left <= 0) {
                    env->state      = ENV_STATE_DECAY;
                    env->decay_left = env->decay + env->attack_left; // remove any time overstepping
                }
                break;

            case ENV_STATE_DECAY:
                gain = env->decay_left * inv_a;
                gain = 1 + (1 - env->sustain) * (env->decay_left / env->decay - 1);
                env->decay_left -= dt;
                if (env->decay_left <= 0) {
                    env->state = ENV_STATE_SUSTAIN;
                }
                break;
            case ENV_STATE_SUSTAIN:
                gain = env->sustain;
                break;
            case ENV_STATE_RELEASE:
                gain = env->release_left / env->release * env->sustain;
                env->release_left -= dt;
                if (env->release_left <= 0) {
                    env->state = ENV_STATE_OFF;
                }
                break;
            default:
            case ENV_STATE_OFF:
                /* factor = 0.0f; */
                break;
        }
        buffer[i] *= gain;
    }
}

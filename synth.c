#include "synth.h"

#include "common.h"
#include "filter.h"

#include <math.h>
#include <string.h>

static float midi_freq_table[256];

void synth_process(Synth *synth, sample_t *buffer, int size)
{
    if (synth->env.state == ENV_STATE_OFF) {
        memset(buffer, 0, sizeof(sample_t) * size);
    } else {
        oscillator_process(&synth->osc, buffer, size);
        filter_process(&synth->flt, buffer, size);
        envelope_process(&synth->env, buffer, size);
    }
}

#define MAX_HISTORY_LEN 12

typedef struct _NoteNode {
    int8_t note;
    int8_t prev;
    int8_t next;
} NoteNode;

#define NO_NODE -1

static int8_t notes_pool_head = 0;
static int8_t notes_head      = NO_NODE;

static NoteNode notes[MAX_HISTORY_LEN];

void synth_init(Synth *synth, float A, float D, float S, float R)
{
    for (int i = 0; i < 128; i++) {
        midi_freq_table[i] = 440.0f * pow(2, (i - 69) / 12.f);
    }

    notes_pool_head = 0;
    int8_t prev     = NO_NODE;
    for (int i = 0; i < MAX_HISTORY_LEN; i++) {
        notes[i].prev = prev;
        prev          = i;
        if (i != (MAX_HISTORY_LEN - 1)) {
            notes[i].next = i + 1;
        } else {
            notes[i].next = NO_NODE;
        }
    }

    oscillator_init(&synth->osc);
    envelope_init(&synth->env, A, D, S, R);
    filter_init(&synth->flt, FILTER_CUTOFF_MAX);
    synth->MAX_PITCH_BEND = powf(2, 2 / 12.0); // 200 cent
    synth->on             = 0;
}

static void update_oscillator(Synth *synth)
{
    bool new_on = true;
    if (notes_head == NO_NODE) {
        new_on = false;
    }

    if (new_on) {
        float note_freq = midi_freq_table[notes[notes_head].note];
        float freq      = note_freq * powf(synth->MAX_PITCH_BEND, synth->bend);
        oscillator_set_freq(&synth->osc, freq);
    }

    if (new_on && !synth->on) {
        oscillator_reset(&synth->osc);
        envelope_gate(&synth->env, true);
    } else if (!new_on && synth->on) {
        envelope_gate(&synth->env, false);
    }

    synth->on = new_on;
}

void synth_handle_note(Synth *synth, int on, int note)
{

    if (on) {

        // assume that on note was not already on!!!

        int note_idx = notes_pool_head;

        if (note_idx != NO_NODE) { // pool not empty
            // pop from pool
            notes_pool_head = notes[note_idx].next;
            // prev is irrelvant in pool, skip setting
        } else { // pool was empty

            // steal oldest note
            note_idx = notes_head;
            while (notes[note_idx].next != NO_NODE) {
                note_idx = notes[note_idx].next;
            }

            notes[notes[note_idx].prev].next = NO_NODE; // make second to last note new last
            // steal done, note_idx is valid
        }

        // note_idx is now valid, and pool has new valid state
        notes[note_idx].note = note;
        notes[note_idx].prev = NO_NODE;
        notes[note_idx].next = notes_head;
        if (notes_head != NO_NODE) {
            notes[notes_head].prev = note_idx;
        }
        notes_head = note_idx;

        update_oscillator(synth);
    } else {
        int note_idx = notes_head;
        while (notes[note_idx].note != note && note_idx != NO_NODE) {
            note_idx = notes[note_idx].next;
        }
        if (note_idx == NO_NODE) {
            // our note was not in the history
            // don't do anything
        } else {
            if (notes[note_idx].prev != NO_NODE) {
                // point prev to next
                notes[notes[note_idx].prev].next = notes[note_idx].next;
            } else {
                // point head to next
                notes_head = notes[note_idx].next;
            }

            if (notes[note_idx].next != NO_NODE) {
                // point next to prev
                notes[notes[note_idx].next].prev = notes[note_idx].prev;
            }

            // put back in pool
            notes[note_idx].prev = NO_NODE;
            notes[note_idx].next = notes_pool_head;
            notes_pool_head      = note_idx;

            update_oscillator(synth);
        }
    }
}

void synth_handle_bend(Synth *synth, float value)
{
    synth->bend = value;
    update_oscillator(synth);
}

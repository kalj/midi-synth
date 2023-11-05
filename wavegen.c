
#include "common.h"
#include "synth.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int n_samples = SAMPLERATE; // 1 sec

    int       block_len = BLOCK_SIZE;
    sample_t *block     = malloc(block_len * sizeof(sample_t));

    Synth synth = {};
    /* synth_init(&synth, 0,0,1, 0); */
    synth_init(&synth, 0.1, 0.1, 0.5, 0.5);

    const char *fname = "wave.dat";
    FILE       *fp    = fopen(fname, "w");

    for (int i = 0; i < n_samples; i += BLOCK_SIZE) {
        if (i == BLOCK_SIZE) {
            synth_handle_note(&synth, 1, 69);
        }

        if (i == BLOCK_SIZE * 25) {
            synth_handle_note(&synth, 1, 64);
        }

        if (i == BLOCK_SIZE * 150) {
            synth_handle_note(&synth, 0, 64);
        }

        synth_process_block(&synth, block);

        for (int i = 0; i < block_len; i++) {
#ifdef SAMPLE_TYPE_F32
            fprintf(fp, "%e\n", block[i]);
#else
            fprintf(fp, "%d\n", block[i]);
#endif
        }
    }

    fclose(fp);

    free(block);

    return 0;
}

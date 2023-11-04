
#include "common.h"
#include "synth.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int n_samples = SAMPLERATE; // 1 sec

    int       block_len = 64;
    sample_t *block     = malloc(block_len * sizeof(sample_t));

    Synth synth = {};
    synth_init(&synth);

    const char *fname = "wave.dat";
    FILE       *fp    = fopen(fname, "w");

    for (int bk = 0; bk * block_len < n_samples; bk++) {
        if (bk == 8) {
            synth_handle_note(&synth, 1, 69);
        }

        if (bk == 100) {
            synth_handle_note(&synth, 1, 64);
        }

        if (bk == 600) {
            synth_handle_note(&synth, 0, 64);
        }

        synth_process(&synth, block, block_len);

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

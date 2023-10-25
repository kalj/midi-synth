
#include "common.h"
#include "synth.h"

#include <stdlib.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    int buffer_len = SAMPLERATE/10;
    sample_t *buffer = malloc(buffer_len*sizeof(sample_t));

    Synth synth = {};
    synth_init(&synth, SAMPLERATE, 1);

    synth_handle_note(&synth, 1, 440);

    synth_fill_buffer(&synth, buffer, buffer_len);


    const char *fname = "wave.dat";

    FILE *fp = fopen(fname,"w");

    for(int i=0; i< buffer_len; i++)
    {
        fprintf(fp, "%d\n", buffer[i]);
    }

    fclose(fp);


    free(buffer);


    return 0;
}

#include "common.h"
#include "synth.h"

#include <alsa/asoundlib.h>
#include <alsa/error.h>
#include <alsa/pcm.h>
#include <alsa/seq.h>
#include <unistd.h>

#define CHK(stm, msg)                                                                              \
    do {                                                                                           \
        int ret = stm;                                                                             \
        if (ret < 0) {                                                                             \
            fprintf(stderr, "%s:%d Error: %s (%s)\n", __FILE__, __LINE__, msg, snd_strerror(ret)); \
            exit(1);                                                                               \
        }                                                                                          \
    } while (0)

/* #define BUFFER_SIZE 128 */
/* #define N_CHANNELS 1 */
#define N_CHANNELS 2


int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;


    snd_pcm_t *pcm_handle;

    CHK(snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0), "Failed to open PCM");

    /* snd_pcm_set_params(pcm_handle, */
    /*                SND_PCM_FORMAT_S16_LE, */
    /*                SND_PCM_ACCESS_RW_INTERLEAVED, */
    /*                2, */
    /*                SAMPLERATE, 0, 20000); */

    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);
    CHK(snd_pcm_hw_params_any(pcm_handle, params), "Failed in params_any");

    /* Set parameters */
    CHK(snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED), "Can't set interleaved mode");

    /* SND_PCM_FORMAT_S16_LE */
    snd_pcm_format_t requested_format = BIT_DEPTH == 8    ? SND_PCM_FORMAT_U8
                                        : BIT_DEPTH == 16 ? SND_PCM_FORMAT_S16_LE
                                        : BIT_DEPTH == 32 ? SND_PCM_FORMAT_S32_LE
                                                          : -1;

    CHK(snd_pcm_hw_params_set_format(pcm_handle, params, requested_format), "Can't set format");
    CHK(snd_pcm_hw_params_set_channels(pcm_handle, params, N_CHANNELS), "Can't set channels number");

    unsigned int requested_rate = SAMPLERATE;
    CHK(snd_pcm_hw_params_set_rate_near(pcm_handle, params, &requested_rate, 0), "Can't set rate");

    CHK(snd_pcm_hw_params_set_period_size(pcm_handle, params, 64, 0), "Can't set period size");
    CHK(snd_pcm_hw_params_set_periods(pcm_handle, params, 8, 0), "Can't set periods");

    /* Write parameters */
    CHK(snd_pcm_hw_params(pcm_handle, params), "Can't set harware parameters");

    /* Read information */
    printf("PCM name:  '%s'\n", snd_pcm_name(pcm_handle));
    printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));

    unsigned int n_channels;
    CHK(snd_pcm_hw_params_get_channels(params, &n_channels), "Failed to get number of params");
    printf("channels           = %u\n", n_channels);

    unsigned int rate;
    CHK(snd_pcm_hw_params_get_rate(params, &rate, 0), "Failed to get rate");
    printf("rate               = %d Hz\n", rate);

    unsigned int val;
    CHK(snd_pcm_hw_params_get_access(params, (snd_pcm_access_t *)&val), "Failed to get access type");
    printf("access type        = %s\n", snd_pcm_access_name((snd_pcm_access_t)val));

    snd_pcm_format_t fmt;
    CHK(snd_pcm_hw_params_get_format(params, &fmt), "Failed to get format");

    int bits_per_sample = snd_pcm_format_width(fmt);
    printf("format             = '%s' (%s) bits:%d\n",
           snd_pcm_format_name(fmt),
           snd_pcm_format_description(fmt),
           bits_per_sample);

    unsigned int period_time;
    CHK(snd_pcm_hw_params_get_period_time(params, &period_time, NULL), "Failed to get period time");
    printf("period time        = %d us\n", period_time);

    snd_pcm_uframes_t period_size;
    CHK(snd_pcm_hw_params_get_period_size(params, &period_size, NULL), "Failed to get period size");
    printf("period size        = %lu frames\n", period_size);

    CHK(snd_pcm_hw_params_get_buffer_time(params, &period_time, NULL), "Failed to get buffer time");
    printf("buffer time        = %d us\n", period_time);

    
    CHK(snd_pcm_hw_params_get_periods(params, &val, NULL), "Failed to get periods");
    printf("periods            = %d\n", val);

    snd_pcm_uframes_t buffer_size;
    CHK(snd_pcm_hw_params_get_buffer_size(params, &buffer_size), "Failed to get buffer size");
    printf("buffer size        = %lu frames\n", buffer_size);

    size_t bytes_per_frame = N_CHANNELS * (bits_per_sample / 8);
    printf("bytes_per_frame    = %lu\n", bytes_per_frame);
    size_t size = period_size * bytes_per_frame;
    printf("period buffer size = %lu\n", size);

    void *buffer = malloc(size);

    /* uint64_t fill_buffer_us = 0; */
    /* uint64_t pcm_write_us   = 0; */
    /* int      n_measurements = 0; */

    snd_seq_t *seq_handle;
    /* CHK(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0), "Failed to open sequencer"); */
    CHK(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK), "Failed to open sequencer");

    CHK(snd_seq_set_client_name(seq_handle, "ksynth"), "Failed to set client name");

    int port = snd_seq_create_simple_port(seq_handle,
                                          "ksynth:in",
                                          SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                          SND_SEQ_PORT_TYPE_SYNTHESIZER);
    /* SND_SEQ_PORT_TYPE_APPLICATION); */
    CHK(port, "Failed to create port");

    Synth synth = {};
    synth_init(&synth, SAMPLERATE, N_CHANNELS);


    printf("Starting main loop\n");

    /* int seq_nfds = snd_seq_poll_descriptors_count(seq_handle, POLLIN); */
    /* int pcm_nfds = snd_pcm_poll_descriptors_count (pcm_handle); */

    /* struct pollfd *pfds = (struct pollfd *)alloca(sizeof(struct pollfd) * (seq_nfds + pcm_nfds)); */
    /* struct pollfd *pfds = (struct pollfd *)alloca(sizeof(struct pollfd) * (seq_nfds + pcm_nfds)); */
    /* snd_seq_poll_descriptors(seq_handle, pfds, seq_nfds, POLLIN); */
    /* snd_pcm_poll_descriptors (pcm_handle, pfds+seq_nfds, pcm_nfds); */

    /* while (1) { */
    /*     if (poll (pfds, seq_nfds + pcm_nfds, 1000) > 0) { */
    /*         for (int l1 = 0; l1 < seq_nfds; l1++) { */
    /*             if (pfds[l1].revents > 0) { */
    /*                 midi_callback(); */
    /*             } */
    /*         } */
    /*         for (int l1 = seq_nfds; l1 < seq_nfds + pcm_nfds; l1++) {     */
    /*             if (pfds[l1].revents > 0) { */
    /*                 int n_processed = playback_callback(BUFSIZE); */
    /*                 if (n_processed < BUFSIZE) { */
    /*                     fprintf (stderr, "xrun !\n"); */
    /*                     snd_pcm_prepare(pcm_handle); */
    /*                 } */
    /*             } */
    /*         }         */
    /*     } */
    /* } */

    /* int note = 35; */
    /* synth_handle_note(&synth, 1, midi_freq_table[note]); */
    while (1) {

        /* if(i%128 == 0) { */

        /*     note++; */
        /*     printf("New note: %3d\n", note); */
        /*     synth_handle_note(&synth, 1, midi_freq_table[note]); */
        /* } */
        /* i++; */
        /* int banan = snd_seq_event_input_pending(seq_handle, 0); */

        snd_seq_event_t *ev = NULL;
        /* CHK(snd_seq_event_input(seq_handle, &ev), "Failed to get input event"); */
        int ret = snd_seq_event_input(seq_handle, &ev);
        if (ret == -EAGAIN) {
            /* printf("Got EAGAIN\n"); */
            /* usleep(100000); */
            /* continue; */
        } else if (ret < 0) {
            printf("Got some error: %d\n", ret);
        } else {

            switch (ev->type) {
                case SND_SEQ_EVENT_NOTEON:
                    printf("Note On   - ch=%d note=%d vel=%d\n",
                           ev->data.note.channel,
                           ev->data.note.note,
                           ev->data.note.velocity);
                    synth_handle_note(&synth, 1, ev->data.note.note);
                    break;
                case SND_SEQ_EVENT_NOTEOFF:
                    printf("Note Off  - ch=%d note=%d vel=%d\n",
                           ev->data.note.channel,
                           ev->data.note.note,
                           ev->data.note.velocity);
                    synth_handle_note(&synth, 0,ev->data.note.note);
                    break;
                case SND_SEQ_EVENT_CONTROLLER:
                    printf("Control   - ch=%d param=%d value=%d\n",
                           ev->data.control.channel,
                           ev->data.control.param,
                           ev->data.control.value);
                    break;
                case SND_SEQ_EVENT_PGMCHANGE:
                    printf("PgmChange - ch=%d param=%d value=%d\n",
                           ev->data.control.channel,
                           ev->data.control.param,
                           ev->data.control.value);
                    break;
                case SND_SEQ_EVENT_PITCHBEND:
                    {
                        float fval = ev->data.control.value/(float)(1<<13);
                        printf("Pitchbend - ch=%d param=%d value=%d  fval=%f\n",
                               ev->data.control.channel,
                               ev->data.control.param,
                               ev->data.control.value,
                               fval);
                        synth_handle_bend(&synth,fval); 
                    }
                    break;
                case SND_SEQ_EVENT_PORT_SUBSCRIBED:
                    printf("Subscribe - dest=%d:%d sender=%d:%d\n",
                           ev->data.connect.dest.client,
                           ev->data.connect.dest.port,
                           ev->data.connect.sender.client,
                           ev->data.connect.sender.port);
                    break;
                default:
                    printf("Unknown   - type=%d\n", ev->type);
                    break;
            }
        }
        /* #if 0 */
        /* struct timespec t1, t2, t3; */
        /* clock_gettime(CLOCK_REALTIME, &t1); */

        synth_fill_buffer(&synth, buffer, period_size);
        /* clock_gettime(CLOCK_REALTIME, &t2); */

        int rc = snd_pcm_writei(pcm_handle, buffer, period_size);
        /* clock_gettime(CLOCK_REALTIME, &t3); */

        if (rc == -EPIPE) {
            /* EPIPE means underrun */
            fprintf(stderr, "underrun occurred\n");
            snd_pcm_prepare(pcm_handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
        } else if (rc != (int)period_size) {
            fprintf(stderr, "short write, write %d frames\n", rc);
        }

        /* fill_buffer_us += (t2.tv_sec * 1000000 + t2.tv_nsec / 1000) - (t1.tv_sec * 1000000 + t1.tv_nsec / 1000); */
        /* pcm_write_us += (t3.tv_sec * 1000000 + t3.tv_nsec / 1000) - (t2.tv_sec * 1000000 + t2.tv_nsec / 1000); */
        /* n_measurements++; */

        /* if ((fill_buffer_us + pcm_write_us) > 1000000) { */
        /*     printf("Fill buffer: %f us\n", (float)(fill_buffer_us) / n_measurements); */
        /*     printf("PCM write:   %f us\n", (float)(pcm_write_us) / n_measurements); */

        /*     fill_buffer_us = 0; */
        /*     pcm_write_us   = 0; */
        /*     n_measurements = 0; */
        /* } */
        /* #endif */
    }

    free(buffer);

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    snd_seq_close(seq_handle);

    return 0;
}

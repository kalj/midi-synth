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
#define N_CHANNELS 1

#if defined(SAMPLE_TYPE_F32)
#define DESIRED_PCM_FORMAT SND_PCM_FORMAT_FLOAT_LE
#elif defined(SAMPLE_TYPE_I8)
#define DESIRED_PCM_FORMAT SND_PCM_FORMAT_S8
#elif defined(SAMPLE_TYPE_I16)
#define DESIRED_PCM_FORMAT SND_PCM_FORMAT_S16_LE
#elif defined(SAMPLE_TYPE_I32)
#define DESIRED_PCM_FORMAT SND_PCM_FORMAT_S32_LE
#endif

snd_pcm_t        *pcm_handle;
snd_seq_t        *seq_handle;
snd_pcm_uframes_t period_size;

int pcm_init()
{

    CHK(snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0), "Failed to open PCM");

    /**
     * Set parameters
     */

    snd_pcm_hw_params_t *params;
    snd_pcm_hw_params_alloca(&params);
#if 0
    CHK(snd_pcm_hw_params_any(pcm_handle, params), "Failed in params_any");

    CHK(snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED), "Can't set interleaved mode");

    CHK(snd_pcm_hw_params_set_format(pcm_handle, params, DESIRED_PCM_FORMAT), "Can't set format");
    CHK(snd_pcm_hw_params_set_channels(pcm_handle, params, N_CHANNELS), "Can't set channels number");

    unsigned int requested_rate = SAMPLERATE;
    CHK(snd_pcm_hw_params_set_rate_near(pcm_handle, params, &requested_rate, 0), "Can't set rate");

    CHK(snd_pcm_hw_params_set_period_size(pcm_handle, params, 64, 0), "Can't set period size");
    CHK(snd_pcm_hw_params_set_periods(pcm_handle, params, 8, 0), "Can't set periods");

    CHK(snd_pcm_hw_params(pcm_handle, params), "Can't set harware parameters");
#else
    CHK(snd_pcm_set_params(pcm_handle,
                           DESIRED_PCM_FORMAT,
                           SND_PCM_ACCESS_RW_INTERLEAVED,
                           N_CHANNELS,
                           SAMPLERATE,
                           0,
                           20000),
        "Failed setting parameters");
    CHK(snd_pcm_hw_params_current(pcm_handle, params), "Failed to get current hw params");
    // 20ms -> 220x4 periods
    // 10ms -> 110x4 periods
    // 5ms -> 55x4 periods
#endif

    unsigned int n_channels;
    unsigned int rate;
    CHK(snd_pcm_hw_params_get_channels(params, &n_channels), "Failed to get number of params");
    CHK(snd_pcm_hw_params_get_rate(params, &rate, 0), "Failed to get rate");
    printf("channels           = %u\n", n_channels);
    printf("rate               = %d Hz\n", rate);

    snd_pcm_format_t fmt;
    CHK(snd_pcm_hw_params_get_format(params, &fmt), "Failed to get format");
    int bits_per_sample = snd_pcm_format_width(fmt);

    printf("format             = '%s' (%s) bits:%d\n",
           snd_pcm_format_name(fmt),
           snd_pcm_format_description(fmt),
           bits_per_sample);

    unsigned int period_time;
    CHK(snd_pcm_hw_params_get_period_time(params, &period_time, NULL), "Failed to get period time");
    CHK(snd_pcm_hw_params_get_period_size(params, &period_size, NULL), "Failed to get period size");
    printf("period:            = %lu frames (%d us)\n", period_size, period_time);

    snd_pcm_uframes_t buffer_size;
    unsigned int      periods;
    unsigned int      buffer_time;
    CHK(snd_pcm_hw_params_get_buffer_time(params, &buffer_time, NULL), "Failed to get buffer time");
    CHK(snd_pcm_hw_params_get_periods(params, &periods, NULL), "Failed to get periods");
    CHK(snd_pcm_hw_params_get_buffer_size(params, &buffer_size), "Failed to get buffer size");
    printf("buffer size        = %lu frames (%d us, %d periods)\n", buffer_size, buffer_time, periods);

    return 0;
}

int midi_init()
{
    /* CHK(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0), "Failed to open sequencer"); */
    CHK(snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK), "Failed to open sequencer");

    CHK(snd_seq_set_client_name(seq_handle, "ksynth"), "Failed to set client name");

    int port = snd_seq_create_simple_port(seq_handle,
                                          "ksynth:in",
                                          SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                          SND_SEQ_PORT_TYPE_SYNTHESIZER);
    /* SND_SEQ_PORT_TYPE_APPLICATION); */
    CHK(port, "Failed to create port");

    return 0;
}

void handle_control_event(Synth *synth, snd_seq_ev_ctrl_t *ctrl)
{
    switch (ctrl->param) {
        case 44:
            synth->env.attack = ctrl->value / 127.0f;
            break;
        case 45:
            synth->env.decay = ctrl->value / 127.0f;
            break;
        case 46:
            synth->env.sustain = ctrl->value / 127.0f;
            break;
        case 47:
            synth->env.release = ctrl->value / 127.0f;
            break;
        case 62: {
            float cutoff =
                FILTER_CUTOFF_MIN + ctrl->value * (FILTER_CUTOFF_MAX - FILTER_CUTOFF_MIN) / 127; // mapped to 20 - 2000
            filter_init(&synth->flt, cutoff);
        } break;
    }
}
void handle_midi_event(Synth *synth, snd_seq_event_t *ev)
{
    // 30 - SND_SEQ_EVENT_START
    // 32 - SND_SEQ_EVENT_STOP
    // 36 - SND_SEQ_EVENT_CLOCK

    switch (ev->type) {
        case SND_SEQ_EVENT_NOTEON:
            printf("Note On   - ch=%d note=%d vel=%d\n",
                   ev->data.note.channel,
                   ev->data.note.note,
                   ev->data.note.velocity);
            synth_handle_note(synth, 1, ev->data.note.note);
            break;
        case SND_SEQ_EVENT_NOTEOFF:
            printf("Note Off  - ch=%d note=%d vel=%d\n",
                   ev->data.note.channel,
                   ev->data.note.note,
                   ev->data.note.velocity);
            synth_handle_note(synth, 0, ev->data.note.note);
            break;
        case SND_SEQ_EVENT_CONTROLLER:
            printf("Control   - ch=%d param=%d value=%d\n",
                   ev->data.control.channel,
                   ev->data.control.param,
                   ev->data.control.value);
            handle_control_event(synth, &ev->data.control);
            break;
        case SND_SEQ_EVENT_PGMCHANGE:
            printf("PgmChange - ch=%d param=%d value=%d\n",
                   ev->data.control.channel,
                   ev->data.control.param,
                   ev->data.control.value);
            break;
        case SND_SEQ_EVENT_PITCHBEND: {
            float fval = ev->data.control.value / (float)(1 << 13);
            printf("Pitchbend - ch=%d param=%d value=%d  fval=%f\n",
                   ev->data.control.channel,
                   ev->data.control.param,
                   ev->data.control.value,
                   fval);
            synth_handle_bend(synth, fval);
        } break;
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

int main()
{
    pcm_init();

    size_t bytes_per_frame = N_CHANNELS * sizeof(sample_t);
    size_t size            = period_size * bytes_per_frame;
    void  *buffer          = malloc(size);

    midi_init();

    Synth synth = {};
    synth_init(&synth, 0.1, 0.5, 0.5, 0.1);

    printf("Starting main loop\n");

    while (1) {

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
            handle_midi_event(&synth, ev);
        }

        synth_process(&synth, buffer, period_size);

        int rc = snd_pcm_writei(pcm_handle, buffer, period_size);

        if (rc == -EPIPE) {
            /* EPIPE means underrun */
            fprintf(stderr, "underrun occurred\n");
            snd_pcm_prepare(pcm_handle);
        } else if (rc < 0) {
            fprintf(stderr, "error from writei: %s\n", snd_strerror(rc));
        } else if (rc != (int)period_size) {
            fprintf(stderr, "short write, write %d frames\n", rc);
        }
    }

    free(buffer);

    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    snd_seq_close(seq_handle);

    return 0;
}

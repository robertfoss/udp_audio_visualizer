#include <pthread.h>
#include <semaphore.h>

#include "animate_process.h"
#include "audio_process.h"
#include "kiss_fft.h"
#include "util.h"

//#define LOG_INFO 1
//#define LOG_WARN 2
//#define LOG_ERR  4
//#ifndef LOG_LEVELS
//#define LOG_LEVELS (LOG_INFO | LOG_WARN | LOG_ERR)
#define LOG_LEVELS (LOG_WARN | LOG_ERR)
#include "log.h"


typedef struct {
    PULSEAUDIO_SAMPLE_TYPE *l;
    PULSEAUDIO_SAMPLE_TYPE *r;
    size_t len;
} stereo_samples_t;

static sem_t data_sem;
static kiss_fft_cfg fft_state_left;
static kiss_fft_cfg fft_state_right;

static stereo_samples_t input_buf[AUDIO_PROCESS_QUEUE_LEN];
static pthread_mutex_t buf_mutex;
static uint32_t buf_idx_write;
static uint32_t buf_size;


static void buf_commit_write()
{
    pthread_mutex_lock(&buf_mutex);
    buf_idx_write = (buf_idx_write + 1) % AUDIO_PROCESS_QUEUE_LEN;
    buf_size++;
    pthread_mutex_unlock(&buf_mutex);
}


static void buf_add(stereo_samples_t samples)
{
    pthread_mutex_lock(&buf_mutex);

    if (buf_size <= 0)
    {
        buf_idx_write = 0;
    }

    if (buf_size >= AUDIO_PROCESS_QUEUE_LEN)
    {
        free(input_buf[buf_idx_write].l);
        free(input_buf[buf_idx_write].r);
        buf_size--;
    }

    input_buf[buf_idx_write] = samples;
    buf_commit_write();

    pthread_mutex_unlock(&buf_mutex);

    sem_post(&data_sem);
}


stereo_samples_t *buf_get_lifo(int32_t idx)
{
    int32_t lifo_idx = buf_idx_write - 1 - idx;
    int r = lifo_idx % AUDIO_PROCESS_QUEUE_LEN;
    if (r < 0) {
        lifo_idx += AUDIO_PROCESS_QUEUE_LEN;
    }

    return &(input_buf[lifo_idx]);
}


static ret_code buf_last_samples(kiss_fft_cpx *l, kiss_fft_cpx *r)
{
    pthread_mutex_lock(&buf_mutex);

    size_t samples_found = 0;

    /* Set i numbers to 0. */
    memset(l, 0, sizeof(kiss_fft_cpx) * AUDIO_PROCESS_FFT_BINS);
    memset(r, 0, sizeof(kiss_fft_cpx) * AUDIO_PROCESS_FFT_BINS);

    for (int i = 0; i < buf_size && samples_found < AUDIO_PROCESS_FFT_BINS; i++)
    {
        stereo_samples_t *sample = buf_get_lifo(i);
        int samples_needed_from_buf = MIN(sample->len, AUDIO_PROCESS_FFT_BINS - samples_found);
        ASSERT(samples_needed_from_buf >= 0);

        for (size_t j = 0; j < samples_needed_from_buf; j++)
        {
            int dst_idx = AUDIO_PROCESS_FFT_BINS - j - samples_found - 1;
            int src_idx = sample->len - j - 1;

            ASSERT(dst_idx >= 0);
            ASSERT(dst_idx < AUDIO_PROCESS_FFT_BINS);
            ASSERT(src_idx >= 0);
            ASSERT(src_idx < (uint32_t) sample->len);

            l[dst_idx].r = sample->l[src_idx];
            r[dst_idx].r = sample->r[src_idx];
        }
        samples_found += sample->len;
    }

    pthread_mutex_unlock(&buf_mutex);

    if (samples_found < AUDIO_PROCESS_FFT_BINS)
    {
        return RET_ERR;
    }
    return RET_OK;
}


static void audio_process_samples()
{
    kiss_fft_cpx l_fft_in[AUDIO_PROCESS_FFT_BINS];
    kiss_fft_cpx l_fft_out[AUDIO_PROCESS_FFT_BINS];
    kiss_fft_cpx r_fft_in[AUDIO_PROCESS_FFT_BINS];
    kiss_fft_cpx r_fft_out[AUDIO_PROCESS_FFT_BINS];

    ret_code r = buf_last_samples(l_fft_in, r_fft_in);
    if (r != RET_OK)
    {
        log_info("Not have enough samples to run FFT");
        return;
    }

    kiss_fft(fft_state_left,  l_fft_in, l_fft_out);
    kiss_fft(fft_state_right, r_fft_in, r_fft_out);
    
    animate_process_add_fft(l_fft_out, r_fft_out);
}


static void *audio_process_thread(void *args)
{
    UNUSED(args);

    while (1)
    {
        sem_wait(&data_sem);
        log_infof("Sample received, sample buffers enqueued: %u", buf_size);

        audio_process_samples();
    }

    return 0;
}


void audio_process_add_samples(PULSEAUDIO_SAMPLE_TYPE *data_left, PULSEAUDIO_SAMPLE_TYPE *data_right, size_t nbr_channel_samples)
{
    stereo_samples_t samples;
    samples.l = data_left;
    samples.r = data_right;
    samples.len = nbr_channel_samples;

    buf_add(samples);
}


void audio_process_start()
{
    debug();

    fft_state_left  = kiss_fft_alloc(AUDIO_PROCESS_FFT_BINS, 0, NULL, NULL);
    fft_state_right = kiss_fft_alloc(AUDIO_PROCESS_FFT_BINS, 0, NULL, NULL);

    sem_init(&data_sem, 0, 0);

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&buf_mutex, &mutex_attr);

    pulseaudio_init();

    pthread_t thread;
    if (pthread_create(&thread, NULL, &audio_process_thread, NULL) != 0) {
        DIE("Failed to start thread");
    }
}

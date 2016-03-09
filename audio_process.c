#include <pthread.h>
#include <semaphore.h>

#include "audio_process.h"
#include "kiss_fft.h"
#include "util.h"



typedef struct {
    PULSEAUDIO_SAMPLE_TYPE *l;
    PULSEAUDIO_SAMPLE_TYPE *r;
    size_t l_len;
    size_t r_len;
} stereo_samples_t;

static sem_t data_sem;
static kiss_fft_cfg fft_state_left;
static kiss_fft_cfg fft_state_right;

static stereo_samples_t input_buf[AUDIO_PROCESS_QUEUE_LEN];
static pthread_mutex_t buf_mutex;
static uint8_t buf_idx_write;
static uint8_t buf_idx_read;
static uint8_t buf_size;


static void buf_commit_write()
{
    pthread_mutex_lock(&buf_mutex);
    buf_idx_write = (buf_idx_write + 1) % AUDIO_PROCESS_QUEUE_LEN;
    buf_size++;
    pthread_mutex_unlock(&buf_mutex);
}


static void buf_commit_read()
{
    pthread_mutex_lock(&buf_mutex);
    buf_idx_read = (buf_idx_read + 1) % AUDIO_PROCESS_QUEUE_LEN;
	buf_size--;
    pthread_mutex_unlock(&buf_mutex);
}


static void buf_add(stereo_samples_t samples)
{
    pthread_mutex_lock(&buf_mutex);

	if (buf_size <= 0)
	{
		buf_idx_write = 0;
		buf_idx_read = 0;
	}

	if (buf_size >= AUDIO_PROCESS_QUEUE_LEN)
	{
		log_err("buffer full, rejected samples");
		return;
	}

    input_buf[buf_idx_write] = samples;
	buf_commit_write();
	
    pthread_mutex_unlock(&buf_mutex);

    sem_post(&data_sem);
}


static stereo_samples_t *buf_pop()
{
    pthread_mutex_lock(&buf_mutex);

    if (buf_size == 0)
    {
        return NULL;
    }

    stereo_samples_t *sample = malloc(sizeof(stereo_samples_t));
    if (sample == NULL)
    {
        return NULL;
    }
    *sample = input_buf[buf_idx_read];

    buf_commit_read();
    pthread_mutex_unlock(&buf_mutex);

    return sample;
}


static void *audio_process_thread(void *args)
{
    UNUSED(args);

    while(1)
    {
        sem_wait(&data_sem);
        log_infof("Sample received, sample buffers enqueued: %u", buf_size);

        stereo_samples_t *samples = buf_pop();
        if (samples == NULL)
        {
            continue;
        }

        free(samples->l);
        free(samples->r);
        free(samples);
    }

    return 0;
}


void audio_process_add_samples(PULSEAUDIO_SAMPLE_TYPE *data_left, PULSEAUDIO_SAMPLE_TYPE *data_right, size_t nbr_channel_samples)
{
    UNUSED(nbr_channel_samples);

    stereo_samples_t samples;
    samples.l = data_left;
    samples.l_len = nbr_channel_samples;
    samples.r = data_right;
    samples.r_len = nbr_channel_samples;

    buf_add(samples);
}


void audio_process_start()
{
    debug();

    fft_state_left  = kiss_fft_alloc(1024, 0, NULL, NULL);
    fft_state_right = kiss_fft_alloc(1024, 0, NULL, NULL);

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

#include <pthread.h>

#include "audio_process.h"
#include "kiss_fft.h"
#include "util.h"

static void *audio_process_thread(void *args)
{
    UNUSED(args);
    pulseaudio_init();

    return 0;
}


void audio_process_add_samples(size_t nbr_samples, PULSEAUDIO_SAMPLE_TYPE *left_data, PULSEAUDIO_SAMPLE_TYPE *right_data)
{


    free(left_data);
    free(right_data);
}


void audio_process_start()
{
    debug();
    pthread_t thread;
    if (pthread_create(&thread, NULL, &audio_process_thread, NULL) != 0) {
        DIE("Failed to start thread");
    }
}

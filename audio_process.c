#include <pthread.h>

#include "kiss_fft.h"
#include "util.h"

static void *audio_process_thread(void *args)
{
    UNUSED(args);
    audio_pulseaudio_init();

    return 0;
}


void audio_process_add_samples(size_t nbr_samples, int16_t *buf)
{

}


void audio_process_start()
{
    debug();
    pthread_t thread;
    if (pthread_create(&thread, NULL, &audio_process_thread, NULL) != 0) {
        DIE("Failed to start thread");
    }
}

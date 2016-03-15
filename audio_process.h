#ifndef AUDIO_PROCESS_H
#define AUDIO_PROCESS_H

#include <stdint.h>

#include "pulseaudio.h"


#define AUDIO_PROCESS_SAMPLE_RATE    48000
#define AUDIO_PROCESS_FFT_BINS       4096
#define AUDIO_PROCESS_TARGET_LATENCY 15  // In msec
#define AUDIO_PROCESS_FFTS_PER_SEC   DIV_ROUND(1000, AUDIO_PROCESS_TARGET_LATENCY)
#define AUDIO_PROCESS_QUEUE_LEN      (4 * 1000 / AUDIO_PROCESS_TARGET_LATENCY)


void audio_process_add_samples(PULSEAUDIO_SAMPLE_TYPE *data_left, PULSEAUDIO_SAMPLE_TYPE *data_right, size_t nbr_channel_samples);
void audio_process_start();


#endif//AUDIO_PROCESS_H

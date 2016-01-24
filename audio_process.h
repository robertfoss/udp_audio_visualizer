#ifndef AUDIO_PROCESS_H
#define AUDIO_PROCESS_H

#include <stdint.h>

#include "pulseaudio.h"


#define AUDIO_PROCESS_SAMPLE_RATE 44100
#define AUDIO_PROCESS_TARGET_LATENCY 10  // In msec
#define AUDIO_PROCESS_FFTS_PER_SEC DIV_ROUND(10, 1000)

#define PULSEAUDIO_NBR_CHANNELS      2
#define PULSEAUDIO_FORMAT            PA_SAMPLE_S16LE
#define PULSEAUDIO_SAMPLE_TYPE       int16_t
#define PULSEAUDIO_SAMPLE_BYTES      2

#define PULSEAUDIO_CHUNK_BYTES      (PULSEAUDIO_NBR_CHANNELS * PULSEAUDIO_SAMPLE_BYTES)
#define PULSEAUDIO_BUFSIZE           DIV_ROUND((AUDIO_PROCESS_SAMPLE_RATE * PULSEAUDIO_SAMPLE_BYTES), AUDIO_PROCESS_FFTS_PER_SEC)


void audio_process_add_samples(size_t nbr_samples, PULSEAUDIO_SAMPLE_TYPE *data_left, PULSEAUDIO_SAMPLE_TYPE *data_right);
void audio_process_start();


#endif//AUDIO_PROCESS_H

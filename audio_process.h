#ifndef AUDIO_PROCESS_H
#define AUDIO_PROCESS_H


#include <stdint.h>


#define AUDIO_PROCESS_SAMPLE_RATE 44100
#define AUDIO_PROCESS_TARGET_LATENCY 10  // In msec
#define AUDIO_PROCESS_FFTS_PER_SEC DIV_ROUND(10, 1000)


void audio_process_add_samples(size_t nbr_samples, int16_t *data_left, int16_t *data_right);
void audio_process_start();

#endif//AUDIO_PROCESS_H

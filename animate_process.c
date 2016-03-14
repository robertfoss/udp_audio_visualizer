#include "animate_process.h"
#include "audio_process.h"
#include "kiss_fft.h"
#include "log.h"


static void animate_process_calc_intensity(uint16_t *intensity, kiss_fft_cpx *fft)
{
    for (int i = 0; i < AUDIO_PROCESS_FFT_BINS; i++)
    {
        uint16_t magnitude = (uint16_t) sqrt(fft[i].r * fft[i].r + fft[i].i * fft[i].i);
        intensity[i] = magnitude;
    }
}


static uint16_t animate_process_fft_bin_to_hz(int idx)
{
    return idx * AUDIO_PROCESS_SAMPLE_RATE / AUDIO_PROCESS_FFT_BINS;
}


void animate_process_add_fft(kiss_fft_cpx *l_fft, kiss_fft_cpx *r_fft)
{
    //TODO: Copy and store input fft, keep buffer of last 1 sec to allow for averages and musical analysis
    uint16_t intensity[AUDIO_PROCESS_FFT_BINS];
    animate_process_calc_intensity(intensity, l_fft);
    
    uint16_t max_val = 0;
    int max_idx = 0;
    for (int i = 0; i < AUDIO_PROCESS_FFT_BINS; i++)
    {
        if (intensity[i] > max_val)
        {
            max_val = intensity[i];
            max_idx = i;
        }
    }
    log_infof("Loudest frequency: %u", animate_process_fft_bin_to_hz(max_idx));
}

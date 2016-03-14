#ifndef ANIMATE_PROCESS_H
#define ANIMATE_PROCESS_H


#include "kiss_fft.h"
#include "util.h"


ret_code animate_process_start();
void animate_process_add_fft(kiss_fft_cpx *l_fft, kiss_fft_cpx *r_fft);

#endif//ANIMATE_PROCESS_H


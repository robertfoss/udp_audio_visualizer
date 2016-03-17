#ifndef UTIL_H
#define UTIL_H

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t ret_code;
#define RET_OK  (ret_code) 0
#define RET_ERR (ret_code) 1

#define DIV_ROUND(top, bottom) ((top + bottom - 1) / (bottom))

#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) \
    __extension__({ typeof (a) _a = (a), _b = (b); _a > _b ? _a : _b; })

#ifdef MIN
#undef MIN
#endif
#define MIN(a,b) \
    __extension__({ typeof (a) _a = (a), _b = (b); _a < _b ? _a : _b; })

#define UNUSED(x) (void)(x)


/* Returns time in milliseconds. */
uint64_t util_time_now();


#endif//UTIL_H

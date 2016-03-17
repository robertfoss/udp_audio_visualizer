#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "log.h"
#include "util.h"


uint64_t util_time_now()
{
    struct timespec tms;

    if (! timespec_get(&tms, TIME_UTC)) {
        DIE("Failed to get time");
    }

    return tms.tv_sec*1000 + tms.tv_nsec/(1000*1000);
}

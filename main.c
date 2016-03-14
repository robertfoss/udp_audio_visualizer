#include <stdio.h>
#include <time.h>

#include "audio_process.h"
#include "udp_listener.h"
#include "udp_sender.h"
#include "sink_manager.h"

//#define LOG_INFO 1
//#define LOG_WARN 2
//#define LOG_ERR  4
//#ifndef LOG_LEVELS
//#define LOG_LEVELS (LOG_INFO | LOG_WARN | LOG_ERR)
#define LOG_LEVELS (LOG_INFO | LOG_WARN | LOG_ERR)
#include "log.h"


int main()
{
    udp_listener_start();
    audio_process_start();

    struct timespec time_sleep;
    time_sleep.tv_sec = 0;
    time_sleep.tv_nsec = 5 * 10 * 1000 * 1000;

    while (1) {
        sink_manager_list_t *list = sink_manager_get_list();
        for (int i = 0; i < list->nbr_sinks; i++) {
            unsigned char txt[] = "0A0A0A0A0A0A0A0A0A";
            udp_sender_send(list->sinks[i], (uint8_t *) txt);
        }
        nanosleep(&time_sleep, NULL);
        sink_manager_free_list(list);
    }
    debug();
}

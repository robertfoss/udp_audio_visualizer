#include <stdio.h>

#include "audio_process.h"
#include "udp_listener.h"
#include "udp_sender.h"
#include "sink_manager.h"


int main()
{
    sink_manager_init();
    udp_listener_start();
    audio_process_start();

    while (1) {
        sink_manager_list_t *list = sink_manager_get_list();
        for (int i = 0; i < list->nbr_sinks; i++) {
            unsigned char txt[] = "0A0A0A0A0A0A0A0A0A";
            udp_sender_send(list->sinks[i], (uint8_t *) &txt);
        }
        sink_manager_free_list(list);
    }
    debug();
}

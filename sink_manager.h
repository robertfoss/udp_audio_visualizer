#ifndef SINK_MANAGER_H
#define SINK_MANAGER_H

#include <stdint.h>
#include <sys/time.h>

#include "sink.h"


#define SINK_MANAGER_INIT_ALLOC_SINKS  10
#define SINK_MANAGER_SINK_TIMEOUT      45


typedef struct sink_manager_list_t {
    sink_t   *sinks;
    uint16_t  nbr_sinks;
    uint16_t  nbr_sinks_allocated;
} sink_manager_list_t;


void sink_manager_init();
void sink_manager_heartbeat(ip_t ip, uint8_t *buf);
sink_manager_list_t *sink_manager_get_list();
void sink_manager_free_list(sink_manager_list_t *list);
void sink_manager_print_sinks();


#endif//SINK_MANAGER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "sink_manager.h"
#include "util.h"


sink_manager_list_t list;
pthread_mutex_t     w_mutex;


static void print_sinks()
{
    printf("Alive sinks:\n");
    for (int i = 0; i < list.nbr_sinks; i++) {
        printf("  %s\n", inet_ntoa(list.sinks[i].ip));
    }
}


static void add_sink(sink_t sink)
{
    log_infof("Adding new sink: %s", inet_ntoa(sink.ip));
    pthread_mutex_lock(&w_mutex);

    if (list.nbr_sinks < list.nbr_sinks_allocated) {
        list.sinks[list.nbr_sinks++] = sink;
    } else {
        uint32_t new_sink_space = list.nbr_sinks_allocated * 2;
        log_infof("List of sinks is full, growing list to have space for %u entries.", new_sink_space);
        list.sinks = realloc(list.sinks, new_sink_space);
        if (list.sinks == NULL) {
            DIE("Unable to allocate more space for sinks");
        }
        list.sinks[list.nbr_sinks++] = sink;
    }
    pthread_mutex_unlock(&w_mutex);

    print_sinks();
}


static void remove_sink(uint32_t idx)
{
    pthread_mutex_lock(&w_mutex);
    if (list.nbr_sinks <= 0) {
        return;
    }

    uint32_t sinks_to_move = list.nbr_sinks - idx;
    list.nbr_sinks--;
    if (sinks_to_move == 0) {
        return;
    }

    memmove(&(list.sinks[idx]), &(list.sinks[idx + 1]), sinks_to_move * sizeof(sink_t));
    pthread_mutex_unlock(&w_mutex);

    print_sinks();
}


static void purge_dead_sinks()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    for (int i = list.nbr_sinks; i >= 0; i--) {
        if (tv.tv_sec - list.sinks[i].last_heartbeat > SINK_MANAGER_SINK_TIMEOUT) {
            remove_sink(i);
        }
    }
}


static sink_t* get_sink(ip_t ip)
{
    sink_t *sink = NULL;
    for (int i = 0; i < list.nbr_sinks; i++) {
        if (memcmp(&(list.sinks[i].ip), &ip, sizeof(ip_t)) == 0) {
            sink = &(list.sinks[i]);
            break;
        }
    }
    return sink;
}


void sink_manager_init()
{
    debug();
    list.sinks = malloc(sizeof(sink_t) * SINK_MANAGER_INIT_ALLOC_SINKS);
    list.nbr_sinks_allocated = SINK_MANAGER_INIT_ALLOC_SINKS;

    if (list.sinks == NULL) {
        DIE("Malloc failed");
    }

    if (pthread_mutex_init(&w_mutex, NULL) != 0) {
        DIE("Unable to initialize mutex");
    }
}


void sink_manager_heartbeat(ip_t ip, uint8_t *buf)
{
    sink_t *sink = get_sink(ip);
    struct timeval tv;
    gettimeofday(&tv, NULL);

    if (sink != NULL) {
        sink->last_heartbeat = tv.tv_sec;
    } else {
        sink_t new_sink;
        if (sink_initialize(&new_sink, ip, buf) != RET_OK) {
            return;
        }
        new_sink.last_heartbeat = tv.tv_sec;
        add_sink(new_sink);
    }
}


sink_manager_list_t *sink_manager_get_list()
{
    pthread_mutex_lock(&w_mutex);
    sink_manager_list_t *ret_list = (sink_manager_list_t *) malloc(sizeof(sink_manager_list_t));
    ASSERT(ret_list != NULL);

    *ret_list = list;
    ret_list->sinks = malloc(sizeof(sink_t) * list.nbr_sinks);
    ASSERT(ret_list->sinks);
    memcpy(ret_list->sinks, list.sinks, sizeof(sink_t) * list.nbr_sinks);

    pthread_mutex_unlock(&w_mutex);

    return ret_list;
}


void sink_manager_free_list(sink_manager_list_t *list)
{
    free(list->sinks);
    free(list);
}

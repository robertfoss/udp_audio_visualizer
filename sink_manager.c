#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "sink_manager.h"
#include "util.h"

//#define LOG_INFO 1
//#define LOG_WARN 2
//#define LOG_ERR  4
//#ifndef LOG_LEVELS
//#define LOG_LEVELS (LOG_INFO | LOG_WARN | LOG_ERR)
#define LOG_LEVELS (LOG_INFO | LOG_WARN | LOG_ERR)
#include "log.h"


static sink_manager_list_t list;
static pthread_mutex_t     w_mutex;


void sink_manager_print_sinks()
{
    uint64_t now = util_time_now();

    if (list.nbr_sinks > 0) {
        log_info("Alive sinks:");
    }

    for (int i = 0; i < list.nbr_sinks; i++)
    {
        log_infof("  %s - last heartbeast %lu\n", inet_ntoa(list.sinks[i].ip), (now - list.sinks[i].last_heartbeat) / 1000);
    }
}


static void add_sink(sink_t sink)
{
    log_infof("Adding new sink: %s", inet_ntoa(sink.ip));
    pthread_mutex_lock(&w_mutex);

    if (list.nbr_sinks < list.nbr_sinks_allocated)
    {
        list.sinks[list.nbr_sinks++] = sink;
    }
    else
    {
        uint32_t new_sink_space = list.nbr_sinks_allocated * 2;
        log_infof("List of sinks is full, growing list to have space for %u entries.", new_sink_space);
        list.sinks = realloc(list.sinks, new_sink_space);
        if (list.sinks == NULL)
        {
            DIE("Unable to allocate more space for sinks");
        }
        list.sinks[list.nbr_sinks++] = sink;
    }
    pthread_mutex_unlock(&w_mutex);

    sink_manager_print_sinks();
}


static void remove_sink(uint32_t idx)
{
    pthread_mutex_lock(&w_mutex);
    if (list.nbr_sinks <= 0)
    {
        return;
    }

    uint32_t sinks_to_move = list.nbr_sinks - idx;
    list.nbr_sinks--;
    if (sinks_to_move == 0)
    {
        return;
    }

    memmove(&(list.sinks[idx]), &(list.sinks[idx + 1]), sinks_to_move * sizeof(sink_t));
    pthread_mutex_unlock(&w_mutex);

    sink_manager_print_sinks();
}


static void purge_dead_sinks()
{
    uint64_t now = util_time_now();

    for (int i = 0; i < list.nbr_sinks; i++)
    {
        log_infof("Sink #%d has been out of contact for %lu ms", i, (now - list.sinks[i].last_heartbeat) / 1000);
        if (now - list.sinks[i].last_heartbeat > (SINK_MANAGER_SINK_TIMEOUT * 1000))
        {
            remove_sink(i);
            log_infof("  %s - timed out. Uncontacted for %lu ms\n", inet_ntoa(list.sinks[i].ip), (now - list.sinks[i].last_heartbeat) / 1000);
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

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    if (pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
        DIE("Unable to initialize mutex attr");
    }
    if (pthread_mutex_init(&w_mutex, &mutex_attr) != 0) {
        DIE("Unable to initialize mutex");
    }
}


void sink_manager_heartbeat(ip_t ip, uint8_t *buf)
{
    sink_t *sink = get_sink(ip);

    if (sink != NULL) {
        sink->last_heartbeat = util_time_now();
    } else {
        sink_t new_sink;
        if (sink_initialize(&new_sink, ip, buf) != RET_OK) {
            return;
        }
        new_sink.last_heartbeat = util_time_now();
        add_sink(new_sink);
    }
}


sink_manager_list_t *sink_manager_get_list()
{
    pthread_mutex_lock(&w_mutex);

    purge_dead_sinks();
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

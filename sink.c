#include <stdbool.h>

#include "sink.h"
#include "util.h"


static uint32_t sink_check(sink_t *sink)
{
    if (sink->config.port == 0) {
        log_info("New sink has invalid port: %u", sink->config.port);
        return RET_ERR;
    }

    if (sink->config.nbr_leds > 10000) {
        log_info("New sink has too leds: %u", sink->config.nbr_leds);
        return RET_ERR;
    }

    if (sink->config.bytes_per_led > 8) {
        log_info("New sink has too many bytes per led: %u", sink->config.bytes_per_led);
        return RET_ERR;
    }

    return RET_OK;
}


static void sink_print(sink_t *sink)
{
    log_info("Sink: port=%u nbr_leds=%u bytes_per_led=%u", sink->config.port, sink->config.nbr_leds,
             sink->config.bytes_per_led);
}

uint32_t sink_initialize(sink_t *sink, ip_t ip, uint8_t *buf)
{
    sink->config = *(sink_config_t *) buf;
    sink->ip = ip;

    if (sink_check(sink) != RET_OK) {
        return RET_ERR;
    }

    sink_print(sink);

    return RET_OK;
}

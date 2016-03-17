#ifndef SINK_H
#define SINK_H

#include <arpa/inet.h>

#include "stdint.h"


typedef struct in_addr ip_t;


typedef struct __attribute__((__packed__)) sink_config_t {
    uint16_t port;         // UDP port of sink
    uint16_t nbr_leds;     // Number of LEDs in this sink
    uint8_t bytes_per_led; // Number of bytes consumed by each LED
} sink_config_t;


typedef struct sink_t {
    ip_t          ip;              // IP address of sink
    uint64_t      first_heartbeat; // Timestamp of first heartbeat of sink
    uint64_t      last_heartbeat;  // Timestamp of last heartbeat of sink
    sink_config_t config;          // Specific configuration options this sink
} sink_t;


uint32_t sink_initialize(sink_t *sink, ip_t ip, uint8_t *buf);


#endif//SINK_H

#include <sys/socket.h>
#include <unistd.h>

#include "sink.h"
#include "udp_sender.h"
#include "util.h"

//#define LOG_INFO 1
//#define LOG_WARN 2
//#define LOG_ERR  4
//#ifndef LOG_LEVELS
//#define LOG_LEVELS (LOG_INFO | LOG_WARN | LOG_ERR)
#define LOG_LEVELS (LOG_INFO | LOG_WARN | LOG_ERR)
#include "log.h"


ret_code udp_sender_send(sink_t sink, uint8_t *buf)
{
    struct sockaddr_in sock_remote;
    uint32_t sock_len = sizeof(struct sockaddr_in);

    int32_t s;
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        DIE("Unable to create socket");
    }

    memset((char *) &sock_remote, 0, sock_len);
    sock_remote.sin_family = AF_INET;
    sock_remote.sin_port = htons(sink.config.port);
    sock_remote.sin_addr = sink.ip;

    if (sendto(s, buf, sink.config.nbr_leds * sink.config.bytes_per_led, 0, (struct sockaddr *) &sock_remote, sock_len) == -1) {
        DIE("send() failed");
    }

    close(s);
    return RET_OK;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
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

#define UDP_LISTENER_BUFLEN 128
#define UDP_LISTENER_PORT   10000

static void *udp_listener_listen(void *args)
{
    UNUSED(args);

    uint8_t buf[UDP_LISTENER_BUFLEN];
    struct sockaddr_in sock_local, sock_remote;

    int32_t s, recv_len;
    uint32_t slen = sizeof(sock_remote);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        DIE("Unable to create socket");
    }

    memset((char *) &sock_local, 0, sizeof(sock_local));

    sock_local.sin_family = AF_INET;
    sock_local.sin_port = htons(UDP_LISTENER_PORT);
    sock_local.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr *) &sock_local, sizeof(sock_local)) == -1)
    {
        DIE("Unable to bind socket");
    }

    while (1)
    {
        if ((recv_len = recvfrom(s, buf, UDP_LISTENER_BUFLEN, 0, (struct sockaddr *) &sock_remote, &slen)) == -1)
        {
            DIE("recvfrom() failed");
        }
        sink_manager_heartbeat(sock_remote.sin_addr, buf);
    }

    return NULL;
}


void udp_listener_start()
{
    debug();

    sink_manager_init();

    pthread_t thread;
    if (pthread_create(&thread, NULL, &udp_listener_listen, NULL) != 0) {
        DIE("Failed to start thread");
    }
}


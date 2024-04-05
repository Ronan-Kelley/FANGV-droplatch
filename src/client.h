#ifndef DROPLATCH_CLIENT_H_HGUARD
#define DROPLATCH_CLIENT_H_HGUARD

#include <arpa/inet.h> // sockaddr_in

/**
 * client configuration values; should not change after
 * startup
 */
typedef struct client_config_t
{
    int port;
    char* ip;
} client_config;

typedef struct client_values_t
{
    int sockfd;
    struct sockaddr_in serveraddr;
} client_values;

#endif

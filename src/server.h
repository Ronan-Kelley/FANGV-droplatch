#ifndef DROPLATCH_SERVER_H_HGUARD
#define DROPLATCH_SERVER_H_HGUARD

#include <netinet/in.h> // sockaddr_in
#include <stdbool.h>    // bool

/**
 *  server configuration values; should not change after startup
 */
typedef struct server_config_t
{
    int port;
    char* ip;
} server_config;

/**
 * server values - used to maintain the state of the server
 */
typedef struct server_values_t
{
    int sockfd;
    struct sockaddr_in addr;
    bool open;
} server_values;

#endif

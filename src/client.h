#ifndef DROPLATCH_CLIENT_H_HGUARD
#define DROPLATCH_CLIENT_H_HGUARD

/**
 * client configuration values; should not change after
 * startup
 */
typedef struct client_config_t
{
    int port;
    char* ip;
} client_config;

#endif

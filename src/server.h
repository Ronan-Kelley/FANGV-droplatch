#ifndef DROPLATCH_SERVER_H_HGUARD
#define DROPLATCH_SERVER_H_HGUARD

#include <netinet/in.h> // sockaddr_in
#include <stdbool.h>    // bool

typedef struct server_values_t
{
    int sockfd;
    struct sockaddr_in addr;
    bool open;
} server_values;

#endif

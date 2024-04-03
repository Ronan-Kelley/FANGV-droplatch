#include "server.h"
#include "server_config.h"

#include <arpa/inet.h>      // inet_addr()
#include <errno.h>          // errno
#include <netinet/in.h>     // sockaddr_in
#include <stdio.h>          // fprintf()
#include <stdlib.h>         // exit()
#include <string.h>         // memset()
#include <sys/socket.h>     // socket()
#include <unistd.h>         // close(), open(), read(), write()

/**
 * initialize the server based on the config, setting up values
 * to be a server_values struct with a valid state
 *
 * TODO return a more accessible constant on failure instead of
 *      returning incrementing numbers
 */
int init_server(server_config* config, server_values* values)
{
    // set up the socket
    values->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // check for errors in setting up the socket
    if (values->sockfd == -1)
    {
        fprintf(stderr, "socket creation failed!\n");
        return 1;
    }

    // zero the addr struct
    memset(&values->addr, 0, sizeof(values->addr));

    // TCP/UDP sockets always have AF_INET for their sin_family value
    values->addr.sin_family = AF_INET;
    // set ip
    values->addr.sin_addr.s_addr = inet_addr(config->ip);
    // set port
    values->addr.sin_port = htons(config->port);

    // TODO remove this, it's just for debugging :^)
    printf(
        "sockfd: %d\nport: %d\nip: %s\nsizeof(values->addr): %lu\n"
        ,values->sockfd
        ,config->port
        ,config->ip
        ,sizeof(values->addr)
    );

    if (connect(values->sockfd, (struct sockaddr*) &values->addr, sizeof(values->addr)) != 0)
    {
        char errName[256];
        strerror_r(errno, errName, sizeof(errName) / sizeof(char));
        fprintf(stderr, "could not connect to socket: %d (%s)\n", errno, errName);
        return 2;
    }

    return 0;
}

int main(int argc, char** argv)
{
    // TODO optionally read port and IP from args
    (void) argc; // both of these casts are compiled out but prevent
    (void) argv; // the compiler from complaining about unused variables

    server_config config;
    server_values values;
    config.port = 8080;
    config.ip = "127.0.0.1";

    if (init_server(&config, &values) != 0)
    {
        exit(1);
    }

    close(values.sockfd);
    
    return 0;
}

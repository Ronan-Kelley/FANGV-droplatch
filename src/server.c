#include "server.h"
#include "client_server_shared.h"

#include <arpa/inet.h>      // inet_addr(), inet_pton()
#include <errno.h>          // errno
#include <netinet/in.h>     // sockaddr_in
#include <poll.h>           // poll(), pollfd
#include <stdio.h>          // fprintf()
#include <stdlib.h>         // exit(), strtol()
#include <string.h>         // memset()
#include <sys/socket.h>     // socket()
#include <unistd.h>         // close(), open(), read(), write(), getopt()

#define MAX_CONN        8

/**
 * initialize the server based on the config, setting up values
 * to be a server_values struct with a valid state
 *
 * TODO return a more accessible constant on failure instead of
 *      returning incrementing numbers
 */
int init_server(server_config* config, server_values* values)
{
    // zero the addr struct
    memset(&values->addr, 0, sizeof(values->addr));

    // TCP/UDP sockets always have AF_INET for their sin_family value
    values->addr.sin_family = AF_INET;
    // set ip
    values->addr.sin_addr.s_addr = inet_addr(config->ip);
    // set port
    values->addr.sin_port = htons(config->port);


    // set up the socket as an internet stream
    values->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // check for errors in setting up the socket
    if (values->sockfd == -1)
    {
        // this software isn't multithreaded as of my writing this, but it ideally will be
        // eventually, so we have to allocate a buffer and use strerror_r() instead of just
        // using str_error() in place.
        char errName[256];
        strerror_r(errno, errName, sizeof(errName) / sizeof(char));
        fprintf(stderr, "socket creation failed: %d (%s)\n", errno, errName);
        return 1;
    }

    // bind to the socket - this basically just asks the kernel to associate the socket
    // we opened earlier with a name, which for AF_INET sockets is an IP and port that
    // get pulled from the sockaddr_in struct.
    if (bind(values->sockfd, (struct sockaddr*)&values->addr, sizeof(values->addr)) != 0)
    {
        char errName[256];
        strerror_r(errno, errName, sizeof(errName) / sizeof(char));
        fprintf(stderr, "socket binding failed: %d (%s)\n", errno, errName);
        return 2;
    }

    // finally, we connect to the socket - from our point of view, this basically just
    // starts the connection.
    if (connect(values->sockfd, (struct sockaddr*)&values->addr, sizeof(values->addr)) != 0)
    {
        char errName[256];
        strerror_r(errno, errName, sizeof(errName) / sizeof(char));
        fprintf(stderr, "socket connection failed: %d (%s)\n", errno, errName);
        return 3;
    }

    return 0;
}

int main(int argc, char** argv)
{
    server_config config;
    server_values values;
    config.port = DEFAULT_PORT;
    config.ip = DEFAULT_IP;

    // parse launch args
    int opt;
    while ((opt = getopt(argc, argv, "a:p:")) != -1)
    {
        switch (opt)
        {
            case 'a':
                // if an invalid IP is passed, we should tell the user and exit with an error code
                if (!strIsValidIP(optarg))
                {
                    fprintf(stderr, "ip \"%s\" is not valid - aborting.\n", optarg);
                    exit(1);
                }

                // if a valid IP is passed, assign the text to config.ip
                config.ip = optarg; // this is a safe assignment because optarg is just a pointer to the part of argv
                                    // the string came from - the pointer will change, but the address moved to
                                    // config.ip will remain valid and pointed at the contents we want.
                break;
            case 'p':
                // parse the port into config.port
                config.port = parsePort(optarg);
                // if the port could not be parsed or was invalid, tell the user and exit with an error code
                if (config.port == -1)
                {
                    fprintf(stderr, "invalid port - ports must fall within the range [0,65535]. Aborting.\n");
                    exit(1);
                }
                break;
            default:
                // if we get here, the user is passing in something weird.
                fprintf(stderr, "usage: program [-p <port>] [-a <ip address>]\n");
                exit(1);
        }
    }

    printf("initializing server on %s:%d\n", config.ip, config.port);

    // attempt to initialize the server, quit on failure
    if (init_server(&config, &values) != 0)
    {
        exit(1);
    }

    close(values.sockfd);
    
    return 0;
}

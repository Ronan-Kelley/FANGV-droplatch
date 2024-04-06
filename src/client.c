#include "client.h"
#include "client_server_shared.h"

#include <arpa/inet.h>      // inet_addr(), inet_pton()
#include <errno.h>          // errno
#include <netinet/in.h>     // sockaddr_in
#include <stdio.h>          // printf()
#include <stdlib.h>         // exit()
#include <string.h>         // memset(), strerror()
#include <unistd.h>         // open(), close(), write(), read(), getopt()

/**
 * attempt to initialize a connection to the droplatch server.
 *
 * return 0 on success, calls exit() on failure
 */
int init_connection(client_config* config, client_values* values)
{
    // set up server address struct
    memset(&values->serveraddr, 0, sizeof(values->serveraddr));
    values->serveraddr.sin_family = AF_INET;
    values->serveraddr.sin_addr.s_addr = inet_addr(config->ip);
    values->serveraddr.sin_port = htons(config->port);

    // create socket
    values->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (values->sockfd == -1)
    {
        // no need to use strerror_r in client, as it doesn't have any
        // reason to be multithreaded
        fprintf(stderr, "socket creation failed: %d (%s)\n", errno, strerror(errno)); 
        exit(1);
    }

    // connect socket to server
    if (connect(values->sockfd, (struct sockaddr*)&values->serveraddr, sizeof(values->serveraddr)) != 0)
    {
        fprintf(stderr, "connection failed: %d (%s)\n", errno, strerror(errno));
        exit(2);
    }
    
    return 0;
}

/// temporary test loop for client
void echoChat(client_values* values)
{
    char buf[256];
    int n;
    for (;;)
    {
        // reset values
        memset(buf, 0, sizeof(buf));
        n = 0;

        // get user input
        printf("> ");
        while ((buf[n++] = getchar()) != '\n');
        // remove endline
        buf[n - 1] = '\0';
        // write user input to server
        write(values->sockfd, buf, sizeof(buf));

        // reset buffer
        memset(buf, 0, sizeof(buf));

        // read server response
        read(values->sockfd, buf, sizeof(buf));
        // print server response
        printf("\"%s\"\n", buf);

        // exit condition
        if (!strncmp(buf, "exit", 4))
        {
            break;
        }
    }
}

int main(int argc, char** argv)
{
    client_config config;
    config.ip = DEFAULT_IP;
    config.port = DEFAULT_PORT;

    client_values values;

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

    printf("attempting to connect to %s:%d\n", config.ip, config.port);
    init_connection(&config, &values);
    printf("connection successfull.\n");

    echoChat(&values);

    close(values.sockfd);

    return 0;
}

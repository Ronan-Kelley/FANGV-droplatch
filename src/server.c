#include "server.h"
#include "client_server_shared.h"

#include <arpa/inet.h>      // inet_addr(), inet_pton()
#include <errno.h>          // errno
#include <netinet/in.h>     // sockaddr_in
#include <poll.h>           // poll(), pollfd
#include <pthread.h>        // multithreading
#include <stdio.h>          // fprintf()
#include <stdlib.h>         // exit(), strtol()
#include <string.h>         // memset(), strtoerr_r()
#include <sys/socket.h>     // socket()
#include <unistd.h>         // close(), open(), read(), write(), getopt()

/**
 * initialize the server based on the config, setting up values
 * to be a server_values struct with a valid state
 *
 * returns 0 on success, calls exit() on failure
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

    // set up the socket as a non-blocking internet stream
    values->server_fd = socket(AF_INET, SOCK_STREAM /*| SOCK_NONBLOCK*/, 0);
    // check for errors in setting up the socket
    if (values->server_fd == -1)
    {
        // this software isn't multithreaded as of my writing this, but it ideally will be
        // eventually, so we have to allocate a buffer and use strerror_r() instead of just
        // using str_error() in place.
        char errName[256];
        strerror_r(errno, errName, sizeof(errName));
        fprintf(stderr, "socket creation failed: %d (%s)\n", errno, errName);
        return 1;
    }

    // bind to the socket - this basically just asks the kernel to associate the socket
    // we opened earlier with a name, which for AF_INET sockets is an IP and port that
    // get pulled from the sockaddr_in struct.
    if (bind(values->server_fd, (struct sockaddr*)&values->addr, sizeof(values->addr)) != 0)
    {
        char errName[256];
        strerror_r(errno, errName, sizeof(errName));
        fprintf(stderr, "socket binding failed: %d (%s)\n", errno, errName);
        return 2;
    }

    // finally, start listening for connections - this doesn't do much on its own,
    // but it does allow us to start accept()ing new connections.
    if (listen(values->server_fd, MAX_CONN) != 0)
    {
        char errName[256];
        strerror_r(errno, errName, sizeof(errName));
        fprintf(stderr, "socket listen failed: %d (%s)\n", errno, errName);
        return 3;
    }

    // initialize various data structures
    for (int i = 0; i < MAX_CONN; ++i)
    {
        /////////////////
        // client data //
        /////////////////
        // zero out client structs
        memset(&values->clients[i], 0, sizeof(values->clients[i]));

        //////////////////
        // polling data //
        //////////////////

        // make every pfd ignored
        values->client_pfds[i].fd = -1;
        // listen for incoming data and hangups
        values->client_pfds[i].events = POLLIN | POLLHUP;
    }

    return 0;
}

/*
 * attempt to accept any incoming connections;
 * if an incoming connection is found, this function will
 * search for an empty client connection struct in the server's
 * array of them and use it to handle the connection.
 *
 * if no empty connection is available or the connection fails,
 * this function returns -1.
 *
 * if there are no connections to accept, this function returns 0.
 *
 * if a connection is accepted successfully, this function returns 1.
 */
int accept_connections(server_values* values)
{
    // poll for new connections with the base timeout
    int num_events = poll(&values->accept_pfd, 1, BASE_POLL_TIMEOUT);

    // did an event happen?
    if (num_events != 0)
    {
        // if an event happened, find an empty client struct and
        // use it for the new connection
        for (int i = 0; i < MAX_CONN; ++i)
        {
            // once an empty client struct is found, use it
            if (!values->clients[i].open)
            {
                values->clients[i].open = 1;
                values->clients[i].len = sizeof(values->clients[i]);
                values->clients[i].client_fd = accept(
                    values->server_fd,
                    (struct sockaddr*)&values->clients[i],
                    &values->clients[i].len
                );

                // check for errors
                if (values->clients[i].client_fd == -1)
                {
                    printf("accept encountered an error!\n");
                    values->clients[i].open = 0;
                    return -1;
                }

                printf("accepted a connection to index %d\n", i);

                return 1;
            }
        }

        return -1;
    }

    return 0;
}

/*
 * manage the list of client pfds - we only want to poll
 * open connections.
 */
void manage_client_pfds(server_values* values)
{
    for (int i = 0; i < MAX_CONN; ++i)
    {
        if (values->clients[i].open)
        {
            values->client_pfds[i].fd = values->clients[i].client_fd;
        }
        else
        {
            // negative pfds are ignored by poll
            values->client_pfds[i].fd = -1;
        }
        // no need to set events since they should be set in server init
    }
}

/*
 * read from and individually handle every open connection;
 * normally this would involve thread pools, but since this
 * will never be handling more than probably 1 connection at
 * a time that feels a bit excessive.
 */
void read_connections(server_values* values)
{
    // keep buffer allocated between calls
    // since we zero it anyway
    static char buf[BUF_SIZE];

    // poll every pfd at with the base timeout; uninitialized connections
    // should have their pfd set to -1 and as such be ignored.
    int num_events = poll(values->client_pfds, MAX_CONN, BASE_POLL_TIMEOUT);

    // only scan through for events if some happened
    if (num_events > 0)
    {
        for (int i = 0; i < MAX_CONN; ++i)
        {
            // handle all sockets that had POLLIN events
            if (values->client_pfds[i].revents & POLLIN)
            {
                // TODO replace this with an event system instead of just
                // echoing back a response
                
                // reset the buffer
                memset(buf, 0, sizeof(buf));

                // read client message to buffer
                read(values->clients[i].client_fd, buf, sizeof(buf));
                printf("received message \"%s\" from client %d\n", buf, i);

                // send message back to client
                write(values->clients[i].client_fd, buf, strlen(buf) + 1);

                // exit condition
                if (!strncmp("exit", buf, 4))
                {
                    values->clients[i].open = 0;
                    close(values->clients[i].client_fd);
                }
            }
            else if (values->client_pfds[i].revents & POLLHUP)
            {
                printf("closing connection on index %d\n", i);
                values->clients[i].open = 0;
                close(values->clients[i].client_fd);
            }
        }
    }
}

/*
 * the signature for POSIX threads has to be void* func(void* params) -
 * this method expects _values to be a pointer to a server_values struct
 * and does not return anything but NULL.
 */
void* server_loop(void* _values)
{
    server_values* values = _values;

    // set up accept_pfd so we can poll server socket for incoming connections asynchronously
    values->accept_pfd.fd = values->server_fd;
    values->accept_pfd.events = POLLIN;

    for (;;)
    {
        // check for new connections
        if (accept_connections(values) == -1)
        {
            // write errors to stderr
            char errName[256];
            strerror_r(errno, errName, sizeof(errName));
            fprintf(stderr, "accept() failed: %d (%s)\n", errno, errName);
        }

        manage_client_pfds(values);

        read_connections(values);
    }

    // clean up after the server
    close(values->server_fd);

    return NULL;
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

    // handle the TCP server stuff in a seperate thread
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, server_loop, (void *)&values);
    
    pthread_exit(NULL);
    exit(0);
}

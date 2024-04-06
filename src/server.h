#ifndef DROPLATCH_SERVER_H_HGUARD
#define DROPLATCH_SERVER_H_HGUARD

#include <netinet/in.h> // sockaddr_in, sockaddr_storage, socklen_t
#include <poll.h>       // poll(), pollfds

#define BASE_POLL_TIMEOUT   ((int) 100)
#define BUF_SIZE            ((int) 256)
#define MAX_CONN            ((int) 8)

/**
 *  server configuration values; should not change after startup
 */
typedef struct server_config_t
{
    int port;
    char* ip;
} server_config;

typedef struct client_connection_t
{
    /// is the socket open?
    int open;
    /// connection socket file descriptor
    int client_fd;
    /// connection socket information
    struct sockaddr_storage addr;
    /// connection socket length for syscalls
    socklen_t len;
} client_connection;

/**
 * server values - used to maintain the state of the server
 */
typedef struct server_values_t
{
    /// server socket file descriptor
    int server_fd;
    /// server socket information
    struct sockaddr_in addr;
    /// list of clients
    client_connection clients[MAX_CONN];

    /// pollfd to use to scan for incoming connections without blocking 
    struct pollfd accept_pfd;
    /// list of pollfds to scan the client connections
    struct pollfd client_pfds[MAX_CONN];
} server_values;

#endif

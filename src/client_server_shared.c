#include "client_server_shared.h"

#include <arpa/inet.h>      // sockaddr_in, inet_pton()
#include <errno.h>          // errno
#include <stdlib.h>         // strtol()

bool strIsValidIP(char* ip_str)
{
    // attempt to parse the IP string with inet_pton
    struct sockaddr_in tmpAddr;
    int result = inet_pton(AF_INET, ip_str, &(tmpAddr.sin_addr));

    // check the result
    return result == 1;
}

int parsePort(char* port_str)
{
    // set errno to 0 so we know if an error actually happened
    errno = 0;

    // attempt to parse the given port arg
    long tmpHolder = strtol(port_str, NULL, 10);

    // if the given port cannot be parsed, return -1
    if (errno != 0)
    {
        return -1;
    }

    // make sure the parsed value is a valid port number
    if (tmpHolder >= 0 && tmpHolder <= 65535)
    {
        // this is a safe cast because we know that tmpHolder is within [0,65535], which
        // all fall easily within the range of a signed 32-bit integer as that is the range
        // of an unsigned 16-bit integer.
        return (int) tmpHolder;
    }

    // if the parsed value is not a valid port number, return the error value -1
    return -1;
}

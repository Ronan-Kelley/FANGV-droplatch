#include "client.h"
#include "client_server_shared.h"

#include <arpa/inet.h>      // inet_addr(), inet_pton()
#include <netinet/in.h>     // sockaddr_in
#include <stdio.h>          // printf()
#include <stdlib.h>         // exit()
#include <unistd.h>         // open(), close(), write(), read(), getopt()

int main(int argc, char** argv)
{
    client_config config;
    config.ip = DEFAULT_IP;
    config.port = DEFAULT_PORT;

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

    return 0;
}

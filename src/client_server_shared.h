#ifndef DROPLATCH_CON_DEFAULTS_H_HGUARD
#define DROPLATCH_CON_DEFAULTS_H_HGUARD

#include <stdbool.h>

#define DEFAULT_PORT    3000            // this port was not chosen for any particular reason
                                        // aside from its not being a system port [0,1024).
#define DEFAULT_IP      "127.0.0.1"

/// determine if a string is a valid IP
bool strIsValidIP(char* ip_str);
/// attempt to parse a port from a string;
/// if the port cannot be parsed or is otherwise invalid,
/// returns -1. Otherwise, returns the port.
int parsePort(char* port_str);

#endif

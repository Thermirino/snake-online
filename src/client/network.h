#ifndef SNAKE_CLIENT_NETWORK_H
#define SNAKE_CLIENT_NETWORK_H

#include <stdbool.h>
#include "client_internal.h"

bool client_connect(client_state* state, const char* hostname, const char* port);
bool receive_server_packets(client_state* state);

#endif

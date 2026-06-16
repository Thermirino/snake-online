#ifndef SNAKE_CLIENT_NETWORK_H
#define SNAKE_CLIENT_NETWORK_H

#include <stdbool.h>
#include "client_internal.h"

typedef enum {
    CLIENT_CONN_OK,
    CLIENT_CONN_ERR_BAD_ARG,
    CLIENT_CONN_ERR_CONNECT,
    CLIENT_CONN_ERR_SEND,
    CLIENT_CONN_ERR_RECV,
    CLIENT_CONN_ERR_INVALID_PACKET
} client_conn_status;

client_conn_status client_connect(client_state* state, const char* hostname, const char* port, const char* nickname, const char** error_text);
void client_disconnect(client_state* state);
bool client_receive_packets(client_state* state);

#endif

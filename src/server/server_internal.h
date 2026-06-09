#ifndef SNAKE_SERVER_INTERNAL
#define SNAKE_SERVER_INTERNAL

#include <stddef.h>
#include <poll.h>
#include <game.h>
#include <protocol.h>

#define MAX_PENDING_CONNECTIONS     16
#define MAX_CLIENTS                 16
#define TICK_MS                     200

typedef enum {
    CLIENT_CONNECTING = 0,
    CLIENT_CONNECTED,
    CLIENT_DISCONNECTED
} client_status;

typedef struct {
    client_status status;
    int fd;
    uint32_t snake_id;
} client;

typedef struct {
    int listenfd;

    client clients[MAX_CLIENTS];
    size_t nclients;

    game_state gs;
} server_state;

bool server_state_init(server_state* state, const char* port,
                       int width, int height);
void server_state_destroy(server_state* state);

#endif

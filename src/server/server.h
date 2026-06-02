#ifndef SNAKE_SERVER
#define SNAKE_SERVER

#include <stddef.h>
#include <poll.h>
#include <game.h>

#define MAX_PENDING_CONNECTIONS     16
#define MAX_CLIENTS                 16

typedef struct {
    int fd;
} client;

typedef struct {
    int listenfd;

    client clients[MAX_CLIENTS];
    size_t nclients;

    game_state gs;
} server_state;


bool server_state_init(server_state* state, const char* port, int width, int height);
void server_state_destroy(server_state* state);

#endif

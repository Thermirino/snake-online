#ifndef SNAKE_CLIENT_INTERNAL_H
#define SNAKE_CLIENT_INTERNAL_H

#include <game.h>
#include <render.h>

#define FPS                 60
#define TICKS_PER_FRAME     1000 / FPS

typedef struct {
    int sockfd;

    uint32_t snake_id;

    game_state gs;
    render_context rctx;
} client_state;

#endif

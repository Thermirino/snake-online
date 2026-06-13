#ifndef SNAKE_CLIENT_INTERNAL_H
#define SNAKE_CLIENT_INTERNAL_H

#include <game.h>
#include <render.h>

#define FPS                 30
#define TICKS_PER_FRAME     1000 / FPS

typedef struct {
    int sockfd;

    uint64_t server_tick_ms;
    uint32_t snake_id;
    uint32_t spectate_snake_id;

    game_state gs;
    game_state prev_gs;
    render_context rctx;

    double time_since_last_tick;
} client_state;

void client_spectate_next(client_state* state);
void client_spectate_prev(client_state* state);

#endif

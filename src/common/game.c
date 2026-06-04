#include "snake.h"
#include <stdio.h>
#include <stdlib.h>
#include <game.h>

bool game_state_init(game_state* gs, int width, int height)
{
    if (!gs || width <= 0 || height <= 0)
        return false;

    gs->brd.width = width;
    gs->brd.height = height;
    gs->snakes = NULL;
    gs->snakes_capacity = 0;
    gs->snakes_size = 0;
    return true;
}

void game_state_destroy(game_state* gs)
{
    if (!gs)
        return;

    for (size_t i = 0; i < gs->snakes_size; i++) {
        snake_destroy(&gs->snakes[i]);
    }
    free(gs->snakes);
    gs->snakes_capacity = 0;
    gs->snakes_size = 0;
}

bool game_state_add_snake(game_state* gs, snake* s)
{
    if (!gs || !s)
        return false;

    if (gs->snakes_capacity == gs->snakes_size) {
        size_t new_capacity = gs->snakes_capacity ? gs->snakes_capacity * 2 : 5;
        snake* snakes = realloc(gs->snakes, new_capacity * sizeof(*snakes));
        if (!snakes) {
            perror("malloc");
            return false;
        }
        gs->snakes = snakes;
        gs->snakes_capacity = new_capacity;
    }
    gs->snakes[gs->snakes_size++] = *s;
    return true;
}

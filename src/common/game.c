#include <game.h>

bool game_state_init(game_state* gs, int width, int height)
{
    gs->brd.width = width;
    gs->brd.height = height;
    gs->snakes = NULL;
    gs->snakes_capacity = 0;
    gs->snakes_size = 0;
    return true;
}

void game_state_destroy(game_state* gs)
{
    (void)gs;
}

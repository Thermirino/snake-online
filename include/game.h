#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <snake.h>

typedef struct {
    int width, height;      // in cells
} board;

typedef struct {
    board brd;
    snake* snakes;
    size_t snakes_capacity;
    size_t snakes_size;
} game_state;

bool game_state_init(game_state* gs, int width, int height);
void game_state_destroy(game_state* gs);

#endif

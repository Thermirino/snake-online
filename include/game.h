#ifndef GAME_H
#define GAME_H

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

#endif

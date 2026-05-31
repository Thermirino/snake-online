#ifndef GAME_H
#define GAME_H

#include <snake.h>

typedef struct {
    int width, height;
} board;

typedef struct {
    board brd;
    snake* snakes;
    int nsnakes;
} game_state;

#endif

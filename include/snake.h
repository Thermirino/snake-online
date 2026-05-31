#ifndef SNAKE_H
#define SNAKE_H

#include <stddef.h>

typedef enum {
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT
} direction;

typedef struct {
    int x, y;
} point;

typedef struct {
    point* points;
    size_t capacity;
    size_t size;
} snake_body;

typedef struct {
    direction dir;
    snake_body* body;
} snake;

#endif

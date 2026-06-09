#ifndef SNAKE_H
#define SNAKE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <color.h>

#define SNAKE_ID_INVALID    0

typedef enum {
    DIR_UP = 0,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT,
    DIR_NONE
} direction;

typedef struct {
    int y, x;                   // in cells
} point;

typedef struct {
    point* points;
    size_t capacity;
    size_t size;
} snake_body;

typedef struct {
    uint32_t id;
    direction dir;
    direction last_move_dir;
    snake_body body;
    color_name color;
    bool grow;
} snake;

bool snake_init(snake* s, int id, direction dir, int y, int x, color_name color);
void snake_destroy(snake* s);
bool snake_move(snake* s);
bool snake_change_direction(snake* s, direction dir);

#endif

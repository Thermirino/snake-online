#ifndef SNAKE_COLOR_H
#define SNAKE_COLOR_H

#include <stddef.h>
#include <stdbool.h>

typedef enum {
    WHITE = 0,
    BLACK,
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    PURPLE,
    PINK,

    DARK_BLUE,

    LIGHT_RED,
    LIGHT_ORANGE,
    LIGHT_YELLOW,
    LIGHT_GREEN,
    LIGHT_BLUE,
    LIGHT_PURPLE,
    LIGHT_PINK,

    COLOR_COUNT
} color_name;

color_name random_color(void);
void color_pool_init(size_t n, ...);
bool color_pool_get(color_name* color);

#endif

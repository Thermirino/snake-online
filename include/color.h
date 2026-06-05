#ifndef SNAKE_COLOR_H
#define SNAKE_COLOR_H

#include <stddef.h>

typedef enum {
    WHITE = 0,
    BLACK,
    RED,
    ORANGE,
    YELLOW,
    GREEN,
    BLUE,
    PURPLE,

    COLOR_COUNT
} color_name;

color_name random_color(void);

#endif

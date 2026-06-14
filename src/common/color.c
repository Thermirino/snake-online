#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <color.h>

static color_name color_pool[COLOR_COUNT];
static size_t color_pool_index;
static size_t color_pool_size;

color_name random_color(void)
{
    return rand() % COLOR_COUNT;
}

void color_pool_init(size_t n, ...)
{
    if (n > COLOR_COUNT)
        n = COLOR_COUNT;

    va_list args;
    va_start(args, n);

    for (size_t i = 0; i < n; i++) {
        color_pool[i] = va_arg(args, color_name);
    }

    va_end(args);

    color_pool_size = n;
    color_pool_index = color_pool_size - 1;
}

bool color_pool_get(color_name* color)
{
    if (color_pool_size == 0) {
        return false;
    }

    *color = color_pool[color_pool_index];

    if (color_pool_index == 0)
        color_pool_index = color_pool_size - 1;
    else
        color_pool_index--;

    return true;
}

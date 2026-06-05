#include <color.h>
#include <stdlib.h>

color_name random_color(void)
{
    return rand() % COLOR_COUNT;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <snake.h>

bool snake_init(snake* s, int id, direction dir, int y, int x, color_name color)
{
    if (!s)
        return false;

    if (id < 0)
        return false;
    s->id = id;

    if (dir < 0 || dir > 3) {
        fprintf(stderr, "Invalid direction value: %d\n", s->dir);
        return false;
    }
    s->dir = dir;

    s->body.size = 1;
    s->body.capacity = 3;
    s->body.points = malloc(s->body.capacity * sizeof(point));
    if (!s->body.points) {
        perror("malloc");
        return false;
    }
    s->body.points[0].y = y;
    s->body.points[0].x = x;
    s->color = color;
    s->grow = false;
    return true;
}

void snake_destroy(snake* s)
{
    if (!s)
        return;

    free(s->body.points);
    s->body.size = 0;
    s->body.capacity = 0;
}

bool snake_move(snake* s)
{
    if (!s || s->body.size == 0)
        return true;

    if (!s->grow) {
        for (size_t i = s->body.size - 1; i > 0; i--) {
            s->body.points[i] = s->body.points[i - 1];
        }
    } else {
        if (s->body.capacity == s->body.size) {
            size_t new_capacity = s->body.capacity ? s->body.capacity * 2 : 5;
            point* p = realloc(s->body.points, new_capacity * sizeof(*p));
            if (!p) {
                perror("malloc");
                return false;
            }
            s->body.points = p;
            s->body.capacity = new_capacity;
        }

        memmove(&s->body.points[1], &s->body.points[0],
                s->body.size * sizeof(point));
        s->body.size++;
        s->grow = false;
    }

    int off_y = 0;
    int off_x = 0;
    switch (s->dir) {
        case DIR_UP:
            off_y = -1;
            break;
        case DIR_RIGHT:
            off_x = 1;
            break;
        case DIR_DOWN:
            off_y = 1;
            break;
        case DIR_LEFT:
            off_x = -1;
            break;
        default:
            fprintf(stderr, "Invalid direction value: %d\n", s->dir);
            return false;
    }
    s->body.points[0].y += off_y;
    s->body.points[0].x += off_x;
    return true;
}

bool snake_change_direction(snake* s, direction dir)
{
    if (!s || dir < 0 || dir > 3)
        return false;

    s->dir = dir;
    return true;
}

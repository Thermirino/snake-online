#include "snake.h"
#include <stdio.h>
#include <stdlib.h>
#include <game.h>

bool game_state_init(game_state* gs, int width, int height)
{
    if (!gs || width <= 0 || height <= 0)
        return false;

    gs->brd.width = width;
    gs->brd.height = height;
    gs->snakes = NULL;
    gs->snakes_capacity = 0;
    gs->snakes_size = 0;
    return true;
}

void game_state_destroy(game_state* gs)
{
    if (!gs)
        return;

    for (size_t i = 0; i < gs->snakes_size; i++) {
        snake_destroy(&gs->snakes[i]);
    }
    free(gs->snakes);
    gs->snakes_capacity = 0;
    gs->snakes_size = 0;
}

bool game_state_add_snake(game_state* gs, snake* s)
{
    if (!gs || !s)
        return false;

    if (gs->snakes_capacity == gs->snakes_size) {
        size_t new_capacity = gs->snakes_capacity ? gs->snakes_capacity * 2 : 5;
        snake* snakes = realloc(gs->snakes, new_capacity * sizeof(*snakes));
        if (!snakes) {
            perror("realloc");
            return false;
        }
        gs->snakes = snakes;
        gs->snakes_capacity = new_capacity;
    }
    gs->snakes[gs->snakes_size++] = *s;
    return true;
}

bool game_check_collision(game_state* gs, point pos)
{
    if (!gs)
        return false;

    for (size_t i = 0; i < gs->snakes_size; i++) {
        snake* s = &gs->snakes[i];
        for (size_t j = 0; j < s->body.size; j++) {
            if (s->body.points[j].x == pos.x &&
                s->body.points[j].y == pos.y)
                return true;
        }
    }
    return false;
}

bool game_is_out_of_bounds(game_state* gs, point pos)
{
    if (!gs || pos.x < 0 || pos.y < 0 ||
        pos.x >= gs->brd.width || pos.y >= gs->brd.height)
        return true;
    return false;
}

bool game_find_free_place_for_snake(game_state* gs, point* pos)
{
    if (!gs || !pos)
        return false;

    for (int y = 0; y < gs->brd.height; y++) {
        for (int x = 0; x < gs->brd.width; x++) {
            pos->y = y;
            pos->x = x;
            if (!game_check_collision(gs, *pos))
                return true;
        }
    }
    return false;
}

bool game_add_player_snake(game_state* gs, uint32_t snake_id, color_name color)
{
    point pos;
    if (!game_find_free_place_for_snake(gs, &pos)) {
        fprintf(stderr, "game_find_free_place_for_snake failed\n");
        return false;
    }

    snake s;
    if (!snake_init(&s, snake_id, DIR_RIGHT, pos.y, pos.x, color)) {
        fprintf(stderr, "snake_init failed\n");
        return false;
    }
    if (!game_state_add_snake(gs, &s)) {
        fprintf(stderr, "game_state_add_snake failed\n");
        return false;
    }
    
    return true;
}

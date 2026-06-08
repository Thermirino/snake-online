#include "snake.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    gs->next_snake_id = 0;
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

bool game_delete_snake(game_state* gs, uint32_t snake_id)
{
    if (!gs)
        return false;

    for (size_t i = 0; i < gs->snakes_size; i++) {
        snake* sn = &gs->snakes[i];
        if (sn->id == snake_id) {
            size_t n = gs->snakes_size - i - 1;
            memmove(&gs->snakes[i], &gs->snakes[i + 1], n * sizeof(snake));

            gs->snakes_size--;
            return true;
        }
    }
    return false;
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

bool game_add_player_snake(game_state* gs, uint32_t* snake_id)
{
    if (!gs || !snake_id)
        return false;

    point pos;
    if (!game_find_free_place_for_snake(gs, &pos)) {
        fprintf(stderr, "game_find_free_place_for_snake failed\n");
        return false;
    }

    snake s;
    color_name color = random_color();
    if (!snake_init(&s, gs->next_snake_id, DIR_RIGHT, pos.y, pos.x, color)) {
        fprintf(stderr, "snake_init failed\n");
        return false;
    }
    if (!game_state_add_snake(gs, &s)) {
        fprintf(stderr, "game_state_add_snake failed\n");
        return false;
    }

    *snake_id = s.id;
    gs->next_snake_id++;
    
    return true;
}

snake* game_find_snake(game_state* gs, uint32_t snake_id)
{
    for (size_t i = 0; i < gs->snakes_size; i++) {
        snake* s = &gs->snakes[i];
        if (s->id == snake_id) {
            return s;
        }
    }
    return NULL;
}

bool game_change_snake_direction(game_state* gs, uint32_t snake_id, direction dir)
{
    if (!gs)
        return false;

    snake* s = game_find_snake(gs, snake_id);
    if (!s) {
        fprintf(stderr, "game_find_snake failed\n");
        return false;
    }
    if (!snake_change_direction(s, dir)) {
        fprintf(stderr, "snake_change_direction failed\n");
        return false;
    }
    return true;
}

static bool move_snakes(game_state* gs)
{
    for (size_t i = 0; i < gs->snakes_size; i++) {
        snake* s = &gs->snakes[i];
        if (!snake_move(s)) {
            fprintf(stderr, "snake_move failed\n");
            return false;
        }
    }
    return true;
}

bool game_update(game_state* gs)
{
    if (!gs)
        return false;

    if (!move_snakes(gs)) {
        fprintf(stderr, "move_snakes failed\n");
        return false;
    }
    return true;
}

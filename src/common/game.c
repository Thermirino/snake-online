#include "snake.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <game.h>

static bool move_snakes(game_state* gs);
static bool add_food(game_state* gs);
static bool remove_food(game_state* gs, size_t index);
static bool check_food_collisions(game_state* gs);
static bool spawn_food(game_state* gs);

bool game_state_init(game_state* gs, int width, int height)
{
    if (!gs)
        return false;

    gs->brd.width = width;
    gs->brd.height = height;

    gs->snakes = NULL;
    gs->snakes_capacity = 0;
    gs->snakes_size = 0;

    gs->food = NULL;
    gs->food_capacity = 0;
    gs->food_size = 0;

    gs->next_snake_id = 1;

    color_pool_init(7, LIGHT_RED, LIGHT_ORANGE, LIGHT_YELLOW, LIGHT_GREEN, LIGHT_BLUE, LIGHT_PURPLE, LIGHT_PINK);

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

    free(gs->food);
    gs->food_capacity = 0;
    gs->food_size = 0;
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

bool game_delete_snake_by_index(game_state* gs, size_t index)
{
    if (index >= gs->snakes_size) {
        return false;
    }

    snake* sn = &gs->snakes[index];
    snake_destroy(sn);

    size_t n = gs->snakes_size - index - 1;
    memmove(&gs->snakes[index], &gs->snakes[index + 1], n * sizeof(snake));

    gs->snakes_size--;

    return true;
}

bool game_delete_snake_by_id(game_state* gs, uint32_t snake_id)
{
    if (!gs)
        return false;

    for (size_t i = 0; i < gs->snakes_size; i++) {
        snake* sn = &gs->snakes[i];
        if (sn->id == snake_id) {
            if (!game_delete_snake_by_index(gs, i)) {
                fprintf(stderr, "game_delete_snake_by_index failed\n");
                return false;
            }
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

    for (size_t i = 0; i < gs->food_size; i++) {
        point* f = &gs->food[i];
        if (f->x == pos.x &&
            f->y == pos.y)
            return true;
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

bool game_find_free_cell(game_state* gs, point* pos)
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

bool game_find_random_free_cell(game_state* gs, point* pos)
{
    if (!gs || !pos)
        return false;

    pos->x = rand() % gs->brd.width;
    pos->y = rand() % gs->brd.height;
    while (game_check_collision(gs, *pos)) {
        pos->x = rand() % gs->brd.width;
        pos->y = rand() % gs->brd.height;
    }
    return true;
}

bool game_find_free_place_for_snake(game_state* gs, point* pos)
{
    if (!gs || !pos)
        return false;

    if (!game_find_free_cell(gs, pos)) {
        fprintf(stderr, "game_find_free_cell failed\n");
        return false;
    }
    return true;
}

bool game_add_snake(game_state* gs, uint32_t* snake_id, const char nickname[MAX_NICKNAME_LEN + 1])
{
    if (!gs || !snake_id)
        return false;

    point pos;
    if (!game_find_free_place_for_snake(gs, &pos)) {
        fprintf(stderr, "game_find_free_place_for_snake failed\n");
        return false;
    }

    color_name color;
    if (!color_pool_get(&color)) {
        fprintf(stderr, "color_pool_get failed\n");
        return false;
    }

    snake s;
    if (!snake_init(&s, gs->next_snake_id, nickname, DIR_RIGHT, pos.y, pos.x, color)) {
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
        return true;
    }
    snake_change_direction(s, dir);

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

static bool add_food(game_state* gs)
{
    if (gs->food_size == gs->food_capacity) {
        size_t new_capacity = gs->food_capacity ? gs->food_capacity * 2 : 5;
        point* p = realloc(gs->food, new_capacity * sizeof(*p));
        if (!p) {
            perror("malloc");
            return false;
        }
        gs->food = p;
        gs->food_capacity = new_capacity;
    }

    if (!game_find_random_free_cell(gs, &gs->food[gs->food_size])) {
        fprintf(stderr, "game_find_random_free_cell failed\n");
        return false;
    }
    gs->food_size++;

    return true;
}

static bool remove_food(game_state* gs, size_t index)
{
    if (index >= gs->food_size) {
        return false;
    }

    size_t n = gs->food_size - index - 1;
    memmove(&gs->food[index], &gs->food[index + 1],
            n * sizeof(point));
    gs->food_size--;
    return true;
}

static bool check_food_collisions(game_state* gs)
{
    for (size_t i = 0; i < gs->snakes_size; i++) {
        snake* s = &gs->snakes[i];
        point* head = &s->body.points[0];

        size_t j = 0;
        while (j < gs->food_size) {
            point* food = &gs->food[j];
            if (head->x == food->x &&
                head->y == food->y) {
                s->grow = true;

                if (!remove_food(gs, j)) {
                    fprintf(stderr, "remove_food failed\n");
                }

            } else
                j++;
        }

    }
    return true;
}

static bool spawn_food(game_state* gs)
{
    size_t amount = gs->brd.height * gs->brd.width * FOOD_PERCENT / 100;
    if (amount < MIN_FOOD_COUNT)
        amount = MIN_FOOD_COUNT;

    while (gs->food_size < amount) {
        if (!add_food(gs)) {
            fprintf(stderr, "add_food failed\n");
            return false;
        }
    }
    return true;
}

static bool check_snake_collisions(game_state* gs,
                                   uint32_t** dead_snake_ids,
                                   size_t* ndead)
{
    *dead_snake_ids = malloc(gs->snakes_size * sizeof(uint32_t));
    if (!*dead_snake_ids) {
        perror("malloc");
        return false;
    }

    size_t* dead_snake_indexes = malloc(gs->snakes_size * sizeof(size_t));
    if (!dead_snake_indexes) {
        perror("malloc");
        return false;
    }
    *ndead = 0;

    bool dead;
    for (size_t i = 0; i < gs->snakes_size; i++) {
        dead = false;
        snake* s1 = &gs->snakes[i];
        point* head = &s1->body.points[0];

        // out of bounds
        if (game_is_out_of_bounds(gs, *head)) {
            dead_snake_indexes[*ndead] = i;
            (*dead_snake_ids)[(*ndead)++] = s1->id;
            dead = true;
            continue;
        }

        // collision with itself
        for (size_t j = 1; j < s1->body.size; j++) {
            if (s1->body.points[j].x == head->x &&
                s1->body.points[j].y == head->y) {
                dead_snake_indexes[*ndead] = i;
                (*dead_snake_ids)[(*ndead)++] = s1->id;
                dead = true;
                break;
            }
        }
        if (dead)
            continue;

        // collision with other snakes
        for (size_t j = 0; j < gs->snakes_size; j++) {
            snake* s2 = &gs->snakes[j];
            if (s1 == s2)
                continue;

            for (size_t k = 0; k < s2->body.size; k++) {
                if (head->x == s2->body.points[k].x &&
                    head->y == s2->body.points[k].y) {
                    dead_snake_indexes[*ndead] = i;
                    (*dead_snake_ids)[(*ndead)++] = s1->id;
                    dead = true;
                    break;
                }
            }

            if (dead)
                break;
        }
    }

    for (size_t i = 0; i < *ndead; i++) {
        size_t snake_index = dead_snake_indexes[i];
        if (!game_delete_snake_by_index(gs, snake_index)) {
            fprintf(stderr, "game_delete_snake_by_id failed\n");
            free(dead_snake_indexes);
            free(*dead_snake_ids);
            *ndead = 0;
            return false;
        }
    }

    free(dead_snake_indexes);
    if (*ndead == 0)
        free(*dead_snake_ids);

    return true;
}

bool game_update(game_state* gs,
                 uint32_t** dead_snake_ids,
                 size_t* ndead)
{
    if (!gs)
        return false;

    if (!move_snakes(gs)) {
        fprintf(stderr, "move_snakes failed\n");
        return false;
    }

    if (!check_snake_collisions(gs, dead_snake_ids, ndead)) {
        fprintf(stderr, "check_snake_collisions failed\n");
        return false;
    }

    if (!check_food_collisions(gs)) {
        fprintf(stderr, "check_food_collisions failed\n");
        return false;
    }

    if (!spawn_food(gs)) {
        fprintf(stderr, "spawn_food failed\n");
        return false;
    }

    return true;
}

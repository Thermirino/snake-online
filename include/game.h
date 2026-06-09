#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <snake.h>

#define FOOD_PERCENT    5
#define MIN_FOOD_COUNT  1

typedef struct {
    int width, height;      // in cells
} board;

typedef struct {
    board brd;

    snake* snakes;
    size_t snakes_capacity;
    size_t snakes_size;

    point* food;
    size_t food_capacity;
    size_t food_size;

    size_t next_snake_id;
} game_state;

bool game_state_init(game_state* gs, int width, int height);
void game_state_destroy(game_state* gs);
bool game_state_add_snake(game_state* gs, snake* s);
bool game_delete_snake_by_index(game_state* gs, size_t index);
bool game_delete_snake_by_id(game_state* gs, uint32_t snake_id);
bool game_check_collision(game_state* gs, point pos);
bool game_is_out_of_bounds(game_state* gs, point pos);
bool game_add_player_snake(game_state* gs, uint32_t* snake_id);
snake* game_find_snake(game_state* gs, uint32_t snake_id);
bool game_change_snake_direction(game_state* gs, uint32_t snake_id, direction dir);

bool game_find_free_cell(game_state* gs, point* pos);
bool game_find_random_free_cell(game_state* gs, point* pos);
bool game_find_free_place_for_snake(game_state* gs, point* pos);

bool game_update(game_state* gs, uint32_t** dead_snake_ids, size_t* ndead);

#endif

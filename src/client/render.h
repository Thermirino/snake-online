#ifndef SNAKE_RENDER_H
#define SNAKE_RENDER_H

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <game.h>
#include <color.h>

typedef struct {
    color_name background;
    color_name grid;
} theme;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* text_font;
    int win_width;              // in pixels
    int win_height;             // in pixels
    int text_font_size;
    int cell_size;              // in pixels
    theme colors;
} render_context;

render_context* render_init(int win_width, int win_height);
void render_destroy(render_context* rctx);
bool render_game(render_context* rctx, const game_state* gs);
bool render_grid(render_context* rctx, const board* brd);
bool render_snake(render_context* rctx, const snake* s);
bool render_snakes(render_context* rctx, const snake* snakes, size_t snakes_size);

#endif

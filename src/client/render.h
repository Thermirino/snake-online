#ifndef SNAKE_RENDER_H
#define SNAKE_RENDER_H

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <game.h>
#include <color.h>

#define BORDER_SIZE     2       // in cells
#define MIN_ZOOM        0.5
#define MAX_ZOOM        1.5

typedef struct {
    color_name background;
    color_name grid;
    color_name food;
} theme;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* text_font;
    int text_font_size;

    int win_width;              // in pixels
    int win_height;             // in pixels

    int camera_y, camera_x;     // in cells
    int camera_w, camera_h;     // in cells
                               
    int cell_size;              // in pixels
    double zoom;

    theme colors;
} render_context;

bool render_init(render_context* rctx, int win_width, int win_height);
void render_destroy(render_context* rctx);
bool render_game(render_context* rctx, const game_state* gs, uint32_t snake_id);
bool render_grid(render_context* rctx, const board* brd);
bool render_snake(render_context* rctx, const snake* s);
bool render_snakes(render_context* rctx, const snake* snakes, size_t snakes_size);
bool render_food(render_context* rctx, point* food, size_t size);
void render_set_zoom(render_context* rctx, double zoom);

#endif

#ifndef SNAKE_RENDER_H
#define SNAKE_RENDER_H

#include <stdbool.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <game.h>
#include <color.h>

#define BORDER_SIZE             2       // in cells
#define MIN_ZOOM                0.5
#define MAX_ZOOM                5.0
#define FOLLOW_CAMERA_SPEED     4.0
#define FREE_CAMERA_SPEED       20

typedef struct {
    color_name background;
    color_name grid;
    color_name food;
    color_name leaderboard_bg;
    color_name leaderboard_fg;
    color_name free_camera_lbl_bg;
    color_name free_camera_lbl_fg;
    color_name game_over_bg;
    color_name game_over_fg;
} theme;

typedef enum {
    CAMERA_FOLLOW = 0,
    CAMERA_FREE
} camera_mode;

typedef struct render_context {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font_small;
    TTF_Font* font_small_bold;

    TTF_Font* font_medium;
    TTF_Font* font_medium_bold;
    TTF_Font* font_medium_bold_outline;

    TTF_Font* font_large;
    TTF_Font* font_large_bold;


    int win_width;              // in pixels
    int win_height;             // in pixels

    theme colors;
} render_context;

typedef struct {
    double y, x;                // in cells
    double w, h;                // in cells
    camera_mode mode;

    int cell_size;              // in pixels
    double zoom;
} camera;

bool render_init(render_context* rctx, int win_width, int win_height);
void render_destroy(render_context* rctx);

bool render_game(render_context* rctx, camera* cam, const game_state* gs, const game_state* prev_gs, uint32_t snake_id, uint32_t spectate_snake_id, double dt, double interp_factor);
bool render_grid(render_context* rctx, camera* cam, const board* brd);
bool render_snake(render_context* rctx, camera* cam, const snake* s, const snake* prev_s, double interp_factor);
bool render_snakes(render_context* rctx, camera* cam, const snake* snakes, size_t snakes_size, const snake* prev_snakes, size_t prev_snakes_size, double interp_factor);
bool render_food(render_context* rctx, camera* cam, point* food, size_t size);
bool render_text(render_context* rctx, TTF_Font* font, const char* text, int x, int y, color_name cname);
bool render_text_outline(render_context* rctx, TTF_Font* font, TTF_Font* outline_font, const char* text, int x, int y, color_name fg, color_name outline);
bool render_leaderboard(render_context* rctx, const game_state* gs);
bool render_free_camera_label(render_context* rctx);
bool render_game_over(render_context* rctx);
bool render_resize_window(render_context* rctx, int win_width, int win_height);
bool render_set_color(render_context* rctx, color_name cname);
bool render_set_colora(render_context* rctx, color_name cname, int alpha);

void camera_init(camera* cam, int win_width, int win_height);
void camera_resize(camera* cam, int win_width, int win_height);
void camera_set_zoom(camera* cam, double zoom, int win_width, int win_height);

#endif

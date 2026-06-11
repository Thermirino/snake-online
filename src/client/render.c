#include <SDL_error.h>
#include <SDL_render.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include "render.h"
#include "game.h"
#include "snake.h"

static const SDL_Color colors[] = {
    { 233, 216, 166, 255 },
    { 0, 18, 25, 255 },
    { 174, 32, 18, 255 },
    { 251, 86, 7, 255 },
    { 255, 190, 11, 255 },
    { 138, 201, 38, 255 },
    { 58, 134, 255, 255 },
    { 131, 56, 236, 255 }
};

/* prototypes of static functions */
static SDL_Color get_color(color_name cname);
static bool set_color(render_context* rs, color_name cname);
static void update_camera_size(render_context* rctx);

bool render_init(render_context* rctx, int win_width, int win_height)
{
    if (!rctx) {
        return false;
    }
    rctx->window = NULL;
    rctx->renderer = NULL;
    rctx->text_font = NULL;

    if (win_width <= 0) {
        win_width = 1024;
    }
    if (win_height <= 0) {
        win_height = 512;
    }
    rctx->win_width = win_width;
    rctx->win_height = win_height;
    rctx->text_font_size = win_height / 20;
    rctx->cell_size = win_height / 20;

    rctx->camera_y = 0.0;
    rctx->camera_x = 0.0;
    rctx->camera_w = (double)rctx->win_width / rctx->cell_size;
    rctx->camera_h = (double)rctx->win_height / rctx->cell_size;
    rctx->smoothness = CAM_SMOOTHNESS;

    rctx->zoom = 1.0;

    rctx->colors.background = WHITE;
    rctx->colors.grid = BLACK;
    rctx->colors.food = RED;

    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
            goto failed;
        }
    }

    rctx->window = SDL_CreateWindow("Snake Online", 
                                  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  rctx->win_width, rctx->win_height, 
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!rctx->window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        goto failed;
    }

    rctx->renderer = SDL_CreateRenderer(rctx->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!rctx->renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        goto failed;
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        goto failed;
    }
    rctx->text_font = TTF_OpenFont("assets/fonts/sans.ttf", rctx->text_font_size);
    if (rctx->text_font == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        goto failed;
    }

    if (SDL_SetRenderDrawBlendMode(rctx->renderer, SDL_BLENDMODE_BLEND) < 0) {
        fprintf(stderr, "SDL_SetRenderDrawBlendMode: %s\n",
                SDL_GetError());
        goto failed;
    }
    set_color(rctx, WHITE);
    SDL_RenderClear(rctx->renderer);

    return true;

failed:
    TTF_CloseFont(rctx->text_font);
    TTF_Quit();
    SDL_DestroyRenderer(rctx->renderer);
    SDL_DestroyWindow(rctx->window);
    SDL_Quit();
    return false;
}

void render_destroy(render_context* rctx)
{
    if (!rctx)
        return;

    TTF_CloseFont(rctx->text_font);
    TTF_Quit();
    SDL_DestroyRenderer(rctx->renderer);
    SDL_DestroyWindow(rctx->window);
    SDL_Quit();
}

static SDL_Color get_color(color_name cname)
{
    return colors[cname];
}

static bool set_color(render_context* rctx, color_name cname)
{
    SDL_Color color = get_color(cname);
    if (SDL_SetRenderDrawColor(rctx->renderer, color.r, color.g, color.b, color.a) < 0) {
        fprintf(stderr, "SDL_SetRenderDrawColor: %s\n",
                SDL_GetError());
        return false;
    }
    return true;
}

bool render_grid(render_context* rctx, const board* brd)
{
    if (!rctx || !brd)
        return false;

    if (!set_color(rctx, rctx->colors.grid)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }

    double scaled_cell_size = rctx->cell_size * rctx->zoom;
    SDL_FRect rect = { .x = 0, .y = 0,
                      .w = scaled_cell_size, .h = scaled_cell_size };
    for (int cy = rctx->camera_y; cy <= rctx->camera_y + rctx->camera_h; cy++) {
        for (int cx = rctx->camera_x; cx <= rctx->camera_x + rctx->camera_w; cx++) {

            if (cy < 0 || cx < 0 ||
                cy >= brd->height || cx >= brd->width)
                continue;

            rect.x = (cx - rctx->camera_x) * scaled_cell_size;
            rect.y = (cy - rctx->camera_y) * scaled_cell_size;
            if (SDL_RenderDrawRectF(rctx->renderer, &rect) < 0) {
                fprintf(stderr, "SDL_RenderDrawRect: %s\n",
                        SDL_GetError());
                return false;
            }
        }
    }
    return true;
}

bool render_snake(render_context* rctx, const snake* s, const snake* prev_s, double interp_factor)
{
    if (!rctx || !s)
        return false;

    if (!set_color(rctx, s->color)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }

    double scaled_cell_size = rctx->cell_size * rctx->zoom;
    SDL_FRect rect = { .x = 0, .y = 0,
        .w = scaled_cell_size, .h = scaled_cell_size };

    for (size_t i = 0; i < s->body.size; i++) {
        double target_x;
        double target_y;

        if (prev_s && i < prev_s->body.size) {
            point* origin = &prev_s->body.points[i];
            point* dest = &s->body.points[i];

            target_x = origin->x + (dest->x - origin->x) * interp_factor;
            target_y = origin->y + (dest->y - origin->y) * interp_factor;
        } else {
            point* p = &s->body.points[i];
            target_x = p->x;
            target_y = p->y;
        }

        rect.x = (target_x - rctx->camera_x) * scaled_cell_size;
        rect.y = (target_y - rctx->camera_y) * scaled_cell_size;

        if (SDL_RenderFillRectF(rctx->renderer, &rect) < 0) {
            fprintf(stderr, "SDL_RenderFillRect: %s\n",
                    SDL_GetError());
            return false;
        }
    }
    return true;
}

bool render_snakes(render_context* rctx, const snake* snakes, size_t snakes_size, const snake* prev_snakes, size_t prev_snakes_size, double interp_factor)
{
    for (size_t i = 0; i < snakes_size; i++) {
        const snake* s = &snakes[i];

        const snake* prev_s = NULL;
        for (size_t j = 0; j < prev_snakes_size; j++) {
            if (prev_snakes[j].id == s->id) {
                prev_s = &prev_snakes[j];
                break;
            }
        }

        if (!render_snake(rctx, s, prev_s, interp_factor)) {
            fprintf(stderr, "render_snake failed\n");
            return false;
        }
    }
    return true;
}

bool render_food(render_context* rctx, point* food, size_t size)
{
    if (!set_color(rctx, rctx->colors.food)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }

    double scaled_cell_size = rctx->cell_size * rctx->zoom;
    SDL_FRect rect = { .x = 0, .y = 0,
                      .w = scaled_cell_size, .h = scaled_cell_size };
    for (size_t i = 0; i < size; i++) {
        rect.x = (food[i].x - rctx->camera_x) * scaled_cell_size;
        rect.y = (food[i].y - rctx->camera_y) * scaled_cell_size;

        if (SDL_RenderFillRectF(rctx->renderer, &rect) < 0) {
            fprintf(stderr, "SDL_RenderFillRect: %s\n",
                    SDL_GetError());
            return false;
        }
    }
    return true;
}

static void center_camera(render_context* rctx, const game_state* gs, const game_state* prev_gs, uint32_t snake_id, double dt, double interp_factor)
{
    snake* s = game_find_snake((game_state*)gs, snake_id);
    if (!s) {
        return;
    }
    snake* prev_s = game_find_snake((game_state*)prev_gs, snake_id);
    if (!prev_s)
        prev_s = s;

    double target_x;
    double target_y;

    if (rctx->camera_w >= gs->brd.width + BORDER_SIZE * 2) {
        target_x = -(rctx->camera_w - gs->brd.width) / 2.0;
    } else {
        double prev_shead_x = prev_s->body.points[0].x;
        double shead_x = s->body.points[0].x;
        double target_shead_x = prev_shead_x + (shead_x - prev_shead_x) * interp_factor;
        target_x = target_shead_x - rctx->camera_w / 2.0;

        if (target_x < -BORDER_SIZE)
            target_x = -BORDER_SIZE;
        else if (target_x >= gs->brd.width - rctx->camera_w + BORDER_SIZE)
            target_x = gs->brd.width - rctx->camera_w + BORDER_SIZE;
    }
    
    if (rctx->camera_h >= gs->brd.height + BORDER_SIZE * 2) {
        target_y = -(rctx->camera_h - gs->brd.height) / 2.0;
    } else {
        double prev_shead_y = prev_s->body.points[0].y;
        double shead_y = s->body.points[0].y;
        double target_shead_y = prev_shead_y + (shead_y - prev_shead_y) * interp_factor;
        target_y = target_shead_y - rctx->camera_h / 2.0;

        if (target_y < -BORDER_SIZE)
            target_y = -BORDER_SIZE;
        else if (target_y >= gs->brd.height - rctx->camera_h + BORDER_SIZE)
            target_y = gs->brd.height - rctx->camera_h + BORDER_SIZE;
    }

    rctx->camera_x += (target_x - rctx->camera_x) * rctx->smoothness * dt;
    rctx->camera_y += (target_y - rctx->camera_y) * rctx->smoothness * dt;
}

static void update_camera_size(render_context* rctx)
{
    double scaled_cell_size = rctx->cell_size * rctx->zoom;
    rctx->camera_w = rctx->win_width  / scaled_cell_size;
    rctx->camera_h = rctx->win_height / scaled_cell_size;
}

void render_set_zoom(render_context* rctx, double zoom)
{
    if (zoom < MIN_ZOOM)
        zoom = MIN_ZOOM;
    if (zoom > MAX_ZOOM)
        zoom = MAX_ZOOM;

    rctx->zoom = zoom;

    update_camera_size(rctx);
}

bool render_resize_window(render_context* rctx, int win_width, int win_height)
{
    rctx->win_width = win_width;
    rctx->win_height = win_height;
    rctx->text_font_size = win_height / 20;

    update_camera_size(rctx);

    TTF_CloseFont(rctx->text_font);
    rctx->text_font = TTF_OpenFont("assets/fonts/sans.ttf", rctx->text_font_size);
    if (rctx->text_font == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        return false;
    }
    return true;
}

bool render_game(render_context* rctx, const game_state* gs, const game_state* prev_gs, 
                 uint32_t snake_id, double dt, double interp_factor)
{
    if (!set_color(rctx, rctx->colors.background)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }
    if (SDL_RenderClear(rctx->renderer)) {
        fprintf(stderr, "SDL_RenderClear: %s\n",
                SDL_GetError());
    }

    if (snake_id == SNAKE_ID_INVALID &&
        gs->snakes_size) {
        snake_id = gs->snakes[0].id;
    }
    center_camera(rctx, gs, prev_gs, snake_id, dt, interp_factor);

    if (!render_snakes(rctx, gs->snakes, gs->snakes_size, prev_gs->snakes, prev_gs->snakes_size, interp_factor)) {
        fprintf(stderr, "render_snakes failed\n");
        return false;
    }
    if (!render_food(rctx, gs->food, gs->food_size)) {
        fprintf(stderr, "render_food failed\n");
        return false;
    }
    if (!render_grid(rctx, &gs->brd)) {
        fprintf(stderr, "render_grid failed\n");
        return false;
    }

    SDL_RenderPresent(rctx->renderer);
    return true;
}

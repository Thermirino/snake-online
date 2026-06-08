#include <SDL_error.h>
#include <SDL_render.h>
#include <stdlib.h>
#include "render.h"

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

    rctx->colors.background = WHITE;
    rctx->colors.grid = BLACK;

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

    rctx->renderer = SDL_CreateRenderer(rctx->window, -1, SDL_RENDERER_ACCELERATED);
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
    SDL_Rect rect = { .x = 0, .y = 0,
                      .w = rctx->cell_size, .h = rctx->cell_size };
    for (int cy = 0; cy < brd->height; cy++) {
        for (int cx = 0; cx < brd->width; cx++) {
            rect.x = cx * rctx->cell_size;
            rect.y = cy * rctx->cell_size;
            if (SDL_RenderDrawRect(rctx->renderer, &rect) < 0) {
                fprintf(stderr, "SDL_RenderDrawRect: %s\n",
                        SDL_GetError());
                return false;
            }
        }
    }
    return true;
}

bool render_snake(render_context* rctx, const snake* s)
{
    if (!rctx || !s)
        return false;

    if (!set_color(rctx, s->color)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }
    SDL_Rect rect = { .x = 0, .y = 0,
                      .w = rctx->cell_size, .h = rctx->cell_size };
    for (size_t i = 0; i < s->body.size; i++) {
        point* p = &s->body.points[i];
        rect.x = p->x * rctx->cell_size;
        rect.y = p->y * rctx->cell_size;
        if (SDL_RenderFillRect(rctx->renderer, &rect) < 0) {
            fprintf(stderr, "SDL_RenderFillRect: %s\n",
                    SDL_GetError());
            return false;
        }
    }
    return true;
}

bool render_snakes(render_context* rctx, const snake* snakes, size_t snakes_size)
{
    for (size_t i = 0; i < snakes_size; i++) {
        const snake* s = &snakes[i];
        if (!render_snake(rctx, s)) {
            fprintf(stderr, "render_snake failed\n");
            return false;
        }
    }
    return true;
}

bool render_game(render_context* rctx, const game_state* gs)
{
    if (!set_color(rctx, rctx->colors.background)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }
    if (SDL_RenderClear(rctx->renderer)) {
        fprintf(stderr, "SDL_RenderClear: %s\n",
                SDL_GetError());
    }

    if (!render_snakes(rctx, gs->snakes, gs->snakes_size)) {
        fprintf(stderr, "render_snakes failed\n");
        return false;
    }
    if (!render_grid(rctx, &gs->brd)) {
        fprintf(stderr, "render_grid failed\n");
        return false;
    }

    SDL_RenderPresent(rctx->renderer);
    return true;
}

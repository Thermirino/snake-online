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

render_context* render_init(int win_width, int win_height)
{
    render_context* rs = malloc(sizeof(*rs));
    if (!rs) {
        perror("malloc");
        return NULL;
    }
    memset(rs, 0, sizeof(*rs));

    if (win_width <= 0) {
        win_width = 1024;
    }
    if (win_height <= 0) {
        win_height = 512;
    }
    rs->win_width = win_width;
    rs->win_height = win_height;
    rs->text_font_size = win_height / 20;
    rs->cell_size = win_height / 20;

    rs->colors.background = WHITE;
    rs->colors.grid = BLACK;

    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
            goto failed;
        }
    }

    rs->window = SDL_CreateWindow("Snake Online", 
                                  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                  rs->win_width, rs->win_height, 
                                  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!rs->window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        goto failed;
    }

    rs->renderer = SDL_CreateRenderer(rs->window, -1, SDL_RENDERER_ACCELERATED);
    if (!rs->renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        goto failed;
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        goto failed;
    }
    rs->text_font = TTF_OpenFont("assets/fonts/sans.ttf", rs->text_font_size);
    if (rs->text_font == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        goto failed;
    }

    if (SDL_SetRenderDrawBlendMode(rs->renderer, SDL_BLENDMODE_BLEND) < 0) {
        fprintf(stderr, "SDL_SetRenderDrawBlendMode: %s\n",
                SDL_GetError());
        goto failed;
    }
    set_color(rs, WHITE);
    SDL_RenderClear(rs->renderer);

    return rs;

failed:
    TTF_CloseFont(rs->text_font);
    TTF_Quit();
    SDL_DestroyRenderer(rs->renderer);
    SDL_DestroyWindow(rs->window);
    SDL_Quit();
    free(rs);
    return NULL;
}

void render_destroy(render_context* rs)
{
    if (!rs)
        return;

    TTF_CloseFont(rs->text_font);
    TTF_Quit();
    SDL_DestroyRenderer(rs->renderer);
    SDL_DestroyWindow(rs->window);
    SDL_Quit();
    free(rs);
}

static SDL_Color get_color(color_name cname)
{
    return colors[cname];
}

static bool set_color(render_context* rs, color_name cname)
{
    SDL_Color color = get_color(cname);
    if (SDL_SetRenderDrawColor(rs->renderer, color.r, color.g, color.b, color.a) < 0) {
        fprintf(stderr, "SDL_SetRenderDrawColor: %s\n",
                SDL_GetError());
        return false;
    }
    return true;
}

bool render_grid(render_context* rs, const board* brd)
{
    if (!rs || !brd)
        return false;

    if (!set_color(rs, rs->colors.grid)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }
    SDL_Rect rect = { .x = 0, .y = 0,
                      .w = rs->cell_size, .h = rs->cell_size };
    for (int cy = 0; cy < brd->height; cy++) {
        for (int cx = 0; cx < brd->width; cx++) {
            rect.x = cx * rs->cell_size;
            rect.y = cy * rs->cell_size;
            if (SDL_RenderDrawRect(rs->renderer, &rect) < 0) {
                fprintf(stderr, "SDL_RenderDrawRect: %s\n",
                        SDL_GetError());
                return false;
            }
        }
    }
    return true;
}

bool render_snake(render_context* rs, const snake* s)
{
    if (!rs || !s)
        return false;

    if (!set_color(rs, s->color)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }
    SDL_Rect rect = { .x = 0, .y = 0,
                      .w = rs->cell_size, .h = rs->cell_size };
    for (size_t i = 0; i < s->body.size; i++) {
        point* p = &s->body.points[i];
        rect.x = p->x * rs->cell_size;
        rect.y = p->y * rs->cell_size;
        if (SDL_RenderFillRect(rs->renderer, &rect) < 0) {
            fprintf(stderr, "SDL_RenderFillRect: %s\n",
                    SDL_GetError());
            return false;
        }
    }
    return true;
}

bool render_snakes(render_context* rs, const snake* snakes, size_t snakes_size)
{
    for (size_t i = 0; i < snakes_size; i++) {
        const snake* s = &snakes[i];
        if (!render_snake(rs, s)) {
            fprintf(stderr, "render_snake failed\n");
            return false;
        }
    }
    return true;
}

bool render_game(render_context* rs, const game_state* gs)
{
    if (!render_snakes(rs, gs->snakes, gs->snakes_size)) {
        fprintf(stderr, "render_snakes failed\n");
        return false;
    }
    if (!render_grid(rs, &gs->brd)) {
        fprintf(stderr, "render_grid failed\n");
        return false;
    }
    return true;
}

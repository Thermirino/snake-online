#include <SDL_pixels.h>
#include <SDL_render.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "render.h"
#include "game.h"
#include "snake.h"

static const SDL_Color colors[] = {
    { 255, 255, 255, 255 },
    { 0, 18, 25, 255 },
    { 174, 32, 18, 255 },
    { 251, 86, 7, 255 },
    { 255, 190, 11, 255 },
    { 138, 201, 38, 255 },
    { 58, 134, 255, 255 },
    { 131, 56, 236, 255 },
    { 20, 24, 33, 255 }
};

/* prototypes of static functions */
static SDL_Color get_color(color_name cname);
static bool set_color(render_context* rctx, color_name cname);
static bool set_colora(render_context* rctx, color_name cname, int alpha);
static void update_camera_size(render_context* rctx);

bool render_init(render_context* rctx, int win_width, int win_height)
{
    if (!rctx) {
        return false;
    }
    rctx->window = NULL;
    rctx->renderer = NULL;
    rctx->font_small = NULL;
    rctx->font_medium = NULL;
    rctx->font_large = NULL;

    if (win_width <= 0) {
        win_width = 1024;
    }
    if (win_height <= 0) {
        win_height = 512;
    }
    rctx->win_width = win_width;
    rctx->win_height = win_height;
    rctx->cell_size = win_height / 20;

    rctx->camera_y = 0.0;
    rctx->camera_x = 0.0;
    rctx->camera_w = (double)rctx->win_width / rctx->cell_size;
    rctx->camera_h = (double)rctx->win_height / rctx->cell_size;
    rctx->camera_mode = CAMERA_FOLLOW;

    rctx->zoom = 1.0;

    rctx->colors.background = WHITE;
    rctx->colors.grid = BLACK;
    rctx->colors.food = RED;
    rctx->colors.leaderboard_bg = DARK_BLUE;
    rctx->colors.leaderboard_fg = WHITE;
    rctx->colors.free_camera_lbl_bg = DARK_BLUE;
    rctx->colors.free_camera_lbl_fg = ORANGE;

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

    int font_size = 24;
    rctx->font_small = TTF_OpenFont("assets/fonts/sans.ttf", font_size);
    if (rctx->font_small == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        goto failed;
    }
    font_size = 28;
    rctx->font_medium = TTF_OpenFont("assets/fonts/sans.ttf", font_size);
    if (rctx->font_medium == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        goto failed;
    }
    font_size = 32;
    rctx->font_large = TTF_OpenFont("assets/fonts/sans.ttf", font_size);
    if (rctx->font_large == NULL) {
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
    TTF_CloseFont(rctx->font_small);
    TTF_CloseFont(rctx->font_medium);
    TTF_CloseFont(rctx->font_large);
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

    TTF_CloseFont(rctx->font_small);
    TTF_CloseFont(rctx->font_medium);
    TTF_CloseFont(rctx->font_large);
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

static bool set_colora(render_context* rctx, color_name cname, int alpha)
{
    SDL_Color color = get_color(cname);
    color.a = alpha;
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

    rctx->camera_x += (target_x - rctx->camera_x) * FOLLOW_CAMERA_SPEED * dt;
    rctx->camera_y += (target_y - rctx->camera_y) * FOLLOW_CAMERA_SPEED * dt;
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

    update_camera_size(rctx);
    return true;
}

bool render_free_camera_label(render_context* rctx)
{

    const char* text = "FREE CAMERA";
    int w, h;
    if (TTF_SizeUTF8(rctx->font_large, text, &w, &h) != 0) {
        fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
        return false;
    }

    int margin = 10;
    int padding = 20;
    int alpha = 220;

    // render bg
    SDL_Rect rect;
    rect.x = rctx->win_width / 2 - w / 2 - padding;
    rect.y = margin;
    rect.w = w + padding * 2;
    rect.h = h + padding * 2;
    if (!set_colora(rctx, rctx->colors.leaderboard_bg, alpha)) {
        fprintf(stderr, "set_colora failed\n");
        return false;
    }
    if (SDL_RenderFillRect(rctx->renderer, &rect) != 0) {
        fprintf(stderr, "SDL_RenderFillRect failed\n");
        return false;
    }

    // render text
    int x = rctx->win_width / 2 - w / 2;
    int y = margin + padding;
    if (!render_text(rctx, rctx->font_large, text, x, y, rctx->colors.free_camera_lbl_fg)) {
        fprintf(stderr, "render_text failed\n");
        return false;
    }
    return true;
}

bool render_game(render_context* rctx, const game_state* gs, const game_state* prev_gs, 
                 uint32_t spectate_snake_id, double dt, double interp_factor)
{
    if (!set_color(rctx, rctx->colors.background)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }
    if (SDL_RenderClear(rctx->renderer)) {
        fprintf(stderr, "SDL_RenderClear: %s\n",
                SDL_GetError());
    }

    if (rctx->camera_mode == CAMERA_FOLLOW)
        center_camera(rctx, gs, prev_gs, spectate_snake_id, dt, interp_factor);

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
    if (!render_leaderboard(rctx, gs)) {
        fprintf(stderr, "render_leaderboard failed\n");
        return false;
    }

    if (rctx->camera_mode == CAMERA_FREE) {
        if (!render_free_camera_label(rctx)) {
            fprintf(stderr, "render_free_camera_label failed\n");
            return false;
        }
    }

    SDL_RenderPresent(rctx->renderer);
    return true;
}

bool render_text(render_context* rctx, TTF_Font* font, const char* text,
                 int x, int y, color_name cname)
{
    SDL_Color color = get_color(cname);
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) {
        fprintf(stderr, "TTF_RenderUTF8_Blended: %s\n", 
                TTF_GetError());
        return false;
    }

    int w = surface->w;
    int h = surface->h;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(rctx->renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        fprintf(stderr, "SDL_CreateTextureFromSurface: %s\n", 
                SDL_GetError());
        return false;
    }

    if (SDL_RenderCopy(rctx->renderer, texture, NULL, &(SDL_Rect) { .x = x, .y = y, .w = w, .h = h}) != 0) {
        fprintf(stderr, "SDL_RenderCopy: %s\n", 
                SDL_GetError());
        SDL_DestroyTexture(texture);
        return false;
    }

    SDL_DestroyTexture(texture);
    return true;
}

static int snake_cmp_by_len(const void* a, const void* b)
{
    const snake* s1 = *(snake**)a;
    const snake* s2 = *(snake**)b;

    if (s1->body.size == s2->body.size)
        return 0;
    else if (s1->body.size > s2->body.size)
        return -1;
    else
        return 1;
}

bool render_leaderboard(render_context* rctx, const game_state* gs)
{
    int margin = 10;
    int padding = 10;
    int alpha = 200;

    size_t nentries = 5;
    int font_large_size = TTF_FontHeight(rctx->font_large);
    int font_med_size = TTF_FontHeight(rctx->font_medium);
    int lb_width = rctx->win_width / 5;
    int lb_height = padding * 2 + font_large_size + font_med_size * nentries;

    // render bg
    SDL_Rect rect;
    rect.x = rctx->win_width - lb_width - margin;
    rect.y = margin;
    rect.w = lb_width;
    rect.h = lb_height;
    if (!set_colora(rctx, rctx->colors.leaderboard_bg, alpha)) {
        fprintf(stderr, "set_colora failed\n");
        return false;
    }
    if (SDL_RenderFillRect(rctx->renderer, &rect) != 0) {
        fprintf(stderr, "SDL_RenderFillRect failed\n");
        return false;
    }

    // render title
    int w, h;
    const char* text = "Leaderboard";
    if (TTF_SizeUTF8(rctx->font_large, text, &w, &h) != 0) {
        fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
        return false;
    }
    int x = rctx->win_width - margin - lb_width / 2 - w / 2;
    int y = margin + padding;
    if (!render_text(rctx, 
                     rctx->font_large, 
                     text,
                     x,
                     y,
                     rctx->colors.leaderboard_fg)) {
        fprintf(stderr, "render_text failed\n");
        return false;
    }
    y += h;

    // sort snakes
    snake** snakes_sorted = malloc(gs->snakes_size * sizeof(snake*));
    if (!snakes_sorted) {
        perror("malloc");
        return false;
    }
    for (size_t i = 0; i < gs->snakes_size; i++) {
        snakes_sorted[i] = &gs->snakes[i];
    }
    qsort(snakes_sorted, gs->snakes_size, sizeof(snake*), snake_cmp_by_len);

    // render fg
    char num[23];
    char score[21];
    for (size_t i = 0; i < gs->snakes_size && i < nentries; i++) {
        snake* sn = snakes_sorted[i];

        x = rctx->win_width - margin - lb_width + padding;
        snprintf(num, sizeof(num), "%zu. ", i + 1);
        if (TTF_SizeUTF8(rctx->font_medium, num, &w, &h) != 0) {
            fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
            free(snakes_sorted);
            return false;
        }
        if (!render_text(rctx, rctx->font_medium, num, x, y, rctx->colors.leaderboard_fg)) {
            fprintf(stderr, "render_text failed\n");
            free(snakes_sorted);
            return false;
        }
        x += w;

        text = "Player";
        if (!render_text(rctx, rctx->font_medium, text, x, y, sn->color)) {
            fprintf(stderr, "render_text failed\n");
            free(snakes_sorted);
            return false;
        }

        snprintf(score, sizeof(score), "%zu", sn->body.size);
        if (TTF_SizeUTF8(rctx->font_medium, score, &w, &h) != 0) {
            fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
            free(snakes_sorted);
            return false;
        }
        x = rctx->win_width - margin - padding - w;
        if (!render_text(rctx, rctx->font_medium, score, x, y, rctx->colors.leaderboard_fg)) {
            fprintf(stderr, "render_text failed\n");
            free(snakes_sorted);
            return false;
        }
        y += h;
    }
    free(snakes_sorted);
    return true;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "render.h"
#include "game.h"
#include "snake.h"

#define max(a, b) ((a) > (b) ? (a) : (b))

static const SDL_Color colors[] = {
    { 255, 255, 255, 255 },
    { 0, 0, 0, 255 },
    { 255, 30, 30, 255 },
    { 255, 143, 0, 255 },
    { 255, 219, 0, 255 },
    { 62, 199, 11, 255 },
    { 0, 150, 255, 255 },
    { 124, 0, 255, 255 },
    { 255, 45, 209, 255 },

    { 20, 24, 33, 255 },

    { 250, 112, 112, 255 },
    { 255, 187, 112, 255 },
    { 255, 250, 183, 255 },
    { 203, 243, 187, 255 },
    { 140, 228, 255, 255 },
    { 183, 163, 227, 255 },
    { 255, 151, 208, 255 },
    
};

/* prototypes of static functions */
static SDL_Color get_color(color_name cname);

static void center_camera(camera* cam, const game_state* gs, const game_state* prev_gs, uint32_t snake_id, double dt, double interp_factor);

bool render_init(render_context* rctx, int win_width, int win_height)
{
    if (!rctx) {
        return false;
    }
    rctx->window = NULL;
    rctx->renderer = NULL;
    rctx->snake.texture = NULL;
    rctx->font_small = NULL;
    rctx->font_small_bold = NULL;
    rctx->font_medium = NULL;
    rctx->font_medium_bold = NULL;
    rctx->font_medium_bold_outline = NULL;
    rctx->font_large = NULL;
    rctx->font_large_bold = NULL;

    if (win_width <= 0) {
        win_width = 1024;
    }
    if (win_height <= 0) {
        win_height = 512;
    }
    rctx->win_width = win_width;
    rctx->win_height = win_height;

    rctx->colors.background = WHITE;
    rctx->colors.grid = BLACK;
    rctx->colors.food = RED;
    rctx->colors.leaderboard_bg = DARK_BLUE;
    rctx->colors.leaderboard_fg = WHITE;
    rctx->colors.free_camera_lbl_bg = DARK_BLUE;
    rctx->colors.free_camera_lbl_fg = ORANGE;
    rctx->colors.game_over_bg = DARK_BLUE;
    rctx->colors.game_over_fg = WHITE;

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

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "IMG_Init failed: %s\n", IMG_GetError());
        goto failed;
    }

    rctx->snake.texture = IMG_LoadTexture(rctx->renderer, "assets/sprites/snake.png");
    if (!rctx->snake.texture) {
        fprintf(stderr, "IMG_LoadTexture failed: %s\n", IMG_GetError());
        goto failed;
    }
    rctx->snake.head[0] = (SDL_Rect){ 0, 0, 42, 42 };
    rctx->snake.head[1] = (SDL_Rect){ 0, 42, 42, 42 };
    rctx->snake.head[2] = (SDL_Rect){ 0, 84, 42, 42 };
    rctx->snake.tail = (SDL_Rect){ 42, 84, 42, 42 };
    rctx->snake.tail_half = (SDL_Rect){ 42, 105, 42, 42 };
    rctx->snake.straight = (SDL_Rect){ 84, 84, 42, 42 };
    rctx->snake.corner = (SDL_Rect){ 42, 0, 42, 42 };

    rctx->food.texture = IMG_LoadTexture(rctx->renderer, "assets/sprites/food.png");
    if (!rctx->food.texture) {
        fprintf(stderr, "IMG_LoadTexture failed: %s\n", IMG_GetError());
        goto failed;
    }
    rctx->food.red_apple = (SDL_Rect){ 0, 32, 32, 32 };

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

    rctx->font_small_bold = TTF_OpenFont("assets/fonts/sans.ttf", font_size);
    if (rctx->font_small_bold == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        goto failed;
    }
    TTF_SetFontStyle(rctx->font_small_bold, TTF_STYLE_BOLD);

    font_size = 28;
    rctx->font_medium = TTF_OpenFont("assets/fonts/sans.ttf", font_size);
    if (rctx->font_medium == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        goto failed;
    }

    rctx->font_medium_bold = TTF_OpenFont("assets/fonts/sans.ttf", font_size);
    if (rctx->font_medium_bold == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        goto failed;
    }
    TTF_SetFontStyle(rctx->font_medium_bold, TTF_STYLE_BOLD);

    rctx->font_medium_bold_outline = TTF_OpenFont("assets/fonts/sans.ttf", font_size);
    if (rctx->font_medium_bold_outline == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        goto failed;
    }
    TTF_SetFontStyle(rctx->font_medium_bold_outline, TTF_STYLE_BOLD);
    TTF_SetFontOutline(rctx->font_medium_bold_outline, 2);

    font_size = 32;
    rctx->font_large = TTF_OpenFont("assets/fonts/sans.ttf", font_size);
    if (rctx->font_large == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        goto failed;
    }

    rctx->font_large_bold = TTF_OpenFont("assets/fonts/sans.ttf", font_size);
    if (rctx->font_large_bold == NULL) {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        goto failed;
    }
    TTF_SetFontStyle(rctx->font_large_bold, TTF_STYLE_BOLD);

    if (SDL_SetRenderDrawBlendMode(rctx->renderer, SDL_BLENDMODE_BLEND) < 0) {
        fprintf(stderr, "SDL_SetRenderDrawBlendMode: %s\n",
                SDL_GetError());
        goto failed;
    }
    if (!render_set_color(rctx, WHITE)) {
        fprintf(stderr, "render_set_color failed\n");
        return false;
    }
    SDL_RenderClear(rctx->renderer);

    return true;

failed:
    TTF_CloseFont(rctx->font_small);
    TTF_CloseFont(rctx->font_small_bold);
    TTF_CloseFont(rctx->font_medium);
    TTF_CloseFont(rctx->font_medium_bold);
    TTF_CloseFont(rctx->font_medium_bold_outline);
    TTF_CloseFont(rctx->font_large);
    TTF_CloseFont(rctx->font_large_bold);
    TTF_Quit();
    SDL_DestroyTexture(rctx->food.texture);
    SDL_DestroyTexture(rctx->snake.texture);
    IMG_Quit();
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
    TTF_CloseFont(rctx->font_small_bold);
    TTF_CloseFont(rctx->font_medium);
    TTF_CloseFont(rctx->font_medium_bold);
    TTF_CloseFont(rctx->font_medium_bold_outline);
    TTF_CloseFont(rctx->font_large);
    TTF_CloseFont(rctx->font_large_bold);
    TTF_Quit();
    SDL_DestroyTexture(rctx->food.texture);
    SDL_DestroyTexture(rctx->snake.texture);
    IMG_Quit();
    SDL_DestroyRenderer(rctx->renderer);
    SDL_DestroyWindow(rctx->window);
    SDL_Quit();
}

static SDL_Color get_color(color_name cname)
{
    return colors[cname];
}

bool render_set_color(render_context* rctx, color_name cname)
{
    SDL_Color color = get_color(cname);
    if (SDL_SetRenderDrawColor(rctx->renderer, color.r, color.g, color.b, color.a) < 0) {
        fprintf(stderr, "SDL_SetRenderDrawColor: %s\n",
                SDL_GetError());
        return false;
    }
    return true;
}

bool render_set_colora(render_context* rctx, color_name cname, int alpha)
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

bool render_grid(render_context* rctx, camera* cam, const board* brd)
{
    if (!rctx || !cam || !brd)
        return false;

    if (!render_set_color(rctx, rctx->colors.grid)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }

    double scaled_cell_size = cam->cell_size * cam->zoom;
    SDL_FRect rect = { .x = 0, .y = 0,
                      .w = scaled_cell_size, .h = scaled_cell_size };
    for (int cy = cam->y; cy <= cam->y + cam->h; cy++) {
        for (int cx = cam->x; cx <= cam->x + cam->w; cx++) {

            if (cy < 0 || cx < 0 ||
                cy >= brd->height || cx >= brd->width)
                continue;

            rect.x = (cx - cam->x) * scaled_cell_size;
            rect.y = (cy - cam->y) * scaled_cell_size;
            if (SDL_RenderDrawRectF(rctx->renderer, &rect) < 0) {
                fprintf(stderr, "SDL_RenderDrawRect: %s\n",
                        SDL_GetError());
                return false;
            }
        }
    }
    return true;
}

static snake_body_type snake_get_body_type(const snake* s, size_t index, double* angle)
{
    if (!s)
        return SNAKE_BODY_UNKNOWN;

    point* prev = index > 0 ? &s->body.points[index - 1] : NULL;
    point* curr = &s->body.points[index];
    point* next = s->body.size > index + 1 ? &s->body.points[index + 1] : NULL;

    if (!prev) {
        // head
        switch (s->dir) {
            case DIR_UP:
                *angle = 180;
                break;
            case DIR_DOWN:
                *angle = 0;
                break;
            case DIR_LEFT:
                *angle = 90;
                break;
            case DIR_RIGHT:
                *angle = 270;
                break;
            default:
                *angle = 0;
                break;
        }
        return SNAKE_BODY_HEAD;
    }
    else if (!next) {
        // tail
        int dx = prev->x - curr->x;
        int dy = prev->y - curr->y;

        if (dx == 0 && dy == 1)
            *angle = 180;
        else if (dx == 0 && dy == -1)
            *angle = 0;
        else if (dx == 1 && dy == 0)
            *angle = 90;
        else /* if (dx == -1 && dy == 0) */
            *angle = 270;
        return SNAKE_BODY_TAIL;
    }

    int dx1 = prev->x - curr->x;
    int dy1 = prev->y - curr->y;

    int dx2 = next->x - curr->x;
    int dy2 = next->y - curr->y;

    if (dy1 == 0 && dy2 == 0) {
        *angle = 90;
        return SNAKE_BODY_STRAIGHT;
    }
    
    if (dx1 == 0 && dx2 == 0) {
        *angle = 0;
        return SNAKE_BODY_STRAIGHT;
    }

    if ((dx1 == 0 && dy1 == -1 && dx2 == 1 && dy2 == 0) ||
        (dx1 == 1 && dy1 == 0 && dx2 == 0 && dy2 == -1)) {
        *angle = -90;
        return SNAKE_BODY_CORNER;
    }

    if ((dx1 == 0 && dy1 == 1 && dx2 == 1 && dy2 == 0) ||
        (dx2 == 0 && dy2 == 1 && dx1 == 1 && dy1 == 0)) {
        *angle = 0;
        return SNAKE_BODY_CORNER;
    }

    if ((dx1 == 0 && dy1 == 1 && dx2 == -1 && dy2 == 0) ||
        (dx2 == 0 && dy2 == 1 && dx1 == -1 && dy1 == 0)) {
        *angle = 90;
        return SNAKE_BODY_CORNER;
    }

    if ((dx1 == 0 && dy1 == -1 && dx2 == -1 && dy2 == 0) ||
        (dx2 == 0 && dy2 == -1 && dx1 == -1 && dy1 == 0)) {
        *angle = 180;
        return SNAKE_BODY_CORNER;
    }

    return SNAKE_BODY_UNKNOWN;
}

bool render_snake_body(render_context* rctx, SDL_FRect* dst, snake_body_type type, double angle, double interp_factor)
{
    SDL_FRect drect = *dst;
    SDL_Rect srect;

    switch (type) {
        case SNAKE_BODY_HEAD:
            {
                int i;
                if (interp_factor < 1.0 / 3) {
                    i = 0;
                } else if (interp_factor < 2.0 / 3) {
                    i = 1;
                } else {
                    i = 2;
                }
                srect = rctx->snake.head[i];
                break;
            }
        case SNAKE_BODY_TAIL:
            srect = rctx->snake.tail;
            break;
        case SNAKE_BODY_TAIL_HALF:
            srect = rctx->snake.tail_half;

            if (angle == 0) {
                drect.y += drect.h;
            } else if (angle == 90) {
                drect.y += drect.h / 2;
                drect.x -= drect.h / 2;
            } else if (angle == 270) {
                drect.y += drect.h / 2;
                drect.x += drect.h / 2;
            }
            break;
        case SNAKE_BODY_STRAIGHT:
            srect = rctx->snake.straight;
            break;
        case SNAKE_BODY_CORNER:
            srect = rctx->snake.corner;
            break;
        default:
            fprintf(stderr, "Unknown snake_body_type: %d\n", type);
            return false;
    }

    if (SDL_RenderCopyExF(rctx->renderer, rctx->snake.texture, &srect, &drect, angle, NULL, SDL_FLIP_NONE) != 0) {
        fprintf(stderr, "SDL_RenderCopyExF: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

bool render_snake(render_context* rctx, camera* cam, const snake* s, const snake* prev_s, double interp_factor)
{
    if (!rctx || !s)
        return false;

    SDL_Color color = get_color(s->color);
    if (SDL_SetTextureColorMod(rctx->snake.texture, color.r, color.g, color.b) != 0) {
        fprintf(stderr, "SDL_SetTextureColorMod: %s\n", SDL_GetError());
        return false;
    }

    double scaled_cell_size = cam->cell_size * cam->zoom;
    SDL_FRect rect;

    for (size_t i = s->body.size; i-- > 0;) {
        rect.w = scaled_cell_size;
        rect.h = scaled_cell_size;

        double angle;
        snake_body_type type = snake_get_body_type(s, i, &angle);
        if (type == SNAKE_BODY_UNKNOWN) {
            fprintf(stderr, "snake_get_body_type failed\n");
            return false;
        }

        double target_x;
        double target_y;
        if (i == 0 && prev_s && prev_s->body.size > 0) {
            // head
            point* origin = &prev_s->body.points[i];
            point* dest = &s->body.points[i];

            target_x = origin->x + (dest->x - origin->x) * interp_factor;
            target_y = origin->y + (dest->y - origin->y) * interp_factor;
        } else if (i == s->body.size - 1 && prev_s && prev_s->body.size > 1) {
            // tail
            type = snake_get_body_type(prev_s, prev_s->body.size - 1, &angle);

            point* origin = &prev_s->body.points[prev_s->body.size - 1];
            point* dest = &s->body.points[i];

            double a;
            if (snake_get_body_type(prev_s, i - 1, &a) == SNAKE_BODY_CORNER &&
                interp_factor > 0.5) {
                type = SNAKE_BODY_TAIL_HALF;
                rect.h /= 2;

                target_x = origin->x + (dest->x - origin->x) * interp_factor;
                target_y = origin->y + (dest->y - origin->y) * interp_factor;
            } else {
                target_x = origin->x + (dest->x - origin->x) * interp_factor;
                target_y = origin->y + (dest->y - origin->y) * interp_factor;
            }
        } else {
            point* p = &s->body.points[i];
            target_x = p->x;
            target_y = p->y;
        }

        rect.x = (target_x - cam->x) * scaled_cell_size;
        rect.y = (target_y - cam->y) * scaled_cell_size;

        if (i == 0 && s->nickname[0]) {
            int text_w, text_h;
            if (TTF_SizeUTF8(rctx->font_medium_bold, s->nickname, &text_w, &text_h) != 0) {
                fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
                return false;
            }

            int text_x = rect.x + rect.w / 2 - text_w / 2;
            int text_y = rect.y - text_h - scaled_cell_size / 2;
            if (!render_text_outline(rctx, rctx->font_medium_bold, rctx->font_medium_bold_outline, s->nickname, text_x, text_y, s->color, BLACK)) {
                fprintf(stderr, "render_text failed\n");
                return false;
            }
        }

        if (!render_snake_body(rctx, &rect, type, angle, interp_factor)) {
            fprintf(stderr, "render_snake_body failed\n");
            return false;
        }
    }

    // render the second-to-last part of the body
    // in the previous position
    if (prev_s && prev_s->body.size > 2) {
        size_t i = prev_s->body.size - 1;
        double angle;
        snake_body_type type = snake_get_body_type(prev_s, i - 1, &angle);

        if (type == SNAKE_BODY_UNKNOWN) {
            fprintf(stderr, "snake_get_body_type failed\n");
            return false;
        }

        if ((type != SNAKE_BODY_STRAIGHT || interp_factor < 0.95)) {
            double target_x;
            double target_y;

            point* p = &s->body.points[i];
            target_x = p->x;
            target_y = p->y;

            rect.x = (target_x - cam->x) * scaled_cell_size;
            rect.y = (target_y - cam->y) * scaled_cell_size;

            if (!render_snake_body(rctx, &rect, type, angle, interp_factor)) {
                fprintf(stderr, "render_snake_body failed\n");
                return false;
            }
        }
    }

    return true;
}

bool render_snakes(render_context* rctx, camera* cam, const snake* snakes, size_t snakes_size, const snake* prev_snakes, size_t prev_snakes_size, double interp_factor)
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

        if (!render_snake(rctx, cam, s, prev_s, interp_factor)) {
            fprintf(stderr, "render_snake failed\n");
            return false;
        }
    }
    return true;
}

bool render_food(render_context* rctx, camera* cam, point* food, size_t size)
{
    double scaled_cell_size = cam->cell_size * cam->zoom;
    SDL_FRect rect = { .x = 0, .y = 0,
                      .w = scaled_cell_size, .h = scaled_cell_size };
    for (size_t i = 0; i < size; i++) {
        rect.x = (food[i].x - cam->x) * scaled_cell_size;
        rect.y = (food[i].y - cam->y) * scaled_cell_size;

        if (SDL_RenderCopyF(rctx->renderer, rctx->food.texture, &rctx->food.red_apple, &rect) != 0) {
            fprintf(stderr, "SDL_RenderCopyF: %s\n", SDL_GetError());
            return false;
        }
    }
    return true;
}

bool render_resize_window(render_context* rctx, int win_width, int win_height)
{
    rctx->win_width = win_width;
    rctx->win_height = win_height;
    return true;
}

bool render_top_label(render_context* rctx, const char* text)
{
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
    if (!render_set_colora(rctx, rctx->colors.leaderboard_bg, alpha)) {
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

bool render_game(render_context* rctx, camera* cam, const game_state* gs, const game_state* prev_gs, uint32_t snake_id, uint32_t spectate_snake_id, size_t max_snake_length, bool controls_visible, double dt, double interp_factor)
{
    if (!render_set_color(rctx, rctx->colors.background)) {
        fprintf(stderr, "set_color failed\n");
        return false;
    }
    if (SDL_RenderClear(rctx->renderer)) {
        fprintf(stderr, "SDL_RenderClear: %s\n",
                SDL_GetError());
    }

    if (cam->mode == CAMERA_FOLLOW) {
        center_camera(cam, gs, prev_gs, spectate_snake_id, dt, interp_factor);
    }

    if (!render_snakes(rctx, cam, gs->snakes, gs->snakes_size, prev_gs->snakes, prev_gs->snakes_size, interp_factor)) {
        fprintf(stderr, "render_snakes failed\n");
        return false;
    }
    if (!render_food(rctx, cam, gs->food, gs->food_size)) {
        fprintf(stderr, "render_food failed\n");
        return false;
    }
    if (!render_grid(rctx, cam, &gs->brd)) {
        fprintf(stderr, "render_grid failed\n");
        return false;
    }
    if (!render_leaderboard(rctx, gs)) {
        fprintf(stderr, "render_leaderboard failed\n");
        return false;
    }

    controls_info_type ci_type = CONTROLS_PLAYING;
    if (cam->mode == CAMERA_FREE) {
        ci_type = CONTROLS_FREE_CAMERA;

        if (!render_top_label(rctx, "FREE CAMERA")) {
            fprintf(stderr, "render_top_label failed\n");
            return false;
        }
    } else if (snake_id == SNAKE_ID_INVALID &&
               spectate_snake_id == SNAKE_ID_INVALID) {
        ci_type = CONTROLS_GAME_OVER;
    } else if (snake_id == SNAKE_ID_INVALID) {
        ci_type = CONTROLS_SPECTATING;

        if (!render_top_label(rctx, "SPECTATE")) {
            fprintf(stderr, "render_top_label failed\n");
            return false;
        }
    }

    if (controls_visible && !render_controls_info(rctx, ci_type)) {
        fprintf(stderr, "render_controls_info failed\n");
        return false;
    }

    if (snake_id == SNAKE_ID_INVALID) {
        if (spectate_snake_id == SNAKE_ID_INVALID)
            if (!render_game_over(rctx, max_snake_length)) {
                fprintf(stderr, "render_game_over failed\n");
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
    SDL_Surface* surface = TTF_RenderUTF8_Blended_Wrapped(font, text, color, 0);
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

bool size_utf8_wrapped(TTF_Font* font, const char* text, int* w, int* h)
{
    char* p = strdup(text);
    if (!p)
        return false;

    char* token = strtok(p, "\n");
    int total_width = 0, total_height = 0;
    int width, height;
    while (token) {
        if (TTF_SizeUTF8(font, token, &width, &height) != 0) {
            fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
            return false;
        }

        total_width = max(total_width, width);
        total_height += height;
        
        token = strtok(NULL, "\n");
    }

    free(p);

    *w = total_width;
    *h = total_height;

    return true;
}

bool render_text_outline(render_context* rctx, TTF_Font* font, TTF_Font* outline_font, const char* text, int x, int y, color_name fg, color_name outline)
{
    int outline_size = TTF_GetFontOutline(outline_font);

    if (!render_text(rctx, outline_font, text, x - outline_size, y - outline_size, outline)) {
        fprintf(stderr, "render_text failed\n");
        return false;
    }
    if (!render_text(rctx, font, text, x, y, fg)) {
        fprintf(stderr, "render_text failed\n");
        return false;
    }
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
    if (!render_set_colora(rctx, rctx->colors.leaderboard_bg, alpha)) {
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
        if (!render_text(rctx, rctx->font_medium, num, x, y, sn->color)) {
            fprintf(stderr, "render_text failed\n");
            free(snakes_sorted);
            return false;
        }
        x += w;

        const char* nickname = sn->nickname;
        if (nickname[0] != '\0')
            if (!render_text(rctx, rctx->font_medium, nickname, x, y, sn->color)) {
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

bool render_game_over(render_context* rctx, size_t max_snake_length)
{
    const char* text = "GAME OVER";
    int w, h;
    if (TTF_SizeUTF8(rctx->font_large, text, &w, &h) != 0) {
        fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
        return false;
    }

    char text2[40];
    snprintf(text2, sizeof(text2), "Maximum length: %zu", max_snake_length);
    int w2, h2;
    if (TTF_SizeUTF8(rctx->font_large, text2, &w2, &h2) != 0) {
        fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
        return false;
    }

    int padding = 20;
    int alpha = 220;

    // render bg
    SDL_Rect rect;
    int max_w = (max(w, w2));
    rect.x = rctx->win_width / 2 - max_w / 2 - padding;
    rect.y = rctx->win_height / 2 - h / 2 - padding;
    rect.w = max_w + padding * 2;
    rect.h = h + h2 + padding * 2;
    if (!render_set_colora(rctx, rctx->colors.game_over_bg, alpha)) {
        fprintf(stderr, "set_colora failed\n");
        return false;
    }
    if (SDL_RenderFillRect(rctx->renderer, &rect) != 0) {
        fprintf(stderr, "SDL_RenderFillRect failed\n");
        return false;
    }

    // render text
    int x = rctx->win_width / 2 - w / 2;
    int y = rctx->win_height / 2 - h / 2;
    if (!render_text(rctx, rctx->font_large, text, x, y, rctx->colors.game_over_fg)) {
        fprintf(stderr, "render_text failed\n");
        return false;
    }
    y += h;


    if (TTF_SizeUTF8(rctx->font_large, text, &w, &h) != 0) {
        fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
        return false;
    }
    x = rctx->win_width / 2 - w2 / 2;
    if (!render_text(rctx, rctx->font_large, text2, x, y, rctx->colors.game_over_fg)) {
        fprintf(stderr, "render_text failed\n");
        return false;
    }
    y += h2;

    return true;
}

void camera_init(camera* cam, int win_width, int win_height)
{
    cam->cell_size = win_height / 20;
    cam->y = 0.0;
    cam->x = 0.0;
    cam->w = (double)win_width / cam->cell_size;
    cam->h = (double)win_height / cam->cell_size;
    cam->mode = CAMERA_FOLLOW;
    cam->zoom = 1.0;
}

void camera_resize(camera* cam, int win_width, int win_height)
{
    double scaled_cell_size = cam->cell_size * cam->zoom;
    cam->w = win_width  / scaled_cell_size;
    cam->h = win_height / scaled_cell_size;
}

void camera_set_zoom(camera* cam, double zoom, int win_width, int win_height)
{
    if (zoom < MIN_ZOOM)
        zoom = MIN_ZOOM;
    if (zoom > MAX_ZOOM)
        zoom = MAX_ZOOM;

    cam->zoom = zoom;

    camera_resize(cam, win_width, win_height);
}

static void center_camera(camera* cam, const game_state* gs, const game_state* prev_gs, uint32_t spectate_snake_id, double dt, double interp_factor)
{
    snake* s = game_find_snake((game_state*)gs, spectate_snake_id);
    if (!s) {
        return;
    }
    snake* prev_s = game_find_snake((game_state*)prev_gs, spectate_snake_id);
    if (!prev_s)
        prev_s = s;

    double target_x;
    double target_y;

    if (cam->w >= gs->brd.width + BORDER_SIZE * 2) {
        target_x = -(cam->w - gs->brd.width) / 2.0;
    } else {
        double prev_shead_x = prev_s->body.points[0].x;
        double shead_x = s->body.points[0].x;
        double target_shead_x = prev_shead_x + (shead_x - prev_shead_x) * interp_factor;
        target_x = target_shead_x - cam->w / 2.0;

        if (target_x < -BORDER_SIZE)
            target_x = -BORDER_SIZE;
        else if (target_x >= gs->brd.width - cam->w + BORDER_SIZE)
            target_x = gs->brd.width - cam->w + BORDER_SIZE;
    }
    
    if (cam->h >= gs->brd.height + BORDER_SIZE * 2) {
        target_y = -(cam->h - gs->brd.height) / 2.0;
    } else {
        double prev_shead_y = prev_s->body.points[0].y;
        double shead_y = s->body.points[0].y;
        double target_shead_y = prev_shead_y + (shead_y - prev_shead_y) * interp_factor;
        target_y = target_shead_y - cam->h / 2.0;

        if (target_y < -BORDER_SIZE)
            target_y = -BORDER_SIZE;
        else if (target_y >= gs->brd.height - cam->h + BORDER_SIZE)
            target_y = gs->brd.height - cam->h + BORDER_SIZE;
    }

    cam->x += (target_x - cam->x) * FOLLOW_CAMERA_SPEED * dt;
    cam->y += (target_y - cam->y) * FOLLOW_CAMERA_SPEED * dt;
}

bool render_controls_info(render_context* rctx, controls_info_type type)
{
    const char* text;
    switch (type) {
        case CONTROLS_PLAYING:
            text = "Controls:\n"
                "h - hide this menu\n"
                "arrows - move\n"
                "c - free camera";
            break;
        case CONTROLS_SPECTATING:
            text = "Controls:\n"
                "h - hide this menu\n"
                "left/right - spectate\n"
                "mouse wheel - zoom\n"
                "c - free camera\n"
                "r - respawn";
            break;
        case CONTROLS_FREE_CAMERA:
            text = "Controls:\n"
                "h - hide this menu\n"
                "arrows - move camera\n"
                "mouse wheel - zoom\n"
                "c - follow camera";
            break;
        case CONTROLS_GAME_OVER:
            text = "Controls:\n"
                "h - hide this menu\n"
                "left/right - spectate\n"
                "c - free camera\n"
                "r - respawn";
            break;
        default:
            return false;
    }

    int w, h;
    if (!size_utf8_wrapped(rctx->font_large, text, &w, &h)) {
        fprintf(stderr, "size_utf8_wrapped failed\n");
        return false;
    }

    int margin = 10;
    int padding = 20;
    int alpha = 220;

    // render bg
    SDL_Rect rect;
    rect.x = margin;
    rect.y = margin;
    rect.w = w + padding * 2;
    rect.h = h + padding * 2;
    if (!render_set_colora(rctx, rctx->colors.game_over_bg, alpha)) {
        fprintf(stderr, "set_colora failed\n");
        return false;
    }
    if (SDL_RenderFillRect(rctx->renderer, &rect) != 0) {
        fprintf(stderr, "SDL_RenderFillRect failed\n");
        return false;
    }

    // render text
    int x = rect.x + padding;
    int y = rect.y + padding;
    if (!render_text(rctx, rctx->font_large, text, x, y, rctx->colors.game_over_fg)) {
        fprintf(stderr, "render_text failed\n");
        return false;
    }
    y += h;
    return true;
}

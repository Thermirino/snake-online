#include <SDL_keyboard.h>
#include <SDL_keycode.h>
#include <stdbool.h>
#include <string.h>
#include <SDL.h>
#include "render.h"
#include "menu.h"
#include "client_internal.h"

static void input_box_init(input_box* ibox)
{
    memset(ibox->input, 0, sizeof(ibox->input));
    ibox->pos = 0;
}

static void input_box_add(input_box* ibox, const char* text)
{
    size_t len = strlen(text);
    if (len > MAX_INPUT_LEN - ibox->pos)
        len = MAX_INPUT_LEN - ibox->pos;

    memcpy(&ibox->input[ibox->pos], text, len);
    ibox->pos += len;
    ibox->input[ibox->pos] = '\0';
}

static void input_box_del_char(input_box* ibox)
{
    if (ibox->pos != 0)
        ibox->input[--ibox->pos] = '\0';
}

void menu_state_init(menu_state* state,
                     const char* hostname,
                     const char* port,
                     const char* nickname)
{
    input_box_init(&state->hostname);
    if (hostname)
        input_box_add(&state->hostname, hostname);

    input_box_init(&state->port);
    if (port)
        input_box_add(&state->port, port);

    input_box_init(&state->nickname);
    if (nickname)
        input_box_add(&state->nickname, nickname);

    state->selected_box = 0;
}

static bool menu_render_input_box(render_context* rctx, TTF_Font* font, const char* text, input_box* input, int x, int y, color_name cname, int selected)
{
    int text_w, text_h;
    if (TTF_SizeUTF8(font, "> ", &text_w, &text_h) != 0) {
        fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
        return false;
    }
    if (selected) {
        if (!render_text(rctx, font, "> ", x, y, cname)) {
            fprintf(stderr, "render_text failed\n");
            return false;
        }
    }
    x += text_w;

    if (TTF_SizeUTF8(font, text, &text_w, &text_h) != 0) {
        fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
        return false;
    }
    if (!render_text(rctx, font, text, x, y, cname)) {
        fprintf(stderr, "render_text failed\n");
        return false;
    }
    x += text_w;

    if (input->input[0]) {
        if (TTF_SizeUTF8(font, input->input, &text_w, &text_h) != 0) {
            fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
            return false;
        }
        if (!render_text(rctx, font, input->input, x, y, cname)) {
            fprintf(stderr, "render_text failed\n");
            return false;
        }
        x += text_w;
    }

    if (selected) {
        if (!render_text(rctx, font, "_", x, y, cname)) {
            fprintf(stderr, "render_text failed\n");
            return false;
        }
    }

    return true;
}

static bool menu_render_error_text(render_context* rctx, TTF_Font* font, const char* error_text, int x, int y, color_name cname)
{
    const char* text = "Error: ";
    int text_w = 0;
    int w1, h1;
    if (TTF_SizeUTF8(font, text, &w1, &h1) != 0) {
        fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
        return false;
    }
    text_w += w1;

    int w2, h2;
    if (TTF_SizeUTF8(font, error_text, &w2, &h2) != 0) {
        fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
        return false;
    }
    text_w += w2;

    x = rctx->win_width / 2 - text_w / 2;
    if (!render_text(rctx, font, text, x, y, cname)) {
        fprintf(stderr, "render_text failed\n");
        return false;
    }
    x += w1;

    if (!render_text(rctx, font, error_text, x, y, cname)) {
        fprintf(stderr, "render_text failed\n");
        return false;
    }
    return true;
}

bool menu_run(render_context* rctx, menu_state* state, const char* error_text, bool* quit_request)
{
    int y, x;

    SDL_StartTextInput();
    render_set_color(rctx, WHITE);

    int text_w, text_h;
    bool rc = true;
    SDL_Event event;
    while (1) {
        Uint64 frame_start = SDL_GetTicks64();

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    *quit_request = true;
                    goto end;
                case SDL_TEXTINPUT:
                    if (state->selected_box == 0) {
                        input_box_add(&state->hostname, event.text.text);
                    } else if (state->selected_box == 1) {
                        input_box_add(&state->port, event.text.text);
                    } else if (state->selected_box == 2) {
                        input_box_add(&state->nickname, event.text.text);
                    }
                    break;
                case SDL_KEYDOWN:
                    if (event.key.repeat)
                        break;
                    if (event.key.keysym.sym == SDLK_DOWN) {
                        state->selected_box++;
                        if (state->selected_box > 2)
                            state->selected_box = 0;
                    } else if (event.key.keysym.sym == SDLK_UP) {
                        if (state->selected_box == 0)
                            state->selected_box = 2;
                        else
                            state->selected_box--;
                    } else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        if (state->selected_box == 0) {
                            input_box_del_char(&state->hostname);
                        } else if (state->selected_box == 1) {
                            input_box_del_char(&state->port);
                        } else if (state->selected_box == 2) {
                            input_box_del_char(&state->nickname);
                        }
                    } else if (event.key.keysym.sym == SDLK_RETURN ||
                               event.key.keysym.sym == SDLK_KP_ENTER) {
                        if (!state->hostname.input[0]) {
                            error_text = "The hostname must not be empty";
                        } else if (!state->port.input[0]) {
                            error_text = "The port must not be empty";
                        } else {
                            goto end;
                        }
                        break;
                    } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        *quit_request = true;
                        goto end;
                    }
                    break;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                        int width = event.window.data1;
                        int height = event.window.data2;
                        if (!render_resize_window(rctx, width, height)) {
                            fprintf(stderr, "render_resize_window failed\n");
                            rc = false;
                            goto end;
                        }
                    }
            }
        }

        if (!render_set_colora(rctx, WHITE, 200)) {
            fprintf(stderr, "set_colora failed\n");
            return false;
        }
        SDL_RenderClear(rctx->renderer);

        const char* text = "Snake Online";
        if (TTF_SizeUTF8(rctx->font_large, text, &text_w, &text_h) != 0) {
            fprintf(stderr, "TTF_SizeUTF8: %s\n", TTF_GetError());
            rc = false;
            break;
        }
        x = rctx->win_width / 2 - text_w / 2;
        y = 10;
        if (!render_text(rctx, rctx->font_large, "Snake Online",  x, y, BLACK)) {
            fprintf(stderr, "render_text failed\n");
            rc = false;
            break;
        }

        if (TTF_SizeUTF8(rctx->font_medium, "A", &text_w, &text_h) != 0) {
            fprintf(stderr, "TTF_SizeUTF8: %s", TTF_GetError());
            rc = false;
            break;
        }

        // bg
        SDL_Rect rect;
        rect.x = rctx->win_width / 5 - text_w;
        rect.y = rctx->win_height / 5 - text_h / 2;
        rect.w = rctx->win_width - rect.x - rect.x;
        rect.h = text_h * 5;
        if (!render_set_colora(rctx, DARK_BLUE, 200)) {
            fprintf(stderr, "set_colora failed\n");
            return false;
        }
        if (SDL_RenderFillRect(rctx->renderer, &rect) != 0) {
            fprintf(stderr, "SDL_RenderFillRect failed\n");
            return false;
        }

        // fg
        color_name fg_color = WHITE;
        x = rctx->win_width / 5;
        y = rctx->win_height / 5;
        if (!menu_render_input_box(rctx, rctx->font_medium, "Hostname: ", &state->hostname, x, y, fg_color, state->selected_box == 0)) {
            fprintf(stderr, "menu_render_input_box failed\n");
            rc = false;
            break;
        }
        y += text_h;

        if (!menu_render_input_box(rctx, rctx->font_medium, "Port: ", &state->port, x, y, fg_color, state->selected_box == 1)) {
            fprintf(stderr, "menu_render_input_box failed\n");
            rc = false;
            break;
        }
        y += text_h;
        
        if (!menu_render_input_box(rctx, rctx->font_medium, "Nickname: ", &state->nickname, x, y, fg_color, state->selected_box == 2)) {
            fprintf(stderr, "menu_render_input_box failed\n");
            rc = false;
            break;
        }
        y += text_h;

        text = "Press Enter to connect";
        if (TTF_SizeUTF8(rctx->font_medium, text, &text_w, &text_h) != 0) {
            fprintf(stderr, "TTF_SizeUTF8: %s", TTF_GetError());
            rc = false;
            break;
        }
        x = rctx->win_width / 2 - text_w / 2;
        if (!render_text(rctx, rctx->font_large, text, x, y, fg_color)) {
            fprintf(stderr, "render_text failed\n");
            rc = false;
            break;
        }
        y += text_h;

        if (error_text) {
            y += text_h;
            x = rctx->win_width / 5;
            if (!menu_render_error_text(rctx, rctx->font_medium, error_text, x, y, RED)) {
                fprintf(stderr, "menu_render_error_text failed\n");
                rc = false;
                break;
            }
        }

        SDL_RenderPresent(rctx->renderer);

        Uint64 frame_time = SDL_GetTicks64() - frame_start;
        if (frame_time < TICKS_PER_FRAME) {
            SDL_Delay(TICKS_PER_FRAME - frame_time);
        }
    }

end:
    SDL_StopTextInput();
    return rc;
}

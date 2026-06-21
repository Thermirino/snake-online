#include <SDL.h>
#include <SDL_keycode.h>
#include <protocol.h>
#include "client_internal.h"
#include "input.h"
#include "render.h"
#include "snake.h"

bool process_input(client_state* state, bool* quit, double dt)
{
    if (!state || !quit)
        return false;

    *quit = false;
    SDL_Event event;
    bool change_direction = false;
    direction dir;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                *quit = true;
                break;
            case SDL_KEYDOWN:
                if (event.key.repeat)
                    break;

                if (event.key.keysym.sym == SDLK_UP) {
                    if (state->cam.mode == CAMERA_FOLLOW &&
                        state->snake_id != SNAKE_ID_INVALID) {
                        dir = DIR_UP;
                        change_direction = true;
                    }
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    if (state->cam.mode == CAMERA_FOLLOW &&
                        state->snake_id != SNAKE_ID_INVALID) {
                        dir = DIR_DOWN;
                        change_direction = true;
                    }
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    if (state->cam.mode == CAMERA_FOLLOW &&
                        state->snake_id != SNAKE_ID_INVALID) {
                        dir = DIR_RIGHT;
                        change_direction = true;
                    } else if (state->cam.mode == CAMERA_FOLLOW) {
                        // state->snake_id == SNAKE_ID_INVALID
                        client_spectate_next(state);
                    }
                } else if (event.key.keysym.sym == SDLK_LEFT) {
                    if (state->cam.mode == CAMERA_FOLLOW &&
                        state->snake_id != SNAKE_ID_INVALID) {
                        dir = DIR_LEFT;
                        change_direction = true;
                    } else if (state->cam.mode == CAMERA_FOLLOW) {
                        // state->snake_id == SNAKE_ID_INVALID
                        client_spectate_prev(state);
                    }
                } else if (event.key.keysym.sym == SDLK_c) {
                    if (state->cam.mode == CAMERA_FOLLOW)
                        state->cam.mode = CAMERA_FREE;
                    else
                        state->cam.mode = CAMERA_FOLLOW;
                } else if (event.key.keysym.sym == SDLK_r &&
                           state->snake_id == SNAKE_ID_INVALID) {

                    respawn_payload resp_payload = { 0 };
                    strncpy(resp_payload.nickname, state->nickname, MAX_NICKNAME_LEN);

                    if (!send_packet(state->sockfd,
                                     PT_RESPAWN,
                                     &resp_payload,
                                     sizeof(resp_payload))) {
                        fprintf(stderr, "send_packet failed\n");
                        return false;
                    }
                } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    *quit = true;
                    break;
                } else if (event.key.keysym.sym == SDLK_h) {
                    state->controls_visible = !state->controls_visible;
                }
                break;
            case SDL_MOUSEWHEEL:
                if (event.wheel.y > 0)
                    camera_set_zoom(&state->cam, state->cam.zoom * 1.1, state->rctx->win_width, state->rctx->win_height);
                else if (event.wheel.y < 0)
                    camera_set_zoom(&state->cam, state->cam.zoom / 1.1, state->rctx->win_width, state->rctx->win_height);

                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    int width = event.window.data1;
                    int height = event.window.data2;
                    if (!render_resize_window(state->rctx, width, height)) {
                        fprintf(stderr, "render_resize_window failed\n");
                        return false;
                    }
                    camera_resize(&state->cam, width, height);
                }
                break;
        }
    }

    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_UP]) {
        if (state->cam.mode == CAMERA_FREE) {
            state->cam.y -= FREE_CAMERA_SPEED * dt;
        }
    }
    if (keystate[SDL_SCANCODE_DOWN]) {
        if (state->cam.mode == CAMERA_FREE) {
            state->cam.y += FREE_CAMERA_SPEED * dt;
        }
    }
    if (keystate[SDL_SCANCODE_RIGHT]) {
        if (state->cam.mode == CAMERA_FREE) {
            state->cam.x += FREE_CAMERA_SPEED * dt;
        }
    }
    if (keystate[SDL_SCANCODE_LEFT]) {
        if (state->cam.mode == CAMERA_FREE) {
            state->cam.x -= FREE_CAMERA_SPEED * dt;
        }
    }

    if (change_direction) {
        input_payload input;
        input.dir = htobe32(dir);
        if (!send_packet(state->sockfd, PT_INPUT, &input, sizeof(input))) {
            fprintf(stderr, "send_packet failed\n");
            return false;
        }
    }

    return true;
}

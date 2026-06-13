#include <SDL.h>
#include <SDL_events.h>
#include <SDL_scancode.h>
#include <SDL_video.h>
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
                    if (state->rctx.camera_mode == CAMERA_FOLLOW &&
                        state->snake_id != SNAKE_ID_INVALID) {
                        dir = DIR_UP;
                        change_direction = true;
                    }
                }
                else if (event.key.keysym.sym == SDLK_DOWN) {
                    if (state->rctx.camera_mode == CAMERA_FOLLOW &&
                        state->snake_id != SNAKE_ID_INVALID) {
                        dir = DIR_DOWN;
                        change_direction = true;
                    }
                }
                else if (event.key.keysym.sym == SDLK_RIGHT) {
                    if (state->rctx.camera_mode == CAMERA_FOLLOW &&
                        state->snake_id != SNAKE_ID_INVALID) {
                        dir = DIR_RIGHT;
                        change_direction = true;
                    } else if (state->rctx.camera_mode == CAMERA_FOLLOW) {
                        // state->snake_id == SNAKE_ID_INVALID
                        client_spectate_next(state);
                    }
                }
                else if (event.key.keysym.sym == SDLK_LEFT) {
                    if (state->rctx.camera_mode == CAMERA_FOLLOW &&
                        state->snake_id != SNAKE_ID_INVALID) {
                        dir = DIR_LEFT;
                        change_direction = true;
                    } else if (state->rctx.camera_mode == CAMERA_FOLLOW) {
                        // state->snake_id == SNAKE_ID_INVALID
                        client_spectate_prev(state);
                    }
                }
                if (event.key.keysym.sym == SDLK_c) {
                    if (state->rctx.camera_mode == CAMERA_FOLLOW)
                        state->rctx.camera_mode = CAMERA_FREE;
                    else
                        state->rctx.camera_mode = CAMERA_FOLLOW;
                }
                break;
            case SDL_MOUSEWHEEL:
                if (event.wheel.y > 0)
                    render_set_zoom(&state->rctx, state->rctx.zoom * 1.1);
                else if (event.wheel.y < 0)
                    render_set_zoom(&state->rctx, state->rctx.zoom / 1.1);

                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    int width = event.window.data1;
                    int height = event.window.data2;
                    if (!render_resize_window(&state->rctx, width, height)) {
                        fprintf(stderr, "render_resize_window failed\n");
                        return false;
                    }
                }
                break;
        }
    }

    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    if (keystate[SDL_SCANCODE_UP]) {
        if (state->rctx.camera_mode == CAMERA_FREE) {
            state->rctx.camera_y -= FREE_CAMERA_SPEED * dt;
        }
    }
    if (keystate[SDL_SCANCODE_DOWN]) {
        if (state->rctx.camera_mode == CAMERA_FREE) {
            state->rctx.camera_y += FREE_CAMERA_SPEED * dt;
        }
    }
    if (keystate[SDL_SCANCODE_RIGHT]) {
        if (state->rctx.camera_mode == CAMERA_FREE) {
            state->rctx.camera_x += FREE_CAMERA_SPEED * dt;
        }
    }
    if (keystate[SDL_SCANCODE_LEFT]) {
        if (state->rctx.camera_mode == CAMERA_FREE) {
            state->rctx.camera_x -= FREE_CAMERA_SPEED * dt;
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

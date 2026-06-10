#include <SDL.h>
#include <SDL_events.h>
#include <SDL_video.h>
#include <protocol.h>
#include "input.h"
#include "render.h"
#include "snake.h"

bool process_input(client_state* state, bool* quit)
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
                if (event.key.keysym.sym == SDLK_UP) {
                    dir = DIR_UP;
                    change_direction = true;
                }
                else if (event.key.keysym.sym == SDLK_DOWN) {
                    dir = DIR_DOWN;
                    change_direction = true;
                }
                else if (event.key.keysym.sym == SDLK_RIGHT) {
                    dir = DIR_RIGHT;
                    change_direction = true;
                }
                else if (event.key.keysym.sym == SDLK_LEFT) {
                    dir = DIR_LEFT;
                    change_direction = true;
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

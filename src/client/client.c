#include <SDL_stdinc.h>
#include <client.h>
#include <SDL_timer.h>
#include "game.h"
#include "input.h"
#include "network.h"
#include "client_internal.h"
#include "render.h"

static bool client_state_init(client_state* state, const char* hostname, const char* port)
{
    if (!client_connect(state, hostname, port)) {
        fprintf(stderr, "client_connect failed\n");
        return false;
    }

    if (!game_state_init(&state->gs, 0, 0)) {
        fprintf(stderr, "game_state_init failed\n");
    }
    if (!game_state_init(&state->prev_gs, 0, 0)) {
        fprintf(stderr, "game_state_init failed\n");
    }

    if (!render_init(&state->rctx, state->gs.brd.width, state->gs.brd.height)) {
        fprintf(stderr, "render_init failed\n");
        client_disconnect(state);
        return false;
    }

    state->time_since_last_tick = 0.0;
    return true;
}

static void client_state_destroy(client_state* state)
{
    if (!state)
        return;

    game_state_destroy(&state->gs);
    game_state_destroy(&state->prev_gs);
    render_destroy(&state->rctx);
    client_disconnect(state);
}

bool client_run(const char* hostname, const char* port)
{
    client_state state;
    if (!client_state_init(&state, hostname, port)) {
        fprintf(stderr, "client_state_init failed\n");
        return false;
    }

    bool rc = true;
    bool quit_request = false;
    Uint64 prev_frame = SDL_GetTicks64();
    while (!quit_request) {
        Uint64 frame_start = SDL_GetTicks64();
        double dt = (frame_start - prev_frame) / 1000.0;

        if (!process_input(&state, &quit_request)) {
            fprintf(stderr, "process_input failed\n");
            rc = false;
            break;
        }

        if (!client_receive_packets(&state)) {
            fprintf(stderr, "client_receive_packets failed\n");
            rc = false;
            break;
        }

        state.time_since_last_tick += dt;
        double tick_duration_sec = state.server_tick_ms / 1000.0;
        double interp_factor = state.time_since_last_tick / tick_duration_sec;
        if (interp_factor > 1.5)
            interp_factor = 1.0;

        if (!render_game(&state.rctx, &state.gs, &state.prev_gs, state.snake_id, dt, interp_factor)) {
            fprintf(stderr, "render_game failed\n");
            rc = false;
            break;
        }

        Uint64 frame_time = SDL_GetTicks64() - frame_start;
        if (frame_time < TICKS_PER_FRAME) {
            SDL_Delay(TICKS_PER_FRAME - frame_time);
        }

        prev_frame = frame_start;
    }
    
    client_state_destroy(&state);

    return rc;
}

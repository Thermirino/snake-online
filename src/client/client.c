#include <SDL_stdinc.h>
#include <client.h>
#include <SDL_timer.h>
#include "game.h"
#include "input.h"
#include "network.h"
#include "client_internal.h"
#include "render.h"
#include "snake.h"

static client_run_status client_state_init(client_state* state, render_context* rctx, const char* hostname, const char* port, const char* nickname, const char** error_text)
{
    memset(state->nickname, 0, sizeof(state->nickname));
    if (nickname)
        strncpy(state->nickname, nickname, MAX_NICKNAME_LEN);

    client_conn_status status;
    if ((status = client_connect(state, hostname, port, state->nickname, error_text)) != CLIENT_CONN_OK) {
        if (status == CLIENT_CONN_ERR_CONNECT)
            return CLIENT_RUN_ERR_CONN;

        fprintf(stderr, "client_connect failed\n");
        return CLIENT_RUN_ERROR;
    }

    if (!game_state_init(&state->gs, 0, 0)) {
        fprintf(stderr, "game_state_init failed\n");
        return CLIENT_RUN_ERROR;
    }
    if (!game_state_init(&state->prev_gs, 0, 0)) {
        game_state_destroy(&state->gs);
        fprintf(stderr, "game_state_init failed\n");
        return CLIENT_RUN_ERROR;
    }
    state->max_length = 0;

    state->rctx = rctx;
    camera_init(&state->cam, rctx->win_width, rctx->win_height);

    state->controls_visible = true;

    state->time_since_last_tick = 0.0;
    return CLIENT_RUN_OK;
}

static void client_state_destroy(client_state* state)
{
    if (!state)
        return;

    game_state_destroy(&state->gs);
    game_state_destroy(&state->prev_gs);
    state->rctx = NULL;
    client_disconnect(state);
}

void client_spectate_next(client_state* state)
{
    if (state->gs.snakes_size) {
        size_t cur = 0;
        for (size_t i = 0; i < state->gs.snakes_size; i++) {
            if (state->gs.snakes[i].id == state->spectate_snake_id) {
                cur = i;
                break;
            }
        }
        cur = (cur + 1) % state->gs.snakes_size;
        state->spectate_snake_id = state->gs.snakes[cur].id;
    } else {
        state->spectate_snake_id = SNAKE_ID_INVALID;
    }
}

void client_spectate_prev(client_state* state)
{
    if (state->gs.snakes_size) {
        size_t cur = 0;
        for (size_t i = 0; i < state->gs.snakes_size; i++) {
            if (state->gs.snakes[i].id == state->spectate_snake_id) {
                cur = i;
                break;
            }
        }
        if (cur == 0)
            state->spectate_snake_id = state->gs.snakes[state->gs.snakes_size - 1].id;
        else {
            cur--;
            state->spectate_snake_id = state->gs.snakes[cur].id;
        }
    }
}

client_run_status client_run(render_context* rctx, const char* hostname, const char* port, const char* nickname, const char** error_text)
{
    client_run_status status;
    client_state state;

    if ((status = client_state_init(&state, rctx, hostname, port, nickname, error_text)) != CLIENT_RUN_OK) {
        return status;
    }

    bool rc = CLIENT_RUN_OK;
    bool quit_request = false;
    Uint64 prev_frame = SDL_GetTicks64();
    while (!quit_request) {
        Uint64 frame_start = SDL_GetTicks64();
        double dt = (frame_start - prev_frame) / 1000.0;

        if (!process_input(&state, &quit_request, dt)) {
            fprintf(stderr, "process_input failed\n");
            rc = CLIENT_RUN_ERROR;
            break;
        }

        if (!client_receive_packets(&state)) {
            fprintf(stderr, "client_receive_packets failed\n");
            rc = CLIENT_RUN_ERROR;
            break;
        }

        state.time_since_last_tick += dt;
        double tick_duration_sec = state.server_tick_ms / 1000.0;
        double interp_factor = state.time_since_last_tick / tick_duration_sec;
        if (interp_factor > 1.5)
            interp_factor = 1.0;

        if (!render_game(state.rctx, &state.cam, &state.gs, &state.prev_gs, state.snake_id, state.spectate_snake_id, state.max_length, state.controls_visible, dt, interp_factor)) {
            fprintf(stderr, "render_game failed\n");
            rc = CLIENT_RUN_ERROR;
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

#include <client.h>
#include "input.h"
#include "network.h"
#include "client_internal.h"

bool client_run(const char* hostname, const char* port)
{
    client_state state;

    if (!client_connect(&state, hostname, port)) {
        fprintf(stderr, "client_connect failed\n");
        return false;
    }

    bool quit_request = false;
    while (!quit_request) {
        if (!process_input(&state, &quit_request)) {
            fprintf(stderr, "process_input failed\n");
            return false;
        }

        if (!client_receive_packets(&state)) {
            fprintf(stderr, "receive_server_packets failed\n");
            return false;
        }

        if (!render_game(&state.rctx, &state.gs)) {
            fprintf(stderr, "render_game failed\n");
            return false;
        }
    }

    return true;
}

#include <stdio.h>
#include <client.h>
#include "render.h"
#include "menu.h"

int main(int argc, char* argv[])
{
    if (argc > 4) {
        fprintf(stderr, "Usage: %s [hostname] [port] [nickname]\n", argv[0]);
        return -1;
    }

    char* host = NULL;
    if (argc >= 2)
        host = argv[1];

    char* port = NULL;
    if (argc >= 3)
        port = argv[2];

    char* nickname = NULL;
    if (argc >= 4)
        nickname = argv[3];

    int win_width = 1920;
    int win_height = 1080;

    render_context rctx;
    if (!render_init(&rctx, win_width, win_height)) {
        fprintf(stderr, "render_init failed\n");
        return false;
    }

    menu_state mstate;
    menu_state_init(&mstate,
                    host,
                    port,
                    nickname);

    const char* error_text = NULL;
    bool quit_request = false;
    while (1) {
        if (!menu_run(&rctx, &mstate, error_text, &quit_request)) {
            fprintf(stderr, "menu_run failed\n");
            return false;
        }
        if (quit_request)
            break;

        host = mstate.hostname.input;
        port = mstate.port.input;
        nickname = mstate.nickname.input;

        error_text = NULL;
        client_run_status status;
        if ((status = client_run(&rctx, host, port, nickname, &error_text)) != CLIENT_RUN_OK) {
            if (status != CLIENT_RUN_ERR_CONN) {
                fprintf(stderr, "client_run failed\n");
                return -1;
            }
        }
    }

    render_destroy(&rctx);

    return 0;
}

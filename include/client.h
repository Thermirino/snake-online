#ifndef SNAKE_CLIENT_H
#define SNAKE_CLIENT_H

#include <stdbool.h>

typedef struct render_context render_context;

typedef enum {
    CLIENT_RUN_OK,
    CLIENT_RUN_ERROR,
    CLIENT_RUN_ERR_CONN
} client_run_status;

client_run_status client_run(render_context* rctx, const char* hostname, const char* port, const char* nickname, int win_width, int win_height, const char** error_text);

#endif

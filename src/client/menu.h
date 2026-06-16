#ifndef MENU_H
#define MENU_H

#include <snake.h>
#include "render.h"

#define MAX_INPUT_LEN   20

typedef struct {
    char input[MAX_INPUT_LEN + 1];
    size_t pos;
} input_box;

typedef struct {
    input_box hostname;
    input_box port;
    input_box nickname;
    int selected_box;
} menu_state;

void menu_state_init(menu_state* state,
                     const char* hostname,
                     const char* port,
                     const char* nickname);
bool menu_run(render_context* rctx, menu_state* state, const char* error_text, bool* quit_request);

#endif

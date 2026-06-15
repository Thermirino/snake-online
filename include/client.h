#ifndef SNAKE_CLIENT_H
#define SNAKE_CLIENT_H

#include <stdbool.h>

bool client_run(const char* hostname, const char* port, const char* nickname, int win_width, int win_height);

#endif

#ifndef SNAKE_SERVER_H
#define SNAKE_SERVER_H

#include <stdbool.h>

#define TICK_MS                     300

bool server_run(const char* port, int width, int height);

#endif

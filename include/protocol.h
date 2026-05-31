#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <snake.h>

typedef enum {
    PT_CONNECT, 
    PT_INPUT, 
    PT_GAME_STATE,
} packet_type;

typedef struct {
} packet_connect;

typedef struct {
    direction dir;
} packet_input;

typedef struct {
} packet_game_state;

typedef struct {
    packet_type type;
    union {
        packet_connect conn;
        packet_input input;
        packet_game_state gs;
    };
} packet;

#endif

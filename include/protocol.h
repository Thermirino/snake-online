#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <game.h>

typedef enum {
    PT_CONNECT, 
    PT_INPUT, 
    PT_GAME_STATE,
} packet_type;

typedef struct {
    uint64_t size;
    uint32_t type;
} packet_header;

typedef struct {
    direction dir;
} packet_input;

typedef struct {
    int32_t width, height;
    uint64_t nsnakes;
} game_state_header;

typedef struct {
    uint32_t dir;
    uint32_t color;
    uint64_t npoints;
} snake_header;

typedef struct {
    int32_t y, x;
} point_data;

bool game_state_serialize(const game_state* gs, uint8_t** buf, size_t* size);
bool game_state_deserialize(uint8_t* data, game_state* gs);
ssize_t recv_all(int fd, void* usrbuf, size_t n);
ssize_t send_all(int fd, const void* usrbuf, size_t n);

#endif

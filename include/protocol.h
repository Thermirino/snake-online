#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <game.h>

#ifdef DEBUG_NETWORK
    #define LOG_NET(fmt, ...)    fprintf(stdout, fmt, __VA_ARGS__)
#else
    #define LOG_NET(fmt, ...)   
#endif

typedef enum {
    RECV_OK,
    RECV_CLOSED,
    RECV_ERROR
} recv_status;

typedef enum {
    PT_CONNECT = 0, 
    PT_CONNECT_ACK,
    PT_RESPAWN,
    PT_RESPAWN_ACK,
    PT_INPUT, 
    PT_GAME_STATE,
    PT_GAME_OVER,
    PT_DISCONNECT,

    PT_COUNT
} packet_type;

/* Common packet header */
typedef struct {
    uint64_t size;          // packet size
    uint32_t type;
} packet_header;

/* PT_CONNECT_ACK payload */
typedef struct {
    uint32_t snake_id;
    uint64_t server_tick_ms;
} connect_ack_payload;

/* PT_RESPAWN_ACK payload */
typedef struct {
    uint32_t snake_id;
} respawn_ack_payload;

/* PT_INPUT payload */
typedef struct {
    uint32_t dir;
} input_payload;

/* PT_GAME_STATE payload */
typedef struct {
    int32_t width, height;  // board size in cells
    uint64_t nsnakes;
    uint64_t nfood;
} game_state_header;

typedef struct {
    uint32_t id;
    uint32_t dir;
    uint32_t color;
    uint64_t npoints;
} snake_header;

typedef struct {
    int32_t y, x;
} point_data;

const char* packet_type_str(packet_type ptype);

bool game_state_serialize(const game_state* gs, uint8_t** buf, size_t* size);
bool game_state_deserialize(uint8_t* data, size_t data_size, game_state* gs);

recv_status recv_packet(int fd, packet_type* ptype, void** payload, size_t* payload_size);
bool send_packet(int fd, packet_type ptype, const void* payload, size_t payload_size);

#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netdb.h>
#include "network.h"
#include "client_internal.h"
#include "game.h"
#include "render.h"
#include "snake.h"
#include <protocol.h>

static int open_clientfd(const char* hostname, const char* port, const char** error_text)
{
    int clientfd;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG;
    int rc;
    if ((rc = getaddrinfo(hostname, port, &hints, &listp))) {
        *error_text = gai_strerror(rc);
        return -1;
    }

    for (p = listp; p; p = p->ai_next) {
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
            *error_text = strerror(errno);
            continue;
        }

        if (connect(clientfd, p->ai_addr, p->ai_addrlen) == -1) {
            *error_text = strerror(errno);
        } else {
            break;
        }
        close(clientfd);
    }

    freeaddrinfo(listp);
    if (!p)
        return -1;
    else
        return clientfd;
}

client_conn_status client_connect(client_state* state, const char* hostname, const char* port, const char* nickname, const char** error_text)
{
    if (!state || !hostname || !port || !error_text)
        return CLIENT_CONN_ERR_BAD_ARG;

    state->sockfd = open_clientfd(hostname, port, error_text);
    if (state->sockfd == -1) {
        return CLIENT_CONN_ERR_CONNECT;
    }

    connect_payload conn_payload = { 0 };
    if (nickname) {
        strncpy(conn_payload.nickname, nickname, MAX_NICKNAME_LEN);
    }

    if (!send_packet(state->sockfd, PT_CONNECT, &conn_payload, sizeof(conn_payload))) {

        fprintf(stderr, "send_packet failed\n");
        close(state->sockfd);
        return CLIENT_CONN_ERR_SEND;
    }

    packet_type ptype;
    void* payload;
    size_t payload_size;
    recv_status status = recv_packet(state->sockfd, &ptype, &payload, &payload_size);
    if (status != RECV_OK) {
        fprintf(stderr, "recv_packet failed\n");
        close(state->sockfd);
        return CLIENT_CONN_ERR_RECV;
    }

    if (ptype != PT_CONNECT_ACK) {
        fprintf(stderr, "Expected PT_CONNECT_ACK (ptype = %d)\n", ptype);
        free(payload);
        close(state->sockfd);
        return CLIENT_CONN_ERR_INVALID_PACKET;
    }
    if (payload_size != sizeof(connect_ack_payload)) {
        fprintf(stderr, "Invalid payload size for a packet type PT_CONNECT_ACK\n");
        free(payload);
        close(state->sockfd);
        return CLIENT_CONN_ERR_INVALID_PACKET;
    }

    connect_ack_payload* ack = payload;
    state->snake_id = be32toh(ack->snake_id);
    state->spectate_snake_id = state->snake_id;
    state->server_tick_ms = be64toh(ack->server_tick_ms);
    free(payload);

    return CLIENT_CONN_OK;
}

void client_disconnect(client_state* state)
{
    if (!state)
        return;

    if (!send_packet(state->sockfd, PT_DISCONNECT, NULL, 0)) {
        fprintf(stderr, "send_packet failed\n");
    }
    close(state->sockfd);
}

static bool handle_packet(client_state* state,
                          packet_type ptype,
                          void* payload,
                          size_t payload_size)
{
    switch (ptype) {
        case PT_GAME_STATE:
            ;
            game_state new_gs;
            if (!game_state_deserialize(payload, payload_size, &new_gs)) {
                fprintf(stderr, "game_state deserialize failed\n");
                return false;
            }

            game_state_destroy(&state->prev_gs);
            state->prev_gs = state->gs;
            state->gs = new_gs;

            snake* sn = game_find_snake(&state->gs, state->snake_id);
            if (sn && sn->body.size > state->max_length) {
                state->max_length = sn->body.size;
            }

            state->time_since_last_tick = 0.0;

            break;
        case PT_GAME_OVER:
            game_delete_snake_by_id(&state->gs, state->snake_id);
            state->snake_id = SNAKE_ID_INVALID;
            state->spectate_snake_id = SNAKE_ID_INVALID;
            break;
        case PT_RESPAWN_ACK:
            if (payload_size != sizeof(respawn_ack_payload))
                return false;

            respawn_ack_payload* rpayload = payload;
            state->snake_id = be32toh(rpayload->snake_id);
            state->spectate_snake_id = state->snake_id;
            state->cam.mode = CAMERA_FOLLOW;
            break;
        default:
            fprintf(stderr, "Invalid packet type (%d)\n",
                    ptype);
            return false;
    }
    return true;
}

bool client_receive_packets(client_state* state)
{
    struct pollfd pfd;
    pfd.fd = state->sockfd;
    pfd.events = POLLIN;

    int rc;
    int timeout = 0;
    while ((rc = poll(&pfd, 1, timeout)) == 1) {
        packet_type ptype;
        void* payload;
        size_t payload_size;

        recv_status status = recv_packet(state->sockfd, &ptype, &payload, &payload_size);
        if (status == RECV_CLOSED) {
            fprintf(stderr, "Connection closed\n");
            return false;
        } else if (status != RECV_OK) {
            fprintf(stderr, "recv_packet failed\n");
            return false;
        }

        if (!handle_packet(state, ptype, payload, payload_size)) {
            fprintf(stderr, "handle_packet failed\n");
            free(payload);
            return false;
        }
        free(payload);
    }


    if (rc < 0) {
        perror("poll");
        return false;
    }
    
    return true;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include <server.h>
#include "game.h"
#include "server_internal.h"
#include "protocol.h"

static int open_listenfd(const char* port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;                
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    int rc;
    if ((rc = getaddrinfo(NULL, port, &hints, &listp))) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return -1;
    }

    for (p = listp; p; p = p->ai_next) {
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                   (const void*)&optval, sizeof(int));

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        close(listenfd);
    }

    freeaddrinfo(listp);
    if (!p)
        return -1;

    if (listen(listenfd, MAX_PENDING_CONNECTIONS) < 0) {
        perror("listen");
        close(listenfd);
        return -1;
    }
    return listenfd;
}

bool server_state_init(server_state* state, const char* port, int width, int height)
{
    state->listenfd = open_listenfd(port);
    if (state->listenfd == -1) {
        fprintf(stderr, "open_listenfd failed\n");
        return false;
    }
    state->nclients = 0;
    if (!game_state_init(&state->gs, width, height)) {
        fprintf(stderr, "game_state_init failed\n");
        return false;
    }
    return true;
}

void server_state_destroy(server_state* state)
{
    close(state->listenfd);
    for (size_t i = 0; i < state->nclients; i++) {
        close(state->clients[i].fd);
    }
    state->nclients = 0;
    game_state_destroy(&state->gs);
}

static bool add_client(server_state* state, int clientfd)
{
    if (state->nclients >= MAX_CLIENTS) {
        fprintf(stderr, "add_client: Too Many Clients\n");
        return false;
    }
    state->clients[state->nclients].fd = clientfd;
    state->clients[state->nclients].status = CLIENT_CONNECTING;
    state->clients[state->nclients].snake_id = -1;
    state->nclients++;
    return true;
}

static void remove_disconnected_clients(server_state* state)
{
    size_t i = 0;
    while (i < state->nclients) {
        client* cl = &state->clients[i];

        if (cl->status == CLIENT_DISCONNECTED) {
            if (!game_delete_snake(&state->gs, cl->snake_id)) {
                fprintf(stderr, "game_delete_snake failed\n");
            }
            close(cl->fd);

            size_t n = state->nclients - i - 1;
            memmove(&state->clients[i], &state->clients[i + 1], n * sizeof(client));
            state->nclients--;

        } else
            i++;
    }
}

static bool accept_connections(server_state* state, struct pollfd* pfds)
{
    if (pfds[0].revents & POLLIN) {
        struct sockaddr_storage addr;
        socklen_t addr_len = sizeof(struct sockaddr_storage);
        int connfd = accept(pfds[0].fd, (struct sockaddr*)&addr, &addr_len);
        if (connfd == -1) {
            perror("accept");
            return false;
        }
        if (!add_client(state, connfd)) {
            return false;
        }

        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];
        int flags = NI_NUMERICHOST | NI_NUMERICSERV;
        int rc = getnameinfo((struct sockaddr*)&addr, addr_len, 
                host, sizeof(host), 
                serv, sizeof(serv), 
                flags);
        if (rc) {
            fprintf(stderr, "getnameinfo: %s\n", gai_strerror(rc));
            return false;
        }
        printf("Client connected (%s:%s) | Total: %zu\n", host, serv, state->nclients);
    }
    return true;
}

static bool handle_packet(server_state* state, 
                          client* client,
                          packet_type ptype,
                          void* payload,
                          size_t payload_size)
{
    (void)payload_size;
    if (client->status == CLIENT_CONNECTING &&
        ptype != PT_CONNECT) {
        printf("The client is not connected (fd = %d)\n",
                client->fd);
        free(payload);
        return false;
    }

    switch (ptype) {
        case PT_CONNECT:
            printf("Received a CONNECT packet from client (fd=%d)\n",
                    client->fd);

            uint32_t snake_id;
            if (!game_add_player_snake(&state->gs, &snake_id)) {
                fprintf(stderr, "game_add_player_snake() failed\n");
                return false;
            }

            client->snake_id = snake_id;
            client->status = CLIENT_CONNECTED;

            connect_ack_payload ack;
            ack.snake_id = htobe32(snake_id);
            if (!send_packet(client->fd,
                             PT_CONNECT_ACK,
                             &ack,
                             sizeof(ack))) {
                fprintf(stderr, "send_packet failed\n");
                return false;
            }
            break;
        case PT_INPUT:
            ;
            input_payload* input = payload;
            if (!game_change_snake_direction(&state->gs, client->snake_id, be32toh(input->dir))) {
                    fprintf(stderr, "game_change_snake_direction failed\n");
                    return false;
            }
            break;
        case PT_DISCONNECT:
            break;
        default:
            fprintf(stderr, "Invalid packet type (%d)\n",
                    ptype);
            return false;
    }
    return true;
}

static bool process_client(server_state* state, client* client, struct pollfd* pfd)
{
    bool ret = true;
    if (pfd->revents & POLLIN) {
        packet_type ptype;
        void* payload;
        size_t payload_size;
        recv_status status = recv_packet(pfd->fd, &ptype,
                                         &payload, &payload_size);
        if (status == RECV_CLOSED) {
            client->status = CLIENT_DISCONNECTED;
            fprintf(stdout, "Client disconnected (fd = %d)\n", client->fd);
            return true;
        } else if (status != RECV_OK) {
            fprintf(stderr, "recv_packet failed\n");
            free(payload);
            return false;
        }

        if (!handle_packet(state, client,
                           ptype, payload, payload_size)) {
            fprintf(stderr, "handle_packet failed\n");
            ret = false;
        }
        free(payload);
    }
    return ret;
}

static bool process_clients(server_state* state,
        struct pollfd* pfds)
{
    for (size_t i = 0; i < state->nclients; i++) {
        if (pfds[i + 1].revents & POLLIN) {
            if (!process_client(state, &state->clients[i], &pfds[i + 1])) {
                fprintf(stderr, "process client (fd = %d) failed\n", pfds[i].fd);
            }
        }
    }
    return true;
}

static bool broadcast_game_state(server_state* state)
{
    uint8_t* payload;
    size_t size;
    if (!game_state_serialize(&state->gs, &payload, &size)) {
        fprintf(stderr, "game_state_serialize failed\n");
        return false;
    }

    for (size_t i = 0; i < state->nclients; i++) {
        client* cl = &state->clients[i];
        if (cl->status == CLIENT_CONNECTED) {
            if (!send_packet(cl->fd, PT_GAME_STATE, payload, size)) {
                fprintf(stderr, "send_packet failed (fd = %d)\n", cl->fd);
                free(payload);
                return false;
            }
        }
    }

    free(payload);

    return true;
}

static uint64_t get_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

bool server_run(const char* port, int width, int height)
{
    server_state state;
    if (!server_state_init(&state, port, width, height)) {
        fprintf(stderr, "server_state_init failed\n");
        return false;
    }
    printf("Server is running on port: %s\n", port);

    bool rc = true;
    struct pollfd pfds[MAX_CLIENTS + 1];
    int nready;
    int poll_timeout = 0;
    uint64_t last_update_time = get_time_ms();
    while (1) {
        uint64_t cur_time = get_time_ms();
        poll_timeout = TICK_MS - (cur_time - last_update_time);
        if (poll_timeout < 0)
            poll_timeout = 0;

        remove_disconnected_clients(&state);

        pfds[0].fd = state.listenfd;
        pfds[0].events = POLLIN;
        for (size_t i = 0; i < MAX_CLIENTS; i++) {
            if (i < state.nclients) {
                pfds[i + 1].fd = state.clients[i].fd;
                pfds[i + 1].events = POLLIN;
            } else {
                pfds[i + 1].fd = -1;
            }
        }

        nready = poll(pfds, state.nclients + 1, poll_timeout);
        if (nready == -1) {
            perror("poll");
            rc = false;
            break;
        }

        if (!accept_connections(&state, pfds)) {
            fprintf(stderr, "accept_connections failed\n");
            rc = false;
            break;
        }

        if (!process_clients(&state, pfds)) {
            fprintf(stderr, "process_clients failed\n");
            rc = false;
            break;
        }

        cur_time = get_time_ms();
        if (cur_time - last_update_time >= TICK_MS) {
            if (!game_update(&state.gs)) {
                fprintf(stderr, "game_update failed\n");
                rc = false;
                break;
            }

            if (!broadcast_game_state(&state)) {
                fprintf(stderr, "broadcast_game_state failed\n");
                rc = false;
                break;
            }

            last_update_time += TICK_MS;
        }
    }

    server_state_destroy(&state);
    return rc;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include "server.h"

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
    state->nclients++;
    return true;
}

static bool process_clients(server_state* state,
                            struct pollfd* pfds, 
                            int nready)
{
    for (size_t i = 1; i < state->nclients - 1 && nready > 0; i++) {
        if (pfds[i].revents & POLLIN) {
            // process_client
            nready--;
        }
    }
    return true;
}

int main(int argc, char* argv[])
{
    /* board size */
    int width = 30;
    int height = 20;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    server_state state;
    if (!server_state_init(&state, argv[1], width, height)) {
        fprintf(stderr, "server_state_init failed\n");
        return -1;
    }

    struct pollfd pfds[MAX_CLIENTS + 1];
    int nready;
    int timeout = 20;
    while (1) {
        pfds[0].fd = state.listenfd;
        pfds[0].events = POLLIN;
        for (size_t i = 0; i < state.nclients; i++) {
            pfds[i + 1].fd = state.clients[i].fd;
            pfds[i + 1].events = POLLIN;
        }

        nready = poll(pfds, state.nclients + 1, timeout);
        if (nready == -1) {
            perror("poll");
            return -1;
        }

        if (pfds[0].revents != 0) {
            if (pfds[0].revents & POLLIN) {
                struct sockaddr_storage addr;
                socklen_t addr_len = sizeof(struct sockaddr_storage);
                int connfd = accept(pfds[0].fd, (struct sockaddr*)&addr, &addr_len);
                if (connfd == -1) {
                    perror("accept");
                    return -1;
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
                    return -1;
                }
                printf("Client connected (%s:%s)\n", host, serv);

                if (!add_client(&state, connfd)) {
                    return -1;
                }
            }
            nready--;
        }
        process_clients(pfds, state.nclients + 1, nready);

    }

    server_state_destroy(&state);
}

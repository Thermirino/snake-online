#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <protocol.h>
#include <string.h>

/* static functions */
static ssize_t recv_all(int fd, void* usrbuf, size_t n);
static ssize_t send_all(int fd, const void* usrbuf, size_t n);

const char* packet_type_str(packet_type ptype)
{
    switch (ptype) {
        case PT_CONNECT:
            return "PT_CONNECT";
        case PT_CONNECT_ACK:
            return "PT_CONNECT_ACK";
        case PT_INPUT:
            return "PT_INPUT";
        case PT_GAME_STATE:
            return "PT_GAME_STATE";
        case PT_DISCONNECT:
            return "PT_DISCONNECT";
        default:
            return "Unknown";
    }
}

bool game_state_serialize(const game_state* gs, uint8_t** buf, size_t* size)
{
    if (!gs || !buf || !size)
        return false;

    // calculate buffer size
    uint64_t buf_size = 0;
    buf_size += sizeof(game_state_header);
    for (size_t i = 0; i < gs->snakes_size; i++) {
        buf_size += sizeof(snake_header);
        buf_size += sizeof(point_data) * gs->snakes[i].body.size;
    }
    for (size_t i = 0; i < gs->food_size; i++) {
        buf_size += sizeof(point_data);
    }

    *buf = malloc(buf_size);
    if (!*buf) {
        perror("malloc");
        return false;
    }
    uint8_t* p = *buf;

    game_state_header ghdr;
    ghdr.width = htobe32(gs->brd.width);
    ghdr.height = htobe32(gs->brd.height);
    ghdr.nsnakes = htobe64(gs->snakes_size);
    ghdr.nfood = htobe64(gs->food_size);
    memcpy(p, &ghdr, sizeof(ghdr));
    p += sizeof(ghdr);

    // serialize snakes
    snake_header shdr;
    for (size_t i = 0; i < gs->snakes_size; i++) {
        shdr.id = htobe32(gs->snakes[i].id);
        shdr.dir = htobe32(gs->snakes[i].dir);
        shdr.color = htobe32(gs->snakes[i].color);
        shdr.npoints = htobe64(gs->snakes[i].body.size);
        memcpy(p, &shdr, sizeof(shdr));
        p += sizeof(shdr);

        point_data pdata;
        for (size_t j = 0; j < gs->snakes[i].body.size; j++) {
            pdata.y = htobe32(gs->snakes[i].body.points[j].y);
            pdata.x = htobe32(gs->snakes[i].body.points[j].x);
            memcpy(p, &pdata, sizeof(pdata));
            p += sizeof(pdata);
        }
    }

    // serialize food
    point_data pdata;
    for (size_t i = 0; i < gs->food_size; i++) {
        pdata.y = htobe32(gs->food[i].y);
        pdata.x = htobe32(gs->food[i].x);

        memcpy(p, &pdata, sizeof(pdata));
        p += sizeof(pdata);
    }

    if (p != *buf + buf_size)
        return false;

    *size = buf_size;
    return true;
}

bool game_state_deserialize(uint8_t* data, size_t data_size, game_state* gs)
{
    if (!data || !gs)
        return false;

    uint8_t* p = data;
    uint8_t* end = p + data_size;

    game_state_header ghdr;
    if (p + sizeof(ghdr) > end) {
        fprintf(stderr, "Incorrect data\n");
        return false;
    }
    memcpy(&ghdr, p, sizeof(ghdr));
    p += sizeof(ghdr);
    gs->brd.width = be32toh(ghdr.width);
    gs->brd.height = be32toh(ghdr.height);
    gs->snakes_size = be64toh(ghdr.nsnakes);

    gs->snakes_capacity = gs->snakes_size;

    // deserialize snakes
    if (gs->snakes_size == 0) {
        gs->snakes = NULL;
    } else {
        snake_header shdr;
        gs->snakes = malloc(gs->snakes_capacity * sizeof(snake));
        if (!gs->snakes) {
            perror("malloc");
            return false;
        }

        for (size_t i = 0; i < gs->snakes_size; i++) {
            if (p + sizeof(shdr) > end) {
fprintf(stderr, "Incorrect data\n");
                for (size_t k = 0; k < i; k++) {
                    free(gs->snakes[k].body.points);
                }
                free(gs->snakes);
                return false;
            }

            memcpy(&shdr, p, sizeof(shdr));
            p += sizeof(shdr);
            gs->snakes[i].id = be32toh(shdr.id);
            gs->snakes[i].dir = be32toh(shdr.dir);
            gs->snakes[i].color = be32toh(shdr.color);
            gs->snakes[i].body.size = be64toh(shdr.npoints);
            gs->snakes[i].body.capacity = gs->snakes[i].body.size;

            gs->snakes[i].body.points = malloc(gs->snakes[i].body.size * sizeof(point));
            if (!gs->snakes[i].body.points) {
                perror("malloc");
                for (size_t k = 0; k <= i; k++) {
                    free(gs->snakes[k].body.points);
                }
                free(gs->snakes);
                return false;
            }

            point_data pdata;
            for (size_t j = 0; j < gs->snakes[i].body.size; j++) {
                if (p + sizeof(pdata) > end) {
                    fprintf(stderr, "Incorrect data\n");
                    for (size_t k = 0; k <= i; k++) {
                        free(gs->snakes[k].body.points);
                    }
                    free(gs->snakes);
                    return false;
                }

                memcpy(&pdata, p, sizeof(pdata));
                p += sizeof(pdata);
                gs->snakes[i].body.points[j].y = be32toh(pdata.y);
                gs->snakes[i].body.points[j].x = be32toh(pdata.x);
            }
        }
    }

    // deserialize food
    gs->food_size = be64toh(ghdr.nfood);
    gs->food_capacity = gs->food_size;

    gs->food = malloc(gs->food_capacity * sizeof(point));
    if (!gs->food) {
        perror("malloc");
        for (size_t k = 0; k < gs->snakes_size; k++) {
            free(gs->snakes[k].body.points);
        }
        free(gs->snakes);
        return false;
    }

    point_data pdata;
    for (size_t i = 0; i < gs->food_size; i++) {
        if (p + sizeof(pdata) > end) {
            fprintf(stderr, "Incorrect data\n");
            free(gs->food);
            for (size_t k = 0; k < i; k++) {
                free(gs->snakes[k].body.points);
            }
            free(gs->snakes);
            return false;
        }

        memcpy(&pdata, p, sizeof(pdata));
        p += sizeof(pdata);
        gs->food[i].y = be32toh(pdata.y);
        gs->food[i].x = be32toh(pdata.x);
    }

    if (p != data + data_size) {
        fprintf(stderr, "Incorrect data\n");
        free(gs->food);
        for (size_t k = 0; k < gs->snakes_size; k++) {
            free(gs->snakes[k].body.points);
        }
        free(gs->snakes);
        return false;
    }
    return true;
}

static ssize_t recv_all(int fd, void* usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nread;
    char* bufp = usrbuf;

    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        }
        else if (nread == 0)
            break;
        nleft -= nread;
        bufp += nread;
    }
    return n - nleft;
}

static ssize_t send_all(int fd, const void* usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    const char* bufp = usrbuf;

    while (nleft > 0) {
        if ((nwritten = write(fd, bufp, nleft)) <= 0) {
            if (errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;

}

recv_status recv_packet(int fd, packet_type* ptype, void** payload, size_t* payload_size)
{
    if (!ptype || !payload || !payload_size)
        return RECV_ERROR;

    packet_header phdr;
    ssize_t nbytes = recv_all(fd, &phdr, sizeof(phdr));
    if (nbytes < 0) {
        perror("recv_all");
        return RECV_ERROR;
    } else if (nbytes == 0) {
        return RECV_CLOSED;
    } else if (nbytes != sizeof(phdr)) {
        fprintf(stderr, "Received insufficient data\n");
        return RECV_ERROR;
    }
    *ptype = be32toh(phdr.type);
    if (*ptype < 0 || *ptype > PT_COUNT) {
        fprintf(stderr, "Invalid packet type (%d)\n", *ptype);
        return RECV_ERROR;
    }
    size_t packet_size = be64toh(phdr.size);

    *payload_size = packet_size - sizeof(phdr);
    if (*payload_size != 0) {
        *payload = malloc(*payload_size);
        if (!*payload) {
            perror("malloc");
            return RECV_ERROR;
        }
        nbytes = recv_all(fd, *payload, *payload_size);
        if (nbytes < 0) {
            free(*payload);
            perror("recv_all");
            return RECV_ERROR;
        } else if ((size_t)nbytes != *payload_size) {
            free(*payload);
            fprintf(stderr, "Received insufficient data\n");
            return RECV_ERROR;
        }
    } else
        *payload = NULL;

    LOG_NET("Received packet, fd = %d, type = %s, payload_size = %zu\n",
            fd, packet_type_str(*ptype), *payload_size);

    return RECV_OK;
}

bool send_packet(int fd, packet_type ptype, const void* payload, size_t payload_size)
{
    if (ptype < 0 || ptype > PT_COUNT) {
        fprintf(stderr, "Invalid packet type (%d)\n", ptype);
        return false;
    }

    packet_header phdr;
    phdr.size = htobe64(sizeof(phdr) + payload_size);
    phdr.type = htobe32(ptype);
    ssize_t nbytes = send_all(fd, &phdr, sizeof(phdr));
    if (nbytes < 0) {
        perror("send_all");
        return false;
    } else if (nbytes != sizeof(phdr)) {
        fprintf(stderr, "send_all error\n");
        return false;
    }

    if (payload_size != 0) {
        nbytes = send_all(fd, payload, payload_size);
        if (nbytes < 0) {
            perror("send_all");
            return false;
        } else if ((size_t)nbytes != payload_size) {
            fprintf(stderr, "send_all error\n");
            return false;
        }
    }

    LOG_NET("Sent packet, fd = %d, type = %s, payload_size = %zu\n",
            fd, packet_type_str(ptype), payload_size);

    return true;
}

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <protocol.h>
#include <string.h>

bool game_state_serialize(const game_state* gs, uint8_t** buf, size_t* size)
{
    if (!gs || !buf || !size)
        return false;

    uint64_t buf_size = 0;
    buf_size += sizeof(packet_header);
    buf_size += sizeof(game_state_header);
    for (size_t i = 0; i < gs->snakes_size; i++) {
        buf_size += sizeof(snake_header);
        buf_size += sizeof(point_data) * gs->snakes[i].body.size;
    }

    *buf = malloc(buf_size);
    if (!*buf) {
        perror("malloc");
        return false;
    }
    uint8_t* p = *buf;
    
    packet_header phdr;
    phdr.size = htobe64(buf_size);
    phdr.type = htobe32(PT_GAME_STATE);
    memcpy(p, &phdr, sizeof(phdr));
    p += sizeof(phdr);

    game_state_header ghdr;
    ghdr.width = htobe32(gs->brd.width);
    ghdr.height = htobe32(gs->brd.height);
    ghdr.nsnakes = htobe64(gs->snakes_size);
    memcpy(p, &ghdr, sizeof(ghdr));
    p += sizeof(ghdr);

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

    if (p != *buf + buf_size)
        return false;

    *size = buf_size;
    return true;
}

bool game_state_deserialize(uint8_t* data, game_state* gs)
{
    if (!data || !gs)
        return false;

    uint8_t* p = data;

    packet_header phdr;
    memcpy(&phdr, p, sizeof(phdr));
    uint64_t packet_size = be64toh(phdr.size);
    uint32_t packet_type = be32toh(phdr.type);
    if (packet_type != PT_GAME_STATE)
        return false;
    p += sizeof(phdr);

    game_state_header ghdr;
    memcpy(&ghdr, p, sizeof(ghdr));
    gs->brd.width = be32toh(ghdr.width);
    gs->brd.height = be32toh(ghdr.height);
    gs->snakes_size = be64toh(ghdr.nsnakes);
    p += sizeof(ghdr);

    gs->snakes_capacity = gs->snakes_size;

    snake_header shdr;
    if (gs->snakes_size == 0) {
        gs->snakes = NULL;
    } else {
        gs->snakes = malloc(gs->snakes_capacity * sizeof(snake));
        if (!gs->snakes) {
            perror("malloc");
            return false;
        }
        for (size_t i = 0; i < gs->snakes_size; i++) {
            memcpy(&shdr, p, sizeof(shdr));
            gs->snakes[i].id = be32toh(shdr.id);
            gs->snakes[i].dir = be32toh(shdr.dir);
            gs->snakes[i].color = be32toh(shdr.color);
            gs->snakes[i].body.size = be64toh(shdr.npoints);
            p += sizeof(shdr);

            gs->snakes[i].body.points = malloc(gs->snakes[i].body.size * sizeof(point));
            if (!gs->snakes[i].body.points) {
                perror("malloc");
                for (size_t j = 0; j < gs->snakes_size; j++) {
                    free(gs->snakes[j].body.points);
                }
                free(gs->snakes);
                return false;
            }

            point_data pdata;
            for (size_t j = 0; j < gs->snakes[i].body.size; j++) {
                memcpy(&pdata, p, sizeof(pdata));
                gs->snakes[i].body.points[j].y = be32toh(pdata.y);
                gs->snakes[i].body.points[j].x = be32toh(pdata.x);
                p += sizeof(pdata);
            }
        }
    }

    if (p != data + packet_size)
        return false;
    return true;
}

ssize_t recv_all(int fd, void* usrbuf, size_t n)
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

ssize_t send_all(int fd, const void* usrbuf, size_t n)
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

bool recv_packet(int fd, packet_type* ptype, void** payload, size_t* payload_size)
{
    if (!ptype || !payload || !payload_size)
        return false;

    packet_header phdr;
    ssize_t nbytes = recv_all(fd, &phdr, sizeof(phdr));
    if (nbytes < 0) {
        perror("recv_all");
        return false;
    } else if (nbytes != sizeof(phdr)) {
        fprintf(stderr, "Received insufficient data\n");
        return false;
    }
    *ptype = be32toh(phdr.type);
    if (*ptype < 0 || *ptype > 2) {
        fprintf(stderr, "Invalid packet type (%d)\n", *ptype);
        return false;
    }
    size_t packet_size = be64toh(phdr.size);

    *payload_size = packet_size - sizeof(phdr);
    if (*payload_size != 0) {
        *payload = malloc(*payload_size);
        if (!*payload) {
            perror("malloc");
            return false;
        }
        nbytes = recv_all(fd, *payload, *payload_size);
        if (nbytes < 0) {
            free(*payload);
            perror("recv_all");
            return false;
        } else if ((size_t)nbytes != *payload_size) {
            free(*payload);
            fprintf(stderr, "Received insufficient data\n");
            return false;
        }
    } else
        *payload = NULL;
    return true;
}

bool send_packet(int fd, packet_type type, const void* payload, size_t payload_size)
{
    packet_header phdr;
    phdr.size = htobe64(sizeof(phdr) + payload_size);
    phdr.type = htobe32(type);
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
    return true;
}

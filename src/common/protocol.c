#include <stdio.h>
#include <stdlib.h>
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
        gs->snakes = malloc(gs->snakes_capacity);
        if (!gs->snakes) {
            perror("malloc");
            return false;
        }
        for (size_t i = 0; i < gs->snakes_size; i++) {
            memcpy(&shdr, p, sizeof(shdr));
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

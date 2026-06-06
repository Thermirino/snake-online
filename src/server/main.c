#include "server.h"
#include <stdio.h>
#include <server.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return -1;
    }

    int width = 20;
    int height = 10;
    if (!server_run(argv[1], width, height))
        return -1;
    return 0;
}

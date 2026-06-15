#include <stdio.h>
#include <client.h>

int main(int argc, char* argv[])
{
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <hostname> <port> [nickname]\n", argv[0]);
        return -1;
    }
    char* host = argv[1];
    char* port = argv[2];

    char* nickname = NULL;
    if (argc == 4)
        nickname = argv[3];

    int win_width = 1024;
    int win_height = 512;

    if (!client_run(host, port, nickname, win_width, win_height))
        return -1;
    return 0;
}

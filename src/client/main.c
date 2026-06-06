#include <stdio.h>
#include <client.h>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
        return -1;
    }
    if (!client_run(argv[1], argv[2]))
        return -1;
    return 0;
}

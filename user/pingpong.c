#include "user/user.h"

int
main(int argc, char *argv[])
{
    int p[2];
    pipe(p);
    if (fork() == 0)
    {
        char buf[512];
        read(p[0], buf, sizeof buf);
        printf("%d: received %s\n", getpid(), buf);
        write(p[1], "pong", 4);
        exit(0);
    }
    else
    {
        char buf[512];
        write(p[1], "ping", 4);
        wait(0);
        read(p[0], buf, 4);
        printf("%d: received %s\n", getpid(), buf);
    }
    return 0;
}

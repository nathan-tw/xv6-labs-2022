#include "user/user.h"
#include "user/primes.h"

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        printf("Usage: primes");
        exit(1);
    }

    int p[2];
    pipe(p);

    if (fork() == 0)
    {
        redirect(0, p); // child 從 pipe 讀取
        primes();
    }
    else
    {
        redirect(1, p);
        for (int i = 2; i <= 35; i++)
            write(1, &i, 4);
        close(1); // 發送 eof 給 fd 1 ，第48行的 read 才會收到結束的訊息。
        wait(0);
    }
    return 0;
}

int primes()
{
    int p[2];
    int i;
    if (read(0, &i, 4))
    {
        printf("prime %d\n", i);
        pipe(p);
        if (fork() == 0)
        {
            redirect(0, p);
            primes();
        }
        else
        {
            int j;
            redirect(1, p);
            while (read(0, &j, 4))
                if (j % i)
                    write(1, &j, 4);
            close(1);
            wait(0);
        }
    }
    exit(0);
}

void redirect(int k, int p[])
{
    close(k);
    dup(p[k]);
    close(p[k]);
    close(p[1 - k]);
}

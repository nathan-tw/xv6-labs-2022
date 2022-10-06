#include "user/user.h"
#include "kernel/param.h"

void xargs(char* argv[]);

int main(int argc, char* argv[]) {
    char* argvs[MAXARG];
    char buf[128];
    char c;
    char *p = buf;

    memset(argvs, 0, sizeof(argvs));
    for(int i = 1; i < argc; i++) {
        argvs[i-1] = argv[i];
    }
    argvs[argc-1] = buf;
    while(read(0, &c, sizeof(c))) {
        if (c != '\n' && p < (buf + sizeof(buf))) {
            *p++ = c;
        } else {
            *p = '\0';
            xargs(argvs);
            p = buf;
        }
    }
    return 0;
}

void xargs(char* argv[]) {
    if(fork() == 0) {
        close(0);
        exec(argv[0], argv);
    } else {
        wait(0);
    }
}
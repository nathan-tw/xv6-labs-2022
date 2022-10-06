#include "user/user.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"

void find(char *path, char *target);
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: find path target\n");
        exit(1);
    }

    find(argv[1], argv[2]);
    return 0;
}

void find(char *path, char *target)
{
    char buf[512], *p;
    int fd;
    struct stat st;
    struct dirent de;
    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    if (st.type != T_DIR)
    {
        fprintf(2, "find: %s is not a directory\n", path);
        close(fd);
        return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {

        if (de.inum == 0)
            continue;
        if (!strcmp(de.name, ".") || !strcmp(de.name, ".."))
            continue;

        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if (stat(buf, &st) < 0)
            continue;
        switch (st.type)
        {
        case T_DIR:
            find(buf, target);
            break;
        case T_FILE:
            if(strcmp(target, de.name) == 0)
                printf("%s/%s\n", path, target);
            break;
        }
    }
}
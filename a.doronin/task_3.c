#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

static void print_ids(const char *tag)
{
    printf("%s\n", tag);
    printf(" real uid: %d, effective uid: %d\n", (int)getuid(), (int)geteuid());
    printf(" real gid: %d, effective gid: %d\n", (int)getgid(), (int)getegid());
}

static void try_open(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL)
        perror("fopen");
    else
    {
        printf("Successfully opened '%s' for reading\n", path);
        fclose(f);
    }
}

int main(int argc, char *argv[])
{
    const char *path = "list";
    if (argc >= 2)
        path = argv[1];

    print_ids("  before changes  ");

    try_open(path);

    if (geteuid() != getuid())
    {
        if (setuid(geteuid()) == -1)
        {
            perror("setuid");
            printf("setuid failed\n");
        }
        else
        {
            printf("setuid(geteuid()) done\n");
        }
    }
    else
    {
        printf("real uid == effective uid\n");
    }

    print_ids("  after changes  ");
    try_open(path);

    return 0;
}
